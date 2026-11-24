#include "BuzzerManager.h"

// ==============================================================================
// BuzzerManager.cpp - Implementasi Driver Buzzer Active-LOW & Prioritas Alarm
// ==============================================================================

BuzzerManager::BuzzerManager() : _muted(false) {}

void BuzzerManager::begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  matikan(); // HIGH = cut-off transistor PNP (bebas dengung)
}

void BuzzerManager::matikan() {
  digitalWrite(PIN_BUZZER, HIGH);
}

void BuzzerManager::mainkanNada(int frekuensi, int durasiMs) {
  if (_muted || frekuensi <= 0 || durasiMs <= 0) {
    matikan();
    return;
  }

  long periodeUs = 1000000L / frekuensi;
  long setengahPeriodeUs = periodeUs / 2;
  long totalSiklus = (long)frekuensi * durasiMs / 1000L;

  for (long i = 0; i < totalSiklus; i++) {
    digitalWrite(PIN_BUZZER, LOW);
    delayMicroseconds(setengahPeriodeUs);
    digitalWrite(PIN_BUZZER, HIGH);
    delayMicroseconds(setengahPeriodeUs);
  }
  matikan();
}

void BuzzerManager::nadaSambutBoot() {
  mainkanNada(2000, 60);
  delay(40);
  mainkanNada(2800, 80);
  matikan();
}

void BuzzerManager::nadaKonfirmasi() {
  mainkanNada(2400, 60);
}

void BuzzerManager::alarmSuhuTinggi() {
  for (int i = 0; i < 2; i++) {
    mainkanNada(2600, 90);
    delay(45);
  }
  matikan();
}

void BuzzerManager::alarmAQI_Level4() {
  for (int i = 0; i < 2; i++) {
    mainkanNada(2200, 90);
    delay(50);
  }
  matikan();
}

void BuzzerManager::alarmAQI_Level5() {
  for (int i = 0; i < 3; i++) {
    mainkanNada(3500, 60);
    delay(20);
    mainkanNada(4200, 60);
    delay(20);
  }
  matikan();
}

void BuzzerManager::alarmJamMalam() {
  mainkanNada(2800, 85);
  delay(35);
  mainkanNada(3400, 85);
  matikan();
}

void BuzzerManager::setMuted(bool mute) {
  _muted = mute;
  if (_muted) {
    matikan();
  }
}

bool BuzzerManager::isMuted() const {
  return _muted;
}

AlarmStatus BuzzerManager::evaluasiAlarm(float suhu, float tempAlert, uint8_t aqi, bool warmingUp, bool adaOrang, bool jamMalam) {
  AlarmStatus st = { false, false, "none" };

  // Prioritas 1: Kehadiran di Jam Malam (FR-05)
  if (jamMalam && adaOrang) {
    st.active = true;
    st.reason = "night_presence";
    if (!_muted) {
      st.sounding = true;
      alarmJamMalam();
    } else {
      matikan();
    }
    return st;
  }

  // Prioritas 2: Suhu Kritis > 35°C (FR-01)
  if (!isnan(suhu) && suhu > tempAlert) {
    st.active = true;
    st.reason = "temp_high";
    if (!_muted) {
      st.sounding = true;
      alarmSuhuTinggi();
    } else {
      matikan();
    }
    return st;
  }

  // Prioritas 3: Udara Buruk AQI 4-5 setelah masa pemanasan 3 menit selesai (FR-03 & NFR-06)
  if (!warmingUp && aqi >= 4) {
    st.active = true;
    st.reason = "air_bad";
    if (!_muted) {
      st.sounding = true;
      if (aqi >= 5) alarmAQI_Level5();
      else alarmAQI_Level4();
    } else {
      matikan();
    }
    return st;
  }

  matikan();
  return st;
}
