#include "FanManager.h"

// ==============================================================================
// FanManager.cpp - Implementasi Logika Kipas & Histeresis Termal FR-02
// ==============================================================================

FanManager::FanManager() : _mode(ModeKipas::OTOMATIS), _fanState(false), _initialized(false) {}

void FanManager::terapkanState(bool nyala) {
  // Cegah re-trigger berulang jika state tidak berubah
  if (_initialized && _fanState == nyala) {
    return;
  }

  bool transisiNyala = (!_fanState && nyala && _initialized);
  _fanState = nyala;
  _initialized = true;

  if (transisiNyala) {
    // Soft-Start Ramp (160 ms) untuk meredam lonjakan arus kejut (inrush current)
    // motor kipas DC yang kerap memicu brownout reset pada rel 5V ESP32
    for (int duty = 64; duty <= 255; duty += 32) {
      analogWrite(PIN_FAN_MOSFET, duty);
      delay(25);
    }
  }

  analogWrite(PIN_FAN_MOSFET, _fanState ? 255 : 0);
  digitalWrite(PIN_FAN_MOSFET, _fanState ? HIGH : LOW);
}

void FanManager::begin() {
  pinMode(PIN_FAN_MOSFET, OUTPUT);
  terapkanState(false); // Pastikan kipas mati saat boot awal
  Serial.println(F("✅ [OK] FanManager (GPIO 7 MOSFET) Siap! Mode: OTOMATIS"));
}

void FanManager::evaluasiSuhu(float suhu) {
  if (isnan(suhu)) {
    // Sensor suhu error, jangan ubah state untuk keamanan
    return;
  }

  switch (_mode) {
    case ModeKipas::SELALU_ON:
      terapkanState(true);
      break;

    case ModeKipas::SELALU_OFF:
      terapkanState(false);
      break;

    case ModeKipas::OTOMATIS:
    default:
      // Logika Histeresis FR-02:
      // Nyala saat suhu > 31.0 °C
      if (suhu > SUHU_KIPAS_ON) {
        terapkanState(true);
      }
      // Mati saat suhu <= (31.0 - 1.0) = 30.0 °C
      else if (suhu <= (SUHU_KIPAS_ON - SUHU_KIPAS_HISTERESIS)) {
        terapkanState(false);
      }
      // Di antara 30.0 °C dan 31.0 °C: pertahankan state sebelumnya
      break;
  }
}

void FanManager::setMode(ModeKipas mode) {
  _mode = mode;
  if (_mode == ModeKipas::SELALU_ON) {
    terapkanState(true);
  } else if (_mode == ModeKipas::SELALU_OFF) {
    terapkanState(false);
  }
}

void FanManager::cycleMode() {
  if (_mode == ModeKipas::OTOMATIS) {
    setMode(ModeKipas::SELALU_ON);
  } else if (_mode == ModeKipas::SELALU_ON) {
    setMode(ModeKipas::SELALU_OFF);
  } else {
    setMode(ModeKipas::OTOMATIS);
  }
}

const char* FanManager::getModeLabel() const {
  switch (_mode) {
    case ModeKipas::OTOMATIS:   return "OTOMATIS";
    case ModeKipas::SELALU_ON:  return "SELALU ON";
    case ModeKipas::SELALU_OFF: return "SELALU OFF";
    default:                    return "UNKNOWN";
  }
}

const char* FanManager::getStatusLabel() const {
  switch (_mode) {
    case ModeKipas::OTOMATIS:
      return _fanState ? "AUTO [NYALA]" : "AUTO [MATI]";
    case ModeKipas::SELALU_ON:
      return "MANUAL [NYALA]";
    case ModeKipas::SELALU_OFF:
      return "MANUAL [MATI]";
    default:
      return "UNKNOWN";
  }
}
