#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C  // Change to 0x3D if your display uses that address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // Initialize I2C
  Wire.begin();

  // Optional: Faster I2C communication
  // Wire.setClock(400000);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  // Clear the display
  display.clearDisplay();

  // Set text properties
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  // Position the cursor
  display.setCursor(10, 25);

  // Print text
  display.println("Hello!");
  // Or use: display.println("Hello, World!");

  // Update the display
  display.display();
}

void loop() {
  // Nothing to do
}
