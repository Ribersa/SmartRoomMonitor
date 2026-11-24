#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// LightManager.h - Pengendali Relay Lampu 5V Adaptif + Kompensasi Optik (FR-08)
// Menggunakan Open-Drain / Hi-Z switching untuk mencegah kebocoran arus optocoupler 5V.
// ==============================================================================

enum class ModeLampu : uint8_t {
  OTOMATIS = 0,
  SELALU_ON = 1,
  SELALU_OFF = 2
};

class LightManager {
private:
  ModeLampu _mode;
  bool _lightState;
  bool _initialized;
  bool _perluKalibrasiCahayaSendiri;
  float _luxPatokanSaatNyala;
  unsigned long _lastPresenceTime;

  void terapkanState(bool nyala);

public:
  LightManager();
  void begin();

  void evaluasi(float lux, bool adaOrang, float darkLux, float externalHysteresisLux, uint32_t emptyTimeoutSec);
  void setMode(ModeLampu mode);
  void setModeFromString(const char* modeStr);

  ModeLampu getMode() const { return _mode; }
  bool isNyala() const { return _lightState; }
  const char* getModeString() const;
};
