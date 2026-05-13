# Spodoptera

IoT solar-powered light trap for monitoring and capturing the fall armyworm (*Spodoptera frugiperda*), a major pest in maize crops.

Project developed for the **MYOSA Contest 2026** by Engineering students at **Universidad del Magdalena**.

## Repository layout

```
spodoptera/
├── firmware/   # ESP32 (Arduino IDE) — MYOSA Motherboard
│   ├── MYOSA_TranslateCode.ino
│   ├── dashboard.html       # Local UI served from the ESP32 SD card
│   └── style.css
└── web/        # Public dashboard — Next.js + Tailwind + Neon Postgres
    └── ...
```

## Features

- **Dual Wi-Fi**: the ESP32 exposes a local AP (`MYOSA_BoardServer`) and at the same time joins the user's home Wi-Fi as a client.
- **Configurable storage modes**: `LOCAL` (SD), `CLOUD` (Vercel) or `BOTH`, selectable from the UI.
- **Pending buffer**: if the ESP32 is in CLOUD/BOTH mode and Wi-Fi drops, data is staged on SD and synced when the link comes back.
- **Programmable operating windows**: AP / LED / Fan / SD logging run on independent schedules.
- **Deep sleep** between windows, with the cloud buffer flushed on wake-up.
- **Public dashboard** with real-time charts at https://spodoptera.vercel.app.

## Team

- Rafael Junior Acosta Vargas
- María de los Ángeles Delgado Villalobos
- Ivan Andres Robles Rodriguez

**Mentor**: Carlos Arturo Robles Algarín
