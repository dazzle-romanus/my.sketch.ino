#define BLYNK_TEMPLATE_ID "TMPL3XgETBW1i"
#define BLYNK_TEMPLATE_NAME "SMART IOT BASED METER"
#define BLYNK_PRINT Serial

#include "EmonLib.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// LCD I2C settings
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD I2C address, 16x2

// Calibration constants
const float vCalibration = 42.5;
const float currCalibration = 1.80;

// Blynk credentials
char auth[] = "fDgEpAnJLR2jOp_WX-GGYEJkH4yU4ej9";
char ssid[] = "dazzle";
char pass[] = "romanus5666";

EnergyMonitor emon;
BlynkTimer timer;

// Energy and cost tracking
float kWh = 0.0;
float cost = 0.0;
const float ratePerkWh = 6.5; // ₹ per kWh
unsigned long lastMillis = millis();

// EEPROM addresses
const int addrKWh = 12;
const int addrCost = 16;

// Display
int displayPage = 0;
const int resetButtonPin = 4;

// ------------------------ SETUP ------------------------ //
void setup() {
  Serial.begin(9600);

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  Blynk.begin(auth, ssid, pass);

  lcd.init();
  lcd.backlight();

  EEPROM.begin(32);
  pinMode(resetButtonPin, INPUT_PULLUP);

  readEnergyDataFromEEPROM();

  // Sensor pin setup
  emon.voltage(35, vCalibration, 1.7); // Voltage pin, calibration, phase shift
  emon.current(34, currCalibration);   // Current pin and calibration

  // Timers
  timer.setInterval(2000L, sendEnergyDataToBlynk);
  timer.setInterval(2000L, changeDisplayPage);
  timer.setInterval(60000L, sendBillToTelegram); // Every 60 seconds
}

// ------------------------ LOOP ------------------------ //
void loop() {
  Blynk.run();
  timer.run();

  // Manual reset button (hold button to reset)
  if (digitalRead(resetButtonPin) == LOW) {
    resetEEPROM();
  }
}

// ------------------------ BLYNK SENDER ------------------------ //
void sendEnergyDataToBlynk() {
  emon.calcVI(20, 2000); // Read voltage and current

  float voltage = emon.Vrms;
  float current = emon.Irms;
  float power = emon.apparentPower;

  float kW = power / 1000.0;
  unsigned long currentMillis = millis();
  float hours = (currentMillis - lastMillis) / 3600000.0;
  kWh += kW * hours;
  cost = kWh * ratePerkWh;
  lastMillis = currentMillis;

  // Send data to Blynk
  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, kWh);
  Blynk.virtualWrite(V4, cost);

  // Optional serial output
  Serial.println("Voltage: " + String(voltage) + " V");
  Serial.println("Current: " + String(current) + " A");
  Serial.println("Power: " + String(power) + " W");
  Serial.println("Energy: " + String(kWh, 3) + " kWh");
  Serial.println("Cost: ₹" + String(cost, 2));

  saveEnergyDataToEEPROM();
}

// ------------------------ DISPLAY PAGE ROTATION ------------------------ //
void changeDisplayPage() {
  lcd.clear();
  emon.calcVI(20, 2000); // Refresh sensor data

  switch (displayPage) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Voltage: ");
      lcd.print(emon.Vrms, 1);
      lcd.print("V");
      lcd.setCursor(0, 1);
      lcd.print("Current: ");
      lcd.print(emon.Irms, 1);
      lcd.print("A");
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Power: ");
      lcd.print(emon.apparentPower, 1);
      lcd.print(" W");
      lcd.setCursor(0, 1);
      lcd.print("Energy: ");
      lcd.print(kWh, 2);
      lcd.print(" kWh");
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Cost: ₹");
      lcd.print(cost, 2);
      lcd.setCursor(0, 1);
      lcd.print("Smart Energy Meter");
      break;
  }

  displayPage = (displayPage + 1) % 3;
}

// ------------------------ BILL SUMMARY ------------------------ //
void sendBillToTelegram() {
  String message = "📊 ENERGY BILL SUMMARY\n";
  message += "Energy Used: " + String(kWh, 2) + " kWh\n";
  message += "Total Cost: ₹" + String(cost, 2);

  // Send message to Blynk Terminal (V5)
  Blynk.virtualWrite(V5, message);

  // You can also add Telegram API integration here if needed
}

// ------------------------ EEPROM SAVE/LOAD ------------------------ //
void saveEnergyDataToEEPROM() {
  EEPROM.put(addrKWh, kWh);
  EEPROM.put(addrCost, cost);
  EEPROM.commit();
}

void readEnergyDataFromEEPROM() {
  EEPROM.get(addrKWh, kWh);
  EEPROM.get(addrCost, cost);

  if (isnan(kWh)) kWh = 0.0;
  if (isnan(cost)) cost = 0.0;
}

// ------------------------ EEPROM RESET ------------------------ //
void resetEEPROM() {
  kWh = 0.0;
  cost = 0.0;
  saveEnergyDataToEEPROM();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EEPROM Reset Done");
  delay(2000);
}
