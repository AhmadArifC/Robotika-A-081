# Laporan Praktikum Robotika — Modul 2

## Pemrograman Robotika dengan Kit iMCLab Menggunakan Python

Praktikum ini membahas pengendalian motor DC dan LED indikator pada Kit Robotika iMCLab berbasis ESP32. Firmware Arduino berjalan pada ESP32, sedangkan program Python mengirimkan perintah melalui komunikasi serial, membaca balasan, dan menyediakan mode demo serta mode interaktif.

> **Catatan integritas pengujian:** Berkas notebook yang diberikan belum memiliki output eksekusi dan belum disertai screenshot atau foto hasil pengujian. Oleh karena itu, laporan ini mendeskripsikan perilaku yang dirancang berdasarkan kode; keberhasilan perangkat secara langsung belum dapat dikonfirmasi dari berkas yang tersedia.

## Daftar Isi

- [BAB 1 — Pendahuluan dan Spesifikasi Sistem](#bab-1--pendahuluan-dan-spesifikasi-sistem)
- [BAB 2 — Catatan Teknis dan Kompatibilitas](#bab-2--catatan-teknis-dan-kompatibilitas)
- [BAB 3 — Implementasi Program dan Hasil Pengujian](#bab-3--implementasi-program-dan-hasil-pengujian)
- [BAB 4 — Dokumentasi Pengujian](#bab-4--dokumentasi-pengujian)
- [BAB 5 — Kesimpulan](#bab-5--kesimpulan)
- [Struktur Repository](#struktur-repository)

---

## BAB 1 — Pendahuluan dan Spesifikasi Sistem

### 1.1 Latar Belakang

Praktikum ini mempelajari pengendalian motor DC dan LED melalui Kit Robotika iMCLab berbasis ESP32 dengan membagi fungsi kendali antara mikrokontroler dan komputer. Program Arduino pada ESP32 menerima perintah serial untuk menyalakan atau mematikan motor, menaikkan atau menurunkan nilai PWM, dan menetapkan nilai PWM secara langsung. Program Python pada Jupyter Notebook mendeteksi port serial, membuka komunikasi dengan ESP32, mengirimkan perintah, serta membaca balasan dari mikrokontroler.

### 1.2 Spesifikasi Perangkat Keras dan Perangkat Lunak

| Komponen | Konfigurasi / Keterangan |
|---|---|
| Mikrokontroler | ESP32 pada Kit Robotika iMCLab; model board spesifik belum disebutkan |
| LED indikator | GPIO 2 |
| Pin arah motor | GPIO 27 dan GPIO 26 |
| Pin PWM / enable motor | GPIO 12 |
| Frekuensi PWM | 30.000 Hz |
| Resolusi PWM | 8 bit, nilai duty cycle 0–255 |
| Duty cycle awal firmware | 200 |
| Langkah perubahan duty cycle | 10 |
| Komunikasi serial | 115200 baud |
| Firmware | Arduino C++ — [`src/imclab_naik_turun.ino`](./src/imclab_naik_turun.ino) |
| Program kendali komputer | Python pada Jupyter Notebook — [`notebooks/Tes_IMCLab_naik_turun.ipynb`](./notebooks/Tes_IMCLab_naik_turun.ipynb) |
| Dependensi Python | `pyserial` |
| Port serial | Dideteksi otomatis oleh program; nomor port aktual belum tercatat |
| Versi Python dan Arduino-ESP32 | Belum terverifikasi dari bahan yang diberikan |

---

## BAB 2 — Catatan Teknis dan Kompatibilitas

Bahan yang diberikan tidak menyertakan catatan kendala aktual, pesan galat, atau hasil troubleshooting. Bagian ini mencatat aspek teknis yang tampak pada implementasi tanpa menyatakan bahwa masalah tertentu benar-benar terjadi saat praktikum.

### 2.1 Kompatibilitas API PWM Arduino-ESP32

Firmware menyediakan fungsi `pwmSetup()` dan `pwmWrite()` untuk membedakan API PWM pada Arduino-ESP32 Core 2.x dan Core 3.x. Core 2.x menggunakan `ledcSetup()`, `ledcAttachPin()`, serta `ledcWrite()` berbasis kanal. Core 3.x menggunakan `ledcAttach()` dan `ledcWrite()` berbasis pin. Mekanisme tersebut merupakan penyesuaian kompatibilitas yang tertulis di kode, bukan bukti bahwa kendala versi tertentu telah terjadi.

### 2.2 Deteksi Port dan Komunikasi Serial

Program Python memeriksa deskripsi perangkat serta pasangan VID/PID yang sudah dicantumkan dalam kode untuk memperkirakan port ESP32. Koneksi serial dibuka pada baud rate `115200` dengan timeout satu detik. Jika port tidak terdeteksi, program menampilkan pesan dan keluar; jika koneksi gagal, program meminta pemeriksaan port dan kabel USB.

Sebelum menjalankan notebook, tutup Serial Monitor atau aplikasi lain yang sedang memakai port yang sama. Notebook membutuhkan paket `pyserial`; pemasangan dapat dilakukan dari terminal dengan:

```bash
python -m pip install pyserial
```

Langkah ini merupakan prasyarat penggunaan program Python, bukan catatan bahwa instalasi tersebut sebelumnya gagal.

---

## BAB 3 — Implementasi Program dan Hasil Pengujian

### 3.1 Tujuan Modul

1. Mengendalikan motor DC dan LED indikator pada Kit iMCLab melalui ESP32.
2. Mengirim perintah kontrol dari Python ke ESP32 menggunakan komunikasi serial.
3. Mengatur nilai PWM melalui perintah kenaikan, penurunan, dan penetapan nilai langsung.
4. Menerapkan deteksi otomatis port serial dan membaca balasan dari ESP32.

### 3.2 Kode Program ESP32 (Arduino C++)

Kode lengkap firmware tersedia pada [`src/imclab_naik_turun.ino`](./src/imclab_naik_turun.ino). Cuplikan berikut menunjukkan konfigurasi pin dan parameter PWM:

```cpp
int ledPin = 2;
int motor1Pin1 = 27;
int motor1Pin2 = 26;
int enable1Pin = 12;

const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;
const int DUTY_STEP = 10;
const int DUTY_MIN  = 0;
const int DUTY_MAX  = 255;
```

Firmware menerima perintah berikut pada input serial. Perintah teks diproses setelah karakter newline atau carriage return diterima.

| Perintah serial ke ESP32 | Fungsi yang dirancang |
|---|---|
| `1` | Mengaktifkan motor dengan duty cycle terakhir; jika nilainya 0, firmware menggunakan nilai awal 200 |
| `0` | Mengatur duty cycle menjadi 0 dan mematikan LED indikator |
| `+` | Menaikkan duty cycle sebesar 10, maksimum 255 |
| `-` | Menurunkan duty cycle sebesar 10, minimum 0 |
| `PWM:xxx` | Mengatur duty cycle ke nilai yang diberikan, dibatasi ke rentang 0–255 |

LED indikator di GPIO 2 menyala ketika nilai `dutyCycle` lebih besar dari 0. Arah motor ditetapkan melalui GPIO 27 dan GPIO 26. Keluaran PWM diberikan melalui GPIO 12.

### 3.3 Kode Program Python

Notebook lengkap tersedia pada [`notebooks/Tes_IMCLab_naik_turun.ipynb`](./notebooks/Tes_IMCLab_naik_turun.ipynb). Program Python menggunakan `pyserial` untuk mendeteksi port, membuka koneksi, mengirim perintah berakhiran newline, serta menampilkan baris balasan yang diterima dari ESP32.

Urutan demo otomatis yang tertulis di notebook adalah:

1. Mengirim perintah motor ON (`1`).
2. Menaikkan kecepatan tiga kali dengan jeda satu detik ant perintah.
3. Menurunkan kecepatan dua kali dengan jeda satu detik ant perintah.
4. Menetapkan PWM ke 150 dan menunggu dua detik.
5. Mengirim perintah motor OFF (`0`).
6. Masuk ke mode interaktif, sampai pengguna mengetik `q` atau menghentikan program.

Dalam mode interaktif, pengguna dapat mengetik `1`, `0`, `+`, `-`, `p150` (atau `p` diikuti angka lain), dan `q`. Perintah interaktif `p150` dikonversi oleh program Python menjadi perintah serial `PWM:150` untuk firmware. Nilai `step` dalam fungsi pembungkus Python digunakan pada teks informasi; besaran perubahan sesungguhnya ditetapkan oleh firmware melalui `DUTY_STEP = 10`.

### 3.4 Hasil Pengujian dan Analisis

Notebook yang diberikan belum menyertakan output eksekusi, sedangkan foto kondisi motor/LED dan log Serial Monitor juga belum dilampirkan. Tabel berikut membedakan skenario yang dirancang dalam kode dari hasil pengamatan aktual.

| Pengujian | Respons yang dirancang oleh kode | Status berdasarkan berkas |
|---|---|---|
| Deteksi dan koneksi serial | Python mencari port, memilih kandidat dengan skor tertinggi, lalu membuka koneksi pada 115200 baud | Belum terverifikasi; tidak ada output eksekusi |
| Perintah `1` | ESP32 memanggil `MotorOn()`, menerapkan PWM dan menyalakan LED jika duty cycle > 0 | Belum terverifikasi melalui pengamatan perangkat |
| Perintah `0` | ESP32 mengatur duty cycle menjadi 0; keluaran PWM menjadi 0 dan LED mati | Belum terverifikasi melalui pengamatan perangkat |
| Perintah `+` | Duty cycle bertambah 10 hingga maksimum 255 | Belum terverifikasi melalui pengamatan perangkat |
| Perintah `-` | Duty cycle berkurang 10 hingga minimum 0 | Belum terverifikasi melalui pengamatan perangkat |
| Perintah `PWM:150` | Firmware menetapkan duty cycle ke 150 dan menerapkan nilai itu ke PWM | Belum terverifikasi melalui pengamatan perangkat |
| Demo otomatis | Kode mengirim ON, tiga perintah naik, dua perintah turun, PWM 150, lalu OFF | Urutan ada di kode; hasil fisik belum dilampirkan |
| Penutupan program | Blok `finally` mencoba mematikan motor, menutup koneksi, dan melaporkan port serial ditutup | Belum terverifikasi melalui eksekusi |

Secara logika, Python bertindak sebagai pengirim perintah dan pembaca balasan, sedangkan ESP32 menjalankan kontrol GPIO dan PWM. Resolusi 8 bit berarti rentang nilai duty cycle yang digunakan adalah 0–255. Namun, tanpa log keluaran dan pengamatan perangkat, keberhasilan operasional motor, LED, dan komunikasi serial belum dapat disimpulkan.

---

## BAB 4 — Dokumentasi Pengujian

Dokumentasi aktual belum disertakan. Tambahkan gambar ke folder [`Dokumentasi/`](./Dokumentasi/) lalu ganti placeholder pada bagian ini dengan path file yang sesuai. GitHub mendukung penyisipan gambar menggunakan sintaks `![teks alternatif](path-relatif)`.

### Gambar 4.1. Keluaran Program Python pada Jupyter Notebook

**Placeholder dokumentasi:** Tambahkan screenshot keluaran notebook ke `Dokumentasi/python-output.png` setelah gambar aktual tersedia.

*Keterangan:* Gambar yang perlu ditambahkan adalah screenshot keluaran notebook saat deteksi port dan pengiriman perintah.

*Analisis:* Screenshot belum tersedia. Setelah ditambahkan, gunakan dokumentasi ini untuk mencocokkan port yang dipilih dan urutan perintah yang ditampilkan Python.

### Gambar 4.2. Keluaran Serial Monitor ESP32

**Placeholder dokumentasi:** Tambahkan screenshot Serial Monitor ke `Dokumentasi/serial-monitor.png` setelah gambar aktual tersedia.

*Keterangan:* Tambahkan screenshot Serial Monitor yang memperlihatkan pesan awal firmware dan respons terhadap perintah kontrol.

*Analisis:* Dokumentasi ini dapat digunakan untuk mencocokkan perintah Python dengan balasan ESP32. Sampai gambar aktual dilampirkan, respons tersebut belum dapat diverifikasi.

### Gambar 4.3. Kondisi Motor DC dan LED Indikator

**Placeholder dokumentasi:** Tambahkan foto kondisi motor dan LED ke `Dokumentasi/kondisi-motor-led.jpg` setelah foto aktual tersedia.

*Keterangan:* Tambahkan foto Kit iMCLab pada kondisi motor dan LED aktif serta saat motor dihentikan.

*Analisis:* Foto aktual dibutuhkan untuk mendukung kesimpulan tentang respons perangkat keras. Gambar placeholder di atas belum merupakan dokumentasi hasil praktikum.

---

## BAB 5 — Kesimpulan

Berdasarkan kode yang diberikan, Modul 2 dirancang untuk mengendalikan motor DC dan LED indikator Kit iMCLab melalui komunikasi serial antara Python dan ESP32. Firmware menyediakan perintah ON/OFF, perubahan duty cycle sebesar 10, dan penetapan PWM langsung pada rentang 0–255. Program Python mendeteksi port serial, mengirim perintah pada baud rate 115200, serta membaca balasan dari ESP32. Karena notebook belum memiliki output eksekusi dan dokumentasi pengamatan perangkat belum tersedia, keberhasilan operasional motor, LED, dan komunikasi serial masih perlu dikonfirmasi melalui pengujian langsung.

---

## Struktur Repository

```text
Modul_2_iMCLab_Python_GitHub/
├── README.md
├── src/
│   └── imclab_naik_turun.ino
├── notebooks/
│   └── Tes_IMCLab_naik_turun.ipynb
└── Dokumentasi/
    └── README.md
```

### Berkas sumber

- [Firmware ESP32 (.ino)](./src/imclab_naik_turun.ino)
- [Notebook Python (.ipynb)](./notebooks/Tes_IMCLab_naik_turun.ipynb)

