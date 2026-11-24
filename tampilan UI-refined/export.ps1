# Render 28 UI screens to PNG via Chrome headless.
# Usage: powershell -File export.ps1
$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$proto = Join-Path $root "prototype"
$out = Join-Path $root "png"
$tmp = "C:\Users\freec\AppData\Local\Temp\opencode\chrome-ui-export"
$scratch = "C:\Users\freec\AppData\Local\Temp\opencode\ui-scratch"
New-Item -ItemType Directory -Force -Path $out, $tmp, $scratch | Out-Null

$chrome = "C:\Program Files\Google\Chrome\Application\chrome.exe"
if (-not (Test-Path $chrome)) { throw "Chrome tidak ditemukan: $chrome" }

$names = @{
  "01" = "01 Masuk"; "02" = "02 Masuk - Kredensial salah"; "03" = "03 Masuk - Memuat sesi";
  "04" = "04 Dashboard - Staff - Normal"; "05" = "05 Dashboard - Pengunjung"; "06" = "06 Dashboard - Offline";
  "07" = "07 Dashboard - Alarm aktif"; "08" = "08 Dashboard - Alarm dibungkam"; "09" = "09 Dashboard - Memuat";
  "10" = "10 Grafik - Ada data"; "11" = "11 Grafik - Belum ada data";
  "12" = "12 Kejadian - Daftar"; "13" = "13 Kejadian - Filter per tipe"; "14" = "14 Kejadian - Kosong";
  "15" = "15 Kontrol - Staff - Normal"; "16" = "16 Kontrol - Pengunjung - Ditolak"; "17" = "17 Kontrol - Offline";
  "18" = "18 Profil - Staff"; "19" = "19 Profil - Pengunjung";
  "20" = "20 Dashboard - Alarm - Pengunjung"; "21" = "21 Dashboard - Alarm - Offline";
  "22" = "22 Kontrol - Berhasil"; "23" = "23 Konfirmasi keluar"; "24" = "24 Grafik - Data parsial";
  "25" = "25 Dashboard - Gelap"; "26" = "26 Kontrol - Gelap"; "27" = "27 Grafik - Gelap"; "28" = "28 Profil - Gelap";
}

$ids = @("01","02","03","04","05","06","07","08","09","10","11","12","13","14",
         "15","16","17","18","19","20","21","22","23","24","25","26","27","28")

$base = ((Join-Path $proto "index.html") -replace '\\','/') -replace ' ','%20'

foreach ($id in $ids) {
  $url = "file:///$base" + "?s=$id&export=1"
  # screenshot ke path TANPA spasi, lalu rename ke nama final
  $scratchFile = Join-Path $scratch ("screen-$id.png")
  $target = Join-Path $out ($names[$id] + ".png")
  $args = @(
    "--headless=new", "--disable-gpu", "--no-sandbox",
    "--user-data-dir=$tmp", "--hide-scrollbars",
    "--window-size=390,844", "--force-device-scale-factor=2",
    "--virtual-time-budget=2500", "--screenshot=$scratchFile", $url
  )
  $p = Start-Process -FilePath $chrome -ArgumentList $args -PassThru -Wait -NoNewWindow
  if (Test-Path $scratchFile) {
    Move-Item -Force $scratchFile $target
    "OK   $($names[$id])"
  } else { "FAIL $id (exit $($p.ExitCode))" }
}

""
"Total: " + (Get-ChildItem $out -Filter *.png | Where-Object { $_.Name -notlike '_*' }).Count + " PNG di $out"
