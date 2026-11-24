# Build firmware produksi SmartRoomMonitor dengan arduino-cli.
# Usage: powershell -File tools\build-firmware.ps1
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$cli = Join-Path $root "arduino-cli.exe"
$sketch = Join-Path $root "firmware\smartroom_esp32s3_firebase"
$fqbn = "esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,CDCOnBoot=cdc"

if (-not (Test-Path $cli)) { throw "arduino-cli.exe tidak ditemukan di $root" }
if (-not (Test-Path (Join-Path $sketch "secrets.h"))) {
    throw "secrets.h belum ada. Salin secrets.h.example menjadi secrets.h di $sketch"
}

& $cli compile --fqbn $fqbn --warnings default $sketch
if ($LASTEXITCODE -ne 0) { throw "Compile gagal (exit $LASTEXITCODE)" }
"OK: firmware ter-compile"
