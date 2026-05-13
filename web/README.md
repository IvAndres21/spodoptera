# Spodoptera — Smart Light Trap Web

Public dashboard for the IoT solar-powered light trap (MYOSA + ESP32) that monitors and controls *Spodoptera frugiperda*.

## Stack

- Next.js 16 (App Router) + React 19
- Tailwind CSS 4
- TypeScript
- Neon Postgres (serverless)
- Recharts + lucide-react

## Local development

```bash
npm install
cp .env.example .env
# Edit .env with your DATABASE_URL from Neon and a DEVICE_KEY
npm run dev
```

## Database setup

1. Create a project at https://console.neon.tech/.
2. Copy the `DATABASE_URL` into `.env` (local) and into Vercel (production).
3. Run the schema:

```bash
psql "$DATABASE_URL" -f lib/schema.sql
```

Or open the Neon **SQL Editor** and paste the contents of `lib/schema.sql`.

## Deploy on Vercel

```bash
vercel link
vercel env add DATABASE_URL production
vercel env add DEVICE_KEY production
vercel --prod
```

The final domain will be `https://spodoptera.vercel.app`.

## API

| Endpoint                    | Method | Auth           | Description                          |
|-----------------------------|--------|----------------|--------------------------------------|
| `/api/measurements`         | POST   | `X-Device-Key` | The ESP32 publishes a measurement    |
| `/api/measurements?limit=N` | GET    | —              | History for charts                   |
| `/api/latest`               | GET    | —              | Latest measurement                   |
| `/api/config`               | GET    | —              | Configured windows                   |
| `/api/config`               | POST   | `X-Device-Key` | Update windows (sync with ESP32)     |

### Payload expected by POST /api/measurements

```json
{
  "device_id": "esp32-trap-001",
  "ts": "2026-05-13T22:15:00",
  "temp": 27.4,
  "pres": 1010.2,
  "alt": 23.5,
  "led": true,
  "fan": false,
  "ap_active": true,
  "led_active": true,
  "fan_active": false,
  "sd_active": true
}
```
