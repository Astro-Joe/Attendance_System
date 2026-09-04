#include "systemGlobal.hpp"
#include <sfm.hpp>


#define SFM_IRQ 4
#define SFM_RX 21
#define SFM_TX 22
#define SFM_VCC 15

// VCC, IRQ, RX and TX pins.Also with the UART number(whether U1 or U2) should be specified.
SFM_Module SFM(SFM_VCC, SFM_IRQ, SFM_RX, SFM_TX, 2);
bool lastTouchState = false;

uint8_t temp = 0; // Used to get recognition return
uint16_t tempUID = 0; // Used to get recognition UID 

void sfmPinInt1() {
    SFM.pinInterrupt();
}

void fingerprint_check() {
    if (SFM.isConnected()) {
        lcd.setCursor(0, 3);
        lcd.print("Scanner Init...");
        Serial.println(">> SFM-V1.7 Scanner Online!");
        
       
        SFM.setRingColor(SFM_RING_BLUE, SFM_RING_OFF, 500); 
        // SFM.setRingColor(SFM_RING_OFF, SFM_RING_OFF, 0);
        delay(1000);
        SFM.setRingColor(SFM_RING_CYAN,SFM_RING_OFF , 500); 
        // SFM.setRingColor(SFM_RING_OFF, SFM_RING_OFF, 0);
        delay(1000);
        SFM.setRingColor(SFM_RING_YELLOW, SFM_RING_OFF, 500); 
        // SFM.setRingColor(SFM_RING_OFF, SFM_RING_OFF, 0);
        delay(1000);
        SFM.setRingColor(SFM_RING_PURPLE, SFM_RING_OFF, 500); 
        // SFM.setRingColor(SFM_RING_OFF, SFM_RING_OFF, 0);
        delay(1000);
        SFM.setRingColor(SFM_RING_RED, SFM_RING_OFF, 500); 
        // SFM.setRingColor(SFM_RING_OFF, SFM_RING_OFF, 0);
        delay(1000);
        SFM.setRingColor(SFM_RING_GREEN, SFM_RING_OFF, 500); 
        delay(1000);
        SFM.setRingColor(SFM_RING_GREEN, -1, 500); 
        
    } 
    else {
        lcd.setCursor(0, 3);
        lcd.print("Missing scanner!");
        Serial.println(">> Scanner missing. Check TX/RX wires.");
    }
}


void fingerprint_reg() {
    Serial.println(">> Ready for registration...");

    if (!sdCardFlag) {
        Serial.println(">> SD card missing...Check SD card");
        waitingFlag_2 = true;
        return;
    }
 
    SFM.setRingColor(SFM_RING_YELLOW, -1, 0);
    Serial.println(">> Place your finger on the scanner...");
    lcd.clear();
    status_bar();
    lcd.setCursor(0, 1);
    lcd.print("Place your finger");
    lcd.setCursor(0, 2);
    lcd.print("on the scanner...");    

    while (digitalRead(SFM_IRQ) == LOW) {
        handle_background_tasks();
        yield();
    }
    temp = SFM.register_3c3r_1st();
    if (temp == SFM_ACK_SUCCESS) {
        Serial.println(">> Please release your finger");
        lcd.clear();
        status_bar();
        lcd.setCursor(0, 1);
        lcd.print("Release your finger");
        SFM.setRingColor(SFM_RING_PURPLE, -1, 0);
        while(digitalRead(SFM_IRQ)) {
            yield();
        }        
        delay(2000);


        Serial.println(">> Please put the same finger on the scanner...");
        lcd.clear();
        status_bar();
        lcd.setCursor(0, 1);
        lcd.print("Put the same finger");
        lcd.setCursor(0, 2);
        lcd.print("on the scanner...");
        
        while(digitalRead(SFM_IRQ) == LOW) {
            yield();
        }
        temp = SFM.register_3c3r_2nd();
        if (temp == SFM_ACK_SUCCESS) {
            Serial.println(">> Please release your finger");
            lcd.clear();
            status_bar();
            lcd.setCursor(0, 1);
            lcd.print("Release your finger");
            // delay(2000);
            SFM.setRingColor(SFM_RING_BLUE, -1, 0);
            
            while(digitalRead(SFM_IRQ) == HIGH) {
                yield();
            }
            delay(2000);


            Serial.println(">> Please put the same finger on the scanner...");
            lcd.clear();
            status_bar();
            lcd.setCursor(0, 1);
            lcd.print("Put the same finger");
            lcd.setCursor(0, 2);
            lcd.print("on the scanner...");

            while (digitalRead(SFM_IRQ) == LOW) {
                handle_background_tasks();
                yield();
            }
            tempUID = 0; 
            temp = SFM.register_3c3r_3rd(tempUID);
            if (temp == SFM_ACK_SUCCESS and tempUID != 0) {
                storedUID = String(tempUID);
                Serial.printf(">> Registration successfull with return UID: %d\n", tempUID);
                lcd.clear();
                status_bar();
                lcd.setCursor(0, 2);
                lcd.print("    REGISTRATION");
                lcd.setCursor(0, 3);
                lcd.print("     SUCCESSFUL");
                SFM.setRingColor(SFM_RING_GREEN, -1);
                delay(3000);
                SFM.setRingColor(SFM_RING_GREEN, -1, 0);
                // waitingFlag_2 = true;
                // trig_mainMenu = '0';

            }
            else {
                lcd.clear();
                status_bar();
                lcd.setCursor(0, 2);
                lcd.print("REGISTRATION FAILED");
                Serial.println(">> Registration failed");
                SFM.setRingColor(SFM_RING_RED, 1, 0);
                delay(1500);
                SFM.setRingColor(SFM_RING_GREEN, 1, 0);
                waitingFlag_2 = true;
            }    
        }
        else {
            lcd.clear();
            status_bar();
            lcd.setCursor(0, 2);
            lcd.print("REGISTRATION FAILED");
            Serial.println(">> Error in 2nd placement");
            SFM.setRingColor(SFM_RING_RED, 1, 0);
            delay(1500);
            SFM.setRingColor(SFM_RING_GREEN, 1, 0);
            waitingFlag_2 = true;
        }        
    }
    else {
        lcd.clear();
        status_bar();
        lcd.setCursor(0, 2);
        lcd.print("REGISTRATION FAILED");
        Serial.println(">> Error in 1st placement");
        SFM.setRingColor(SFM_RING_RED, 1, 0);
        delay(1500);
        SFM.setRingColor(SFM_RING_GREEN, 1, 0);
        waitingFlag_2 = true;
    }
}


void fingerprint_recog() {
    Serial.println(">> Fingerprint recognition ongoing...");
    // Serial.printf("IRQ=%d  last=%d\n",digitalRead(SFM_IRQ),lastTouchState);

    SFM.setRingColor(SFM_RING_YELLOW, SFM_RING_OFF, 500);
    // Serial.println(">> Place your finger on the scanner...");
    lcd.clear();
    status_bar();
    lcd.setCursor(0, 1);
    lcd.print("Place your finger");
    lcd.setCursor(0, 2);
    lcd.print("on the scanner...");  

    Serial.println(">> Place you finger on the scanner");
    while (digitalRead(SFM_IRQ) == LOW) {
        handle_background_tasks();
        yield();
    }

    SFM.pinInterrupt();
    SFM.setRingColor(SFM_RING_CYAN, SFM_RING_OFF);
    delay(2000);
    
    tempUID = 0;
    temp = SFM.recognition_1vN(tempUID);

    if (temp == SFM_ACK_SUCCESS && tempUID != 0) {
        retrievedUID = String(tempUID); 
        SFM.setRingColor(SFM_RING_GREEN, SFM_RING_OFF, 500);
        // lcd.clear();
        // status_bar();
        // lcd.setCursor(0, 2);
        // lcd.print("MATRIC ");
        // lcd.setCursor(0 , 3);
        // lcd.print("   ATTENDACE TAKEN");
        Serial.printf(">> Successfully matched with UID: %d\n", tempUID);
        delay(1500);
        SFM.setRingColor(SFM_RING_GREEN, -1, 0);
        // while(digitalRead(SFM_IRQ) == HIGH) {
        //     yield();
        // } 
    }
    else {
        SFM.setRingColor(SFM_RING_RED, -1, 0);
        lcd.clear();
        status_bar();
        lcd.setCursor(0 , 1);
        lcd.print(" RECOGNITION FAILED");
        lcd.setCursor(0, 2);
        lcd.print("    TRY AGAIN OR");
        lcd.setCursor(0, 3);
        lcd.print("  GO AND REGISTER");
        Serial.println(">> Fingerprint recognition failed");
        delay(2000);
        waitingFlag_2 = true;
        SFM.setRingColor(SFM_RING_GREEN, -1, 0);
    }
    //     }
    // }
}


void reset_scanner() {
    Serial.println("\n>> WARNING: Erasing all fingerprints from scanner...");
    
    // Turn solid yellow while processing
    SFM.setRingColor(SFM_RING_YELLOW, SFM_RING_OFF);

    while (digitalRead(SFM_IRQ) == LOW) {
        handle_background_tasks();
        yield();
    }

    SFM.pinInterrupt();
    
    // Fire the nuclear command
    uint8_t temp = SFM.deleteAllUser(); 
    
    if (temp == SFM_ACK_SUCCESS) {
        Serial.println(">> SUCCESS: Scanner memory is completely wiped.");
        SFM.setRingColor(SFM_RING_GREEN, SFM_RING_OFF);
        delay(2000);
    } else {
        Serial.println(">> ERROR: Failed to wipe scanner memory.");
        SFM.setRingColor(SFM_RING_RED, SFM_RING_OFF);
        delay(2000);
    }
    
    // Turn the light off when finished
    SFM.setRingColor(SFM_RING_GREEN, -1, 0);
}