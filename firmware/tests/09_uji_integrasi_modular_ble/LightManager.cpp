#include "LightManager.h"

// ==============================================================================
// LightManager.cpp - Implementasi Logika Kontrol Lampu Adaptif FR-08
// Menggunakan teknik Open-Drain / Hi-Z Switching untuk mengatasi kebocoran
// tegangan 1.7V optocoupler modul relay 5V saat dikendalikan GPIO 3.3V ESP32.
// ==============================================================================

LightManager::LightManager()
  : _mode(ModeLampu::OTOMATIS),
    _lightState(false),
    _initialized(false),
    _lastPresenceTime(0) {}

void LightManager::terapkanState(bool nyala) {
  // Cegah re-konfigurasi pinMode berulang setiap siklus loop jika state sama
  if (_initialized && _lightState == nyala) {
    return;
  }

  _lightState = nyala;
  _initialized = true;

  if (_lightState) {
    // NYALA: Tarik pin ke GND (Active-LOW) untuk menyalakan optocoupler & relay
    pinMode(PIN_RELAY_LIGHT, OUTPUT);
    digitalWrite(PIN_RELAY_LIGHT, LOW);
  } else {
    // MATI: Ubah pin ke mode INPUT (High-Impedance / Mengambang).
    // Ini memutus arus total dan mencegah selisih voltase (5V - 3.3V = 1.7V)
    // yang membuat optocoupler menyala terus-menerus.
    pinMode(PIN_RELAY_LIGHT, INPUT);
  }
}

void LightManager::begin() {
  terapkanState(false); // Pastikan relay lepas / lampu mati saat boot awal
  _lastPresenceTime = millis();
  Serial.println(F("✅ [OK] LightManager (GPIO 15 Relay) Siap! Mode: OTOMATIS"));
}

void LightManager::evaluasi(float lux, bool adaOrang) {
  unsigned long now = millis();

  switch (_mode) {
    case ModeLampu::SELALU_ON:
      terapkanState(true);
      break;

    case ModeLampu::SELALU_OFF:
      terapkanState(false);
      break;

    case ModeLampu::OTOMATIS:
    default:
      // Catat waktu terakhir ada orang
      if (adaOrang) {
        _lastPresenceTime = now;
      }

      // 1. Kondisi Ruangan Terang (>= 50 Lux): Lampu WAJIB MATI (hemat energi)
      if (lux >= 0.0 && lux >= AMBANG_CAHAYA_GELAP_LUX) {
        terapkanState(false);
        break;
      }

      // 2. Kondisi Ruangan Gelap (< 50 Lux):
      if (lux >= 0.0 && lux < AMBANG_CAHAYA_GELAP_LUX) {
        if (adaOrang) {
          // Gelap DAN Ada Orang -> Nyalakan Lampu!
          terapkanState(true);
        } else {
          // Gelap tapi Ruangan Kosong -> Cek apakah sudah melebihi timeout kekosongan
          if (now - _lastPresenceTime >= TIMEOUT_KOSONG_LAMPU_MS) {
            terapkanState(false); // Padamkan lampu karena orang sudah pergi
          }
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

void LightManager::cycleMode() {
  if (_mode == ModeLampu::OTOMATIS) {
    setMode(ModeLampu::SELALU_ON);
  } else if (_mode == ModeLampu::SELALU_ON) {
    setMode(ModeLampu::SELALU_OFF);
  } else {
    setMode(ModeLampu::OTOMATIS);
  }
}

const char* LightManager::getModeLabel() const {
  switch (_mode) {
    case ModeLampu::OTOMATIS:   return "OTOMATIS";
    case ModeLampu::SELALU_ON:  return "SELALU ON";
    case ModeLampu::SELALU_OFF: return "SELALU OFF";
    default:                    return "UNKNOWN";
  }
}

const char* LightManager::getStatusLabel() const {
  switch (_mode) {
    case ModeLampu::OTOMATIS:
      return _lightState ? "AUTO [NYALA]" : "AUTO [MATI]";
    case ModeLampu::SELALU_ON:
      return "MANUAL [NYALA]";
    case ModeLampu::SELALU_OFF:
      return "MANUAL [MATI]";
    default:
      return "UNKNOWN";
  }
}
