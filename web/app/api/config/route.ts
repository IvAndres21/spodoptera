import { NextRequest, NextResponse } from "next/server";
import { sql } from "@/lib/db";

export const runtime = "edge";

export async function GET(req: NextRequest) {
  const { searchParams } = new URL(req.url);
  const deviceId = searchParams.get("device_id") ?? "esp32-trap-001";

  try {
    const rows = await sql`
      SELECT * FROM windows_config WHERE device_id = ${deviceId} LIMIT 1
    `;
    return NextResponse.json({ data: rows[0] ?? null });
  } catch (err) {
    console.error("DB select error:", err);
    return NextResponse.json({ error: "DB error" }, { status: 500 });
  }
}

export async function POST(req: NextRequest) {
  const expected = process.env.DEVICE_KEY;
  if (expected && req.headers.get("x-device-key") !== expected) {
    return NextResponse.json({ error: "Unauthorized" }, { status: 401 });
  }

  let body: Record<string, number | string>;
  try {
    body = await req.json();
  } catch {
    return NextResponse.json({ error: "Invalid JSON" }, { status: 400 });
  }

  const deviceId = (body.device_id as string) || "esp32-trap-001";

  try {
    await sql`
      INSERT INTO windows_config (
        device_id,
        led_sh, led_sm, led_eh, led_em,
        fan_sh, fan_sm, fan_eh, fan_em,
        ap_sh,  ap_sm,  ap_eh,  ap_em,
        sd_sh,  sd_sm,  sd_eh,  sd_em,
        fan_dur, threshold,
        updated_at
      ) VALUES (
        ${deviceId},
        ${Number(body.led_sh ?? 0)}, ${Number(body.led_sm ?? 0)},
        ${Number(body.led_eh ?? 0)}, ${Number(body.led_em ?? 0)},
        ${Number(body.fan_sh ?? 0)}, ${Number(body.fan_sm ?? 0)},
        ${Number(body.fan_eh ?? 0)}, ${Number(body.fan_em ?? 0)},
        ${Number(body.ap_sh  ?? 0)}, ${Number(body.ap_sm  ?? 0)},
        ${Number(body.ap_eh  ?? 0)}, ${Number(body.ap_em  ?? 0)},
        ${Number(body.sd_sh  ?? 0)}, ${Number(body.sd_sm  ?? 0)},
        ${Number(body.sd_eh  ?? 0)}, ${Number(body.sd_em  ?? 0)},
        ${Number(body.fan_dur ?? 1)}, ${Number(body.threshold ?? 10)},
        NOW()
      )
      ON CONFLICT (device_id) DO UPDATE SET
        led_sh = EXCLUDED.led_sh, led_sm = EXCLUDED.led_sm,
        led_eh = EXCLUDED.led_eh, led_em = EXCLUDED.led_em,
        fan_sh = EXCLUDED.fan_sh, fan_sm = EXCLUDED.fan_sm,
        fan_eh = EXCLUDED.fan_eh, fan_em = EXCLUDED.fan_em,
        ap_sh  = EXCLUDED.ap_sh,  ap_sm  = EXCLUDED.ap_sm,
        ap_eh  = EXCLUDED.ap_eh,  ap_em  = EXCLUDED.ap_em,
        sd_sh  = EXCLUDED.sd_sh,  sd_sm  = EXCLUDED.sd_sm,
        sd_eh  = EXCLUDED.sd_eh,  sd_em  = EXCLUDED.sd_em,
        fan_dur   = EXCLUDED.fan_dur,
        threshold = EXCLUDED.threshold,
        updated_at = NOW()
    `;
    return NextResponse.json({ ok: true });
  } catch (err) {
    console.error("DB upsert error:", err);
    return NextResponse.json({ error: "DB error" }, { status: 500 });
  }
}
