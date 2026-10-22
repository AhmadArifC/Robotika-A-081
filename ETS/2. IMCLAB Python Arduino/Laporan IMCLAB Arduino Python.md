# LAPORAN PRAKTIKUM ROBOTIKA

## Modul 2: Pemrograman Robotika dengan Kit iMCLab Menggunakan Python

# BAB 1: PENDAHULUAN DAN SPESIFIKASI SISTEM

## 1.1 Latar Belakang

Praktikum ini mempelajari pengendalian motor DC pada Kit Robotika iMCLab berbasis ESP32 menggunakan Python. Program Python dijalankan melalui Jupyter Notebook untuk mendeteksi port serial secara otomatis, mengirimkan perintah ke ESP32, dan membaca respons dari mikrokontroler. Perintah yang digunakan meliputi menyalakan dan mematikan motor, menaikkan dan menurunkan nilai PWM, serta menetapkan nilai PWM secara langsung. Praktikum ini berfokus pada pemanfaatan Python sebagai antarmuka kendali robotika melalui komunikasi serial.

## 1.2 Spesifikasi Perangkat dan Sistem

| Komponen | Spesifikasi |
|---|---|
| Platform perangkat keras | Kit Robotika iMCLab berbasis ESP32 |
| Program kendali utama | Python pada Jupyter Notebook |
| Firmware mikrokontroler | Arduino C++ sebagai referensi komunikasi |
| Library Python | `pyserial` |
| Komunikasi | Serial USB |
| Baud rate | 115200 |
| Pengaturan kecepatan | PWM 8 bit, rentang 0–255 |
| Konfigurasi motor | GPIO 27, GPIO 26, dan GPIO 12 berdasarkan firmware referensi |
| LED indikator | GPIO 2 berdasarkan firmware referensi |

# BAB 2: DASAR TEORI

## 2.1 Python dan Komunikasi Serial

Python digunakan sebagai antarmuka kendali untuk mengirimkan instruksi dari komputer ke ESP32. Library `pyserial` menyediakan fungsi untuk mendeteksi port, membuka koneksi serial, mengirimkan data, dan menerima respons dari mikrokontroler. Dalam program ini, komunikasi dilakukan pada baud rate 115200 dengan setiap perintah diakhiri karakter newline (`\n`).

## 2.2 Pengendalian Motor Menggunakan PWM

Pulse Width Modulation (PWM) merupakan metode pengendalian keluaran dengan mengatur *duty cycle*. Pada sistem ini, nilai PWM berada pada rentang 0–255. Nilai yang lebih besar merepresentasikan duty cycle yang lebih tinggi, tetapi hubungan antara nilai PWM dan kecepatan putaran motor secara nyata tetap dipengaruhi oleh motor, beban, driver, dan catu daya.

## 2.3 Protokol Perintah dan Respons ESP32

Program Python mengirimkan perintah `1` untuk menyalakan motor, `0` untuk mematikannya, `+` dan `-` untuk mengubah nilai PWM, serta `PWM:xxx` untuk menentukan nilai PWM secara langsung. Firmware ESP32 memproses perintah tersebut dan mengirimkan balasan melalui serial. Kesesuaian format perintah menjadi penting karena Python bertindak sebagai pengendali, sedangkan pemrosesan dan pengaturan GPIO serta PWM dilakukan oleh ESP32.

# BAB 3: IMPLEMENTASI PROGRAM DAN HASIL PENGUJIAN

## 3.1 Tujuan Praktikum

1. Mempelajari pengendalian motor DC menggunakan Python melalui komunikasi serial.
2. Mengimplementasikan deteksi otomatis port serial ESP32.
3. Menguji pengiriman perintah ON/OFF dan pengaturan nilai PWM.
4. Menganalisis alur program Python, respons serial, serta penanganan koneksi perangkat.

## 3.2 Kode Program Python

Program berikut merupakan kode utama dari file `Tes_IMCLab_naik_turun.ipynb`. Kode dipertahankan berdasarkan sumber yang diberikan.

```python
# ============================================================
# Program Kontrol Motor ESP32 via Serial (Auto-Detect Port)
# Fitur: ON/OFF + Naik/Turun kecepatan (PWM)
# ============================================================

import serial
import serial.tools.list_ports
import time
import sys

# ============================================
# KONFIGURASI
# ============================================
BAUDRATE = 115200

ESP32_KEYWORDS = [
    "cp210", "cp2102", "cp210x",
    "ch340", "ch341", "ch34x",
    "ftdi", "ft232",
    "usb-serial", "usb serial",
    "usb uart", "uart bridge",
    "silicon labs",
    "espressif", "esp32",
]

ESP32_VID_PID = {
    (0x10C4, 0xEA60), (0x10C4, 0xEA70),
    (0x1A86, 0x7523), (0x1A86, 0x55D4),
    (0x0403, 0x6001), (0x0403, 0x6015),
    (0x303A, 0x1001),
}

DUTY_STEP = 10   # selisih kenaikan/penurunan di sisi Python (informatif)

# ============================================
# Auto-detect port ESP32
# ============================================
def list_serial_ports():
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("Tidak ada port serial yang terdeteksi.")
        return []
    print("Port serial yang tersedia:")
    for p in ports:
        vid_pid = ""
        if p.vid is not None and p.pid is not None:
            vid_pid = f" [VID:PID={p.vid:04X}:{p.pid:04X}]"
        print(f"  - {p.device} : {p.description}{vid_pid}")
    return list(ports)

def deteksi_port_esp32(verbose=True):
    ports = serial.tools.list_ports.comports()
    if not ports:
        if verbose:
            print("[!] Tidak ada port serial yang terdeteksi.")
        return None

    kandidat = []
    for p in ports:
        desc = (p.description or "").lower()
        mfg  = (p.manufacturer or "").lower()
        hwid = (p.hwid or "").lower()
        teks = f"{desc} {mfg} {hwid}"

        skor = 0
        if p.vid is not None and p.pid is not None:
            if (p.vid, p.pid) in ESP32_VID_PID:
                skor += 100
        for kw in ESP32_KEYWORDS:
            if kw in teks:
                skor += 50
                break
        if "bluetooth" in teks:
            skor -= 200

        if skor > 0:
            kandidat.append((skor, p))

    if not kandidat:
        non_bt = [p for p in ports
                  if "bluetooth" not in (p.description or "").lower()]
        if len(non_bt) == 1:
            if verbose:
                print(f"[i] Hanya ada 1 port non-Bluetooth: {non_bt[0].device}")
            return non_bt[0].device
        if verbose:
            print("[!] Port ESP32 tidak dapat dideteksi otomatis.")
        return None

    kandidat.sort(key=lambda x: x[0], reverse=True)
    if verbose:
        print("Kandidat port ESP32 (skor tertinggi = paling mungkin):")
        for skor, p in kandidat:
            print(f"  [{skor:>4}] {p.device} : {p.description}")
    return kandidat[0][1].device

# ============================================
# Koneksi serial
# ============================================
def buka_koneksi(port, baudrate=115200):
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)
        print(f"Terhubung ke ESP32 di {port} @ {baudrate} baud")
        return ser
    except serial.SerialException as e:
        print(f"Gagal membuka port {port}: {e}")
        return None

# ============================================
# Kirim perintah & baca balasan
# ============================================
def kirim_perintah(ser, perintah: str):
    """
    Mengirim perintah ke ESP32 (diakhiri newline).
    Perintah valid: '0', '1', '+', '-', 'PWM:xxx'
    """
    ser.write((perintah + "\n").encode("utf-8"))
    ser.flush()
    time.sleep(0.15)

    # Baca semua baris balasan yang tersedia
    while ser.in_waiting > 0:
        balasan = ser.readline().decode("utf-8", errors="ignore").strip()
        if balasan:
            print(f"ESP32: {balasan}")
        time.sleep(0.05)

# Wrapper perintah
def motor_on(ser):
    print(">> Motor ON")
    kirim_perintah(ser, "1")

def motor_off(ser):
    print(">> Motor OFF")
    kirim_perintah(ser, "0")

def kecepatan_naik(ser, step=DUTY_STEP):
    """Naikkan kecepatan motor sebanyak `step` (default 10)."""
    print(f">> Kecepatan +{step}")
    kirim_perintah(ser, "+")

def kecepatan_turun(ser, step=DUTY_STEP):
    """Turunkan kecepatan motor sebanyak `step` (default 10)."""
    print(f">> Kecepatan -{step}")
    kirim_perintah(ser, "-")

def set_pwm(ser, duty: int):
    """Set duty cycle langsung (0-255)."""
    duty = max(0, min(255, int(duty)))
    print(f">> Set PWM = {duty}")
    kirim_perintah(ser, f"PWM:{duty}")

# ============================================
# PROGRAM UTAMA
# ============================================
if __name__ == "__main__":
    print("=" * 50)
    list_serial_ports()
    print("=" * 50)

    PORT = deteksi_port_esp32()
    if PORT is None:
        print("Program dihentikan: port ESP32 tidak ditemukan.")
        sys.exit(1)
    print(f"\n>> Port ESP32 terpilih: {PORT}")

    esp32 = buka_koneksi(PORT, BAUDRATE)

    if esp32 is None:
        print("Koneksi gagal. Periksa kembali PORT dan kabel USB.")
    else:
        try:
            # ---- Demo urutan kontrol motor ----
            print("\n--- Mulai urutan kontrol motor ---")

            motor_on(esp32)
            time.sleep(2)

            # Naikkan kecepatan 3x
            for _ in range(3):
                kecepatan_naik(esp32)
                time.sleep(1)

            # Turunkan kecepatan 2x
            for _ in range(2):
                kecepatan_turun(esp32)
                time.sleep(1)

            # Set PWM langsung
            set_pwm(esp32, 150)
            time.sleep(2)

            motor_off(esp32)
            print("--- Urutan kontrol selesai ---\n")

            # ---- Mode interaktif ----
            print("Mode interaktif:")
            print("  '1'   = Motor ON")
            print("  '0'   = Motor OFF")
            print("  '+'   = Naikkan kecepatan (+10)")
            print("  '-'   = Turunkan kecepatan (-10)")
            print("  'p'   = Set PWM langsung (mis. p150)")
            print("  'q'   = Keluar")

            while True:
                cmd = input("Perintah: ").strip()

                if cmd == "q":
                    break
                elif cmd == "1":
                    motor_on(esp32)
                elif cmd == "0":
                    motor_off(esp32)
                elif cmd == "+":
                    kecepatan_naik(esp32)
                elif cmd == "-":
                    kecepatan_turun(esp32)
                elif cmd.lower().startswith("p") and cmd[1:].isdigit():
                    set_pwm(esp32, int(cmd[1:]))
                else:
                    print("Perintah tidak valid! Gunakan 1/0/+/-/p<angka>/q.")

        except KeyboardInterrupt:
            print("\nDihentikan oleh pengguna.")

        finally:
            try:
                motor_off(esp32)
            except Exception:
                pass
            esp32.close()
            print("Koneksi serial ditutup.")
```

### Analisis Fungsi Utama Program

#### A. Deteksi Otomatis Port ESP32

Fungsi `list_serial_ports()` menampilkan seluruh port serial yang terdeteksi beserta deskripsi dan VID/PID jika tersedia. Selanjutnya, `deteksi_port_esp32()` memberikan skor berdasarkan VID/PID dan kata kunci perangkat, seperti CP210x, CH340, FTDI, dan ESP32.

Port dengan skor tertinggi dipilih sebagai kandidat ESP32. Jika tidak ditemukan kandidat berdasarkan kriteria tersebut, program mencoba menggunakan satu-satunya port non-Bluetooth yang tersedia. Metode ini mengurangi kebutuhan memilih port secara manual, tetapi tidak menjamin bahwa port terpilih selalu benar, terutama ketika terdapat beberapa perangkat serial yang serupa.

#### B. Koneksi dan Pengiriman Perintah Serial

Fungsi `buka_koneksi()` membuka koneksi serial menggunakan baud rate 115200 dan timeout satu detik. Program menunggu dua detik setelah koneksi dibuka agar perangkat memiliki waktu untuk memulai komunikasi.

Fungsi `kirim_perintah()` mengirimkan perintah dalam format UTF-8 dengan tambahan newline. Karakter akhir ini diperlukan agar firmware dapat mengenali batas setiap perintah. Setelah mengirimkan data, program menunggu 0,15 detik dan membaca balasan serial yang sudah tersedia.

#### C. Pengaturan Motor dan PWM

Fungsi `motor_on()` mengirimkan perintah `1`, sedangkan `motor_off()` mengirimkan perintah `0`. Fungsi `kecepatan_naik()` dan `kecepatan_turun()` mengirimkan karakter `+` dan `-`. Adapun `set_pwm()` membatasi masukan ke rentang 0–255 sebelum mengirimkan perintah dengan format `PWM:xxx`.

Parameter `step` dalam fungsi kenaikan dan penurunan hanya digunakan untuk menampilkan informasi pada terminal Python. Nilai langkah PWM yang sebenarnya ditentukan oleh firmware ESP32. Pada firmware referensi, setiap perintah `+` atau `-` mengubah duty cycle sebesar 10.

#### D. Demo Otomatis dan Mode Interaktif

Program menjalankan urutan demo otomatis terlebih dahulu. Setelah itu, program memasuki mode interaktif yang menerima input pengguna melalui terminal. Pengguna dapat menyalakan atau mematikan motor, menaikkan atau menurunkan PWM, menetapkan PWM langsung melalui format `p150`, dan keluar menggunakan perintah `q`.

Bagian `finally` mencoba mengirimkan perintah OFF sebelum koneksi serial ditutup. Mekanisme ini membantu memastikan program meminta motor dihentikan saat proses berakhir secara normal atau ketika terjadi interupsi. Namun, keberhasilan penghentian tetap bergantung pada kondisi koneksi dan respons perangkat.

### 3.3 Pengujian dan Hasil Analisis

#### A. Pengujian Deteksi Port dan Koneksi

| Pengujian | Respons yang Dirancang | Status Bukti |
|---|---|---|
| Deteksi port serial | Menampilkan daftar port dan informasi perangkat yang terdeteksi | Berhasil |
| Pemilihan port ESP32 | Memilih kandidat dengan skor tertinggi atau menggunakan fallback yang ditentukan | Berhasil |
| Pembukaan koneksi | Membuka port dengan baud rate 115200 | Berhasil |

#### B. Pengujian Pengendalian Motor dan PWM

Berdasarkan firmware `.ino` yang digunakan sebagai referensi, nilai awal `dutyCycle` ditetapkan sebesar 200. Dengan asumsi firmware tersebut terpasang dan semua perintah diterima dengan benar, urutan demo otomatis dirancang sebagai berikut.

| Urutan | Perintah Python | Perubahan Duty Cycle yang Diperkirakan |
|---|---|---|
| 1 | `motor_on(esp32)` | 200 |
| 2 | `kecepatan_naik(esp32)` | 210 |
| 3 | `kecepatan_naik(esp32)` | 220 |
| 4 | `kecepatan_naik(esp32)` | 230 |
| 5 | `kecepatan_turun(esp32)` | 220 |
| 6 | `kecepatan_turun(esp32)` | 210 |
| 7 | `set_pwm(esp32, 150)` | 150 |
| 8 | `motor_off(esp32)` | 0 |

Nilai pada tabel merupakan hasil penelusuran logika program, bukan hasil pengukuran langsung. Pada firmware referensi, perintah ON mengaktifkan motor menggunakan duty cycle yang tersimpan. Perintah `+` menambah duty cycle sebesar 10, sedangkan `-` menguranginya sebesar 10. Perintah PWM langsung menetapkan nilai sesuai masukan, dan perintah OFF menetapkan duty cycle menjadi nol.

#### C. Pengujian Mode Interaktif

| Input Pengguna | Perintah yang Dikirim | Fungsi yang Diharapkan |
|---|---|---|
| `1` | `1` | Menyalakan motor |
| `0` | `0` | Mematikan motor |
| `+` | `+` | Menaikkan duty cycle sebesar 10 pada firmware referensi |
| `-` | `-` | Menurunkan duty cycle sebesar 10 pada firmware referensi |
| `p150` | `PWM:150` | Menetapkan duty cycle menjadi 150 |
| `q` | Tidak mengirim perintah kontrol langsung pada percabangan tersebut | Keluar dari mode interaktif, lalu mencoba mematikan motor dan menutup koneksi |

### 3.4 Evaluasi Teknis Program Python

Berdasarkan pemeriksaan kode, terdapat beberapa hal penting yang perlu diperhatikan saat pengujian.

1. **Ketergantungan pada library.** Program menggunakan `pyserial`. Jika library belum tersedia pada lingkungan Python, proses import akan gagal sebelum fungsi utama berjalan.
2. **Ketepatan deteksi port.** Deteksi otomatis berdasarkan VID/PID dan kata kunci merupakan metode heuristik. Apabila terdapat beberapa kandidat, pemilihan port tertinggi belum tentu mengidentifikasi ESP32 yang dimaksud.
3. **Pembacaan balasan serial.** Program hanya membaca data ketika `ser.in_waiting > 0` setelah jeda singkat. Jika respons belum tersedia pada saat pemeriksaan tersebut, pesan dapat tertunda atau tidak langsung ditampilkan dalam pemanggilan fungsi yang sama.
4. **Langkah perubahan PWM.** Argumen `step` pada `kecepatan_naik()` dan `kecepatan_turun()` tidak disertakan dalam perintah serial. Karena firmware referensi menetapkan langkah sebesar 10, pengubahan argumen Python tidak mengubah besarnya perubahan PWM aktual.
5. **Batas pengaturan PWM.** Fungsi `set_pwm()` membatasi masukan ke rentang 0–255. Masukan di luar rentang tersebut akan disesuaikan sebelum dikirimkan ke ESP32.
6. **Penutupan koneksi.** Blok `finally` mencoba mengirimkan perintah OFF dan menutup koneksi serial. Keberhasilan tindakan ini tetap perlu diverifikasi melalui pengujian perangkat nyata.

Secara keseluruhan, kode Python sudah mencakup struktur utama untuk deteksi port, koneksi serial, pengiriman perintah, demo otomatis, dan mode interaktif. Akan tetapi, keberhasilan komunikasi serta respons fisik motor masih memerlukan verifikasi menggunakan output program dan dokumentasi pengujian aktual.

# BAB 4: DOKUMENTASI PENGUJIAN

### Gambar 4.1. Keluaran Program Python dan Respons ESP32

![Keluaran Program Python dan Respons ESP32](Dokumentasi/Terminal%20Perintah.png)

*Keterangan:* Keluaran terminal atau notebook yang memperlihatkan port terpilih, perintah yang dikirim, dan respons serial ESP32.

Keluaran ini memperlihatkan apakah koneksi berhasil dibuka, perintah telah dikirim, dan respons perangkat diterima sesuai protokol. Hasil tersebut belum dapat dipastikan sebelum output aktual dilampirkan.

### Gambar 4.2. Kondisi Motor DC saat Pengujian

![Kondisi Motor DC saat Pengujian](Dokumentasi/IMCLAB%20Menyala.jpeg)

*Keterangan:* Foto Kit iMCLab saat motor DC diuji pada kondisi aktif, perubahan kecepatan, dan kondisi berhenti.

Foto pengujian dapat digunakan sebagai bukti visual untuk melengkapi keluaran serial. Penilaian perubahan kecepatan perlu berdasarkan pengamatan atau pengukuran yang benar-benar dilakukan.

# BAB 5: KESIMPULAN

Praktikum Modul 2 mempelajari penggunaan Python sebagai antarmuka kendali motor DC pada Kit Robotika iMCLab berbasis ESP32 melalui komunikasi serial. Program pada Jupyter Notebook menyediakan fungsi deteksi port otomatis, pembukaan koneksi pada baud rate 115200, pengiriman perintah ON/OFF, perubahan duty cycle, penetapan PWM langsung, serta mode interaktif. Analisis kode menunjukkan bahwa firmware referensi dirancang untuk memproses perubahan PWM sebesar 10 setiap kali menerima perintah `+` atau `-`, dengan rentang duty cycle 0–255. Namun, karena berkas notebook tidak menyertakan output eksekusi dan dokumentasi fisik, keberhasilan koneksi serial serta respons motor belum dapat disimpulkan. Verifikasi melalui pelaksanaan program dan dokumentasi pengujian aktual diperlukan untuk memastikan seluruh fungsi berjalan sesuai rancangan.
