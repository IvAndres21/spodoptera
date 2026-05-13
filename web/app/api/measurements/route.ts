import { NextRequest, NextResponse } from "next/server";
import { sql } from "@/lib/db";

export const runtime = "edge";

function authorize(req: NextRequest): boolean {
  const expected = process.env.DEVICE_KEY;
  if (!expected) return true; // sin clave configurada → modo abierto (no recomendado)
  const got = req.headers.get("x-device-key");
  return got === expected;
}

export async function POST(req: NextRequest) {
  if (!authorize(req)) {
    return NextResponse.json({ error: "Unauthorized" }, { status: 401 });
  }

  let body: Record<string, unknown>;
  try {
    body = await req.json();
  } catch {
    return NextResponse.json({ error: "Invalid JSON" }, { status: 400 });
  }

  const deviceId =
    (body.device_id as string) ||
    req.headers.get("x-device-id") ||
    "unknown";

  const ts = body.ts as string | undefined;
  if (!ts) {
    return NextResponse.json({ error: "Missing ts" }, { status: 400 });
  }

  const temp = Number(body.temp ?? 0);
  const pres = Number(body.pres ?? 0);
  const alt = Number(body.alt ?? 0);
  const led = Boolean(body.led);
  const fan = Boolean(body.fan);
  const apActive = Boolean(body.ap_active);
  const ledActive = Boolean(body.led_active);
  const fanActive = Boolean(body.fan_active);
  const sdActive = Boolean(body.sd_active);

  try {
    await sql`
      INSERT INTO measurements
        (device_id, ts, temp, pres, alt, led, fan,
         ap_active, led_active, fan_active, sd_active)
      VALUES
        (${deviceId}, ${ts}, ${temp}, ${pres}, ${alt}, ${led}, ${fan},
         ${apActive}, ${ledActive}, ${fanActive}, ${sdActive})
    `;
    return NextResponse.json({ ok: true });
  } catch (err) {
    console.error("DB insert error:", err);
    return NextResponse.json({ error: "DB error" }, { status: 500 });
  }
}

export async function GET(req: NextRequest) {
  const { searchParams } = new URL(req.url);
  const deviceId = searchParams.get("device_id") ?? "esp32-trap-001";
  const limit = Math.min(parseInt(searchParams.get("limit") ?? "100"), 1000);

  try {
    const rows = await sql`
      SELECT id, device_id, ts, temp, pres, alt, led, fan,
             ap_active, led_active, fan_active, sd_active, created_at
      FROM measurements
      WHERE device_id = ${deviceId}
      ORDER BY ts DESC
      LIMIT ${limit}
    `;
    return NextResponse.json({ data: rows });
  } catch (err) {
    console.error("DB select error:", err);
    return NextResponse.json({ error: "DB error" }, { status: 500 });
  }
}
