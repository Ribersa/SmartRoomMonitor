// ==============================================================================
// 07_uji_radar_ld2410.ino (VERSI MAKSIMAL - FULL FITUR MODUL M3)
// Proyek: SmartRoomMonitor (RPL) - PIC Modul M3: Deswiryawan Saragih (241111008)
// Deskripsi: Pengujian komprehensif Radar mmWave 24 GHz Hilink HLK-LD2410
//            mencakup seluruh kebutuhan Modul M3 (FR-05 & FR-06):
//            1. Auto-Swap Pin UART (otomatis mendeteksi jika TX/RX terbalik).
//            2. Pembatasan jarak deteksi (Default 0.5m - 0.75m) & kontrol live.
//            3. Visualisasi bar energi gerak & energi napas mikro (diam).
//            4. Simulasi Peringatan Jam Malam (22:00-06:00) + Buzzer (FR-05).
//            5. Simulasi Tombol MUTE Darurat via Serial (FR-06).
// Hardware: ESP32-S3 DevKitC-1 N16R8 + HLK-LD2410 + Buzzer Pasif (GPIO 6)
// ==============================================================================

#include <ld2410.h>

#define RADAR_RX_PIN 18 // Default: terima dari TX Radar
#define RADAR_TX_PIN 17 // Default: kirim ke RX Radar
#define RADAR_BAUD   256000
#define BUZZER_PIN   6  // Buzzer Active-LOW (HIGH = Mati, LOW = Nyala)

ld2410 radar;
unsigned long lastPrintTime = 0;

// State Simulasi Fitur M3 (FR-05 & FR-06)
bool modeJamMalam = false; // Simulasi jam 22:00 - 06:00 (FR-05)
bool alarmMuted   = false; // Status MUTE darurat oleh Staff (FR-06)
int activeRxPin   = RADAR_RX_PIN;
int activeTxPin   = RADAR_TX_PIN;

// Pemutus arus Buzzer (Modul Active-LOW PNP: HIGH = Cut-Off / Bisu Total)
void matikanBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
}

// Pembangkit frekuensi suara untuk Buzzer Pasif
void mainkanNada(int frekuensi, int durasiMs) {
  long periodeUs = 1000000L / frekuensi;
  long setengahPeriodeUs = periodeUs / 2;
  long totalSiklus = (long)frekuensi * durasiMs / 1000L;

  for (long i = 0; i < totalSiklus; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(setengahPeriodeUs);
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(setengahPeriodeUs);
  }
  matikanBuzzer();
}

// Bunyi peringatan singkat saat terdeteksi orang di Jam Malam (FR-05)
void bunyiAlarmJamMalam() {
  mainkanNada(2800, 90);
  delay(40);
  mainkanNada(3400, 90);
  matikanBuzzer();
}

// Gambar grafik bar energi [████░░░░░░] (0 - 100)
void cetakBarEnergi(uint8_t energi, char* bufferOut) {
  int terisi = energi / 10;
  if (terisi > 10) terisi = 10;
  strcpy(bufferOut, "[");
  for (int i = 0; i < 10; i++) {
    strcat(bufferOut, (i < terisi) ? "#" : ".");
  }
  strcat(bufferOut, "]");
}

// Tampilkan menu kontrol interaktif di Serial Monitor
void tampilkanMenuBantuan() {
  Serial.println("\n==================== PANEL KONTROL LIVE RADAR M3 ====================");
  Serial.println("  Ketik karakter berikut di kotak pesan Serial Monitor lalu Enter:");
  Serial.println("  [0] Limit Jarak Super Dekat (~0.5m / Gate 0, Timeout 1 dtk)");
  Serial.println("  [1] Limit Jarak Meja Kerja  (~0.75m / Gate 1, Timeout 2 dtk) [DEFAULT]");
  Serial.println("  [2] Limit Jarak Area Kasur  (~2.25m / Gate 3, Timeout 5 dtk)");
  Serial.println("  [3] Limit Jarak Kamar Penuh (~4.50m / Gate 6, Timeout 5 dtk)");
  Serial.println("  [4] Limit Jarak Maksimal    (~6.00m / Gate 8, Timeout 5 dtk)");
  Serial.println("  [m] Toggle Simulasi JAM MALAM 22:00-06:00 (FR-05 Alarm Intrusi)");
  Serial.println("  [x] Tekan MUTE DARURAT (FR-06 Bungkam Buzzer)");
  Serial.println("  [i] Baca Info Firmware & Konfigurasi Gate Radar Saat Ini");
  Serial.println("  [r] Factory Reset Radar ke Setelan Pabrik");
  Serial.println("  [h] Tampilkan Menu Bantuan Ini");
  Serial.println("=====================================================================\n");
}

// Fungsi mengatur batas jarak gate radar (1 Gate = ~0.75 meter)
void aturBatasJarakGate(uint8_t gateMaks, uint16_t timeoutDetik, const char* labelMode) {
  Serial.printf("\n⚙️ [KONFIGURASI] Mengatur jarak ke %s (Gate %d, Timeout %d dtk)...\n",
                labelMode, gateMaks, timeoutDetik);
  if (radar.setMaxValues(gateMaks, gateMaks, timeoutDetik)) {
    Serial.println("✅ [SUKSES] Batas jarak berhasil diterapkan ke chip radar!\n");
    mainkanNada(2400, 60);
  } else {
    Serial.println("⚠️ [GAGAL] Radar belum merespons perintah konfigurasi. Pastikan kabel RX radar terhubung.\n");
  }
}

// Baca & tampilkan informasi internal chip radar
void bacaInfoRadar() {
  Serial.println("\n🔍 [INFO RADAR] Membaca parameter internal HLK-LD2410...");
  if (radar.requestFirmwareVersion()) {
    Serial.printf("   - Versi Firmware : v%d.%02d.%08X\n",
                  radar.firmware_major_version,
                  radar.firmware_minor_version,
                  radar.firmware_bugfix_version);
  }
  if (radar.requestCurrentConfiguration()) {
    Serial.printf("   - Max Gate Gerak : Gate %d (~%.2f meter)\n",
                  radar.max_moving_gate, (radar.max_moving_gate + 1) * 0.75);
    Serial.printf("   - Max Gate Diam  : Gate %d (~%.2f meter)\n",
                  radar.max_stationary_gate, (radar.max_stationary_gate + 1) * 0.75);
    Serial.printf("   - Waktu Timeout  : %d detik\n", radar.sensor_idle_time);
    Serial.println("   - Ambang Sensitivitas per Gate (0 - 8):");
    for (int g = 0; g <= 8; g++) {
      Serial.printf("     Gate %d (~%.2fm) -> Gerak: %d | Diam/Napas: %d\n",
                    g, (g + 1) * 0.75,
                    radar.motion_sensitivity[g],
                    radar.stationary_sensitivity[g]);
    }
  }
  Serial.println();
}

// Auto-Discovery koneksi UART (mencoba pin normal lalu otomatis balik jika tertukar)
bool hubungkanRadarOtomatis() {
  // Percobaan 1: Konfigurasi Standar (RX=18, TX=17)
  Serial.print("🔄 [UART] Mencoba koneksi standar (RX=GPIO 18, TX=GPIO 17)... ");
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, 18, 17);
  radar.begin(Serial1, false);

  unsigned long tStart = millis();
  while (millis() - tStart < 2000) {
    radar.read();
    if (radar.isConnected()) {
      activeRxPin = 18;
      activeTxPin = 17;
      Serial.println("TERHUBUNG!");
      return true;
    }
    delay(10);
  }
  Serial.println("Belum ada respons.");

  // Percobaan 2: Auto-Swap (RX=17, TX=18) jika kabel TX/RX tertukar
  Serial.print("🔄 [AUTO-SWAP] Mencoba membalik jalur pin (RX=GPIO 17, TX=GPIO 18)... ");
  Serial1.end();
  delay(100);
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, 17, 18);
  radar.begin(Serial1, false);

  tStart = millis();
  while (millis() - tStart < 2000) {
    radar.read();
    if (radar.isConnected()) {
      activeRxPin = 17;
      activeTxPin = 18;
      Serial.println("TERHUBUNG! (Pin otomatis disesuaikan)");
      return true;
    }
    delay(10);
  }
  Serial.println("Gagal.");

  // Kembalikan ke default untuk percobaan berkala di loop
  Serial1.end();
  Serial1.begin(RADAR_BAUD, SERIAL_8N1, 18, 17);
  radar.begin(Serial1, false);
  return false;
}

void prosesInputSerial() {
  if (!Serial.available()) return;

  char cmd = Serial.read();
  // Bersihkan sisa newline
  while (Serial.available()) {
    char c = Serial.peek();
    if (c == '\r' || c == '\n') Serial.read();
    else break;
  }

  switch (cmd) {
    case '0':
      aturBatasJarakGate(0, 1, "Super Dekat (~0.5m)");
      break;
    case '1':
      aturBatasJarakGate(1, 2, "Meja Kerja (~0.75m)");
      break;
    case '2':
      aturBatasJarakGate(3, 5, "Area Kasur (~2.25m)");
      break;
    case '3':
      aturBatasJarakGate(6, 5, "Kamar Penuh (~4.50m)");
      break;
    case '4':
      aturBatasJarakGate(8, 5, "Maksimal (~6.00m)");
      break;
    case 'm':
    case 'M':
      modeJamMalam = !modeJamMalam;
      alarmMuted = false; // Reset mute saat ganti mode
      Serial.printf("\n🌙 [MODE JAM MALAM FR-05] Status: %s\n\n",
                    modeJamMalam ? "AKTIF (22:00 - 06:00 -> Alarm Menyala Jika Ada Orang!)" : "NONAKTIF (Siang Hari Normal)");
      mainkanNada(modeJamMalam ? 3000 : 1800, 80);
      break;
    case 'x':
    case 'X':
      alarmMuted = true;
      matikanBuzzer();
      Serial.println("\n🔕 [MUTE DARURAT FR-06] Alarm dibungkam oleh Staff! Buzzer dihentikan.\n");
      break;
    case 'i':
    case 'I':
      bacaInfoRadar();
      break;
    case 'r':
    case 'R':
      Serial.println("\n♻️ [RESET] Mengembalikan radar ke pengaturan pabrik...");
      if (radar.requestFactoryReset()) {
        radar.requestRestart();
        Serial.println("✅ [SUKSES] Factory reset berhasil!\n");
      } else {
        Serial.println("⚠️ [GAGAL] Factory reset tidak direspons.\n");
      }
      break;
    case 'h':
    case 'H':
      tampilkanMenuBantuan();
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  matikanBuzzer();

  delay(2000);

  Serial.println("\n=====================================================================");
  Serial.println("  [SmartRoomMonitor] Pengujian Maksimal Radar HLK-LD2410 (Modul M3)");
  Serial.println("  Fitur: Auto-Pin Swap | Limit Jarak 0.5m | Simulasi FR-05 & FR-06");
  Serial.println("=====================================================================");

  if (hubungkanRadarOtomatis()) {
    // Nada sambut sukses
    mainkanNada(2000, 60);
    delay(50);
    mainkanNada(2800, 90);

    // Otomatis limit jarak ke ~0.5m - 0.75m (Gate 1, Timeout 2 detik) sesuai permintaan
    aturBatasJarakGate(1, 2, "Meja Kerja (~0.5m - 0.75m)");
    bacaInfoRadar();
  } else {
    Serial.println("⚠️ [PERINGATAN] Radar belum terdeteksi saat boot. Pastikan daya 5V & GND menyala.");
  }

  tampilkanMenuBantuan();
}

void loop() {
  // 1. Baca stream paket radar secara kontinu (wajib non-blocking)
  radar.read();

  // 2. Cek apakah ada perintah ketikan dari pengguna di Serial Monitor
  prosesInputSerial();

  // 3. Tampilkan telemetri setiap 500 ms
  if (millis() - lastPrintTime >= 500) {
    lastPrintTime = millis();

    if (radar.isConnected()) {
      bool adaOrang = radar.presenceDetected();
      bool bergerak = radar.movingTargetDetected();
      bool diam     = radar.stationaryTargetDetected();

      uint16_t jarakGerak  = radar.movingTargetDistance();
      uint8_t  energiGerak = radar.movingTargetEnergy();
      uint16_t jarakDiam   = radar.stationaryTargetDistance();
      uint8_t  energiDiam  = radar.stationaryTargetEnergy();

      char barGerak[16], barDiam[16];
      cetakBarEnergi(energiGerak, barGerak);
      cetakBarEnergi(energiDiam, barDiam);

      // Tentukan Label Status FR-05 (Tidak Ada / Bergerak / Diam)
      const char* statusFR05 = "TIDAK ADA";
      if (adaOrang) {
        if (bergerak) statusFR05 = "BERGERAK";
        else if (diam) statusFR05 = "DIAM";
      } else {
        // Jika ruangan kembali kosong, reset status mute untuk kejadian berikutnya
        alarmMuted = false;
      }

      // Cetak Baris Telemetri Lengkap
      if (!adaOrang) {
        Serial.printf("🔲 [STATUS: %-9s] Ruangan Kosong | Mode: %s\n",
                      statusFR05, modeJamMalam ? "🌙 MALAM" : "☀️ SIANG");
        matikanBuzzer();
      } else {
        Serial.printf("🎯 [STATUS: %-9s] Gerak: %3dcm %s %3d%% | Diam/Napas: %3dcm %s %3d%% | %s\n",
                      statusFR05,
                      jarakGerak, barGerak, energiGerak,
                      jarakDiam, barDiam, energiDiam,
                      modeJamMalam ? (alarmMuted ? "🔕 MUTE" : "🚨 ALARM!") : "☀️ SIANG");

        // Logika FR-05 & FR-06: Bunyi peringatan jika Jam Malam & belum di-Mute
        if (modeJamMalam && !alarmMuted) {
          Serial.println("   ⚠️ [PERINGATAN JAM MALAM FR-05] Terdeteksi aktivitas! (Ketik 'x' untuk MUTE FR-06)");
          bunyiAlarmJamMalam();
        } else {
          matikanBuzzer();
        }
      }
    } else {
      Serial.println("⏳ [MENUNGGU RADAR] Belum ada frame UART masuk (Cek suplai 5V & Common GND)...");
    }
  }
}
