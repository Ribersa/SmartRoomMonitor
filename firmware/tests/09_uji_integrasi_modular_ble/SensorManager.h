#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <DFRobot_ENS160.h>
#include <BH1750.h>
#include <ld2410.h>
#include "config.h"

// ==============================================================================
// SensorManager.h - Pengelola Terpadu 4 Sensor: SHT30-D, ENS160, BH1750, LD2410
// ==============================================================================

struct TelemetriData {
  // Modul M1: Suhu & Kelembapan
  float suhu;
  float kelembapan;
  bool sht30Ok;

  // Modul M2: Kualitas Udara
  uint8_t aqi;
  uint16_t tvoc;
  uint16_t eco2;
  const char* labelAQI;
  bool ens160Ok;

  // Modul M4: Iluminansi Cahaya
  float lux;
  const char* labelCahaya;
  bool bh1750Ok;

  // Modul M3: Radar Keberadaan & Napas (Terfilter Anti-False-Positive)
  bool radarOk;
  bool adaOrang;
  bool sedangVerifikasi;
  bool bergerak;
  bool diam;
  uint16_t jarakGerak;
  uint8_t energiGerak;
  uint16_t jarakDiam;
  uint8_t energiDiam;
  char barGerak[14];
  char barDiam[14];
  const char* statusRadar;
};

// State Machine 3-Tahap untuk membedakan Manusia vs Benda Jatuh/Dilempar
enum StateKehadiranRadar : uint8_t {
  STATE_RADAR_KOSONG = 0,       // Tidak ada target valid
  STATE_RADAR_VERIFIKASI = 1,   // Sinyal masuk < 1.5 detik (menguji durasi & komponen napas/still)
  STATE_RADAR_MANUSIA_HADIR = 2 // Terkonfirmasi manusia (memicu Lampu FR-08 / Alarm FR-05)
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

  int _activeRxPin;
  int _activeTxPin;

  // Variabel State Machine Filter Anti-False-Positive Radar
  StateKehadiranRadar _stateKehadiran;
  unsigned long _waktuMulaiVerifikasi;
  unsigned long _waktuTerakhirSinyalAda;
  unsigned long _waktuTerakhirRadarKonek;
  bool _jejakNapasTerdeteksi;

  bool hubungkanRadarAutoSwap();
  void evaluasiFilterKehadiran();
  void cetakBarEnergi(uint8_t energi, char* bufferOut);

public:
  SensorManager();
  void begin();
  void updateRadar(); // Wajib dipanggil di setiap siklus loop() tanpa delay
  TelemetriData bacaSemua();

  // Pengaturan & Kalibrasi Radar M3
  bool setRadarGate(uint8_t gateMaks, uint16_t timeoutDetik, const char* label);
  void boostSensitivitasNapas();
  bool setBluetoothRadar(bool aktif);
  void bacaInfoRadar(char* bufferOut, size_t maxLen);

  bool isRadarConnected();
};
