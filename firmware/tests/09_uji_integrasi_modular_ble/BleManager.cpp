#include "BleManager.h"

// Pointer statis tunggal untuk callback global BLE
static BleManager* s_instance = nullptr;
static volatile char s_pendingCmd = 0;

class ServerCallbacksHandler: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    if (s_instance) s_instance->setConnected(true);
    Serial.println(F("\n📲 [BLE] Smartphone Terhubung! Telemetri Nirkabel Aktif."));
  }
  void onDisconnect(BLEServer* pServer) {
    if (s_instance) s_instance->setConnected(false);
    Serial.println(F("\n📲 [BLE] Smartphone Terputus. Menunggu koneksi ulang..."));
  }
};

class RxCallbacksHandler: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      s_pendingCmd = rxValue[0];
    }
  }
};

BleManager::BleManager()
  : _pServer(nullptr),
    _pTxCharacteristic(nullptr),
    _bleConnected(false),
    _oldBleConnected(false),
    _buzzer(nullptr),
    _sensors(nullptr),
    _fan(nullptr),
    _modeJamMalam(false),
    _intervalTelemetriMs(INTERVAL_TELEMETRI_DEFAULT_MS) {
  s_instance = this;
}

void BleManager::begin(BuzzerManager* buzzer, SensorManager* sensors, FanManager* fan) {
  _buzzer = buzzer;
  _sensors = sensors;
  _fan = fan;

  BLEDevice::init(BLE_DEVICE_NAME);
  _pServer = BLEDevice::createServer();
  _pServer->setCallbacks(new ServerCallbacksHandler());

  BLEService *pService = _pServer->createService(BLE_SERVICE_UUID);

  _pTxCharacteristic = pService->createCharacteristic(
                         BLE_CHAR_UUID_TX,
                         BLECharacteristic::PROPERTY_NOTIFY
                       );
  _pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           BLE_CHAR_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new RxCallbacksHandler());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println(F("📡 [BLE] Nordic UART Service Siap! Nama: 'SmartRoom-ESP32'"));
}

void BleManager::kirimPesan(const char* teks) {
  Serial.print(teks);
  if (_bleConnected && _pTxCharacteristic != nullptr) {
    _pTxCharacteristic->setValue((uint8_t*)teks, strlen(teks));
    _pTxCharacteristic->notify();
    delay(5); // Jeda aman buffer stack BLE
  }
}

void BleManager::tampilkanMenuBantuan() {
  kirimPesan("\r\n================ PUSAT KONTROL MODULAR (USB & BLE) ================\r\n");
  kirimPesan("  [1] Jarak Radar: Meja Dekat   (~1.50m / Gate 2, Peka Napas)\r\n");
  kirimPesan("  [2] Jarak Radar: Area Kasur   (~2.25m / Gate 3, Peka Napas)\r\n");
  kirimPesan("  [3] Jarak Radar: Kamar Penuh  (~4.50m / Gate 6, Peka Napas)\r\n");
  kirimPesan("  [4] Jarak Radar: Jarak Maks   (~6.00m / Gate 8, Peka Napas)\r\n");
  kirimPesan("  [s] Boost Sensitivitas Napas/Diam (Threshold = 15%)\r\n");
  kirimPesan("  [k] Ganti Mode KIPAS 5V (FR-02): OTOMATIS -> SELALU ON -> OFF\r\n");
  kirimPesan("  [m] Toggle Simulasi JAM MALAM 22:00-06:00 (FR-05 Alarm Radar)\r\n");
  kirimPesan("  [x] Tekan MUTE DARURAT (FR-06 Bungkam Semua Alarm Buzzer)\r\n");
  kirimPesan("  [t] Toggle Interval Telemetri (1 dtk vs 2 dtk)\r\n");
  kirimPesan("  [i] Tampilkan Info Konfigurasi Radar & Diagnostik Sistem\r\n");
  kirimPesan("  [h] Tampilkan Menu Bantuan Ini\r\n");
  kirimPesan("===================================================================\r\n\n");
}

void BleManager::eksekusiPerintah(char cmd) {
  char buf[160];
  switch (cmd) {
    case '1':
      if (_sensors) {
        _sensors->setRadarGate(2, 5, "Meja Dekat");
        kirimPesan("⚙️ [RADAR] Jarak diset: Meja Dekat (~1.50m / Gate 2)\r\n");
        if (_buzzer) _buzzer->nadaKonfirmasi();
      }
      break;
    case '2':
      if (_sensors) {
        _sensors->setRadarGate(3, 5, "Area Kasur");
        kirimPesan("⚙️ [RADAR] Jarak diset: Area Kasur (~2.25m / Gate 3)\r\n");
        if (_buzzer) _buzzer->nadaKonfirmasi();
      }
      break;
    case '3':
      if (_sensors) {
        _sensors->setRadarGate(6, 5, "Kamar Penuh");
        kirimPesan("⚙️ [RADAR] Jarak diset: Kamar Penuh (~4.50m / Gate 6)\r\n");
        if (_buzzer) _buzzer->nadaKonfirmasi();
      }
      break;
    case '4':
      if (_sensors) {
        _sensors->setRadarGate(8, 5, "Jarak Maks");
        kirimPesan("⚙️ [RADAR] Jarak diset: Maksimal (~6.00m / Gate 8)\r\n");
        if (_buzzer) _buzzer->nadaKonfirmasi();
      }
      break;
    case 's':
    case 'S':
      if (_sensors) {
        _sensors->boostSensitivitasNapas();
        kirimPesan("🫁 [KALIBRASI] Threshold napas mikro diset ke 15% (Sangat Peka)!\r\n");
        if (_buzzer) _buzzer->nadaKonfirmasi();
      }
      break;
    case 'k':
    case 'K':
      if (_fan) {
        _fan->cycleMode();
        snprintf(buf, sizeof(buf), "\r\n🌀 [KIPAS FR-02] Mode diubah ke: %s (Status: %s)\r\n\n",
                 _fan->getModeLabel(), _fan->getStatusLabel());
        kirimPesan(buf);
        if (_buzzer) _buzzer->nadaKonfirmasi();
      }
      break;
    case 't':
    case 'T':
      _intervalTelemetriMs = (_intervalTelemetriMs == INTERVAL_TELEMETRI_DEFAULT_MS)
                              ? INTERVAL_TELEMETRI_CEPAT_MS
                              : INTERVAL_TELEMETRI_DEFAULT_MS;
      snprintf(buf, sizeof(buf), "\r\n⏱️ [INTERVAL] Laju streaming diubah ke tiap %lu detik!\r\n\n",
               _intervalTelemetriMs / 1000);
      kirimPesan(buf);
      break;
    case 'm':
    case 'M':
      _modeJamMalam = !_modeJamMalam;
      if (_buzzer) _buzzer->setMuted(false); // Reset mute saat ganti mode
      snprintf(buf, sizeof(buf), "\r\n🌙 [JAM MALAM FR-05]: %s\r\n\n",
               _modeJamMalam ? "AKTIF (Alarm berbunyi jika ada orang!)" : "NONAKTIF (Siang Normal)");
      kirimPesan(buf);
      if (_buzzer) _buzzer->mainkanNada(_modeJamMalam ? 3000 : 1800, 80);
      break;
    case 'x':
    case 'X':
      if (_buzzer) {
        _buzzer->setMuted(true);
        kirimPesan("\r\n🔕 [MUTE DARURAT FR-06] Alarm buzzer dibungkam seketika!\r\n\n");
      }
      break;
    case 'i':
    case 'I':
      if (_sensors) {
        char info[160];
        _sensors->bacaInfoRadar(info, sizeof(info));
        snprintf(buf, sizeof(buf), "\r\n🔍 [DIAGNOSTIK] %s\r\n\n", info);
        kirimPesan(buf);
      }
      break;
    case 'h':
    case 'H':
      tampilkanMenuBantuan();
      break;
  }
}

void BleManager::update() {
  // 1. Rekoneksi otomatis advertising BLE jika terputus
  if (!_bleConnected && _oldBleConnected) {
    delay(300);
    _pServer->startAdvertising();
    _oldBleConnected = _bleConnected;
  }
  if (_bleConnected && !_oldBleConnected) {
    _oldBleConnected = _bleConnected;
  }

  // 2. Layani perintah masuk dari smartphone (RX)
  if (s_pendingCmd != 0) {
    char c = s_pendingCmd;
    s_pendingCmd = 0;
    eksekusiPerintah(c);
  }

  // 3. Layani perintah masuk dari USB Serial
  if (Serial.available()) {
    char c = Serial.read();
    while (Serial.available()) {
      char next = Serial.peek();
      if (next == '\r' || next == '\n') Serial.read();
      else break;
    }
    eksekusiPerintah(c);
  }
}

void BleManager::kirimTelemetri(const TelemetriData& d) {
  char baris[160];
  kirimPesan("\r\n================ TELEMETRI SMART ROOM ================\r\n");

  // Suhu & Kelembapan (M1)
  if (!isnan(d.suhu) && !isnan(d.kelembapan)) {
    snprintf(baris, sizeof(baris), "🌡️  Suhu: %.2f °C | 💧 Kelembapan: %.2f %%\r\n", d.suhu, d.kelembapan);
  } else {
    snprintf(baris, sizeof(baris), "🌡️  SHT30: Sensor gagal baca / belum siap\r\n");
  }
  kirimPesan(baris);

  // Kualitas Udara (M2)
  snprintf(baris, sizeof(baris), "🍃  AQI: %s | TVOC: %d ppb | eCO2: %d ppm\r\n", d.labelAQI, d.tvoc, d.eco2);
  kirimPesan(baris);

  // Cahaya (M4)
  if (d.lux >= 0.0) {
    snprintf(baris, sizeof(baris), "💡  Cahaya: %.1f Lux (%s)\r\n", d.lux, d.labelCahaya);
  } else {
    snprintf(baris, sizeof(baris), "💡  BH1750: Gagal baca lux\r\n");
  }
  kirimPesan(baris);

  // Radar Keberadaan & Napas (M3)
  snprintf(baris, sizeof(baris), "🎯  Radar: [%-8s] Grk:%3dcm %s%3d%% | Diam:%3dcm %s%3d%%\r\n",
           d.statusRadar, d.jarakGerak, d.barGerak, d.energiGerak, d.jarakDiam, d.barDiam, d.energiDiam);
  kirimPesan(baris);

  // Aktuator Kipas 5V (FR-02)
  if (_fan) {
    snprintf(baris, sizeof(baris), "🌀  Kipas: %s\r\n", _fan->getStatusLabel());
    kirimPesan(baris);
  }

  // Status Keamanan & Buzzer
  const char* statusBuzzer = "🔕MATI";
  if (_buzzer && _buzzer->isMuted()) {
    statusBuzzer = "🔕MUTE DARURAT";
  } else if (_modeJamMalam && d.adaOrang) {
    statusBuzzer = "🚨ALARM PENYUSUP (FR-05)";
  } else if (d.aqi == 5) {
    statusBuzzer = "🚨SIRINE UDARA KRITIS (Level 5)";
  } else if (d.aqi == 4) {
    statusBuzzer = "⚠️BIP UDARA BURUK (Level 4)";
  }

  snprintf(baris, sizeof(baris), "🛡️  Mode: %s | Buzzer: %s\r\n",
           _modeJamMalam ? "🌙JAM MALAM" : "☀️NORMAL SIANG", statusBuzzer);
  kirimPesan(baris);
  kirimPesan("======================================================\r\n");
}
