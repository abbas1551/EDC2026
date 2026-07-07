#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C // Change to 0x3D if necessary

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long previousMillis = 0;
int frameCount = 0;
int fps = 0;

void setup() {
  Serial.begin(115200); // Note: Set your serial monitor to 115200 baud

  // Initialize I2C
  Wire.begin();
  
  // UNCOMMENT THE LINE BELOW TO UNLOCK HIGHER FPS!
  // Wire.setClock(400000); 

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  unsigned long currentMillis = millis();

  // Every 1000 milliseconds (1 second), calculate the FPS
  if (currentMillis - previousMillis >= 1000) {
    fps = frameCount;
    frameCount = 0;
    previousMillis = currentMillis;
    
    Serial.print("Refresh Rate: ");
    Serial.print(fps);
    Serial.println(" FPS");
  }

  // 1. Clear the buffer
  display.clearDisplay();

  // 2. Draw the FPS text
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.print("FPS: ");
  display.print(fps);

  // 3. Draw a fast-moving element to visually confirm the speed
  int movingX = (currentMillis / 5) % SCREEN_WIDTH;
  display.fillRect(movingX, 40, 10, 10, SSD1306_WHITE);

  // 4. Send the buffer to the screen
  display.display();
  
  // 5. Log that a frame was drawn
  frameCount++;
}
