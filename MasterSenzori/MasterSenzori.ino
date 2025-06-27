// ESP32 Sensor Unit: Creates a WiFi Access Point, reads sensors, and sends commands to the pump ESP32.

#include "DHT.h"       
#include <WiFi.h>      
#include <WiFiClient.h> 

// Sensor Definitions
#define DPIN 32               
#define DTYPE DHT22           
#define RAIN_SENSOR_PIN 23    
#define SOIL_SENSOR_PIN 34    

// Soil moisture sensor calibration values
const int DRY_VALUE = 4095; 
const int WET_VALUE = 1500; 

// Irrigation thresholds for soil moisture percentage
// Adjust these values based on your plant's needs.
const int MOISTURE_THRESHOLD_LOW = 40;
const int MOISTURE_THRESHOLD_HIGH = 70;

// WiFi Access Point Definitions
const char *ssid = "ESP32_Irrigation_AP";
const char *password = "supersecurepassword"; 

// Pump ESP32 (Receiver) Connection Details 
const IPAddress PUMP_SERVER_IP(192, 168, 4, 2);
const int PUMP_SERVER_PORT = 80;

// Global Objects
DHT dht(DPIN, DTYPE);
WiFiClient client;

// Variables to track state and prevent excessive commands
String currentIrrigationCommand = "";
unsigned long lastCommandSendTime = 0;
const unsigned long COMMAND_SEND_INTERVAL = 10000;

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n--- ESP32 Sensor Unit Setup ---");

  // Initialize DHT sensor
  dht.begin();

  // Configure sensor pins
  pinMode(RAIN_SENSOR_PIN, INPUT);

  // Start WiFi Access Point
  Serial.print("Configuring Access Point: ");
  Serial.println(ssid);
  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Soft AP creation failed! Restarting...");
    ESP.restart();
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Access Point IP Address: ");
  Serial.println(myIP);
  Serial.println("AP setup complete. Waiting for clients...");
}

void loop() {
  if (millis() - lastCommandSendTime < COMMAND_SEND_INTERVAL) {
    delay(500);
    return;
  }
  lastCommandSendTime = millis(); // Update the last command send time

  Serial.println("\n--- Sensor Reading Cycle ---");

  // --- Read Soil Moisture Sensor ---
  int soilSensorValue = analogRead(SOIL_SENSOR_PIN);
  int moisturePercentage = map(soilSensorValue, DRY_VALUE, WET_VALUE, 0, 100);
  moisturePercentage = constrain(moisturePercentage, 0, 100);

  Serial.print("Raw Soil Moisture Sensor Value: ");
  Serial.println(soilSensorValue);
  Serial.print("Percentage Soil Moisture: ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  // Read Rain Sensor
  int rainDetected = digitalRead(RAIN_SENSOR_PIN);
  if (rainDetected == LOW) {
    Serial.println("Rain Detected!");
  } else {
    Serial.println("No Rain Detected.");
  }

  // DHT Sensor
  float temperatureC = dht.readTemperature(false); 
  float temperatureF = dht.readTemperature(true);  
  float humidity = dht.readHumidity(); 

  // Check if any readings failed
  if (isnan(temperatureC) || isnan(temperatureF) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.print(" C (");
    Serial.print(temperatureF);
    Serial.print(" F), Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
  }

  Serial.println("---------------------------------");

  // Determine Irrigation Command
  String newCommand = "";
  if (rainDetected == HIGH && moisturePercentage < MOISTURE_THRESHOLD_LOW) {
    newCommand = "/start";
  }
  else if (rainDetected == LOW || moisturePercentage > MOISTURE_THRESHOLD_HIGH) {
    newCommand = "/stop";
  }

  // Only send a command if it's different from the last one sent, or if it's a new command
  if (newCommand.length() > 0 && newCommand != currentIrrigationCommand) {
    Serial.print("Sending command: ");
    Serial.println(newCommand);

    //Send HTTP GET Request to Pump ESP32
    if (client.connect(PUMP_SERVER_IP, PUMP_SERVER_PORT)) {
      Serial.print("Successfully connected to Pump ESP32 at ");
      Serial.println(PUMP_SERVER_IP);

      // Construct and send the HTTP GET request
      client.print("GET ");
      client.print(newCommand);
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(PUMP_SERVER_IP.toString());
      client.println("Connection: close");
      client.println();

      // Read and print the response from the pump ESP32
      unsigned long timeout = millis();
      while (client.connected() && (millis() - timeout < 1000)) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
        }
      }
      client.stop();
      Serial.println("\nDisconnected from Pump ESP32.");
      currentIrrigationCommand = newCommand;
    } else {
      Serial.print("Failed to connect to Pump ESP32 at ");
      Serial.println(PUMP_SERVER_IP);
      Serial.println("Please ensure the pump ESP32 is running and connected to this AP.");
    }
  } else if (newCommand.length() == 0) {
    Serial.println("No irrigation command needed based on current sensor data.");
  } else {
    Serial.println("Command already sent and active: " + currentIrrigationCommand);
  }
}
