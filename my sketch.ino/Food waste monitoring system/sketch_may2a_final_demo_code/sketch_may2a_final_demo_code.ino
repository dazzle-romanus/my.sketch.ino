

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// DHT setup
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Gas sensor setup
#define gasSensorPin A0

// Buzzer & LED setup
#define buzzerPin 3
#define redLedPin 4
#define greenLedPin 5

// GSM Module (SIM800L)
#define rxPin 8
#define txPin 7
SoftwareSerial gsm(rxPin, txPin);

// Thresholds
float methaneThreshold = 50.0;
float temperatureThreshold = 30.0;
float humidityThreshold = 70.0;

// Timer for periodic SMS
unsigned long lastSmsTime = 0;
const unsigned long smsInterval = 7200000; // 2 hours in milliseconds
String lastStatus = "";

void setup() {
  lcd.init();
  lcd.backlight();
  dht.begin();
  Serial.begin(9600);
  gsm.begin(9600);

  pinMode(buzzerPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  digitalWrite(buzzerPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);

  displayScrollingText("Smart Food Monitoring Device");
  delay(2000);
  lcd.clear();

  delay(3000); // Wait for GSM module
  gsm.println("AT");
  delay(1000);
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float methaneConcentration = averageGasValue();

  if (isnan(temperature) || isnan(humidity)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");
    delay(2000);
    return;
  }

  bool isSpoiled = checkSpoilage(methaneConcentration, temperature, humidity);

  displayScrollingText("Temp: " + String(temperature, 1) + "C");
  displayScrollingText("Hum: " + String(humidity, 1) + "%");
  displayScrollingText("Methane: " + String(methaneConcentration, 1) + "ppm");

  if (methaneConcentration < methaneThreshold) {
    displayScrollingText("Status: Fresh & Safe");
  } else {
    displayScrollingText("Status: Spoilage Detected!");
  }

  // Send SMS immediately if status changed to SPOILED
  if (isSpoiled && lastStatus != "SPOILED") {
    sendSMS("⚠️ Food Spoilage Detected!\nTemp: " + String(temperature) + "C\nHumidity: " + String(humidity) + "%\nMethane: " + String(methaneConcentration) + " ppm");
    // Optional emergency call:
    // makeCall("9876543210");
  }

  // Send periodic SMS every 2 hours
  if (millis() - lastSmsTime > smsInterval) {
    String statusText = isSpoiled ? "⚠️ SPOILED" : "✅ Fresh & Safe";
    sendSMS("Food Status Update: " + statusText + "\nTemp: " + String(temperature) + "C\nHumidity: " + String(humidity) + "%\nMethane: " + String(methaneConcentration) + " ppm");
    lastSmsTime = millis();
  }

  lastStatus = isSpoiled ? "SPOILED" : "GOOD";

  // LED & Continuous Buzzer logic
  if (methaneConcentration > methaneThreshold) {
  digitalWrite(redLedPin, HIGH);   
  digitalWrite(greenLedPin, LOW);  

  // Continuous faster beep using millis()
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;
  const unsigned long beepInterval = 100;  // 100 ms toggle

  if (currentMillis - previousMillis >= beepInterval) {
    previousMillis = currentMillis;
    digitalWrite(buzzerPin, !digitalRead(buzzerPin));  // Toggle buzzer
  }
} else {
  digitalWrite(greenLedPin, HIGH); 
  digitalWrite(redLedPin, LOW);    
  digitalWrite(buzzerPin, LOW);    
}


  // Send sensor values to ESP8266 via Serial
  Serial.print("T:");
  Serial.print(temperature);
  Serial.print(",H:");
  Serial.print(humidity);
  Serial.print(",M:");
  Serial.print(methaneConcentration);
  Serial.println();

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C  ");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %  ");

  Serial.print("Methane: ");
  Serial.print(methaneConcentration);
  Serial.print(" ppm  ");

  Serial.print("Status: ");
  Serial.println(isSpoiled ? "Spoiled" : "Fresh");

  delay(2000);
}

float averageGasValue() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(gasSensorPin);
    delay(10);
  }
  return (sum / 20.0) / 1024.0 * 100;  // Convert to ppm estimate
}

bool checkSpoilage(float methane, float temp, float hum) {
  return (methane > methaneThreshold || temp > temperatureThreshold || hum > humidityThreshold);
}

void displayScrollingText(String text) {
  lcd.clear();
  int len = text.length();
  if (len <= 16) {
    lcd.setCursor(0, 0);
    lcd.print(text);
  } else {
    for (int i = 0; i <= len - 16; i++) {
      lcd.setCursor(0, 0);
      lcd.print(text.substring(i, i + 16));
      delay(400);
    }
  }
  delay(1000);
}

void sendSMS(String message) {
  gsm.println("AT+CMGF=1"); // SMS text mode
  delay(500);
  gsm.println("AT+CMGS=\"+916383361989\"");  // Replace with your mobile number
  delay(500);
  gsm.print(message);
  delay(500);
  gsm.write(26); // CTRL+Z
  delay(3000);
}

void makeCall(String number) {
  gsm.print("ATD");
  gsm.print(number);
  gsm.println(";");
  delay(10000);
  gsm.println("ATH"); // Hang up
}
