#pragma once
#include <Arduino.h>

// ==============================================================================
// config.h - Sentralisasi Konfigurasi Hardware, Pinout, dan Parameter SRS v0.9
// Proyek: SmartRoomMonitor (RPL) - Firmware Produksi ESP32-S3 + Firebase RTDB
// Diselaraskan penuh dengan Dokumen Pemodelan UML & SRS v0.9.
// ==============================================================================

// --- Opsi Fitur Radio ---
#define ENABLE_BLE_DEBUG                0

// --- Definisi Pinout Resmi ESP32-S3 DevKitC-1 N16R8 ---
// Catatan: Hindari GPIO 26-37 (jalur internal Octal Flash/PSRAM).
#define PIN_I2C_SDA                     4    // Bus I2C bersama (SHT30, ENS160, BH1750)
#define PIN_I2C_SCL                     5
#define PIN_RADAR_RX                    18   // UART1 RX ESP32 <- TX Radar LD2410B
#define PIN_RADAR_TX                    17   // UART1 TX ESP32 -> RX Radar LD2410B
#define PIN_BUZZER                      6    // Driver Transistor PNP Active-LOW (VCC 3.3V)
#define PIN_FAN_MOSFET                  7    // Gate Driver MOSFET D4184 Kipas 5V (FR-02)
#define PIN_RELAY_LIGHT                 15   // Trigger Relay 1-Ch Open-Drain Lampu 5V (FR-08)

// --- Baud Rate Komunikasi ---
#define SERIAL_BAUD                     115200
#define RADAR_BAUD                      256000

// --- Penjadwalan Waktu & Interval (ms) ---
#define INTERVAL_SENSOR_EVAL_MS         2000UL    // Evaluasi sensor & aktuator lokal di Core 1
#define INTERVAL_TELEMETRI_PUSH_MS      3000UL    // Kirim /room1/current ke RTDB (FR-B02 <= 5s, NFR-01 <= 7s)
#define INTERVAL_CONTROL_POLL_MS        2000UL    // Ambil /room1/actuator & /room1/alarm/mute (FR-06 <= 7s)
#define INTERVAL_THRESHOLD_POLL_MS      30000UL   // Sinkronisasi /room1/thresholds dari Firebase Console
#define INTERVAL_HISTORY_LOG_MS         600000UL  // Simpan 1 titik /room1/history tiap 10 menit (144 titik/24 jam, UML UC-07)
#define HISTORY_RETENTION_SEC           86400UL   // Retensi grafik 24 jam (86400 detik)
#define WARMUP_ENS160_MS                180000UL  // Masa pemanasan 3 menit bebas alarm palsu (FR-03, NFR-06)
#define WARMUP_BASELINE_START_MS        120000UL  // Mulai sampling baseline TVOC di menit ke-2 s.d. ke-3
#define DEBOUNCE_EVENT_MS               60000UL   // Debounce pencatatan event bahaya 60 detik (FR-01)
#define WIFI_RECONNECT_INTERVAL_MS      5000UL    // Coba sambung ulang Wi-Fi non-blocking (NFR-02)
#define FIREBASE_TOKEN_REFRESH_MS       3000000UL // Refresh idToken Auth tiap 50 menit (exp 60 menit)

// --- Parameter Radar mmWave Hilink HLK-LD2410B (3-Layer Anti-False-Positive) ---
#define RADAR_DEFAULT_GATE_MAX          2         // ~1.50m (Gate 2 - Meja Kerja)
#define RADAR_DEFAULT_TIMEOUT_DETIK     3         // 3 detik hardware timeout
#define RADAR_BREATHING_THRESHOLD       15        // Ambang batas napas mikro peka (15%)
#define RADAR_CONFIRM_DELAY_MS          800UL     // Layer 1 (On-Delay): tolak lemparan < 400ms
#define RADAR_GLITCH_TOLERANCE_MS       800UL     // Toleransi jeda frame selama verifikasi
#define RADAR_BIOMETRIC_STILL_MIN       20        // Layer 2: jejak napas/mikro-gerak valid (20%)
#define RADAR_ENTRY_MOVING_THRESHOLD    35        // Layer 3: ambang gerak masuk (35%)
#define RADAR_ENTRY_STILL_THRESHOLD     20        // Layer 3: ambang diam masuk (20%)
#define RADAR_HOLD_THRESHOLD            10        // Layer 3: ambang tahan keberadaan longgar (10%)
#define RADAR_HOLD_GRACE_MS             3000UL    // Masa tahan ekstra 3 detik saat napas melemah

// --- Nilai Default Thresholds (Dapat Ditimpa Dinamis dari /room1/thresholds) ---
#define SUHU_KIPAS_ON_DEFAULT                   31.0f // FR-02: Kipas ON saat suhu > 31.0 °C
#define SUHU_KIPAS_HISTERESIS_DEFAULT           1.0f  // FR-02: Kipas OFF saat turun 1.0 °C (<= 30.0 °C)
#define SUHU_ALERT_DEFAULT                      35.0f // FR-01: Peringatan suhu tinggi > 35.0 °C
#define AIR_DELTA_MODERATE_DEFAULT              80    // NFR-06: Kenaikan TVOC dari baseline kategori Sedang
#define AIR_DELTA_BAD_DEFAULT                   160   // NFR-06 & UML UC-04: Kenaikan >= 160 kategori Buruk
#define AMBANG_CAHAYA_GELAP_LUX_DEFAULT         40.0f // FR-07/08 & UML UC-05: < 40 Lux dianggap Gelap
#define HISTERESIS_CAHAYA_EKSTERNAL_LUX_DEFAULT 200.0f // FR-08: +200 Lux eksternal baru dianggap terang
#define TIMEOUT_KOSONG_LAMPU_SEC_DEFAULT        10    // FR-08: 10 detik kosong sebelum lampu padam
#define JAM_MALAM_MULAI_DEFAULT                 22    // FR-05: Jam malam 22:00 WIB
#define JAM_MALAM_SELESAI_DEFAULT               6     // FR-05: Berakhir 06:00 WIB

// --- NTP Waktu Indonesia Barat (WIB / UTC+7) ---
#define NTP_GMT_OFFSET_SEC              (7 * 3600)
#define NTP_DAYLIGHT_OFFSET_SEC         0
#define NTP_SERVER_PRIMARY              "pool.ntp.org"
#define NTP_SERVER_SECONDARY            "time.google.com"

// --- Path Root Ruangan di Firebase RTDB ---
#define FIREBASE_ROOM_PATH              "/room1"
