#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C    // change to 0x3D if your panel needs it

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---- Donut geometry ----
const float R1 = 1.0f;     // tube radius
const float R2 = 2.0f;     // ring radius
const float K2 = 5.0f;     // camera distance
const float K1 = 44.0f;    // zoom (raise toward ~50 to fill more screen)

// ---- Surface sampling (smaller steps = denser, no gaps, but slower) ----
const float THETA_STEP = 0.07f;   // around the tube
const float PHI_STEP   = 0.02f;   // around the ring
#define NUM_THETA 90
#define NUM_PHI   315

// ---- Lookup tables (filled once) so the render loop never calls sin/cos ----
float tCos[NUM_THETA], tSin[NUM_THETA];
float ringX[NUM_THETA];           // R2 + R1*cos(theta)
float ringY[NUM_THETA];           // R1*sin(theta)
float pCos[NUM_PHI],  pSin[NUM_PHI];

// ---- Per-pixel depth buffer (stores 1/z; bigger = closer) ----
float zbuf[SCREEN_WIDTH * SCREEN_HEIGHT];

// ---- 4x4 Bayer matrix: dithers brightness into 1 bit to fake grayscale ----
const uint8_t bayer[4][4] = {
  {  0,  8,  2, 10 },
  { 12,  4, 14,  6 },
  {  3, 11,  1,  9 },
  { 15,  7, 13,  5 }
};

unsigned long lastFpsMs = 0;
int frames = 0, fps = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);          // 400 kHz I2C -> much higher frame rate

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  for (int i = 0; i < NUM_THETA; i++) {
    float th = i * THETA_STEP;
    tCos[i]  = cosf(th);
    tSin[i]  = sinf(th);
    ringX[i] = R2 + R1 * tCos[i];
    ringY[i] = R1 * tSin[i];
  }
  for (int j = 0; j < NUM_PHI; j++) {
    float ph = j * PHI_STEP;
    pCos[j] = cosf(ph);
    pSin[j] = sinf(ph);
  }

  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  // Time-based rotation -> constant speed regardless of FPS
  float t  = millis() * 0.001f;
  float A  = t * 0.9f;            // pitch
  float B  = t * 0.5f;            // yaw
  float cA = cosf(A), sA = sinf(A);
  float cB = cosf(B), sB = sinf(B);

  display.clearDisplay();
  memset(zbuf, 0, sizeof(zbuf));
  uint8_t *buf = display.getBuffer();

  for (int i = 0; i < NUM_THETA; i++) {
    float rx = ringX[i], ry = ringY[i];
    float ct = tCos[i],  st = tSin[i];

    for (int j = 0; j < NUM_PHI; j++) {
      float cp = pCos[j], sp = pSin[j];

      // 3D position of this surface point (rotation baked in)
      float x   = rx * (cB * cp + sA * sB * sp) - ry * cA * sB;
      float y   = rx * (sB * cp - sA * cB * sp) + ry * cA * cB;
      float z   = K2 + cA * rx * sp + ry * sA;
      float ooz = 1.0f / z;        // depth

      int xp = (int)(SCREEN_WIDTH  * 0.5f + K1 * ooz * x);
      int yp = (int)(SCREEN_HEIGHT * 0.5f - K1 * ooz * y);
      if ((unsigned)xp >= SCREEN_WIDTH || (unsigned)yp >= SCREEN_HEIGHT) continue;

      // Luminance = surface normal . light direction
      float L = cp * ct * sB - cA * ct * sp - sA * st
              + cB * (cA * st - ct * sA * sp);
      if (L <= 0.0f) continue;     // facing away from the light

      int idx = xp + yp * SCREEN_WIDTH;
      if (ooz > zbuf[idx]) {       // closer than whatever was here?
        zbuf[idx] = ooz;

        float bright = L * 0.707f;                 // map ~0..1.41 -> 0..1
        if (bright > 1.0f) bright = 1.0f;
        float thresh = (bayer[yp & 3][xp & 3] + 0.5f) * (1.0f / 16.0f);

        int bit  = yp & 7;
        int bidx = xp + (yp >> 3) * SCREEN_WIDTH;
        if (bright > thresh) buf[bidx] |=  (1 << bit);   // lit  -> pixel on
        else                 buf[bidx] &= ~(1 << bit);   // dark -> pixel off
      }
    }
  }

  // Tiny FPS read-out, drawn on top (white glyphs only)
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(fps);
  display.print(F(" fps"));

  display.display();

  frames++;
  unsigned long now = millis();
  if (now - lastFpsMs >= 1000) {
    fps = frames; frames = 0; lastFpsMs = now;
    Serial.print("FPS: "); Serial.println(fps);
  }
}
