import { Lightbulb, Wifi, Wind, HardDrive } from "lucide-react";
import { sql, type WindowsConfig } from "@/lib/db";

function pad(n: number) {
  return String(n).padStart(2, "0");
}

async function getConfig(deviceId: string): Promise<WindowsConfig | null> {
  try {
    const rows = (await sql`
      SELECT * FROM windows_config WHERE device_id = ${deviceId} LIMIT 1
    `) as WindowsConfig[];
    return rows[0] ?? null;
  } catch (err) {
    console.error("getConfig error", err);
    return null;
  }
}

const windows = [
  { key: "led", title: "LED Atractor", icon: Lightbulb, color: "text-amber-300" },
  { key: "ap",  title: "Punto de Acceso", icon: Wifi, color: "text-indigo-300" },
  { key: "fan", title: "Ventilador", icon: Wind, color: "text-emerald-300" },
  { key: "sd",  title: "Almacenamiento SD", icon: HardDrive, color: "text-violet-300" },
] as const;

export default async function WindowsConfigCard({
  deviceId = "esp32-trap-001",
}: {
  deviceId?: string;
}) {
  const cfg = await getConfig(deviceId);

  if (!cfg) {
    return (
      <div className="rounded-2xl border border-white/10 bg-white/5 p-6 text-sm text-slate-400">
        Sin configuración registrada para <code className="text-emerald-300">{deviceId}</code>.
      </div>
    );
  }

  return (
    <div className="grid gap-4 sm:grid-cols-2">
      {windows.map(({ key, title, icon: Icon, color }) => {
        const sh = cfg[`${key}_sh` as keyof WindowsConfig] as number;
        const sm = cfg[`${key}_sm` as keyof WindowsConfig] as number;
        const eh = cfg[`${key}_eh` as keyof WindowsConfig] as number;
        const em = cfg[`${key}_em` as keyof WindowsConfig] as number;
        return (
          <div
            key={key}
            className="rounded-2xl border border-white/10 bg-gradient-to-br from-white/[0.06] to-transparent p-5 backdrop-blur-sm"
          >
            <div className="flex items-center gap-3">
              <Icon className={`h-5 w-5 ${color}`} />
              <span className="font-semibold text-slate-100">{title}</span>
            </div>
            <div className="mt-4 flex items-center gap-3">
              <div className="rounded-lg border border-white/10 bg-slate-900/50 px-3 py-2 font-mono text-lg text-white tabular-nums">
                {pad(sh)}:{pad(sm)}
              </div>
              <span className="text-slate-500">→</span>
              <div className="rounded-lg border border-white/10 bg-slate-900/50 px-3 py-2 font-mono text-lg text-white tabular-nums">
                {pad(eh)}:{pad(em)}
              </div>
            </div>
            <div className="mt-2 text-xs text-slate-500">
              Inicio → Fin
            </div>
          </div>
        );
      })}

      <div className="rounded-2xl border border-white/10 bg-gradient-to-br from-white/[0.04] to-transparent p-5 sm:col-span-2">
        <div className="grid grid-cols-2 gap-4">
          <div>
            <div className="text-xs uppercase tracking-wider text-slate-400">
              Tiempo de ventilador
            </div>
            <div className="mt-1 text-2xl font-semibold text-white">
              {cfg.fan_dur} <span className="text-base font-normal text-slate-400">min</span>
            </div>
          </div>
          <div>
            <div className="text-xs uppercase tracking-wider text-slate-400">
              Umbral de detección
            </div>
            <div className="mt-1 text-2xl font-semibold text-white">
              {cfg.threshold} <span className="text-base font-normal text-slate-400">det.</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
