#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

#define BUZZER_PIN     2
#define LED_B_PIN      3
#define LED_L_PIN      4
#define LED_D_PIN      5
#define RELAY_B_PIN    6
#define RELAY_L_PIN    7
#define RELAY_D_PIN    8
#define SENSOR_PIN     9

// SoftwareSerial: RX = pin 10, TX = pin 11
SoftwareSerial espSerial(10, 11);  // RX, TX

RTC_DS3231 rtc;

bool breakfast_done = false;
bool lunch_done = false;
bool dinner_done = false;
bool lastSensorState = LOW;

DateTime now;

struct TimeWindow {
  int start_hour;
  int start_minute;
  int end_hour;
  int end_minute;
  bool* done_flag;
  int led_pin;
  int relay_pin;
  const char* meal_name;
};

TimeWindow meals[] = {
  {8, 30, 9, 30, &breakfast_done, LED_B_PIN, RELAY_B_PIN, "Breakfast"},
  {13, 30, 14, 30, &lunch_done, LED_L_PIN, RELAY_L_PIN, "Lunch"},
  {19, 30, 20, 30, &dinner_done, LED_D_PIN, RELAY_D_PIN, "Dinner"}
};

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);  // Communication with ESP32

  Wire.begin(); // I2C for RTC
  rtc.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_B_PIN, OUTPUT);
  pinMode(LED_L_PIN, OUTPUT);
  pinMode(LED_D_PIN, OUTPUT);
  pinMode(RELAY_B_PIN, OUTPUT);
  pinMode(RELAY_L_PIN, OUTPUT);
  pinMode(RELAY_D_PIN, OUTPUT);

  // Ensure relays are OFF initially
  digitalWrite(RELAY_B_PIN, HIGH);
  digitalWrite(RELAY_L_PIN, HIGH);
  digitalWrite(RELAY_D_PIN, HIGH);
}

void loop() {
  now = rtc.now();

  Serial.print("Current Time: ");
  Serial.print(now.year()); Serial.print('/');
  Serial.print(now.month()); Serial.print('/');
  Serial.print(now.day()); Serial.print(" ");
  Serial.print(now.hour()); Serial.print(':');
  Serial.print(now.minute()); Serial.print(':');
  Serial.println(now.second());

  for (int i = 0; i < 3; i++) {
    checkMealWindow(meals[i]);
  }

  delay(1000); // Stability
}

void checkMealWindow(TimeWindow meal) {
  DateTime now = rtc.now();
  bool sensorState = digitalRead(SENSOR_PIN);

  int current_minutes = now.hour() * 60 + now.minute();
  int start_minutes = meal.start_hour * 60 + meal.start_minute;
  int end_minutes = meal.end_hour * 60 + meal.end_minute;

  if (current_minutes >= start_minutes && current_minutes <= end_minutes && !*(meal.done_flag)) {
    digitalWrite(meal.led_pin, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(meal.led_pin, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(500);

    if (sensorState == HIGH && lastSensorState == LOW) {
      triggerDispenser(meal.relay_pin);
      Serial.print("Dispensed: "); Serial.println(meal.meal_name);
      sendDataToESP32(meal.meal_name, true);
      *(meal.done_flag) = true;
    }

  } else if (current_minutes > end_minutes && !*(meal.done_flag)) {
    *(meal.done_flag) = true;
    digitalWrite(meal.led_pin, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.print("Missed: "); Serial.println(meal.meal_name);
    sendDataToESP32(meal.meal_name, false);
  }

  lastSensorState = sensorState;
}

void triggerDispenser(int relay_pin) {
  digitalWrite(relay_pin, LOW);
  delay(500);
  digitalWrite(relay_pin, HIGH);
}

void sendDataToESP32(const char* meal_name, bool success) {
  DateTime now = rtc.now();
  char buffer[64];

  if (success) {
    snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d %02d:%02d:%02d %s tablets done",
             now.day(), now.month(), now.year(),
             now.hour(), now.minute(), now.second(),
             meal_name);
  } else {
    snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d %02d:%02d:%02d Missed %s tablets",
             now.day(), now.month(), now.year(),
             now.hour(), now.minute(), now.second(),
             meal_name);
  }

  Serial.print("Sending to ESP32: ");
  Serial.println(buffer);

  espSerial.println(buffer);  // Send via SoftwareSerial
}    