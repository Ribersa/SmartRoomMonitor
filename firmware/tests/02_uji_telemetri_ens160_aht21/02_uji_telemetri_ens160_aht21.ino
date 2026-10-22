// ==============================================================================
// 02_uji_telemetri_ens160_aht21.ino
// Proyek: SmartRoomMonitor (RPL)
// Deskripsi: Uji pembacaan data riil suhu & kelembapan (AHT21) serta kualitas udara
//            AQI, TVOC, dan eCO2 (ENS160) lengkap dengan kompensasi lingkungan.
// Hardware: ESP32-S3 + Modul Combo ENS160 & AHT21
// Pinout: SDA -> GPIO 4, SCL -> GPIO 5, VCC -> 3V3, GND -> GND
// ==============================================================================

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <DFRobot_ENS160.h>

#define I2C_SDA 4
#define I2C_SCL 5

Adafruit_AHTX0 aht;
DFRobot_ENS160_I2C ens160(&Wire, 0x53); // Alamat hardware ENS160: 0x53

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=======================================================");
  Serial.println("  [SmartRoomMonitor] 02 - Uji Telemetri AHT21 + ENS160");
  Serial.println("=======================================================");

  Wire.begin(I2C_SDA, I2C_SCL);

  // Inisialisasi AHT21
  if (!aht.begin(&Wire)) {
    Serial.println("[ERROR] Gagal mendeteksi sensor AHT21! Cek kabel jumper.");
  } else {
    Serial.println("[OK] Sensor AHT21 (Suhu & Kelembapan) Siap!");
  }

  // Inisialisasi ENS160
  if (ens160.begin() != 0) {
    Serial.println("[ERROR] Gagal mendeteksi sensor ENS160! Cek kabel jumper.");
  } else {
    Serial.println("[OK] Sensor ENS160 (Kualitas Udara) Siap!");
    ens160.setPWRMode(ENS160_STANDARD_MODE);
  }

  Serial.println("\nMemulai pembacaan telemetri (interval 3 detik)...\n");
}

void loop() {
  // 1. Baca data suhu & kelembapan dari AHT21
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  float suhu = temp.temperature;
  float kelembapan = humidity.relative_humidity;

  // 2. Berikan data kompensasi suhu & kelembapan ke chip ENS160
  ens160.setTempAndHum(suhu, kelembapan);

  // 3. Baca data kualitas udara dari ENS160
  uint8_t status = ens160.getENS160Status();
  uint8_t aqi    = ens160.getAQI();    // 1: Sangat Baik ... 5: Sangat Buruk
  uint16_t tvoc  = ens160.getTVOC();   // ppb
  uint16_t eco2  = ens160.getECO2();   // ppm

  // 4. Tampilkan ke Serial Monitor
  Serial.println("-------------------------------------------------------");
  Serial.printf("🌡️  Suhu        : %.2f °C\n", suhu);
  Serial.printf("💧  Kelembapan  : %.2f %%\n", kelembapan);

  if (status == 1) {
    Serial.println("🍃  Kualitas Udara : [Status: Sedang Pemanasan (Warm-up)...]");
  } else {
    Serial.printf("🍃  AQI Index   : %d ", aqi);
    if (aqi == 1) Serial.println("(Sangat Baik / Bagus)");
    else if (aqi == 2) Serial.println("(Baik)");
    else if (aqi == 3) Serial.println("(Sedang)");
    else if (aqi == 4) Serial.println("(Buruk)");
    else Serial.println("(Sangat Buruk)");

    Serial.printf("    TVOC        : %d ppb\n", tvoc);
    Serial.printf("    eCO2        : %d ppm\n", eco2);
  }

  delay(3000); // Siklus sampling tiap 3 detik
}
