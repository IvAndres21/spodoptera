import { readFileSync } from "node:fs";
import { neon } from "@neondatabase/serverless";

// Load .env.local manually (Node does not load it automatically).
const envFile = readFileSync(".env.local", "utf8");
const env = Object.fromEntries(
  envFile
    .split("\n")
    .filter((line) => line && !line.startsWith("#"))
    .map((line) => {
      const i = line.indexOf("=");
      return [line.slice(0, i), line.slice(i + 1).replace(/^"|"$/g, "")];
    })
);

if (!env.DATABASE_URL) {
  console.error("Missing DATABASE_URL in .env.local");
  process.exit(1);
}

const sql = neon(env.DATABASE_URL);
const schema = readFileSync("lib/schema.sql", "utf8");

// Strip SQL comments and split on `;` followed by newline (preserves multi-line statements).
const cleaned = schema
  .split("\n")
  .filter((line) => !line.trim().startsWith("--"))
  .join("\n");

const statements = cleaned
  .split(/;\s*$/m)
  .map((s) => s.trim())
  .filter((s) => s.length > 0);

let ok = 0;
let failed = 0;
for (const stmt of statements) {
  const preview = stmt.split("\n")[0].slice(0, 80);
  try {
    await sql.query(stmt);
    ok++;
    console.log(`OK:   ${preview}`);
  } catch (err) {
    failed++;
    console.error(`FAIL: ${preview}\n      ${err.message}`);
  }
}

console.log(`\nDone. OK=${ok} Failed=${failed}`);
process.exit(failed > 0 ? 1 : 0);
