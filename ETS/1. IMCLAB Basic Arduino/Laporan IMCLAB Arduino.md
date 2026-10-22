# BAB 1: PENDAHULUAN & SPESIFIKASI SISTEM

## 1.1 Latar Belakang

Praktikum ini bertujuan mempelajari pengendalian LED dan motor DC menggunakan mikrokontroler **DOIT ESP32 DEVKIT V1**. Sistem dikendalikan melalui komunikasi serial pada Arduino IDE. Karakter `1` digunakan untuk menyalakan LED dan motor, sedangkan karakter `0` digunakan untuk mematikannya. Kecepatan motor diatur menggunakan *Pulse Width Modulation* (PWM), sementara arah putaran ditentukan melalui pin kontrol driver motor.

## 1.2 Spesifikasi Perangkat Keras dan Perangkat Lunak

### A. Perangkat Keras

| Komponen | Spesifikasi/Fungsi |
|---|---|
| Mikrokontroler | DOIT ESP32 DEVKIT V1 |
| LED | Dikendalikan melalui GPIO 2 |
| Driver motor | Mengendalikan motor DC berdasarkan sinyal kontrol dan PWM |
| Motor DC | Aktuator yang dikendalikan oleh ESP32 |
| Pin kontrol motor | GPIO 27 dan GPIO 26 |
| Pin PWM motor | GPIO 12 |

### B. Perangkat Lunak

| Komponen | Spesifikasi/Fungsi |
|---|---|
| Arduino IDE | Lingkungan pemrograman ESP32 |
| ESP32 Arduino Core | Menggunakan API versi 2.x |
| Serial Monitor | Komunikasi serial dengan baud rate 115200 |
| PWM | Frekuensi 30.000 Hz, resolusi 8 bit, kanal 0 |

Versi Arduino IDE dan perangkat lunak pendukung lainnya belum didokumentasikan.

### C. Konfigurasi Pin

| Pin ESP32 | Fungsi |
|---|---|
| GPIO 2 | Kontrol LED |
| GPIO 27 | Kontrol arah motor |
| GPIO 26 | Kontrol arah motor |
| GPIO 12 | Sinyal PWM untuk driver motor |

## 1.3 Ringkasan Sistem

Program menerima perintah berupa karakter melalui Serial Monitor. Ketika karakter `1` diterima, ESP32 menyalakan LED dan mengaktifkan motor DC dengan konfigurasi arah maju. Ketika karakter `0` diterima, LED dimatikan dan motor dihentikan.

Pengaturan kecepatan motor menggunakan PWM dengan frekuensi 30.000 Hz dan resolusi 8 bit. Nilai `dutyCycle` ditetapkan sebesar 250, setara dengan sekitar 98,04% dari rentang PWM 0–255.
