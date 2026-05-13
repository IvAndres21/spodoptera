# Spodoptera

Trampa de luz solar IoT para monitoreo y captura del gusano cogollero (*Spodoptera frugiperda*), una plaga clave del maíz.

Proyecto desarrollado para el **MYOSA Contest 2026** por estudiantes de Ingeniería de la **Universidad del Magdalena**.

## Repositorio

```
spodoptera/
├── firmware/   # ESP32 (Arduino IDE) — MYOSA Motherboard
│   ├── MYOSA_TranslateCode.ino
│   ├── dashboard.html       # UI local servida desde la SD del ESP32
│   └── style.css
└── web/        # Dashboard público — Next.js + Tailwind + Neon Postgres
    └── ...
```

## Características

- **Doble Wi-Fi**: el ESP32 expone un AP local (`MYOSA_BoardServer`) y simultáneamente se conecta como cliente a la red Wi-Fi del usuario.
- **Modos de almacenamiento configurables desde la UI**: `LOCAL` (SD), `CLOUD` (Vercel) o `BOTH`.
- **Buffer pendiente**: si el ESP32 está en modo CLOUD/BOTH y se pierde Wi-Fi, los datos se guardan en SD y se sincronizan al reconectar.
- **Ventanas de operación programables**: AP / LED / Ventilador / Logging SD se activan en horarios independientes.
- **Deep sleep** entre ventanas, con buffer de cloud que se vacía al despertar.
- **Dashboard público**: gráficas en tiempo real en https://spodoptera.vercel.app.

## Equipo

- Rafael Junior Acosta Vargas
- María de los Ángeles Delgado Villalobos
- Ivan Andres Robles Rodriguez

**Mentor**: Carlos Arturo Robles Algarín
