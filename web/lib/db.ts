import { neon, NeonQueryFunction } from "@neondatabase/serverless";

let _client: NeonQueryFunction<false, false> | null = null;

function getClient(): NeonQueryFunction<false, false> {
  if (_client) return _client;
  const url = process.env.DATABASE_URL;
  if (!url) {
    throw new Error(
      "DATABASE_URL no definida. Configúrala en .env o en Vercel (Environment Variables)."
    );
  }
  _client = neon(url);
  return _client;
}

// Proxy que delega al cliente real solo cuando se invoca (lazy).
export const sql = ((
  ...args: Parameters<NeonQueryFunction<false, false>>
) => getClient()(...args)) as NeonQueryFunction<false, false>;

export type Measurement = {
  id: number;
  device_id: string;
  ts: string;
  temp: number;
  pres: number;
  alt: number;
  led: boolean;
  fan: boolean;
  ap_active: boolean;
  led_active: boolean;
  fan_active: boolean;
  sd_active: boolean;
  created_at: string;
};

export type WindowsConfig = {
  device_id: string;
  led_sh: number; led_sm: number; led_eh: number; led_em: number;
  fan_sh: number; fan_sm: number; fan_eh: number; fan_em: number;
  ap_sh: number;  ap_sm: number;  ap_eh: number;  ap_em: number;
  sd_sh: number;  sd_sm: number;  sd_eh: number;  sd_em: number;
  fan_dur: number;
  threshold: number;
  updated_at: string;
};
