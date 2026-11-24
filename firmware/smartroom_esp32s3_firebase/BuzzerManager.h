#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// BuzzerManager.h - Driver Buzzer Pasif Active-LOW & Evaluator Alarm SRS v0.9
// Menangani FR-01 (Suhu > 35°C), FR-03 (Udara Buruk pasca warm-up),
// FR-05 (Jam Malam 22:00-06:00), dan FR-06 (Alarm Lokal Offline + Mute Darurat).
// ==============================================================================

struct AlarmStatus {
  bool active;          // true jika ada kondisi bahaya terpicu (walau sedang di-mute)
  bool sounding;        // true jika buzzer sedang berbunyi (active && !muted)
  const char* reason;   // "none", "night_presence", "temp_high", "air_bad"
};

class BuzzerManager {
private:
  bool _muted;

public:
  BuzzerManager();
  void begin();
  void matikan();
  void mainkanNada(int frekuensi, int durasiMs);

  void nadaSambutBoot();
  void nadaKonfirmasi();
  void alarmSuhuTinggi();
  void alarmAQI_Level4();
  void alarmAQI_Level5();
  void alarmJamMalam();

  void setMuted(bool mute);
  bool isMuted() const;

  AlarmStatus evaluasiAlarm(float suhu, float tempAlert, uint8_t aqi, bool warmingUp, bool adaOrang, bool jamMalam);
};
