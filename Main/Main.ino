#include "systemGlobal.hpp"
#include "display.hpp"
#include "peripheral.hpp"
#include "accessControl.hpp"
#include "network.hpp"
#include "fingerprint.hpp"



bool refresh_screen;
bool waitingFlag_1;         
bool waitingFlag_2;
bool waitingFlag_3;
bool waitingFlag_4;
// bool waitingFlag_5;
bool waitingFlag_prompting = true;
bool keypad_enable; 
unsigned char trig_mainMenu;
String input_course_code;
String input_course_passwd;
String matric_no;
String securityCode;
String timer_input; //====Stored timer input from promptInput=====
unsigned long currentTime;
unsigned long timer;
String time_str;
int timer_prev;
char key_press;
char full_date;
// const char *stored_filename;
// const char *stored_filename_2;
String current_session;
String retrievedUID;
String storedUID;
String recorded_matric_no;
// Variables for background time updates
unsigned long previousMinuteMillis = 0;
const long minuteInterval = 60000; // 1 minute



NetworkServer server(80); 
bool hotspot_state = false;

// char timeFull[15]; 
// char date[11];

unsigned long startTime;
unsigned long elapsedTime;

//=========RTC Initialization====
RTC_DS3231 rtc;

//======SD Card Initialization======
SdFat sd;
File32 dataFile; // Creating of File object for the SD card.
// const char *filename = "lecturer_data.jsonl";
// const char *filename2 = "student_data.jsonl";

void setup() {
  Serial.begin(115200);


  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  lcd.begin(20, 4);

  Serial.println(">> Initializing SD card via SdFat...");
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  delay(100);


  // pinMode(buzzer, OUTPUT);
  // digitalWrite(buzzer, LOW);
  pinMode(check_LED, OUTPUT);
  digitalWrite(check_LED, LOW);

  customKeypad.begin();

  projectDisplay();

  RTC_module_check();

  SD_module_check();

  create_file("lecturer_data.jsonl","matric_number-key.jsonl");
  create_file(".UID-key.jsonl");
  keypad_check();

  // reset_scanner();
  // erasing_student_records();
  // erasinG_lecturer_records();
  
  // Serial.println(">> System clean. Ready for new registration");


  // delay(1000);
  Serial.println(">> Initializing fingerprint scanner...");
  SFM.setPinInterrupt(sfmPinInt1);
  pinMode(SFM_IRQ, INPUT);
  SFM.enable();

  fingerprint_check();
  delay(1000);

  

  Main_menu();
  Serial.println(">> Main menu ran!");
  Serial.println(">> Status bar ran!");

  



}

void loop() {
  handle_background_tasks();
  // webpage_connection();
  char key_press = customKeypad.getKey();

  if (key_press) {
    Serial.print("Button Pressed: ");
    Serial.print("---------------");
    Serial.print(key_press);
    Serial.println("---------------");
    // buzzer_sound();
    
    if (key_press >= '0' && key_press <= '3') {
      refresh_screen = true;
      if (refresh_screen) {
        Serial.println(">> Screen Refreshed");
      }
    }
  }

  // elapsedTime = millis() - startTime;
  // if (elapsedTime >= 60000) {
  //   time_update();
  // }

  switch (key_press) {
  // case 'C':
  //   mainState = CODE;
  //   break;
  case 'C':
    mainState = MAIN_MENU;
    refresh_screen = true;
    Serial.println(">> mainState number: " + String(mainState));
    lcd.noBlink();
    break;
  case '1':
    mainState = LECTURER_SIGN_UP;
    Serial.println(">> mainState number: " + String(mainState));
    break;
  case '2':
    mainState = STUDENT_SIGN_UP;
    Serial.println(">> mainState number: " + String(mainState));
    break;
  case '3':
    mainState = ATTENDANCE_PAGE;
    Serial.println(">> mainState number: " + String(mainState));
    break;
  }

  if (refresh_screen) {
    switch (mainState) {
      case MAIN_MENU:
        Main_menu();
        Serial.println(">> Main menu original ran!");
        refresh_screen = false;
        break;
      case LECTURER_SIGN_UP:
        Serial.println(">> Lecturer sign up running...");
        lecturer_signup();
        refresh_screen = false;
        break;
      case STUDENT_SIGN_UP:
        Serial.println(">> Student sign up running...");
        student_signup();
        refresh_screen = false;
        break;
      case ATTENDANCE_PAGE:
        Serial.println(">> Attendance page running...");
        attendancePage();
        subState = ATTENDANCE_PAGE_MENU;
        refresh_screen = false;
        waitingFlag_3 = true;
        keypad_enable = true;
        waitingFlag_4 = false;

        while (waitingFlag_3) {
          handle_background_tasks();
          // webpage_connection();
          // Serial.println(">> Waiting Flag 3 loop active ");
          // delay(500);
          // Serial.println(">> Waiting flag 4: " + String(bool(waitingFlag_4)));
          // delay(1000);
          if (waitingFlag_4) {
            currentTime  = millis();
            elapsedTime = currentTime - startTime;
            // Serial.println("================================");
            // Serial.println(">> Keypad state: " + String(keypad_enable));
            // Serial.println(">> Elapsed time: " + String(elapsedTime));
            // Serial.println(">> Timer: " + String(timer) +"ms");
            // Serial.println("================================");
        
            // delay(1000);
            if(elapsedTime >= timer) {
              Serial.println(">> Session Ended!");
              lcd.clear();
              status_bar();
              lcd.setCursor(0, 2);
              lcd.print("   SESSION ENDED!  ");
              delay(3000);
              attendancePage();
              subState = ATTENDANCE_PAGE_MENU;
              waitingFlag_4 = false;
              keypad_enable = true;
            }
          }
          
          char key_press = customKeypad.getKey();
          if (key_press) {
            if (key_press == 'C') {
              switch (subState) {
                case LECTURER_SIGN_IN:
                case STUDENT_SIGN_IN:
                case SYSTEM_SETTINGS:
                  if (!keypad_enable) {
                    attendancePage_mod();
                    subState = ATTENDANCE_PAGE_MENU;
                    continue;
                  }
                  else {
                    attendancePage();
                    subState = ATTENDANCE_PAGE_MENU;
                    continue;
                  }
                  
                case ATTENDANCE_PAGE_MENU:
                  if (!keypad_enable) {
                    continue;
                  } 
                  waitingFlag_3 = false;
                  waitingFlag_2 = true;
                  continue;
              }
            }

            switch (key_press) {
              case '1':
                subState = LECTURER_SIGN_IN;
                if (!keypad_enable) {
                  lcd.setCursor(0, 1);
                  lcd.print("                   ");
                  lcd.setCursor(0, 2);
                  lcd.print("   ATTENDANCE IN   ");
                  lcd.setCursor(0, 3); 
                  lcd.print("      PROGRESS     ");                
                  // delay(3000);
                }
                else { 
                  lecturer_sign_in();
                  Serial.println(">> Keypad enable state: " + String(keypad_enable));
                  if (!keypad_enable) {
                    attendancePage_mod();
                    subState = ATTENDANCE_PAGE_MENU;
                    startTime = millis();
                  }
                  else {
                    attendancePage();
                    subState = ATTENDANCE_PAGE_MENU;
                  }
                }
                break;
              case '2':
                if (!keypad_enable) {  
                  student_sign_in();
                }
                else {
                  Serial.println(">> No attendance in progress!");
                  lcd.clear();
                  status_bar();
                  lcd.setCursor(0, 2);
                  lcd.print("   NO ATTENDANCE");
                  lcd.setCursor(0, 3);
                  lcd.print("    IN PROGRESS"); 
                  delay(2000);
                  attendancePage();
                  subState = ATTENDANCE_PAGE_MENU;
                }
                break;
              case '3':
                // funcState = HOTSPOT_OFF;
                waitingFlag_prompting = true;
                subState = SYS_SETTINGS_MENU;
                securityCode = Security(F("Enter PIN: "));
                Serial.println(">> Escaped Security function");
                if (subState == SYS_SETTINGS_MENU && securityCode == "") {
                  if (!keypad_enable) {
                    attendancePage_mod();
                    subState = ATTENDANCE_PAGE_MENU;
                    continue;
                  }
                  else {
                    attendancePage();
                    subState = ATTENDANCE_PAGE_MENU;   
                    continue;              
                  }
                }
                system_settings();

                waitingFlag_1 = true;
                Serial.println(">> waitingFlag_1 state: " + String(waitingFlag_1));
                while (waitingFlag_1) {
                  handle_background_tasks();

                  webpage_connection();
                  // Serial.println(">> Waiting Flag 1 loop running");
                  // Serial.println(">> Current subState: " + String(subState));
                  // delay(500);
                  char key_press = customKeypad.getKey();
                  if (key_press) {
                    if (key_press == 'C') {
                      if (subState == HOTSPOT_SETTINGS || subState == OTHER_SETTINGS || subState == END_SESSION) {
                        system_settings();
                        subState = SYS_SETTINGS_MENU;
                        continue;
                      } 
                      else if (subState == SYS_SETTINGS_MENU) {
                        hotspot_state = false;
                        hotspot_off();
                        if (!keypad_enable) {
                          waitingFlag_1 = false;
                          attendancePage_mod();
                          subState = ATTENDANCE_PAGE_MENU;
                        }
                        else {
                          waitingFlag_1 = false;
                          attendancePage();
                          subState = ATTENDANCE_PAGE_MENU;
                        }
                        continue;
                      }
                      continue;
                    }
                    // buzzer_sound();
                    switch (key_press) {
                      case '1': 
                        // funcState = HOTSPOT_OFF;  
                        if (hotspot_state) {
                          Serial.println(">> Inside hotspot settings...");
                          Serial.println(">> Hotspot On");
                          Serial.println(">> Press 1 to On or Press 2 to Off....");
                          lcd.clear();
                          status_bar();
                          lcd.setCursor(0, 1);
                          lcd.print("1. [ON]    2. [OFF]");
                          lcd.setCursor(0, 2);
                          lcd.print("    IP ADDRESS:   ");
                          lcd.setCursor(0, 3);
                          lcd.print("    ");
                          lcd.print(GH_IP);
                        }
                        else {
                          hotspot_settings();
                        }
                        // Serial.println(">> waitingFlag_1 state: " +
                        // String(waitingFlag_1)); Serial.println(">> subState Number: " +
                        // String(subState));    

                        waitingFlag_2 = true;

                        while (waitingFlag_2) {
                        
                        handle_background_tasks();
                        char key_press = customKeypad.getKey();
                        if (key_press) {
                            switch (key_press) {
                              case 'C':
                                if (hotspot_state) {
                                  waitingFlag_2 = false;
                                  system_settings();
                                  subState = SYS_SETTINGS_MENU;
                                }
                                break;
                              case '1':
                                // funcState = HOTSPOT_ON;
                                hotspot_state = true;
                                hotspot_setup();
                                // webpage_connection();
                                lcd.clear();
                                status_bar();
                                lcd.setCursor(0, 1);
                                lcd.print("     HOTSPOT ON    ");
                                Serial.println(">> Hotpsot On...");
                                lcd.setCursor(0, 2);
                                lcd.print("    IP ADDRESS:   ");
                                lcd.setCursor(0, 3);
                                lcd.print("    ");
                                lcd.print(GH_IP);
                                Serial.print(">> IP Address: ");
                                Serial.println(GH_IP);
                                // waitingFlag_2 = false;
                                // system_settings();
                                // subState = SYS_SETTINGS_MENU;
                                // break;
                                continue;

                              case '2':
                                // funcState = HOTSPOT_OFF;
                                hotspot_state = false;
                                hotspot_off();
                                lcd.clear();
                                status_bar();
                                lcd.setCursor(0, 2);
                                lcd.print("     HOTSPOT OFF");
                                Serial.println(">> Hotspot off....");
                                delay(3000);
                                waitingFlag_2 = false;
                                system_settings();
                                subState = SYS_SETTINGS_MENU;
                                break;
                            }

                          }
                        }
                        
                        break;

                      case '2': 
                        set_timer();
                        break;

                      case '3':
                        if (!keypad_enable) {
                          end_session();
                          waitingFlag_2 = true;

                          while (waitingFlag_2) {
                            handle_background_tasks();
                            char key_press = customKeypad.getKey();
                            if (key_press) {
                              switch (key_press) {
                                case '1':
                                  timer = elapsedTime;
                                  waitingFlag_2 = false;
                                  waitingFlag_1 = false;

                                  continue;

                                case '2':
                                  waitingFlag_2 = false;
                                  system_settings();
                                  subState = SYS_SETTINGS_MENU;
                                  continue;
                              }
                            }
                          }
                        }
                        else {
                          subState = OTHER_SETTINGS;
                          waitingFlag_2 = true;
                          other_settings();  

                          while (waitingFlag_2) {
                            handle_background_tasks();
                            char key_press = customKeypad.getKey();
                            if (key_press) {
                              switch (key_press) {
                                case 'C':
                                  switch (subState) {
                                    case DATE_TIME:
                                    case PASSWORD:
                                      subState = OTHER_SETTINGS;
                                      other_settings();
                                      continue;

                                    case OTHER_SETTINGS:
                                      waitingFlag_2 = false;
                                      subState = SYS_SETTINGS_MENU;
                                      system_settings();
                                      continue;
                                  }

                                case '1':
                                  date_and_time();
                                  break;
                                
                                case '2':
                                  change_passwd();
                                  break;
                              }                                                           
                            }
                          }
                        }                
                        break;                        
                    }
                  }
                }

            }
          }
        }

        break;
    }
  }

  while (waitingFlag_2) {
    handle_background_tasks();
    Serial.println(">> Main menu from escape key ran");
    mainState = MAIN_MENU;
    Main_menu();
    lcd.noBlink();
    refresh_screen = true;
    trig_mainMenu = '1';
    waitingFlag_2 = false;
  }
}


