#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C 
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -----------------------------------------------------------
// 3D GEOMETRY DEFINITIONS
// -----------------------------------------------------------

// The 6 faces of a cube. Each face has 4 vertices (X, Y, Z).
// Defined as normalized coordinates (-1 to 1).
const float cube_faces[6][4][3] = {
  {{-1, 1, -1}, {1, 1, -1}, {1, 1, 1}, {-1, 1, 1}},    // Top
  {{-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1}}, // Bottom
  {{1, -1, -1}, {1, 1, -1}, {1, 1, 1}, {1, -1, 1}},    // Right
  {{-1, -1, -1}, {-1, 1, -1}, {-1, 1, 1}, {-1, -1, 1}}, // Left
  {{-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}},    // Front
  {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}}  // Back
};

// The directional normal for each face (determines how it "explodes" outward)
const float face_normals[6][3] = {
  {0, 1, 0}, {0, -1, 0}, {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1}
};

// The hidden object inside: An Octahedron (Diamond)
const float octa_vertices[6][3] = {
  {0, 1, 0}, {0, -1, 0}, {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1}
};
const int octa_edges[12][2] = {
  {0,2}, {0,3}, {0,4}, {0,5}, {1,2}, {1,3}, {1,4}, {1,5}, {2,4}, {4,3}, {3,5}, {5,2}
};

// -----------------------------------------------------------

void setup() {
  Wire.begin();
  Wire.setClock(1000000); // 1MHz I2C for ultra-smooth framerate

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;); 
  }
}

void loop() {
  display.clearDisplay();
  
  // Time variable to drive the perfect loop
  float t = millis() / 1000.0; 
  
  // Oscillate expansion from 0.0 (closed) to 15.0 (fully open) using a Sine wave
  float expand_dist = (sin(t * 1.5) + 1.0) * 8.0; 
  
  // Global rotation angles for the whole scene
  float angleX = t * 0.5;
  float angleY = t * 0.7;
  float angleZ = t * 0.3;

  // Pre-calculate Sines and Cosines for global rotation to save CPU time
  float sX = sin(angleX), cX = cos(angleX);
  float sY = sin(angleY), cY = cos(angleY);
  float sZ = sin(angleZ), cZ = cos(angleZ);

  // Inner Diamond rotation angles (spins faster than the cube!)
  float inAngleX = t * 1.2;
  float inAngleY = t * -1.5;
  float inAngleZ = t * 0.8;
  float in_sX = sin(inAngleX), in_cX = cos(inAngleX);
  float in_sY = sin(inAngleY), in_cY = cos(inAngleY);
  float in_sZ = sin(inAngleZ), in_cZ = cos(inAngleZ);

  // --- RENDER THE INNER DIAMOND ---
  int proj_octa[6][2];
  for (int i = 0; i < 6; i++) {
    float x = octa_vertices[i][0] * 6.0; // Size of diamond
    float y = octa_vertices[i][1] * 6.0;
    float z = octa_vertices[i][2] * 6.0;
    
    // Rotate Inner Object
    float x1 = x*in_cY - z*in_sY; float z1 = x*in_sY + z*in_cY;
    float y2 = y*in_cX - z1*in_sX; float z2 = y*in_sX + z1*in_cX;
    float x3 = x1*in_cZ - y2*in_sZ; float y3 = x1*in_sZ + y2*in_cZ;
    
    // Project Inner Object
    float z_offset = 50.0 + z2;
    proj_octa[i][0] = (int)((x3 * 80.0) / z_offset) + (SCREEN_WIDTH / 2);
    proj_octa[i][1] = (int)((y3 * 80.0) / z_offset) + (SCREEN_HEIGHT / 2);
  }
  
  // Draw Diamond Edges (only if the cube is open enough to see it!)
  if (expand_dist > 2.0) {
    for (int i = 0; i < 12; i++) {
      display.drawLine(proj_octa[octa_edges[i][0]][0], proj_octa[octa_edges[i][0]][1],
                       proj_octa[octa_edges[i][1]][0], proj_octa[octa_edges[i][1]][1], SSD1306_WHITE);
    }
  }

  // --- RENDER THE EXPLODING CUBE ---
  for (int f = 0; f < 6; f++) {
    int proj_face[4][2];
    
    // Calculate expansion offset for this specific face
    float offX = face_normals[f][0] * expand_dist;
    float offY = face_normals[f][1] * expand_dist;
    float offZ = face_normals[f][2] * expand_dist;

    for (int v = 0; v < 4; v++) {
      // Scale base cube by 12, then add the expansion offset
      float x = (cube_faces[f][v][0] * 12.0) + offX;
      float y = (cube_faces[f][v][1] * 12.0) + offY;
      float z = (cube_faces[f][v][2] * 12.0) + offZ;
      
      // Apply Global Rotation
      float x1 = x*cY - z*sY; float z1 = x*sY + z*cY;
      float y2 = y*cX - z1*sX; float z2 = y*sX + z1*cX;
      float x3 = x1*cZ - y2*sZ; float y3 = x1*sZ + y2*cZ;
      
      // Perspective projection
      float z_offset = 50.0 + z2;
      proj_face[v][0] = (int)((x3 * 80.0) / z_offset) + (SCREEN_WIDTH / 2);
      proj_face[v][1] = (int)((y3 * 80.0) / z_offset) + (SCREEN_HEIGHT / 2);
    }
    
    // Draw the 4 edges of this specific face
    for (int i = 0; i < 4; i++) {
      int next_i = (i + 1) % 4;
      display.drawLine(proj_face[i][0], proj_face[i][1],
                       proj_face[next_i][0], proj_face[next_i][1], SSD1306_WHITE);
    }
  }

  display.display();
}
