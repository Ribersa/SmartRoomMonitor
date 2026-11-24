// ==============================================================================
// smartroom_esp32s3_firebase.ino - FIRMWARE PRODUKSI DUAL-CORE ESP32-S3 + FIREBASE
// Proyek: SmartRoomMonitor (RPL) - Sinkron SRS v0.9 & Pemodelan UML
// Arsitektur:
//   - Core 1 (loop): UART Radar LD2410B (256kbps), I2C (SHT30, ENS160, BH1750),
//                    Baseline airDelta (NFR-06), Kipas Histeresis, Lampu Adaptif +200Lx,
//                    Buzzer Lokal Offline Failsafe (FR-06) & Pencatatan Events
//   - Core 0 (task): Wi-Fi Auto-Reconnect, NTP WIB (UTC+7), Firebase Auth REST,
//                    Sinkronisasi RTDB (/current, /actuator, /alarm, /thresholds, /history, /events)
// ==============================================================================

#include "config.h"
#include "SharedState.h"
#include "TimeManager.h"
#include "SensorManager.h"
#include "FanManager.h"
#include "LightManager.h"
#include "BuzzerManager.h"
#include "FirebaseManager.h"

SharedState     sharedState;
TimeManager     timeMgr;
SensorManager   sensors;
FanManager      fan;
LightManager    light;
BuzzerManager   buzzer;
FirebaseManager firebaseMgr;

unsigned long lastSensorEvalMs    = 0;
unsigned long lastTempEventMs     = 0;
unsigned long lastAirEventMs      = 0;
unsigned long lastNightEventMs    = 0;
bool          prevAlarmActive     = false;

void tungguDenganRadar(unsigned long durasiMs) {
  unsigned long tStart = millis();
  while (millis() - tStart < durasiMs) {
    sensors.updateRadar();
    delay(2);
  }
}

void tampilkanMenuSerial() {
  Serial.println(F("\n---------------------------------------------------------------------"));
  Serial.println(F("  SmartRoomMonitor ESP32-S3 (Firebase RTDB Produksi - SRS v0.9 / UML)"));
  Serial.println(F("  Perintah Serial Debug: [s] Status | [x] Toggle Mute | [h] Bantuan"));
  Serial.println(F("---------------------------------------------------------------------\n"));
}

void prosesSerialDebug() {
  while (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == '\n' || cmd == '\r' || cmd == ' ') continue;

    if (cmd == 'x' || cmd == 'X') {
      bool baru = !buzzer.isMuted();
      buzzer.setMuted(baru);
      sharedState.requestLocalMuteUpdate(baru);
      Serial.printf("[DEBUG] Alarm Mute lokal -> %s\n", baru ? "AKTIF" : "NONAKTIF");
    } else if (cmd == 's' || cmd == 'S') {
      NetworkStatusSnapshot net = sharedState.getNetworkStatus();
      char jamBuf[16];
      timeMgr.formatJamWIB(jamBuf, sizeof(jamBuf));
      Serial.printf("[STATUS] Wi-Fi: %s | Auth: %s | NTP: %s (%s) | HTTP: %d | Kipas: %s(%s) | Lampu: %s(%s)\n",
                    net.wifiConnected ? "OK" : "OFFLINE",
                    net.firebaseAuthed ? "OK" : "BELUM",
                    net.ntpSynced ? "OK" : "BELUM",
                    jamBuf,
                    net.lastHttpCode,
                    fan.getModeString(), fan.isNyala() ? "ON" : "OFF",
                    light.getModeString(), light.isNyala() ? "ON" : "OFF");
    } else if (cmd == 'h' || cmd == 'H') {
      tampilkanMenuSerial();
    }
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1500);

  Serial.println(F("\n====================================================================="));
  Serial.println(F("  [SmartRoomMonitor] FIRMWARE PRODUKSI ESP32-S3 + FIREBASE (UML v0.9)"));
  Serial.println(F("====================================================================="));

  sharedState.begin();
  buzzer.begin();
  sensors.begin();
  fan.begin();
  light.begin();

  firebaseMgr.begin(&sharedState, &timeMgr);

  buzzer.nadaSambutBoot();
  tampilkanMenuSerial();
}

void loop() {
  // 1. Pompa UART Radar 256kbps secara non-blocking pada Core 1
  sensors.updateRadar();

  // 2. Terapkan perintah Staff dari Firebase (/room1/actuator & /room1/alarm/mute)
  ActuatorControlState ctrl;
  if (sharedState.consumeRemoteControls(ctrl)) {
    fan.setModeFromString(ctrl.fanMode);
    light.setModeFromString(ctrl.lampMode);
    buzzer.setMuted(ctrl.alarmMute);
  }

  // 3. Cek input Serial debug
  prosesSerialDebug();

  // 4. Evaluasi sensor, aktuator lokal, dan alarm (berjalan otonom walau offline)
  unsigned long now = millis();
  if (now - lastSensorEvalMs >= INTERVAL_SENSOR_EVAL_MS) {
    lastSensorEvalMs = now;

    ThresholdSnapshot th = sharedState.getThresholds();
    TelemetriData data = sensors.bacaSemua(th.lightDarkLux, th.airDeltaModerate, th.airDeltaBad);

    // Evaluasi Kipas FR-02 dengan jeda 60ms saat beralih
    bool kipasSebelum = fan.isNyala();
    fan.evaluasiSuhu(data.suhu, th.tempFanOn, th.tempFanHysteresis);
    if (fan.isNyala() != kipasSebelum) {
      tungguDenganRadar(60);
    }

    // Evaluasi Lampu Adaptif FR-08 dengan jeda 60ms saat beralih
    bool lampuSebelum = light.isNyala();
    light.evaluasi(data.lux, data.adaOrang, th.lightDarkLux, th.lightExternalHysteresisLux, th.lampEmptyTimeoutSec);
    if (light.isNyala() != lampuSebelum) {
      tungguDenganRadar(60);
    }

    // Evaluasi Jam Malam NTP WIB (FR-05) & Alarm Buzzer Lokal (FR-01, FR-03, FR-05, FR-06)
    bool jamMalam = timeMgr.isJamMalam(th.nightStartHour, th.nightEndHour);

    // UML UC-10: mute berakhir setelah 60 detik bila kondisi bahaya belum mereda
    if (buzzer.isRearmDue()) {
      buzzer.setMuted(false);
      sharedState.requestLocalMuteUpdate(false);
      Serial.println(F("[ALARM] Masa mute 60 detik habis, alarm aktif kembali."));
    }

    AlarmStatus alarm = buzzer.evaluasiAlarm(
      data.suhu, th.tempAlert, data.airBad, data.aqi, data.warmingUp, data.adaOrang, jamMalam
    );
    sensors.updateRadar();

    // Auto-unmute saat seluruh kondisi bahaya sudah benar-benar normal
    if (prevAlarmActive && !alarm.active && buzzer.isMuted()) {
      buzzer.setMuted(false);
      sharedState.requestLocalMuteUpdate(false);
    }
    prevAlarmActive = alarm.active;

    sharedState.setAlarmState(alarm.active, alarm.sounding, alarm.statusStr, alarm.reason);

    // Pencatatan Event Log oleh ESP32 (FR-01, FR-03, FR-04, FR-05) — debounce 60 detik
    uint64_t epochMs = timeMgr.getEpochSeconds() * 1000ULL;
    char jamBuf[16];
    timeMgr.formatJamWIB(jamBuf, sizeof(jamBuf));

    if (!isnan(data.suhu) && data.suhu > th.tempAlert &&
        (lastTempEventMs == 0 || now - lastTempEventMs >= DEBOUNCE_EVENT_MS)) {
      lastTempEventMs = now;
      char msg[128];
      snprintf(msg, sizeof(msg), "%.1f C, di atas batas %.1f C. Buzzer berbunyi (%s).", data.suhu, th.tempAlert, jamBuf);
      sharedState.pushEvent(epochMs, "temp_high", "danger", msg);
    }

    if (!data.warmingUp && data.airBad &&
        (lastAirEventMs == 0 || now - lastAirEventMs >= DEBOUNCE_EVENT_MS)) {
      lastAirEventMs = now;
      char msg[128];
      snprintf(msg, sizeof(msg), "Delta %u dari baseline (AQI %u), label Buruk (%s).", data.airDelta, data.aqi, jamBuf);
      sharedState.pushEvent(epochMs, "air_bad", (data.aqi >= 5) ? "danger" : "warning", msg);
    }

    if (jamMalam && data.adaOrang &&
        (lastNightEventMs == 0 || now - lastNightEventMs >= DEBOUNCE_EVENT_MS)) {
      lastNightEventMs = now;
      char msg[128];
      snprintf(msg, sizeof(msg), "Ada orang (%s) pada jam malam (%s).", data.presenceState, jamBuf);
      sharedState.pushEvent(epochMs, "night_presence", "danger", msg);
    }

    // Perbarui snapshot /room1/current untuk dikirim Core 0 ke Firebase
    CurrentSnapshot snap;
    memset(&snap, 0, sizeof(snap));
    snap.valid            = true;
    snap.temperature      = data.suhu;
    snap.humidity         = data.kelembapan;
    strncpy(snap.humidityLabel, data.labelKelembapan, sizeof(snap.humidityLabel) - 1);
    snap.airDelta         = data.airDelta;
    strncpy(snap.airLabel, data.labelAQI, sizeof(snap.airLabel) - 1);
    snap.airAqi           = data.aqi;
    snap.airTvoc          = data.tvoc;
    snap.airEco2          = data.eco2;
    snap.airWarmingUp     = data.warmingUp;
    snap.lightLux         = data.lux;
    strncpy(snap.lightLabel, data.labelCahaya, sizeof(snap.lightLabel) - 1);
    strncpy(snap.presence, data.presenceState, sizeof(snap.presence) - 1);
    snap.presenceDetected = data.adaOrang;
    snap.fanStatus        = fan.isNyala();
    snap.lampStatus       = light.isNyala();
    snap.nightModeActive  = jamMalam;
    snap.timestampMs      = epochMs;

    sharedState.setCurrent(snap);

    NetworkStatusSnapshot net = sharedState.getNetworkStatus();
    Serial.printf("[%s] T:%.1fC H:%.0f%% | Air:%s(d=%u,AQI=%u) | Lux:%.0f(%s) | Radar:%s | Fan:%s Lamp:%s | Alarm:%s | Cloud:%s(HTTP %d)\n",
                  jamBuf,
                  isnan(data.suhu) ? 0.0f : data.suhu,
                  isnan(data.kelembapan) ? 0.0f : data.kelembapan,
                  data.labelAQI, data.airDelta, data.aqi,
                  data.lux < 0 ? 0.0f : data.lux, data.labelCahaya,
                  data.radarOk ? data.presenceState : "DISCONNECT",
                  fan.isNyala() ? "ON" : "OFF",
                  light.isNyala() ? "ON" : "OFF",
                  alarm.statusStr,
                  net.wifiConnected ? (net.firebaseAuthed ? "ONLINE" : "AUTH..") : "OFFLINE",
                  net.lastHttpCode);
  }
}
