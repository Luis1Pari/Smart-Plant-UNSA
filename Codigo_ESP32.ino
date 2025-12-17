#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* thingSpeakKey = "YOUR_KEY"; 
const char* scriptURL = "YOUR_URL";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SOIL_MOISTURE_PIN 34
#define PUMP_PIN 25 

unsigned long lastCloudTime = 0;
const unsigned long cloudInterval = 60000; 

void drawFace(int x, int y, int mood) {
  display.drawCircle(x, y, 20, SSD1306_WHITE); 
  display.fillCircle(x - 6, y - 6, 4, SSD1306_WHITE); 
  display.fillCircle(x + 6, y - 6, 4, SSD1306_WHITE); 

  if (mood == 1) { // 😊 Feliz
    display.drawLine(x - 7, y + 8, x - 3, y + 10, SSD1306_WHITE);
    display.drawLine(x - 3, y + 10, x + 3, y + 10, SSD1306_WHITE);
    display.drawLine(x + 3, y + 10, x + 7, y + 8, SSD1306_WHITE);
  } else if (mood == 0) { // 😐 Neutral
    display.drawLine(x - 7, y + 10, x + 7, y + 10, SSD1306_WHITE);
  } else { // ☹️ Triste
    display.drawLine(x - 7, y + 10, x - 3, y + 8, SSD1306_WHITE);
    display.drawLine(x - 3, y + 8, x + 3, y + 8, SSD1306_WHITE);
    display.drawLine(x + 3, y + 8, x + 7, y + 10, SSD1306_WHITE);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;); 
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10);
  display.print("Smart Plant UNSA"); 
  display.display();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado");
}

void loop() {
  int moistureValue = analogRead(SOIL_MOISTURE_PIN);
  int percentage = map(moistureValue, 4095, 1200, 0, 100);
  percentage = constrain(percentage, 0, 100);

  display.clearDisplay();
  display.setTextSize(1); 
  display.setCursor(5, 5);
  display.print("Humedad:");
  
  display.setTextSize(3);  
  display.setCursor(10, 30);
  display.print(percentage);
  display.print("%");

  if (percentage > 60) {
    drawFace(100, 30, 1); 
    digitalWrite(PUMP_PIN, LOW);
  } else if (percentage >= 30) {
    drawFace(100, 30, 0); 
    digitalWrite(PUMP_PIN, LOW);
  } else {
    drawFace(100, 30, -1); 
    digitalWrite(PUMP_PIN, HIGH);
    display.setTextSize(1);
    display.setCursor(10, 55);
    display.print("REGANDO...");
  }

  display.display();

  if (millis() - lastCloudTime >= cloudInterval || lastCloudTime == 0) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      // Enviar a ThingSpeak (Tu línea original)
      http.begin("http://api.thingspeak.com/update?api_key=" + String(thingSpeakKey) + "&field1=" + String(percentage));
      http.GET();
      http.end();
      
      // 2. Google Sheets (CORREGIDO PARA QUE LLEGUE YA)
      HTTPClient httpG;
      String urlGoogle = String(scriptURL) + "?valor=" + String(percentage);
      httpG.begin(urlGoogle); 
      httpG.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      
      int httpCodeG = httpG.GET(); // Esto dispara el envío
      Serial.print("Respuesta Excel: "); 
      Serial.println(httpCodeG); 
      
      httpG.end();
      
      lastCloudTime = millis(); // Reinicia el reloj de 1 minuto
    }
  }

  delay(2000); 
}
