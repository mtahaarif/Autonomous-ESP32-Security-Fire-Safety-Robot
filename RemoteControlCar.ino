#include <ESP32Servo.h>

#define SERVO_PIN 5         // Servo motor signal pin
#define FLAME_SENSOR_PIN1 35 // Flame sensor pin 1
#define FLAME_SENSOR_PIN2 32 // Flame sensor pin 2
#define FLAME_SENSOR_PIN3 33 // Flame sensor pin 3
#define RELAY_PIN 26        // Relay for the pump
#define SMOKE_SENSOR_PIN 18 // Smoke sensor pin
#define BUZZER_PIN 19       // Buzzer pin

Servo servoMotor; // Create a Servo object

// Define PWM frequencies and resolution (if you need them later for buzzer, etc.)
const int pwmFreq = 50;    // PWM frequency for servo
const int pwmResolution = 8; // 8-bit resolution (0-255 range)

void setup() {
  Serial.begin(115200);

  // Initialize the flame sensor pins as inputs
  pinMode(FLAME_SENSOR_PIN1, INPUT);
  pinMode(FLAME_SENSOR_PIN2, INPUT);
  pinMode(FLAME_SENSOR_PIN3, INPUT);

  // Initialize the smoke sensor pin as an input
  pinMode(SMOKE_SENSOR_PIN, INPUT);

  // Initialize the relay pin
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Ensure the pump is off initially

  // Initialize the buzzer pin (not used for siren, just for smoke high-low effect)
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure the buzzer is off initially

  // Initialize the servo motor (using Servo library)
  servoMotor.attach(SERVO_PIN, 500, 2400); // Attach servo to the pin with proper PWM range
  servoMotor.write(0); // Start with the servo at 0 degrees

  Serial.println("Setup complete. Waiting for flame or smoke detection...");
}

void loop() {
  // Read the flame sensor values
  int flameSensorValue1 = digitalRead(FLAME_SENSOR_PIN1);
  int flameSensorValue2 = digitalRead(FLAME_SENSOR_PIN2);
  int flameSensorValue3 = digitalRead(FLAME_SENSOR_PIN3);

  // Read the smoke sensor value
  int smokeSensorValue = digitalRead(SMOKE_SENSOR_PIN);

  // Output sensor values to Serial Monitor
  Serial.print("Flame Sensor 1 Value: ");
  Serial.println(flameSensorValue1);
  Serial.print("Flame Sensor 2 Value: ");
  Serial.println(flameSensorValue2);
  Serial.print("Flame Sensor 3 Value: ");
  Serial.println(flameSensorValue3);
  Serial.print("Smoke Sensor Value: ");
  Serial.println(smokeSensorValue);

  // Check for flame detection from any sensor
  if ((flameSensorValue1 == LOW || flameSensorValue2 == LOW || flameSensorValue3 == LOW) && smokeSensorValue == LOW) {
    Serial.println("Both flame and smoke detected! Activating all responses.");

    // Rotate servo and activate pump
    rotateServo(0, 180, 10);
    rotateServo(180, 0, -10);
    digitalWrite(RELAY_PIN, LOW); // Turn on the pump

  } else if (flameSensorValue1 == LOW || flameSensorValue2 == LOW || flameSensorValue3 == LOW) {
    // Flame detected only
    Serial.println("Flame detected! Servo moving and pump activating.");
    delay(100);
    rotateServo(0, 180, 10);
    rotateServo(180, 0, -10);
    delay(100);
    digitalWrite(RELAY_PIN, LOW); // Turn on the pump
  } else if (smokeSensorValue == LOW) {
    // Smoke detected
    Serial.println("Smoke detected! Activating smoke effect.");
    // Smoke detection - high effect (e.g., buzzer ON)
    digitalWrite(BUZZER_PIN, HIGH); // Buzzer ON for high smoke
  } else {
    // No flame or smoke detected
    servoMotor.write(0); // Reset servo to 0 degrees
    digitalWrite(RELAY_PIN, HIGH); // Deactivate the pump
    digitalWrite(BUZZER_PIN, LOW); // Buzzer OFF
  }


}

// Function to rotate the servo between specified positions
void rotateServo(int start, int end, int step) {
  if (step > 0) {
    for (int pos = start; pos <= end; pos += step) {
      Serial.print("Servo rotating to position: ");
      Serial.println(pos);
      servoMotor.write(pos);
      delay(25);
    }
  } else {
    for (int pos = start; pos >= end; pos += step) {
      Serial.print("Servo rotating to position: ");
      Serial.println(pos);
      servoMotor.write(pos);
      delay(25);
   }
  }
}