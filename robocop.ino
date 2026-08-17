#include <ESP32Servo.h>

#define SERVO_PIN 5            // Servo motor signal pin
#define FLAME_SENSOR_PIN1 35   // Flame sensor pin 1
#define FLAME_SENSOR_PIN2 32   // Flame sensor pin 2
#define FLAME_SENSOR_PIN3 33   // Flame sensor pin 3
#define RELAY_PIN 26           // Relay for the pump
#define SMOKE_SENSOR_PIN 18    // Smoke sensor pin
#define BUZZER_PIN 19          // Buzzer pin
#define WATER_SENSOR_PIN 34    // Analog pin connected to the HW-038 sensor
#define MAX_SENSOR_VALUE 1880   // Max value observed when sensor is fully submerged
#define LOW_WATER_LEVEL_THRESHOLD 500   // Threshold for "Low Water Level"

Servo servoMotor; // Create a Servo object

int sensorValue = 0;  // Variable to store the water sensor reading

void setup() {
  Serial.begin(115200);  // Initialize Serial Monitor
  pinMode(FLAME_SENSOR_PIN1, INPUT);   // Initialize flame sensor pins
  pinMode(FLAME_SENSOR_PIN2, INPUT);
  pinMode(FLAME_SENSOR_PIN3, INPUT);
  pinMode(SMOKE_SENSOR_PIN, INPUT);   // Initialize smoke sensor pin
  pinMode(RELAY_PIN, OUTPUT);         // Initialize the relay pin for the pump
  pinMode(BUZZER_PIN, OUTPUT);        // Initialize the buzzer pin
  pinMode(WATER_SENSOR_PIN, INPUT);   // Initialize the water sensor pin

  // Initialize the servo motor
  servoMotor.attach(SERVO_PIN, 500, 2400);
  servoMotor.write(0);  // Start with the servo at 0 degrees

  digitalWrite(RELAY_PIN, HIGH); // Ensure pump is off initially
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is off initially

  Serial.println("Setup complete. Waiting for flame, smoke, or low water level...");
}

void loop() {
  // Read the flame sensor values
  int flameSensorValue1 = digitalRead(FLAME_SENSOR_PIN1);
  int flameSensorValue2 = digitalRead(FLAME_SENSOR_PIN2);
  int flameSensorValue3 = digitalRead(FLAME_SENSOR_PIN3);

  // Read the smoke sensor value
  int smokeSensorValue = digitalRead(SMOKE_SENSOR_PIN);

  // Read the water level sensor value
  sensorValue = analogRead(WATER_SENSOR_PIN);

  // Output the current sensor value to Serial Monitor
  Serial.print("Water Sensor Value: ");
  Serial.println(sensorValue);

  // Calculate the water level percentage (for better readability)
  float waterLevelPercentage = ((float)sensorValue / (float)MAX_SENSOR_VALUE) * 100;
  Serial.print("Water Level: ");
  Serial.print(waterLevelPercentage);
  Serial.println("%");

  // Output the flame sensor values to Serial Monitor
  Serial.print("Flame Sensor 1 Value: ");
  Serial.println(flameSensorValue1);
  Serial.print("Flame Sensor 2 Value: ");
  Serial.println(flameSensorValue2);
  Serial.print("Flame Sensor 3 Value: ");
  Serial.println(flameSensorValue3);
  Serial.print("Smoke Sensor Value: ");
  Serial.println(smokeSensorValue);

  // Check if water level is below the threshold (low water level)
  if (sensorValue < LOW_WATER_LEVEL_THRESHOLD) {
    Serial.println("Low Water Level - Actuators Disabled");

    // If water level is low, deactivate actuators
    deactivateActuators();

    // If no flame is detected, wait for water level to rise
    if (flameSensorValue1 == HIGH && flameSensorValue2 == HIGH && flameSensorValue3 == HIGH) {
      Serial.println("Waiting for water level to rise...");
    }
  } else {
    // If water level is sufficient, check for flame
    if ((flameSensorValue1 == LOW || flameSensorValue2 == LOW || flameSensorValue3 == LOW)) {
      // Flame detected only
      Serial.println("Flame detected! Activating flame response.");
      activateActuators(); // Activate actuators only if flame is detected and water level is sufficient
    } 
    else {
      // No flame detected
      deactivateActuators();
    }
  }

  // Smoke detection: Regardless of water level or flame detection
  if (smokeSensorValue == LOW) {
    Serial.println("Smoke detected! Activating smoke effect.");
    digitalWrite(BUZZER_PIN, HIGH); // Buzzer ON for high smoke
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Buzzer OFF when no smoke detected
  }

  delay(500); // Delay for readability
}

// Function to activate the actuators (servo and pump)
void activateActuators() {
  // Rotate servo motor and activate pump
  rotateServo(0, 180, 10);
  rotateServo(180, 0, -10);
  digitalWrite(RELAY_PIN, LOW); // Turn on the pump
}

// Function to deactivate the actuators (servo and pump)
void deactivateActuators() {
  // Reset servo and deactivate pump
  servoMotor.write(0); // Reset servo to 0 degrees
  digitalWrite(RELAY_PIN, HIGH); // Turn off the pump
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