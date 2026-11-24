#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config.h"

// ==============================================================================
// SharedState.h - Pertukaran Data Thread-Safe antara Core 1 (Hardware) & Core 0 (Cloud)
// Diselaraskan dengan struktur JSON UML (/room1/current, /actuator, /alarm, /thresholds).
// ==============================================================================

struct CurrentSnapshot {
  bool valid;
  float temperature;
  float humidity;
  char humidityLabel[16];

  uint16_t airDelta;
  char airLabel[24];       // "Memanaskan", "Baik", "Sedang", "Buruk"
  uint8_t airAqi;
  uint16_t airTvoc;
  uint16_t airEco2;
  bool airWarmingUp;

  float lightLux;
  char lightLabel[16];     // "Gelap", "Terang"

  char presence[16];       // "tidak ada", "bergerak", "diam"
  bool presenceDetected;

  bool fanStatus;
  bool lampStatus;
  bool nightModeActive;

  uint64_t timestampMs;    // Unix epoch dalam milidetik (13 digit, cocok System.currentTimeMillis)
};

struct ActuatorControlState {
  char fanMode[8];         // "auto", "on", "off" (dari /room1/actuator/fanMode)
  char lampMode[8];        // "auto", "on", "off" (dari /room1/actuator/lampMode)
  bool alarmMute;          // dari /room1/alarm/mute
  bool hasRemoteUpdate;
  bool muteLocalResetDirty;
};

struct AlarmStateSnapshot {
  bool active;
  bool buzzer;
  char status[16];         // "normal", "aktif", "dibungkam"
  char reason[24];         // "none", "night_presence", "temp_high", "air_bad"
  bool dirty;
};

struct ThresholdSnapshot {
  float tempFanOn;
  float tempFanHysteresis;
  float tempAlert;
  uint16_t airDeltaModerate;
  uint16_t airDeltaBad;
  float lightDarkLux;
  float lightExternalHysteresisLux;
  uint32_t lampEmptyTimeoutSec;
  int nightStartHour;
  int nightEndHour;
};

struct EventItem {
  uint64_t timestampMs;
  char type[24];           // "temp_high", "air_bad", "night_presence"
  char severity[16];       // "warning", "danger", "info"
  char message[128];
};

struct NetworkStatusSnapshot {
  bool wifiConnected;
  bool firebaseAuthed;
  bool ntpSynced;
  int lastHttpCode;
  unsigned long lastSuccessPushMs;
};

class SharedState {
private:
  SemaphoreHandle_t _mutex;
  CurrentSnapshot _current;
  ActuatorControlState _controls;
  AlarmStateSnapshot _alarm;
  ThresholdSnapshot _thresholds;
  NetworkStatusSnapshot _netStatus;

  static const uint8_t MAX_EVENTS = 8;
  EventItem _eventQueue[MAX_EVENTS];
  uint8_t _eventHead;
  uint8_t _eventTail;
  uint8_t _eventCount;

public:
  SharedState() : _mutex(nullptr), _eventHead(0), _eventTail(0), _eventCount(0) {
    memset(&_current, 0, sizeof(_current));
    strcpy(_current.humidityLabel, "Nyaman");
    strcpy(_current.airLabel, "Memanaskan");
    strcpy(_current.lightLabel, "Terang");
    strcpy(_current.presence, "tidak ada");
    _current.airWarmingUp = true;

    memset(&_controls, 0, sizeof(_controls));
    strcpy(_controls.fanMode, "auto");
    strcpy(_controls.lampMode, "auto");

    memset(&_alarm, 0, sizeof(_alarm));
    strcpy(_alarm.status, "normal");
    strcpy(_alarm.reason, "none");
    _alarm.dirty = true; // Sinkronkan status awal /room1/alarm saat boot

    _thresholds.tempFanOn = SUHU_KIPAS_ON_DEFAULT;
    _thresholds.tempFanHysteresis = SUHU_KIPAS_HISTERESIS_DEFAULT;
    _thresholds.tempAlert = SUHU_ALERT_DEFAULT;
    _thresholds.airDeltaModerate = AIR_DELTA_MODERATE_DEFAULT;
    _thresholds.airDeltaBad = AIR_DELTA_BAD_DEFAULT;
    _thresholds.lightDarkLux = AMBANG_CAHAYA_GELAP_LUX_DEFAULT;
    _thresholds.lightExternalHysteresisLux = HISTERESIS_CAHAYA_EKSTERNAL_LUX_DEFAULT;
    _thresholds.lampEmptyTimeoutSec = TIMEOUT_KOSONG_LAMPU_SEC_DEFAULT;
    _thresholds.nightStartHour = JAM_MALAM_MULAI_DEFAULT;
    _thresholds.nightEndHour = JAM_MALAM_SELESAI_DEFAULT;

    memset(&_netStatus, 0, sizeof(_netStatus));
  }

  void begin() {
    if (_mutex == nullptr) {
      _mutex = xSemaphoreCreateMutex();
    }
  }

  void setCurrent(const CurrentSnapshot& snap) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      _current = snap;
      xSemaphoreGive(_mutex);
    }
  }

  bool getCurrent(CurrentSnapshot& out) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      out = _current;
      xSemaphoreGive(_mutex);
      return out.valid;
    }
    return false;
  }

  void setRemoteActuatorModes(const char* fanMode, const char* lampMode) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      strncpy(_controls.fanMode, fanMode, sizeof(_controls.fanMode) - 1);
      _controls.fanMode[sizeof(_controls.fanMode) - 1] = '\0';
      strncpy(_controls.lampMode, lampMode, sizeof(_controls.lampMode) - 1);
      _controls.lampMode[sizeof(_controls.lampMode) - 1] = '\0';
      _controls.hasRemoteUpdate = true;
      xSemaphoreGive(_mutex);
    }
  }

  void setRemoteAlarmMute(bool muted) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (!_controls.muteLocalResetDirty) {
        _controls.alarmMute = muted;
        _controls.hasRemoteUpdate = true;
      }
      xSemaphoreGive(_mutex);
    }
  }

  bool consumeRemoteControls(ActuatorControlState& out) {
    bool updated = false;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      out = _controls;
      updated = _controls.hasRemoteUpdate;
      _controls.hasRemoteUpdate = false;
      xSemaphoreGive(_mutex);
    }
    return updated;
  }

  void requestLocalMuteUpdate(bool muted) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      _controls.alarmMute = muted;
      _controls.muteLocalResetDirty = true;
      _alarm.dirty = true;
      xSemaphoreGive(_mutex);
    }
  }

  void setAlarmState(bool active, bool buzzer, const char* statusStr, const char* reasonStr) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      bool changed = (_alarm.active != active) ||
                     (_alarm.buzzer != buzzer) ||
                     (strcmp(_alarm.status, statusStr) != 0) ||
                     (strcmp(_alarm.reason, reasonStr) != 0);
      if (changed) {
        _alarm.active = active;
        _alarm.buzzer = buzzer;
        strncpy(_alarm.status, statusStr, sizeof(_alarm.status) - 1);
        _alarm.status[sizeof(_alarm.status) - 1] = '\0';
        strncpy(_alarm.reason, reasonStr, sizeof(_alarm.reason) - 1);
        _alarm.reason[sizeof(_alarm.reason) - 1] = '\0';
        _alarm.dirty = true;
      }
      xSemaphoreGive(_mutex);
    }
  }

  bool consumeAlarmDirty(AlarmStateSnapshot& outAlarm, bool& muteDirty, bool& muteVal) {
    bool anyDirty = false;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      anyDirty = _alarm.dirty || _controls.muteLocalResetDirty;
      outAlarm = _alarm;
      muteDirty = _controls.muteLocalResetDirty;
      muteVal = _controls.alarmMute;
      _alarm.dirty = false;
      _controls.muteLocalResetDirty = false;
      xSemaphoreGive(_mutex);
    }
    return anyDirty;
  }

  void setThresholds(const ThresholdSnapshot& th) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      _thresholds = th;
      xSemaphoreGive(_mutex);
    }
  }

  ThresholdSnapshot getThresholds() {
    ThresholdSnapshot out = _thresholds;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      out = _thresholds;
      xSemaphoreGive(_mutex);
    }
    return out;
  }

  bool pushEvent(uint64_t timestampMs, const char* type, const char* severity, const char* message) {
    bool ok = false;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (_eventCount == MAX_EVENTS) {
        _eventHead = (_eventHead + 1) % MAX_EVENTS;
        _eventCount--;
      }
      EventItem& item = _eventQueue[_eventTail];
      item.timestampMs = timestampMs;
      strncpy(item.type, type, sizeof(item.type) - 1);
      item.type[sizeof(item.type) - 1] = '\0';
      strncpy(item.severity, severity, sizeof(item.severity) - 1);
      item.severity[sizeof(item.severity) - 1] = '\0';
      strncpy(item.message, message, sizeof(item.message) - 1);
      item.message[sizeof(item.message) - 1] = '\0';

      _eventTail = (_eventTail + 1) % MAX_EVENTS;
      _eventCount++;
      ok = true;
      xSemaphoreGive(_mutex);
    }
    return ok;
  }

  bool peekEvent(EventItem& out) {
    bool has = false;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (_eventCount > 0) {
        out = _eventQueue[_eventHead];
        has = true;
      }
      xSemaphoreGive(_mutex);
    }
    return has;
  }

  void dropFrontEvent() {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (_eventCount > 0) {
        _eventHead = (_eventHead + 1) % MAX_EVENTS;
        _eventCount--;
      }
      xSemaphoreGive(_mutex);
    }
  }

  void setNetworkStatus(const NetworkStatusSnapshot& st) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      _netStatus = st;
      xSemaphoreGive(_mutex);
    }
  }

  NetworkStatusSnapshot getNetworkStatus() {
    NetworkStatusSnapshot out = _netStatus;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      out = _netStatus;
      xSemaphoreGive(_mutex);
    }
    return out;
  }
};
