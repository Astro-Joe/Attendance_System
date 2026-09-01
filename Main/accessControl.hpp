#include "systemGlobal.hpp"


extern bool waitingFlag_prompting;

//====Prompting User Input====
String promptInput(String prompt) {
  Serial.println(">> Prompting input...");
  lcd.clear();
  status_bar();
  lcd.setCursor(0, 1);
  lcd.print(prompt);
  lcd.setCursor(0, 2);
  lcd.blink();
  String userInput = "";
  String pin_mask = "";
  // int time_prev = timer;
  switch (funcState) {
    case PASSWD_FUNCTIONALITY:
      Serial.println(">> Enter password...");
      break;

    case COURSE_CODE_FUNCTIONALITY:
      Serial.println(">> Enter course code...");
      break;

    case MATRIC_NUMBER_FUNCTIONALITY:
      Serial.println(">> Enter matriculation number...");
      break;
  }

  while (waitingFlag_prompting) {

    handle_background_tasks();
    char key_press = customKeypad.getKey();
    if (key_press) {
      switch (key_press) {
        case 'E':
          switch (funcState) {
            case PASSWD_FUNCTIONALITY:  
              if (userInput.length() < 8 || userInput.length() > 8) {

                lcd.setCursor(0, 3);
                lcd.print(F(" INVALID PASSWORD! "));
                Serial.println(">> Invalid password!");
                Serial.println(">> Enter password...");
                delay(2000);
                lcd.setCursor(0, 1);
                lcd.print(F("                   "));
                lcd.setCursor(0, 2);
                lcd.print(F("                   "));
                lcd.setCursor(0, 3);
                lcd.print(F("                   "));
                lcd.setCursor(0,1);
                lcd.print(prompt);
                lcd.setCursor(0,2);
                userInput.clear();
                pin_mask.clear();
                continue;
              }
              else {
                // lcd.setCursor(0, 1);
                // lcd.print(F("                   "));
                // lcd.setCursor(0, 2);
                // lcd.print(F("                   "));
                // lcd.setCursor(0,3);
                // lcd.print(F("    INFO SAVED!    "));
                // delay(2000);
                lcd.noBlink();
                waitingFlag_prompting = false;
                waitingFlag_2 = false; //Just to be safe
                return userInput;
              }
            // }
            case COURSE_CODE_FUNCTIONALITY: 
              if (userInput.length() < 11 || userInput.length() > 11) {
                lcd.setCursor(0, 3);
                lcd.print(F("INVALID COURSE CODE"));
                Serial.println(">> Invalid course code!");
                Serial.println(">> Enter course code...");
                delay(2000);
                lcd.setCursor(0, 1);
                lcd.print(F("                   "));
                lcd.setCursor(0, 2);
                lcd.print(F("                   "));              
                lcd.setCursor(0, 3);
                lcd.print(F("                   "));
                lcd.setCursor(0,1);
                lcd.print(prompt);
                lcd.setCursor(0,2);
                userInput.clear();
                continue;
              } 
              else {
                delay(1000);
                lcd.noBlink();
                waitingFlag_prompting = false;
                return userInput;
              }
            case MATRIC_NUMBER_FUNCTIONALITY:
              if (userInput.length() < 12 || userInput.length() > 12) {
                lcd.setCursor(0, 3);
                lcd.print(F("INVALID MATRIC NUM!"));
                Serial.println(">> Invalid matric number!");
                delay(2000);
                lcd.setCursor(0, 1);
                lcd.print(F("                   "));
                lcd.setCursor(0, 2);
                lcd.print(F("                   "));              
                lcd.setCursor(0, 3);
                lcd.print(F("                   "));
                lcd.setCursor(0,1);
                lcd.print(prompt);
                lcd.setCursor(0,2);
                userInput.clear();
                continue;
              } 
              else {
                lcd.noBlink();
                waitingFlag_prompting = false;
                return userInput;
              }
            case TIMER_FUCNTIONALITY:
              int time_comp = userInput.toInt();
              // int time = userInput.toInt();
              // Serial.print(">> Time: " + String(time));
              if (time_comp < 10) {
                lcd.setCursor(0, 1);
                lcd.print("                   ");
                lcd.setCursor(0, 2);
                lcd.print("                   ");     
                lcd.setCursor(0, 2);
                lcd.print("MINIMUM IS 10 MINS");
                delay(3000);
                lcd.setCursor(0, 2);
                lcd.print("                    ");    
                lcd.setCursor(0, 1);
                lcd.print(prompt);    
                lcd.setCursor(0, 2);      
                userInput.clear();
                continue;                   
              }
              else if (time_comp > 120) {
                lcd.setCursor(0, 1);
                lcd.print("                   ");
                lcd.setCursor(0, 2);
                lcd.print("                   ");     
                lcd.setCursor(0, 2);
                lcd.print("MAXMIMUM IS 120 MINS");
                delay(3000);
                lcd.setCursor(0, 2);
                lcd.print("                    ");    
                lcd.setCursor(0, 1);
                lcd.print(prompt);  
                lcd.setCursor(0, 2);     
                userInput.clear();                      
                continue;     
              }
              else{
                int time = userInput.toInt();
                lcd.noBlink();
                time_str = String(time);

                lcd.setCursor(0, 1);
                lcd.print("                   ");
                lcd.setCursor(0, 2);
                lcd.print("                   ");     
                lcd.setCursor(0, 2);
                lcd.print(" TIMER HAS BEEN SET");
                lcd.setCursor(0, 3);
                lcd.print("    TO " + time_str + "MINS"); 
                delay(3000);
                waitingFlag_prompting =false;
                return time_str;
              }
                
          }
        case 'C': 
          if (userInput.isEmpty()){
            if (funcState == TIMER_FUCNTIONALITY) {
              // lcd.noBlink();
              // waitingFlag_prompting = false;
              // waitingFlag_2 = true;
              // trig_mainMenu = '0';
              Serial.println(">> Previously set time: " + String(timer_prev));
              
              if (timer_prev != 0) {
                userInput = timer_prev;
              }
              else {
                userInput = "10";
              }
            
              // lcd.setCursor(0, 1);
              // lcd.print("                   ");
              // lcd.setCursor(0, 2);
              // lcd.print("                   ");     
              // lcd.setCursor(0, 2);
              // lcd.print("FIELD CAN'T BE EMPTY");
              // delay(3000);
              // lcd.setCursor(0, 2);
              // lcd.print("                    ");    
              // lcd.setCursor(0, 1);
              // lcd.print(prompt); 
              // lcd.setCursor(0, 1); 
              // continue;
            }
            lcd.noBlink();
            waitingFlag_prompting = false;
            waitingFlag_2 = true;
            trig_mainMenu = '0';
            return userInput;


          }
          else if (funcState == PASSWD_FUNCTIONALITY) {
            unsigned char inputLength = userInput.length() -1;
            unsigned char pin_mask_length = pin_mask.length() - 1;
            Serial.print(">> Input length: ");
            Serial.println(pin_mask.length());
            Serial.print(">> System input length: ");
            Serial.println(pin_mask_length);     
              
            userInput.remove(inputLength, 1);
            pin_mask.remove(pin_mask_length, 1);
            lcd.setCursor(0, 2);
            lcd.print("                   ");
            lcd.setCursor(0, 2);
            lcd.print(pin_mask);
            Serial.println(">> Backspace: " + userInput); 
            continue;             
          }
          else {
            unsigned char inputLength = userInput.length() -1;
            Serial.print(">> Input length: ");
            Serial.println(userInput.length());
            Serial.print(">> System input length: ");
            Serial.println(inputLength);

            userInput.remove(inputLength, 1);
            lcd.setCursor(0, 2);
            lcd.print("                   ");
            lcd.setCursor(0, 2);
            lcd.print(userInput);
            Serial.println(">> Backspace: " + userInput);
            continue;            
          }
        default:
          if (funcState == PASSWD_FUNCTIONALITY) {
            userInput += key_press;
            pin_mask += "*";
            lcd.setCursor(0, 2);
            // lcd.print(codeInput);
            lcd.print(pin_mask);
            // lcd.setCursor(0, 3);
            // lcd.print(userInput);
            Serial.println(">> codeInput: " + userInput);
          }
          else {
            userInput += key_press;
            lcd.setCursor(0, 2);
            lcd.print(userInput);
            Serial.println(">> userInput: " + userInput);            
          }
      }
    }
  }
}




//====Security for system where ever needed====
String Security(String prompt) {
  Serial.println(">> Security function running...");
  Serial.println(">> Enter the security code...");
  lcd.clear();
  status_bar();
  lcd.setCursor(0, 1);
  lcd.print(prompt);
  lcd.setCursor(0, 2);
  lcd.blink();
  String codeInput = "";
  String pin_mask = "";
  String code = "12345678";
  
  while (waitingFlag_prompting) {
    handle_background_tasks();

    char key_press = customKeypad.getKey();
    if (key_press) {
      switch (key_press) {
        case 'E':
          if (codeInput != code) {           
            lcd.setCursor(0, 2);
            lcd.print(F("    INVALID PIN!   "));
            Serial.println(">> Invalid PIN");
            Serial.println(">> Try again...");
            delay(2000);
            lcd.setCursor(0, 1);
            lcd.print(F("                   "));
            lcd.setCursor(0, 2);
            lcd.print(F("                   "));
            lcd.setCursor(0,1);
            lcd.print(F("ENETR PIN:"));
            Serial.println(">> Enter the security code...");
            lcd.setCursor(0,2);
            codeInput = "";
            pin_mask = "";
            continue;
          } 
          else {
            // lcd.setCursor(0, 1);
            // lcd.print(F("                    "));
            // lcd.setCursor(0, 2);
            // lcd.print(F("                    "));
            lcd.setCursor(0, 3);
            lcd.print(F("  ACCESS GRANTED!  "));
            Serial.println(">> Access Granted");
            delay(1000);
            lcd.noBlink();
            waitingFlag_prompting = false;
            continue;
          }
        case 'C': 
          if (codeInput.isEmpty()){
            Serial.println(">> Current SubState: " + String(subState));
            if (subState == SYS_SETTINGS_MENU) {
              lcd.noBlink();
              waitingFlag_prompting = false;
              return codeInput;
            }
            // else {
            lcd.noBlink();
            waitingFlag_prompting = false;
            waitingFlag_2 = true;
            return codeInput;
            // }
          }
          else {
            unsigned char inputLength = codeInput.length() -1;
            unsigned char abstractLength = pin_mask.length() -1; 
            Serial.print(">> Input length: ");
            Serial.println(codeInput.length());
            Serial.print(">> System input length: ");
            Serial.println(inputLength);
          
            codeInput.remove(inputLength, 1);
            pin_mask.remove(abstractLength, 1);
            lcd.setCursor(0, 2);
            lcd.print("                   ");
            lcd.setCursor(0, 2);
            lcd.print(pin_mask);
            Serial.println(">> Backspace: " + codeInput);
            continue;
          }

        default:
          codeInput += key_press;
          pin_mask += "*";
          lcd.setCursor(0, 2);
          // lcd.print(codeInput);
          lcd.print(pin_mask);
          Serial.println(">> codeInput: " + codeInput);

      }
    }
  }
  return codeInput;
}
