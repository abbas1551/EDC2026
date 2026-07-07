#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* ssid = "Wifi SSID";
const char* password = "Password";
const char* serverBaseURL = "please check your email";

// The Data ID this device wants to request from the server (max 4 digits)
const int dataId = 1;

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

    // Build the URL with the query parameter, e.g. .../api/esp32?id=1
    String url = String(serverBaseURL) + "?id=" + String(dataId);

    if (http.begin(client, url)) {
      Serial.println("Requesting:");
      Serial.println(url);

      int httpResponseCode = http.GET();
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Server Response:");
        Serial.println(response);

        if (httpResponseCode == 200) {
          
          // Parse the JSON response manually without ArduinoJson
          String receivedDataId = extractJsonValue(response, "Data ID");
          String receivedDataType = extractJsonValue(response, "Data Type");
          String receivedData = extractJsonValue(response, "Data");

          // Check if we successfully extracted at least one expected key
          if (receivedDataId != "" || receivedDataType != "" || receivedData != "") {
            Serial.println("Parsed values:");
            Serial.print("Data ID: ");
            Serial.println(receivedDataId);
            Serial.print("Data Type: ");
            Serial.println(receivedDataType);
            Serial.print("Data: ");
            Serial.println(receivedData);

            // TODO: use receivedData / receivedDataType as needed in your logic
          } else {
            Serial.println("JSON parse failed: Could not find target keys in the response.");
          }
          
        } else if (httpResponseCode == 404) {
          Serial.println("No data found for this Data ID.");
        } else {
          Serial.println("Unexpected response code.");
        }
      } else {
        Serial.print("GET failed: ");
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
