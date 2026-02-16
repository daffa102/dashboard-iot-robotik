#include <WiFi.h>
#include <HTTPClient.h>

const char WIFI_SSID[] = "Wokwi-GUEST";        
const char WIFI_PASSWORD[] = ""; 

// Ganti demgan IP Address komputer Anda jika menggunakan XAMPP (contoh: 192.168.1.5)
// Atau domain hosting Anda
String HOST_NAME = "http://rapip.zakkal.my.id"; // Tanpa tanda / di akhir
// String PATH_NAME = "/products/arduino.php";
String queryString = "kualitas_air=80.5&tahan=150.2&udara=29.1&daya_listrik=95.0"; // Jangan pakai /public_html, langsung nama filenya saja jika ditaruh di root

void setup() {
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Simulasi data sensor (Ganti dengan pembacaan sensor sebenarnya)
  float kualitas_air = random(0, 100); 
  float tahan = random(0, 500); 
  float udara = random(20, 40); 
  float daya_listrik = random(10, 100);

  // Periksa koneksi WiFi
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    // Buat Query String
    String queryString = "?kualitas_air=" + String(kualitas_air) 
                       + "&tahan=" + String(tahan)
                       + "&udara=" + String(udara)
                       + "&daya_listrik=" + String(daya_listrik);

    // URL Lengkap
    String serverPath = HOST_NAME + PATH_NAME + queryString;
    
    Serial.println("Request URL: " + serverPath);

    http.begin(serverPath.c_str());
    
    // Kirim HTTP GET request
    int httpCode = http.GET();

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response: " + payload);
      } else {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }

  // Kirim data setiap 5 detik
  delay(5000);
}