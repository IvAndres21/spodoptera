import { Sparkles } from "lucide-react";

export default function Hero() {
  return (
    <section className="relative overflow-hidden border-b border-white/10">
      <div className="absolute inset-0 -z-10 bg-[radial-gradient(ellipse_at_top,_rgba(99,102,241,0.25),_transparent_50%)]" />
      <div className="absolute inset-0 -z-10 bg-[radial-gradient(ellipse_at_bottom_right,_rgba(34,197,94,0.18),_transparent_50%)]" />

      <div className="mx-auto max-w-6xl px-6 pt-24 pb-20 sm:pt-32 sm:pb-28">
        <div className="inline-flex items-center gap-2 rounded-full border border-white/15 bg-white/5 px-3 py-1 text-xs font-medium text-emerald-300 backdrop-blur">
          <Sparkles className="h-3.5 w-3.5" />
          MYOSA Contest · Universidad del Magdalena
        </div>

        <h1 className="mt-6 text-4xl font-bold tracking-tight text-white sm:text-6xl">
          <span className="bg-gradient-to-br from-emerald-300 via-indigo-300 to-violet-400 bg-clip-text text-transparent">
            Spodoptera
          </span>
          <span className="block text-2xl font-medium text-slate-300 sm:text-3xl mt-2">
            Trampa de luz solar IoT para control de plagas
          </span>
        </h1>

        <p className="mt-6 max-w-2xl text-base text-slate-300 sm:text-lg leading-relaxed">
          Una trampa inteligente que combina luz UV-A, sensores ambientales y energía solar
          para monitorear y capturar el gusano cogollero (<em>Spodoptera frugiperda</em>),
          una plaga clave del maíz. Los datos se sincronizan en tiempo real desde el campo
          al dashboard que estás viendo.
        </p>

        <div className="mt-8 flex flex-wrap items-center gap-3 text-sm text-slate-300">
          <a
            href="#dashboard"
            className="inline-flex items-center gap-2 rounded-full bg-emerald-500 px-5 py-2.5 font-semibold text-emerald-950 transition hover:bg-emerald-400"
          >
            Ver dashboard
          </a>
          <a
            href="https://blog.myosa-sensors.org/"
            target="_blank"
            rel="noopener noreferrer"
            className="inline-flex items-center gap-2 rounded-full border border-white/20 bg-white/5 px-5 py-2.5 font-medium text-slate-100 backdrop-blur transition hover:bg-white/10"
          >
            MYOSA Blog
          </a>
        </div>
      </div>
    </section>
  );
}
