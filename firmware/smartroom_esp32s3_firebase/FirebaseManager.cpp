#include "FirebaseManager.h"

#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #error "File secrets.h tidak ditemukan! Salin secrets.h.example menjadi secrets.h dan isi kredensial Anda."
#endif

// ==============================================================================
// FirebaseManager.cpp - Implementasi Task Cloud Core 0 (Non-Blocking & Offline-Safe)
// ==============================================================================

FirebaseManager::FirebaseManager()
  : _shared(nullptr),
    _timeMgr(nullptr),
    _taskHandle(nullptr),
    _tokenObtainedMs(0),
    _lastWifiAttemptMs(0),
    _lastTelemetryPushMs(0),
    _lastControlPollMs(0),
    _lastThresholdPollMs(0),
    _lastHistoryLogMs(0),
    _lastHistoryPruneMs(0),
    _initialThresholdLoaded(false) {}

void FirebaseManager::begin(SharedState* shared, TimeManager* timeMgr) {
  _shared = shared;
  _timeMgr = timeMgr;

  // Catatan: setInsecure() digunakan agar koneksi TLS tetap terenkripsi tanpa
  // kendala rotasi Root CA saat demo hotspot; dapat diganti setCACert() untuk pinning.
  _secureClient.setInsecure();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  _lastWifiAttemptMs = millis();

  // Jalankan seluruh aktivitas jaringan pada Core 0 agar Core 1 (Radar 256kbps + Aktuator) tidak terganggu
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

void FirebaseManager::pushTelemetry() {
  TelemetrySnapshot snap;
  if (!_shared->getTelemetry(snap)) {
    return;
  }

  JsonDocument doc;
  doc["temperature"]            = isnan(snap.temperature) ? 0.0f : roundf(snap.temperature * 100.0f) / 100.0f;
  doc["humidity"]               = isnan(snap.humidity) ? 0.0f : roundf(snap.humidity * 100.0f) / 100.0f;
  doc["humidity_label"]         = snap.humidityLabel;
  doc["air_quality_label"]      = snap.airQualityLabel;
  doc["air_quality_aqi"]        = snap.airQualityAqi;
  doc["air_quality_tvoc"]       = snap.airQualityTvoc;
  doc["air_quality_eco2"]       = snap.airQualityEco2;
  doc["air_quality_warming_up"] = snap.airQualityWarmingUp;
  doc["light_lux"]              = (snap.lightLux < 0.0f) ? 0.0f : roundf(snap.lightLux * 10.0f) / 10.0f;
  doc["light_label"]            = snap.lightLabel;
  doc["presence_state"]         = snap.presenceState;
  doc["presence_detected"]      = snap.presenceDetected;
  doc["alarm_active"]           = snap.alarmActive;
  doc["alarm_reason"]           = snap.alarmReason;
  doc["night_mode_active"]      = snap.nightModeActive;
  uint64_t ts = snap.deviceTimestamp;
  if (ts == 0 && _timeMgr && _timeMgr->isSynced()) {
    ts = _timeMgr->getEpochSeconds();
  }
  doc["device_timestamp"]       = ts;

  String body;
  serializeJson(doc, body);

  String resp;
  int code = httpRequest("PUT", buildRtdbUrl("/telemetry"), body, resp);
  if (code == 401) {
    _idToken = ""; // Paksa refresh token pada siklus berikutnya
  }
  publishNetworkStatus(code, code == 200);
}

void FirebaseManager::syncActuatorStatusIfDirty() {
  bool fanSt = false, lampSt = false, muteDirty = false, muteVal = false;
  if (!_shared->consumeActuatorStatusDirty(fanSt, lampSt, muteDirty, muteVal)) {
    return;
  }

  JsonDocument doc;
  doc["fan_status"] = fanSt;
  doc["lamp_status"] = lampSt;
  if (muteDirty) {
    doc["alarm_muted"] = muteVal;
  }

  String body;
  serializeJson(doc, body);

  String resp;
  int code = httpRequest("PATCH", buildRtdbUrl("/controls"), body, resp);
  if (code == 401) {
    _idToken = "";
  }
  publishNetworkStatus(code, false);
}

void FirebaseManager::pollControls() {
  String resp;
  int code = httpRequest("GET", buildRtdbUrl("/controls"), "", resp);
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
    const char* fanMode  = doc["fan_mode"]  | "auto";
    const char* lampMode = doc["lamp_mode"] | "auto";
    bool alarmMuted      = doc["alarm_muted"] | false;
    _shared->setRemoteControls(fanMode, lampMode, alarmMuted);
  }
  publishNetworkStatus(code, false);
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
    th.tempFanOn                  = doc["temp_fan_on"]                   | th.tempFanOn;
    th.tempFanHysteresis          = doc["temp_fan_hysteresis"]           | th.tempFanHysteresis;
    th.tempAlert                  = doc["temp_alert"]                    | th.tempAlert;
    th.lightDarkLux               = doc["light_dark_lux"]                | th.lightDarkLux;
    th.lightExternalHysteresisLux = doc["light_external_hysteresis_lux"] | th.lightExternalHysteresisLux;
    th.lampEmptyTimeoutSec        = doc["lamp_empty_timeout_sec"]        | th.lampEmptyTimeoutSec;
    th.nightStartHour             = doc["night_start_hour"]              | th.nightStartHour;
    th.nightEndHour               = doc["night_end_hour"]                | th.nightEndHour;
    _shared->setThresholds(th);
    _initialThresholdLoaded = true;
  }
  publishNetworkStatus(code, false);
}

void FirebaseManager::pushHistoryIfDue() {
  unsigned long now = millis();
  if (_lastHistoryLogMs != 0 && (now - _lastHistoryLogMs < INTERVAL_HISTORY_LOG_MS)) {
    return;
  }

  if (!_timeMgr || !_timeMgr->isSynced()) {
    return;
  }

  TelemetrySnapshot snap;
  if (!_shared->getTelemetry(snap)) {
    return;
  }

  uint64_t epoch = _timeMgr->getEpochSeconds();
  if (epoch == 0) {
    return;
  }

  JsonDocument doc;
  doc["t"]   = isnan(snap.temperature) ? 0.0f : roundf(snap.temperature * 10.0f) / 10.0f;
  doc["h"]   = isnan(snap.humidity) ? 0.0f : roundf(snap.humidity * 10.0f) / 10.0f;
  doc["aqi"] = snap.airQualityAqi;
  doc["lux"] = (snap.lightLux < 0.0f) ? 0 : (int)roundf(snap.lightLux);

  String body;
  serializeJson(doc, body);

  char subPathBuf[40];
  snprintf(subPathBuf, sizeof(subPathBuf), "/history/%llu", (unsigned long long)epoch);
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
  // Bersihkan riwayat > 24 jam setiap 30 menit sekali
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
        if (deleted >= 5) break; // Batasi per siklus agar tidak menahan sinkronisasi
      }
    }
    _lastHistoryPruneMs = now;
  }
}

void FirebaseManager::flushEventQueue() {
  EventItem ev;
  uint8_t sentCount = 0;

  while (_shared->peekEvent(ev) && sentCount < 3) {
    JsonDocument doc;
    doc["timestamp"] = (ev.timestamp > 0) ? ev.timestamp : (_timeMgr ? _timeMgr->getEpochSeconds() : 0ULL);
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
          pollControls();
        }

        syncActuatorStatusIfDirty();

        if (now - _lastTelemetryPushMs >= INTERVAL_TELEMETRI_PUSH_MS) {
          _lastTelemetryPushMs = now;
          pushTelemetry();
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
