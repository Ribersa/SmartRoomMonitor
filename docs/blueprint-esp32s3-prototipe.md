# Blueprint & Skematik Prototipe ESP32-S3
*(ESP32-S3 + SHT30-D + ENS160/AHT21 + BH1750 + HLK-LD2410 + Buzzer + Fan MOSFET + Relay Lamp)*

Dokumen ini adalah spesifikasi perakitan breadboard dan arsitektur komunikasi khusus untuk mikrokontroler **ESP32-S3 DevKit** (Xtensa dual-core LX7, USB OTG, BLE 5.0).

---

## 1. Pemetaan Pinout Resmi ESP32-S3

| Pin ESP32-S3 | Komponen Terhubung | Pin Komponen | Protokol / Sifat Sinyal | Catatan Teknis |
|---|---|---|---|---|
| **3V3** | SHT30-D, ENS160/AHT21, BH1750 | VCC | Daya 3.3V DC | Suplai rail 3.3V sensor I2C |
| **VIN (5V)** | Radar LD2410, Buzzer, Kipas, Relay | VCC / 5V | Daya 5V DC | Jalur daya 5V utama adaptor USB 2A |
| **GND** | Semua Modul & Aktuator | GND | Ground Bersama | Terhubung ke satu titik ground sentral |
| **GPIO 4** | SHT30-D, ENS160, BH1750 | **SDA** | I2C Data Bus | Bus I2C bersama (logika 3.3V) |
| **GPIO 5** | SHT30-D, ENS160, BH1750 | **SCL** | I2C Clock Bus | Bus I2C bersama (logika 3.3V) |
| **GPIO 18** | HLK-LD2410 | **TX** | Input Serial UART | Menerima telemetri radar (Koneksi silang: TX -> RX S3) |
| **GPIO 17** | HLK-LD2410 | **RX** | Output Serial UART | Mengirim konfigurasi radar (Koneksi silang: RX -> TX S3) |
| **GPIO 16** | HLK-LD2410 | **OUT** | Input Digital GPIO | Sinyal interupsi deteksi cepat (*Active HIGH*) |
| **GPIO 6** | Modul Active Buzzer | Sinyal / I/O | Output Digital | Alarm keamanan malam (22-06) & peringatan udara buruk |
| **GPIO 7** | Modul MOSFET D4184 | PWM / TRIG | Output PWM (LEDC) | Pengaturan RPM halus kipas 5V (Target 25 kHz) |
| **GPIO 15** | Modul Relay 1 Channel | IN / Sinyal | Output Digital | Pemicu saklar lampu USB 5V (Otomatis gelap + ada orang) |
| **GPIO 2** | LED Status Sistem | Anoda (+) | Output Digital | Indikator visual koneksi via resistor 330 Ω |

---

## 2. Peta Alamat I2C Bus (Semua Bebas Bentrok)

| Modul Sensor | Alamat I2C Default | Alamat Alternatif | Parameter yang Dibaca |
|---|---|---|---|
| **BH1750** | `0x23` (ADDR ke GND) | `0x5C` (ADDR ke VCC) | Iluminansi Cahaya Ruangan (Lux) |
| **AHT21** (on-board ENS) | `0x38` | Tetap | Suhu Internal Komponen Gas (Pembanding) |
| **Sensirion SHT30-D** | `0x44` (ADDR ke GND) | `0x45` (ADDR ke VCC) | Suhu Ruangan Murni (±0.2°C) & Kelembapan (±2% RH) |
| **ENS160** | `0x52` (ADDR ke GND) | `0x53` (ADDR ke VCC) | Kualitas Udara (AQI 1-5 UBA, TVOC ppb, eCO2 ppm) |

---

## 3. Diagram Visual Wiring Breadboard (ESP32-S3)

```text
       +-----------------------------------------------------------------------------------------------------------+
  (+)  | [3.3V Rail] ====(dari ESP32-S3 3V3)==================+===================+===================+=========== |
  (-)  | [GND Rail]  ====(dari ESP32-S3 GND)===+==============|===================|===================|=========== |
       +---------------------------------------|--------------|-------------------|-------------------|------------+
                                               |              |                   |                   |
                                       [Elco 100uF]           |                   |                   |
                                               |              |                   |                   |
   A   |       [ ESP32-S3 DEVKIT ]             |   [ SHT30-D ]|   [ ENS160/AHT21 ]|         [ BH1750 ]|   (Aktuator)
   B   |   Khurs IOT (Pre-soldered)            |              |                   |                   |
   C   |                                       |  +---------+ |   +-------------+ |        +--------+ |   [BUZZER]
   D   |                                       |  |VCC  3.3V|<+   |VCC      3.3V|<+        |VCC 3.3V|<+   Pin: GPIO 6
   E   |                                       |  |GND  ----+<----|GND      ----+<----+    |GND ----+<----+
       |                                       |  |SCL   (5)|<--+ |SCL       (5)|<---+|    |SCL  (5)|<--++   [MOSFET FAN]
   = PARIT TENGAH BREADBOARD ==================|==|SDA   (4)|<-+|==|SDA       (4)|<-+||====|SDA  (4)|<-+|| Pin: GPIO 7
       |                                       |  +---------+  || +-------------+  |||     +--------+   |||
   F   |   Pin ESP32-S3 yang Dipakai:          |       |       ||      |           |||          |       |||   [RELAY LAMP]
   G   |   * 3V3    ---> Rel Atas (+)          |   [C 100nF]   ||  [C 100nF]       |||      [C 100nF]   ||| Pin: GPIO 15
   H   |   * GND    ---> Rel Atas (-)          |   (VCC-GND)   ||  (VCC-GND)       |||      (VCC-GND)   |||
   I   |   * VIN    ---> Rel Bawah (+)         |               ||                  |||                  |||
   J   |   * GPIO 4 ---> Bus SDA --------------+---------------+-------------------++-------------------+-+
       |   * GPIO 5 ---> Bus SCL ------------------------------+--------------------+---------------------+
       |   * GPIO 18 --> Kabel TX Radar ------------------------------------------------+
       |   * GPIO 17 --> Kabel RX Radar ---------------------------------------------+  |   [ RADAR HLK-LD2410 ]
       |   * GPIO 16 --> Kabel OUT Radar -----------------------------------------+  |  |   (Kabel Bawaan 5-Pin)
       |   * GPIO 6  --> Pin Sinyal Buzzer                                        |  |  |   * VCC -> Rel 5V
       |   * GPIO 7  --> Pin Trigger MOSFET Kipas                                 |  |  |   * GND -> Rel GND
       |   * GPIO 15 --> Pin Sinyal Modul Relay Lampu                             |  |  |   * TX  -> GPIO 18
       |   * GPIO 2  --> [R 330Ω] -> (+)LED(-) -> GND                             |  |  |   * RX  -> GPIO 17
       +--------------------------------------------------------------------------|--|--|-- * OUT -> GPIO 16
  (+)  | [5V Rail]   ====(dari ESP32-S3 VIN)=================[Elco 1000uF/16V]----+--|--|--+
  (-)  | [GND Rail]  ====(Jumper sambung ke Rel Atas GND)=========================+--|--+--+
       +-----------------------------------------------------------------------------+
```
