#include "systemGlobal.hpp"

// =====Sounding of buzzer=====
// void buzzer_sound() {
//   digitalWrite(buzzer, HIGH);
//   delay(5);
//   digitalWrite(buzzer, LOW);
// }


//========Checking RTC module Initialization======
void RTC_module_check() {
  if (!rtc.begin()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Couldn't find RTC"));
    Serial.println(">> Couldn't find RTC");
    digitalWrite(check_LED, HIGH);
    delay(1000);
  } 
  // else if (rtc.lostPower()) {
  //   u8g2.clearBuffer();
  //   u8g2.drawStr(0, 16, "RTC lost power");
  //   u8g2.sendBuffer();
  //   digitalWrite(check_LED, HIGH);
  //   delay(1000);
  //   digitalWrite(check_LED, LOW);
  //   delay(1000);
  // }
  else {
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("RTC Init..."));
    Serial.println(">> RTC Initialized successfully");
    delay(1000);
  }
}

// void RTC_module_check() {
//   Serial.println("Starting RTC...");
//   if (!rtc.begin()) {
//     Serial.println("CRITICAL ERROR: RTC Failed to initialize!");
//     Serial.println("System Halted.");
//     while (1) { delay(10); } // THIS IS CRUCIAL. It stops the crash trap.
//   }
//   Serial.println("RTC Initialized Successfully.");
// }


void keypad_check() {
  Wire.beginTransmission(KEYPAD_ADDRESS);
  if (Wire.endTransmission() == 0) {
    Serial.println(">> Keypad Initialized...");
    customKeypad.begin(); // Safe to wake up the chip
    lcd.setCursor(0,2);
    lcd.print(F("Keypad Init..."));
    delay(1000);
  } 
  else {
    Serial.println(">> CRITICAL ERROR: Backpack not found at this address.");
    Serial.println(">> Check your SDA/SCL wires and your I2CADDR!");
    lcd.setCursor(0,2);
    lcd.print(F("Couldn't find keypad"));
    digitalWrite(check_LED, HIGH);
    delay(1000);
    // while(1) { delay(10);  // Trap the ESP32 here so it doesn't crash
  }
}


// ======Updating the time======
void time_update() {
    if (elapsedTime >= 60000) {
        DateTime now = rtc.now();
        char timeFull[15];
        sprintf(timeFull, "%02d:%02d", now.hour(), now.minute());
        lcd.setCursor(15, 0);
        lcd.print(timeFull);
    }
}