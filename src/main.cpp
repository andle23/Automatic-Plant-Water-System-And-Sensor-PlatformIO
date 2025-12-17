#include <Arduino.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include "secrets.h" 
#include <BlynkSimpleEsp32.h>
 


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
// BLYNK & WIFI CREDENTIALS
// ---------------------------

// Use the credentials from secrets.h
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

// Blynk timer
BlynkTimer timer;


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
bool manualOverride = false; // For manual control via Blynk

// To determine water bucket low
int lowWaterCount = 0;
const int LOW_WATER_CONFIRMATIONS = 3;


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
// Send data to Blynk
// ---------------------------

void sendSensorData() {

  int soil = analogRead(SOIL_PIN);
  int moisturePercent = map(soil, SOIL_DRY_THRESHOLD, SOIL_WET_THRESHOLD, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  
  long waterLevel = getDistanceCM();
  
  // Send to Blynk virtual pins
  Blynk.virtualWrite(V0, moisturePercent);      // Moisture %
  Blynk.virtualWrite(V1, waterLevel);           // Water level
  Blynk.virtualWrite(V2, isPumpRunning ? 1 : 0); // Pump status
  Blynk.virtualWrite(V3, soil);                 // Raw soil value

}


// ---------------------------
// Manual pump control from Blynk
// ---------------------------

BLYNK_WRITE(V4) {

  int buttonState = param.asInt();
  
   
  if (buttonState == 1) {

    manualOverride = true;
    digitalWrite(RELAY_PIN, LOW); // Pump ON
    isPumpRunning = true;

    Blynk.logEvent("pump_manual", "Manual pump activated");

  } 
  else {

    manualOverride = false;
    digitalWrite(RELAY_PIN, HIGH); // Pump OFF
    isPumpRunning = false;

  }
}


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

  // Connect to Blynk/Wifi
  lcd.print("Connecting WiFi");
  Blynk.begin(auth, ssid, pass);
  lcd.clear();
  lcd.print("WiFi Connected!");
  delay(1000);
  lcd.clear();

  // Setup Blynk timer to send data every second
  timer.setInterval(1000L, sendSensorData);

  Serial.println("Setup complete.");
}


// ---------------------------
// MAIN LOOP
// ---------------------------

void loop() {

  Blynk.run();
  timer.run();

  // Skip automatic control if manual override is active
  if (manualOverride) {

    lcd.setCursor(0, 0);
    lcd.print("MANUAL MODE    ");
    lcd.setCursor(0, 1);
    lcd.print("Pump: ");
    lcd.print(isPumpRunning ? "ON " : "OFF");
    delay(500);
    return;

  }

  // Read moisture sensor
  int soil = analogRead(SOIL_PIN);  // 0–4095

  Serial.print("Soil Raw Value: ");
  Serial.println(soil);


  // Convert to moisture percentage (DISPLAY ONLY)
  int moisturePercent = map(soil,SOIL_DRY_THRESHOLD,  SOIL_WET_THRESHOLD, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);


  // Moisture (Percentage)
  lcd.setCursor(0, 0);
  lcd.print("Moisture:");
  lcd.print(" ");
  lcd.print(moisturePercent);
  lcd.print("%   ");
  

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

      // Send notification to Blynk
      Blynk.logEvent("low_water", "Water tank is empty!");

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

      delay(2000); // pump on 2s. Adjust base on plant water needs
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
   


