#pragma once 
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <RTClib.h>
#include <Keypad.h>
#include <Keypad_I2C.h>



#define I2C_SDA_PIN 23
#define I2C_SCL_PIN 25

// #define FINGER_RX 16
// #define FINGER_TX 17
// #define buzzer 13
#define check_LED 15

extern String input_course_code; // Stores lecturers' course code
extern String input_course_passwd; // Stores lecturers' assigned course protection code
extern String matric_no; // Students' inputted matric number
extern unsigned long startTime;
extern unsigned long elapsedTime;
extern bool refresh_screen;
extern bool waitingFlag_1;
extern bool waitingFlag_2;
extern bool waitingFlag_4;// extern bool waitingFlag_3;
extern bool waitingFlag_prompting;// extern bool waitingFlag_5;
extern bool sdCardFlag; 
extern unsigned char trig_mainMenu;// extern unsigned char key_press;
extern unsigned long timer; // Time for how long attndance can be taken
extern bool keypad_enable; // Prevents going back to sign up page during a current session
extern String time_str; // User timer input converted to string
extern String timer_input;
extern int timer_prev; // Previously stored timer
extern char full_date; // Full date '
// extern const char *stored_filename;
// extern const char *stored_filename_2;
extern String storedUID;
extern String retrievedUID;
extern String current_session; // File created for the current attendance session
extern String recorded_matric_no; // Stores matric number of signed in students
IPAddress GH_IP;
extern bool hotspot_state;
extern unsigned long previousMinuteMillis;
extern const long minuteInterval;


char date[11];
char timeFull[15]; 


//====For prompting user inputs===
String promptInput(String prompt);
//====For security prompt====
String Security(String prompt);
//===For logging lecturer data upon sign up===
void lecturer_log_data(const char *filename, String course_code, String course_passwd);
//====For logging student data upon sign up====
void student_log_data(const char *filename, String UID, String matric_number);
//===Attendance page during a current session===
extern void attendancePage_mod();
//===Creating new files===
bool create_file(const char *filename, const char *filenanme_2 = "empty.txt"); 
//====For logging students matric no after fingerprint scan====
void attendance_log_data();
//===For registering fingerprints===
void fingerprint_reg(); 
//====Recognizing fingerprints====
void fingerprint_recog(); 
//====For validation student identity=====
extern bool student_validation(const char *filename, String input_matric_no = "", String UID = "");
//=====For validating inputted course====
extern bool course_validation(const char *filename, String inputCourse, String inputPassword = "");
//===For recording Matric number======
void attendance_log_data(const char *filename, String recorded_matric);
//===For wipinng all students login info from the sd card====
void erasing_student_records();
//====For wiping all registered finegrprints from the fingerprint scanner. Works in hand with erasing_students_records()====
void reset_scanner();





//========Contains different screen pages======
enum mainPage {
  MAIN_MENU, //====Showing the options to access other screens.====
  LECTURER_SIGN_UP, //====Lecturers' sign up page====
  STUDENT_SIGN_UP, //====Student sign up page====
  ATTENDANCE_PAGE //====Attendance page====
}; 
mainPage mainState; //=====Object for Main Pages=====

//======Contains different screen sub-pages=====
enum subPage {
  ATTENDANCE_PAGE_MENU, //====Contains Lecturers' sign in, Students' sign in and System settings page====
  LECTURER_SIGN_IN,
  STUDENT_SIGN_IN,
  SYSTEM_SETTINGS,
  SYS_SETTINGS_MENU,
  HOTSPOT_SETTINGS,
  END_SESSION,
  DATE_TIME,
  PASSWORD,
  // TIMER, //Redunadnt : using promptInput and funcState
  OTHER_SETTINGS
};
subPage subState; //=====Object for subpages======== 

//====For changing the functionality of a partcular function========
enum diffFunc {
  DEFAULT_STATE, //====The default state====
  PASSWD_FUNCTIONALITY, //====State for enabling password functionality in promptInput=====
  COURSE_CODE_FUNCTIONALITY, //====State for enabling course code functionality in promptInput====
  MATRIC_NUMBER_FUNCTIONALITY, //===State for enabling matric number fucntionality in prompt input===
  TIMER_FUCNTIONALITY //===State for enabling timer functionality in prompt input====
};
diffFunc funcState = DEFAULT_STATE; //=====Object for checking if a function's functionality can be changed======

//======RTC setup======
extern RTC_DS3231 rtc;

// =======LCD Setup=====
LiquidCrystal lcd(18,13,14,19,16,17); // change d2 and d3 to GPIO 21 and 22 respectively


//======Keypad Initialization======
#define KEYPAD_ADDRESS 0x27

const unsigned char ROWS = 4; 
const unsigned char COLS = 3; 

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'C','0','E'}
};
byte rowPins[ROWS] = {0, 1, 2,4}; 
byte colPins[COLS] = {5, 6, 7};
// byte rowPins[ROWS] = {0, 1, 2, 3}; 
// byte colPins[COLS] = {4, 5,6};
Keypad_I2C customKeypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, KEYPAD_ADDRESS);


//=====Array containing months of the year======
char monthsOftheYear[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                           "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

//======Date and Time Tab======
void status_bar() {
  DateTime now = rtc.now();
  //===Full date===
  sprintf(date, "%02d-%02d-%02d", now.day(), now.month(), now.year());
  // time may be removed
  sprintf(timeFull, "%02d:%02d", now.hour(), now.minute());

  lcd.setCursor(0, 0);
  lcd.print(date);
  lcd.setCursor(15, 0);
  lcd.print(timeFull);

}

void handle_background_tasks() {
    
  // 1. Check the Clock
  unsigned long currentMillis = millis();
  if (currentMillis - previousMinuteMillis >= 60000) {
    previousMinuteMillis = currentMillis;
    status_bar(); 
  }

}