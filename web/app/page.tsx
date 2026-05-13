import Hero from "./components/Hero";
import DashboardLive from "./components/DashboardLive";
import WindowsConfigCard from "./components/WindowsConfigCard";

export const dynamic = "force-dynamic";

export default function Home() {
  return (
    <main>
      <Hero />

      <DashboardLive deviceId="esp32-trap-001" />

      <section className="mx-auto max-w-6xl px-6 pb-20">
        <h2 className="text-2xl font-bold text-white sm:text-3xl">
          Configured operating windows
        </h2>
        <p className="mt-1 text-sm text-slate-400 mb-6">
          Schedules programmed on the trap for each subsystem.
        </p>
        <WindowsConfigCard deviceId="esp32-trap-001" />
      </section>

      <footer className="border-t border-white/10 py-10">
        <div className="mx-auto max-w-6xl px-6 text-sm text-slate-400">
          <div className="flex flex-wrap items-center justify-between gap-4">
            <div>
              <div className="font-semibold text-white">Spodoptera Project</div>
              <div className="mt-1 text-xs">
                Universidad del Magdalena — MYOSA Contest 2026
              </div>
            </div>
            <div className="flex flex-wrap gap-x-6 gap-y-2 text-xs">
              <a
                href="https://blog.myosa-sensors.org/"
                target="_blank"
                rel="noopener noreferrer"
                className="hover:text-emerald-300"
              >
                MYOSA Blog
              </a>
              <a
                href="https://www.unimagdalena.edu.co/"
                target="_blank"
                rel="noopener noreferrer"
                className="hover:text-emerald-300"
              >
                Unimagdalena
              </a>
            </div>
          </div>
        </div>
      </footer>
    </main>
  );
}
