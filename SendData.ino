#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* ssid = "GUEST-N";
const char* password = "june@2026";
const char* serverURL = "https://workshop.veritasprism.com/api/esp32";

// Customize these as needed for your device
const char* groupName = "Arduinophile";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    // For testing only.
    // Replace with a CA certificate for production.
    client.setInsecure();

    HTTPClient http;
    if (http.begin(client, serverURL)) {
      http.addHeader("Content-Type", "application/json");

      // Get this device's MAC address
      String macAddress = WiFi.macAddress();

      // Example location data — replace with real sensor readings if needed
      float latitude = 28.6139;
      float longitude = 77.2090;

      String message = "Lat: " + String(latitude, 4) + ", Lon: " + String(longitude, 4);

      // Build the JSON payload matching the server's expected format
      String payload = "{";
      payload += "\"MAC\":\"" + macAddress + "\",";
      payload += "\"Group Name\":\"" + String(groupName) + "\",";
      payload += "\"Message\":\"" + message + "\",";
      payload += "\"Data Type\":\"N/A\",";
      payload += "\"Data\":\"N/A\"";
      payload += "}";

      Serial.println("Sending JSON:");
      Serial.println(payload);

      int httpResponseCode = http.POST(payload);

      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Server Response:");
        Serial.println(response);
      } else {
        Serial.print("POST failed: ");
        Serial.println(http.errorToString(httpResponseCode));
      }

      http.end();
    } else {
      Serial.println("Unable to connect to server.");
    }
  } else {
    Serial.println("WiFi disconnected.");
  }

  delay(10000);
}
