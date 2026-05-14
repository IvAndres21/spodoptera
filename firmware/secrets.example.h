#pragma once
// ----------------------------------------------------------------------------
//  Secrets template.
//  Copy this file to secrets.h (same folder) and fill in your real values.
//  secrets.h is gitignored so the device key never reaches the repository.
// ----------------------------------------------------------------------------

// Shared key with the backend (sent as the X-Device-Key header).
#define DEFAULT_DEVICE_KEY "your-device-key-here"

// Identifier this trap reports to the cloud.
#define DEFAULT_DEVICE_ID  "esp32-trap-001"
