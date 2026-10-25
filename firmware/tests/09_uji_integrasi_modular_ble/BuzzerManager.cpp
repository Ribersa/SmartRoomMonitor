#include "BuzzerManager.h"

// ==============================================================================
// BuzzerManager.cpp - Implementasi Driver Buzzer Pasif Active-LOW
// ==============================================================================

BuzzerManager::BuzzerManager() : _muted(false) {}

void BuzzerManager::begin() {
  pinMode(PIN_BUZZER, OUTPUT);
  matikan(); // Kunci transistor PNP mati total (HIGH = Cutoff)
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
    digitalWrite(PIN_BUZZER, LOW);  // Transistor ON (tarik membran buzzer)
    delayMicroseconds(setengahPeriodeUs);
    digitalWrite(PIN_BUZZER, HIGH); // Transistor OFF (lepas membran)
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

void BuzzerManager::alarmAQI_Level4() {
  for (int i = 0; i < 2; i++) {
    mainkanNada(2200, 100);
    delay(60);
  }
  matikan();
}

void BuzzerManager::alarmAQI_Level5() {
  for (int i = 0; i < 3; i++) {
    mainkanNada(3500, 70);
    delay(20);
    mainkanNada(4200, 70);
    delay(20);
  }
  matikan();
}

void BuzzerManager::alarmJamMalam() {
  mainkanNada(2800, 90);
  delay(40);
  mainkanNada(3400, 90);
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

void BuzzerManager::evaluasiAlarm(uint8_t aqi, bool adaOrang, bool modeJamMalam) {
  if (_muted) {
    matikan();
    return;
  }

  // Prioritas 1: Jam Malam (Keberadaan tak diizinkan di malam hari)
  if (modeJamMalam && adaOrang) {
    alarmJamMalam();
    return;
  }

  // Prioritas 2: Bahaya Kritis Kualitas Udara Level 5
  if (aqi == 5) {
    alarmAQI_Level5();
    return;
  }

  // Prioritas 3: Peringatan Kualitas Udara Level 4
  if (aqi == 4) {
    alarmAQI_Level4();
    return;
  }

  // Kondisi Normal / Udara Aman
  matikan();
}
