#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "config.h"
#include "BuzzerManager.h"
#include "SensorManager.h"
#include "FanManager.h"
#include "LightManager.h"

// ==============================================================================
// BleManager.h - Pengelola BLE Nordic UART Service (NUS) & Perintah Dua Arah
// ==============================================================================

class BleManager {
private:
  BLEServer* _pServer;
  BLECharacteristic* _pTxCharacteristic;
  bool _bleConnected;
  bool _oldBleConnected;

  BuzzerManager* _buzzer;
  SensorManager* _sensors;
  FanManager*    _fan;
  LightManager*  _light;

  bool _modeJamMalam;
  unsigned long _intervalTelemetriMs;

  void eksekusiPerintah(char cmd);

public:
  BleManager();
  void begin(BuzzerManager* buzzer, SensorManager* sensors, FanManager* fan, LightManager* light);
  void update(); // Tangani rekoneksi advertising & perintah masuk
  void kirimPesan(const char* teks);
  void kirimTelemetri(const TelemetriData& d);
  void tampilkanMenuBantuan();

  bool isJamMalam() const { return _modeJamMalam; }
  unsigned long getIntervalMs() const { return _intervalTelemetriMs; }
  bool isConnected() const { return _bleConnected; }

  // Callback friend/internal
  void setConnected(bool conn) { _bleConnected = conn; }
};
