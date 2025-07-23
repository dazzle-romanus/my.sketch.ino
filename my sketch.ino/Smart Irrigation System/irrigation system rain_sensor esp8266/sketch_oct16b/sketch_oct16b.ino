// Include the necessary libraries
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Blynk authentication details

char auth[] = "235QwLEw79ZySGElCbWP4cmmAzdHdGk-";  // Enter your Blynk Auth token
char ssid[] = "dazzle";  // Enter your WIFI SSID
char pass[] = "romanus5656";  // Enter your WIFI Password

BlynkTimer timer;

// Define the rain sensor pin
#define rainSensor A0  // A0 Rain Sensor
int rain_ToggleValue;

void setup() {
  Serial.begin(9600);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // Timer to check rain sensor status
  timer.setInterval(1000L, rainSensorReading);  // Read rain sensor every second
}

// Function to read rain sensor values and send them to Blynk
void rainSensorReading() {
  int rainValue = analogRead(rainSensor);
  rainValue = map(rainValue, 0, 1024, 0, 100);  // Adjust based on rain sensor calibration

  Blynk.virtualWrite(V4, rainValue);  // Display rain sensor data on a gauge (V4)
  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);  // Print the rain sensor value to the serial monitor

  // If rain is detected (value above threshold), send notification and turn on LED in Blynk
  if (rainValue > 50) {  // Adjust threshold based on calibration
    Blynk.logEvent("rain", "WARNING! Rain Detected!");  // Send notification to Blynk
    WidgetLED LED(V5);
    LED.on();  // Turn on LED in Blynk app
  } else {
    WidgetLED LED(V5);
    LED.off(); // Turn off LED in Blynk app
  }
}

// Virtual pin to control rain sensor functionality
BLYNK_WRITE(V6) {
  rain_ToggleValue = param.asInt();  
}

void loop() {
  if (rain_ToggleValue == 1) {
    rainSensorReading();  // Run rain sensor reading
  } else {
    WidgetLED LED(V5);
    LED.off();  // Ensure LED is off when the rain sensor is disabled
  }

  Blynk.run();  // Run the Blynk library
  timer.run();  // Run the Blynk timer
}
