#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <DHT.h>

// Blynk Auth Token
char auth[] = "kuLOtQHP1JOalR94soOrt7Z6vw-yHG8U";

// WiFi credentials
char ssid[] = "dazzle";
char pass[] = "romanus5666";

// Motor Pins for L298N Motor Driver
#define IN1 D1
#define IN2 D2
#define IN3 D3
#define IN4 D4
#define ENA D5  // Motor 1 speed (optional)
#define ENB D6  // Motor 2 speed (optional)

// Servo Pin
#define SERVO_PIN D7

// DHT11 Sensor Pin
#define DHTPIN D8
#define DHTTYPE DHT11

// Relay Pins
#define RELAY_PIN D0          // Water pump relay
#define LIGHT_RELAY_PIN D5    // D5 (valid pin on ESP8266)

Servo servoMotor;
DHT dht(DHTPIN, DHTTYPE);

// Variables for motor, servo, and sensor
bool motor1State = false;
bool motor2State = false;
bool servoMotorState = false;
bool waterPumpState = false;
bool lightRelayState = false;
int servoSpeed = 0.1;  // Default speed (in milliseconds)
float temperature, humidity;


// Blynk button to control DC Motor 1 (on/off)
BLYNK_WRITE(V1) {
  motor1State = param.asInt();  // On/Off for Motor 1
  if (motor1State) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 255);  // Full speed for Motor 1
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}

// Blynk button to control DC Motor 2 (on/off)
BLYNK_WRITE(V2) {
  motor2State = param.asInt();  // On/Off for Motor 2
  if (motor2State) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 255);  // Full speed for Motor 2
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
}

// Servo Control
BLYNK_WRITE(V3) {
  servoMotorState = param.asInt();
}

// Servo Speed
BLYNK_WRITE(V4) {
  servoSpeed = param.asInt();
}

// Water Pump Control
BLYNK_WRITE(V5) {
  waterPumpState = param.asInt();
  digitalWrite(RELAY_PIN, waterPumpState ? LOW : HIGH);  // Active LOW
}

// Night Light Control
BLYNK_WRITE(V8) {
  lightRelayState = param.asInt();
  digitalWrite(LIGHT_RELAY_PIN, lightRelayState ? LOW : HIGH);  // Active LOW
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  servoMotor.attach(SERVO_PIN);
  dht.begin();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Water pump OFF

  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  digitalWrite(LIGHT_RELAY_PIN, HIGH);  // Light OFF
}

void loop() {
  Blynk.run();

  if (servoMotorState) {
    for (int angle = 0; angle <= 120; angle++) {
      servoMotor.write(angle);
      delay(servoSpeed);
    }
    for (int angle = 120; angle >= 0; angle--) {
      servoMotor.write(angle);
      delay(servoSpeed);
    }
  }

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  Blynk.virtualWrite(V6, temperature);
  Blynk.virtualWrite(V7, humidity);
}
