#include "SensorManager.h"

// ==============================================================================
// SensorManager.cpp - Implementasi 4 Sensor + Filter Radar 3-Lapis + Baseline airDelta
// ==============================================================================

SensorManager::SensorManager()
  : _ens160(&Wire, 0x53),
    _sht30Ok(false),
    _ens160Ok(false),
    _bh1750Ok(false),
    _radarOk(false),
    _bootMillis(0),
    _tvocBaseline(0.0f),
    _baselineSampleSum(0),
    _baselineSampleCount(0),
    _baselineLocked(false),
    _stateKehadiran(STATE_RADAR_KOSONG),
    _waktuMulaiVerifikasi(0),
    _waktuTerakhirSinyalAda(0),
    _waktuTerakhirRadarKonek(0),
    _jejakNapasTerdeteksi(false) {}

bool SensorManager::hubungkanRadarAutoSwap() {
  Serial.print(F("[UART] Inisialisasi radar LD2410B (RX=18, TX=17)... "));
  Serial1.setRxBufferSize(1024);
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
  _radar.begin(Serial1, false);

  unsigned long tStart = millis();
  while (millis() - tStart < 2000) {
    _radar.read();
    if (_radar.isConnected()) {
      _waktuTerakhirRadarKonek = millis();
      Serial.println(F("TERHUBUNG!"));
      return true;
    }
    delay(10);
  }
  Serial.println(F("Belum respons."));

  Serial.print(F("[UART] Auto-swap pin radar (RX=17, TX=18)... "));
  Serial1.end();
  delay(100);
  Serial1.setRxBufferSize(1024);
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, PIN_RADAR_TX, PIN_RADAR_RX);
  _radar.begin(Serial1, false);

  tStart = millis();
  while (millis() - tStart < 2000) {
    _radar.read();
    if (_radar.isConnected()) {
      _waktuTerakhirRadarKonek = millis();
      Serial.println(F("TERHUBUNG! (Auto-swap OK)"));
      return true;
    }
    delay(10);
  }
  Serial.println(F("Gagal."));

  Serial1.end();
  Serial1.setRxBufferSize(1024);
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
  _radar.begin(Serial1, false);
  return false;
}

void SensorManager::begin() {
  _bootMillis = millis();
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  if (_sht30.begin(0x44)) {
    _sht30Ok = true;
    Serial.println(F("[OK] SHT30-D (0x44) siap."));
  } else {
    Serial.println(F("[ERR] SHT30-D tidak terdeteksi di 0x44."));
  }

  if (_ens160.begin() == 0) {
    _ens160.setPWRMode(ENS160_STANDARD_MODE);
    _ens160Ok = true;
    Serial.println(F("[OK] ENS160 (0x53) siap. Warm-up & kalibrasi baseline 3 menit aktif (NFR-06)."));
  } else {
    Serial.println(F("[ERR] ENS160 tidak terdeteksi di 0x53."));
  }

  if (_lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    _bh1750Ok = true;
    Serial.println(F("[OK] BH1750 (0x23) siap."));
  } else {
    Serial.println(F("[ERR] BH1750 tidak terdeteksi di 0x23."));
  }

  _radarOk = hubungkanRadarAutoSwap();
  if (_radarOk) {
    setRadarGate(RADAR_DEFAULT_GATE_MAX, RADAR_DEFAULT_TIMEOUT_DETIK);
    boostSensitivitasNapas();
    Serial.println(F("[OK] Radar HLK-LD2410B terkalibrasi (3-Layer Validator + Back-Lobe Shield)."));
  } else {
    Serial.println(F("[WARN] Radar belum respons. Periksa suplai 5V dan common GND."));
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
      if (eDiam >= RADAR_BIOMETRIC_STILL_MIN) {
        _jejakNapasTerdeteksi = true;
      }

      bool sinyalMasihAda = rawHadir &&
                            (eGerak >= RADAR_HOLD_THRESHOLD ||
                             eDiam  >= RADAR_HOLD_THRESHOLD);

      if (sinyalMasihAda) {
        _waktuTerakhirSinyalAda = now;
        bool durasiCukup = (now - _waktuMulaiVerifikasi >= RADAR_CONFIRM_DELAY_MS);
        bool gerakAktif  = (eGerak >= RADAR_ENTRY_MOVING_THRESHOLD);

        if (durasiCukup && (gerakAktif || _jejakNapasTerdeteksi)) {
          _stateKehadiran = STATE_RADAR_MANUSIA_HADIR;
        }
      } else if (now - _waktuTerakhirSinyalAda > RADAR_GLITCH_TOLERANCE_MS) {
        _stateKehadiran = STATE_RADAR_KOSONG;
        _jejakNapasTerdeteksi = false;
      }
      break;
    }

    case STATE_RADAR_MANUSIA_HADIR: {
      bool sinyalTahan = rawHadir &&
                         (eGerak >= RADAR_HOLD_THRESHOLD ||
                          eDiam  >= RADAR_HOLD_THRESHOLD);

      if (sinyalTahan) {
        _waktuTerakhirSinyalAda = now;
      } else if (now - _waktuTerakhirSinyalAda > RADAR_HOLD_GRACE_MS) {
        _stateKehadiran = STATE_RADAR_KOSONG;
        _jejakNapasTerdeteksi = false;
      }
      break;
    }
  }
}

bool SensorManager::isEns160WarmingUp() const {
  return (millis() - _bootMillis) < WARMUP_ENS160_MS;
}

TelemetriData SensorManager::bacaSemua(float darkLuxThreshold, uint16_t airDeltaModerate, uint16_t airDeltaBad) {
  TelemetriData d;
  memset(&d, 0, sizeof(TelemetriData));

  // 1. SHT30-D (M1)
  d.suhu = NAN;
  d.kelembapan = NAN;
  d.sht30Ok = _sht30Ok;
  d.labelKelembapan = "Nyaman";

  if (_sht30Ok) {
    d.suhu = _sht30.readTemperature();
    _radar.read();
    d.kelembapan = _sht30.readHumidity();
    _radar.read();

    if (!isnan(d.kelembapan)) {
      if (d.kelembapan < 40.0f) d.labelKelembapan = "Kering";
      else if (d.kelembapan <= 60.0f) d.labelKelembapan = "Nyaman";
      else d.labelKelembapan = "Lembap";
    }

    if (!isnan(d.suhu) && !isnan(d.kelembapan) && _ens160Ok) {
      _ens160.setTempAndHum(d.suhu, d.kelembapan);
    }
  }

  // 2. ENS160 (M2) + Masa Pemanasan 3 Menit & Perhitungan airDelta terhadap Baseline (FR-03, NFR-06)
  d.ens160Ok = _ens160Ok;
  unsigned long elapsedBoot = millis() - _bootMillis;
  d.warmingUp = (elapsedBoot < WARMUP_ENS160_MS);
  d.airDelta = 0;
  d.airBad = false;

  if (_ens160Ok) {
    d.aqi  = _ens160.getAQI();
    d.tvoc = _ens160.getTVOC();
    d.eco2 = _ens160.getECO2();

    if (d.warmingUp || d.aqi == 0) {
      d.labelAQI = "Memanaskan";
      // Rekam sampel baseline udara normal pada menit ke-2 s.d. ke-3 saat elemen mulai stabil
      if (elapsedBoot >= WARMUP_BASELINE_START_MS && d.aqi > 0) {
        _baselineSampleSum += d.tvoc;
        _baselineSampleCount++;
      }
    } else {
      if (!_baselineLocked) {
        if (_baselineSampleCount > 0) {
          _tvocBaseline = (float)_baselineSampleSum / (float)_baselineSampleCount;
        } else {
          _tvocBaseline = (float)d.tvoc;
        }
        _baselineLocked = true;
        Serial.printf("[ENS160] Baseline udara normal terkunci: %.1f ppb TVOC.\n", _tvocBaseline);
      } else if ((float)d.tvoc < _tvocBaseline) {
        // Adaptasi perlahan ke bawah jika udara ruangan membaik lebih bersih dari baseline awal
        _tvocBaseline = (_tvocBaseline * 0.95f) + ((float)d.tvoc * 0.05f);
      }

      uint16_t baseInt = (uint16_t)roundf(_tvocBaseline);
      d.airDelta = (d.tvoc > baseInt) ? (d.tvoc - baseInt) : 0;

      if (d.airDelta >= airDeltaBad || d.aqi >= 4) {
        d.labelAQI = "Buruk";
        d.airBad = true;
      } else if (d.airDelta >= airDeltaModerate || d.aqi == 3) {
        d.labelAQI = "Sedang";
      } else {
        d.labelAQI = "Baik";
      }
    }
  } else {
    d.labelAQI = "Memanaskan";
  }
  _radar.read();

  // 3. BH1750 (M4) — Sesuai UML UC-05: < darkLuxThreshold = "Gelap", >= = "Terang"
  d.bh1750Ok = _bh1750Ok;
  d.lux = -1.0f;
  d.labelCahaya = "Terang";
  if (_bh1750Ok) {
    d.lux = _lightMeter.readLightLevel();
    d.labelCahaya = (d.lux < darkLuxThreshold) ? "Gelap" : "Terang";
  }
  _radar.read();

  // 4. Radar HLK-LD2410B (M3)
  updateRadar();
  d.radarOk = _radar.isConnected() || (_radarOk && (millis() - _waktuTerakhirRadarKonek < 3000));
  d.presenceState = "tidak ada";

  if (d.radarOk) {
    evaluasiFilterKehadiran();
    d.adaOrang         = (_stateKehadiran == STATE_RADAR_MANUSIA_HADIR);
    d.sedangVerifikasi = (_stateKehadiran == STATE_RADAR_VERIFIKASI);
    d.bergerak         = _radar.movingTargetDetected();
    d.diam             = _radar.stationaryTargetDetected();
    d.jarakGerak       = _radar.movingTargetDistance();
    d.energiGerak      = _radar.movingTargetEnergy();
    d.jarakDiam        = _radar.stationaryTargetDistance();
    d.energiDiam       = _radar.stationaryTargetEnergy();

    if (d.adaOrang) {
      if (d.bergerak && d.energiGerak >= RADAR_ENTRY_MOVING_THRESHOLD) {
        d.presenceState = "bergerak";
      } else {
        d.presenceState = "diam";
      }
    }
  }

  return d;
}

bool SensorManager::setRadarGate(uint8_t gateMaks, uint16_t timeoutDetik) {
  uint8_t gateFix = (gateMaks < 2) ? 2 : gateMaks;
  return _radar.setMaxValues(gateFix, gateFix, timeoutDetik);
}

void SensorManager::boostSensitivitasNapas() {
  _radar.setGateSensitivityThreshold(0, 65, 50);
  _radar.setGateSensitivityThreshold(1, 45, 30);
  _radar.setGateSensitivityThreshold(2, 30, RADAR_BREATHING_THRESHOLD);
  _radar.setGateSensitivityThreshold(3, 35, RADAR_BREATHING_THRESHOLD);
  _radar.setGateSensitivityThreshold(4, 40, 20);
  _radar.setGateSensitivityThreshold(5, 40, 20);
  _radar.setGateSensitivityThreshold(6, 45, 25);
}

bool SensorManager::isRadarConnected() {
  return _radar.isConnected();
}
