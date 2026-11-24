#include "BuzzerManager.h"

// ==============================================================================
// BuzzerManager.cpp - Implementasi Driver Buzzer Active-LOW & Prioritas Alarm
// ==============================================================================

BuzzerManager::BuzzerManager() : _muted(false), _mutedAtMs(0) {}

void BuzzerManager::begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  matikan();
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
  if (mute && !_muted) {
    _mutedAtMs = millis();
  }
  _muted = mute;
  if (_muted) {
    matikan();
  }
}

bool BuzzerManager::isMuted() const {
  return _muted;
}

bool BuzzerManager::isRearmDue() const {
  // UML UC-10: mute berlaku maksimal 60 detik, setelah itu alarm aktif lagi
  return _muted && (millis() - _mutedAtMs >= ALARM_REARM_MS);
}

AlarmStatus BuzzerManager::evaluasiAlarm(float suhu, float tempAlert, bool airBad, uint8_t aqi, bool warmingUp, bool adaOrang, bool jamMalam) {
  AlarmStatus st = { false, false, "normal", "none" };

  // Prioritas 1: Kehadiran di Jam Malam (FR-05)
  if (jamMalam && adaOrang) {
    st.active = true;
    st.reason = "night_presence";
    if (!_muted) {
      st.sounding = true;
      st.statusStr = "aktif";
      alarmJamMalam();
    } else {
      st.statusStr = "dibungkam";
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
      st.statusStr = "aktif";
      alarmSuhuTinggi();
    } else {
      st.statusStr = "dibungkam";
      matikan();
    }
    return st;
  }

  // Prioritas 3: Udara Buruk pasca warm-up 3 menit (FR-03 & NFR-06)
  if (!warmingUp && airBad) {
    st.active = true;
    st.reason = "air_bad";
    if (!_muted) {
      st.sounding = true;
      st.statusStr = "aktif";
      if (aqi >= 5) alarmAQI_Level5();
      else alarmAQI_Level4();
    } else {
      st.statusStr = "dibungkam";
      matikan();
    }
    return st;
  }

  matikan();
  return st;
}
