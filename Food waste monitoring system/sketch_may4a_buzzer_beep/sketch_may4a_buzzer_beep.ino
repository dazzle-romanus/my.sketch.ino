#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust I2C address if needed

// DHT setup
#define DHTPIN 3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Gas sensor setup
#define gasSensorPin A0  // MQ-4 connected to A0

// Buzzer & LED setup
#define buzzerPin 4   // Buzzer connected to D3
#define redLedPin 5   // Red LED connected to D4
#define greenLedPin 6  // Green LED connected to D5

// Global Spoilage Detection Thresholds
float methaneThreshold = 50.0;  // Adjusted threshold
float temperatureThreshold = 30.0;  // High temp indicating spoilage
float humidityThreshold = 70.0;  // High humidity indicating spoilage

void setup() {
  lcd.init();
  lcd.backlight();
  dht.begin();
  Serial.begin(9600);

  // Pin modes
  pinMode(buzzerPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  // Initial state
  digitalWrite(buzzerPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);

  // Display startup message with scrolling effect
  displayScrollingText("Smart Food Monitoring Device");

  delay(2000);  // Wait before clearing screen
  lcd.clear();
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float methaneConcentration = averageGasValue();  // <-- Updated to use average

  // Error handling for sensors
  if (isnan(temperature) || isnan(humidity)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");
    delay(2000);
    return;
  }

  // Check spoilage condition
  bool isSpoiled = checkSpoilage(methaneConcentration, temperature, humidity);

  // Display results on LCD with scrolling effect
  displayScrollingText("Temp: " + String(temperature, 1) + "C");
  displayScrollingText("Hum: " + String(humidity, 1) + "%");
  displayScrollingText("Methane: " + String(methaneConcentration, 1) + "ppm");

  // Display Fresh & Safe or Spoilage Detected based on the methane level
  if (methaneConcentration < methaneThreshold) {
    displayScrollingText("Status: Fresh & Safe");
  } else {
    displayScrollingText("Status: Spoilage Detected!");
  }

  // Send final status to ESP8266
  if (isSpoiled) {
    Serial.println("Spoilage Detected!"); 
  } else {
    Serial.println("Fresh & Safe"); 
  }

  // Buzzer & LED logic
   if (methaneConcentration > methaneThreshold) {
    digitalWrite(redLedPin, HIGH);   
    digitalWrite(greenLedPin, LOW);  

    // Beep 20 times quickly like an alarm
    for (int i = 0; i < 20; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(200);
      digitalWrite(buzzerPin, LOW);
      delay(200);
    }
  } else {
    digitalWrite(greenLedPin, HIGH); 
    digitalWrite(redLedPin, LOW);    
    digitalWrite(buzzerPin, LOW);    
  }


  // Send detailed sensor data to ESP8266
  Serial.print("T:");
  Serial.print(temperature);
  Serial.print(",H:");
  Serial.print(humidity);
  Serial.print(",M:");
  Serial.print(methaneConcentration);
  Serial.println();

  // Print data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C  ");
  
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %  ");
  
  Serial.print("Methane Concentration: ");
  Serial.print(methaneConcentration);
  Serial.print(" ppm  ");
  
  if (methaneConcentration < methaneThreshold) {
    Serial.println("Status: Fresh & Safe");
  } else {
    Serial.println("Status: Detected");
  }

  delay(2000);  // Delay between sensor readings
}

// Function to calculate average methane concentration
float averageGasValue() {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(gasSensorPin);
    delay(10); // Small delay between samples
  }
  return (sum / 20.0) / 1024.0 * 100;  // Average and convert
}

// Function to check if spoilage is happening
bool checkSpoilage(float methane, float temp, float hum) {
  return (methane > methaneThreshold || temp > temperatureThreshold || hum > humidityThreshold);
}

// Function to scroll text on LCD if too long
void displayScrollingText(String text) {
  lcd.clear();
  int len = text.length();
  if (len <= 16) { // If text fits, print directly
    lcd.setCursor(0, 0);
    lcd.print(text);
  } else { // If text is too long, scroll it
    for (int i = 0; i <= len - 16; i++) {
      lcd.setCursor(0, 0);
      lcd.print(text.substring(i, i + 16));
      delay(500);
    }
  }
  delay(2000); // Pause before the next reading
}
