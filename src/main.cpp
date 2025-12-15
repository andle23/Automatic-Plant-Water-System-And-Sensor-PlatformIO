#include <Arduino.h>
#include <LiquidCrystal.h>


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
// Calibration and Settings
// ---------------------------

// 1. Water Level Settings (Ultrasonic)
const int EMPTY_TANK_THRESHOLD = 10; // adjust base on water bucket

// 2. Moisture Settings (Analog 0-4095)
const int SOIL_DRY_THRESHOLD = 2100; // If > 2100, start watering b/c dry
const int SOIL_WET_THRESHOLD = 1550; // If < 1550, stop watering b/c wet

// State Variable
bool isPumpRunning = false;

// To determine water bucket low
int lowWaterCount = 0;
const int LOW_WATER_CONFIRMATIONS = 3;


// ---------------------------
// SETUP
// ---------------------------

void setup() {
  Serial.begin(115200);

  // LCD
  lcd.begin(16, 2);
  lcd.print("System Start...");
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

  const int numReadings = 5;
  long total = 0;
  int validReadings = 0;

  for (int i = 0; i < numReadings; i++) {

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 25000);
    
    if (duration != 0) {

      long distance = duration * 0.0343 / 2;
      total += distance;
      validReadings++;

    }
    delay(50); // Small delay between readings
  }
  
  if (validReadings == 0) {

    return -1; // No valid readings

  }
  
  return total / validReadings; // Return average
}


// ---------------------------
// MAIN LOOP
// ---------------------------

void loop() {

  // Read moisture sensor
  int soil = analogRead(SOIL_PIN);  // 0–4095

  Serial.print("Soil Raw Value: ");
  Serial.println(soil);


  // Convert to moisture percentage (DISPLAY ONLY)
  int moisturePercent = map(soil,SOIL_DRY_THRESHOLD,  SOIL_WET_THRESHOLD, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  

  // Read ultrasonic distance ONLY when pump is OFF
  long waterLevel = 999; // Default to "unknown"
  
  if (!isPumpRunning) {

    waterLevel = getDistanceCM();
    if (waterLevel == -1) waterLevel = 999; // Error = assume empty

  }
 

  Serial.print("Ultrasonic Distance: ");
  Serial.print(waterLevel);
  Serial.println(" cm");

  
  // 1st, check if water bucket is empty 
  if (waterLevel > EMPTY_TANK_THRESHOLD) {
    
    lowWaterCount++;
    
    if (lowWaterCount >= LOW_WATER_CONFIRMATIONS) {

      // TANK IS EMPTY - STOP EVERYTHING
      digitalWrite(RELAY_PIN, HIGH); // Pump OFF
      isPumpRunning = false;
      Serial.println("WARNING: Low Water!"); // for checking

    } 
    
  }
  else {

    // TANK HAS WATER - Check Soil Moisture
    lowWaterCount = 0;
    if (soil > SOIL_DRY_THRESHOLD) {

      // Too Dry -> Turn ON
      digitalWrite(RELAY_PIN, LOW); 
      isPumpRunning = true;

      // UPDATE LCD TO SHOW PUMP ON
      lcd.setCursor(0, 1);
      lcd.print("Pump: ON      ");

      delay(3000); // pump on 3s
      Serial.println("Soil dry -> Pump ON"); // for checking


      // Stop and check
      digitalWrite(RELAY_PIN, HIGH);
      isPumpRunning = false;

      // UPDATE LCD TO SHOW PUMP OFF
      lcd.setCursor(0, 1);
      lcd.print("Pump: OFF     ");

      Serial.println("Checking soil...");
      delay(5000); // Wait 5s for water to absorb

    } 
    else if (soil < SOIL_WET_THRESHOLD) {

      // Wet enough -> Turn pump OFF
      digitalWrite(RELAY_PIN, HIGH); 
      isPumpRunning = false;
      delay(5000); // Wait 5s
      Serial.println("Soil wet -> Pump OFF"); // for checking

    }
    
  }

  // Display on LCD

  // Moisture (Percentage)
  lcd.setCursor(0, 0);
  lcd.print("Moisture:");
  lcd.print(" ");
  lcd.print(moisturePercent);
  lcd.print("%   ");

  // Pump status or Low Water Warning
  lcd.setCursor(0, 1);

  if (waterLevel > EMPTY_TANK_THRESHOLD) {

    lcd.print("LOW WATER! ");

  } 
  else {

    lcd.print("Pump: ");

    if (isPumpRunning) {

      lcd.print("ON      ");

    } 
    else {

      lcd.print("OFF     ");

    }
  }

  delay(500);

}



