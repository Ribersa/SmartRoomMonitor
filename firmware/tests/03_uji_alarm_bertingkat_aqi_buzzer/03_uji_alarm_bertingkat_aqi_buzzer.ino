// ==============================================================================
// 03_uji_alarm_bertingkat_aqi_buzzer.ino
// Proyek: SmartRoomMonitor (RPL)
// Deskripsi: Sistem alarm bertingkat (Graduated Alarm) 5 Level AQI menggunakan
//            Buzzer Pasif. Level 1-3 senyap, Level 4 peringatan sedang (2200 Hz),
//            Level 5 sirine bahaya melengking maksimal (3500-4200 Hz).
// Hardware: ESP32-S3 + Modul ENS160/AHT21 + Modul Buzzer Pasif 3-Pin
// Pinout: Buzzer Signal -> GPIO 6, I2C SDA -> GPIO 4, I2C SCL -> GPIO 5
// ==============================================================================

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <DFRobot_ENS160.h>

#define I2C_SDA 4
#define I2C_SCL 5
#define BUZZER_PIN 6

Adafruit_AHTX0 aht;
DFRobot_ENS160_I2C ens160(&Wire, 0x53);

// Fungsi pembangkit frekuensi suara untuk Buzzer Pasif
void mainkanNada(int frekuensi, int durasiMs) {
  long periodeUs = 1000000L / frekuensi;
  long setengahPeriodeUs = periodeUs / 2;
  long totalSiklus = (long)frekuensi * durasiMs / 1000L;

  for (long i = 0; i < totalSiklus; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(setengahPeriodeUs);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(setengahPeriodeUs);
  }
}

// LEVEL 4: Peringatan Udara Buruk (Nada Bip Teratur 2.200 Hz)
void alarmLevel4_Peringatan() {
  for (int i = 0; i < 3; i++) {
    mainkanNada(2200, 120); // Nada sedang
    delay(100);
  }
}

// LEVEL 5: Bahaya Kritis (Sirine Melengking Kencang 3.500 Hz - 4.200 Hz)
void alarmLevel5_KritisMelengking() {
  for (int i = 0; i < 4; i++) {
    mainkanNada(3500, 80); // Frekuensi resonansi tinggi
    delay(30);
    mainkanNada(4200, 80); // Melengking tajam menusuk
    delay(30);
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("\n=======================================================");
  Serial.println("  [SmartRoomMonitor] 03 - Alarm Bertingkat 5-Level AQI");
  Serial.println("=======================================================");

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!aht.begin(&Wire)) {
    Serial.println("[ERROR] Sensor AHT21 tidak terdeteksi!");
  }
  if (ens160.begin() != 0) {
    Serial.println("[ERROR] Sensor ENS160 tidak terdeteksi!");
  } else {
    ens160.setPWRMode(ENS160_STANDARD_MODE);
  }

  // Nada sambut singkat saat booting: "Pip-Pip!"
  mainkanNada(2000, 70);
  delay(60);
  mainkanNada(2600, 100);

  Serial.println("[OK] Rangkaian Sensor + Buzzer Siap!\n");
}

void loop() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  float suhu = temp.temperature;
  float kelembapan = humidity.relative_humidity;

  // Kompensasi lingkungan ke ENS160
  ens160.setTempAndHum(suhu, kelembapan);

  uint8_t aqi    = ens160.getAQI();
  uint16_t tvoc  = ens160.getTVOC();
  uint16_t eco2  = ens160.getECO2();

  Serial.println("-------------------------------------------------------");
  Serial.printf("🌡️  Suhu: %.2f °C | 💧 Kelembapan: %.2f %%\n", suhu, kelembapan);
  Serial.printf("🍃  TVOC: %d ppb | eCO2: %d ppm | AQI Index: %d ", tvoc, eco2, aqi);

  // LOGIKA ALARM BERTINGKAT SESUAI AQI
  switch (aqi) {
    case 1:
      Serial.println("-> [LEVEL 1: Sangat Baik] 🔕 Senyap");
      digitalWrite(BUZZER_PIN, LOW);
      break;

    case 2:
      Serial.println("-> [LEVEL 2: Baik] 🔕 Senyap");
      digitalWrite(BUZZER_PIN, LOW);
      break;

    case 3:
      Serial.println("-> [LEVEL 3: Sedang] 🔕 Senyap");
      digitalWrite(BUZZER_PIN, LOW);
      break;

    case 4:
      Serial.println("-> [LEVEL 4: Buruk] ⚠️ BUNYI PERINGATAN (Sedang)");
      alarmLevel4_Peringatan();
      break;

    case 5:
      Serial.println("-> [LEVEL 5: Sangat Buruk] 🚨 ALARM KRITIS (Melengking Maksimal!)");
      alarmLevel5_KritisMelengking();
      break;

    default:
      digitalWrite(BUZZER_PIN, LOW);
      break;
  }

  delay(2000);
}
