#include <Wire.h>
#include <TensorFlowLite_ESP32.h>
// Include your exported model
#include "gesture_model.h" 

// TensorFlow Lite Micro libraries
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
// Add this right below your other tensorflow includes
#include <tensorflow/lite/micro/micro_error_reporter.h>

// ==========================================
// MPU6050 GLOBALS
// ==========================================
const int MPU_ADDR = 0x68; 
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

// ==========================================
// TENSORFLOW LITE GLOBALS
// ==========================================

tflite::ErrorReporter* error_reporter = nullptr; // <--- ADD THIS LINE
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Allocate memory for the model's tensors.
constexpr int kTensorArenaSize = 10 * 1024; 
uint8_t tensor_arena[kTensorArenaSize];

// ==========================================
// GESTURE SETTINGS
// ==========================================
// IMPORTANT: Update NUM_SAMPLES to match how many timesteps your Python model uses!
// Example: If your Python script groups 50 rows per gesture, keep this at 50.
const int NUM_SAMPLES = 50; 
const int NUM_FEATURES = 6; // ax, ay, az, gx, gy, gz

// Array to map output indexes to your classes (adjust based on your Python labels)
const char* CLASSES[] = {"Circle", "Idle", "Random"};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // 1. Initialize MPU6050 via Direct I2C
  Wire.begin(21, 22); 
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // Set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.println("MPU6050 Initialized");

  // 2. Initialize TensorFlow Lite
  model = tflite::GetModel(gesture_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Pulls in all standard TFLite operations
  // Pulls in all standard TFLite operations
  static tflite::AllOpsResolver resolver;

  // Set up the error reporter
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Build the interpreter (Now with 5 arguments!)
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;
  // Allocate memory
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    while (1);
  }

  // Assign input/output pointers
  input = interpreter->input(0);
  output = interpreter->output(0);
  
  Serial.println("TensorFlow Lite Initialized. Waiting for gestures...");
}

void loop() {
  // 1. Collect a window of data
  for (int i = 0; i < NUM_SAMPLES; i++) {
    
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false); 
    
    Wire.requestFrom(MPU_ADDR, 14, true); // Request 14 registers
    
    if (Wire.available() == 14) {
      AcX = (Wire.read() << 8 | Wire.read()); 
      AcY = (Wire.read() << 8 | Wire.read()); 
      AcZ = (Wire.read() << 8 | Wire.read()); 
      Tmp = (Wire.read() << 8 | Wire.read()); // Read but ignore Tmp for ML
      GyX = (Wire.read() << 8 | Wire.read()); 
      GyY = (Wire.read() << 8 | Wire.read()); 
      GyZ = (Wire.read() << 8 | Wire.read()); 
      
      // Scale exactly as you did for training data
      float ax = (AcX / 16384.0) * 9.81;
      float ay = (AcY / 16384.0) * 9.81;
      float az = (AcZ / 16384.0) * 9.81;

      float gx = GyX / 131.0;
      float gy = GyY / 131.0;
      float gz = GyZ / 131.0;

      // Calculate the index in the 1D input tensor
      int tensor_index = i * NUM_FEATURES;

      // Feed data directly to the neural network input tensor
      input->data.f[tensor_index + 0] = ax;
      input->data.f[tensor_index + 1] = ay;
      input->data.f[tensor_index + 2] = az;
      input->data.f[tensor_index + 3] = gx;
      input->data.f[tensor_index + 4] = gy;
      input->data.f[tensor_index + 5] = gz;
    }
    
    // Maintain the exact 20ms delay you used when generating the CSV
    delay(20); 
  }

 // 2. Run Inference on the collected window
TfLiteStatus invoke_status = interpreter->Invoke();
if (invoke_status != kTfLiteOk) {
  Serial.println("Invoke failed!");
  return;
}

// 3. Interpret the Single Binary Output Node
float circle_probability = output->data.f[0]; // Only index 0 exists!

// 4. Print the result based on the threshold
if (circle_probability > 0.80) { 
  Serial.print("Detected: CIRCLE! (Confidence: ");
  Serial.print(circle_probability * 100, 1);
  Serial.println("%)");
} else {
  Serial.print("Detected: Rest / Random Motion (Non-Circle: ");
  Serial.print((1.0 - circle_probability) * 100, 1);
  Serial.println("%)");
}
}