#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

// Blynk Auth Token
char auth[] = "Y4qfI1OLIAxIT2bzhZKBus0EUbNdsFCO";

// WiFi credentials
char ssid[] = "dazzle";
char pass[] = "romanus5666";

// Motor Pins for L298N Motor Driver
#define IN1 D1  // Motor 1 control
#define IN2 D2
#define IN3 D3  // Motor 2 control
#define IN4 D4
#define ENA D5  // Motor 1 speed (optional, if needed)
#define ENB D6  // Motor 2 speed (optional, if needed)

// Servo Pin
#define SERVO_PIN D7

Servo servoMotor;

// Variables for motor and servo control
bool motor1State = false;
bool motor2State = false;
bool servoMotorState = false;
int servoSpeed = 00.00000000000001;  // Default speed (in milliseconds)

// Blynk button to control DC Motor 1 (on/off)
BLYNK_WRITE(V1) {
  motor1State = param.asInt();  // On/Off for Motor 1
  if (motor1State) {
    // Turn on Motor 1 (forward)
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 255);  // Full speed for Motor 1
  } else {
    // Turn off Motor 1
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}

// Blynk button to control DC Motor 2 (on/off)
BLYNK_WRITE(V2) {
  motor2State = param.asInt();  // On/Off for Motor 2
  if (motor2State) {
    // Turn on Motor 2 (forward)
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 255);  // Full speed for Motor 2
  } else {
    // Turn off Motor 2
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
}

void loop() {
  Blynk.run();

  // If servo motor is on, move between 0° and 50°
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
}
