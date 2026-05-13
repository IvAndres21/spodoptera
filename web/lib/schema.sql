-- ============================================================================
--  Spodoptera — Postgres schema (Neon)
--  Run manually via Neon SQL Editor or `psql $DATABASE_URL -f schema.sql`
-- ============================================================================

CREATE TABLE IF NOT EXISTS measurements (
    id          BIGSERIAL PRIMARY KEY,
    device_id   TEXT        NOT NULL,
    ts          TIMESTAMPTZ NOT NULL,
    temp        REAL        NOT NULL,
    pres        REAL        NOT NULL,
    alt         REAL        NOT NULL,
    led         BOOLEAN     NOT NULL DEFAULT FALSE,
    fan         BOOLEAN     NOT NULL DEFAULT FALSE,
    ap_active   BOOLEAN     NOT NULL DEFAULT FALSE,
    led_active  BOOLEAN     NOT NULL DEFAULT FALSE,
    fan_active  BOOLEAN     NOT NULL DEFAULT FALSE,
    sd_active   BOOLEAN     NOT NULL DEFAULT FALSE,
    created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS measurements_device_ts_idx
    ON measurements (device_id, ts DESC);

CREATE INDEX IF NOT EXISTS measurements_ts_idx
    ON measurements (ts DESC);

CREATE TABLE IF NOT EXISTS windows_config (
    device_id   TEXT        PRIMARY KEY,
    led_sh      SMALLINT    NOT NULL DEFAULT 19,
    led_sm      SMALLINT    NOT NULL DEFAULT 30,
    led_eh      SMALLINT    NOT NULL DEFAULT 4,
    led_em      SMALLINT    NOT NULL DEFAULT 0,
    fan_sh      SMALLINT    NOT NULL DEFAULT 19,
    fan_sm      SMALLINT    NOT NULL DEFAULT 30,
    fan_eh      SMALLINT    NOT NULL DEFAULT 4,
    fan_em      SMALLINT    NOT NULL DEFAULT 0,
    ap_sh       SMALLINT    NOT NULL DEFAULT 8,
    ap_sm       SMALLINT    NOT NULL DEFAULT 0,
    ap_eh       SMALLINT    NOT NULL DEFAULT 23,
    ap_em       SMALLINT    NOT NULL DEFAULT 50,
    sd_sh       SMALLINT    NOT NULL DEFAULT 18,
    sd_sm       SMALLINT    NOT NULL DEFAULT 30,
    sd_eh       SMALLINT    NOT NULL DEFAULT 23,
    sd_em       SMALLINT    NOT NULL DEFAULT 50,
    fan_dur     SMALLINT    NOT NULL DEFAULT 1,
    threshold   SMALLINT    NOT NULL DEFAULT 10,
    updated_at  TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Default config for the first device
INSERT INTO windows_config (device_id)
VALUES ('esp32-trap-001')
ON CONFLICT (device_id) DO NOTHING;
