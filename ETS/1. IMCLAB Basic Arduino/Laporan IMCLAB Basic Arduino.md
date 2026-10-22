# LAPORAN PRAKTIKUM ROBOTIKA
## Pengendalian LED dan Motor DC Menggunakan ESP32

# BAB 1: PENDAHULUAN & SPESIFIKASI SISTEM

## 1.1 Latar Belakang

Praktikum ini mempelajari pengendalian LED dan motor DC menggunakan DOIT ESP32 DEVKIT V1 melalui Serial Monitor. Karakter `1` digunakan untuk menyalakan LED dan motor, sedangkan karakter `0` digunakan untuk mematikannya. Kecepatan motor diatur menggunakan PWM.

## 1.2 Spesifikasi Sistem

| Komponen | Konfigurasi |
|---|---|
| Mikrokontroler | DOIT ESP32 DEVKIT V1 |
| LED | GPIO 2 |
| Kontrol motor | GPIO 27 dan GPIO 26 |
| PWM motor | GPIO 12 |
| Pemrograman | Arduino IDE |
| Baud rate | 115200 |
| Frekuensi PWM | 30.000 Hz |
| Resolusi PWM | 8 bit |

# BAB 2: DASAR TEORI

## 2.1 Mikrokontroler ESP32

ESP32 merupakan mikrokontroler yang digunakan untuk menjalankan program dan mengendalikan komponen berdasarkan konfigurasi pin yang ditentukan.

## 2.2 General Purpose Input/Output (GPIO)

GPIO digunakan sebagai pin masukan atau keluaran digital. Pada praktikum ini, GPIO digunakan untuk mengendalikan LED dan menentukan arah putaran motor DC.

## 2.3 Pulse Width Modulation (PWM)

PWM merupakan metode pengaturan keluaran melalui perubahan *duty cycle*. Pada praktikum ini, PWM digunakan untuk mengatur daya yang diberikan kepada driver motor DC.

## 2.4 Komunikasi Serial

Komunikasi serial digunakan untuk mengirimkan perintah dari Serial Monitor ke ESP32. Program menerima karakter `1` untuk mengaktifkan LED dan motor, serta `0` untuk mematikannya.

# BAB 3: IMPLEMENTASI PROGRAM DAN HASIL PENGUJIAN

## 3.1 Tujuan Praktikum

1. Mengendalikan LED melalui komunikasi serial.
2. Mengoperasikan motor DC menggunakan ESP32.
3. Mengatur keluaran PWM untuk mengendalikan motor.

## 3.2 Kode Program

```cpp
int ledPin = 2;
char myData = 0;

int motor1Pin1 = 27;
int motor1Pin2 = 26;
int enable1Pin = 12;

// Setting PWM properties untuk ESP32 Core 2.x
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 250;

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);

  digitalWrite(ledPin, LOW);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(pwmChannel, 0);

  Serial.println("Testing DC Motor...");
  Serial.println("Kirim 1 untuk menyalakan motor.");
  Serial.println("Kirim 0 untuk mematikan motor.");
}

void MotorOn() {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);

  ledcWrite(pwmChannel, dutyCycle);
}

void MotorOff() {
  ledcWrite(pwmChannel, 0);

  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    myData = Serial.read();

    if (myData == '1') {
      Serial.println("LED is ON!");
      digitalWrite(ledPin, HIGH);
      MotorOn();
    }
    else if (myData == '0') {
      Serial.println("LED is OFF!");
      digitalWrite(ledPin, LOW);
      MotorOff();
    }
  }
}
```

## 3.3 Hasil Pengujian dan Analisis

| Perintah | Hasil Pengujian | Status |
|---|---|---|
| `1` | LED menyala dan motor DC bergerak | Berhasil |
| `0` | LED mati dan motor DC berhenti | Berhasil |

Berdasarkan pengujian, ESP32 berhasil menerima perintah melalui Serial Monitor dan mengendalikan LED serta motor DC sesuai input. Perintah `1` mengaktifkan LED dan motor, sedangkan `0` mematikan LED dan menghentikan motor. Pengaturan PWM digunakan untuk mengendalikan keluaran motor dengan nilai `dutyCycle` sebesar 250 dari rentang 0–255.

Hasil ini menunjukkan bahwa komunikasi serial, kontrol GPIO, dan pengendalian motor melalui PWM berfungsi sesuai rancangan program.

# BAB 4: DOKUMENTASI PENGUJIAN

### Gambar 4.1. Screenshot Serial Monitor

![Screenshot Serial Monitor](Dokumentasi/Serial%20Monitor%20saat%20mengirim%20perintah.png)

Keterangan: Tampilan Serial Monitor saat menerima perintah untuk menyalakan dan mematikan LED serta motor DC.

### Gambar 4.2. Kondisi LED dan Motor Mati

![LED dan Motor Mati](Dokumentasi/LED%20dan%20Motor%20Mati.jpeg)

Keterangan: Kondisi LED dan motor DC saat sistem dimatikan.

### Gambar 4.3. Kondisi LED dan Motor Menyala

![LED dan Motor Nyala](Dokumentasi/LED%20dan%20Motor%20Nyala.jpeg)

Keterangan: Kondisi LED dan motor DC saat sistem diaktifkan.

# BAB 5: KESIMPULAN

Praktikum berhasil mengimplementasikan pengendalian LED dan motor DC menggunakan DOIT ESP32 DEVKIT V1 melalui komunikasi serial. Perintah `1` berhasil menyalakan LED dan menjalankan motor, sedangkan perintah `0` berhasil mematikan LED dan menghentikan motor. Pengujian ini menunjukkan bahwa kontrol GPIO dan PWM dapat digunakan untuk mengendalikan aktuator sesuai perintah yang diberikan.
