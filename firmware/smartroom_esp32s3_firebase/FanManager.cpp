#include "FanManager.h"

// ==============================================================================
// FanManager.cpp - Implementasi Histeresis Termal Kipas FR-02
// ==============================================================================

FanManager::FanManager()
  : _mode(ModeKipas::OTOMATIS), _fanState(false), _initialized(false) {}

void FanManager::terapkanState(bool nyala) {
  if (_initialized && _fanState == nyala) {
    return;
  }
  _fanState = nyala;
  _initialized = true;
  digitalWrite(PIN_FAN_MOSFET, _fanState ? HIGH : LOW);
}

void FanManager::begin() {
  pinMode(PIN_FAN_MOSFET, OUTPUT);
  terapkanState(false);
  Serial.println(F("[OK] FanManager (GPIO 7 MOSFET) siap. Mode: auto"));
}

void FanManager::evaluasiSuhu(float suhu, float tempFanOn, float tempFanHysteresis) {
  switch (_mode) {
    case ModeKipas::SELALU_ON:
      terapkanState(true);
      break;

    case ModeKipas::SELALU_OFF:
      terapkanState(false);
      break;

    case ModeKipas::OTOMATIS:
    default:
      if (isnan(suhu)) {
        return;
      }
      if (suhu > tempFanOn) {
        terapkanState(true);
      } else if (suhu <= (tempFanOn - tempFanHysteresis)) {
        terapkanState(false);
      }
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

void FanManager::setModeFromString(const char* modeStr) {
  if (modeStr == nullptr) return;
  if (strcmp(modeStr, "on") == 0) {
    setMode(ModeKipas::SELALU_ON);
  } else if (strcmp(modeStr, "off") == 0) {
    setMode(ModeKipas::SELALU_OFF);
  } else {
    setMode(ModeKipas::OTOMATIS);
  }
}

const char* FanManager::getModeString() const {
  switch (_mode) {
    case ModeKipas::SELALU_ON:  return "on";
    case ModeKipas::SELALU_OFF: return "off";
    case ModeKipas::OTOMATIS:
    default:                    return "auto";
  }
}
