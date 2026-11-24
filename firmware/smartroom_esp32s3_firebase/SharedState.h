#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config.h"

// ==============================================================================
// SharedState.h - Pertukaran Data Thread-Safe antara Core 1 (Hardware) & Core 0 (Cloud)
// Menggunakan FreeRTOS Mutex tanpa alokasi String dinamis untuk mencegah heap fragmentation.
// ==============================================================================

struct TelemetrySnapshot {
  bool valid;
  float temperature;
  float humidity;
  char humidityLabel[16];

  uint8_t airQualityAqi;
  uint16_t airQualityTvoc;
  uint16_t airQualityEco2;
  bool airQualityWarmingUp;
  char airQualityLabel[24];

  float lightLux;
  char lightLabel[16];

  bool presenceDetected;
  char presenceState[16]; // "tidak ada", "bergerak", "diam"

  bool alarmActive;
  char alarmReason[24];   // "none", "night_presence", "temp_high", "air_bad"
  bool nightModeActive;

  uint64_t deviceTimestamp;
};

struct ControlSnapshot {
  // Perintah dari Staff (Firebase -> ESP32)
  char fanMode[8];        // "auto", "on", "off"
  char lampMode[8];       // "auto", "on", "off"
  bool alarmMuted;
  bool hasRemoteUpdate;

  // Status aktual aktuator (ESP32 -> Firebase)
  bool fanStatus;
  bool lampStatus;
  bool actuatorStatusDirty;
  bool alarmMuteLocalResetDirty;
};

struct ThresholdSnapshot {
  float tempFanOn;
  float tempFanHysteresis;
  float tempAlert;
  float lightDarkLux;
  float lightExternalHysteresisLux;
  uint32_t lampEmptyTimeoutSec;
  int nightStartHour;
  int nightEndHour;
};

struct EventItem {
  uint64_t timestamp;
  char type[24];      // "temp_high", "air_bad", "night_presence"
  char severity[16];  // "warning", "danger", "info"
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
  TelemetrySnapshot _telemetry;
  ControlSnapshot _controls;
  ThresholdSnapshot _thresholds;
  NetworkStatusSnapshot _netStatus;

  static const uint8_t MAX_EVENTS = 8;
  EventItem _eventQueue[MAX_EVENTS];
  uint8_t _eventHead;
  uint8_t _eventTail;
  uint8_t _eventCount;

public:
  SharedState() : _mutex(nullptr), _eventHead(0), _eventTail(0), _eventCount(0) {
    memset(&_telemetry, 0, sizeof(_telemetry));
    strcpy(_telemetry.humidityLabel, "Nyaman");
    strcpy(_telemetry.airQualityLabel, "Memanaskan");
    strcpy(_telemetry.lightLabel, "Normal");
    strcpy(_telemetry.presenceState, "tidak ada");
    strcpy(_telemetry.alarmReason, "none");
    _telemetry.airQualityWarmingUp = true;

    memset(&_controls, 0, sizeof(_controls));
    strcpy(_controls.fanMode, "auto");
    strcpy(_controls.lampMode, "auto");
    _controls.actuatorStatusDirty = true; // Sinkronkan status awal aktuator ke RTDB saat boot

    _thresholds.tempFanOn = SUHU_KIPAS_ON_DEFAULT;
    _thresholds.tempFanHysteresis = SUHU_KIPAS_HISTERESIS_DEFAULT;
    _thresholds.tempAlert = SUHU_ALERT_DEFAULT;
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

  void setTelemetry(const TelemetrySnapshot& snap) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      _telemetry = snap;
      xSemaphoreGive(_mutex);
    }
  }

  bool getTelemetry(TelemetrySnapshot& out) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      out = _telemetry;
      xSemaphoreGive(_mutex);
      return out.valid;
    }
    return false;
  }

  void setRemoteControls(const char* fanMode, const char* lampMode, bool alarmMuted) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      strncpy(_controls.fanMode, fanMode, sizeof(_controls.fanMode) - 1);
      _controls.fanMode[sizeof(_controls.fanMode) - 1] = '\0';
      strncpy(_controls.lampMode, lampMode, sizeof(_controls.lampMode) - 1);
      _controls.lampMode[sizeof(_controls.lampMode) - 1] = '\0';
      if (!_controls.alarmMuteLocalResetDirty) {
        _controls.alarmMuted = alarmMuted;
      }
      _controls.hasRemoteUpdate = true;
      xSemaphoreGive(_mutex);
    }
  }

  bool consumeRemoteControls(ControlSnapshot& out) {
    bool updated = false;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      out = _controls;
      updated = _controls.hasRemoteUpdate;
      _controls.hasRemoteUpdate = false;
      xSemaphoreGive(_mutex);
    }
    return updated;
  }

  void setActuatorStatus(bool fanStatus, bool lampStatus) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (_controls.fanStatus != fanStatus || _controls.lampStatus != lampStatus) {
        _controls.fanStatus = fanStatus;
        _controls.lampStatus = lampStatus;
        _controls.actuatorStatusDirty = true;
      }
      xSemaphoreGive(_mutex);
    }
  }

  void requestLocalMuteUpdate(bool muted) {
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      _controls.alarmMuted = muted;
      _controls.alarmMuteLocalResetDirty = true;
      xSemaphoreGive(_mutex);
    }
  }

  bool consumeActuatorStatusDirty(bool& fanStatus, bool& lampStatus, bool& muteDirty, bool& muteVal) {
    bool anyDirty = false;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      anyDirty = _controls.actuatorStatusDirty || _controls.alarmMuteLocalResetDirty;
      fanStatus = _controls.fanStatus;
      lampStatus = _controls.lampStatus;
      muteDirty = _controls.alarmMuteLocalResetDirty;
      muteVal = _controls.alarmMuted;
      _controls.actuatorStatusDirty = false;
      _controls.alarmMuteLocalResetDirty = false;
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

  bool pushEvent(uint64_t timestamp, const char* type, const char* severity, const char* message) {
    bool ok = false;
    if (_mutex && xSemaphoreTake(_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (_eventCount == MAX_EVENTS) {
        // Buang event tertua jika antrean penuh saat offline lama
        _eventHead = (_eventHead + 1) % MAX_EVENTS;
        _eventCount--;
      }
      EventItem& item = _eventQueue[_eventTail];
      item.timestamp = timestamp;
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
