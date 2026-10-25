#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// BuzzerManager.h - Pengendali Driver Buzzer Pasif Active-LOW (Transistor PNP)
// ==============================================================================

class BuzzerManager {
private:
  bool _muted;

public:
  BuzzerManager();
  void begin();
  void matikan();
  void mainkanNada(int frekuensi, int durasiMs);

  // Pola nada fungsional
  void nadaSambutBoot();
  void nadaKonfirmasi();
  void alarmAQI_Level4();
  void alarmAQI_Level5();
  void alarmJamMalam();

  // Manajemen status MUTE darurat (FR-06)
  void setMuted(bool mute);
  bool isMuted() const;

  // Evaluator alarm terpusat
  void evaluasiAlarm(uint8_t aqi, bool adaOrang, bool modeJamMalam);
};
