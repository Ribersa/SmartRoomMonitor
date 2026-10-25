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

// --- Baud Rate Komunikasi ---
#define SERIAL_BAUD        115200
#define RADAR_BAUD         256000

// --- Pengaturan Waktu & Interval (ms) ---
#define INTERVAL_TELEMETRI_DEFAULT_MS  2000  // Standar SRS v0.9 NFR-01 (<= 3s)
#define INTERVAL_TELEMETRI_CEPAT_MS    1000

// --- Parameter Radar mmWave Hilink HLK-LD2410 ---
#define RADAR_DEFAULT_GATE_MAX         2     // ~1.50m (Gate 2 - Meja Dekat)
#define RADAR_DEFAULT_TIMEOUT_DETIK    5     // 5 detik timeout keberadaan
#define RADAR_BREATHING_THRESHOLD      15    // Ambang batas napas mikro peka (15%)

// --- UUID Standar Nordic Semiconductor UART Service (NUS) ---
#define BLE_DEVICE_NAME        "SmartRoom-ESP32"
#define BLE_SERVICE_UUID       "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHAR_UUID_RX       "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_CHAR_UUID_TX       "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
