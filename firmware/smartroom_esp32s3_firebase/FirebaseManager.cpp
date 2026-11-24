#include "FirebaseManager.h"

#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #error "File secrets.h tidak ditemukan! Salin secrets.h.example menjadi secrets.h dan isi kredensial Anda."
#endif

// ==============================================================================
// FirebaseManager.cpp - Implementasi Task Cloud Core 0 (Sinkron UML & SRS v0.9)
// ==============================================================================

FirebaseManager::FirebaseManager()
  : _shared(nullptr),
    _timeMgr(nullptr),
    _taskHandle(nullptr),
    _tokenObtainedMs(0),
    _lastWifiAttemptMs(0),
    _lastCurrentPushMs(0),
    _lastControlPollMs(0),
    _lastThresholdPollMs(0),
    _lastHistoryLogMs(0),
    _lastHistoryPruneMs(0),
    _initialThresholdLoaded(false) {}

void FirebaseManager::begin(SharedState* shared, TimeManager* timeMgr) {
  _shared = shared;
  _timeMgr = timeMgr;

  _secureClient.setInsecure();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  _lastWifiAttemptMs = millis();

  xTaskCreatePinnedToCore(
    FirebaseManager::taskEntry,
    "FirebaseCore0Task",
    12288,
    this,
    1,
    &_taskHandle,
    0
  );

  Serial.println(F("[OK] FirebaseManager task diluncurkan pada Core 0."));
}

void FirebaseManager::taskEntry(void* pvParameters) {
  FirebaseManager* self = static_cast<FirebaseManager*>(pvParameters);
  self->runLoop();
}

void FirebaseManager::ensureWifiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    if (_timeMgr) {
      _timeMgr->begin();
    }
    return;
  }

  unsigned long now = millis();
  if (now - _lastWifiAttemptMs >= WIFI_RECONNECT_INTERVAL_MS) {
    _lastWifiAttemptMs = now;
    Serial.println(F("[WIFI] Menghubungkan ulang ke Wi-Fi (Core 0, hardware lokal tetap aktif)..."));
    WiFi.disconnect(false, false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

int FirebaseManager::httpRequest(const char* method, const String& url, const String& payload, String& responseOut) {
  HTTPClient http;
  http.setConnectTimeout(4000);
  http.setTimeout(5000);

  if (!http.begin(_secureClient, url)) {
    return -1;
  }

  http.addHeader("Content-Type", "application/json");
  int code = -1;

  if (strcmp(method, "GET") == 0) {
    code = http.GET();
  } else if (strcmp(method, "PUT") == 0) {
    code = http.PUT(payload);
  } else if (strcmp(method, "PATCH") == 0) {
    code = http.PATCH(payload);
  } else if (strcmp(method, "POST") == 0) {
    code = http.POST(payload);
  } else if (strcmp(method, "DELETE") == 0) {
    code = http.sendRequest("DELETE");
  }

  if (code > 0) {
    responseOut = http.getString();
  } else {
    responseOut = "";
  }

  http.end();
  return code;
}

bool FirebaseManager::authenticateDevice() {
  String authUrl = String("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=") + FIREBASE_WEB_API_KEY;

  JsonDocument reqDoc;
  reqDoc["email"] = FIREBASE_DEVICE_EMAIL;
  reqDoc["password"] = FIREBASE_DEVICE_PASSWORD;
  reqDoc["returnSecureToken"] = true;

  String reqBody;
  serializeJson(reqDoc, reqBody);

  String respBody;
  int code = httpRequest("POST", authUrl, reqBody, respBody);

  if (code == 200) {
    JsonDocument respDoc;
    DeserializationError err = deserializeJson(respDoc, respBody);
    if (!err && respDoc["idToken"].is<const char*>()) {
      _idToken = respDoc["idToken"].as<String>();
      _tokenObtainedMs = millis();
      Serial.println(F("[AUTH] Login akun perangkat Firebase berhasil (idToken aktif)."));
      publishNetworkStatus(code, false);
      return true;
    }
  }

  Serial.printf("[AUTH] Gagal autentikasi perangkat Firebase (HTTP %d).\n", code);
  _idToken = "";
  publishNetworkStatus(code, false);
  return false;
}

bool FirebaseManager::ensureAuthenticated() {
  unsigned long now = millis();
  if (_idToken.length() > 0 && (now - _tokenObtainedMs < FIREBASE_TOKEN_REFRESH_MS)) {
    return true;
  }
  return authenticateDevice();
}

String FirebaseManager::buildRtdbUrl(const char* subPath) const {
  return String(FIREBASE_RTDB_URL) + String(FIREBASE_ROOM_PATH) + String(subPath) + ".json?auth=" + _idToken;
}

void FirebaseManager::publishNetworkStatus(int lastHttpCode, bool pushedOk) {
  if (!_shared) return;
  NetworkStatusSnapshot st = _shared->getNetworkStatus();
  st.wifiConnected = (WiFi.status() == WL_CONNECTED);
  st.firebaseAuthed = (_idToken.length() > 0);
  st.ntpSynced = (_timeMgr && _timeMgr->isSynced());
  if (lastHttpCode != 0) {
    st.lastHttpCode = lastHttpCode;
  }
  if (pushedOk) {
    st.lastSuccessPushMs = millis();
  }
  _shared->setNetworkStatus(st);
}

void FirebaseManager::pushCurrent() {
  CurrentSnapshot snap;
  if (!_shared->getCurrent(snap)) {
    return;
  }

  uint64_t tsMs = snap.timestampMs;
  if (tsMs == 0 && _timeMgr && _timeMgr->isSynced()) {
    tsMs = _timeMgr->getEpochSeconds() * 1000ULL;
  }

  JsonDocument doc;
  doc["temperature"]      = isnan(snap.temperature) ? 0.0f : roundf(snap.temperature * 100.0f) / 100.0f;
  doc["humidity"]         = isnan(snap.humidity) ? 0.0f : roundf(snap.humidity * 100.0f) / 100.0f;
  doc["humidityLabel"]    = snap.humidityLabel;
  doc["airDelta"]         = snap.airDelta;
  doc["airLabel"]         = snap.airLabel;
  doc["airAqi"]           = snap.airAqi;
  doc["airTvoc"]          = snap.airTvoc;
  doc["airEco2"]          = snap.airEco2;
  doc["airWarmingUp"]     = snap.airWarmingUp;
  doc["lightLux"]         = (snap.lightLux < 0.0f) ? 0.0f : roundf(snap.lightLux * 10.0f) / 10.0f;
  doc["lightLabel"]       = snap.lightLabel;
  doc["presence"]         = snap.presence;
  doc["presenceDetected"] = snap.presenceDetected;
  doc["fanStatus"]        = snap.fanStatus;
  doc["lampStatus"]       = snap.lampStatus;
  doc["nightModeActive"]  = snap.nightModeActive;
  doc["timestamp"]        = tsMs;

  String body;
  serializeJson(doc, body);

  String resp;
  int code = httpRequest("PUT", buildRtdbUrl("/current"), body, resp);
  if (code == 401) {
    _idToken = "";
  }
  publishNetworkStatus(code, code == 200);
}

void FirebaseManager::syncAlarmIfDirty() {
  AlarmStateSnapshot alarmSt;
  bool muteDirty = false;
  bool muteVal = false;
  if (!_shared->consumeAlarmDirty(alarmSt, muteDirty, muteVal)) {
    return;
  }

  JsonDocument doc;
  doc["active"] = alarmSt.active;
  doc["buzzer"] = alarmSt.buzzer;
  doc["status"] = alarmSt.status;
  doc["reason"] = alarmSt.reason;
  if (muteDirty) {
    doc["mute"] = muteVal;
  }

  String body;
  serializeJson(doc, body);

  String resp;
  int code = httpRequest("PATCH", buildRtdbUrl("/alarm"), body, resp);
  if (code == 401) {
    _idToken = "";
  }
  publishNetworkStatus(code, false);
}

void FirebaseManager::pollActuatorAndMute() {
  // 1. Baca /room1/actuator (fanMode, lampMode dari Staff)
  String respAct;
  int codeAct = httpRequest("GET", buildRtdbUrl("/actuator"), "", respAct);
  if (codeAct == 401) {
    _idToken = "";
    publishNetworkStatus(codeAct, false);
    return;
  }
  if (codeAct == 200 && respAct != "null" && respAct.length() >= 2) {
    JsonDocument doc;
    if (deserializeJson(doc, respAct) == DeserializationError::Ok) {
      const char* fanMode  = doc["fanMode"]  | "auto";
      const char* lampMode = doc["lampMode"] | "auto";
      _shared->setRemoteActuatorModes(fanMode, lampMode);
    }
  }

  // 2. Baca /room1/alarm/mute (perintah MUTE dari Staff — UML UC-10)
  String respMute;
  int codeMute = httpRequest("GET", buildRtdbUrl("/alarm/mute"), "", respMute);
  if (codeMute == 401) {
    _idToken = "";
    publishNetworkStatus(codeMute, false);
    return;
  }
  if (codeMute == 200) {
    respMute.trim();
    if (respMute == "true") {
      _shared->setRemoteAlarmMute(true);
    } else if (respMute == "false") {
      _shared->setRemoteAlarmMute(false);
    }
  }
  publishNetworkStatus(codeMute, false);
}

void FirebaseManager::pollThresholds() {
  String resp;
  int code = httpRequest("GET", buildRtdbUrl("/thresholds"), "", resp);
  if (code == 401) {
    _idToken = "";
    publishNetworkStatus(code, false);
    return;
  }
  if (code != 200 || resp == "null" || resp.length() < 2) {
    publishNetworkStatus(code, false);
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, resp) == DeserializationError::Ok) {
    ThresholdSnapshot th = _shared->getThresholds();
    th.tempFanOn                  = doc["tempFanOn"]                  | th.tempFanOn;
    th.tempFanHysteresis          = doc["tempFanHysteresis"]          | th.tempFanHysteresis;
    th.tempAlert                  = doc["tempAlert"]                  | th.tempAlert;
    th.airDeltaModerate           = doc["airDeltaModerate"]           | th.airDeltaModerate;
    th.airDeltaBad                = doc["airDeltaBad"]                | th.airDeltaBad;
    th.lightDarkLux               = doc["lightDarkLux"]               | th.lightDarkLux;
    th.lightExternalHysteresisLux = doc["lightExternalHysteresisLux"] | th.lightExternalHysteresisLux;
    th.lampEmptyTimeoutSec        = doc["lampEmptyTimeoutSec"]        | th.lampEmptyTimeoutSec;
    th.nightStartHour             = doc["nightStartHour"]             | th.nightStartHour;
    th.nightEndHour               = doc["nightEndHour"]               | th.nightEndHour;
    _shared->setThresholds(th);
    _initialThresholdLoaded = true;
  }
  publishNetworkStatus(code, false);
}

void FirebaseManager::pushHistoryIfDue() {
  unsigned long now = millis();
  // Interval 10 menit = 144 titik / 24 jam (selaras UML UC-07 limitToLast(144))
  if (_lastHistoryLogMs != 0 && (now - _lastHistoryLogMs < INTERVAL_HISTORY_LOG_MS)) {
    return;
  }

  if (!_timeMgr || !_timeMgr->isSynced()) {
    return;
  }

  CurrentSnapshot snap;
  if (!_shared->getCurrent(snap)) {
    return;
  }

  uint64_t epochSec = _timeMgr->getEpochSeconds();
  if (epochSec == 0) {
    return;
  }

  JsonDocument doc;
  doc["temperature"] = isnan(snap.temperature) ? 0.0f : roundf(snap.temperature * 10.0f) / 10.0f;
  doc["humidity"]    = isnan(snap.humidity) ? 0.0f : roundf(snap.humidity * 10.0f) / 10.0f;
  doc["airDelta"]    = snap.airDelta;
  doc["airAqi"]      = snap.airAqi;
  doc["lightLux"]    = (snap.lightLux < 0.0f) ? 0.0f : roundf(snap.lightLux * 10.0f) / 10.0f;
  doc["timestamp"]   = epochSec * 1000ULL;

  String body;
  serializeJson(doc, body);

  char subPathBuf[40];
  snprintf(subPathBuf, sizeof(subPathBuf), "/history/%llu", (unsigned long long)epochSec);
  String resp;
  int code = httpRequest("PUT", buildRtdbUrl(subPathBuf), body, resp);
  if (code == 200) {
    _lastHistoryLogMs = now;
  } else if (code == 401) {
    _idToken = "";
  }
}

void FirebaseManager::pruneOldHistoryIfDue() {
  unsigned long now = millis();
  if (_lastHistoryPruneMs != 0 && (now - _lastHistoryPruneMs < 1800000UL)) {
    return;
  }
  if (!_timeMgr || !_timeMgr->isSynced()) {
    return;
  }

  uint64_t cutoffEpoch = _timeMgr->getEpochSeconds() - HISTORY_RETENTION_SEC;
  String url = String(FIREBASE_RTDB_URL) + String(FIREBASE_ROOM_PATH) + "/history.json?shallow=true&auth=" + _idToken;
  String resp;
  int code = httpRequest("GET", url, "", resp);
  if (code != 200 || resp == "null" || resp.length() < 2) {
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, resp) == DeserializationError::Ok && doc.is<JsonObject>()) {
    uint8_t deleted = 0;
    for (JsonPair kv : doc.as<JsonObject>()) {
      uint64_t keyEpoch = strtoull(kv.key().c_str(), nullptr, 10);
      if (keyEpoch > 0 && keyEpoch < cutoffEpoch) {
        String delPath = String("/history/") + kv.key().c_str();
        String delResp;
        httpRequest("DELETE", buildRtdbUrl(delPath.c_str()), "", delResp);
        deleted++;
        if (deleted >= 5) break;
      }
    }
    _lastHistoryPruneMs = now;
  }
}

void FirebaseManager::flushEventQueue() {
  EventItem ev;
  uint8_t sentCount = 0;

  while (_shared->peekEvent(ev) && sentCount < 3) {
    uint64_t tsMs = ev.timestampMs;
    if (tsMs == 0 && _timeMgr && _timeMgr->isSynced()) {
      tsMs = _timeMgr->getEpochSeconds() * 1000ULL;
    }

    JsonDocument doc;
    doc["timestamp"] = tsMs;
    doc["type"]      = ev.type;
    doc["severity"]  = ev.severity;
    doc["message"]   = ev.message;

    String body;
    serializeJson(doc, body);

    String resp;
    int code = httpRequest("POST", buildRtdbUrl("/events"), body, resp);
    if (code == 200) {
      _shared->dropFrontEvent();
      sentCount++;
    } else {
      if (code == 401) {
        _idToken = "";
      }
      break;
    }
  }
}

void FirebaseManager::runLoop() {
  for (;;) {
    ensureWifiConnected();

    if (WiFi.status() == WL_CONNECTED) {
      if (ensureAuthenticated()) {
        unsigned long now = millis();

        if (!_initialThresholdLoaded || (now - _lastThresholdPollMs >= INTERVAL_THRESHOLD_POLL_MS)) {
          _lastThresholdPollMs = now;
          pollThresholds();
        }

        if (now - _lastControlPollMs >= INTERVAL_CONTROL_POLL_MS) {
          _lastControlPollMs = now;
          pollActuatorAndMute();
        }

        syncAlarmIfDirty();

        if (now - _lastCurrentPushMs >= INTERVAL_TELEMETRI_PUSH_MS) {
          _lastCurrentPushMs = now;
          pushCurrent();
        }

        flushEventQueue();
        pushHistoryIfDue();
        pruneOldHistoryIfDue();
      } else {
        vTaskDelay(pdMS_TO_TICKS(3000));
      }
    } else {
      publishNetworkStatus(0, false);
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
