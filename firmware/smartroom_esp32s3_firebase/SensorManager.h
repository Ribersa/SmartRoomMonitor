#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <DFRobot_ENS160.h>
#include <BH1750.h>
#include <ld2410.h>
#include "config.h"

// ==============================================================================
// SensorManager.h - Pengelola 4 Sensor Produksi SRS v0.9
// SHT30-D (M1), ENS160 + Warm-up 3 Menit (M2), HLK-LD2410B 3-Layer (M3), BH1750 (M4)
// ==============================================================================

struct TelemetriData {
  // Modul M1: Suhu & Kelembapan
  float suhu;
  float kelembapan;
  const char* labelKelembapan; // "Kering", "Nyaman", "Lembap"
  bool sht30Ok;

  // Modul M2: Kualitas Udara
  uint8_t aqi;
  uint16_t tvoc;
  uint16_t eco2;
  bool warmingUp;              // true selama 3 menit pertama (FR-03, NFR-06)
  const char* labelAQI;        // "Memanaskan", "Baik", "Sedang", "Buruk"
  bool ens160Ok;

  // Modul M4: Iluminansi Cahaya
  float lux;
  const char* labelCahaya;     // "Gelap", "Normal", "Terang"
  bool bh1750Ok;

  // Modul M3: Radar Kehadiran 3-Lapis Anti-False-Positive
  bool radarOk;
  bool adaOrang;
  bool sedangVerifikasi;
  bool bergerak;
  bool diam;
  uint16_t jarakGerak;
  uint8_t energiGerak;
  uint16_t jarakDiam;
  uint8_t energiDiam;
  const char* presenceState;   // "tidak ada", "bergerak", "diam" (SRS FR-05)
};

enum StateKehadiranRadar : uint8_t {
  STATE_RADAR_KOSONG = 0,
  STATE_RADAR_VERIFIKASI = 1,
  STATE_RADAR_MANUSIA_HADIR = 2
};

class SensorManager {
private:
  Adafruit_SHT31 _sht30;
  DFRobot_ENS160_I2C _ens160;
  BH1750 _lightMeter;
  ld2410 _radar;

  bool _sht30Ok;
  bool _ens160Ok;
  bool _bh1750Ok;
  bool _radarOk;

  unsigned long _bootMillis;
  StateKehadiranRadar _stateKehadiran;
  unsigned long _waktuMulaiVerifikasi;
  unsigned long _waktuTerakhirSinyalAda;
  unsigned long _waktuTerakhirRadarKonek;
  bool _jejakNapasTerdeteksi;

  bool hubungkanRadarAutoSwap();
  void evaluasiFilterKehadiran();

public:
  SensorManager();
  void begin();
  void updateRadar();
  TelemetriData bacaSemua(float darkLuxThreshold);

  bool setRadarGate(uint8_t gateMaks, uint16_t timeoutDetik);
  void boostSensitivitasNapas();
  bool isRadarConnected();
  bool isEns160WarmingUp() const;
};
