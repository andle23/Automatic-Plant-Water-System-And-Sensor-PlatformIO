#include <Arduino.h>
#include <LiquidCrystal.h>


// SIMPLE RELAY / PUMP TEST (ESP32 + active-LOW relay)
const int RELAY_PIN = 15; // your relay pin

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // OFF (active LOW)
  delay(500);
  Serial.println("Relay test ready");
}

void loop() {
  Serial.println("TURN PUMP ON");
  digitalWrite(RELAY_PIN, LOW);   // ON
  delay(3000);                    // run 3s

  Serial.println("TURN PUMP OFF");
  digitalWrite(RELAY_PIN, HIGH);  // OFF
  delay(3000);
}




/*

// ---------------------------
// PIN DEFINITIONS
// ---------------------------

// LCD pins
const int rs = 13, en = 12, d4 = 14, d5 = 27, d6 = 26, d7 = 25;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Ultrasonic Sensor
#define TRIG_PIN 33
#define ECHO_PIN 32

// Capacittive Soil Moisture Sensor
#define SOIL_PIN 34   // AOUT

// 5v Relay
#define RELAY_PIN 15  // Active LOW

// ---------------------------
void setup() {
  Serial.begin(115200);

  // LCD
  lcd.begin(16, 2);
  lcd.print("Water System");
  delay(1500);
  lcd.clear();

  // Relay control
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // pump OFF (active LOW)

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Soil moisture
  pinMode(SOIL_PIN, INPUT);

  Serial.println("Setup complete.");
}

// ---------------------------
// Read ultrasonic distance
// ---------------------------

long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // timeout 25ms
  long distance = duration * 0.0343 / 2;

  return distance;
}

// ---------------------------
// MAIN LOOP
// ---------------------------

void loop() {
  // Read moisture sensor
  int soil = analogRead(SOIL_PIN);  // 0–4095
  bool isDry = soil < 1500;         // adjust threshold based on test

  // Read ultrasonic distance
  long waterLevel = getDistanceCM();

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("Moist:");
  lcd.print(soil);
  lcd.print("    ");

  lcd.setCursor(0, 1);
  lcd.print("Dist:");
  lcd.print(waterLevel);
  lcd.print("cm   ");

  // Pump logic
  if (isDry) {
    Serial.println("Soil dry → Pump ON");
    digitalWrite(RELAY_PIN, LOW);   // active LOW → turn pump ON
  } else {
    Serial.println("Soil OK → Pump OFF");
    digitalWrite(RELAY_PIN, HIGH);  // turn pump OFF
  }

  delay(500);
}


*/
