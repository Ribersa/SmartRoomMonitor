/**
 * Render prototype screens to PNG at exactly 390x844 CSS px @2x (780x1688).
 * Uses Chrome DevTools Protocol so the layout viewport matches 390px,
 * which --window-size does not guarantee when the OS has display scaling.
 */
const { spawn } = require("node:child_process");
const fs = require("node:fs");
const path = require("node:path");

const ROOT = path.resolve(__dirname, "..");
const PROTO = path.join(ROOT, "prototype", "index.html");
const OUT = path.join(ROOT, "png");
const CHROME = "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe";
const PORT = 9333;
const W = 390, H = 844, DSF = 2;

const NAMES = {
  "01": "01 Masuk", "02": "02 Masuk - Kredensial salah", "03": "03 Masuk - Memuat sesi",
  "04": "04 Dashboard - Staff - Normal", "05": "05 Dashboard - Pengunjung", "06": "06 Dashboard - Offline",
  "07": "07 Dashboard - Alarm aktif", "08": "08 Dashboard - Alarm dibungkam", "09": "09 Dashboard - Memuat",
  "10": "10 Grafik - Ada data", "11": "11 Grafik - Belum ada data",
  "12": "12 Kejadian - Daftar", "13": "13 Kejadian - Filter per tipe", "14": "14 Kejadian - Kosong",
  "15": "15 Kontrol - Staff - Normal", "16": "16 Kontrol - Pengunjung - Ditolak", "17": "17 Kontrol - Offline",
  "18": "18 Profil - Staff", "19": "19 Profil - Pengunjung",
  "20": "20 Dashboard - Alarm - Pengunjung", "21": "21 Dashboard - Alarm - Offline",
  "22": "22 Kontrol - Berhasil", "23": "23 Konfirmasi keluar", "24": "24 Grafik - Data parsial",
  "25": "25 Dashboard - Gelap", "26": "26 Kontrol - Gelap", "27": "27 Grafik - Gelap", "28": "28 Profil - Gelap",
  "29": "29 Grafik - Metrik lain", "30": "30 Grafik - Kelembapan", "31": "31 Grafik - Udara",
};
const IDS = Object.keys(NAMES).sort();

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

class CDP {
  constructor(ws) { this.ws = ws; this.id = 0; this.pending = new Map(); this.sessions = new Map();
    ws.addEventListener("message", (ev) => {
      const msg = JSON.parse(ev.data);
      if (msg.id && this.pending.has(msg.id)) {
        const { resolve, reject } = this.pending.get(msg.id);
        this.pending.delete(msg.id);
        msg.error ? reject(new Error(msg.error.message)) : resolve(msg.result);
      }
    });
  }
  send(method, params = {}, sessionId) {
    const id = ++this.id;
    const payload = { id, method, params };
    if (sessionId) payload.sessionId = sessionId;
    this.ws.send(JSON.stringify(payload));
    return new Promise((resolve, reject) => this.pending.set(id, { resolve, reject }));
  }
}

async function main() {
  fs.mkdirSync(OUT, { recursive: true });
  const userDir = path.join(process.env.TEMP || ".", "cdp-chrome-profile");
  const chrome = spawn(CHROME, [
    "--headless=new", "--disable-gpu", "--no-sandbox", "--hide-scrollbars",
    "--no-first-run", "--no-default-browser-check", "--disable-extensions",
    `--remote-debugging-port=${PORT}`, `--user-data-dir=${userDir}`, "about:blank",
  ], { stdio: "ignore" });

  let version;
  for (let i = 0; i < 60; i++) {
    try {
      version = await (await fetch(`http://127.0.0.1:${PORT}/json/version`)).json();
      if (version.webSocketDebuggerUrl) break;
    } catch { /* not up yet */ }
    await sleep(250);
  }
  if (!version?.webSocketDebuggerUrl) { chrome.kill(); throw new Error("Chrome CDP tidak siap"); }

  const ws = new WebSocket(version.webSocketDebuggerUrl);
  await new Promise((res, rej) => { ws.addEventListener("open", res); ws.addEventListener("error", rej); });
  const cdp = new CDP(ws);

  const { targetId } = await cdp.send("Target.createTarget", { url: "about:blank" });
  const { sessionId } = await cdp.send("Target.attachToTarget", { targetId, flatten: true });
  await cdp.send("Page.enable", {}, sessionId);
  await cdp.send("Emulation.setDeviceMetricsOverride",
    { width: W, height: H, deviceScaleFactor: DSF, mobile: false }, sessionId);

  const url = (id) => `file:///${PROTO.replace(/\\/g, "/")}?s=${id}&export=1`;

  for (const id of IDS) {
    await cdp.send("Page.navigate", { url: url(id) }, sessionId);
    await sleep(120);
    // tunggu font & layout selesai
    for (let i = 0; i < 40; i++) {
      const r = await cdp.send("Runtime.evaluate", {
        expression: "document.readyState === 'complete' && document.fonts.status === 'loaded' && !!document.querySelector('.stage')",
        returnByValue: true,
      }, sessionId);
      if (r.result?.value === true) break;
      await sleep(100);
    }
    await sleep(80);
    const shot = await cdp.send("Page.captureScreenshot",
      { format: "png", captureBeyondViewport: false, fromSurface: true }, sessionId);
    const file = path.join(OUT, NAMES[id] + ".png");
    fs.writeFileSync(file, Buffer.from(shot.data, "base64"));
    const size = fs.statSync(file).size;
    console.log(`OK   ${NAMES[id]}  ${Math.round(size / 1024)}KB`);
  }

  ws.close();
  chrome.kill();
  console.log(`\nTotal: ${IDS.length} PNG di ${OUT}`);
}

main().catch((e) => { console.error("FAIL:", e.message); process.exit(1); });
