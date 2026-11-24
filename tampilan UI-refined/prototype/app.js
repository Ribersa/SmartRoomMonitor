/* SmartRoomMonitor UI — data-driven prototype */
const $ = (s) => document.querySelector(s);
const params = new URLSearchParams(location.search);
const INITIAL = (params.get("s") || location.hash.replace("#", "") || "04").padStart(2, "0");
let SCREEN = INITIAL;
let DARK = false;
const EXPORT = params.get("export") === "1";

const NAMES = {
  "01": "Masuk", "02": "Kredensial salah", "03": "Memuat sesi",
  "04": "Dashboard Staff", "05": "Dashboard Pengunjung", "06": "Offline",
  "07": "Alarm aktif", "08": "Alarm dibungkam", "09": "Dashboard memuat",
  "10": "Grafik ada data", "11": "Grafik kosong", "12": "Kejadian daftar",
  "13": "Filter tipe", "14": "Kejadian kosong",
  "15": "Kontrol Staff", "16": "Kontrol ditolak", "17": "Kontrol offline",
  "18": "Profil Staff", "19": "Profil Pengunjung",
  "20": "Alarm · Pengunjung", "21": "Alarm · Offline", "22": "Kontrol berhasil",
  "23": "Konfirmasi keluar", "24": "Grafik parsial", "29": "Grafik metrik lain", "30": "Grafik kelembapan", "31": "Grafik udara",
  "25": "Dashboard gelap", "26": "Kontrol gelap", "27": "Grafik gelap", "28": "Profil gelap",
};
const GROUPS = [
  ["Masuk", ["01","02","03"]],
  ["Dashboard", ["04","05","06","07","08","09","20","21"]],
  ["Grafik", ["10","11","24","29","30","31"]],
  ["Kejadian", ["12","13","14"]],
  ["Kontrol", ["15","16","17","22"]],
  ["Profil", ["18","19","23"]],
  ["Mode gelap", ["25","26","27","28"]],
];
const NAV_GO = { home: "04", chart: "10", list: "12", sliders: "15", profile: "18" };

/* ---------- icons (inline SVG, no emoji) ---------- */
const ic = {
  wifi: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M5 12.5a10 10 0 0 1 14 0"/><path d="M8.5 16a5 5 0 0 1 7 0"/><path d="M2 9a15 15 0 0 1 20 0"/><line x1="12" y1="19.5" x2="12" y2="19.5"/></svg>`,
  battery: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="7" width="17" height="10" rx="2.5"/><line x1="22" y1="10.5" x2="22" y2="13.5"/></svg>`,
  thermo: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><path d="M14 14.76V4.5a2.5 2.5 0 0 0-5 0v10.26a4.5 4.5 0 1 0 5 0Z"/><line x1="11.5" y1="8" x2="11.5" y2="15"/></svg>`,
  drop: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M12 3s6 6.5 6 11a6 6 0 0 1-12 0c0-4.5 6-11 6-11Z"/></svg>`,
  wind: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><path d="M3 8h11a3 3 0 1 0-3-3"/><path d="M3 12h15a3 3 0 1 1-3 3"/><path d="M3 16h7"/></svg>`,
  sun: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><circle cx="12" cy="12" r="4"/><path d="M12 2v2M12 20v2M4.9 4.9l1.4 1.4M17.7 17.7l1.4 1.4M2 12h2M20 12h2M4.9 19.1l1.4-1.4M17.7 6.3l1.4-1.4"/></svg>`,
  user: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><circle cx="12" cy="8" r="4"/><path d="M4 21a8 8 0 0 1 16 0"/></svg>`,
  bell: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M18 8a6 6 0 1 0-12 0c0 7-3 7-3 9h18c0-2-3-2-3-9Z"/><path d="M10 21h4"/></svg>`,
  mute: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M11 5 6 9H3v6h3l5 4V5Z"/><line x1="16" y1="9" x2="22" y2="15"/><line x1="22" y1="9" x2="16" y2="15"/></svg>`,
  alert: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M10.3 3.9 1.8 18a2 2 0 0 0 1.7 3h17a2 2 0 0 0 1.7-3L13.7 3.9a2 2 0 0 0-3.4 0Z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12" y2="17"/></svg>`,
  off: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><path d="M1 1l22 22"/><path d="M16.7 16.7A11 11 0 0 1 5 12.5M4.3 8.4A11 11 0 0 1 19.4 8"/><path d="M8.5 12a5 5 0 0 1 5-3.9M2 5a16 16 0 0 1 4.3 2.6"/><line x1="12" y1="19.5" x2="12" y2="19.5"/></svg>`,
  home: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linejoin="round"><path d="M3 10.5 12 3l9 7.5V20a1 1 0 0 1-1 1h-5v-6H9v6H4a1 1 0 0 1-1-1Z"/></svg>`,
  chart: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M3 3v18h18"/><path d="M6 15l4-5 3 3 5-7"/></svg>`,
  list: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><line x1="8" y1="6" x2="20" y2="6"/><line x1="8" y1="12" x2="20" y2="12"/><line x1="8" y1="18" x2="20" y2="18"/><circle cx="4" cy="6" r="1"/><circle cx="4" cy="12" r="1"/><circle cx="4" cy="18" r="1"/></svg>`,
  sliders: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><line x1="4" y1="7" x2="20" y2="7"/><line x1="4" y1="17" x2="20" y2="17"/><circle cx="9" cy="7" r="2.2" fill="var(--surface)"/><circle cx="15" cy="17" r="2.2" fill="var(--surface)"/></svg>`,
  profile: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><circle cx="12" cy="12" r="9"/><circle cx="12" cy="9" r="3"/><path d="M6.5 19a6 6 0 0 1 11 0"/></svg>`,
  eye: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M2 12s3.6-6 10-6 10 6 10 6-3.6 6-10 6S2 12 2 12Z"/><circle cx="12" cy="12" r="2.6"/></svg>`,
  eyeoff: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M3 3l18 18"/><path d="M10.6 6.2A9.7 9.7 0 0 1 12 6c6.4 0 10 6 10 6a15 15 0 0 1-3.2 3.9M6.3 6.9C3.6 8.6 2 12 2 12s3.6 6 10 6a9.6 9.6 0 0 0 4-.8"/><path d="M9.5 9.5a3.5 3.5 0 0 0 4.9 4.9"/></svg>`,
  chev: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="m9 6 6 6-6 6"/></svg>`,
  check: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m4 12 5 5L20 6"/></svg>`,
  shield: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M12 3 4 6v6c0 5 3.4 8 8 9 4.6-1 8-4 8-9V6Z"/><path d="m9 12 2 2 4-4"/></svg>`,
  info: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><circle cx="12" cy="12" r="9"/><line x1="12" y1="11" x2="12" y2="16"/><line x1="12" y1="8" x2="12" y2="8"/></svg>`,
  fan: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="2"/><path d="M12 10c0-4 1-7 4-7 2 0 3 2 2 4-1 2-4 3-6 3ZM14 12c4 0 7 1 7 4 0 2-2 3-4 2-2-1-3-4-3-6ZM12 14c0 4-1 7-4 7-2 0-3-2-2-4 1-2 4-3 6-3ZM10 12c-4 0-7-1-7-4 0-2 2-3 4-2 2 1 3 4 3 6Z"/></svg>`,
  bulb: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M9 18h6M10 21h4"/><path d="M12 3a6 6 0 0 0-4 10.5c.7.8 1 1.5 1 2.5h6c0-1 .3-1.7 1-2.5A6 6 0 0 0 12 3Z"/></svg>`,
  lock: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><rect x="4" y="10" width="16" height="11" rx="2"/><path d="M8 10V7a4 4 0 0 1 8 0v3"/></svg>`,
  logout: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M9 4H5a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h4"/><path d="M16 16l4-4-4-4M20 12H9"/></svg>`,
  palette: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><path d="M12 3a9 9 0 1 0 0 18c1.5 0 2-1 2-2s-.7-2 0-3 2-1 3-1h1a4 4 0 0 0 4-4A9 9 0 0 0 12 3Z"/><circle cx="7.5" cy="11" r="1"/><circle cx="10" cy="7" r="1"/><circle cx="15" cy="7.5" r="1"/></svg>`,
  clock: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"><circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/></svg>`,
  plus: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><line x1="12" y1="5" x2="12" y2="19"/><line x1="5" y1="12" x2="19" y2="12"/></svg>`,
  minus: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><line x1="5" y1="12" x2="19" y2="12"/></svg>`,
  inbox: `<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M3 13h5l2 3h4l2-3h5"/><path d="M5 5h14l2 8v6a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1v-6Z"/></svg>`,
};

/* ---------- shared chrome ---------- */
function statusbar() {
  return `<div class="statusbar"><span>9:41</span><span class="sb-right">${ic.wifi}${ic.battery}</span></div>`;
}
function nav(active) {
  const items = [
    ["Dashboard", "home"], ["Grafik", "chart"], ["Kejadian", "list"],
    ["Kontrol", "sliders"], ["Profil", "profile"],
  ];
  return `<nav class="bottomnav">${items.map(([label, key]) =>
    `<div class="nav-item ${key === active ? "active" : ""}" data-go="${NAV_GO[key]}">${ic[key]}<span>${label}</span></div>`
  ).join("")}</nav>`;
}
function topbar(title, sub, opts = {}) {
  return `<div class="topbar"><div class="row"><div><h1>${title}</h1>${sub ? `<div class="sub">${sub}</div>` : ""}</div>${opts.avatar ? `<div class="avatar">SR</div>` : ""}</div></div>`;
}
function statusLine(state = "ok") {
  if (state === "loading") return `<div class="statusline"><span class="sk" style="width:150px;height:12px"></span></div>`;
  if (state === "off") return `<div class="statusline"><span class="dot off"></span>Perangkat offline · data terakhir 12:04:31</div>`;
  return `<div class="statusline"><span class="dot"></span>Tersambung · Diperbarui 12:04:31</div>`;
}
function offlineBanner() {
  return `<div class="banner danger">${ic.off}<div><div class="b-title">Perangkat offline</div><div class="b-body">Data terhenti lebih dari 15 detik. Data terakhir tetap ditampilkan.</div></div></div>`;
}
function alarmBanner(mode, role) {
  // mode: active | muted | visitor | offline
  const isStaff = role !== "visitor";
  if (mode === "muted") {
    return `<div class="banner">${ic.mute}<div><div class="b-title">Alarm dibungkam</div><div class="b-body">Bunyi peringatan dimatikan. Alarm aktif lagi dalam 60 detik bila kondisi bahaya belum mereda.</div><div class="b-time">Dibungkam 12:03:58 WIB</div></div></div>`;
  }
  const extra = mode === "offline" ? " · Perangkat offline" : "";
  return `<div class="banner danger">${ic.alert}<div><div class="b-title">Peringatan aktif${extra}</div><div class="b-body">Suhu 36,4 °C melebihi batas 35,0 °C. Perangkat membunyikan alarm.</div><div class="b-time">12:04:31 WIB · Suhu tinggi</div></div></div>
  ${isStaff ? `<button class="btn danger block" data-go="08">${ic.mute} MUTE Darurat</button>` : `<div class="note">Hanya Staff yang dapat membungkam alarm.</div>`}`;
}

/* ---------- metric cards ---------- */
function metricCard(o) {
  return `<div class="card"><div class="card-head"><div class="card-label">${ic[o.icon]}${o.label}</div><span class="badge ${o.badge}">${o.status}</span></div>
  <div class="metric"><span class="val">${o.val}</span><span class="unit">${o.unit}</span>${o.aux ? `<span class="aux">${o.aux}</span>` : ""}</div>
  ${o.foot ? `<div class="card-foot">${o.foot}</div>` : ""}</div>`;
}
function dashboardCards(variant) {
  const warm = variant === "warm";
  const air = warm
    ? { label: "Kualitas udara", icon: "wind", val: "—", unit: "", badge: "warn", status: "Memanaskan", aux: "Delta 0 ppb", foot: `<span>Masa pemanasan 3 menit</span><span><strong>Tidak ada peringatan</strong></span>` }
    : { label: "Kualitas udara", icon: "wind", val: "18", unit: "ppb", badge: "ok", status: "Baik", aux: "AQI 1", foot: `<span>Delta dari baseline</span><span><strong>Buruk ≥ 160 ppb</strong></span>` };
  return `
  ${metricCard({ label: "Suhu", icon: "thermo", val: "36,4", unit: "°C", badge: "bad", status: "Di atas batas", aux: "", foot: `<span>Batas peringatan</span><span><strong>35,0 °C</strong></span>` })}
  ${metricCard({ label: "Kelembapan", icon: "drop", val: "58", unit: "%RH", badge: "ok", status: "Nyaman", foot: `<span>Rentang nyaman</span><span><strong>40–60 %RH</strong></span>` })}
  ${metricCard(air)}
  ${metricCard({ label: "Cahaya", icon: "sun", val: "112", unit: "lux", badge: "ok", status: "Terang", foot: `<span>Ambang gelap</span><span><strong>40 lux</strong></span>` })}
  ${metricCard({ label: "Kehadiran", icon: "user", val: "1", unit: "orang", badge: "ok", status: "Diam", foot: `<span>Jam malam</span><span><strong>22:00–06:00 WIB</strong></span>` })}
  ${metricCard({ label: "Kipas & lampu", icon: "fan", val: "Nyala", unit: "", badge: "ok", status: "Otomatis", foot: `<span>Kipas fisik</span><span><strong>Nyala</strong></span>` })}`;
}

/* ---------- chart ---------- */
const METRICS = {
  temp: {
    name: "Suhu", unit: "°C", icon: "thermo", min: 25, max: 36, limit: 35, limitLabel: "Batas 35 °C",
    series: [26.8,27.0,27.3,27.6,28.0,28.6,29.2,29.8,30.4,31.1,31.6,31.2,30.8,30.2,29.6,29.1,28.7,28.4,28.2,28.0,27.8,27.5,27.2,27.0],
    partial: [27.4, 27.9, 28.3, 28.0, 28.6, 29.1],
    stats: [["Terendah","26,8 °C"],["Rata-rata","28,9 °C"],["Tertinggi","31,6 °C"]],
    statsPartial: [["Terendah","27,4 °C"],["Rata-rata","28,2 °C"],["Tertinggi","29,1 °C"]],
  },
  humidity: {
    name: "Kelembapan", unit: "%RH", icon: "drop", min: 40, max: 70, limit: 60, limitLabel: "Batas nyaman 60 %RH",
    series: [58,57,56,55,54,55,57,59,61,63,62,60,58,56,55,54,55,56,57,58,58,59,58,58],
    partial: [58,58,57,57,58,58],
    stats: [["Terendah","54 %RH"],["Rata-rata","57,5 %RH"],["Tertinggi","63 %RH"]],
    statsPartial: [["Terendah","57 %RH"],["Rata-rata","57,7 %RH"],["Tertinggi","58 %RH"]],
  },
  air: {
    name: "Kualitas udara", unit: "ppb", icon: "wind", min: 0, max: 200, limit: 160, limitLabel: "Buruk ≥ 160 ppb",
    series: [12,14,18,22,30,44,62,88,118,146,172,158,130,96,68,44,30,22,18,16,14,12,12,10],
    partial: [8, 10, 14, 20, 26, 34],
    stats: [["Terendah","10 ppb"],["Rata-rata","60,2 ppb"],["Tertinggi","172 ppb"]],
    statsPartial: [["Terendah","8 ppb"],["Rata-rata","18,7 ppb"],["Tertinggi","34 ppb"]],
  },
};

function chartSVG(metricKey, partial) {
  const m = METRICS[metricKey] || METRICS.temp;
  const W = 320, H = 150, pl = 30, pr = 10, pt = 12, pb = 22;
  const vals = partial ? m.partial : m.series;
  const n = vals.length;
  const x = (i) => pl + (i / (n - 1)) * (W - pl - pr);
  const y = (v) => pt + (1 - (v - m.min) / (m.max - m.min)) * (H - pt - pb);
  const line = vals.map((v, i) => `${i === 0 ? "M" : "L"}${x(i).toFixed(1)},${y(v).toFixed(1)}`).join(" ");
  const limitY = y(m.limit);
  const ticks = [m.min, m.min + (m.max - m.min) / 3, m.min + 2 * (m.max - m.min) / 3, m.max]
    .map((v) => Math.round(v));
  const grid = ticks.map((v) => `<line x1="${pl}" y1="${y(v)}" x2="${W-pr}" y2="${y(v)}" stroke="var(--border-soft)" stroke-width="1"/><text x="${pl-6}" y="${y(v)+3}" text-anchor="end" font-size="9" fill="var(--faint)">${v}</text>`).join("");
  const dots = vals.map((v, i) => `<circle cx="${x(i).toFixed(1)}" cy="${y(v).toFixed(1)}" r="2" fill="var(--danger)"/>`).join("");
  return `<svg viewBox="0 0 ${W} ${H}" role="img" aria-label="Grafik ${m.name} 24 jam, garis ambang ${m.limit} ${m.unit} putus-putus">
    ${grid}
    <line x1="${pl}" y1="${limitY}" x2="${W-pr}" y2="${limitY}" stroke="var(--danger)" stroke-width="1.4" stroke-dasharray="5 4"/>
    <text x="${W-pr}" y="${limitY-5}" text-anchor="end" font-size="9" font-weight="700" fill="var(--danger)">${m.limitLabel}</text>
    <path d="${line}" fill="none" stroke="var(--text)" stroke-width="2.2" stroke-linejoin="round" stroke-linecap="round"/>
    ${dots}
    <text x="${pl}" y="${H-6}" font-size="9" fill="var(--faint)">00.00</text>
    <text x="${x((n-1)/2).toFixed(1)}" y="${H-6}" text-anchor="middle" font-size="9" fill="var(--faint)">${partial ? "00.30" : "12.00"}</text>
    <text x="${W-pr}" y="${H-6}" text-anchor="end" font-size="9" fill="var(--faint)">${partial ? "01.00" : "23.00"}</text>
  </svg>`;
}

function chartCard(metricKey, partial) {
  const m = METRICS[metricKey] || METRICS.temp;
  const stats = partial ? m.statsPartial : m.stats;
  return `<div class="card">
    <div class="card-head"><div class="card-label">${ic[m.icon]}${m.name} · 24 jam terakhir</div><span class="badge">${m.unit}</span></div>
    <div class="chart-wrap">${chartSVG(metricKey, partial)}</div>
    <div class="chart-legend"><span><i></i> ${m.name} ruangan</span><span><i class="dash"></i> ${m.limitLabel}</span></div>
    <div class="stats">${stats.map(([k,v])=>`<div class="stat"><div class="k">${k}</div><div class="v">${v}</div></div>`).join("")}</div>
    <div class="card-foot" style="margin-top:14px"><span>${partial ? m.partial.length : 144} titik · interval 10 menit</span><span><strong>Sumber: room1/history</strong></span></div>
  </div>`;
}

/* Chip metrik: ringkas = hanya Suhu; expand = Suhu, Kelembapan, Udara */
function metricChips(expanded, active) {
  const activeKey = active || "temp";
  if (!expanded) {
    return `<div class="chips">
      <span class="chip active">Suhu</span>
      <button class="chip chip-expand" data-go="29">${ic.plus} Metrik lain</button>
    </div>`;
  }
  return `<div class="chips">
    ${[["temp","Suhu"],["humidity","Kelembapan"],["air","Udara"]].map(([k,label]) =>
      `<span class="chip ${k===activeKey?"active":""}" data-go="${k==="temp"?"10":k==="humidity"?"30":"31"}">${label}</span>`).join("")}
    <button class="chip chip-expand" data-go="10">${ic.minus} Ringkas</button>
  </div>`;
}

/* ---------- events ---------- */
const EVENTS = [
  { t: "12:04", title: "Suhu tinggi", msg: "36,4 °C, di atas batas 35,0 °C. Buzzer berbunyi.", tag: "TEMP_HIGH · BAHAYA", danger: true },
  { t: "11:47", title: "Udara buruk", msg: "Delta 172 ppb dari baseline, AQI 4. Label Buruk.", tag: "AIR_BAD · BAHAYA", danger: true },
  { t: "03:12", title: "Ada orang di jam malam", msg: "Terdeteksi keberadaan orang (diam) pada 03:12 WIB.", tag: "NIGHT_PRESENCE · BAHAYA", danger: true },
  { t: "22:38", title: "Udara sedang", msg: "Delta 92 ppb dari baseline. Label Sedang.", tag: "AIR_BAD · PERINGATAN", danger: false },
];
function eventRow(e) {
  return `<div class="event"><span class="ev-dot ${e.danger ? "danger" : ""}"></span><div class="ev-body">
    <div class="ev-top"><span class="ev-title">${e.title}</span><span class="ev-time">${e.t}</span></div>
    <div class="ev-msg">${e.msg}</div><div class="ev-tag">${e.tag}</div></div></div>`;
}
function chips(active) {
  return `<div class="chips">${["Semua","Suhu tinggi","Udara buruk","Jam malam"].map((c,i)=>`<span class="chip ${i===active?"active":""}" data-go="${i===1?"13":"12"}">${c}</span>`).join("")}</div>`;
}

/* ---------- profile ---------- */
function profileContent(role) {
  const isStaff = role === "staff";
  const roleLabel = isStaff ? "Staff" : "Pengunjung";
  return `<div class="profile-head"><div class="pa">${isStaff ? "ST" : "PG"}</div><div><div class="pn">${isStaff ? "Staff Pengelola" : "Pengunjung Ruangan"}</div><div class="pr">Peran: ${roleLabel}</div></div></div>
  <div class="section-head"><h2>Akun</h2></div>
  <div class="card tight">
    <div class="list-row">${ic.shield}<span class="lr-label">Peran</span><span class="lr-val">${roleLabel}</span></div>
    <div class="list-row">${ic.palette}<span class="lr-label">Tampilan</span><span class="lr-val">Ikut sistem</span>${ic.chev}</div>
    <div class="list-row">${ic.bell}<span class="lr-label">Notifikasi</span><span class="lr-val">Aktif</span>${ic.chev}</div>
  </div>
  <div class="section-head"><h2>Hak akses</h2></div>
  <div class="card tight">
    <div class="list-row">${ic.eye}<span class="lr-label">Lihat dashboard & grafik</span>${ic.check}</div>
    <div class="list-row">${ic.sliders}<span class="lr-label">Kontrol kipas, lampu & MUTE</span><span style="color:${isStaff?"var(--success)":"var(--faint)"}">${isStaff ? ic.check : ic.lock}</span></div>
  </div>
  <div class="note" style="margin-top:10px">Batas peringatan diubah melalui Firebase Console, bukan dari aplikasi.</div>
  <button class="btn ghost block" data-go="23">${ic.logout} Keluar</button>
  <div class="login-foot">SmartRoomMonitor · SRS v1.0 · ruang room1</div>`;
}

/* ---------- login ---------- */
function loginForm(error) {
  return `<div class="login-hero"><div class="login-mark">${ic.home}</div><h1>SmartRoomMonitor</h1><p>Pantau kondisi ruangan, terima peringatan, dan atur kipas serta lampu dari satu layar.</p></div>
  ${error ? `<div class="err-summary" role="alert" tabindex="-1" id="errsum"><div class="es-title">${ic.alert} Email atau kata sandi salah</div><div class="es-body">Periksa kembali lalu coba lagi. Pastikan akun Anda sudah terdaftar pada Firebase Auth.</div></div>` : ""}
  <div class="field"><label for="email">Email</label><div class="input-wrap"><input id="email" class="input ${error?"invalid":""}" type="email" value="staff@smartroom.local" aria-describedby="${error?"email-err":""}"></div>${error?`<div class="field-err" id="email-err">${ic.alert} Periksa alamat email</div>`:""}</div>
  <div class="field"><label for="pass">Kata sandi</label><div class="input-wrap"><input id="pass" class="input ${error?"invalid":""}" type="password" value="rahasia123"><button class="input-btn" aria-label="Tampilkan kata sandi">${ic.eyeoff}</button></div></div>
  <div class="link">Lupa kata sandi?</div>
  <button class="btn block" data-go="04">Masuk</button>
  <div class="login-foot">Akses tanpa login ditolak.<br>Hubungi pengelola ruangan untuk mendapatkan akun.</div>`;
}

/* ---------- screen registry ---------- */
const screens = {
  "01": () => ({ nav: null, body: `<div class="viewport" style="padding-top:4px">${loginForm(false)}</div>` }),
  "02": () => ({ nav: null, body: `<div class="viewport" style="padding-top:4px">${loginForm(true)}</div>` }),
  "03": () => ({ nav: null, body: `<div class="viewport" style="padding-top:4px"><div class="login-hero"><div class="login-mark">${ic.home}</div><h1>SmartRoomMonitor</h1></div><div class="sk sk-line" style="width:60%;height:16px"></div><div class="sk sk-line" style="width:85%"></div><div class="sk sk-card"></div><div class="sk sk-card"></div><div class="note" style="margin-top:20px">Memuat sesi tersimpan…</div></div>` }),

  "04": (d) => ({ nav: "home", body: `<div class="viewport has-nav">${topbar("Dashboard", "Ruang 1 · Selasa, 24 November 2026", {avatar:true})}${statusLine()}${dashboardCards()}</div>` }),
  "05": (d) => ({ nav: "home", body: `<div class="viewport has-nav">${topbar("Dashboard", "Ruang 1 · Selasa, 24 November 2026", {avatar:true})}${statusLine()}${dashboardCards()}<div class="note">Anda masuk sebagai Pengunjung. Kontrol kipas, lampu, dan tombol MUTE tidak tersedia.</div></div>` }),
  "06": (d) => ({ nav: "home", body: `<div class="viewport has-nav">${topbar("Dashboard", "Ruang 1", {avatar:true})}${statusLine("off")}${offlineBanner()}${dashboardCards()}</div>` }),
  "07": (d) => ({ nav: "home", body: `<div class="viewport has-nav">${topbar("Dashboard", "Ruang 1", {avatar:true})}${statusLine()}${alarmBanner("active","staff")}${dashboardCards()}</div>` }),
  "08": (d) => ({ nav: "home", body: `<div class="viewport has-nav">${topbar("Dashboard", "Ruang 1", {avatar:true})}${statusLine()}${alarmBanner("muted","staff")}${dashboardCards()}</div>` }),
  "09": (d) => ({ nav: "home", body: `<div class="viewport has-nav">${topbar("Dashboard", "Ruang 1")}${statusLine("loading")}<div class="sk sk-card"></div><div class="sk sk-card"></div><div class="sk sk-card"></div><div class="note" style="margin-top:16px">Menunggu data pertama dari perangkat…</div></div>` }),
  "20": (d) => ({ nav: "home", body: `<div class="viewport has-nav">${topbar("Dashboard", "Ruang 1", {avatar:true})}${statusLine()}${alarmBanner("active","visitor")}${dashboardCards()}</div>` }),
  "21": (d) => ({ nav: "home", body: `<div class="viewport has-nav">${topbar("Dashboard", "Ruang 1", {avatar:true})}${statusLine("off")}${offlineBanner()}${alarmBanner("offline","staff")}${dashboardCards()}</div>` }),

  "10": () => ({ nav: "chart", body: `<div class="viewport has-nav">${topbar("Grafik 24 Jam", "Ruang 1 · Suhu")}${statusLine()}${metricChips(false)}${chartCard("temp",false)}<div class="note">Grafik menampilkan suhu secara default. Tekan "Metrik lain" untuk kelembapan dan kualitas udara.</div></div>` }),
  "29": () => ({ nav: "chart", body: `<div class="viewport has-nav">${topbar("Grafik 24 Jam", "Ruang 1 · Suhu")}${statusLine()}${metricChips(true,"temp")}${chartCard("temp",false)}</div>` }),
  "30": () => ({ nav: "chart", body: `<div class="viewport has-nav">${topbar("Grafik 24 Jam", "Ruang 1 · Kelembapan")}${statusLine()}${metricChips(true,"humidity")}${chartCard("humidity",false)}</div>` }),
  "31": () => ({ nav: "chart", body: `<div class="viewport has-nav">${topbar("Grafik 24 Jam", "Ruang 1 · Kualitas udara")}${statusLine()}${metricChips(true,"air")}${chartCard("air",false)}<div class="note">Nilai udara adalah airDelta terhadap baseline normal, bukan angka absolut.</div></div>` }),
  "11": (d) => ({ nav: "chart", body: `<div class="viewport has-nav">${topbar("Grafik 24 Jam", "Ruang 1 · Suhu")}${statusLine()}${metricChips(false)}<div class="card"><div class="empty"><div class="em-icon">${ic.chart}</div><h3>Belum ada data</h3><p>Perangkat menyimpan satu titik tiap 10 menit. Grafik menampilkan hingga 144 titik untuk 24 jam. Data akan muncul setelah titik pertama tersimpan.</p></div></div></div>` }),
  "24": (d) => ({ nav: "chart", body: `<div class="viewport has-nav">${topbar("Grafik 24 Jam", "Ruang 1 · Suhu")}${statusLine()}${metricChips(false)}${chartCard("temp",true)}<div class="note">Perangkat baru aktif 60 menit. Grafik akan terus terisi sampai 144 titik.</div></div>` }),

  "12": (d) => ({ nav: "list", body: `<div class="viewport has-nav">${topbar("Kejadian", "24 jam terakhir · 4 catatan")}${statusLine()}<div class="section-head"><h2>Riwayat kejadian</h2><span class="hint">Ditulis ESP32</span></div><div class="card tight">${EVENTS.slice(0,3).map(eventRow).join("")}</div><div class="note">Warna bukan satu-satunya penanda. Setiap baris memuat label tipe dan tingkat keparahan.</div></div>` }),
  "13": (d) => ({ nav: "list", body: `<div class="viewport has-nav">${topbar("Kejadian", "Filter: Suhu tinggi")}${statusLine()}${chips(1)}<div class="section-head"><h2>1 kejadian ditemukan</h2></div><div class="card tight">${eventRow(EVENTS[0])}</div></div>` }),
  "14": (d) => ({ nav: "list", body: `<div class="viewport has-nav">${topbar("Kejadian", "24 jam terakhir")}${statusLine()}${chips(0)}<div class="card"><div class="empty"><div class="em-icon">${ic.inbox}</div><h3>Tidak ada kejadian</h3><p>Belum ada kondisi bahaya dalam 24 jam terakhir. Kejadian suhu tinggi, udara buruk, dan kehadiran jam malam akan muncul di sini.</p></div></div></div>` }),

  "15": (d) => ({ nav: "sliders", body: `<div class="viewport has-nav">${topbar("Kontrol", "Khusus Staff · Ruang 1")}${statusLine()}${controlCard("Kipas","fan","Nyala","Otomatis","Menyala di atas 31,0 °C, mati pada 30,0 °C atau kurang.")}${controlCard("Lampu","bulb","Mati","Otomatis","Menyala saat gelap dan ada orang; padam saat cahaya luar naik 200 lux atau ruangan kosong 10 detik.")}<div class="note">Mode disimpan di room1/actuator. Status fisik dibaca dari room1/current.</div></div>` }),
  "16": (d) => ({ nav: "sliders", body: `<div class="viewport has-nav">${topbar("Kontrol", "Ruang 1")}${statusLine()}<div class="banner">${ic.lock}<div><div class="b-title">Akses ditolak</div><div class="b-body">Anda masuk sebagai Pengunjung. Hanya Staff yang dapat mengubah mode kipas, lampu, dan membungkam alarm.</div></div></div><button class="btn ghost block" data-go="04">Kembali ke Dashboard</button></div>` }),
  "17": (d) => ({ nav: "sliders", body: `<div class="viewport has-nav">${topbar("Kontrol", "Khusus Staff · Ruang 1")}${statusLine("off")}${offlineBanner()}${controlCard("Kipas","fan","Nyala","Otomatis","Perintah tertunda sampai perangkat kembali terhubung.",true)}${controlCard("Lampu","bulb","Mati","Otomatis","Perintah tertunda sampai perangkat kembali terhubung.",true)}</div>` }),
  "22": (d) => ({ nav: "sliders", body: `<div class="viewport has-nav">${topbar("Kontrol", "Khusus Staff · Ruang 1")}${statusLine()}${controlCard("Kipas","fan","Nyala","Selalu ON","Mengabaikan suhu ruangan.")}${controlCard("Lampu","bulb","Mati","Otomatis","Menyala saat gelap dan ada orang.")}<div class="snackbar" role="status">${ic.check} Mode kipas diperbarui menjadi Selalu ON</div></div>` }),

  "18": (d) => ({ nav: "profile", body: `<div class="viewport has-nav">${topbar("Profil", "Akun & preferensi")}${profileContent("staff")}</div>` }),
  "19": (d) => ({ nav: "profile", body: `<div class="viewport has-nav">${topbar("Profil", "Akun & preferensi")}${profileContent("visitor")}</div>` }),
  "23": (d) => ({ nav: "profile", body: `<div class="viewport has-nav">${topbar("Profil", "Akun & preferensi")}${profileContent("staff")}<div class="scrim"><div class="dialog" role="dialog" aria-modal="true"><h3>Keluar dari akun?</h3><p>Sesi Anda akan dihapus dan Anda kembali ke layar masuk. Data ruangan tetap tersimpan.</p><div class="d-actions"><button class="btn" data-go="18">Batal</button><button class="btn ghost" data-go="01">${ic.logout} Keluar</button></div></div></div></div>` }),

  "25": (d) => ({ nav: "home", body: `<div class="viewport has-nav">${topbar("Dashboard", "Ruang 1 · Tema gelap", {avatar:true})}${statusLine()}${dashboardCards("warm")}</div>` }),
  "26": (d) => ({ nav: "sliders", body: `<div class="viewport has-nav">${topbar("Kontrol", "Khusus Staff · Tema gelap")}${statusLine()}${controlCard("Kipas","fan","Nyala","Otomatis","Menyala di atas 31,0 °C, mati pada 30,0 °C atau kurang.")}${controlCard("Lampu","bulb","Mati","Otomatis","Padam saat ruangan kosong 10 detik.")}</div>` }),
  "27": (d) => ({ nav: "chart", body: `<div class="viewport has-nav">${topbar("Grafik 24 Jam", "Ruang 1 · Tema gelap")}${statusLine()}${metricChips(false)}${chartCard("temp",false)}</div>` }),
  "28": (d) => ({ nav: "profile", body: `<div class="viewport has-nav">${topbar("Profil", "Tema gelap aktif")}${profileContent("staff")}</div>` }),
};

function controlCard(name, icon, status, active, note, disabled) {
  const opts = ["Otomatis","Selalu ON","Selalu OFF"];
  return `<div class="card">
    <div class="card-head"><div class="card-label">${ic[icon]}${name}</div><span class="badge ${status==="Nyala"?"ok":""}">${status}</span></div>
    <div class="mode-status"><span class="dot ${status==="Nyala"?"":"warn"}"></span>Status fisik: <strong>${status}</strong></div>
    <div class="segmented">${opts.map(o=>`<div class="seg ${o===active?"active":""}" data-seg>${o}</div>`).join("")}</div>
    <div class="card-foot" style="margin-top:14px"><span>${note}</span></div>
  </div>`;
}

/* ---------- render & router ---------- */
function sheet() {
  if (EXPORT) return "";
  return `<button class="proto-fab" id="protoFab" aria-label="Pilih layar">Layar</button>
  <div class="proto-scrim" id="protoScrim" hidden></div>
  <aside class="proto-sheet" id="protoSheet" hidden aria-label="Daftar kondisi layar">
    <div class="ps-head"><strong>Pilih kondisi</strong><span class="ps-now" id="psNow"></span></div>
    <div class="ps-body">${GROUPS.map(([g, ids]) => `<div class="ps-group"><div class="ps-label">${g}</div>${ids.map(id =>
      `<button class="ps-item" data-go="${id}"><span class="ps-id">${id}</span>${NAMES[id]}</button>`).join("")}</div>`).join("")}</div>
  </aside>`;
}

function render() {
  DARK = ["25","26","27","28"].includes(SCREEN);
  document.documentElement.setAttribute("data-theme", DARK ? "dark" : "light");
  document.body.setAttribute("data-theme", DARK ? "dark" : "light");
  const build = screens[SCREEN] || screens["04"];
  const out = build(DARK);
  $("#app").innerHTML = `<div class="stage"><div class="phone">${statusbar()}${out.body}${out.nav ? nav(out.nav) : ""}</div>${sheet()}</div>`;
  if (EXPORT) document.body.classList.add("export");
  const now = $("#psNow"); if (now) now.textContent = `${SCREEN} · ${NAMES[SCREEN] || ""}`;
  if (!EXPORT) { try { history.replaceState(null, "", `?s=${SCREEN}`); } catch (err) {} }
}

function go(id) {
  id = String(id).padStart(2, "0");
  if (!screens[id]) return;
  SCREEN = id;
  render();
  $("#protoSheet")?.setAttribute("hidden", "");
  $("#protoScrim")?.setAttribute("hidden", "");
}

document.addEventListener("click", (e) => {
  const seg = e.target.closest("[data-seg]");
  if (seg) {
    const group = seg.parentElement.querySelectorAll(".seg");
    if (!seg.classList.contains("active")) {
      group.forEach((s) => s.classList.remove("active"));
      seg.classList.add("active");
    }
    return;
  }
  const target = e.target.closest("[data-go]");
  if (target) { go(target.dataset.go); return; }
  if (e.target.closest("#protoFab")) {
    $("#protoSheet").removeAttribute("hidden");
    $("#protoScrim").removeAttribute("hidden");
    return;
  }
  if (e.target.closest("#protoScrim")) {
    $("#protoSheet").setAttribute("hidden", "");
    $("#protoScrim").setAttribute("hidden", "");
  }
});

document.addEventListener("keydown", (e) => {
  if (e.key === "Escape") {
    $("#protoSheet")?.setAttribute("hidden", "");
    $("#protoScrim")?.setAttribute("hidden", "");
  }
});

window.addEventListener("hashchange", () => {
  const id = location.hash.replace("#", "").padStart(2, "0");
  if (screens[id] && id !== SCREEN) { SCREEN = id; render(); }
});

render();
