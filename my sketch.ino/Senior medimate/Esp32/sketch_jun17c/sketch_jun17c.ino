#include <WiFi.h>
#include <WebServer.h>

#define RXD2 18  // ESP32 UART2 RX - connect to Arduino TX (D11)
#define TXD2 19  // ESP32 UART2 TX - not used, but defined

const char* ssid = "dazzle";
const char* pass = "romanus5666";

WebServer server(80);

String date_now = "";
String time_now = "";
String breakfast_status = "Not yet processed";
String lunch_status = "Not yet processed";
String dinner_status = "Not yet processed";

void setup() {
  Serial.begin(9600);               // USB Serial Monitor
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // UART2 for Arduino data

  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();

  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    Serial.println("Received from Arduino: " + data);
    parseData(data);
  }
}

void parseData(String data) {
  int dateEnd = data.indexOf(' ');
  int timeEnd = data.indexOf(' ', dateEnd + 1);

  if (dateEnd == -1 || timeEnd == -1) return; // invalid format

  date_now = data.substring(0, dateEnd);
  time_now = data.substring(dateEnd + 1, timeEnd);
  String message = data.substring(timeEnd + 1);

  if (message.indexOf("Breakfast") != -1) {
    breakfast_status = message;
  } else if (message.indexOf("Lunch") != -1) {
    lunch_status = message;
  } else if (message.indexOf("Dinner") != -1) {
    dinner_status = message;
  }
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Tablet Log</title><meta http-equiv='refresh' content='10'>";
  html += "<style>body{font-family:Arial;background:#f9f9f9;padding:20px;}h2{color:#2E8B57;}</style></head><body>";
  html += "<h2>Senior MediMate Tablet Log</h2>";
  html += "<p><strong>Date:</strong> " + date_now + "</p>";
  html += "<p><strong>Time:</strong> " + time_now + "</p>";
  html += "<p><strong>Breakfast:</strong> " + breakfast_status + "</p>";
  html += "<p><strong>Lunch:</strong> " + lunch_status + "</p>";
  html += "<p><strong>Dinner:</strong> " + dinner_status + "</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}
