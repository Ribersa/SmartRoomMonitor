// ==============================================================================
// 04_uji_telemetri_sht30_ens160_bh1750_buzzer.ino
// Proyek: SmartRoomMonitor (RPL)
// Deskripsi: Pengujian integrasi multi-sensor I2C (SHT30, ENS160, BH1750) dengan
//            sistem alarm bertingkat menggunakan Buzzer Pasif (Active-LOW).
// Hardware: ESP32-S3 DevKitC-1 N16R8
// Bus I2C: GPIO 4 (SDA), GPIO 5 (SCL)
//   - SHT30-D: 0x44 (Suhu presisi M1 & Kelembapan)
//   - ENS160: 0x53 (Kualitas Udara AQI, TVOC, eCO2)
//   - BH1750: 0x23 (Iluminansi Cahaya Ruangan M4)
// Aktuator: GPIO 6 (Buzzer Pasif Driver Transistor PNP - Active-LOW)
// ==============================================================================

#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <DFRobot_ENS160.h>
#include <BH1750.h>

#define I2C_SDA 4
#define I2C_SCL 5
#define BUZZER_PIN 6

// Inisialisasi Sensor-Sensor I2C
Adafruit_SHT31 sht30 = Adafruit_SHT31();  // Alamat: 0x44
DFRobot_ENS160_I2C ens160(&Wire, 0x53);   // Alamat: 0x53
BH1750 lightMeter;                         // Alamat: 0x23

// Fungsi pemutus arus Buzzer (Active-LOW: HIGH = Transistor PNP Mati Total)
void matikanBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
}

// Driver nada frekuensi Buzzer Pasif
void mainkanNada(int frekuensi, int durasiMs) {
  long periodeUs = 1000000L / frekuensi;
  long setengahPeriodeUs = periodeUs / 2;
  long totalSiklus = (long)frekuensi * durasiMs / 1000L;

  for (long i = 0; i < totalSiklus; i++) {
    digitalWrite(BUZZER_PIN, LOW);  // Transistor ON (tarik membran)
    delayMicroseconds(setengahPeriodeUs);
    digitalWrite(BUZZER_PIN, HIGH); // Transistor OFF (lepas membran)
    delayMicroseconds(setengahPeriodeUs);
  }
  matikanBuzzer();
}

// LEVEL 4: Peringatan Udara Buruk (Nada Bip Teratur 2.200 Hz)
void alarmLevel4_Peringatan() {
  for (int i = 0; i < 3; i++) {
    mainkanNada(2200, 120);
    delay(100);
  }
  matikanBuzzer();
}

// LEVEL 5: Bahaya Kritis (Sirine Melengking Kencang 3.500 Hz - 4.200 Hz)
void alarmLevel5_KritisMelengking() {
  for (int i = 0; i < 4; i++) {
    mainkanNada(3500, 80);
    delay(30);
    mainkanNada(4200, 80);
    delay(30);
  }
  matikanBuzzer();
}

void setup() {
  Serial.begin(115200);

  // Kunci buzzer mati total saat awal boot
  pinMode(BUZZER_PIN, OUTPUT);
  matikanBuzzer();

  delay(2000);

  Serial.println("\n=======================================================");
  Serial.println("  [SmartRoomMonitor] SHT30 + ENS160 + BH1750 + Buzzer");
  Serial.println("=======================================================");

  // Inisialisasi Bus I2C bersama (GPIO 4 SDA, GPIO 5 SCL)
  Wire.begin(I2C_SDA, I2C_SCL);

  // 1. Inisialisasi SHT30 (0x44)
  if (!sht30.begin(0x44)) {
    Serial.println("[ERROR] SHT30 tidak terdeteksi di 0x44!");
  } else {
    Serial.println("[OK] SHT30 (Suhu & Kelembapan) Siap!");
  }

  // 2. Inisialisasi ENS160 (0x53)
  if (ens160.begin() != 0) {
    Serial.println("[ERROR] ENS160 tidak terdeteksi di 0x53!");
  } else {
    ens160.setPWRMode(ENS160_STANDARD_MODE);
    Serial.println("[OK] ENS160 (Kualitas Udara) Siap!");
  }

  // 3. Inisialisasi BH1750 (0x23)
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("[ERROR] BH1750 tidak terdeteksi di 0x23!");
  } else {
    Serial.println("[OK] BH1750 (Sensor Cahaya Ambient) Siap!");
  }

  // Nada sambut singkat tanda boot sukses
  mainkanNada(2000, 70);
  delay(60);
  mainkanNada(2600, 100);
  matikanBuzzer();

  Serial.println("[OK] Semua Sensor Siap Membaca Data!\n");
}

void loop() {
  // 1. Baca data SHT30
  float suhu = sht30.readTemperature();
  float kelembapan = sht30.readHumidity();

  if (!isnan(suhu) && !isnan(kelembapan)) {
    ens160.setTempAndHum(suhu, kelembapan); // Umpankan ke ENS160 untuk kompensasi gas MOX
  }

  // 2. Baca data ENS160
  uint8_t aqi    = ens160.getAQI();
  uint16_t tvoc  = ens160.getTVOC();
  uint16_t eco2  = ens160.getECO2();

  // 3. Baca intensitas cahaya dari BH1750
  float lux = lightMeter.readLightLevel();

  // Tampilkan Telemetri
  Serial.println("------------------------------------------------------------------");
  if (!isnan(suhu) && !isnan(kelembapan)) {
    Serial.printf("🌡️  Suhu: %.2f °C | 💧 Kelembapan: %.2f %%\n", suhu, kelembapan);
  } else {
    Serial.println("🌡️  SHT30: Gagal membaca");
  }

  if (lux >= 0) {
    Serial.printf("💡  Iluminansi: %.2f lux ", lux);
    if (lux < 50.0) {
      Serial.println("(🌙 Gelap / Remang)");
    } else if (lux < 250.0) {
      Serial.println("(💡 Pencahayaan Ruang Normal)");
    } else {
      Serial.println("(☀️ Terang Benderang)");
    }
  } else {
    Serial.println("💡  BH1750: Gagal membaca lux");
  }

  Serial.printf("🍃  TVOC: %d ppb | eCO2: %d ppm | AQI Index: %d ", tvoc, eco2, aqi);

  // Logika 5 Tingkat Alarm AQI
  switch (aqi) {
    case 1:
      Serial.println("-> [LEVEL 1: Sangat Baik] 🔕");
      matikanBuzzer();
      break;
    case 2:
      Serial.println("-> [LEVEL 2: Baik] 🔕");
      matikanBuzzer();
      break;
    case 3:
      Serial.println("-> [LEVEL 3: Sedang] 🔕");
      matikanBuzzer();
      break;
    case 4:
      Serial.println("-> [LEVEL 4: Buruk] ⚠️ BUNYI PERINGATAN");
      alarmLevel4_Peringatan();
      break;
    case 5:
      Serial.println("-> [LEVEL 5: Sangat Buruk] 🚨 ALARM KRITIS");
      alarmLevel5_KritisMelengking();
      break;
    default:
      matikanBuzzer();
      break;
  }

  delay(2000);
}
