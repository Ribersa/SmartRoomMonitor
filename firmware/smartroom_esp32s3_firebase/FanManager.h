#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// FanManager.h - Pengendali Kipas 5V DC via MOSFET D4184 (SRS v0.9 FR-02)
// Mendukung threshold suhu dinamis dari /room1/thresholds di Firebase.
// ==============================================================================

enum class ModeKipas : uint8_t {
  OTOMATIS = 0,
  SELALU_ON = 1,
  SELALU_OFF = 2
};

class FanManager {
private:
  ModeKipas _mode;
  bool _fanState;
  bool _initialized;

  void terapkanState(bool nyala);

public:
  FanManager();
  void begin();

  void evaluasiSuhu(float suhu, float tempFanOn, float tempFanHysteresis);
  void setMode(ModeKipas mode);
  void setModeFromString(const char* modeStr);

  ModeKipas getMode() const { return _mode; }
  bool isNyala() const { return _fanState; }
  const char* getModeString() const;
};
