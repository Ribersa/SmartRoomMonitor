// ==============================================================================
// 07_uji_radar_ld2410.ino (VERSI ULTIMATE: RADAR M3 + BLE SERIAL + FR-05/FR-06)
// Proyek: SmartRoomMonitor (RPL) - PIC Modul M3: Deswiryawan Saragih (241111008)
// Deskripsi: Pengujian komprehensif Radar mmWave 24 GHz Hilink HLK-LD2410
//            dengan dukungan ganda: Kabel USB Serial Monitor & Bluetooth BLE HP!
// Fitur Utama:
//   1. Streaming & Kontrol 2 Arah via "Serial Bluetooth Terminal" (BLE NUS).
//   2. Auto-Swap Pin UART (otomatis mendeteksi jika TX/RX radar terbalik).
//   3. Kalibrasi Sensitivitas Napas (Stationary Detection) agar orang diam tetap terdeteksi.
//   4. Visualisasi bar energi gerak & energi napas mikro (diam).
//   5. Simulasi Peringatan Jam Malam (22:00-06:00) + Buzzer (FR-05).
//   6. Simulasi Tombol MUTE Darurat via HP / Serial (FR-06).
// ==============================================================================

#include <ld2410.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define RADAR_RX_PIN 18 // Default: terima dari TX Radar
#define RADAR_TX_PIN 17 // Default: kirim ke RX Radar
#define RADAR_BAUD   256000
#define BUZZER_PIN   6  // Buzzer Active-LOW (HIGH = Mati, LOW = Nyala)

// UUID Standar Nordic Semiconductor UART Service (NUS) untuk Serial Bluetooth Terminal
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

ld2410 radar;
unsigned long lastPrintTime = 0;
unsigned long printIntervalMs = 1000; // Default cetak tiap 1 detik (bisa diubah ke 3 detik dengan 't')

// State BLE
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
bool bleConnected = false;
bool oldBleConnected = false;
volatile char pendingBleCmd = 0;

// State Simulasi Fitur M3 (FR-05 & FR-06)
bool modeJamMalam = false; // Simulasi jam 22:00 - 06:00 (FR-05)
bool alarmMuted   = false; // Status MUTE darurat oleh Staff (FR-06)
int activeRxPin   = RADAR_RX_PIN;
int activeTxPin   = RADAR_TX_PIN;

// Fungsi bantu kirim teks ke USB Serial sekaligus ke Bluetooth HP
void kirimPesan(const char* teks) {
  Serial.print(teks);
  if (bleConnected && pTxCharacteristic != NULL) {
    pTxCharacteristic->setValue((uint8_t*)teks, strlen(teks));
    pTxCharacteristic->notify();
  }
}

// Callback Koneksi Server BLE
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      bleConnected = true;
      Serial.println("\n📲 [BLE] Smartphone Terhubung! Kontrol & Telemetri Nirkabel Aktif.");
    };

    void onDisconnect(BLEServer* pServer) {
      bleConnected = false;
      Serial.println("\n📲 [BLE] Smartphone Terputus. Kembali mengiklankan sinyal...");
    }
};

// Callback Penerima Perintah Ketikan dari HP (RX Characteristic)
class MyRxCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        pendingBleCmd = rxValue[0]; // Simpan karakter perintah untuk dieksekusi di loop
      }
    }
};

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

void bunyiAlarmJamMalam() {
  mainkanNada(2800, 90);
  delay(40);
  mainkanNada(3400, 90);
  matikanBuzzer();
}

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
  kirimPesan("\r\n================ PANEL KONTROL RADAR M3 (USB & BLE) ================\r\n");
  kirimPesan("  [1] Mode Meja Dekat   (~1.50m / Gate 2, Peka Napas, Timeout 5s)\r\n");
  kirimPesan("  [2] Mode Area Kasur   (~2.25m / Gate 3, Peka Napas, Timeout 5s)\r\n");
  kirimPesan("  [3] Mode Kamar Penuh  (~4.50m / Gate 6, Peka Napas, Timeout 5s)\r\n");
  kirimPesan("  [4] Mode Jarak Maks   (~6.00m / Gate 8, Peka Napas, Timeout 5s)\r\n");
  kirimPesan("  [s] Boost Sensitivitas Diam/Napas Maksimal (Threshold = 15%)\r\n");
  kirimPesan("  [t] Toggle Interval Layar: Cepat (1 dtk) vs Santai NFR-01 (3 dtk)\r\n");
  kirimPesan("  [m] Toggle Simulasi JAM MALAM 22:00-06:00 (FR-05 Alarm)\r\n");
  kirimPesan("  [x] Tekan MUTE DARURAT (FR-06 Bungkam Buzzer)\r\n");
  kirimPesan("  [i] Baca Info Firmware & Konfigurasi Gate Radar\r\n");
  kirimPesan("  [r] Factory Reset Radar\r\n");
  kirimPesan("  [h] Tampilkan Menu Ini\r\n");
  kirimPesan("====================================================================\r\n\n");
}

// Fungsi untuk mempertajam kepekaan deteksi orang diam (bernapas)
// Catatan: Di protokol LD2410, semakin KECIL nilai threshold (0-100), semakin PEKA sensornya!
void optimalkanSensitivitasNapas() {
  kirimPesan("🫁 [KALIBRASI NAPAS] Menurunkan ambang batas (threshold) diam agar peka napas...\r\n");
  // Gate 0 & 1: Gerak=30, Diam=20 (Catatan: di beberapa versi FW, Diam baru aktif mulai Gate 2)
  radar.setGateSensitivityThreshold(0, 30, 20);
  radar.setGateSensitivityThreshold(1, 30, 20);
  // Gate 2 & 3 (~0.75m - 2.25m): Zona duduk di meja/kasur -> buat SANGAT PEKA terhadap napas (Threshold Diam = 15)
  radar.setGateSensitivityThreshold(2, 30, 15);
  radar.setGateSensitivityThreshold(3, 35, 15);
  // Gate 4 - 6: Zona tengah kamar (Threshold Diam = 20)
  radar.setGateSensitivityThreshold(4, 40, 20);
  radar.setGateSensitivityThreshold(5, 40, 20);
  radar.setGateSensitivityThreshold(6, 45, 25);
  kirimPesan("✅ [SIAP] Sensitivitas deteksi orang diam/bernapas telah di-boost!\r\n\n");
}

void aturBatasJarakGate(uint8_t gateMaks, uint16_t timeoutDetik, const char* labelMode) {
  // PENTING: Protokol HLK-LD2410 mewajibkan max_stationary_gate minimal Gate 2
  // karena Gate 0 & Gate 1 tidak memiliki deteksi diam (stationary) mandiri!
  uint8_t gateDiam = (gateMaks < 2) ? 2 : gateMaks;
  uint8_t gateGerak = (gateMaks < 2) ? 2 : gateMaks;

  char buf[128];
  snprintf(buf, sizeof(buf), "\r\n⚙️ [KONFIGURASI] Set jarak ke %s (Gate %d, Timeout %ds)...\r\n",
           labelMode, gateGerak, timeoutDetik);
  kirimPesan(buf);

  if (radar.setMaxValues(gateGerak, gateDiam, timeoutDetik)) {
    kirimPesan("✅ [SUKSES] Batas jarak & deteksi diam berhasil diterapkan!\r\n\n");
    mainkanNada(2400, 60);
  } else {
    kirimPesan("⚠️ [GAGAL] Radar belum merespons perintah konfigurasi.\r\n\n");
  }
}

void bacaInfoRadar() {
  char buf[128];
  kirimPesan("\r\n🔍 [INFO RADAR] Membaca parameter internal HLK-LD2410...\r\n");
  if (radar.requestFirmwareVersion()) {
    snprintf(buf, sizeof(buf), "   - Firmware: v%d.%02d.%08X\r\n",
             radar.firmware_major_version,
             radar.firmware_minor_version,
             radar.firmware_bugfix_version);
    kirimPesan(buf);
  }
  if (radar.requestCurrentConfiguration()) {
    snprintf(buf, sizeof(buf), "   - Max Gate Gerak: %d (~%.2fm) | Max Gate Diam: %d (~%.2fm) | Timeout: %ds\r\n\n",
             radar.max_moving_gate, (radar.max_moving_gate + 1) * 0.75,
             radar.max_stationary_gate, (radar.max_stationary_gate + 1) * 0.75,
             radar.sensor_idle_time);
    kirimPesan(buf);
  }
}

bool hubungkanRadarOtomatis() {
  Serial.print("🔄 [UART] Mencoba koneksi standar (RX=GPIO 18, TX=GPIO 17)... ");
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, 18, 17);
  radar.begin(Serial1, false);

  unsigned long tStart = millis();
  while (millis() - tStart < 2000) {
    radar.read();
    if (radar.isConnected()) {
      activeRxPin = 18;
      activeTxPin = 17;
      Serial.println("TERHUBUNG!");
      return true;
    }
    delay(10);
  }
  Serial.println("Belum ada respons.");

  Serial.print("🔄 [AUTO-SWAP] Mencoba membalik jalur pin (RX=GPIO 17, TX=GPIO 18)... ");
  Serial1.end();
  delay(100);
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, 17, 18);
  radar.begin(Serial1, false);

  tStart = millis();
  while (millis() - tStart < 2000) {
    radar.read();
    if (radar.isConnected()) {
      activeRxPin = 17;
      activeTxPin = 18;
      Serial.println("TERHUBUNG! (Pin otomatis disesuaikan)");
      return true;
    }
    delay(10);
  }
  Serial.println("Gagal.");

  Serial1.end();
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, 18, 17);
  radar.begin(Serial1, false);
  return false;
}

void eksekusiPerintah(char cmd) {
  char buf[128];
  switch (cmd) {
    case '0':
    case '1': aturBatasJarakGate(2, 5, "Meja Dekat (~1.50m / Gate 2)"); break;
    case '2': aturBatasJarakGate(3, 5, "Area Kasur (~2.25m / Gate 3)"); break;
    case '3': aturBatasJarakGate(6, 5, "Kamar Penuh (~4.50m / Gate 6)"); break;
    case '4': aturBatasJarakGate(8, 5, "Maksimal (~6.00m / Gate 8)"); break;
    case 's':
    case 'S':
      optimalkanSensitivitasNapas();
      mainkanNada(2600, 70);
      break;
    case 't':
    case 'T':
      printIntervalMs = (printIntervalMs == 1000) ? 3000 : 1000;
      snprintf(buf, sizeof(buf), "\r\n⏱️ [INTERVAL CETAK] Diubah ke tiap %lu detik!\r\n\n", printIntervalMs / 1000);
      kirimPesan(buf);
      break;
    case 'm':
    case 'M':
      modeJamMalam = !modeJamMalam;
      alarmMuted = false;
      snprintf(buf, sizeof(buf), "\r\n🌙 [MODE JAM MALAM FR-05]: %s\r\n\n",
               modeJamMalam ? "AKTIF (Alarm Menyala Jika Ada Orang!)" : "NONAKTIF (Siang Normal)");
      kirimPesan(buf);
      mainkanNada(modeJamMalam ? 3000 : 1800, 80);
      break;
    case 'x':
    case 'X':
      alarmMuted = true;
      matikanBuzzer();
      kirimPesan("\r\n🔕 [MUTE DARURAT FR-06] Alarm dibungkam! Buzzer dihentikan.\r\n\n");
      break;
    case 'i':
    case 'I': bacaInfoRadar(); break;
    case 'r':
    case 'R':
      kirimPesan("\r\n♻️ [RESET] Mengembalikan radar ke setelan pabrik...\r\n");
      if (radar.requestFactoryReset()) {
        radar.requestRestart();
        kirimPesan("✅ [SUKSES] Factory reset berhasil!\r\n\n");
      } else {
        kirimPesan("⚠️ [GAGAL] Factory reset tidak direspons.\r\n\n");
      }
      break;
    case 'h':
    case 'H': tampilkanMenuBantuan(); break;
  }
}

void prosesInputSemuaJalur() {
  // 1. Cek perintah dari Bluetooth HP
  if (pendingBleCmd != 0) {
    char c = pendingBleCmd;
    pendingBleCmd = 0;
    eksekusiPerintah(c);
  }

  // 2. Cek perintah dari USB Serial Monitor Laptop
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

  Serial.println("📡 [BLE] Bluetooth Aktif! Nama: 'SmartRoom-ESP32' (Siap dikonek dari HP)");
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  matikanBuzzer();

  delay(2000);

  Serial.println("\n=====================================================================");
  Serial.println("  [SmartRoomMonitor] Radar HLK-LD2410 (M3) + BLE Serial Control");
  Serial.println("=====================================================================");

  // Aktifkan Bluetooth BLE agar bisa dipantau & dikontrol dari HP
  inisialisasiBLE();

  // Hubungkan ke Radar
  if (hubungkanRadarOtomatis()) {
    mainkanNada(2000, 60);
    delay(50);
    mainkanNada(2800, 90);

    // 1. Set batas jarak ke Gate 2 (~1.5m) karena protokol LD2410 baru mengaktifkan
    //    deteksi napas/diam (stationary) mulai dari Gate 2 ke atas!
    aturBatasJarakGate(2, 5, "Meja Dekat (~1.50m / Gate 2)");

    // 2. Pertajam sensitivitas deteksi diam (napas)
    optimalkanSensitivitasNapas();

    bacaInfoRadar();
  } else {
    Serial.println("⚠️ [PERINGATAN] Radar belum merespons saat boot. Cek kabel 5V & Common GND.");
  }

  tampilkanMenuBantuan();
}

void loop() {
  radar.read();
  prosesInputSemuaJalur();

  // Rekoneksi otomatis BLE jika HP terputus
  if (!bleConnected && oldBleConnected) {
    delay(300);
    pServer->startAdvertising();
    oldBleConnected = bleConnected;
  }
  if (bleConnected && !oldBleConnected) {
    oldBleConnected = bleConnected;
  }

  // Tampilkan telemetri sesuai interval (default 1000 ms, bisa diubah ke 3000 ms pakai 't')
  if (millis() - lastPrintTime >= printIntervalMs) {
    lastPrintTime = millis();
    char barisOut[160];

    if (radar.isConnected()) {
      bool adaOrang = radar.presenceDetected();
      bool bergerak = radar.movingTargetDetected();
      bool diam     = radar.stationaryTargetDetected();

      uint16_t jarakGerak  = radar.movingTargetDistance();
      uint8_t  energiGerak = radar.movingTargetEnergy();
      uint16_t jarakDiam   = radar.stationaryTargetDistance();
      uint8_t  energiDiam  = radar.stationaryTargetEnergy();

      char barGerak[16], barDiam[16];
      cetakBarEnergi(energiGerak, barGerak);
      cetakBarEnergi(energiDiam, barDiam);

      const char* statusFR05 = "KOSONG";
      if (adaOrang) {
        if (bergerak) statusFR05 = "BERGERAK";
        else if (diam) statusFR05 = "DIAM";
      } else {
        alarmMuted = false;
      }

      if (!adaOrang) {
        snprintf(barisOut, sizeof(barisOut), "🔲 [%-8s] Aman | Mode: %s\r\n",
                 statusFR05, modeJamMalam ? "🌙MALAM" : "☀️SIANG");
        kirimPesan(barisOut);
        matikanBuzzer();
      } else {
        snprintf(barisOut, sizeof(barisOut),
                 "🎯 [%-8s] Gerak:%3dcm %s%3d%% | Diam:%3dcm %s%3d%% | %s\r\n",
                 statusFR05,
                 jarakGerak, barGerak, energiGerak,
                 jarakDiam, barDiam, energiDiam,
                 modeJamMalam ? (alarmMuted ? "🔕MUTE" : "🚨ALARM") : "☀️SIANG");
        kirimPesan(barisOut);

        if (modeJamMalam && !alarmMuted) {
          kirimPesan("   ⚠️ [ALARM JAM MALAM FR-05] Ketik 'x' untuk MUTE!\r\n");
          bunyiAlarmJamMalam();
        } else {
          matikanBuzzer();
        }
      }
    } else {
      kirimPesan("⏳ [MENUNGGU RADAR] Cek suplai 5V & Common GND...\r\n");
    }
  }
}
