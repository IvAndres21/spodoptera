type Props = {
  label: string;
  active: boolean;
  icon?: React.ReactNode;
  activeText?: string;
  inactiveText?: string;
};

export default function StatusPill({
  label,
  active,
  icon,
  activeText = "ACTIVO",
  inactiveText = "INACTIVO",
}: Props) {
  return (
    <div className="flex items-center justify-between rounded-xl border border-white/10 bg-white/5 px-4 py-3">
      <div className="flex items-center gap-3 text-sm font-medium text-slate-200">
        {icon && <span className="text-slate-300">{icon}</span>}
        <span>{label}</span>
      </div>
      <span
        className={
          "inline-flex items-center gap-2 rounded-full border px-3 py-1 text-xs font-bold tracking-wider " +
          (active
            ? "border-emerald-400/30 bg-emerald-500/15 text-emerald-300"
            : "border-white/15 bg-white/5 text-slate-400")
        }
      >
        <span
          className={
            "h-1.5 w-1.5 rounded-full " +
            (active ? "bg-emerald-400 shadow-[0_0_8px_rgba(52,211,153,0.6)]" : "bg-slate-500")
          }
        />
        {active ? activeText : inactiveText}
      </span>
    </div>
  );
}
