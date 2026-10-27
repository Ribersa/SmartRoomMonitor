#include "SensorManager.h"

// ==============================================================================
// SensorManager.cpp - Implementasi Pengelola 4 Sensor
// ==============================================================================

SensorManager::SensorManager()
  : _ens160(&Wire, 0x53),
    _sht30Ok(false),
    _ens160Ok(false),
    _bh1750Ok(false),
    _radarOk(false),
    _activeRxPin(PIN_RADAR_RX),
    _activeTxPin(PIN_RADAR_TX),
    _stateKehadiran(STATE_RADAR_KOSONG),
    _waktuMulaiVerifikasi(0),
    _waktuTerakhirSinyalAda(0),
    _waktuTerakhirRadarKonek(0),
    _jejakNapasTerdeteksi(false) {}

bool SensorManager::hubungkanRadarAutoSwap() {
  Serial.print(F("🔄 [UART] Inisialisasi radar (RX=18, TX=17)... "));
  Serial1.setRxBufferSize(1024); // Perbesar buffer UART (256000 baud) agar tidak overflow saat baca I2C/BLE
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
  _radar.begin(Serial1, false);

  unsigned long tStart = millis();
  while (millis() - tStart < 2000) {
    _radar.read();
    if (_radar.isConnected()) {
      _activeRxPin = PIN_RADAR_RX;
      _activeTxPin = PIN_RADAR_TX;
      _waktuTerakhirRadarKonek = millis();
      Serial.println(F("TERHUBUNG!"));
      return true;
    }
    delay(10);
  }
  Serial.println(F("Belum respons."));

  Serial.print(F("🔄 [AUTO-SWAP] Membalik pin UART (RX=17, TX=18)... "));
  Serial1.end();
  delay(100);
  Serial1.setRxBufferSize(1024);
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, PIN_RADAR_TX, PIN_RADAR_RX);
  _radar.begin(Serial1, false);

  tStart = millis();
  while (millis() - tStart < 2000) {
    _radar.read();
    if (_radar.isConnected()) {
      _activeRxPin = PIN_RADAR_TX;
      _activeTxPin = PIN_RADAR_RX;
      _waktuTerakhirRadarKonek = millis();
      Serial.println(F("TERHUBUNG! (Auto-swap OK)"));
      return true;
    }
    delay(10);
  }
  Serial.println(F("Gagal."));

  // Kembalikan ke default
  Serial1.end();
  Serial1.setRxBufferSize(1024);
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
  _radar.begin(Serial1, false);
  return false;
}

void SensorManager::begin() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  // Inisialisasi SHT30-D (0x44)
  if (_sht30.begin(0x44)) {
    _sht30Ok = true;
    Serial.println(F("✅ [OK] SHT30-D (Suhu & RH) Siap!"));
  } else {
    Serial.println(F("❌ [GAGAL] SHT30-D tidak terdeteksi pada 0x44!"));
  }

  // Inisialisasi ENS160 (0x53)
  if (_ens160.begin() == 0) {
    _ens160.setPWRMode(ENS160_STANDARD_MODE);
    _ens160Ok = true;
    Serial.println(F("✅ [OK] ENS160 (Kualitas Udara) Siap!"));
  } else {
    Serial.println(F("❌ [GAGAL] ENS160 tidak terdeteksi pada 0x53!"));
  }

  // Inisialisasi BH1750 (0x23)
  if (_lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    _bh1750Ok = true;
    Serial.println(F("✅ [OK] BH1750 (Iluminansi Cahaya) Siap!"));
  } else {
    Serial.println(F("❌ [GAGAL] BH1750 tidak terdeteksi pada 0x23!"));
  }

  // Inisialisasi Radar mmWave HLK-LD2410
  _radarOk = hubungkanRadarAutoSwap();
  if (_radarOk) {
    setRadarGate(RADAR_DEFAULT_GATE_MAX, RADAR_DEFAULT_TIMEOUT_DETIK, "Meja Dekat (~1.50m / Gate 2)");
    boostSensitivitasNapas();
    // Catatan: Status Bluetooth OFF sudah tersimpan permanen di Flash NVM radar.
    // Tidak dipanggil ulang saat boot agar radar tidak mengalami reboot paksa yang memicu DISCONNECT.
    Serial.println(F("📶 [OK] Radar LD2410 Siap & Terkalibrasi!"));
  } else {
    Serial.println(F("⚠️ [PERINGATAN] Radar belum respons. Cek kabel 5V & Common GND!"));
  }
}

void SensorManager::updateRadar() {
  _radar.read();
  if (_radar.isConnected()) {
    _waktuTerakhirRadarKonek = millis();
    _radarOk = true;
  }
  evaluasiFilterKehadiran();
}

void SensorManager::evaluasiFilterKehadiran() {
  unsigned long now = millis();
  bool koneksiAktif = _radar.isConnected() || (_radarOk && (now - _waktuTerakhirRadarKonek < 3000));
  if (!koneksiAktif) {
    _stateKehadiran = STATE_RADAR_KOSONG;
    _jejakNapasTerdeteksi = false;
    return;
  }

  bool rawHadir = _radar.presenceDetected();
  uint8_t eGerak = _radar.movingTargetEnergy();
  uint8_t eDiam  = _radar.stationaryTargetEnergy();

  switch (_stateKehadiran) {
    case STATE_RADAR_KOSONG: {
      // Layer 3 (Strict Entry Hysteresis): Harus melampaui ambang masuk ketat untuk mulai verifikasi
      bool sinyalMasukKuat = rawHadir &&
                             (eGerak >= RADAR_ENTRY_MOVING_THRESHOLD ||
                              eDiam  >= RADAR_ENTRY_STILL_THRESHOLD);
      if (sinyalMasukKuat) {
        _stateKehadiran = STATE_RADAR_VERIFIKASI;
        _waktuMulaiVerifikasi = now;
        _waktuTerakhirSinyalAda = now;
        _jejakNapasTerdeteksi = (eDiam >= RADAR_BIOMETRIC_STILL_MIN);
      }
      break;
    }

    case STATE_RADAR_VERIFIKASI: {
      // Catat jejak biometrik napas mikro jika terdeteksi
      if (eDiam >= RADAR_BIOMETRIC_STILL_MIN) {
        _jejakNapasTerdeteksi = true;
      }

      bool sinyalMasihAda = rawHadir &&
                            (eGerak >= RADAR_HOLD_THRESHOLD ||
                             eDiam  >= RADAR_HOLD_THRESHOLD);

      if (sinyalMasihAda) {
        _waktuTerakhirSinyalAda = now;

        // Sah sebagai manusia jika salah satu syarat biometrik terpenuhi:
        // 1. Gerakan aktif berkelanjutan selama >= 800ms (benda jatuh/dilempar pasti mendarat < 400ms)
        // 2. ATAU terdeteksi pantulan napas/diam (eDiam >= 20%) selama >= 800ms (orang duduk santai)
        bool durasiCukup = (now - _waktuMulaiVerifikasi >= RADAR_CONFIRM_DELAY_MS);
        bool gerakAktif  = (eGerak >= RADAR_ENTRY_MOVING_THRESHOLD);

        if (durasiCukup && (gerakAktif || _jejakNapasTerdeteksi)) {
          _stateKehadiran = STATE_RADAR_MANUSIA_HADIR;
        }
      } else {
        // Objek dilempar/jatuh hilang dalam < 400ms. Jika sinyal mati melebihi toleransi glitch -> batal!
        if (now - _waktuTerakhirSinyalAda > RADAR_GLITCH_TOLERANCE_MS) {
          _stateKehadiran = STATE_RADAR_KOSONG;
          _jejakNapasTerdeteksi = false;
        }
      }
      break;
    }

    case STATE_RADAR_MANUSIA_HADIR: {
      // Layer 3 (Hold Hysteresis): Gunakan ambang bawah (15%) agar napas halus tetap terjaga
      bool sinyalTahan = rawHadir &&
                         (eGerak >= RADAR_HOLD_THRESHOLD ||
                          eDiam  >= RADAR_HOLD_THRESHOLD);

      if (sinyalTahan) {
        _waktuTerakhirSinyalAda = now;
      } else {
        // Grace period 3 detik sebelum ruangan dinyatakan benar-benar kosong
        if (now - _waktuTerakhirSinyalAda > RADAR_HOLD_GRACE_MS) {
          _stateKehadiran = STATE_RADAR_KOSONG;
          _jejakNapasTerdeteksi = false;
        }
      }
      break;
    }
  }
}

void SensorManager::cetakBarEnergi(uint8_t energi, char* bufferOut) {
  int terisi = energi / 10;
  if (terisi > 10) terisi = 10;
  strcpy(bufferOut, "[");
  for (int i = 0; i < 10; i++) {
    strcat(bufferOut, (i < terisi) ? "#" : ".");
  }
  strcat(bufferOut, "]");
}

TelemetriData SensorManager::bacaSemua() {
  TelemetriData d;
  memset(&d, 0, sizeof(TelemetriData));

  // 1. Baca SHT30-D
  d.suhu = NAN;
  d.kelembapan = NAN;
  d.sht30Ok = _sht30Ok;
  if (_sht30Ok) {
    d.suhu = _sht30.readTemperature();
    _radar.read(); // Kuras buffer UART radar di sela jeda I2C SHT30
    d.kelembapan = _sht30.readHumidity();
    _radar.read();

    // Kompensasi dinamis ke ENS160
    if (!isnan(d.suhu) && !isnan(d.kelembapan) && _ens160Ok) {
      _ens160.setTempAndHum(d.suhu, d.kelembapan);
    }
  }

  // 2. Baca ENS160
  d.ens160Ok = _ens160Ok;
  if (_ens160Ok) {
    d.aqi  = _ens160.getAQI();
    d.tvoc = _ens160.getTVOC();
    d.eco2 = _ens160.getECO2();

    switch (d.aqi) {
      case 1: d.labelAQI = "1-Sangat Baik"; break;
      case 2: d.labelAQI = "2-Baik"; break;
      case 3: d.labelAQI = "3-Sedang"; break;
      case 4: d.labelAQI = "4-Buruk (Peringatan)"; break;
      case 5: d.labelAQI = "5-Sangat Buruk (Bahaya)"; break;
      default: d.labelAQI = "Menghitung..."; break;
    }
  } else {
    d.labelAQI = "Sensor Gagal";
  }
  _radar.read();

  // 3. Baca BH1750
  d.bh1750Ok = _bh1750Ok;
  d.lux = -1.0;
  if (_bh1750Ok) {
    d.lux = _lightMeter.readLightLevel();
    if (d.lux < 50.0) d.labelCahaya = "🌙 Gelap";
    else if (d.lux < 250.0) d.labelCahaya = "💡 Normal";
    else d.labelCahaya = "☀️ Terang";
  } else {
    d.labelCahaya = "Gagal Baca";
  }

  // 4. Baca Radar HLK-LD2410 (Pastikan buffer terbaru sudah diparse)
  updateRadar();
  d.radarOk = _radar.isConnected() || (_radarOk && (millis() - _waktuTerakhirRadarKonek < 3000));
  strcpy(d.barGerak, "[..........]");
  strcpy(d.barDiam,  "[..........]");

  if (d.radarOk) {
    evaluasiFilterKehadiran();

    // Status kehadiran yang dikirim ke Lampu (FR-08) & Alarm (FR-05) sudah terfilter!
    d.adaOrang         = (_stateKehadiran == STATE_RADAR_MANUSIA_HADIR);
    d.sedangVerifikasi = (_stateKehadiran == STATE_RADAR_VERIFIKASI);
    d.bergerak         = _radar.movingTargetDetected();
    d.diam             = _radar.stationaryTargetDetected();

    d.jarakGerak  = _radar.movingTargetDistance();
    d.energiGerak = _radar.movingTargetEnergy();
    d.jarakDiam   = _radar.stationaryTargetDistance();
    d.energiDiam  = _radar.stationaryTargetEnergy();

    cetakBarEnergi(d.energiGerak, d.barGerak);
    cetakBarEnergi(d.energiDiam, d.barDiam);

    if (d.adaOrang) {
      if (d.bergerak && d.energiGerak >= RADAR_ENTRY_MOVING_THRESHOLD) d.statusRadar = "BERGERAK";
      else if (d.diam) d.statusRadar = "DIAM";
      else d.statusRadar = "HADIR";
    } else if (d.sedangVerifikasi) {
      d.statusRadar = "VERIF...";
    } else {
      d.statusRadar = "KOSONG";
    }
  } else {
    d.statusRadar = "DISCONNECT";
  }

  return d;
}

bool SensorManager::setRadarGate(uint8_t gateMaks, uint16_t timeoutDetik, const char* label) {
  uint8_t gateFix = (gateMaks < 2) ? 2 : gateMaks; // LD2410 butuh minimal Gate 2 untuk deteksi diam
  return _radar.setMaxValues(gateFix, gateFix, timeoutDetik);
}

void SensorManager::boostSensitivitasNapas() {
  // Gate 0 (0 - 75 cm): Pasang ambang tinggi (65% / 50%) untuk memblokir kebocoran radiasi belakang PCB (Back-Lobe)
  _radar.setGateSensitivityThreshold(0, 65, 50);
  // Gate 1 (75 - 150 cm): Ambang menengah (45% / 30%)
  _radar.setGateSensitivityThreshold(1, 45, 30);
  // Gate 2 & 3 (1.5m - 2.5m di Depan Sensor): Sangat peka napas mikro (15%)
  _radar.setGateSensitivityThreshold(2, 30, RADAR_BREATHING_THRESHOLD);
  _radar.setGateSensitivityThreshold(3, 35, RADAR_BREATHING_THRESHOLD);
  _radar.setGateSensitivityThreshold(4, 40, 20);
  _radar.setGateSensitivityThreshold(5, 40, 20);
  _radar.setGateSensitivityThreshold(6, 45, 25);
}

bool SensorManager::setBluetoothRadar(bool aktif) {
  if (!_radar.isConnected()) return false;

  // Frame Protokol Resmi Hi-Link HLK-LD2410 (Little-Endian)
  const uint8_t cmdMasukConfig[] = {
    0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFF, 0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01
  };
  const uint8_t cmdKeluarConfig[] = {
    0xFD, 0xFC, 0xFB, 0xFA, 0x02, 0x00, 0xFE, 0x00, 0x04, 0x03, 0x02, 0x01
  };
  uint8_t cmdBluetooth[] = {
    0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xA4, 0x00, (uint8_t)(aktif ? 0x01 : 0x00), 0x00, 0x04, 0x03, 0x02, 0x01
  };

  // 1. Masuk Mode Konfigurasi
  Serial1.write(cmdMasukConfig, sizeof(cmdMasukConfig));
  Serial1.flush();
  delay(150);

  // 2. Kirim Perintah Kontrol Bluetooth (0x00A4: 0x0000 = Matikan, 0x0001 = Nyalakan)
  Serial1.write(cmdBluetooth, sizeof(cmdBluetooth));
  Serial1.flush();
  delay(150);

  // 3. Keluar Mode Konfigurasi (Menyimpan permanen ke Flash/NVM & Restart Radar)
  Serial1.write(cmdKeluarConfig, sizeof(cmdKeluarConfig));
  Serial1.flush();
  delay(300);

  // Bersihkan sisa frame respons dari buffer serial
  while (Serial1.available()) {
    Serial1.read();
  }

  return true;
}

void SensorManager::bacaInfoRadar(char* bufferOut, size_t maxLen) {
  if (!_radar.isConnected()) {
    snprintf(bufferOut, maxLen, "Radar tidak terhubung!\r\n");
    return;
  }
  _radar.requestFirmwareVersion();
  _radar.requestCurrentConfiguration();

  snprintf(bufferOut, maxLen,
           "Firmware Radar: v%d.%02d.%08X | Max Gate Gerak: %d (~%.2fm) | Diam: %d (~%.2fm)",
           _radar.firmware_major_version, _radar.firmware_minor_version, _radar.firmware_bugfix_version,
           _radar.max_moving_gate, (_radar.max_moving_gate + 1) * 0.75,
           _radar.max_stationary_gate, (_radar.max_stationary_gate + 1) * 0.75);
}

bool SensorManager::isRadarConnected() {
  return _radar.isConnected();
}
