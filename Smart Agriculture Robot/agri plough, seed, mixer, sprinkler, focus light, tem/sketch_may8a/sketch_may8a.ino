





You said:
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <DHT.h>

// Blynk Auth Token
char auth[] = "kuLOtQHP1JOalR94soOrt7Z6vw-yHG8U";                          //kuLOtQHP1JOalR94soOrt7Z6vw-yHG8U

// WiFi credentials
char ssid[] = "dazzle";
char pass[] = "romanus5666";

// Motor Pins for L298N Motor Driver
#define IN1 D1  // Motor 1 control
#define IN2 D2
#define IN3 D3  // Motor 2 control
#define IN4 D4
#define ENA D5  // Motor 1 speed (optional)
#define ENB D6  // Motor 2 speed (optional)

// Servo Pin
#define SERVO_PIN D7

// DHT11 Sensor Pin
#define DHTPIN D8  // DHT11 data pin connected to D8
#define DHTTYPE DHT11  // DHT11 sensor type

// Relay Pin for water pump
#define RELAY_PIN D0  // Relay control pin

Servo servoMotor;
DHT dht(DHTPIN, DHTTYPE);

// Variables for motor, servo, and sensor
bool motor1State = false;
bool motor2State = false;
bool servoMotorState = false;
bool waterPumpState = false;
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

// Blynk button to control Servo motor (on/off)
BLYNK_WRITE(V3) {
  servoMotorState = param.asInt();  // On/Off for Servo motor
}

// Blynk slider to adjust Servo motor speed
BLYNK_WRITE(V4) {
  servoSpeed = param.asInt();  // Speed adjustment in milliseconds
}

// Blynk button to control Water Pump (Relay on/off)
BLYNK_WRITE(V5) {
  waterPumpState = param.asInt();  // On/Off for Water Pump
  digitalWrite(RELAY_PIN, waterPumpState ? LOW : HIGH);  // Relay is active LOW
}

void setup() {
  // Start serial communication
  Serial.begin(115200);
  
  // Setup Blynk
  Blynk.begin(auth, ssid, pass);
  
  // Motor Pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Attach servo
  servoMotor.attach(SERVO_PIN);

  // Initialize DHT11 sensor
  dht.begin();

  // Relay pin setup
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Ensure relay is off initially
}

void loop() {
  Blynk.run();

  // If servo motor is on, move between 0° and 120°
  if (servoMotorState) {
    for (int angle = 0; angle <= 120; angle++) {
      servoMotor.write(angle);
      delay(servoSpeed);  // Adjust speed using the slider
    }
    for (int angle = 120; angle >= 0; angle--) {
      servoMotor.write(angle);
      delay(servoSpeed);  // Adjust speed using the slider
    }
  }

  // Read temperature and humidity from DHT11
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Send temperature and humidity to Blynk
  Blynk.virtualWrite(V6, temperature);  // Temperature display on Blynk
  Blynk.virtualWrite(V7, humidity);     // Humidity display on Blynk
}  