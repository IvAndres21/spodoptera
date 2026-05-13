# Spodoptera — Smart Light Trap Web

Dashboard público para la trampa de luz solar IoT (MYOSA + ESP32) que monitorea y controla *Spodoptera frugiperda*.

## Stack

- Next.js 16 (App Router) + React 19
- Tailwind CSS 4
- TypeScript
- Neon Postgres (serverless)
- Recharts + lucide-react

## Desarrollo local

```bash
npm install
cp .env.example .env
# Edita .env con tu DATABASE_URL de Neon y un DEVICE_KEY
npm run dev
```

## Setup de la base de datos

1. Crea un proyecto en https://console.neon.tech/.
2. Copia el `DATABASE_URL` y guárdalo en `.env` (local) y en Vercel (producción).
3. Ejecuta el schema:

```bash
psql "$DATABASE_URL" -f lib/schema.sql
```

O abre el **SQL Editor** de Neon y pega el contenido de `lib/schema.sql`.

## Deploy en Vercel

```bash
vercel link
vercel env add DATABASE_URL production
vercel env add DEVICE_KEY production
vercel --prod
```

El dominio final será `https://spodoptera.vercel.app`.

## API

| Endpoint                    | Método | Auth        | Descripción                              |
|-----------------------------|--------|-------------|------------------------------------------|
| `/api/measurements`         | POST   | `X-Device-Key` | El ESP32 publica una medición           |
| `/api/measurements?limit=N` | GET    | —           | Histórico para gráficas                  |
| `/api/latest`               | GET    | —           | Última medición                          |
| `/api/config`               | GET    | —           | Ventanas configuradas                    |
| `/api/config`               | POST   | `X-Device-Key` | Actualiza ventanas (sync con ESP32)     |

### Payload esperado en POST /api/measurements

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
