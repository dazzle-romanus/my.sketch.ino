#define alcoholSensor A0  // Pin for the alcohol sensor
#define relayPin 9        // Pin for the relay module connected to the motor
#define buzzer 8          // Pin for the buzzer
#define led 7             // Pin for the LED

void setup() {
  pinMode(alcoholSensor, INPUT);  // Alcohol sensor input
  pinMode(relayPin, OUTPUT);      // Relay module output
  pinMode(buzzer, OUTPUT);        // Buzzer output
  pinMode(led, OUTPUT);           // LED output
  Serial.begin(9600);             // Initialize serial communication
}

void loop() {
  int alcoholLevel = analogRead(alcoholSensor);  // Read alcohol sensor value

  Serial.print("Alcohol sensor value: ");
  Serial.println(alcoholLevel);

  if (alcoholLevel > 930) {  // Adjust threshold based on your sensor calibration
    // If alcohol is detected, stop the motor, turn on buzzer and LED
    digitalWrite(relayPin, LOW);  // Turn off the motor (Relay module LOW)
    digitalWrite(buzzer, HIGH);   // Turn on the buzzer
    digitalWrite(led, HIGH);      // Turn on the LED
  } else {
    // If no alcohol is detected, keep motor running, buzzer and LED off
    digitalWrite(relayPin, HIGH);  // Turn on the motor (Relay module HIGH)
    digitalWrite(buzzer, LOW);     // Turn off the buzzer
    digitalWrite(led, LOW);        // Turn off the LED
  }

  delay(500);  // Small delay for sensor stability
}
