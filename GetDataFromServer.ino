#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- WiFi & API Configuration ---
const char* ssid = "Wifi";
const char* password = "Password";
const char* serverBaseURL = "sent over email";
const int dataId = 1;

// --- OLED Configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C // Change to 0x3D if your display uses that address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Helper function to extract a value from a simple JSON string
String extractJsonValue(String json, String key) {
  String searchKey = "\"" + key + "\"";
  int keyIndex = json.indexOf(searchKey);
  if (keyIndex == -1) return ""; // Key not found

  // Find the colon separating key and value
  int colonIndex = json.indexOf(':', keyIndex + searchKey.length());
  if (colonIndex == -1) return "";

  // Skip any whitespace after the colon
  int valStart = colonIndex + 1;
  while (valStart < json.length() && (json[valStart] == ' ' || json[valStart] == '\t' || json[valStart] == '\n' || json[valStart] == '\r')) {
    valStart++;
  }

  // Check if the value is a string (enclosed in quotes)
  if (json[valStart] == '\"') {
    int endQuote = json.indexOf('\"', valStart + 1);
    if (endQuote != -1) {
      return json.substring(valStart + 1, endQuote);
    }
  } else {
    // If it's a number, boolean, or null (not enclosed in quotes)
    int endComma = json.indexOf(',', valStart);
    int endBrace = json.indexOf('}', valStart);
    int valEnd = json.length();

    if (endComma != -1 && endBrace != -1) {
      valEnd = min(endComma, endBrace);
    } else if (endComma != -1) {
      valEnd = endComma;
    } else if (endBrace != -1) {
      valEnd = endBrace;
    }

    String value = json.substring(valStart, valEnd);
    value.trim(); // Remove any trailing whitespace
    return value;
  }
  return "";
}

// Helper function to update the OLED screen with messages
void updateDisplayMessage(String msg1, String msg2 = "") {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(msg1);
  if (msg2 != "") {
    display.println(msg2);
  }
  display.display();
}

void setup() {
  Serial.begin(115200);

  // --- Initialize OLED ---
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true); // Loop forever if OLED init fails
  }
  
  display.setTextColor(SSD1306_WHITE);
  updateDisplayMessage("Starting ESP32...", "Connecting WiFi");

  // --- Initialize WiFi ---
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi");
  updateDisplayMessage("WiFi Connected!", WiFi.localIP().toString());
  delay(2000); // Pause so the user can see the IP address
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // For testing only

    HTTPClient http;
    String url = String(serverBaseURL) + "?id=" + String(dataId);

    if (http.begin(client, url)) {
      Serial.println("Requesting: " + url);
      updateDisplayMessage("Fetching data...");

      int httpResponseCode = http.GET();
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);

      if (httpResponseCode > 0) {
        String response = http.getString();
        
        if (httpResponseCode == 200) {
          // Parse JSON manually
          String receivedDataId = extractJsonValue(response, "Data ID");
          String receivedDataType = extractJsonValue(response, "Data Type");
          String receivedData = extractJsonValue(response, "Data");

          if (receivedDataId != "" || receivedDataType != "" || receivedData != "") {
            // Update OLED with parsed data
            display.clearDisplay();
            display.setCursor(0, 0);
            
            // Print ID & Type in smaller text
            display.setTextSize(1);
            display.print("ID: "); display.println(receivedDataId);
            display.print("Type: "); display.println(receivedDataType);
            display.println("---");
            
            // Print actual Data slightly larger if you prefer (set to 2) 
            // Sticking to 1 ensures long data strings don't run off-screen easily
            display.setTextSize(1); 
            display.print("Data: ");
            display.println(receivedData);
            
            display.display();
            
            // Also print to Serial
            Serial.println("Data ID: " + receivedDataId);
            Serial.println("Data Type: " + receivedDataType);
            Serial.println("Data: " + receivedData);
          } else {
            updateDisplayMessage("Parse Error", "Keys not found");
          }
        } else if (httpResponseCode == 404) {
          updateDisplayMessage("Error 404", "ID not found");
        } else {
          updateDisplayMessage("HTTP Error", String(httpResponseCode));
        }
      } else {
        updateDisplayMessage("GET failed", http.errorToString(httpResponseCode));
      }
      http.end();
    } else {
      updateDisplayMessage("Connection Failed");
    }
  } else {
    updateDisplayMessage("WiFi Disconnected");
  }

  // Wait 10 seconds before requesting again
  delay(10000);
}
