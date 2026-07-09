#include <Wire.h>

const int MPU_ADDR = 0x68; 
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); 
  
  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  
  Wire.write(0);     
  Wire.endTransmission(true);

  // Print CSV Header once on startup
  Serial.println("timestamp,ax,ay,az,gx,gy,gz");
}

void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); 
  Wire.endTransmission(false); 
  
  Wire.requestFrom(MPU_ADDR, 14, true);
  
  if (Wire.available() == 14) {
    AcX = (Wire.read() << 8 | Wire.read()); 
    AcY = (Wire.read() << 8 | Wire.read()); 
    AcZ = (Wire.read() << 8 | Wire.read()); 
    Tmp = (Wire.read() << 8 | Wire.read()); // Read but ignore for ML
    GyX = (Wire.read() << 8 | Wire.read()); 
    GyY = (Wire.read() << 8 | Wire.read()); 
    GyZ = (Wire.read() << 8 | Wire.read()); 
    
    // Scale Accelerometer to m/s^2 (Assuming +/- 2g default)
    float ax = (AcX / 16384.0) * 9.81;
    float ay = (AcY / 16384.0) * 9.81;
    float az = (AcZ / 16384.0) * 9.81;

    // Scale Gyroscope to degrees/sec (Assuming +/- 250 deg/s default)
    float gx = GyX / 131.0;
    float gy = GyY / 131.0;
    float gz = GyZ / 131.0;

    // Print as Comma Separated Values
    Serial.print(millis()); Serial.print(",");
    Serial.print(ax, 2); Serial.print(",");
    Serial.print(ay, 2); Serial.print(",");
    Serial.print(az, 2); Serial.print(",");
    Serial.print(gx, 2); Serial.print(",");
    Serial.print(gy, 2); Serial.print(",");
    Serial.println(gz, 2);
  }
  
  // Delay 20ms = ~50Hz sampling rate (great for gesture ML)
  delay(20); 
}
