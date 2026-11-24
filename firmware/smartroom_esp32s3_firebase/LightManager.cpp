#include "LightManager.h"

// ==============================================================================
// LightManager.cpp - Kontrol Lampu Adaptif FR-08 & Anti-Optical-Feedback Loop
// ==============================================================================

LightManager::LightManager()
  : _mode(ModeLampu::OTOMATIS),
    _lightState(false),
    _initialized(false),
    _perluKalibrasiCahayaSendiri(false),
    _luxPatokanSaatNyala(0.0f),
    _lastPresenceTime(0) {}

void LightManager::terapkanState(bool nyala) {
  if (_initialized && _lightState == nyala) {
    return;
  }

  bool baruMenyala = (!_lightState && nyala);
  _lightState = nyala;
  _initialized = true;

  if (_lightState) {
    pinMode(PIN_RELAY_LIGHT, OUTPUT);
    digitalWrite(PIN_RELAY_LIGHT, LOW); // Active-LOW tarik optocoupler ke GND
    if (baruMenyala) {
      _perluKalibrasiCahayaSendiri = true;
    }
  } else {
    // High-Impedance memutus arus total (mengatasi selisih 5V - 3.3V = 1.7V)
    pinMode(PIN_RELAY_LIGHT, INPUT);
    _perluKalibrasiCahayaSendiri = false;
  }
}

void LightManager::begin() {
  terapkanState(false);
  _lastPresenceTime = millis();
  Serial.println(F("[OK] LightManager (GPIO 15 Relay Open-Drain) siap. Mode: auto"));
}

void LightManager::evaluasi(float lux, bool adaOrang, float darkLux, float externalHysteresisLux, uint32_t emptyTimeoutSec) {
  unsigned long now = millis();
  unsigned long timeoutMs = (unsigned long)emptyTimeoutSec * 1000UL;

  switch (_mode) {
    case ModeLampu::SELALU_ON:
      terapkanState(true);
      break;

    case ModeLampu::SELALU_OFF:
      terapkanState(false);
      break;

    case ModeLampu::OTOMATIS:
    default:
      if (adaOrang) {
        _lastPresenceTime = now;
      }

      if (lux < 0.0f) {
        break;
      }

      if (!_lightState) {
        if (lux < darkLux && adaOrang) {
          terapkanState(true);
        }
      } else {
        if (_perluKalibrasiCahayaSendiri) {
          _luxPatokanSaatNyala = lux;
          _perluKalibrasiCahayaSendiri = false;
        } else if (lux < _luxPatokanSaatNyala) {
          _luxPatokanSaatNyala = lux;
        }

        if (lux >= (_luxPatokanSaatNyala + externalHysteresisLux)) {
          terapkanState(false);
          break;
        }

        if (!adaOrang && (now - _lastPresenceTime >= timeoutMs)) {
          terapkanState(false);
        }
      }
      break;
  }
}

void LightManager::setMode(ModeLampu mode) {
  _mode = mode;
  if (_mode == ModeLampu::SELALU_ON) {
    terapkanState(true);
  } else if (_mode == ModeLampu::SELALU_OFF) {
    terapkanState(false);
  }
}

void LightManager::setModeFromString(const char* modeStr) {
  if (modeStr == nullptr) return;
  if (strcmp(modeStr, "on") == 0) {
    setMode(ModeLampu::SELALU_ON);
  } else if (strcmp(modeStr, "off") == 0) {
    setMode(ModeLampu::SELALU_OFF);
  } else {
    setMode(ModeLampu::OTOMATIS);
  }
}

const char* LightManager::getModeString() const {
  switch (_mode) {
    case ModeLampu::SELALU_ON:  return "on";
    case ModeLampu::SELALU_OFF: return "off";
    case ModeLampu::OTOMATIS:
    default:                    return "auto";
  }
}
