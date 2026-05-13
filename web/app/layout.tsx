import type { Metadata } from "next";
import { Geist, Geist_Mono } from "next/font/google";
import "./globals.css";

const geistSans = Geist({
  variable: "--font-geist-sans",
  subsets: ["latin"],
});

const geistMono = Geist_Mono({
  variable: "--font-geist-mono",
  subsets: ["latin"],
});

export const metadata: Metadata = {
  title: "Spodoptera — Smart Light Trap",
  description:
    "Trampa de luz solar inteligente para monitoreo y control de Spodoptera frugiperda en cultivos de maíz. Plataforma IoT con MYOSA + ESP32.",
  keywords: [
    "Spodoptera frugiperda",
    "trampa de luz",
    "IoT",
    "MYOSA",
    "agricultura sostenible",
    "Unimagdalena",
  ],
  openGraph: {
    title: "Spodoptera — Smart Light Trap",
    description:
      "Monitoreo en tiempo real de una trampa de luz solar IoT para control de plagas agrícolas.",
    type: "website",
  },
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html
      lang="es"
      className={`${geistSans.variable} ${geistMono.variable} h-full antialiased`}
    >
      <body className="min-h-full flex flex-col bg-slate-950 text-slate-100">
        {children}
      </body>
    </html>
  );
}
