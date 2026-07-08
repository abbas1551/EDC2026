#include <Wire.h>

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup() {
  Serial.begin(115200);
  
  // 1. Start I2C explicitly on ESP32 pins 21 (SDA) and 22 (SCL)
  Wire.begin(21, 22); 
  
  // 2. Wake up the MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // Target the PWR_MGMT_1 register
  Wire.write(0);     // Write 0 to wake it up
  byte error = Wire.endTransmission(true);
  
  if (error == 0) {
    Serial.println("Successfully woke up MPU6050!");
  } else {
    Serial.print("Error communicating during wake-up. Code: ");
    Serial.println(error);
  }
}

void loop() {
  // 3. Point to the starting register for data (ACCEL_XOUT_H)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); 
  Wire.endTransmission(false); // 'false' keeps the connection active
  
  // 4. Request 14 consecutive bytes from the sensor
  Wire.requestFrom(MPU_ADDR, 14, true);
  
  // 5. Check if we actually got the 14 bytes back
if (Wire.available() == 14) {
    // Read raw data
    AcX = (Wire.read() << 8 | Wire.read()); 
    AcY = (Wire.read() << 8 | Wire.read()); 
    AcZ = (Wire.read() << 8 | Wire.read()); 
    Tmp = (Wire.read() << 8 | Wire.read()); 
    GyX = (Wire.read() << 8 | Wire.read()); 
    GyY = (Wire.read() << 8 | Wire.read()); 
    GyZ = (Wire.read() << 8 | Wire.read()); 
    
    // Scale Accelerometer to m/s^2
    // Formula: (Raw / 16384.0) * 9.81
    float ms2X = (AcX / 16384.0) * 9.81;
    float ms2Y = (AcY / 16384.0) * 9.81;
    float ms2Z = (AcZ / 16384.0) * 9.81;

    // Print the scaled values in m/s^2
    Serial.print("Accel (m/s^2): X = "); Serial.print(ms2X, 2);
    Serial.print(" | Y = "); Serial.print(ms2Y, 2);
    Serial.print(" | Z = "); Serial.println(ms2Z, 2);
    
    // Print the Temperature 
    Serial.print("Temp:          ");
    Serial.print((Tmp / 340.00) + 36.53, 1); Serial.println(" C");
    
    Serial.println("-----------------------------------------");
  }else {
    Serial.println("Error: Did not receive 14 bytes from MPU6050");
  }
  
  delay(1000); // Wait 1 second before reading again
}
