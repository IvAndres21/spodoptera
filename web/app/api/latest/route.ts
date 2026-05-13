import { NextRequest, NextResponse } from "next/server";
import { sql } from "@/lib/db";

export const runtime = "edge";

export async function GET(req: NextRequest) {
  const { searchParams } = new URL(req.url);
  const deviceId = searchParams.get("device_id") ?? "esp32-trap-001";

  try {
    const rows = await sql`
      SELECT id, device_id, ts, temp, pres, alt, led, fan,
             ap_active, led_active, fan_active, sd_active, created_at
      FROM measurements
      WHERE device_id = ${deviceId}
      ORDER BY ts DESC
      LIMIT 1
    `;
    return NextResponse.json({ data: rows[0] ?? null });
  } catch (err) {
    console.error("DB select error:", err);
    return NextResponse.json({ error: "DB error" }, { status: 500 });
  }
}
