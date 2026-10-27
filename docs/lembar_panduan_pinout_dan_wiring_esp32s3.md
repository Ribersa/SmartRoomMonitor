# Lembar Panduan Pinout & Wiring Breadboard ESP32-S3
**Proyek:** SmartRoomMonitor (ESP32-S3 DevKitC-1 N16R8)  
**Dokumen Referensi:** Standar Perakitan Fisik Bersih (Clean Breadboard Setup)  
**Target Hardware:** ESP32-S3 + SHT30-D + ENS160 + BH1750 + HLK-LD2410 + Buzzer + MOSFET Kipas + Relay Lampu  

---

## 1. Aturan Emas Rel Daya (Power Rail Discipline)

Untuk melindungi modul sensor 3.3V agar tidak rusak terkena tegangan 5V, gunakan disiplin pembagian dua rel panjang di breadboard:

```text
======================= REL ATAS: KHUSUS 3.3V (SENSOR) =======================
  [+] Rel Merah : 3.3V DC (Disuplai murni dari pin 3V3 ESP32)
  [-] Rel Biru  : GND (Ground Bersama)
  Komponen      : SHT30-D, ENS160, BH1750 (Opsional: Elco 100µF)
==============================================================================

====================== REL BAWAH: KHUSUS 5.0V (AKTUATOR) ======================
  [+] Rel Merah : 5.0V DC (Disuplai dari Adaptor Dinding 3A via Type-C Breakout / VIN)
  [-] Rel Biru  : GND (Ground Bersama)
  Komponen      : HLK-LD2410 Radar, Buzzer PNP, MOSFET Kipas, Relay Lampu
  Peredam Kejut : [Kapasitor Elco 1000µF / 16V] melintang antara (+) 5V dan (-) GND!
==============================================================================

========================= JUMPER COMMON GROUND (WAJIB) =========================
  Hubungkan (-) Rel Biru Atas ke (-) Rel Biru Bawah menggunakan seutas kabel jumper hitam.
  Semua sinyal digital (I2C, UART, PWM, Relay) WAJIB berbagi referensi ground yang sama!
==============================================================================
```

---

## 2. Tabel Master Pinout ESP32-S3 (Pin-ke-Pin)

| Pin ESP32-S3 | Terhubung Menuju | Pin pada Modul | Level Tegangan | Fungsi & Catatan Perakitan |
|---|---|---|---|---|
| **3V3** | **Rel Atas (+)** | - | 3.3V DC | Sumber daya rel 3.3V untuk semua modul sensor |
| **GND** | **Rel Atas (-)** | - | 0V (Ground) | Titik ground utama mikrokontroler |
| **5V / VIN** | **Rel Bawah (+)** | - | 5.0V DC | Terhubung ke rel daya 5V adaptor eksternal |
| **GPIO 4** | SHT30, ENS160, BH1750 | **SDA** | 3.3V Logika | Jalur Data Bus I2C bersama (Paralel ketiga sensor) |
| **GPIO 5** | SHT30, ENS160, BH1750 | **SCL** | 3.3V Logika | Jalur Clock Bus I2C bersama (Paralel ketiga sensor) |
| **GPIO 18** | Radar HLK-LD2410 | **TX** | 3.3V UART | **Koneksi Silang:** TX Radar masuk ke RX ESP32 (GPIO 18) |
| **GPIO 17** | Radar HLK-LD2410 | **RX** | 3.3V UART | **Koneksi Silang:** RX Radar menerima dari TX ESP32 (GPIO 17) |
| **GPIO 16** | Radar HLK-LD2410 | **OUT** | 3.3V Digital | *(Opsional)* Pin interupsi sinyal deteksi cepat |
| **GPIO 6** | Modul Buzzer Pasif | **I/O / SIG** | 3.3V / 5V | Driver Transistor PNP Active-LOW (LOW = Bunyi) |
| **GPIO 7** | Modul MOSFET D4184 | **TRIG / PWM** | 3.3V PWM | Kontrol kecepatan kipas 5V via gerbang MOSFET |
| **GPIO 15** | Modul Relay 1-Channel | **IN** | 3.3V / Open-Drain | Trigger Lampu (LOW = Relay ON, INPUT/Hi-Z = Relay OFF) |
| **GPIO 2** | LED Indikator | **Anoda (+)** | 3.3V via 330Ω | *(Opsional)* Indikator visual aktivitas/koneksi |

---

## 3. Rincian Pengkabelan Tiap Modul

### 3.1 Kelompok Sensor I2C (Rel Atas 3.3V)
Semua pin SDA digabung ke baris breadboard yang sama (GPIO 4), dan semua pin SCL digabung ke GPIO 5:
- **Sensirion SHT30-D (Suhu & RH - Alamat `0x44`):**
  - `VCC` $\rightarrow$ Rel Atas (+) [3.3V]
  - `GND` $\rightarrow$ Rel Atas (-) [GND]
  - `SDA` $\rightarrow$ GPIO 4
  - `SCL` $\rightarrow$ GPIO 5
- **ScioSense ENS160 (Kualitas Udara - Alamat `0x53`):**
  - `VCC` $\rightarrow$ Rel Atas (+) [3.3V]
  - `GND` $\rightarrow$ Rel Atas (-) [GND]
  - `SDA` $\rightarrow$ GPIO 4
  - `SCL` $\rightarrow$ GPIO 5
  - *(Catatan: Pin ADDR biarkan mengambang atau ke VCC untuk alamat 0x53)*
- **Rohm BH1750 (Cahaya Lux - Alamat `0x23`):**
  - `VCC` $\rightarrow$ Rel Atas (+) [3.3V]
  - `GND` $\rightarrow$ Rel Atas (-) [GND]
  - `ADDR` $\rightarrow$ Rel Atas (-) [GND] (Wajib ke GND agar alamatnya 0x23)
  - `SDA` $\rightarrow$ GPIO 4
  - `SCL` $\rightarrow$ GPIO 5

---

### 3.2 Radar mmWave Hilink HLK-LD2410 (Rel Bawah 5V)
> [!IMPORTANT]
> Radar HLK-LD2410 membutuhkan tegangan suplai **5.0V penuh** (minimal 4.5V) agar osilator RF 24 GHz bekerja stabil. Jangan dicolok ke rel 3.3V!

- **Pin 1 (VCC / Kabel Merah):** Rel Bawah (+) [5V]
- **Pin 2 (GND / Kabel Hitam):** Rel Bawah (-) [GND]
- **Pin 3 (OUT / Kabel Kuning):** ESP32 **GPIO 16** *(Opsional)*
- **Pin 4 (TX / Kabel Biru):** ESP32 **GPIO 18**
- **Pin 5 (RX / Kabel Hijau/Putih):** ESP32 **GPIO 17**

---

### 3.3 Aktuator Beban Berat (Rel Bawah 5V)

#### A. Driver Kipas 5V (Modul MOSFET D4184)
- **Sisi Kontrol (Pin Header):**
  - `TRIG / PWM` $\rightarrow$ ESP32 **GPIO 7**
  - `GND` $\rightarrow$ Rel Bawah (-) [GND]
- **Sisi Daya (Terminal Sekrup):**
  - `VIN (+)` $\rightarrow$ Rel Bawah (+) [5V]
  - `VIN (-)` $\rightarrow$ Rel Bawah (-) [GND]
  - `V+ (Output)` $\rightarrow$ Kabel Merah (+) Kipas 5V
  - `V- (Output)` $\rightarrow$ Kabel Hitam (-) Kipas 5V

#### B. Driver Lampu 5V (Modul Relay 1-Channel)
- **Sisi Daya & Kontrol:**
  - `VCC` $\rightarrow$ Rel Bawah (+) [5V]
  - `GND` $\rightarrow$ Rel Bawah (-) [GND]
  - `IN` $\rightarrow$ ESP32 **GPIO 15**
- **Sisi Saklar Kontak (Terminal Sekrup):**
  - `NO (Normally Open)` $\rightarrow$ Sambungkan ke Rel Bawah (+) [5V]
  - `COM (Common)` $\rightarrow$ Sambungkan ke Kabel Positif (+) Lampu 5V
  - *(Kabel Negatif (-) Lampu langsung dicolok ke Rel Bawah (-) [GND])*

#### C. Modul Buzzer Pasif Active-LOW (Transistor PNP)
- `VCC` $\rightarrow$ Rel Bawah (+) [5V]
- `GND` $\rightarrow$ Rel Bawah (-) [GND]
- `I/O / IN` $\rightarrow$ ESP32 **GPIO 6**

---

## 4. Pemasangan Kapasitor Toren (Elco 16V 1000µF)

```text
                  REL BAWAH BREADBOARD (5V SISTEM)
    Jalur Merah (+)  ==================[+]====================
                                        | (Kaki Panjang)
                                  [ELCO 1000µF / 16V]
                                        | (Kaki Pendek + Garis Minus Strip)
    Jalur Biru  (-)  ==================[-]====================
```

- **Kaki Panjang (+):** Tancapkan ke lubang rel **Merah (+)** 5V.
- **Kaki Pendek (-):** Perhatikan sablonan garis putih/abu-abu bertanda **`(-)`** pada bodi tabung, tancapkan ke lubang rel **Biru (-)** GND.
- **Posisi Terbaik:** Tancapkan sedekat mungkin dengan titik masuk kabel Type-C Breakout dan titik cabang kabel Relay/Kipas.

---

## 5. Checklist Sebelum Adaptor Dicolok ke Listrik

1. [ ] **Periksa Polaritas Elco 1000µF:** Pastikan kaki garis minus tidak tertukar ke jalur merah 5V.
2. [ ] **Periksa Common Ground:** Pastikan kabel jumper hitam terpasang menghubungkan rel biru atas dan rel biru bawah.
3. [ ] **Periksa Jalur Relay:** Pastikan terminal `NO` dan `COM` relay tidak menjepit kabel 5V dan GND sekaligus (itu membuat korsleting saat relay *klik*).
4. [ ] **Uji Kabel Radar:** Pastikan kabel TX masuk ke GPIO 18 dan RX masuk ke GPIO 17 (koneksi silang).
5. [ ] **Uji Sensor 3.3V:** Pastikan tidak ada satupun kabel VCC dari SHT30, ENS160, atau BH1750 yang nyasar ke rel bawah 5V.
