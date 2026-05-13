"use client";

import { useEffect, useState } from "react";
import {
  Thermometer,
  Gauge,
  Mountain,
  Lightbulb,
  Wind,
  Wifi,
  HardDrive,
  CircleDot,
} from "lucide-react";
import SensorCard from "./SensorCard";
import StatusPill from "./StatusPill";
import HistoryChart from "./HistoryChart";

type Measurement = {
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
};

export default function DashboardLive({
  deviceId = "esp32-trap-001",
}: {
  deviceId?: string;
}) {
  const [latest, setLatest] = useState<Measurement | null>(null);
  const [history, setHistory] = useState<Measurement[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    let mounted = true;

    async function fetchData() {
      try {
        const [latestRes, historyRes] = await Promise.all([
          fetch(`/api/latest?device_id=${encodeURIComponent(deviceId)}`, {
            cache: "no-store",
          }),
          fetch(
            `/api/measurements?device_id=${encodeURIComponent(deviceId)}&limit=100`,
            { cache: "no-store" }
          ),
        ]);
        const latestJson = await latestRes.json();
        const historyJson = await historyRes.json();
        if (!mounted) return;
        setLatest(latestJson.data);
        setHistory((historyJson.data ?? []).reverse());
        setError(null);
      } catch (e) {
        console.error(e);
        if (mounted) setError("Could not load the data");
      } finally {
        if (mounted) setLoading(false);
      }
    }

    fetchData();
    const interval = setInterval(fetchData, 10000);

    return () => {
      mounted = false;
      clearInterval(interval);
    };
  }, [deviceId]);

  const lastUpdate = latest
    ? new Date(latest.ts).toLocaleString("en-US", {
        dateStyle: "medium",
        timeStyle: "medium",
      })
    : "—";

  return (
    <section id="dashboard" className="mx-auto max-w-6xl px-6 py-16">
      <div className="flex flex-wrap items-end justify-between gap-4 mb-8">
        <div>
          <h2 className="text-2xl font-bold text-white sm:text-3xl">
            Real-time data
          </h2>
          <p className="mt-1 text-sm text-slate-400">
            Last reading:{" "}
            <span className="font-mono text-emerald-300">{lastUpdate}</span>
            {" · "}
            Device:{" "}
            <span className="font-mono text-indigo-300">{deviceId}</span>
          </p>
        </div>
        <div className="flex items-center gap-2 text-xs text-slate-400">
          <span className="relative flex h-2 w-2">
            <span className="absolute inline-flex h-full w-full animate-ping rounded-full bg-emerald-400 opacity-75" />
            <span className="relative inline-flex h-2 w-2 rounded-full bg-emerald-400" />
          </span>
          Refreshing every 10 s
        </div>
      </div>

      {error && (
        <div className="mb-6 rounded-xl border border-rose-500/30 bg-rose-500/10 px-4 py-3 text-sm text-rose-200">
          {error}
        </div>
      )}

      {loading && !latest && (
        <div className="rounded-2xl border border-white/10 bg-white/5 p-8 text-center text-slate-400">
          Loading...
        </div>
      )}

      {!loading && !latest && (
        <div className="rounded-2xl border border-white/10 bg-white/5 p-8 text-center text-slate-400">
          No data yet. Once the trap publishes its first measurement it will
          appear here.
        </div>
      )}

      {latest && (
        <>
          {/* Sensors */}
          <div className="grid gap-4 sm:grid-cols-3">
            <SensorCard
              icon={<Thermometer className="h-5 w-5" />}
              label="Temperature"
              value={latest.temp.toFixed(2)}
              unit="°C"
              accent="amber"
            />
            <SensorCard
              icon={<Gauge className="h-5 w-5" />}
              label="Pressure"
              value={latest.pres.toFixed(2)}
              unit="hPa"
              accent="indigo"
            />
            <SensorCard
              icon={<Mountain className="h-5 w-5" />}
              label="Altitude"
              value={latest.alt.toFixed(2)}
              unit="m"
              accent="emerald"
            />
          </div>

          {/* Actuators */}
          <div className="mt-6 grid gap-4 sm:grid-cols-2">
            <ActuatorCard
              icon={<Lightbulb className="h-5 w-5" />}
              title="Attraction Light"
              subtitle="UV-A LED"
              active={latest.led}
            />
            <ActuatorCard
              icon={<Wind className="h-5 w-5" />}
              title="Fan"
              subtitle="Capture system"
              active={latest.fan}
            />
          </div>

          {/* Window status */}
          <div className="mt-12">
            <h3 className="text-lg font-semibold text-white">
              Operating windows status
            </h3>
            <p className="mt-1 text-sm text-slate-400">
              Whether each subsystem is currently inside its active time window.
            </p>
            <div className="mt-4 grid gap-3 sm:grid-cols-2">
              <StatusPill
                label="Access Point (AP)"
                active={latest.ap_active}
                icon={<Wifi className="h-4 w-4" />}
              />
              <StatusPill
                label="Attractor LED"
                active={latest.led_active}
                icon={<Lightbulb className="h-4 w-4" />}
              />
              <StatusPill
                label="Fan"
                active={latest.fan_active}
                icon={<Wind className="h-4 w-4" />}
              />
              <StatusPill
                label="SD Logging"
                active={latest.sd_active}
                icon={<HardDrive className="h-4 w-4" />}
              />
            </div>
          </div>

          {/* History */}
          {history.length > 1 && (
            <div className="mt-12">
              <h3 className="text-lg font-semibold text-white">
                Recent history
              </h3>
              <p className="mt-1 text-sm text-slate-400">
                Last {history.length} measurements.
              </p>
              <HistoryChart data={history} />
            </div>
          )}
        </>
      )}
    </section>
  );
}

function ActuatorCard({
  icon,
  title,
  subtitle,
  active,
}: {
  icon: React.ReactNode;
  title: string;
  subtitle: string;
  active: boolean;
}) {
  return (
    <div className="flex items-center justify-between rounded-2xl border border-white/10 bg-gradient-to-br from-white/[0.06] to-transparent p-5 backdrop-blur-sm">
      <div className="flex items-center gap-4">
        <div
          className={
            "rounded-xl p-3 " +
            (active
              ? "bg-emerald-500/20 text-emerald-300"
              : "bg-white/5 text-slate-400")
          }
        >
          {icon}
        </div>
        <div>
          <div className="font-semibold text-white">{title}</div>
          <div className="text-xs text-slate-400">{subtitle}</div>
        </div>
      </div>
      <span
        className={
          "inline-flex items-center gap-2 rounded-full border px-4 py-1.5 text-xs font-bold tracking-wider " +
          (active
            ? "border-emerald-400/30 bg-emerald-500/15 text-emerald-300"
            : "border-white/15 bg-white/5 text-slate-400")
        }
      >
        <CircleDot className="h-3 w-3" />
        {active ? "ON" : "OFF"}
      </span>
    </div>
  );
}
