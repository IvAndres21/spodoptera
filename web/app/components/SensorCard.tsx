type Props = {
  icon: React.ReactNode;
  label: string;
  value: string | number;
  unit: string;
  accent?: "emerald" | "indigo" | "violet" | "amber";
};

const accentClasses: Record<NonNullable<Props["accent"]>, string> = {
  emerald: "from-emerald-500/20 to-emerald-500/5 border-emerald-400/30",
  indigo: "from-indigo-500/20 to-indigo-500/5 border-indigo-400/30",
  violet: "from-violet-500/20 to-violet-500/5 border-violet-400/30",
  amber: "from-amber-500/20 to-amber-500/5 border-amber-400/30",
};

export default function SensorCard({
  icon,
  label,
  value,
  unit,
  accent = "indigo",
}: Props) {
  return (
    <div
      className={`relative overflow-hidden rounded-2xl border bg-gradient-to-br p-6 backdrop-blur-sm ${accentClasses[accent]}`}
    >
      <div className="flex items-center justify-between">
        <span className="text-xs font-semibold uppercase tracking-wider text-slate-300">
          {label}
        </span>
        <div className="text-slate-100/80">{icon}</div>
      </div>
      <div className="mt-4 flex items-baseline gap-2">
        <span className="text-4xl font-bold tabular-nums text-white">
          {value}
        </span>
        <span className="text-sm font-medium text-slate-400">{unit}</span>
      </div>
    </div>
  );
}
