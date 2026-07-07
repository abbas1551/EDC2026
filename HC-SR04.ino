// Define the ESP32 pins connected to the HC-SR04
const int trigPin = 5;
const int echoPin = 18;

// Define variables for the duration of the ping and the calculated distance
long duration;
float distanceCm;

void setup() {
  // Start the serial monitor to view the output
  Serial.begin(115200);
  
  // Set the pin modes
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  // 1. Clear the trigPin by setting it LOW for 2 microseconds
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // 2. Trigger the sensor by setting trigPin HIGH for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 3. Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // 4. Calculate the distance
  // The speed of sound is roughly 343 meters per second, or 0.0343 cm per microsecond.
  // We divide by 2 because the sound wave travels out to the object and back.
  distanceCm = duration * 0.0343 / 2;
  
  // 5. Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");
  
  // Wait a short delay before taking the next reading
  delay(100);
}
