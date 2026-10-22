/**************************************************
 * iMCLab + ESP32 + Blynk IoT + OTA (Blynk.Air + ArduinoOTA)
 * RPM Monitoring dan Kontrol PWM
 **************************************************/

#define BLYNK_TEMPLATE_ID "TMPL66zkXauCd"
#define BLYNK_TEMPLATE_NAME "Robotika"
#define BLYNK_AUTH_TOKEN "7nAAyRSoP1ZcDF6lXtPPO7tslv8i4-Ra"

// Versi firmware: WAJIB dinaikkan setiap kirim update lewat Blynk.Air
#define BLYNK_FIRMWARE_VERSION "1.0.1"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoOTA.h>          // <<< ARDUINO OTA
#include <BlynkSimpleEsp32.h>

// Konfigurasi Wifi (2.4 GHz)
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Playmedia";
char pass[] = "muazzam";

// Konfigurasi ArduinoOTA (jaringan lokal)      
const char* OTA_HOSTNAME = "iMCLab-ESP32";       
const char* OTA_PASSWORD = "Ota12345";           

// ============================================================
// Motor dan LED
// ============================================================
int ledPin = 2;

int motor1Pin1 = 27;
int motor1Pin2 = 26;
int enable1Pin = 12;

// ============================================================
// Sensor RPM
// ============================================================
const byte pin_rpm = 13;
const int HOLES = 2;

volatile unsigned int rev = 0;
volatile unsigned long lastPulseTime = 0;
const unsigned long DEBOUNCE_US = 500;

// ============================================================
// PWM
// ============================================================
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;
const int DUTY_STEP = 10;
const int DUTY_MIN = 0;
const int DUTY_MAX = 255;
bool motorEnable = false;

// ============================================================
// RPM
// ============================================================
float rpm = 0.0;
unsigned long old_time = 0;

// ============================================================
String inputString = "";
BlynkTimer timer;

// ============================================================
// OTA (Blynk.Air)
// ============================================================
String otaUrl;
bool otaRequested = false;

// ============================================================
// PWM helper (kompatibel core ESP32 2.x dan 3.x)
// ============================================================
void pwmBegin()
{
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(enable1Pin, freq, resolution);
#else
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);
#endif
}

void pwmWrite(int duty)
{
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(enable1Pin, duty);
#else
  ledcWrite(pwmChannel, duty);
#endif
}

// ============================================================
// ISR Encoder
// ============================================================
void IRAM_ATTR isr()
{
  unsigned long now = micros();
  if (now - lastPulseTime > DEBOUNCE_US)
  {
    rev++;
    lastPulseTime = now;
  }
}

// ============================================================
// Sync setelah reconnect Blynk
// ============================================================
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
}

// ============================================================
// Hitung RPM
// ============================================================
void hitungRPM()
{
  unsigned long cur_time = millis();

  if (cur_time - old_time >= 1000)
  {
    detachInterrupt(digitalPinToInterrupt(pin_rpm));

    float rot_per_sec = (float)rev / (float)HOLES;
    rpm = rot_per_sec * 60.0;
    rev = 0;

    attachInterrupt(digitalPinToInterrupt(pin_rpm), isr, FALLING);
    old_time = millis();
  }
}

// ============================================================
// Terapkan PWM ke Motor
// ============================================================
void applyMotor()
{
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  pwmWrite(dutyCycle);
  digitalWrite(ledPin, dutyCycle > 0);
}

// Hentikan motor total (dipakai sebelum OTA)
void stopMotor()
{
  dutyCycle = 0;
  motorEnable = false;
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  pwmWrite(0);
  digitalWrite(ledPin, LOW);
}

// ============================================================
// Status Serial
// ============================================================
void tampilkanStatus(const char* aksi)
{
  Serial.print(aksi);
  Serial.print(" | PWM=");
  Serial.print(dutyCycle);
  Serial.print(" | RPM=");
  Serial.println(rpm, 1);
}

// ============================================================
// Kirim Data ke Blynk
// ============================================================
void kirimKeBlynk()
{
  Blynk.virtualWrite(V0, rpm);
  Blynk.virtualWrite(V1, dutyCycle);
  Blynk.virtualWrite(V2, dutyCycle);
  Blynk.virtualWrite(V3, dutyCycle > 0);
}

// ============================================================
// Slider PWM dari Blynk (V2)
// ============================================================
BLYNK_WRITE(V2)
{
  dutyCycle = constrain(param.asInt(), DUTY_MIN, DUTY_MAX);
  motorEnable = (dutyCycle > 0);
  applyMotor();
  Serial.print("PWM dari Blynk = ");
  Serial.println(dutyCycle);
}

// ============================================================
// Tombol ON/OFF dari Blynk (V3)
// ============================================================
BLYNK_WRITE(V3)
{
  motorEnable = param.asInt();

  if (motorEnable)
  {
    if (dutyCycle == 0)
      dutyCycle = 200;
  }
  else
  {
    dutyCycle = 0;
  }

  applyMotor();
  Serial.print("Motor ");
  Serial.println(motorEnable ? "ON" : "OFF");
}

// ============================================================
// OTA 1: Blynk.Air
// ============================================================
BLYNK_WRITE(InternalPinOTA)
{
  otaUrl = param.asString();
  otaRequested = true;
  Serial.println("[Blynk.Air] Permintaan OTA diterima");
}

void runBlynkAirOTA()
{
  stopMotor();
  detachInterrupt(digitalPinToInterrupt(pin_rpm));
  Serial.println("[Blynk.Air] URL: " + otaUrl);

  Blynk.disconnect();

  HTTPClient http;
  WiFiClient plainClient;
  WiFiClientSecure secureClient;

  if (otaUrl.startsWith("https"))
  {
    secureClient.setInsecure();
    http.begin(secureClient, otaUrl);
  }
  else
  {
    http.begin(plainClient, otaUrl);
  }

  const char* headerKeys[] = { "x-MD5" };
  http.collectHeaders(headerKeys, 1);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK)
  {
    Serial.printf("[Blynk.Air] Gagal, HTTP code = %d\n", httpCode);
    http.end();
    attachInterrupt(digitalPinToInterrupt(pin_rpm), isr, FALLING);
    return;
  }

  int contentLength = http.getSize();
  Serial.printf("[Blynk.Air] Ukuran firmware: %d byte\n", contentLength);

  if (contentLength <= 0 || !Update.begin(contentLength))
  {
    Serial.println("[Blynk.Air] Partisi OTA tidak cukup / ukuran tidak valid");
    http.end();
    attachInterrupt(digitalPinToInterrupt(pin_rpm), isr, FALLING);
    return;
  }

  if (http.hasHeader("x-MD5"))
  {
    String md5 = http.header("x-MD5");
    if (md5.length() == 32)
    {
      md5.toLowerCase();
      Update.setMD5(md5.c_str());
      Serial.println("[Blynk.Air] MD5: " + md5);
    }
  }

  unsigned long t0 = millis();
  size_t written = Update.writeStream(http.getStream());
  http.end();

  Serial.printf("[Blynk.Air] Tertulis %u / %d byte dalam %lu ms\n",
                (unsigned)written, contentLength, millis() - t0);

  if (written != (size_t)contentLength || !Update.end() || !Update.isFinished())
  {
    Serial.printf("[Blynk.Air] Update gagal, error #%d\n", Update.getError());
    attachInterrupt(digitalPinToInterrupt(pin_rpm), isr, FALLING);
    return;
  }

  Serial.println("[Blynk.Air] Update sukses, reboot...");
  delay(500);
  ESP.restart();
}

// ============================================================
// OTA 2: ArduinoOTA (WiFi lokal)                  // <<< ARDUINO OTA
// ============================================================
void setupArduinoOTA()
{
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    stopMotor();                                  // motor mati selama update
    detachInterrupt(digitalPinToInterrupt(pin_rpm));
    Serial.println("[ArduinoOTA] Mulai update");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[ArduinoOTA] Progres: %u%%\r", (progress * 100) / total);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\n[ArduinoOTA] Selesai, reboot...");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[ArduinoOTA] Error[%u]\n", error);
    attachInterrupt(digitalPinToInterrupt(pin_rpm), isr, FALLING);
  });

  ArduinoOTA.begin();

  Serial.print("[ArduinoOTA] Siap | Hostname: ");
  Serial.print(OTA_HOSTNAME);
  Serial.print(" | IP: ");
  Serial.println(WiFi.localIP());
}

// ============================================================
// Setup
// ============================================================
void setup()
{
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(pin_rpm, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pin_rpm), isr, FALLING);

  pwmBegin();

  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  pwmWrite(0);

  old_time = millis();

  // Koneksi ke Blynk (sekaligus WiFi)
  Blynk.begin(auth, ssid, pass);

  setupArduinoOTA();                              

  timer.setInterval(1000L, kirimKeBlynk);

  Serial.println("================================");
  Serial.println("iMCLab + Blynk + OTA Ready");
  Serial.println("Firmware v" BLYNK_FIRMWARE_VERSION);
  Serial.println("================================");
  Serial.println("1 = ON");
  Serial.println("0 = OFF");
  Serial.println("+ = PWM naik");
  Serial.println("- = PWM turun");
  Serial.println("PWM:200");
  Serial.println("R = lihat RPM");
  Serial.println("V = lihat versi firmware");
}

// ============================================================
// Loop
// ============================================================
void loop()
{
  if (otaRequested)
  {
    otaRequested = false;
    runBlynkAirOTA();
  }

  ArduinoOTA.handle();                            
  Blynk.run();
  timer.run();
  hitungRPM();

  while (Serial.available() > 0)
  {
    char c = Serial.read();

    if (c == '\n')
    {
      inputString.trim();

      if (inputString.startsWith("PWM:"))
      {
        int val = inputString.substring(4).toInt();
        val = constrain(val, DUTY_MIN, DUTY_MAX);

        dutyCycle = val;
        motorEnable = (dutyCycle > 0);
        applyMotor();
        tampilkanStatus("Set PWM");
      }
      else if (inputString.length() > 0)
      {
        char cmd = inputString.charAt(0);

        if (cmd == '1')
        {
          if (dutyCycle == 0)
            dutyCycle = 200;

          motorEnable = true;
          applyMotor();
          tampilkanStatus("Motor ON");
        }
        else if (cmd == '0')
        {
          dutyCycle = 0;
          motorEnable = false;
          applyMotor();
          Serial.println("Motor OFF");
        }
        else if (cmd == '+')
        {
          dutyCycle += DUTY_STEP;
          if (dutyCycle > DUTY_MAX)
            dutyCycle = DUTY_MAX;

          motorEnable = true;
          applyMotor();
          tampilkanStatus("PWM NAIK");
        }
        else if (cmd == '-')
        {
          dutyCycle -= DUTY_STEP;
          if (dutyCycle < DUTY_MIN)
            dutyCycle = DUTY_MIN;

          motorEnable = (dutyCycle > 0);
          applyMotor();
          tampilkanStatus("PWM TURUN");
        }
        else if (cmd == 'R' || cmd == 'r')
        {
          Serial.print("RPM = ");
          Serial.print(rpm, 1);
          Serial.print(" | PWM = ");
          Serial.println(dutyCycle);
        }
        else if (cmd == 'V' || cmd == 'v')
        {
          Serial.println("Firmware v" BLYNK_FIRMWARE_VERSION);
        }
      }
      inputString = "";
    }
    else if (c != '\r')
    {
      inputString += c;
    }
  }
}