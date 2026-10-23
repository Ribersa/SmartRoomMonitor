// ==============================================================================
// 05_uji_telemetri_ble_serial.ino
// Proyek: SmartRoomMonitor (RPL)
// Deskripsi: Streaming telemetri multi-sensor ke HP via Bluetooth Low Energy (BLE)
//            menggunakan protokol Nordic UART Service (NUS).
// Kompatibel: Serial Bluetooth Terminal (Kai Morich) & nRF Connect (Android/iOS).
// Hardware: ESP32-S3 DevKitC-1 + SHT30 + ENS160 + BH1750 + Buzzer
// ==============================================================================

#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <DFRobot_ENS160.h>
#include <BH1750.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define I2C_SDA 4
#define I2C_SCL 5
#define BUZZER_PIN 6

// UUID Standar Nordic Semiconductor UART Service (NUS)
// Standar ini yang dikenali langsung oleh aplikasi "Serial Bluetooth Terminal"
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

Adafruit_SHT31 sht30 = Adafruit_SHT31();
DFRobot_ENS160_I2C ens160(&Wire, 0x53);
BH1750 lightMeter;

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("\n>>> [BLE] HP Terhubung ke Terminal Serial! <<<");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("\n>>> [BLE] HP Terputus. Menunggu koneksi baru... <<<");
    }
};

void matikanBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  matikanBuzzer();

  delay(2000);
  Serial.println("\n=======================================================");
  Serial.println("  [SmartRoomMonitor] BLE Nordic UART Terminal Ready");
  Serial.println("=======================================================");

  Wire.begin(I2C_SDA, I2C_SCL);
  sht30.begin(0x44);
  ens160.begin();
  ens160.setPWRMode(ENS160_STANDARD_MODE);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  // Inisialisasi BLE
  BLEDevice::init("SmartRoom-ESP32");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Buat Layanan Nordic UART
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Buat Karakteristik TX (ESP32 kirim ke HP)
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Buat Karakteristik RX (HP kirim ke ESP32)
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );

  pService->start();

  // Mulai Advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("[OK] BLE UART Aktif! Nama: 'SmartRoom-ESP32'");
  Serial.println("[PETUNJUK] Buka Serial Bluetooth Terminal -> Menu Devices -> Tab 'Bluetooth LE'.\n");
}

void loop() {
  // 1. Baca data sensor
  float suhu = sht30.readTemperature();
  float kelembapan = sht30.readHumidity();

  if (!isnan(suhu) && !isnan(kelembapan)) {
    ens160.setTempAndHum(suhu, kelembapan);
  }

  uint8_t aqi   = ens160.getAQI();
  uint16_t tvoc = ens160.getTVOC();
  uint16_t eco2 = ens160.getECO2();
  float lux     = lightMeter.readLightLevel();

  // 2. Format baris teks serial dengan CRLF (\r\n) agar tampil rapi baris per baris
  char teksTerminal[128];
  snprintf(teksTerminal, sizeof(teksTerminal), 
           "🌡️ %.1fC | 💧 %.1f%% | 💡 %.0f lx | AQI:%d (TVOC:%d)\r\n", 
           suhu, kelembapan, lux, aqi, tvoc);

  Serial.print(teksTerminal);

  // 3. Kirim ke HP jika terhubung
  if (deviceConnected) {
    pTxCharacteristic->setValue((uint8_t*)teksTerminal, strlen(teksTerminal));
    pTxCharacteristic->notify();
  }

  // Auto-reconnect advertising
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(2000);
}
