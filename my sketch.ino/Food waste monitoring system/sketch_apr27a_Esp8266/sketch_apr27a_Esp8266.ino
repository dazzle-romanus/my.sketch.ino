#define BLYNK_TEMPLATE_ID "Food Monitoring Device"
#define BLYNK_TEMPLATE_NAME "Food Monitoring Device"
#define BLYNK_AUTH_TOKEN "d83jzkeEVroQ0FcgKXFm3YwlT691oDNo"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "Dazzle";
char pass[] = "romanus6565";

void setup() {
  Serial.begin(9600); // Communication with Arduino
  Serial.println("ESP8266 starting...");

  WiFi.begin(ssid, pass); // Start Wi-Fi connection (non-blocking)
  Blynk.config(BLYNK_AUTH_TOKEN); // Configure Blynk (without blocking)
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  }

  if (Serial.available()) {
    String data = Serial.readStringUntil('\n'); // Read Arduino Serial

    Serial.println("Received from Arduino: " + data);

    if (data.startsWith("T:")) {
      float temperature = extractValue(data, 'T');
      float humidity = extractValue(data, 'H');
      float methane = extractValue(data, 'M');

      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");

      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");

      Serial.print("Methane: ");
      Serial.print(methane);
      Serial.println(" ppm");

      if (WiFi.status() == WL_CONNECTED) { // Only send to Blynk if Wi-Fi OK
        Blynk.virtualWrite(V0, temperature);
        Blynk.virtualWrite(V1, humidity);
        Blynk.virtualWrite(V2, methane);

        if (methane > 55.0) {
          Blynk.virtualWrite(V3, 0);   // Green LED OFF
          Blynk.virtualWrite(V4, 255); // Red LED ON
          Blynk.logEvent("food_alert", "Your food is Rotten");
        } else {
          Blynk.virtualWrite(V3, 255); // Green LED ON
          Blynk.virtualWrite(V4, 0);   // Red LED OFF
          Blynk.logEvent("food_alert", "Your food is Fresh and Safe");
        }
      }
    }
    else if (data == "Spoilage Detected!" || data == "Fresh & Safe") {
      Serial.println("Food Status: " + data);
    }
  }
}

// Helper function
float extractValue(String input, char parameter) {
  int startIndex = input.indexOf(parameter + String(":")) + 2;
  int endIndex = input.indexOf(",", startIndex);
  if (endIndex == -1) endIndex = input.length();
  String valueStr = input.substring(startIndex, endIndex);
  return valueStr.toFloat();
}