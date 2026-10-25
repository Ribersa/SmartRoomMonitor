// ==============================================================================
// 08_uji_integrasi_semua_sensor_ble.ino
// Proyek: SmartRoomMonitor (RPL) - Tim Pengembang ESP32-S3
// PIC Modul M3: Deswiryawan Saragih (241111008)
// Deskripsi: Pengujian integrasi penuh SELURUH SUBSISTEM SENSOR (SHT30-D, ENS160,
//            BH1750, HLK-LD2410) bersama Buzzer dan Telemetri BLE 2-Arah (NUS).
// Hardware & Pinout:
//   - I2C Bus: GPIO 4 (SDA), GPIO 5 (SCL) -> Tegangan 3.3V
//       * SHT30-D : 0x44 (Suhu Presisi M1 & Kelembapan Relatif)
//       * ENS160  : 0x53 (Kualitas Udara M2: UBA AQI, TVOC ppb, eCO2 ppm)
//       * BH1750  : 0x23 (Sensor Cahaya Ambient M4: Iluminansi Lux)
//   - UART1: GPIO 18 (RX), GPIO 17 (TX) @ 256.000 baud
//       * HLK-LD2410 : mmWave Radar 24 GHz M3 (Deteksi Gerak & Napas Mikro)
//       * Daya Radar : Jalur Khusus 5V DC + Common GND ke ESP32
//   - Aktuator: GPIO 6
//       * Buzzer Pasif Driver Transistor PNP (Active-LOW: HIGH=Mati, LOW=Siklus)
//   - Komunikasi Nirkabel:
//       * Bluetooth Low Energy (BLE 5.0) Nordic UART Service (NUS)
//       * Nama Device: "SmartRoom-ESP32" (Kompatibel Serial Bluetooth Terminal HP)
// ==============================================================================

#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <DFRobot_ENS160.h>
#include <BH1750.h>
#include <ld2410.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- Definisi Pinout & Parameter ---
#define I2C_SDA_PIN   4
#define I2C_SCL_PIN   5
#define RADAR_RX_PIN  18
#define RADAR_TX_PIN  17
#define RADAR_BAUD    256000
#define BUZZER_PIN    6

// UUID Standar Nordic Semiconductor UART Service (NUS)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// --- Instansiasi Sensor & Radar ---
Adafruit_SHT31 sht30 = Adafruit_SHT31();
DFRobot_ENS160_I2C ens160(&Wire, 0x53);
BH1750 lightMeter;
ld2410 radar;

// --- State BLE & Loop ---
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
bool bleConnected = false;
bool oldBleConnected = false;
volatile char pendingBleCmd = 0;

unsigned long lastPrintTime = 0;
unsigned long printIntervalMs = 2000; // Standar 2 detik (sesuai batas SRS NFR-01 <= 3s)

// --- State Fungsional SmartRoom ---
bool modeJamMalam = false; // Simulasi jam 22:00 - 06:00 (FR-05)
bool alarmMuted   = false; // MUTE darurat buzzer (FR-06)
bool sht30Ok      = false;
bool ens160Ok     = false;
bool bh1750Ok     = false;
bool radarOk      = false;

// Driver pemutus arus Buzzer (Active-LOW: HIGH = Transistor PNP Mati Total)
void matikanBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
}

// Driver pembuat gelombang frekuensi Buzzer Pasif
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

// Peringatan AQI Level 4: Udara Buruk (Nada Bip Teratur 2.200 Hz)
void alarmAQI_Level4() {
  for (int i = 0; i < 2; i++) {
    mainkanNada(2200, 100);
    delay(60);
  }
  matikanBuzzer();
}

// Peringatan AQI Level 5: Bahaya Kritis (Sirine Melengking 3.500 Hz - 4.200 Hz)
void alarmAQI_Level5() {
  for (int i = 0; i < 3; i++) {
    mainkanNada(3500, 70);
    delay(20);
    mainkanNada(4200, 70);
    delay(20);
  }
  matikanBuzzer();
}

// Peringatan Jam Malam FR-05: Nada Interupsi Khusus
void alarmJamMalam() {
  mainkanNada(2800, 90);
  delay(40);
  mainkanNada(3400, 90);
  matikanBuzzer();
}

// Fungsi bantu broadcast teks ke USB Serial dan BLE HP
void kirimPesan(const char* teks) {
  Serial.print(teks);
  if (bleConnected && pTxCharacteristic != NULL) {
    pTxCharacteristic->setValue((uint8_t*)teks, strlen(teks));
    pTxCharacteristic->notify();
    delay(5); // Jeda mikro agar buffer notifikasi BLE stack tertransmisi lancar
  }
}

// Callback Koneksi BLE Server
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      bleConnected = true;
      Serial.println("\n📲 [BLE] Smartphone Terhubung! Telemetri Nirkabel Semua Sensor Aktif.");
    };
    void onDisconnect(BLEServer* pServer) {
      bleConnected = false;
      Serial.println("\n📲 [BLE] Smartphone Terputus. Kembali mengiklankan sinyal 'SmartRoom-ESP32'...");
    }
};

// Callback Penerima Perintah Ketikan dari HP (RX Characteristic)
class MyRxCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        pendingBleCmd = rxValue[0];
      }
    }
};

void cetakBarEnergi(uint8_t energi, char* bufferOut) {
  int terisi = energi / 10;
  if (terisi > 10) terisi = 10;
  strcpy(bufferOut, "[");
  for (int i = 0; i < 10; i++) {
    strcat(bufferOut, (i < terisi) ? "#" : ".");
  }
  strcat(bufferOut, "]");
}

void tampilkanMenuBantuan() {
  kirimPesan("\r\n================ PUSAT KONTROL SMART ROOM (USB & BLE) ================\r\n");
  kirimPesan("  [1] Set Jarak Radar: Meja Dekat   (~1.50m / Gate 2, Peka Napas)\r\n");
  kirimPesan("  [2] Set Jarak Radar: Area Kasur   (~2.25m / Gate 3, Peka Napas)\r\n");
  kirimPesan("  [3] Set Jarak Radar: Kamar Penuh  (~4.50m / Gate 6, Peka Napas)\r\n");
  kirimPesan("  [4] Set Jarak Radar: Jarak Maks   (~6.00m / Gate 8, Peka Napas)\r\n");
  kirimPesan("  [s] Boost Sensitivitas Napas/Diam (Threshold = 15%)\r\n");
  kirimPesan("  [m] Toggle Simulasi JAM MALAM 22:00-06:00 (FR-05 Alarm Radar)\r\n");
  kirimPesan("  [x] Tekan MUTE DARURAT (FR-06 Bungkam Semua Alarm Buzzer)\r\n");
  kirimPesan("  [t] Toggle Interval Telemetri (1 dtk vs 2 dtk)\r\n");
  kirimPesan("  [i] Tampilkan Info Konfigurasi Radar & Status Sensor\r\n");
  kirimPesan("  [h] Tampilkan Menu Bantuan Ini\r\n");
  kirimPesan("======================================================================\r\n\n");
}

void optimalkanSensitivitasNapas() {
  kirimPesan("🫁 [KALIBRASI] Menyetel threshold diam radar ke 15% agar sangat peka napas...\r\n");
  radar.setGateSensitivityThreshold(0, 30, 20);
  radar.setGateSensitivityThreshold(1, 30, 20);
  radar.setGateSensitivityThreshold(2, 30, 15);
  radar.setGateSensitivityThreshold(3, 35, 15);
  radar.setGateSensitivityThreshold(4, 40, 20);
  radar.setGateSensitivityThreshold(5, 40, 20);
  radar.setGateSensitivityThreshold(6, 45, 25);
  kirimPesan("✅ [SIAP] Sensitivitas napas mikro berhasil dioptimalkan!\r\n\n");
}

void aturBatasJarakGate(uint8_t gateMaks, uint16_t timeoutDetik, const char* labelMode) {
  uint8_t gateFix = (gateMaks < 2) ? 2 : gateMaks;
  char buf[128];
  snprintf(buf, sizeof(buf), "\r\n⚙️ [RADAR] Mengatur jarak deteksi ke %s (Gate %d, Timeout %ds)...\r\n",
           labelMode, gateFix, timeoutDetik);
  kirimPesan(buf);

  if (radar.setMaxValues(gateFix, gateFix, timeoutDetik)) {
    kirimPesan("✅ [SUKSES] Batas jarak radar berhasil diterapkan!\r\n\n");
    mainkanNada(2400, 60);
  } else {
    kirimPesan("⚠️ [GAGAL] Radar tidak merespons perintah konfigurasi.\r\n\n");
  }
}

void bacaInfoSistem() {
  char buf[160];
  kirimPesan("\r\n🔍 [STATUS DIAGNOSTIK HARDWARE]\r\n");
  snprintf(buf, sizeof(buf), "   - SHT30-D (0x44) : %s\r\n", sht30Ok ? "TERSEDIA (Normal)" : "GAGAL");
  kirimPesan(buf);
  snprintf(buf, sizeof(buf), "   - ENS160  (0x53) : %s\r\n", ens160Ok ? "TERSEDIA (Standard Mode)" : "GAGAL");
  kirimPesan(buf);
  snprintf(buf, sizeof(buf), "   - BH1750  (0x23) : %s\r\n", bh1750Ok ? "TERSEDIA (Continuous H-Res)" : "GAGAL");
  kirimPesan(buf);
  snprintf(buf, sizeof(buf), "   - HLK-LD2410 UART: %s\r\n", radar.isConnected() ? "TERHUBUNG (256000 baud)" : "BELUM RESPONS");
  kirimPesan(buf);

  if (radar.requestFirmwareVersion()) {
    snprintf(buf, sizeof(buf), "     * Firmware Radar: v%d.%02d.%08X\r\n",
             radar.firmware_major_version,
             radar.firmware_minor_version,
             radar.firmware_bugfix_version);
    kirimPesan(buf);
  }
  if (radar.requestCurrentConfiguration()) {
    snprintf(buf, sizeof(buf), "     * Jangkauan Radar: Gerak Gate %d (~%.2fm) | Diam Gate %d (~%.2fm)\r\n",
             radar.max_moving_gate, (radar.max_moving_gate + 1) * 0.75,
             radar.max_stationary_gate, (radar.max_stationary_gate + 1) * 0.75);
    kirimPesan(buf);
  }
  snprintf(buf, sizeof(buf), "   - BLE NUS Service: AKTIF (Device: 'SmartRoom-ESP32' | Status: %s)\r\n\n",
           bleConnected ? "TERKONEKSI HP" : "STANDBY SCAN");
  kirimPesan(buf);
}

bool inisialisasiRadarAutoSwap() {
  Serial.print("🔄 [UART] Mencoba inisialisasi radar (RX=GPIO 18, TX=GPIO 17)... ");
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  radar.begin(Serial1, false);

  unsigned long tStart = millis();
  while (millis() - tStart < 2000) {
    radar.read();
    if (radar.isConnected()) {
      Serial.println("TERHUBUNG!");
      return true;
    }
    delay(10);
  }
  Serial.println("Belum ada respons.");

  Serial.print("🔄 [AUTO-SWAP] Mencoba membalik pin UART (RX=GPIO 17, TX=GPIO 18)... ");
  Serial1.end();
  delay(100);
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, RADAR_TX_PIN, RADAR_RX_PIN);
  radar.begin(Serial1, false);

  tStart = millis();
  while (millis() - tStart < 2000) {
    radar.read();
    if (radar.isConnected()) {
      Serial.println("TERHUBUNG! (Pin otomatis disesuaikan)");
      return true;
    }
    delay(10);
  }
  Serial.println("Gagal.");

  // Kembalikan ke konfigurasi default
  Serial1.end();
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  radar.begin(Serial1, false);
  return false;
}

void inisialisasiBLE() {
  BLEDevice::init("SmartRoom-ESP32");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyRxCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("📡 [BLE] Nordic UART Aktif! Nama: 'SmartRoom-ESP32' (Siap dipasangkan)");
}

void eksekusiPerintah(char cmd) {
  char buf[128];
  switch (cmd) {
    case '1': aturBatasJarakGate(2, 5, "Meja Dekat (~1.50m / Gate 2)"); break;
    case '2': aturBatasJarakGate(3, 5, "Area Kasur (~2.25m / Gate 3)"); break;
    case '3': aturBatasJarakGate(6, 5, "Kamar Penuh (~4.50m / Gate 6)"); break;
    case '4': aturBatasJarakGate(8, 5, "Jarak Maksimal (~6.00m / Gate 8)"); break;
    case 's':
    case 'S':
      optimalkanSensitivitasNapas();
      mainkanNada(2600, 70);
      break;
    case 't':
    case 'T':
      printIntervalMs = (printIntervalMs == 2000) ? 1000 : 2000;
      snprintf(buf, sizeof(buf), "\r\n⏱️ [INTERVAL] Kecepatan telemetri diubah ke tiap %lu detik!\r\n\n", printIntervalMs / 1000);
      kirimPesan(buf);
      break;
    case 'm':
    case 'M':
      modeJamMalam = !modeJamMalam;
      alarmMuted = false;
      snprintf(buf, sizeof(buf), "\r\n🌙 [JAM MALAM FR-05]: %s\r\n\n",
               modeJamMalam ? "AKTIF (Alarm berbunyi jika ada gerakan/keberadaan!)" : "NONAKTIF (Mode Normal)");
      kirimPesan(buf);
      mainkanNada(modeJamMalam ? 3000 : 1800, 80);
      break;
    case 'x':
    case 'X':
      alarmMuted = true;
      matikanBuzzer();
      kirimPesan("\r\n🔕 [MUTE DARURAT FR-06] Seluruh alarm buzzer dibungkam seketika!\r\n\n");
      break;
    case 'i':
    case 'I': bacaInfoSistem(); break;
    case 'h':
    case 'H': tampilkanMenuBantuan(); break;
  }
}

void prosesInputSemuaKanal() {
  if (pendingBleCmd != 0) {
    char c = pendingBleCmd;
    pendingBleCmd = 0;
    eksekusiPerintah(c);
  }
  if (Serial.available()) {
    char cmd = Serial.read();
    while (Serial.available()) {
      char c = Serial.peek();
      if (c == '\r' || c == '\n') Serial.read();
      else break;
    }
    eksekusiPerintah(cmd);
  }
}

void setup() {
  Serial.begin(115200);

  // Amankan pin Buzzer terlebih dahulu
  pinMode(BUZZER_PIN, OUTPUT);
  matikanBuzzer();

  delay(2000);

  Serial.println("\n=====================================================================");
  Serial.println("  [SmartRoomMonitor] PENGUJIAN INTEGRASI SELURUH SENSOR + BLE NUS");
  Serial.println("=====================================================================");

  // 1. Inisialisasi Bus I2C Bersama (GPIO 4 SDA, GPIO 5 SCL)
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // Inisialisasi SHT30-D (0x44)
  if (sht30.begin(0x44)) {
    sht30Ok = true;
    Serial.println("✅ [OK] SHT30-D (Suhu & RH) Siap!");
  } else {
    Serial.println("❌ [GAGAL] SHT30-D tidak terdeteksi pada 0x44!");
  }

  // Inisialisasi ENS160 (0x53)
  if (ens160.begin() == 0) {
    ens160.setPWRMode(ENS160_STANDARD_MODE);
    ens160Ok = true;
    Serial.println("✅ [OK] ENS160 (Kualitas Udara AQI/TVOC/eCO2) Siap!");
  } else {
    Serial.println("❌ [GAGAL] ENS160 tidak terdeteksi pada 0x53!");
  }

  // Inisialisasi BH1750 (0x23)
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    bh1750Ok = true;
    Serial.println("✅ [OK] BH1750 (Iluminansi Cahaya) Siap!");
  } else {
    Serial.println("❌ [GAGAL] BH1750 tidak terdeteksi pada 0x23!");
  }

  // 2. Inisialisasi Bluetooth Low Energy (NUS)
  inisialisasiBLE();

  // 3. Inisialisasi Radar HLK-LD2410
  radarOk = inisialisasiRadarAutoSwap();
  if (radarOk) {
    aturBatasJarakGate(2, 5, "Meja Dekat (~1.50m / Gate 2)");
    optimalkanSensitivitasNapas();
  } else {
    Serial.println("⚠️ [PERINGATAN] Radar belum merespons. Pastikan kabel daya 5V & Common GND terpasang!");
  }

  // Nada sambut boot sukses
  mainkanNada(2000, 60);
  delay(40);
  mainkanNada(2800, 80);
  matikanBuzzer();

  tampilkanMenuBantuan();
}

void loop() {
  // 1. Polling radar wajib dieksekusi tanpa blocking di setiap iterasi loop
  radar.read();

  // 2. Layani perintah interaktif dari Bluetooth HP dan USB Serial
  prosesInputSemuaKanal();

  // 3. Manajemen rekoneksi otomatis BLE advertising
  if (!bleConnected && oldBleConnected) {
    delay(300);
    pServer->startAdvertising();
    oldBleConnected = bleConnected;
  }
  if (bleConnected && !oldBleConnected) {
    oldBleConnected = bleConnected;
  }

  // 4. Siklus Pengambilan Telemetri Terjadwal (default tiap 2 detik)
  if (millis() - lastPrintTime >= printIntervalMs) {
    lastPrintTime = millis();

    // A. Pembacaan SHT30-D
    float suhu = NAN;
    float rh   = NAN;
    if (sht30Ok) {
      suhu = sht30.readTemperature();
      rh   = sht30.readHumidity();
      // Umpankan ke ENS160 untuk kompensasi resistansi gas MOX secara real-time
      if (!isnan(suhu) && !isnan(rh) && ens160Ok) {
        ens160.setTempAndHum(suhu, rh);
      }
    }

    // B. Pembacaan ENS160
    uint8_t  aqi  = 0;
    uint16_t tvoc = 0;
    uint16_t eco2 = 0;
    if (ens160Ok) {
      aqi  = ens160.getAQI();
      tvoc = ens160.getTVOC();
      eco2 = ens160.getECO2();
    }

    // C. Pembacaan BH1750
    float lux = -1.0;
    if (bh1750Ok) {
      lux = lightMeter.readLightLevel();
    }

    // D. Evaluasi Data Radar LD2410
    bool adaOrang = false;
    bool bergerak = false;
    bool diam     = false;
    uint16_t jarakGerak  = 0;
    uint8_t  energiGerak = 0;
    uint16_t jarakDiam   = 0;
    uint8_t  energiDiam  = 0;

    char barGerak[16] = "[..........]";
    char barDiam[16]  = "[..........]";
    const char* labelRadar = "KOSONG";

    if (radar.isConnected()) {
      adaOrang = radar.presenceDetected();
      bergerak = radar.movingTargetDetected();
      diam     = radar.stationaryTargetDetected();

      jarakGerak  = radar.movingTargetDistance();
      energiGerak = radar.movingTargetEnergy();
      jarakDiam   = radar.stationaryTargetDistance();
      energiDiam  = radar.stationaryTargetEnergy();

      cetakBarEnergi(energiGerak, barGerak);
      cetakBarEnergi(energiDiam, barDiam);

      if (adaOrang) {
        if (bergerak) labelRadar = "BERGERAK";
        else if (diam) labelRadar = "DIAM";
      } else {
        alarmMuted = false; // Reset mute saat ruangan kembali kosong
      }
    } else {
      labelRadar = "DISCONNECT";
    }

    // E. Format dan Broadcast Data Telemetri Terpadu
    char baris[160];
    kirimPesan("\r\n================ TELEMETRI SMART ROOM ================\r\n");

    // Baris 1: Suhu & Kelembapan (M1)
    if (!isnan(suhu) && !isnan(rh)) {
      snprintf(baris, sizeof(baris), "🌡️  Suhu: %.2f °C | 💧 Kelembapan: %.2f %%\r\n", suhu, rh);
    } else {
      snprintf(baris, sizeof(baris), "🌡️  SHT30: Sensor belum siap / gagal baca\r\n");
    }
    kirimPesan(baris);

    // Baris 2: Kualitas Udara (M2)
    const char* labelAQI = "Menghitung...";
    if (aqi == 1) labelAQI = "1-Sangat Baik";
    else if (aqi == 2) labelAQI = "2-Baik";
    else if (aqi == 3) labelAQI = "3-Sedang";
    else if (aqi == 4) labelAQI = "4-Buruk (Peringatan)";
    else if (aqi == 5) labelAQI = "5-Sangat Buruk (Bahaya)";

    snprintf(baris, sizeof(baris), "🍃  AQI: %s | TVOC: %d ppb | eCO2: %d ppm\r\n", labelAQI, tvoc, eco2);
    kirimPesan(baris);

    // Baris 3: Cahaya Ambient (M4)
    if (lux >= 0.0) {
      const char* kondisiCahaya = (lux < 50.0) ? "🌙 Gelap" : ((lux < 250.0) ? "💡 Pencahayaan Normal" : "☀️ Terang");
      snprintf(baris, sizeof(baris), "💡  Cahaya: %.1f Lux (%s)\r\n", lux, kondisiCahaya);
    } else {
      snprintf(baris, sizeof(baris), "💡  BH1750: Gagal baca lux\r\n");
    }
    kirimPesan(baris);

    // Baris 4: Radar Keberadaan & Napas (M3)
    snprintf(baris, sizeof(baris), "🎯  Radar: [%-8s] Grk:%3dcm %s%3d%% | Diam:%3dcm %s%3d%%\r\n",
             labelRadar, jarakGerak, barGerak, energiGerak, jarakDiam, barDiam, energiDiam);
    kirimPesan(baris);

    // Baris 5: Status Keamanan & Aktuator Buzzer
    const char* statusBuzzer = "🔕MATI";
    bool picuAlarmJamMalam = (modeJamMalam && adaOrang && !alarmMuted);
    bool picuAlarmAQI4    = (aqi == 4 && !alarmMuted);
    bool picuAlarmAQI5    = (aqi == 5 && !alarmMuted);

    if (alarmMuted) {
      statusBuzzer = "🔕MUTE DARURAT";
    } else if (picuAlarmJamMalam) {
      statusBuzzer = "🚨ALARM PENYUSUP (FR-05)";
    } else if (picuAlarmAQI5) {
      statusBuzzer = "🚨SIRINE UDARA KRITIS (Level 5)";
    } else if (picuAlarmAQI4) {
      statusBuzzer = "⚠️BIP UDARA BURUK (Level 4)";
    }

    snprintf(baris, sizeof(baris), "🛡️  Mode: %s | Buzzer: %s\r\n",
             modeJamMalam ? "🌙JAM MALAM" : "☀️NORMAL SIANG", statusBuzzer);
    kirimPesan(baris);
    kirimPesan("======================================================\r\n");

    // F. Eksekusi Nada Alarm Buzzer Berdasarkan Prioritas
    if (picuAlarmJamMalam) {
      alarmJamMalam();
    } else if (picuAlarmAQI5) {
      alarmAQI_Level5();
    } else if (picuAlarmAQI4) {
      alarmAQI_Level4();
    } else {
      matikanBuzzer();
    }
  }
}
