// ==============================================================================
// 09_uji_integrasi_modular_ble.ino (FIRMWARE MODULAR MULTI-SENSOR + FAN + LIGHT + BLE)
// Proyek: SmartRoomMonitor (RPL) - Tim Pengembang ESP32-S3
// PIC Modul M3: Deswiryawan Saragih (241111008)
// Deskripsi: Versi modular de-bloat terlengkap dari seluruh subsistem:
//            - Sensor  : SHT30-D (M1), ENS160 (M2), HLK-LD2410 (M3), BH1750 (M4)
//            - Aktuator: Buzzer (M2/M3), Fan MOSFET (M1), Relay Lampu (M4)
//            - Wireless: Bluetooth Low Energy 5.0 Nordic UART Service (NUS)
//            Memenuhi 100% dari 8 Kebutuhan Fungsional (FR-01 s.d. FR-08) SRS v0.9!
// ==============================================================================

#include "config.h"
#include "BuzzerManager.h"
#include "SensorManager.h"
#include "FanManager.h"
#include "LightManager.h"
#include "BleManager.h"

// Instansiasi Seluruh Modul Manajer
BuzzerManager buzzer;
SensorManager sensors;
FanManager    fan;
LightManager  light;
BleManager    ble;

unsigned long lastTick = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1500);

  Serial.println(F("\n====================================================================="));
  Serial.println(F("  [SmartRoomMonitor] FIRMWARE MODULAR LENGKAP (ALL SENSORS & ACTUATORS)"));
  Serial.println(F("====================================================================="));

  // Inisialisasi masing-masing subsistem
  buzzer.begin();
  sensors.begin();
  fan.begin();
  light.begin();
  ble.begin(&buzzer, &sensors, &fan, &light);

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

    // A. Baca seluruh sensor dengan auto-kompensasi suhu
    TelemetriData data = sensors.bacaSemua();

    // B. Evaluasi histeresis termal kipas FR-02 (nyala > 31 °C, mati <= 30 °C)
    fan.evaluasiSuhu(data.suhu);

    // C. Evaluasi kontrol lampu adaptif FR-08 (nyala jika gelap < 50 Lux & ada orang)
    light.evaluasi(data.lux, data.adaOrang);

    // D. Evaluasi alarm buzzer berdasarkan kondisi udara & jam malam
    buzzer.evaluasiAlarm(data.aqi, data.adaOrang, ble.isJamMalam());

    // E. Broadcast hasil telemetri terpadu ke smartphone via BLE & Serial
    ble.kirimTelemetri(data);
  }
}
