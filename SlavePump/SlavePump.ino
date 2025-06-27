// ESP32 Pump Unit: Connects to the Sensor ESP32's AP, hosts a web server, and controls the pump/valve.

#include <WiFi.h>
#include <WebServer.h>

// WiFi Station Details (to connect to the Sensor ESP32's AP)
const char *ssid = "ESP32_Irrigation_AP";
const char *password = "supersecurepassword";

// Pump/Valve Definitions
#define PUMP_PIN 25
#define ELECTROVALVE_PIN 26

// IMPORTANT: Assuming an ACTIVE-HIGH relay module for Normally Closed (NC) contacts.
// If the pump started with HIGH, it means:
// HIGH signal from ESP32 -> Activates relay (turns device ON/opens valve)
// LOW signal from ESP32 -> Deactivates relay (turns device OFF/closes valve)

WebServer server(80);

//Variables to track pump state
bool isPumping = false;

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n--- ESP32 Pump Unit Setup ---");

  // Configure pump and electrovalve pins as outputs
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(ELECTROVALVE_PIN, OUTPUT);

  // Initially ensure both the pump and electrovalve are OFF/CLOSED.
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(ELECTROVALVE_PIN, LOW);
  Serial.println("Pump and Electrovalve initially set to OFF/CLOSED.");

  // Connect to WiFi Access Point
  Serial.print("Connecting to Access Point: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected successfully!");
  Serial.print("Assigned IP address: ");
  Serial.println(WiFi.localIP());

  //Define Web Server Handlers
  server.on("/start", handleStart);
  server.on("/stop", handleStop);
  server.onNotFound(handleNotFound);

  // Start the web server
  server.begin();
  Serial.println("HTTP Server started. Listening for commands...");
}

void loop() {
  server.handleClient();

  delay(10);
}

//Web Server Request Handlers
void handleStart() {
  Serial.println("Received '/start' command.");
  if (!isPumping) {
    digitalWrite(ELECTROVALVE_PIN, HIGH);
    delay(10);
    digitalWrite(PUMP_PIN, HIGH);
    isPumping = true;
    Serial.println("Pump ON & Electrovalve OPEN (Irrigation Started)");
    // Send a success response back to the client (Sensor ESP32)
    server.send(200, "text/plain", "Pump ON & Electrovalve OPEN - Irrigation Started");
  } else {
    Serial.println("Irrigation is already ON.");
    server.send(200, "text/plain", "Irrigation is already ON");
  }
}

// Handler for the "/stop" command
void handleStop() {
  Serial.println("Received '/stop' command.");
  if (isPumping) {
    // To turn OFF/CLOSED, send LOW signal (deactivates the relay)
    digitalWrite(PUMP_PIN, LOW);        // Turn the pump OFF
    digitalWrite(ELECTROVALVE_PIN, LOW); // Close the electrovalve
    isPumping = false;
    Serial.println("Pump OFF & Electrovalve CLOSED (Irrigation Stopped)");
    // Send a success response back to the client (Sensor ESP32)
    server.send(200, "text/plain", "Pump OFF & Electrovalve CLOSED - Irrigation Stopped");
  } else {
    Serial.println("Irrigation is already OFF.");
    server.send(200, "text/plain", "Irrigation is already OFF");
  }
}

// Handler for any requests that don't match defined paths
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) { 
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  Serial.print(message); // Print the full request details for debugging
  server.send(404, "text/plain", message); // Send a 404 Not Found response
}
