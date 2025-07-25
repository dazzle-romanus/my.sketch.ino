// Blynk IOT Smart Plant Monitoring System

/* Connections
Relay 1. D1 (Water Pump)
Relay 2. D2 (Water Pump)
Btn.   D7
Soil.  A0
PIR.   D5
Temp.  D4
*/

// Include the library files
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x3F, 16, 2);

char auth[] = "235QwLEw79ZySGElCbWP4cmmAzdHdGk-";  // Enter your Blynk Auth token
char ssid[] = "dazzle";  // Enter your WIFI SSID
char pass[] = "romanus5656";  // Enter your WIFI Password

DHT dht(D4, DHT11);  // (DHT sensor pin, sensor type)  D4 DHT11 Temperature Sensor
BlynkTimer timer;

// Define component pins
#define soil A0     // A0 Soil Moisture Sensor
#define PIR D5      // D5 PIR Motion Sensor
#define RELAY_PIN_1 D1  // D1 Relay 1 (Other relay)
#define RELAY_PIN_2 D2  // D2 Relay 2 (Water pump)
#define PUSH_BUTTON_1 D7   // D7 Button
#define VPIN_BUTTON_1 V11  // Virtual pin for Water pump (Relay 1)
#define VPIN_BUTTON_2 V12  // Virtual pin for Water Pump (Relay 2)

int PIR_ToggleValue;
int relay1State = HIGH;
int relay2State = HIGH;
int pushButton1State = HIGH;

// Create three variables for pressure
double T, P;
char status;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();
  pinMode(PIR, INPUT);

  // Setup relay pins
  pinMode(RELAY_PIN_1, OUTPUT); // water pump relay 1
  digitalWrite(RELAY_PIN_1, HIGH);
  
  pinMode(RELAY_PIN_2, OUTPUT);  // Water pump relay 2
  digitalWrite(RELAY_PIN_2, HIGH);

  pinMode(PUSH_BUTTON_1, INPUT_PULLUP);
  digitalWrite(RELAY_PIN_1, relay1State);

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  dht.begin();

  lcd.setCursor(0, 0);
  lcd.print("  Initializing  ");
  for (int a = 5; a <= 10; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();
  lcd.setCursor(11, 1);
  lcd.print("W:OFF");

  // Call the function
  timer.setInterval(100L, soilMoistureSensor);
  timer.setInterval(100L, DHT11sensor);
  timer.setInterval(500L, checkPhysicalButton);
}

// Get the DHT11 sensor values
void DHT11sensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);

  lcd.setCursor(8, 0);
  lcd.print("H:");
  lcd.print(h);
}

// Get the soil moisture values
void soilMoistureSensor() {
  int value = analogRead(soil);
  value = map(value, 0, 1024, 0, 100);
  value = (value - 100) * -1;

  Blynk.virtualWrite(V3, value);
  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(value);
  lcd.print(" ");
}

// Get the PIR sensor values
void PIRsensor() {
  bool value = digitalRead(PIR);
  if (value) {
    Blynk.logEvent("pirmotion", "WARNING! Motion Detected!");  // Enter your Event Name
    WidgetLED LED(V5);
    LED.on();
  } else {
    WidgetLED LED(V5);
    LED.off();
  }
}

BLYNK_WRITE(V6) {
  PIR_ToggleValue = param.asInt();
}

BLYNK_CONNECTED() {
  // Request the latest state from the server
  Blynk.syncVirtual(VPIN_BUTTON_1);  // Sync state for Water Pump (Relay 1)
  Blynk.syncVirtual(VPIN_BUTTON_2);  // Sync state for Water Pump (Relay 2)
}

// Control Relay 1 (water pump)
BLYNK_WRITE(VPIN_BUTTON_1) {
  relay1State = param.asInt();
  // Invert the logic to match the relay's active-low behavior
  if (relay1State == 1) {
    digitalWrite(RELAY_PIN_1, LOW);  // Turn ON relay
  } else {
    digitalWrite(RELAY_PIN_1, HIGH);  // Turn OFF relay
  }
}

// Control Relay 2 (Water Pump)
BLYNK_WRITE(VPIN_BUTTON_2) {
  relay2State = param.asInt();
  // Invert the logic to match the relay's active-low behavior
  if (relay2State == 1) {
    digitalWrite(RELAY_PIN_2, LOW);  // Turn ON relay
  } else {
    digitalWrite(RELAY_PIN_2, HIGH);  // Turn OFF relay
  }
}

void checkPhysicalButton() {
  if (digitalRead(PUSH_BUTTON_1) == LOW) {
    // pushButton1State is used to avoid sequential toggles
    if (pushButton1State != LOW) {
      // Toggle Relay state
      relay1State = !relay1State;
      digitalWrite(RELAY_PIN_1, relay1State == 1 ? LOW : HIGH);

      // Update Button Widget
      Blynk.virtualWrite(VPIN_BUTTON_1, relay1State);
    }
    pushButton1State = LOW;
  } else {
    pushButton1State = HIGH;
  }
}

void loop() {
  if (PIR_ToggleValue == 1) {
    lcd.setCursor(5, 1);
    lcd.print("M:ON ");
    PIRsensor();
  } else {
    lcd.setCursor(5, 1);
    lcd.print("M:OFF");
    WidgetLED LED(V5);
    LED.off();
  }

  // Update LCD for Relay 1
  if (relay1State == HIGH) {
    lcd.setCursor(11, 1);
    lcd.print("R1:ON ");
  } else {
    lcd.setCursor(11, 1);
    lcd.print("R1:OFF");
  }

  // Update LCD for Relay 2 (Water Pump)
  if (relay2State == HIGH) {
    lcd.setCursor(5, 0);
    lcd.print("WP:ON ");
  } else {
    lcd.setCursor(5, 0);
    lcd.print("WP:OFF");
  }

  Blynk.run();  // Run the Blynk library
  timer.run();  // Run the Blynk timer
}
