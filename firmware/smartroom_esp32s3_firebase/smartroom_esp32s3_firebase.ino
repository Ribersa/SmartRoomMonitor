// ==============================================================================
// smartroom_esp32s3_firebase.ino - FIRMWARE PRODUKSI DUAL-CORE ESP32-S3 + FIREBASE
// Proyek: SmartRoomMonitor (RPL) - Baseline SRS v0.9 (FR-01 s.d. FR-08, NFR-01 s.d. NFR-08)
// Arsitektur:
//   - Core 1 (loop): UART Radar LD2410B (256kbps), I2C (SHT30, ENS160, BH1750),
//                    Kontrol Kipas Histeresis, Lampu Adaptif +200Lx, Buzzer Offline Failsafe
//   - Core 0 (task): Wi-Fi Auto-Reconnect, NTP WIB (UTC+7), Firebase Auth REST,
//                    Sinkronisasi RTDB (/telemetry, /controls, /thresholds, /history, /events)
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
  Serial.println(F("  SmartRoomMonitor ESP32-S3 (Firebase RTDB Produksi - SRS v0.9)"));
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
  Serial.println(F("  [SmartRoomMonitor] FIRMWARE PRODUKSI ESP32-S3 + FIREBASE RTDB v0.9"));
  Serial.println(F("====================================================================="));

  sharedState.begin();
  buzzer.begin();
  sensors.begin();
  fan.begin();
  light.begin();

  // Mulai task Wi-Fi, NTP, dan Firebase pada Core 0
  firebaseMgr.begin(&sharedState, &timeMgr);

  buzzer.nadaSambutBoot();
  tampilkanMenuSerial();
}

void loop() {
  // 1. Pompa UART Radar 256kbps secara non-blocking pada Core 1
  sensors.updateRadar();

  // 2. Cek perintah kontrol terbaru yang ditarik Core 0 dari Firebase (/room1/controls)
  ControlSnapshot ctrl;
  if (sharedState.consumeRemoteControls(ctrl)) {
    fan.setModeFromString(ctrl.fanMode);
    light.setModeFromString(ctrl.lampMode);
    buzzer.setMuted(ctrl.alarmMuted);
    sharedState.setActuatorStatus(fan.isNyala(), light.isNyala());
  }

  // 3. Cek input Serial debug
  prosesSerialDebug();

  // 4. Evaluasi sensor, aktuator lokal, dan alarm (berjalan otonom walau offline)
  unsigned long now = millis();
  if (now - lastSensorEvalMs >= INTERVAL_SENSOR_EVAL_MS) {
    lastSensorEvalMs = now;

    ThresholdSnapshot th = sharedState.getThresholds();
    TelemetriData data = sensors.bacaSemua(th.lightDarkLux);

    // Evaluasi Kipas FR-02 dengan jeda 60ms saat beralih untuk stabilitas daya
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

    // Evaluasi Jam Malam NTP WIB (FR-05) & Alarm Buzzer (FR-01, FR-03, FR-05, FR-06)
    bool jamMalam = timeMgr.isJamMalam(th.nightStartHour, th.nightEndHour);
    AlarmStatus alarm = buzzer.evaluasiAlarm(
      data.suhu, th.tempAlert, data.aqi, data.warmingUp, data.adaOrang, jamMalam
    );
    sensors.updateRadar();

    // Auto-unmute jika seluruh kondisi bahaya telah benar-benar reda
    if (prevAlarmActive && !alarm.active && buzzer.isMuted()) {
      buzzer.setMuted(false);
      sharedState.requestLocalMuteUpdate(false);
    }
    prevAlarmActive = alarm.active;

    // Pencatatan Event Log (FR-01, FR-03, FR-04, FR-05) dengan debounce 60 detik
    uint64_t epochNow = timeMgr.getEpochSeconds();
    char jamBuf[16];
    timeMgr.formatJamWIB(jamBuf, sizeof(jamBuf));

    if (!isnan(data.suhu) && data.suhu > th.tempAlert &&
        (lastTempEventMs == 0 || now - lastTempEventMs >= DEBOUNCE_EVENT_MS)) {
      lastTempEventMs = now;
      char msg[128];
      snprintf(msg, sizeof(msg), "Suhu ruangan mencapai %.1f C melebihi batas %.1f C (%s)", data.suhu, th.tempAlert, jamBuf);
      sharedState.pushEvent(epochNow, "temp_high", "danger", msg);
    }

    if (!data.warmingUp && data.aqi >= 4 &&
        (lastAirEventMs == 0 || now - lastAirEventMs >= DEBOUNCE_EVENT_MS)) {
      lastAirEventMs = now;
      char msg[128];
      snprintf(msg, sizeof(msg), "Kualitas udara buruk (AQI %u, TVOC %u ppb, eCO2 %u ppm) pada %s", data.aqi, data.tvoc, data.eco2, jamBuf);
      sharedState.pushEvent(epochNow, "air_bad", (data.aqi >= 5) ? "danger" : "warning", msg);
    }

    if (jamMalam && data.adaOrang &&
        (lastNightEventMs == 0 || now - lastNightEventMs >= DEBOUNCE_EVENT_MS)) {
      lastNightEventMs = now;
      char msg[128];
      snprintf(msg, sizeof(msg), "Terdeteksi keberadaan orang (%s) pada jam malam (%s)", data.presenceState, jamBuf);
      sharedState.pushEvent(epochNow, "night_presence", "danger", msg);
    }

    // Perbarui snapshot thread-safe untuk dikirim ke Firebase oleh Core 0
    TelemetrySnapshot snap;
    memset(&snap, 0, sizeof(snap));
    snap.valid               = true;
    snap.temperature         = data.suhu;
    snap.humidity            = data.kelembapan;
    strncpy(snap.humidityLabel, data.labelKelembapan, sizeof(snap.humidityLabel) - 1);
    snap.airQualityAqi       = data.aqi;
    snap.airQualityTvoc      = data.tvoc;
    snap.airQualityEco2      = data.eco2;
    snap.airQualityWarmingUp = data.warmingUp;
    strncpy(snap.airQualityLabel, data.labelAQI, sizeof(snap.airQualityLabel) - 1);
    snap.lightLux            = data.lux;
    strncpy(snap.lightLabel, data.labelCahaya, sizeof(snap.lightLabel) - 1);
    snap.presenceDetected    = data.adaOrang;
    strncpy(snap.presenceState, data.presenceState, sizeof(snap.presenceState) - 1);
    snap.alarmActive         = alarm.active;
    strncpy(snap.alarmReason, alarm.reason, sizeof(snap.alarmReason) - 1);
    snap.nightModeActive     = jamMalam;
    snap.deviceTimestamp     = epochNow;

    sharedState.setTelemetry(snap);
    sharedState.setActuatorStatus(fan.isNyala(), light.isNyala());

    // Log ringkas ke Serial Monitor
    NetworkStatusSnapshot net = sharedState.getNetworkStatus();
    Serial.printf("[%s] T:%.1fC H:%.0f%%(%s) | AQI:%u(%s) TVOC:%u | Lux:%.0f(%s) | Radar:%s | Fan:%s Lamp:%s | Cloud:%s(HTTP %d)\n",
                  jamBuf,
                  isnan(data.suhu) ? 0.0f : data.suhu,
                  isnan(data.kelembapan) ? 0.0f : data.kelembapan,
                  data.labelKelembapan,
                  data.aqi, data.labelAQI, data.tvoc,
                  data.lux < 0 ? 0.0f : data.lux, data.labelCahaya,
                  data.radarOk ? data.presenceState : "DISCONNECT",
                  fan.isNyala() ? "ON" : "OFF",
                  light.isNyala() ? "ON" : "OFF",
                  net.wifiConnected ? (net.firebaseAuthed ? "ONLINE" : "AUTH..") : "OFFLINE",
                  net.lastHttpCode);
  }
}
