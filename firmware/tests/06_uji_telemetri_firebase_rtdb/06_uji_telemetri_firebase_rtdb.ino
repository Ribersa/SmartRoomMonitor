// ==============================================================================
// 06_uji_telemetri_firebase_rtdb.ino
// Proyek: SmartRoomMonitor (RPL)
// Deskripsi: Pengiriman data multi-sensor (SHT30, ENS160, BH1750) secara nirkabel
//            langsung ke Firebase Realtime Database (RTDB) via REST API HTTPS.
// Hardware: ESP32-S3 DevKitC-1 N16R8
// ==============================================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <DFRobot_ENS160.h>
#include <BH1750.h>

// -----------------------------------------------------------------------------
// KONFIGURASI JARINGAN & CLOUD
// (Ganti placeholder di bawah ini langsung di Arduino IDE laptopmu)
// -----------------------------------------------------------------------------
#define WIFI_SSID     "NAMA_WIFI_ATAU_HOTSPOT"
#define WIFI_PASSWORD "PASSWORD_WIFI_KAMU"

// URL Firebase Realtime Database milik proyekmu (pastikan berakhiran .app tanpa slash '/')
#define FIREBASE_HOST "https://GANTI-PROJECT-ID-default-rtdb.asia-southeast1.firebasedatabase.app"

// Pin Hardware
#define I2C_SDA 4
#define I2C_SCL 5
#define BUZZER_PIN 6

Adafruit_SHT31 sht30 = Adafruit_SHT31();
DFRobot_ENS160_I2C ens160(&Wire, 0x53);
BH1750 lightMeter;

void matikanBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
}

void mainkanNada(int frekuensi, int durasiMs) {
  long periodeUs = 1000000L / frekuensi;
  long setengahPeriodeUs = periodeUs / 2;
  long totalSiklus = (long)frekuensi * durasiMs / 1000L;

  for (long i = 0; i < totalSiklus; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(setengahPeriodeUs);
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(setengahPeriodeUs);
  }
  matikanBuzzer();
}

void hubungkanWiFi() {
  Serial.print("[WiFi] Menghubungkan ke ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int percobaan = 0;
  while (WiFi.status() != WL_CONNECTED && percobaan < 25) {
    delay(500);
    Serial.print(".");
    percobaan++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Berhasil Terhubung!");
    Serial.print("[WiFi] Alamat IP ESP32: ");
    Serial.println(WiFi.localIP());
    // Nada sambut singkat
    mainkanNada(2000, 60);
    delay(50);
    mainkanNada(2600, 80);
    matikanBuzzer();
  } else {
    Serial.println("\n[WiFi] Gagal terhubung! Periksa SSID & Password Hotspot.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  matikanBuzzer();

  delay(2000);
  Serial.println("\n=======================================================");
  Serial.println("  [SmartRoomMonitor] ESP32-S3 -> Firebase RTDB Stream");
  Serial.println("=======================================================");

  // Inisialisasi Bus I2C Sensor
  Wire.begin(I2C_SDA, I2C_SCL);
  sht30.begin(0x44);
  ens160.begin();
  ens160.setPWRMode(ENS160_STANDARD_MODE);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  // Sambungkan ke Jaringan Internet
  hubungkanWiFi();
}

void loop() {
  // Jika Wi-Fi putus, coba rekoneksi otomatis
  if (WiFi.status() != WL_CONNECTED) {
    hubungkanWiFi();
    delay(1000);
    return;
  }

  // 1. Baca Data Sensor Fisik
  float suhu = sht30.readTemperature();
  float kelembapan = sht30.readHumidity();

  if (!isnan(suhu) && !isnan(kelembapan)) {
    ens160.setTempAndHum(suhu, kelembapan);
  }

  uint8_t aqi   = ens160.getAQI();
  uint16_t tvoc = ens160.getTVOC();
  uint16_t eco2 = ens160.getECO2();
  float lux     = lightMeter.readLightLevel();

  // 2. Logika Pengaman Buzzer (tetap aktif di background)
  if (aqi >= 5) {
    // Alarm jika udara sangat buruk
    for (int i = 0; i < 2; i++) {
      mainkanNada(3500, 60);
      mainkanNada(4200, 60);
    }
  } else {
    matikanBuzzer();
  }

  // 3. Format Data JSON untuk Firebase RTDB
  String jsonPayload = "{";
  jsonPayload += "\"suhu\":" + (isnan(suhu) ? "0.0" : String(suhu, 2)) + ",";
  jsonPayload += "\"kelembapan\":" + (isnan(kelembapan) ? "0.0" : String(kelembapan, 2)) + ",";
  jsonPayload += "\"cahaya_lux\":" + (lux < 0 ? "0.0" : String(lux, 2)) + ",";
  jsonPayload += "\"aqi\":" + String(aqi) + ",";
  jsonPayload += "\"tvoc_ppb\":" + String(tvoc) + ",";
  jsonPayload += "\"eco2_ppm\":" + String(eco2) + ",";
  jsonPayload += "\"status_alat\":\"ONLINE\",";
  jsonPayload += "\"waktu_boot_ms\":" + String(millis());
  jsonPayload += "}";

  // 4. Kirim ke Firebase REST API via HTTPS PUT (Node: /ruangan_1/telemetri.json)
  WiFiClientSecure client;
  client.setInsecure(); // Mengabaikan validasi sertifikat SSL agar hemat memori RAM

  HTTPClient http;
  String fullUrl = String(FIREBASE_HOST) + "/ruangan_1/telemetri.json";

  http.begin(client, fullUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.PUT(jsonPayload);

  Serial.println("------------------------------------------------------------------");
  if (httpResponseCode == 200) {
    Serial.println("☁️ [FIREBASE SUKSES] Data Terkirim ke Cloud (HTTP 200 OK)!");
    Serial.printf("📊 Payload: %s\n", jsonPayload.c_str());
  } else {
    Serial.printf("❌ [FIREBASE ERROR] Gagal kirim! Kode HTTP: %d\n", httpResponseCode);
    if (httpResponseCode == 401 || httpResponseCode == 403) {
      Serial.println("⚠️  Penyebab: Tab 'Rules' di Realtime Database belum bernilai true.");
    }
  }

  http.end();

  // Kirim data setiap 3 detik sekali
  delay(3000);
}
