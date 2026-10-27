#pragma once
#include <Arduino.h>

// ==============================================================================
// config.h - Sentralisasi Konfigurasi Hardware, Pinout, dan Parameter Sistem
// Proyek: SmartRoomMonitor (RPL) - Tim Pengembang ESP32-S3
// PIC Modul M3: Deswiryawan Saragih (241111008)
// ==============================================================================

// --- Definisi Pinout ESP32-S3 DevKitC-1 N16R8 ---
#define PIN_I2C_SDA        4    // Bus I2C bersama (SHT30, ENS160, BH1750)
#define PIN_I2C_SCL        5
#define PIN_RADAR_RX       18   // UART1 terhubung ke TX Radar
#define PIN_RADAR_TX       17   // UART1 terhubung ke RX Radar
#define PIN_BUZZER         6    // Driver Transistor PNP Active-LOW
#define PIN_FAN_MOSFET     7    // Gate Kontrol Driver MOSFET Kipas 5V (FR-02)
#define PIN_RELAY_LIGHT    15   // Sinyal Trigger Modul Relay 1-Channel Lampu (FR-08)

// --- Baud Rate Komunikasi ---
#define SERIAL_BAUD        115200
#define RADAR_BAUD         256000

// --- Pengaturan Waktu & Interval (ms) ---
#define INTERVAL_TELEMETRI_DEFAULT_MS  2000  // Standar SRS v0.9 NFR-01 (<= 3s)
#define INTERVAL_TELEMETRI_CEPAT_MS    1000

// --- Parameter Radar mmWave Hilink HLK-LD2410 ---
#define RADAR_DEFAULT_GATE_MAX         2     // ~1.50m (Gate 2 - Meja Dekat)
#define RADAR_DEFAULT_TIMEOUT_DETIK    3     // 3 detik hardware timeout (dikombinasi filter software)
#define RADAR_BREATHING_THRESHOLD      15    // Ambang batas napas mikro peka (15%)

// --- Parameter Filter Anti-False-Positive Radar (3-Layer Human Validator) ---
#define RADAR_CONFIRM_DELAY_MS         800   // Layer 1 (On-Delay): Tahan 800ms (responsif tapi tetap buang lemparan <400ms)
#define RADAR_GLITCH_TOLERANCE_MS      800   // Toleransi jeda frame (800ms) agar timer verifikasi tidak gampang reset
#define RADAR_BIOMETRIC_STILL_MIN      20    // Layer 2 (Moving+Still): Jejak napas/mikro-gerak valid (20%)
#define RADAR_ENTRY_MOVING_THRESHOLD   35    // Layer 3 (Histeresis): Ambang gerak masuk (35%)
#define RADAR_ENTRY_STILL_THRESHOLD    20    // Layer 3 (Histeresis): Ambang diam masuk (20%)
#define RADAR_HOLD_THRESHOLD           10    // Layer 3 (Histeresis): Ambang tahan keberadaan longgar (10%)
#define RADAR_HOLD_GRACE_MS            3000  // Masa tahan ekstra saat sinyal napas melemah sesaat (3 detik)

// --- Parameter Kipas 5V (SRS v0.9 FR-02) ---
#define SUHU_KIPAS_ON              31.0f // Kipas menyala saat suhu > 31.0 °C
#define SUHU_KIPAS_HISTERESIS      1.0f  // Kipas mati setelah suhu turun 1.0 °C (<= 30.0 °C)

// --- Parameter Lampu & Relay 5V (SRS v0.9 FR-08) ---
#define AMBANG_CAHAYA_GELAP_LUX    50.0f // Di bawah 50 Lux dianggap gelap
#define RELAY_TRIGGER_ACTIVE       HIGH  // Active-HIGH trigger: HIGH = Relay ON (Klik/Lampu Nyala)
#define RELAY_TRIGGER_INACTIVE     LOW   // LOW = Relay OFF (Lepas/Lampu Padam)
#define TIMEOUT_KOSONG_LAMPU_MS    10000 // 10 detik tanpa orang sebelum lampu otomatis mati

// --- UUID Standar Nordic Semiconductor UART Service (NUS) ---
#define BLE_DEVICE_NAME        "SmartRoom-ESP32"
#define BLE_SERVICE_UUID       "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHAR_UUID_RX       "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHAR_UUID_TX       "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
