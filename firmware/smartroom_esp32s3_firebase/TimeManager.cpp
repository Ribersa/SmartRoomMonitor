#include "TimeManager.h"

// ==============================================================================
// TimeManager.cpp - Implementasi Sinkronisasi NTP WIB & Evaluasi Jam Malam FR-05
// ==============================================================================

static const time_t MIN_VALID_EPOCH = 1700000000L; // > Nov 2023 menandakan NTP valid

TimeManager::TimeManager() : _configured(false) {}

void TimeManager::begin() {
  if (_configured) {
    return;
  }
  configTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER_PRIMARY, NTP_SERVER_SECONDARY);
  _configured = true;
}

bool TimeManager::isSynced() const {
  time_t now = time(nullptr);
  return (now >= MIN_VALID_EPOCH);
}

uint64_t TimeManager::getEpochSeconds() const {
  time_t now = time(nullptr);
  if (now >= MIN_VALID_EPOCH) {
    return (uint64_t)now;
  }
  return 0ULL;
}

bool TimeManager::isJamMalam(int startHour, int endHour) const {
  time_t now = time(nullptr);
  if (now < MIN_VALID_EPOCH) {
    // Belum tersinkron NTP (mis. boot offline): jangan memicu alarm jam malam palsu
    return false;
  }

  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  int jam = timeinfo.tm_hour;

  if (startHour == endHour) {
    return false;
  }

  if (startHour > endHour) {
    // Rentang melewati tengah malam (contoh: 22:00 s.d. 06:00)
    return (jam >= startHour || jam < endHour);
  }
  return (jam >= startHour && jam < endHour);
}

void TimeManager::formatJamWIB(char* bufferOut, size_t maxLen) const {
  time_t now = time(nullptr);
  if (now < MIN_VALID_EPOCH) {
    snprintf(bufferOut, maxLen, "--:-- WIB");
    return;
  }

  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  snprintf(bufferOut, maxLen, "%02d:%02d WIB", timeinfo.tm_hour, timeinfo.tm_min);
}
