// ============================================================================
//  Smart Solar-Powered Light Trap — MYOSA + ESP32
//  Firmware with dual Wi-Fi mode (AP + STA) and local / cloud storage.
// ============================================================================

#include <Wire.h>
#include <BarometricPressure.h>
#include <oled.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

Preferences preferences;

// ----------------------------------------------------------------------------
//  I2C
// ----------------------------------------------------------------------------
#define SDA_PIN 21
#define SCL_PIN 22
SemaphoreHandle_t xI2C_Mutex;

// ----------------------------------------------------------------------------
//  BMP180 / OLED / RTC
// ----------------------------------------------------------------------------
BarometricPressure Pr(ULTRA_HIGH_RESOLUTION);
oLed display(SCREEN_WIDTH, SCREEN_HEIGHT);
RTC_DS1307 rtc;

// ----------------------------------------------------------------------------
//  Operating windows (NVS-backed)
// ----------------------------------------------------------------------------
int apStartH = 8,  apStartM = 0;
int apEndH   = 23, apEndM   = 50;

int ledStartH = 19, ledStartM = 30;
int ledEndH   = 4,  ledEndM   = 0;

int fanStartH = 19, fanStartM = 30;
int fanEndH   = 4,  fanEndM   = 0;

int sdStartH = 18, sdStartM = 30;
int sdEndH   = 23, sdEndM   = 50;

// ----------------------------------------------------------------------------
//  SD Card
// ----------------------------------------------------------------------------
#define SD_CS 5
int lastLoggedDay = -1;
unsigned long lastLogTime = 0;
const unsigned long logInterval = 60000 * 10;  // 10 min

String folderBMP      = "/Data";
String monthFolderBMP = "";
String filenameBMP    = "";

const char *pendingFolder = "/Pending";
const char *pendingFile   = "/Pending/queue.csv";

// ----------------------------------------------------------------------------
//  Actuators
// ----------------------------------------------------------------------------
bool manualFanOverride = false;
bool manualFanState    = false;
bool manualLedOverride = false;
bool manualLedState    = false;

const int ledPin     = 26;
const int Freq       = 1000;
const int resolution = 8;
const int dutyCycle  = 50;   // ~20%

#define FAN_PIN 27
int AREA_THRESHOLD = 10;
int areaCounter    = 0;
int FAN_ON_TIME    = 60000;
const unsigned long EVALUATION_WINDOW = 60000 * 5;
unsigned long fanStartTime  = 0;
unsigned long lastEvaluation = 0;

#define INTERRUPT_BUTTON 35
enum DisplayMode { MODE_BMP, MODE_OPERATING_WINDOWS, MODE_TOTAL };
volatile DisplayMode currentMode = MODE_BMP;

bool forceAwake = false;
unsigned long forceAwakeStart = 0;

// ----------------------------------------------------------------------------
//  Global state
// ----------------------------------------------------------------------------
struct SystemData {
  float temperature, pressure, altitude;
  int hour, minute, second, day, month, year;
  bool ledState;
  bool loadState;
  int  areaCount;
  bool rtcAvailable, bmpAvailable, sdAvailable;
};
SystemData systemData = {0};

// ----------------------------------------------------------------------------
//  WiFi: AP + STA (dual mode)
// ----------------------------------------------------------------------------
WebServer server(80);

const char *apSsid     = "MYOSA_BoardServer";
const char *apPassword = "prueba123";
bool apIsActive = false;

String staSsid     = "";
String staPassword = "";
bool   staEnabled  = false;
bool   staConnected = false;
unsigned long lastStaCheck = 0;
const unsigned long staCheckInterval = 30000;

// ----------------------------------------------------------------------------
//  Storage mode
// ----------------------------------------------------------------------------
enum StorageMode { STORAGE_LOCAL = 0, STORAGE_CLOUD = 1, STORAGE_BOTH = 2 };
StorageMode storageMode = STORAGE_LOCAL;

// ----------------------------------------------------------------------------
//  Cloud
// ----------------------------------------------------------------------------
const char *cloudEndpoint = "https://spodoptera.vercel.app/api/measurements";
String deviceKey = "";
String deviceId  = "esp32-trap-001";

// ----------------------------------------------------------------------------
//  Prototypes
// ----------------------------------------------------------------------------
void getTime();
void readBMP();
void showData();
void LED_OP();
void evaluateArea();
void controlFan();

void updateFileNames();
void initLogFolders();
void logBMP();

bool isInsideWindow(int sH, int sM, int eH, int eM);
bool operatingWindow_AP();
bool operatingWindow_SD();
bool operatingWindow_LED();
bool operatingWindow_Fan();

void startSTA();
void stopSTA();
void manageSTA();
bool sendToCloud(const JsonDocument &doc);
void queuePending(const String &csvLine);
void flushPendingBuffer();

void handleRoot();
void handleStatic();
void handleEstado();
void handleArchives();
void handleDownload();
void handleSaveWindows();
void handleControl();
void handleClearManual();
void handleWifi();
void handleStorageMode();
void handleConnState();

// ----------------------------------------------------------------------------
//  Interrupts / Deep sleep
// ----------------------------------------------------------------------------
volatile unsigned long lastInterruptTime = 0;
const unsigned long INTERRUPT_DEBOUNCE = 300;

void IRAM_ATTR Interrupt_rutine() {
  unsigned long now = millis();
  if (now - lastInterruptTime > INTERRUPT_DEBOUNCE) {
    currentMode = (DisplayMode)((currentMode + 1) % MODE_TOTAL);
    lastInterruptTime = now;
  }
}

void checkDeepSleep() {
  bool activeAP  = isInsideWindow(apStartH, apStartM, apEndH, apEndM);
  bool activeLED = isInsideWindow(ledStartH, ledStartM, ledEndH, ledEndM);
  bool activeFan = isInsideWindow(fanStartH, fanStartM, fanEndH, fanEndM);
  bool activeSD  = isInsideWindow(sdStartH, sdStartM, sdEndH, sdEndM);

  bool anyManual = manualLedOverride || manualFanOverride;

  if (!activeAP && !activeLED && !activeFan && !activeSD && !anyManual) {
    if (forceAwake && (millis() - forceAwakeStart < 300000)) return;
    Serial.println("Entering deep sleep...");

    if (xSemaphoreTake(xI2C_Mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
      digitalWrite(ledPin, LOW);

      if (staConnected) {
        WiFi.disconnect(true);
        staConnected = false;
      }

      DateTime now = rtc.now();
      int currentTotal = now.hour() * 60 + now.minute();
      int starts[] = {
        apStartH * 60 + apStartM,
        ledStartH * 60 + ledStartM,
        sdStartH * 60 + sdStartM,
        fanStartH * 60 + fanStartM
      };
      int minWait = 1440;
      for (int i = 0; i < 4; i++) {
        int diff = (starts[i] > currentTotal)
                     ? (starts[i] - currentTotal)
                     : (1440 - currentTotal + starts[i]);
        if (diff < minWait) minWait = diff;
      }
      long sleepSeconds = (minWait * 60) - now.second();
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 0);
      esp_sleep_enable_timer_wakeup((uint64_t)sleepSeconds * 1000000ULL);
      Serial.printf("Sleeping %ld seconds...\n", sleepSeconds);
      Serial.flush();
      delay(100);
      esp_deep_sleep_start();
    }
  }
}

// ----------------------------------------------------------------------------
//  FreeRTOS tasks
// ----------------------------------------------------------------------------
void taskControl(void *pv) {
  for (;;) {
    LED_OP();
    evaluateArea();
    controlFan();
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

void taskUI(void *pv) {
  for (;;) {
    if (xSemaphoreTake(xI2C_Mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      if (systemData.rtcAvailable) getTime();
      if (systemData.bmpAvailable) readBMP();
      showData();
      xSemaphoreGive(xI2C_Mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void taskComm(void *pv) {
  for (;;) {
    bool shouldBeActive = operatingWindow_AP() || forceAwake;
    if (shouldBeActive && !apIsActive) {
      WiFi.mode(staEnabled ? WIFI_AP_STA : WIFI_AP);
      WiFi.softAP(apSsid, apPassword);
      Serial.print("AP IP: ");
      Serial.println(WiFi.softAPIP());
      if (staEnabled) startSTA();
      server.begin();
      apIsActive = true;
    } else if (!shouldBeActive && apIsActive) {
      WiFi.softAPdisconnect(true);
      if (staEnabled && staConnected) {
        WiFi.disconnect(true);
        staConnected = false;
      }
      WiFi.mode(WIFI_OFF);
      apIsActive = false;
    }
    if (apIsActive) server.handleClient();

    manageSTA();

    if (systemData.sdAvailable
        && millis() - lastLogTime >= logInterval
        && operatingWindow_SD()) {
      if (xSemaphoreTake(xI2C_Mutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        logBMP();
        xSemaphoreGive(xI2C_Mutex);
        lastLogTime = millis();
      }
    }

    static unsigned long lastSleepCheck = 0;
    if (millis() - lastSleepCheck > 5000) {
      checkDeepSleep();
      lastSleepCheck = millis();
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ============================================================================
//  SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(20);
  Serial.println("Serial started");

  preferences.begin("config", false);

  ledStartH = preferences.getInt("ledSH", ledStartH);
  ledStartM = preferences.getInt("ledSM", ledStartM);
  ledEndH   = preferences.getInt("ledEH", ledEndH);
  ledEndM   = preferences.getInt("ledEM", ledEndM);

  fanStartH = preferences.getInt("fanSH", fanStartH);
  fanStartM = preferences.getInt("fanSM", fanStartM);
  fanEndH   = preferences.getInt("fanEH", fanEndH);
  fanEndM   = preferences.getInt("fanEM", fanEndM);

  apStartH = preferences.getInt("apSH", apStartH);
  apStartM = preferences.getInt("apSM", apStartM);
  apEndH   = preferences.getInt("apEH", apEndH);
  apEndM   = preferences.getInt("apEM", apEndM);

  sdStartH = preferences.getInt("sdSH", sdStartH);
  sdStartM = preferences.getInt("sdSM", sdStartM);
  sdEndH   = preferences.getInt("sdEH", sdEndH);
  sdEndM   = preferences.getInt("sdEM", sdEndM);

  FAN_ON_TIME    = preferences.getInt("fanDur", FAN_ON_TIME);
  AREA_THRESHOLD = preferences.getInt("threshold", AREA_THRESHOLD);

  staSsid     = preferences.getString("staSsid", "");
  staPassword = preferences.getString("staPass", "");
  staEnabled  = staSsid.length() > 0;
  storageMode = (StorageMode)preferences.getInt("storageMode", STORAGE_LOCAL);
  deviceKey   = preferences.getString("devKey", "");
  deviceId    = preferences.getString("devId", "esp32-trap-001");

  preferences.end();

  pinMode(INTERRUPT_BUTTON, INPUT);
  attachInterrupt(INTERRUPT_BUTTON, Interrupt_rutine, FALLING);

  ledcAttach(ledPin, Freq, resolution);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  Wire.begin();
  Wire.setClock(100000);

  if (!rtc.begin()) {
    Serial.println("[ERROR] RTC not found");
    systemData.rtcAvailable = false;
  } else {
    systemData.rtcAvailable = true;
  }
  if (systemData.rtcAvailable && !rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  if (systemData.rtcAvailable) getTime();

  if (!Pr.begin()) {
    Serial.println("BMP180 not detected");
    systemData.bmpAvailable = false;
  } else {
    systemData.bmpAvailable = true;
  }

  SPI.begin();
  delay(10);
  if (!SD.begin(SD_CS)) {
    Serial.println("[ERROR] SD not found — continuing without SD");
    systemData.sdAvailable = false;
  } else {
    systemData.sdAvailable = true;
    initLogFolders();
    if (!SD.exists(pendingFolder)) SD.mkdir(pendingFolder);
  }
  if (systemData.sdAvailable) updateFileNames();

  if (!display.begin()) {
    Serial.println("OLED not detected");
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LightTrap online");
    display.display();
    delay(200);
    display.clearDisplay();
  }

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleStatic);
  server.on("/estado", HTTP_GET, handleEstado);
  server.on("/archivos", HTTP_GET, handleArchives);
  server.on("/descargar", HTTP_GET, handleDownload);
  server.on("/saveWindows", HTTP_POST, handleSaveWindows);
  server.on("/clearManual", HTTP_POST, handleClearManual);
  server.on("/control", HTTP_POST, handleControl);
  server.on("/wifi", HTTP_POST, handleWifi);
  server.on("/storageMode", HTTP_POST, handleStorageMode);
  server.on("/connState", HTTP_GET, handleConnState);
  server.on("/style.css", []() {
    File file = SD.open("/style.css");
    if (file) {
      server.streamFile(file, "text/css");
      file.close();
    } else {
      server.send(404, "text/plain", "CSS Not Found");
    }
  });

  xI2C_Mutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(taskControl, "ControlTask", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(taskUI,      "UITask",      4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(taskComm,    "CommTask",    8192, NULL, 1, NULL, 0);
}

void loop() { }

// ============================================================================
//  STA management
// ============================================================================
void startSTA() {
  if (staSsid.length() == 0) return;
  Serial.printf("Connecting STA to %s...\n", staSsid.c_str());
  WiFi.begin(staSsid.c_str(), staPassword.c_str());
}

void stopSTA() {
  WiFi.disconnect(true);
  staConnected = false;
}

void manageSTA() {
  if (!staEnabled || !apIsActive) return;

  if (millis() - lastStaCheck < staCheckInterval && lastStaCheck > 0) return;
  lastStaCheck = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (!staConnected) {
      staConnected = true;
      Serial.print("STA connected. IP: ");
      Serial.println(WiFi.localIP());
      flushPendingBuffer();
    }
  } else {
    if (staConnected) {
      staConnected = false;
      Serial.println("STA disconnected");
    }
    if (staSsid.length() > 0) {
      WiFi.begin(staSsid.c_str(), staPassword.c_str());
    }
  }
}

// ============================================================================
//  Cloud
// ============================================================================
bool sendToCloud(const JsonDocument &doc) {
  if (!staConnected) return false;

  HTTPClient http;
  http.begin(cloudEndpoint);
  http.addHeader("Content-Type", "application/json");
  if (deviceKey.length() > 0) {
    http.addHeader("X-Device-Key", deviceKey);
  }
  http.addHeader("X-Device-Id", deviceId);

  String payload;
  serializeJson(doc, payload);

  int code = http.POST(payload);
  http.end();

  if (code >= 200 && code < 300) return true;
  Serial.printf("Cloud POST failed: %d\n", code);
  return false;
}

void queuePending(const String &csvLine) {
  if (!systemData.sdAvailable) return;
  File f = SD.open(pendingFile, FILE_APPEND);
  if (!f) return;
  f.println(csvLine);
  f.close();
}

void flushPendingBuffer() {
  if (!systemData.sdAvailable || !staConnected) return;
  if (!SD.exists(pendingFile)) return;

  File f = SD.open(pendingFile, FILE_READ);
  if (!f) return;

  int flushed = 0;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    int idx[5];
    int j = 0;
    for (int i = 0; i < (int)line.length() && j < 5; i++) {
      if (line[i] == ',') idx[j++] = i;
    }
    if (j < 5) continue;

    JsonDocument d;
    d["device_id"] = deviceId;
    d["ts"]   = line.substring(0, idx[0]);
    d["temp"] = line.substring(idx[0] + 1, idx[1]).toFloat();
    d["pres"] = line.substring(idx[1] + 1, idx[2]).toFloat();
    d["alt"]  = line.substring(idx[2] + 1, idx[3]).toFloat();
    d["led"]  = line.substring(idx[3] + 1, idx[4]).toInt() != 0;
    d["fan"]  = line.substring(idx[4] + 1).toInt() != 0;

    if (!sendToCloud(d)) {
      f.close();
      return;
    }
    flushed++;
  }
  f.close();
  SD.remove(pendingFile);
  if (flushed > 0) Serial.printf("Pending flush: %d rows\n", flushed);
}

// ============================================================================
//  Web handlers
// ============================================================================
void handleRoot() {
  File file = SD.open("/dashboard.html");
  if (!file) {
    server.send(500, "text/plain", "Error loading dashboard");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void handleStatic() {
  String path = server.uri();
  if (SD.exists(path)) {
    File file = SD.open(path);
    String contentType = "text/plain";
    if (path.endsWith(".html"))      contentType = "text/html; charset=utf-8";
    else if (path.endsWith(".js"))   contentType = "application/javascript";
    else if (path.endsWith(".css"))  contentType = "text/css";
    server.streamFile(file, contentType);
    file.close();
  } else {
    server.send(404, "text/plain", "File not found");
  }
}

void handleEstado() {
  JsonDocument doc;
  doc["temp"] = systemData.temperature;
  doc["pres"] = systemData.pressure;
  doc["alt"]  = systemData.altitude;

  char fechaBuf[24];
  sprintf(fechaBuf, "%04d-%02d-%02dT%02d:%02d:%02d",
          systemData.year, systemData.month, systemData.day,
          systemData.hour, systemData.minute, systemData.second);
  doc["timestamp"] = fechaBuf;

  doc["led"]        = systemData.ledState;
  doc["fan"]        = systemData.loadState;
  doc["areaCount"]  = systemData.areaCount;
  doc["manualLed"]  = manualLedOverride;
  doc["manualFan"]  = manualFanOverride;

  doc["ledSH"] = ledStartH; doc["ledSM"] = ledStartM;
  doc["ledEH"] = ledEndH;   doc["ledEM"] = ledEndM;
  doc["fanSH"] = fanStartH; doc["fanSM"] = fanStartM;
  doc["fanEH"] = fanEndH;   doc["fanEM"] = fanEndM;
  doc["apSH"]  = apStartH;  doc["apSM"]  = apStartM;
  doc["apEH"]  = apEndH;    doc["apEM"]  = apEndM;
  doc["sdSH"]  = sdStartH;  doc["sdSM"]  = sdStartM;
  doc["sdEH"]  = sdEndH;    doc["sdEM"]  = sdEndM;

  doc["apActive"]  = operatingWindow_AP();
  doc["ledActive"] = operatingWindow_LED();
  doc["sdActive"]  = operatingWindow_SD();
  doc["fanActive"] = operatingWindow_Fan();

  doc["fanDur"]    = FAN_ON_TIME / 60000;
  doc["threshold"] = AREA_THRESHOLD;

  doc["staEnabled"]   = staEnabled;
  doc["staConnected"] = staConnected;
  doc["staSsid"]      = staSsid;
  doc["staIp"]        = staConnected ? WiFi.localIP().toString() : "";
  doc["storageMode"]  = (int)storageMode;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleArchives() {
  if (!server.hasArg("dir")) {
    server.send(400, "text/plain", "Missing dir parameter");
    return;
  }
  String directory = "/" + server.arg("dir");
  if (!SD.exists(directory)) {
    server.send(404, "text/plain", "Directory not found");
    return;
  }
  File root = SD.open(directory);
  if (!root || !root.isDirectory()) {
    server.send(404, "text/plain", "Not a valid directory");
    return;
  }
  JsonDocument doc;
  JsonArray array = doc.to<JsonArray>();
  File file = root.openNextFile();
  while (file) {
    JsonObject item = array.add<JsonObject>();
    item["name"] = file.name();
    item["type"] = file.isDirectory() ? "dir" : "file";
    file = root.openNextFile();
  }
  root.close();
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleDownload() {
  if (!server.hasArg("dir") || !server.hasArg("file")) {
    server.send(400, "text/plain", "Missing parameters");
    return;
  }
  String path = "/" + server.arg("dir") + "/" + server.arg("file");
  if (!SD.exists(path)) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  File file = SD.open(path);
  if (!file) {
    server.send(500, "text/plain", "Error opening file");
    return;
  }
  server.sendHeader("Content-Disposition",
                    "attachment; filename=" + server.arg("file"));
  server.streamFile(file, "text/csv");
  file.close();
}

void handleClearManual() {
  manualLedOverride = false;
  manualFanOverride = false;
  server.send(200, "text/plain", "Manual cleared");
}

void handleControl() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "No body");
    return;
  }
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  if (doc.containsKey("led")) {
    manualLedOverride = true;
    manualLedState    = doc["led"].as<bool>();
  }
  if (doc.containsKey("state")) {
    manualFanOverride = true;
    manualFanState    = doc["state"].as<bool>();
  } else if (doc.containsKey("fan") && doc["fan"].is<bool>()) {
    manualFanOverride = true;
    manualFanState    = doc["fan"].as<bool>();
  }
  server.send(200, "text/plain", "OK");
}

void handleSaveWindows() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (doc.containsKey("fanDur")) {
    int v = constrain(doc["fanDur"].as<int>(), 1, 10);
    FAN_ON_TIME = v * 60000;
  }
  if (doc.containsKey("threshold")) {
    AREA_THRESHOLD = constrain(doc["threshold"].as<int>(), 1, 30);
  }

  apStartH = doc["apSH"]; apStartM = doc["apSM"];
  apEndH   = doc["apEH"]; apEndM   = doc["apEM"];

  ledStartH = doc["ledSH"]; ledStartM = doc["ledSM"];
  ledEndH   = doc["ledEH"]; ledEndM   = doc["ledEM"];

  fanStartH = doc["fanSH"]; fanStartM = doc["fanSM"];
  fanEndH   = doc["fanEH"]; fanEndM   = doc["fanEM"];

  sdStartH = doc["sdSH"]; sdStartM = doc["sdSM"];
  sdEndH   = doc["sdEH"]; sdEndM   = doc["sdEM"];

  preferences.begin("config", false);
  preferences.putInt("ledSH", ledStartH); preferences.putInt("ledSM", ledStartM);
  preferences.putInt("ledEH", ledEndH);   preferences.putInt("ledEM", ledEndM);
  preferences.putInt("fanSH", fanStartH); preferences.putInt("fanSM", fanStartM);
  preferences.putInt("fanEH", fanEndH);   preferences.putInt("fanEM", fanEndM);
  preferences.putInt("apSH",  apStartH);  preferences.putInt("apSM",  apStartM);
  preferences.putInt("apEH",  apEndH);    preferences.putInt("apEM",  apEndM);
  preferences.putInt("sdSH",  sdStartH);  preferences.putInt("sdSM",  sdStartM);
  preferences.putInt("sdEH",  sdEndH);    preferences.putInt("sdEM",  sdEndM);
  preferences.putInt("fanDur",    FAN_ON_TIME / 60000);
  preferences.putInt("threshold", AREA_THRESHOLD);
  preferences.end();

  server.send(200, "application/json", "{\"message\":\"Windows saved\"}");
}

void handleWifi() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  String newSsid = doc["ssid"] | "";
  String newPass = doc["password"] | "";

  if (newSsid.length() == 0) {
    staEnabled = false;
    stopSTA();
    staSsid = "";
    staPassword = "";
  } else {
    staSsid     = newSsid;
    staPassword = newPass;
    staEnabled  = true;
    WiFi.mode(WIFI_AP_STA);
    startSTA();
  }

  preferences.begin("config", false);
  preferences.putString("staSsid", staSsid);
  preferences.putString("staPass", staPassword);
  preferences.end();

  server.send(200, "application/json", "{\"message\":\"WiFi updated\"}");
}

void handleStorageMode() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }
  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  String modeStr = doc["mode"] | "LOCAL";
  if      (modeStr == "LOCAL") storageMode = STORAGE_LOCAL;
  else if (modeStr == "CLOUD") storageMode = STORAGE_CLOUD;
  else if (modeStr == "BOTH")  storageMode = STORAGE_BOTH;

  if (doc.containsKey("deviceKey")) deviceKey = doc["deviceKey"].as<String>();
  if (doc.containsKey("deviceId"))  deviceId  = doc["deviceId"].as<String>();

  preferences.begin("config", false);
  preferences.putInt("storageMode", (int)storageMode);
  preferences.putString("devKey", deviceKey);
  preferences.putString("devId",  deviceId);
  preferences.end();

  server.send(200, "application/json", "{\"message\":\"Storage mode updated\"}");
}

void handleConnState() {
  JsonDocument doc;
  doc["staEnabled"]   = staEnabled;
  doc["staConnected"] = staConnected;
  doc["staSsid"]      = staSsid;
  doc["staIp"]        = staConnected ? WiFi.localIP().toString() : "";
  doc["storageMode"]  = (int)storageMode;
  doc["deviceId"]     = deviceId;
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// ============================================================================
//  Operating windows
// ============================================================================
bool isInsideWindow(int sH, int sM, int eH, int eM) {
  int current = systemData.hour * 60 + systemData.minute;
  int start   = sH * 60 + sM;
  int end     = eH * 60 + eM;
  if (start < end) return (current >= start && current < end);
  return (current >= start || current < end);
}
bool operatingWindow_AP()  { return isInsideWindow(apStartH, apStartM, apEndH, apEndM); }
bool operatingWindow_LED() { return isInsideWindow(ledStartH, ledStartM, ledEndH, ledEndM); }
bool operatingWindow_SD()  { return isInsideWindow(sdStartH, sdStartM, sdEndH, sdEndM); }
bool operatingWindow_Fan() { return isInsideWindow(fanStartH, fanStartM, fanEndH, fanEndM); }

// ============================================================================
//  RTC + BMP180
// ============================================================================
void getTime() {
  DateTime now = rtc.now();
  systemData.hour   = now.hour();
  systemData.minute = now.minute();
  systemData.second = now.second();
  systemData.day    = now.day();
  systemData.month  = now.month();
  systemData.year   = now.year();
}

void readBMP() {
  static float tempSum = 0, presSum = 0, altSum = 0;
  static int sampleCount = 0;
  const int targetSamples = 5;

  if (Pr.ping()) {
    tempSum += Pr.getTempC();
    presSum += Pr.getPressurePascal();
    altSum  += Pr.getAltitude(SEA_LEVEL_AVG_PRESSURE);
  }
  sampleCount++;
  if (sampleCount >= targetSamples) {
    systemData.temperature = tempSum / targetSamples;
    systemData.pressure    = presSum / targetSamples;
    systemData.altitude    = altSum  / targetSamples;
    tempSum = presSum = altSum = 0;
    sampleCount = 0;
  }
}

// ============================================================================
//  LED + FAN
// ============================================================================
void LED_OP() {
  static bool lastState = false;
  bool currentState = manualLedOverride ? manualLedState : operatingWindow_LED();
  systemData.ledState = currentState;
  if (currentState != lastState) {
    ledcWrite(ledPin, currentState ? dutyCycle : 0);
    lastState = currentState;
  }
}

void evaluateArea() {
  if (!operatingWindow_Fan()) return;
  unsigned long now = millis();

  if (areaCounter >= AREA_THRESHOLD && !systemData.loadState) {
    systemData.loadState = true;
    fanStartTime = now;
    digitalWrite(FAN_PIN, HIGH);
    areaCounter = 0;
    systemData.areaCount = 0;
  }
  if (now - lastEvaluation >= EVALUATION_WINDOW) {
    if (!systemData.loadState) areaCounter = 0;
    systemData.areaCount = 0;
    lastEvaluation = now;
  }
}

void controlFan() {
  unsigned long now = millis();

  if (manualFanOverride) {
    digitalWrite(FAN_PIN, manualFanState ? HIGH : LOW);
    systemData.loadState = manualFanState;
    return;
  }
  if (!operatingWindow_Fan()) {
    digitalWrite(FAN_PIN, LOW);
    systemData.loadState = false;
    areaCounter = 0;
    return;
  }
  if (systemData.loadState && (now - fanStartTime >= (unsigned long)FAN_ON_TIME)) {
    digitalWrite(FAN_PIN, LOW);
    systemData.loadState = false;
  }
}

// ============================================================================
//  SD logger
// ============================================================================
void updateFileNames() {
  char dateBuffer[20];
  char monthBuffer[10];
  sprintf(monthBuffer, "%04d-%02d", systemData.year, systemData.month);
  sprintf(dateBuffer,  "%04d-%02d-%02d",
          systemData.year, systemData.month, systemData.day);
  monthFolderBMP = folderBMP + "/" + String(monthBuffer);
  filenameBMP    = monthFolderBMP + "/" + String(dateBuffer) + ".csv";
}

void initLogFolders() {
  updateFileNames();
  if (!SD.exists(folderBMP))      SD.mkdir(folderBMP);
  if (!SD.exists(monthFolderBMP)) SD.mkdir(monthFolderBMP);
}

void logBMP() {
  if (systemData.day != lastLoggedDay) {
    updateFileNames();
    initLogFolders();
    lastLoggedDay = systemData.day;
  }

  char ts[24];
  sprintf(ts, "%04d-%02d-%02dT%02d:%02d:%02d",
          systemData.year, systemData.month, systemData.day,
          systemData.hour, systemData.minute, systemData.second);

  if (storageMode == STORAGE_LOCAL || storageMode == STORAGE_BOTH) {
    if (!SD.exists(filenameBMP)) {
      File f = SD.open(filenameBMP, FILE_WRITE);
      if (f) {
        f.println("Date,Time,Temp,Pres,Alt,LED,Fan");
        f.close();
      }
    }
    File f = SD.open(filenameBMP, FILE_APPEND);
    if (f) {
      f.printf("%02d/%02d/%04d,%02d:%02d,%.2f,%.2f,%.2f,%d,%d\n",
               systemData.day, systemData.month, systemData.year,
               systemData.hour, systemData.minute,
               systemData.temperature, systemData.pressure,
               systemData.altitude,
               systemData.ledState ? 1 : 0,
               systemData.loadState ? 1 : 0);
      f.close();
    }
  }

  if (storageMode == STORAGE_CLOUD || storageMode == STORAGE_BOTH) {
    JsonDocument d;
    d["device_id"]  = deviceId;
    d["ts"]         = ts;
    d["temp"]       = systemData.temperature;
    d["pres"]       = systemData.pressure;
    d["alt"]        = systemData.altitude;
    d["led"]        = systemData.ledState;
    d["fan"]        = systemData.loadState;
    d["ap_active"]  = operatingWindow_AP();
    d["led_active"] = operatingWindow_LED();
    d["fan_active"] = operatingWindow_Fan();
    d["sd_active"]  = operatingWindow_SD();

    bool ok = staConnected && sendToCloud(d);
    if (!ok) {
      char line[160];
      snprintf(line, sizeof(line),
               "%s,%.2f,%.2f,%.2f,%d,%d",
               ts,
               systemData.temperature, systemData.pressure,
               systemData.altitude,
               systemData.ledState ? 1 : 0,
               systemData.loadState ? 1 : 0);
      queuePending(String(line));
    }
  }
}

// ============================================================================
//  OLED
// ============================================================================
void showData() {
  display.clearDisplay();
  display.setCursor(0, 0);

  if (currentMode == MODE_BMP) {
    display.printf("Date: %02d/%02d/%04d\n",
                   systemData.day, systemData.month, systemData.year);
    display.printf("Time: %02d:%02d\n", systemData.hour, systemData.minute);
    display.println();
    display.printf("Temp: %.1f C\n", systemData.temperature);
    display.printf("Pres: %.1f kPa\n", systemData.pressure);
    display.printf("Alt : %.1f m\n", systemData.altitude);
    display.printf("LED : %s\n", systemData.ledState  ? "ON" : "OFF");
    display.printf("Fan : %s\n", systemData.loadState ? "ON" : "OFF");
  } else if (currentMode == MODE_OPERATING_WINDOWS) {
    display.println("Active Windows:");
    display.println("--------------------");
    display.printf("WiFi AP: %s\n",
                   (operatingWindow_AP() || forceAwake) ? "ACTIVE" : "INACTIVE");
    display.printf("LEDs   : %s\n", operatingWindow_LED() ? "ACTIVE" : "INACTIVE");
    display.printf("SD Log : %s\n", operatingWindow_SD()  ? "ACTIVE" : "INACTIVE");
    display.printf("Fan    : %s\n", operatingWindow_Fan() ? "ACTIVE" : "INACTIVE");
    display.println();
    if (staConnected) {
      display.printf("STA: %s\n", WiFi.localIP().toString().c_str());
    } else if (staEnabled) {
      display.println("STA: connecting...");
    } else {
      display.println("STA: disabled");
    }
  }
  display.display();
}
