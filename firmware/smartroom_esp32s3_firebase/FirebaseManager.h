#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "SharedState.h"
#include "TimeManager.h"

// ==============================================================================
// FirebaseManager.h - Task Jaringan Core 0 (Wi-Fi + NTP + Firebase Auth + RTDB)
// Path diselaraskan penuh dengan UML: /room1/current, /actuator, /alarm, /history, /events
// ==============================================================================

class FirebaseManager {
private:
  SharedState* _shared;
  TimeManager* _timeMgr;
  TaskHandle_t _taskHandle;

  WiFiClientSecure _secureClient;
  String _idToken;
  unsigned long _tokenObtainedMs;

  unsigned long _lastWifiAttemptMs;
  unsigned long _lastCurrentPushMs;
  unsigned long _lastControlPollMs;
  unsigned long _lastThresholdPollMs;
  unsigned long _lastHistoryLogMs;
  unsigned long _lastHistoryPruneMs;
  bool _initialThresholdLoaded;

  static void taskEntry(void* pvParameters);
  void runLoop();

  void ensureWifiConnected();
  bool authenticateDevice();
  bool ensureAuthenticated();

  int httpRequest(const char* method, const String& url, const String& payload, String& responseOut);
  String buildRtdbUrl(const char* subPath) const;

  void pushCurrent();
  void syncAlarmIfDirty();
  void pollActuatorAndMute();
  void pollThresholds();
  void pushHistoryIfDue();
  void pruneOldHistoryIfDue();
  void flushEventQueue();
  void publishNetworkStatus(int lastHttpCode, bool pushedOk);

public:
  FirebaseManager();
  void begin(SharedState* shared, TimeManager* timeMgr);
};
