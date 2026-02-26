#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// 1. KONFIGURASI PIN SENSOR
#define PH_PIN 34          // Sensor pH di GPIO 34 (ADC1)
#define TURBIDITY_PIN 35    // Sensor Turbidity di GPIO 35 (ADC1)
#define DS18B20_PIN 4      // Sensor Suhu di GPIO 4

// 2. KONFIGURASI WIFI & SERVER
const char* ssid     = "Andhika";     // GANTI INI
const char* password = "kosjujun123"; // GANTI INI
String serverName    = "http://daffa.underwaterdrone.my.id/api.php";

// 3. INISIALISASI OBJEK
LiquidCrystal_I2C lcd(0x27, 16, 2); 
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

// Variabel Data
float phValue, turbidityNTU, temperatureC;
unsigned long lastTime = 0;
unsigned long timerDelay = 5000; // Kirim data setiap 5 detik

void setup() {
  Serial.begin(115200);
  
  // Setup LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Drone System");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");

  // Setup Sensor & WiFi
  sensors.begin();
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  lcd.clear();
}

void loop() {
  // --- A. PEMBACAAN SENSOR ---
  
  // 1. Baca pH (Sederhana dengan Kalibrasi)
  int adcPH = analogRead(PH_PIN);
  float voltagePH = adcPH * (3.3 / 4095.0);
  phValue = 3.5 * voltagePH + 2.5; // Kalibrasi manual: Sesuaikan angka 2.5 agar pas 7.0 di air netral

  // 2. Baca Turbidity
  int adcTurb = analogRead(TURBIDITY_PIN);
  float voltTurb = adcTurb * (3.3 / 4095.0);
  // Rumus estimasi NTU (Semakin rendah voltase, semakin keruh)
  turbidityNTU = -1120.4 * (voltTurb * voltTurb) + 5742.3 * voltTurb - 4352.9; 
  if(turbidityNTU < 0) turbidityNTU = 0;

  // 3. Baca Suhu
  sensors.requestTemperatures(); 
  temperatureC = sensors.getTempCByIndex(0);

  // --- B. UPDATE TAMPILAN LCD ---
  lcd.setCursor(0, 0);
  lcd.print("PH:" + String(phValue, 1) + "  T:" + String(temperatureC, 1) + "C");
  lcd.setCursor(0, 1);
  lcd.print("Turb:" + String(turbidityNTU, 0) + " NTU   ");

  // --- C. KIRIM DATA KE DASHBOARD (SINKRON DENGAN api.php) ---
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      // Susun URL sesuai variabel di api.php kamu
      // kualitas_air = pH, tahan = Turbidity, udara = Suhu
      String url = serverName + "?kualitas_air=" + String(phValue, 2) + 
                   "&tahan=" + String(turbidityNTU, 2) + 
                   "&udara=" + String(temperatureC, 2) + 
                   "&daya_listrik=100"; // Baterai diasumsikan 100%

      Serial.println("Mengirim data ke: " + url);
      
      http.begin(url.c_str());
      int httpResponseCode = http.GET();
      
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println("Server Response: " + payload);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}