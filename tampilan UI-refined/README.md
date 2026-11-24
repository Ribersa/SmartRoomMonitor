# Tampilan UI — Refined

Prototipe UI hasil refinement dari 19 layar Figma di `../tampilan UI-20261124T120108Z-1-001/`. Folder asli tidak diubah.

## Isi

| Path | Fungsi |
|---|---|
| `ui-spec.md` | Token light/dark, type scale, komponen, matriks layar, copy final, checklist verifikasi |
| `prototype/index.html` | Satu halaman. Tanpa `?s=` masuk mode jelajah dengan panel 28 kondisi; `?s=<id>` membuka satu layar |
| `prototype/style.css` | Token CSS + seluruh komponen + panel navigator prototipe |
| `prototype/app.js` | Data & registry 28 layar, router hash/query, hotspot klik (SVG inline, tanpa emoji) |
| `prototype/fonts/` | Plus Jakarta Sans 400–700 (self-host, render deterministik/offline) |
| `tools/render.js` | Render PNG 390×844 @2x via Chrome DevTools Protocol (viewport akurat) |
| `png/` | Hasil render 31 layar, 780×1688 px |

## Cara pakai

Buka `prototype/index.html` di browser. Prototipe punya dua mode:

**1. Mode jelajah (interaktif)** — tanpa `?s=`:

- Tombol **Layar** di kanan bawah membuka panel berisi 7 grup dan 28 kondisi; klik untuk pindah.
- Bottom nav, tombol MUTE, chip filter, tombol Masuk/Keluar, dan dialog sudah bisa diklik.
- Segmented control bisa dipilih; URL ikut berubah (`?s=08`) sehingga kondisi bisa dibagikan.
- `Esc` menutup panel.

**2. Mode layar tunggal** — `?s=04` untuk membuka satu kondisi langsung, atau `#08` lewat hash.

```text
prototype/index.html          -> mulai dari Dashboard, panel aktif
prototype/index.html?s=07     -> langsung Alarm aktif
prototype/index.html?s=25     -> Dashboard gelap
```

**Alur klik yang sudah tersambung**

| Dari | Aksi | Ke |
|---|---|---|
| 01/02 Masuk | tombol Masuk | 04 |
| 04 Dashboard | MUTE Darurat | 08 |
| 04,10,12,15,18 | bottom nav | 04 / 10 / 12 / 15 / 18 |
| 12 Kejadian | chip Suhu tinggi | 13 |
| 15 Kontrol | segmen mode | berubah di tempat |
| 16 Ditolak | Kembali ke Dashboard | 04 |
| 18 Profil | Keluar | 23 |
| 23 Dialog | Batal / Keluar | 18 / 01 |
| semua | tombol Layar | panel 31 kondisi |

**Render ulang PNG**

```powershell
cd "E:\SmartroomMonitor\tampilan UI-refined"
node tools\render.js
```

Butuh Google Chrome di `C:\Program Files\Google\Chrome\Application\chrome.exe` dan Node.js. `export.ps1` sudah usang (bug display scaling Windows membuat viewport 492 px, bukan 390) — jangan dipakai.

## Daftar layar

| Id | Layar | Tema | Status |
|---|---|---|---|
| 01 | Masuk | Terang | Diperbaiki |
| 02 | Masuk · Kredensial salah | Terang | Diperbaiki (error summary + inline) |
| 03 | Masuk · Memuat sesi | Terang | Diperbaiki |
| 04 | Dashboard · Staff · Normal | Terang | Diperbaiki |
| 05 | Dashboard · Pengunjung | Terang | Diperbaiki |
| 06 | Dashboard · Offline | Terang | Diperbaiki |
| 07 | Dashboard · Alarm aktif | Terang | Diperbaiki (MUTE 1 tap) |
| 08 | Dashboard · Alarm dibungkam | Terang | Diperbaiki (re-arm 60 dtk) |
| 09 | Dashboard · Memuat | Terang | Diperbaiki (tanpa status palsu) |
| 10 | Grafik · Ada data | Terang | Diperbaiki (axis, satuan, ambang dashed) |
| 11 | Grafik · Belum ada data | Terang | Diperbaiki (10 menit / 144 titik) |
| 12 | Kejadian · Daftar | Terang | Diperbaiki (3 tipe event) |
| 13 | Kejadian · Filter per tipe | Terang | Diperbaiki |
| 14 | Kejadian · Kosong | Terang | Diperbaiki |
| 15 | Kontrol · Staff · Normal | Terang | Diperbaiki (kosong 10 dtk) |
| 16 | Kontrol · Pengunjung · Ditolak | Terang | Diperbaiki |
| 17 | Kontrol · Offline | Terang | Diperbaiki |
| 18 | Profil · Staff | Terang | Diperbaiki (typo) |
| 19 | Profil · Pengunjung | Terang | Diperbaiki |
| 20 | Dashboard · Alarm · Pengunjung | Terang | **Baru** |
| 21 | Dashboard · Alarm · Offline | Terang | **Baru** |
| 22 | Kontrol · Berhasil | Terang | **Baru** |
| 23 | Konfirmasi keluar | Terang | **Baru** |
| 24 | Grafik · Data parsial | Terang | **Baru** |
| 25 | Dashboard · Gelap | Gelap | **Baru** |
| 26 | Kontrol · Gelap | Gelap | **Baru** |
| 27 | Grafik · Gelap | Gelap | **Baru** |
| 28 | Profil · Gelap | Gelap | **Baru** |
| 29 | Grafik · Metrik lain | Terang | **Baru** (chip expand) |
| 30 | Grafik · Kelembapan | Terang | **Baru** |
| 31 | Grafik · Udara | Terang | **Baru** |

## Acuan

- `docs/SRS-SmartRoomMonitor-v1.0.md`
- `firebase/schema-v0.9.json`, `firebase/database.rules.json`
- Firmware `firmware/smartroom_esp32s3_firebase/`
- Diagram `docs/uml_revisi_puml/`

## Catatan

Prototipe ini adalah acuan visual untuk pembaruan manual di Figma; berkas Figma tidak diedit langsung. Untuk implementasi Android, token di `ui-spec.md` dipetakan ke Material 3 `colorScheme` dan `Typography` (Kotlin + Jetpack Compose, minSdk 26).
