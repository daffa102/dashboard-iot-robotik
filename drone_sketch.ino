#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =============================================
// 1. KONFIGURASI PIN SENSOR
// =============================================
#define PH_PIN        34  // Sensor pH di GPIO 34 (ADC1)
#define TURBIDITY_PIN 35  // Sensor Turbidity di GPIO 35 (ADC1)

// =============================================
// 2. KONFIGURASI WIFI & SERVER
// =============================================
const char* ssid     = "DaffaDrone";       // Ganti dengan SSID WiFi kamu
const char* password = "daffa123";   // Ganti dengan password WiFi kamu
String serverName    = "http://daffa.underwaterdrone.my.id/api.php";

// =============================================
// 3. INISIALISASI OBJEK & VARIABEL
// =============================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

float phValue      = 0.0;
float turbidityNTU = 0.0;

unsigned long lastTime   = 0;
unsigned long timerDelay = 5000; // Kirim data ke server setiap 5 detik

// =============================================
// SETUP
// =============================================
void setup() {
  Serial.begin(115200);

  // Setup LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Drone System");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");

  // Koneksi WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.println("IP: " + WiFi.localIP().toString());
  lcd.clear();
}

// =============================================
// LOOP
// =============================================
void loop() {

  // --- A. PEMBACAAN SENSOR ---

  // 1. Baca pH
  int   adcPH    = analogRead(PH_PIN);
  float voltagePH = adcPH * (3.3 / 4095.0);
  // Kalibrasi manual: ubah angka 2.5 agar pH terbaca 7.0 di air netral
  phValue = 3.5 * voltagePH + 2.5;

  // 2. Baca Turbidity
  int   adcTurb  = analogRead(TURBIDITY_PIN);
  float voltTurb = adcTurb * (3.3 / 4095.0);
  // Estimasi NTU: semakin rendah voltase → semakin keruh
  turbidityNTU = -1120.4 * (voltTurb * voltTurb) + 5742.3 * voltTurb - 4352.9;
  if (turbidityNTU < 0) turbidityNTU = 0;

  // --- B. UPDATE TAMPILAN LCD ---
  lcd.setCursor(0, 0);
  lcd.print("pH: " + String(phValue, 2) + "        "); // spasi untuk clear sisa karakter
  lcd.setCursor(0, 1);
  lcd.print("Turb:" + String(turbidityNTU, 0) + " NTU   ");

  // Juga tampilkan di Serial Monitor
  Serial.println("=== Pembacaan Sensor ===");
  Serial.println("pH        : " + String(phValue, 2));
  Serial.println("Turbidity : " + String(turbidityNTU, 0) + " NTU");

  // --- C. KIRIM DATA KE SERVER (setiap timerDelay ms) ---
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      // Susun URL dengan parameter yang sesuai api.php
      // kualitas_air = nilai pH
      // tahan        = nilai Turbidity (NTU)
      // daya_listrik = baterai (hardcoded 100 karena belum ada sensor baterai)
      String url = serverName
                   + "?kualitas_air=" + String(phValue, 2)
                   + "&tahan="        + String(turbidityNTU, 2)
                   + "&daya_listrik=100";

      Serial.println("Mengirim ke: " + url);

      http.begin(url.c_str());
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.println("HTTP Response: " + String(httpResponseCode));
        Serial.println("Server: " + http.getString());
      } else {
        Serial.println("HTTP Error: " + String(httpResponseCode));
      }

      http.end();
    } else {
      Serial.println("WiFi Disconnected! Mencoba reconnect...");
      WiFi.reconnect();
    }

    lastTime = millis();
  }

  delay(500); // Jeda kecil agar ADC stabil
}