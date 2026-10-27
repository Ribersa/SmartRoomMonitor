#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// FanManager.h - Pengendali Aktuator Kipas 5V DC via Driver MOSFET D4184
// Kepatuhan: SRS v0.9 Kebutuhan Fungsional FR-02 (Kontrol Kipas & Histeresis)
// ==============================================================================

enum class ModeKipas {
  OTOMATIS,    // Nyala saat suhu > 31 °C, mati saat <= 30 °C
  SELALU_ON,   // Selalu menyala 100%
  SELALU_OFF   // Selalu mati total
};

class FanManager {
private:
  ModeKipas _mode;
  bool _fanState; // true = nyala, false = mati
  bool _initialized;

  void terapkanState(bool nyala);

public:
  FanManager();
  void begin();

  // Evaluasi suhu real-time dengan histeresis FR-02
  void evaluasiSuhu(float suhu);

  // Ganti mode operasional
  void setMode(ModeKipas mode);
  void cycleMode(); // Ganti berputar: OTOMATIS -> SELALU_ON -> SELALU_OFF -> OTOMATIS

  ModeKipas getMode() const { return _mode; }
  bool isNyala() const { return _fanState; }

  const char* getModeLabel() const;
  const char* getStatusLabel() const;
};
