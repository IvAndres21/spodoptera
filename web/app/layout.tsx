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
    "Smart solar-powered light trap for monitoring and controlling Spodoptera frugiperda in maize crops. IoT platform powered by MYOSA + ESP32.",
  keywords: [
    "Spodoptera frugiperda",
    "light trap",
    "IoT",
    "MYOSA",
    "sustainable agriculture",
    "Unimagdalena",
  ],
  openGraph: {
    title: "Spodoptera — Smart Light Trap",
    description:
      "Real-time monitoring of an IoT solar-powered light trap for agricultural pest control.",
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
      lang="en"
      className={`${geistSans.variable} ${geistMono.variable} h-full antialiased`}
    >
      <body className="min-h-full flex flex-col bg-slate-950 text-slate-100">
        {children}
      </body>
    </html>
  );
}
