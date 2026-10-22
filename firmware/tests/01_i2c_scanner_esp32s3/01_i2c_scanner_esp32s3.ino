// ==============================================================================
// 01_i2c_scanner_esp32s3.ino
// Proyek: SmartRoomMonitor (RPL)
// Deskripsi: Uji pemindaian bus I2C untuk mendeteksi alamat sensor hardware.
// Hardware: ESP32-S3 DevKitC-1 N16R8
// Pinout: SDA -> GPIO 4, SCL -> GPIO 5
// Hasil Uji: Ditemukan 0x38 (AHT21) & 0x53 (ENS160)
// ==============================================================================

#include <Wire.h>

#define I2C_SDA 4
#define I2C_SCL 5

void setup() {
  Serial.begin(115200);
  delay(2000); // Jeda inisialisasi serial monitor

  Serial.println("\n=======================================================");
  Serial.println("  [SmartRoomMonitor] 01 - I2C Scanner ESP32-S3");
  Serial.println("=======================================================");
  Serial.printf("Inisialisasi I2C: SDA=GPIO %d, SCL=GPIO %d...\n", I2C_SDA, I2C_SCL);
  
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("Memulai pemindaian perangkat I2C...\n");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  Serial.println("Sedang memindai bus I2C...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("  [OK] Perangkat ditemukan di alamat: 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);

      // Identifikasi komponen SmartRoomMonitor
      if (address == 0x38) {
        Serial.println(" -> Sensor AHT21 (Suhu & Kelembapan)");
      } else if (address == 0x52 || address == 0x53) {
        Serial.println(" -> Sensor ENS160 (Kualitas Udara AQI/TVOC/eCO2)");
      } else if (address == 0x44) {
        Serial.println(" -> Sensor SHT30-D (Suhu & Kelembapan Presisi)");
      } else if (address == 0x23) {
        Serial.println(" -> Sensor BH1750 (Intensitas Cahaya / Lux)");
      } else {
        Serial.println(" -> Perangkat I2C lainnya");
      }

      nDevices++;
    } else if (error == 4) {
      Serial.print("  [ERROR] Masalah komunikasi di alamat 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("  [!] Tidak ada perangkat I2C yang terdeteksi. Periksa kabel jumper!\n");
  } else {
    Serial.printf("Selesai. Total %d perangkat aktif ditemukan.\n\n", nDevices);
  }

  delay(5000);
}
