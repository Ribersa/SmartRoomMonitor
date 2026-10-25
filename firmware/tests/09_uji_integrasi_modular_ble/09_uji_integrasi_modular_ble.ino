// ==============================================================================
// 09_uji_integrasi_modular_ble.ino (FIRMWARE MODULAR MULTI-SENSOR + BLE)
// Proyek: SmartRoomMonitor (RPL) - Tim Pengembang ESP32-S3
// PIC Modul M3: Deswiryawan Saragih (241111008)
// Deskripsi: Versi modular de-bloat dari pengujian integrasi seluruh sensor
//            (SHT30-D, ENS160, BH1750, HLK-LD2410) + Buzzer + BLE NUS.
//            Struktur kode terpecah bersih ke dalam:
//              - config.h        : Pinout, baud, dan konstanta
//              - BuzzerManager   : Driver suara, nada alarm, & MUTE
//              - SensorManager   : Pembacaan 4 sensor & auto-kompensasi
//              - BleManager      : Bluetooth LE NUS & remote control HP
// ==============================================================================

#include "config.h"
#include "BuzzerManager.h"
#include "SensorManager.h"
#include "BleManager.h"

// Instansiasi Modul Manajer
BuzzerManager buzzer;
SensorManager sensors;
BleManager    ble;

unsigned long lastTick = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1500);

  Serial.println(F("\n======================================================="));
  Serial.println(F("  [SmartRoomMonitor] FIRMWARE MODULAR MULTI-SENSOR + BLE"));
  Serial.println(F("======================================================="));

  // Inisialisasi masing-masing subsistem
  buzzer.begin();
  sensors.begin();
  ble.begin(&buzzer, &sensors);

  // Sambut boot sukses & tampilkan menu
  buzzer.nadaSambutBoot();
  ble.tampilkanMenuBantuan();
}

void loop() {
  // 1. Update radar mikro non-blocking di tiap siklus loop
  sensors.updateRadar();

  // 2. Layani komunikasi & perintah nirkabel BLE / Serial
  ble.update();

  // 3. Siklus telemetri terjadwal (default tiap 2 detik, NFR-01)
  if (millis() - lastTick >= ble.getIntervalMs()) {
    lastTick = millis();

    // Baca seluruh sensor dengan auto-kompensasi suhu
    TelemetriData data = sensors.bacaSemua();

    // Evaluasi alarm buzzer berdasarkan kondisi udara & jam malam
    buzzer.evaluasiAlarm(data.aqi, data.adaOrang, ble.isJamMalam());

    // Broadcast hasil telemetri ke smartphone via BLE & laptop via Serial
    ble.kirimTelemetri(data);
  }
}
