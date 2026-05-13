"use client";

import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  ResponsiveContainer,
  CartesianGrid,
  Legend,
} from "recharts";

type Measurement = {
  ts: string;
  temp: number;
  pres: number;
  alt: number;
};

function formatTime(ts: string) {
  const d = new Date(ts);
  return d.toLocaleTimeString("en-US", {
    hour: "2-digit",
    minute: "2-digit",
  });
}

export default function HistoryChart({ data }: { data: Measurement[] }) {
  const chartData = data.map((m) => ({
    time: formatTime(m.ts),
    temp: m.temp,
    pres: m.pres,
    alt: m.alt,
  }));

  return (
    <div className="mt-4 rounded-2xl border border-white/10 bg-white/5 p-4 sm:p-6">
      <div className="h-80 w-full">
        <ResponsiveContainer width="100%" height="100%">
          <LineChart data={chartData} margin={{ top: 10, right: 12, bottom: 0, left: -10 }}>
            <CartesianGrid stroke="rgba(255,255,255,0.08)" strokeDasharray="4 4" />
            <XAxis
              dataKey="time"
              stroke="rgba(255,255,255,0.45)"
              tick={{ fontSize: 11 }}
              interval="preserveStartEnd"
              minTickGap={32}
            />
            <YAxis
              yAxisId="left"
              stroke="rgba(255,255,255,0.45)"
              tick={{ fontSize: 11 }}
            />
            <YAxis
              yAxisId="right"
              orientation="right"
              stroke="rgba(255,255,255,0.45)"
              tick={{ fontSize: 11 }}
            />
            <Tooltip
              contentStyle={{
                background: "rgba(15, 23, 42, 0.95)",
                border: "1px solid rgba(255,255,255,0.12)",
                borderRadius: 12,
                color: "white",
                fontSize: 12,
              }}
              labelStyle={{ color: "rgb(148, 163, 184)" }}
            />
            <Legend wrapperStyle={{ fontSize: 12, color: "rgb(203, 213, 225)" }} />
            <Line
              yAxisId="left"
              type="monotone"
              dataKey="temp"
              name="Temperature (°C)"
              stroke="#fbbf24"
              strokeWidth={2}
              dot={false}
            />
            <Line
              yAxisId="right"
              type="monotone"
              dataKey="pres"
              name="Pressure (hPa)"
              stroke="#818cf8"
              strokeWidth={2}
              dot={false}
            />
            <Line
              yAxisId="left"
              type="monotone"
              dataKey="alt"
              name="Altitude (m)"
              stroke="#34d399"
              strokeWidth={2}
              dot={false}
            />
          </LineChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
}
