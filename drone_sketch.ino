#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h> // Library untuk kontrol ESC/Servo pada ESP32

// =============================================
// 1. KONFIGURASI PIN & OFFSET
// =============================================
#define PH_PIN        35  
#define TURBIDITY_PIN 34  
#define TEMP_PIN      4   
#define MOTOR_PIN     13  

const float PH_OFFSET = -1.1;

// =============================================
// 2. KONFIGURASI WIFI & SERVER
// =============================================
const char* ssid     = "daffa";       
const char* password = "daffa123";               
String serverName    = "http://daffa.underwaterdrone.my.id/api.php";

// =============================================
// 3. INISIALISASI OBJEK
// =============================================
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
Servo thruster; // Objek untuk mengontrol ESC

void setup() {
  Serial.begin(115200);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Drone System");

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  pinMode(TEMP_PIN, INPUT_PULLUP);
  sensors.begin();
  
  // --- INISIALISASI ESC (ARMING) ---
  thruster.attach(MOTOR_PIN, 1000, 2000); // Attach pin dengan range pulsa 1000-2000us
  thruster.writeMicroseconds(1500);       // Kirim sinyal NETRAL (Stop) untuk arming
  Serial.println("ESC Arming... Please wait.");
  
  lcd.clear();
  lcd.print("Arming ESC...");
  delay(2000); // Tunggu 2 detik agar ESC mengenali sinyal netral (beeping berhenti)

  lcd.clear();
  lcd.print("Ready!");
  delay(1000);
}

void loop() {
  // --- A. BACA SENSOR SUHU ---
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  if (temp == -127.00) temp = 0.0;

  // --- B. BACA SENSOR PH ---
  int rawPH = analogRead(PH_PIN);
  float voltPH = rawPH * (3.3 / 4095.0);
  float ph = 3.5 * voltPH + PH_OFFSET;
  if (ph > 14.0) ph = 14.0;
  if (ph < 0.0) ph = 0.0;

  // --- C. BACA SENSOR TURBIDITY ---
  int rawTurb = analogRead(TURBIDITY_PIN);
  float voltTurb = rawTurb * (3.3 / 4095.0);
  float ntu = 0;
  // Kalibrasi kasar berdasarkan voltase
  if (voltTurb < 1.0) ntu = 3000;
  else if (voltTurb > 2.5) ntu = 0;
  else ntu = map(voltTurb * 100, 100, 250, 3000, 0);

  // --- D. POLLING PERINTAH MOTOR ---
  checkMotorCommand();

  // --- E. KIRIM DATA KE API ---
  sendDataToServer(ph, ntu, temp);

  // --- F. UPDATE LCD ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PH:"); lcd.print(ph, 1);
  lcd.print(" TMP:"); lcd.print(temp, 1);
  lcd.setCursor(0, 1);
  lcd.print("TURB:"); lcd.print((int)ntu);
  lcd.print(" NTU");

  delay(3000);
}

void checkMotorCommand() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = serverName + "?get_command=true";
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      if (payload.indexOf("\"START\"") != -1) {
        thruster.writeMicroseconds(1900); // Jalan Maju (Full Throttle)
        Serial.println("Motor: START (PWM 1900)");
      } else {
        thruster.writeMicroseconds(1500); // Berhenti (Neutral)
        Serial.println("Motor: STOP (PWM 1500)");
      }
    }
    http.end();
  }
}

void sendDataToServer(float ph, float turb, float temp) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Note: 'tahan' digunakan untuk turbidity di dashboard
    String url = serverName + "?kualitas_air=" + String(ph, 2) +
                 "&tahan=" + String(turb, 2) +
                 "&daya_listrik=100" +
                 "&suhu=" + String(temp, 2);
    
    Serial.println("Sending data to dashboard...");
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.print("HTTP Success: ");
      Serial.println(httpCode);
    }
    http.end();
  }
}