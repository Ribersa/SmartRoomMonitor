#pragma once
#include <Arduino.h>
#include <time.h>
#include "config.h"

// ==============================================================================
// TimeManager.h - Sinkronisasi Jam NTP WIB (UTC+7) & Evaluator Jam Malam FR-05
// ==============================================================================

class TimeManager {
private:
  bool _configured;

public:
  TimeManager();
  void begin();
  bool isSynced() const;
  uint64_t getEpochSeconds() const;
  bool isJamMalam(int startHour, int endHour) const;
  void formatJamWIB(char* bufferOut, size_t maxLen) const;
};
