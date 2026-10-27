#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// LightManager.h - Pengendali Lampu Ruangan via Modul Relay 1-Channel 5V
// Kepatuhan: SRS v0.9 Kebutuhan Fungsional FR-08 (Kontrol Lampu Adaptif)
// ==============================================================================

enum class ModeLampu {
  OTOMATIS,    // Nyala saat Gelap (< 50 Lux) DAN Ada Orang; Mati saat Terang / Kosong
  SELALU_ON,   // Selalu menyala
  SELALU_OFF   // Selalu mati total
};

class LightManager {
private:
  ModeLampu _mode;
  bool _lightState; // true = nyala (relay aktif), false = mati
  bool _initialized;
  bool _perluKalibrasiCahayaSendiri;
  float _luxPatokanSaatNyala;
  unsigned long _lastPresenceTime;

  void terapkanState(bool nyala);

public:
  LightManager();
  void begin();

  // Evaluasi kondisi cahaya (BH1750) dan keberadaan (LD2410) sesuai FR-08
  void evaluasi(float lux, bool adaOrang);

  // Ganti mode operasional
  void setMode(ModeLampu mode);
  void cycleMode(); // Ganti berputar: OTOMATIS -> SELALU_ON -> SELALU_OFF -> OTOMATIS

  ModeLampu getMode() const { return _mode; }
  bool isNyala() const { return _lightState; }

  const char* getModeLabel() const;
  const char* getStatusLabel() const;
};
