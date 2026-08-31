#include "esp32-hal.h"
#include "systemGlobal.hpp"


//=======Displayig Project Title and Name of students======
void projectDisplay() {
  lcd.setCursor(0,0);
  lcd.print(" ATTENDANCE SYSTEM ");
  lcd.setCursor(0,2);
  lcd.print("BY: AKANDE OGOOLUWA");
  lcd.setCursor(0,3);
  lcd.print(" KENHONO OLUWASEGUN");
  delay(3000);
  Serial.println(">> Project display successfully ran!");
}


//=====Showing the options to access other screens======
void Main_menu() {
  lcd.clear();
  status_bar();
  lcd.setCursor(0, 1);
  lcd.print("1. LECTURER SIGN UP");
  lcd.setCursor(0,2);
  lcd.print("2. STUDENT SIGN UP");
  lcd.setCursor(0, 3);
  lcd.print("3. ATTENDANCE PAGE");
  // Serial.println(">> Main menu ran!");
}

//====Page to store lecturers' course details at first====
void lecturer_signup () {
  // create_file("lecturer_data.jsonl");
  const char *current_filename = "lecturer_data.jsonl";
  waitingFlag_2 = false;
  waitingFlag_prompting = true;
  Security(F("ENTER PIN: "));
  if (waitingFlag_2) {
    waitingFlag_2 = false;
    return;
  }

  waitingFlag_prompting = true;
  funcState = COURSE_CODE_FUNCTIONALITY;
  input_course_code = promptInput(F("ENTER COURSE CODE: "));
  // Serial.println(">> Funtion State: " + String(funcState));
  course_validation(current_filename, input_course_code);
  if (waitingFlag_2) {
    return;
  }  
  Serial.println(">> Course Code: " + input_course_code);
  funcState =  DEFAULT_STATE;


  waitingFlag_prompting = true;
  funcState = PASSWD_FUNCTIONALITY;
  input_course_passwd = promptInput(F("CREATE A PASSWORD: "));
  Serial.println(">> Course Protection Code: " + input_course_passwd);
  if (waitingFlag_2) {
    return;
  }
  lecturer_log_data(current_filename, input_course_code, input_course_passwd);

  funcState = DEFAULT_STATE;
  waitingFlag_2 = true;
  trig_mainMenu = '0';
}

//====Page to store students' details and fingerprint at first====e
void student_signup () {
  waitingFlag_2 = false;
  const char *current_filename = "matric_number-key.jsonl";
  // create_file("student_data.jsonl");
  waitingFlag_prompting = true;
  funcState = MATRIC_NUMBER_FUNCTIONALITY;
  lcd.clear();
  status_bar();
  // lcd.setCursor(0, 2);
  // lcd.print("     PAGE UNDER");
  // lcd.setCursor(0, 3); 
  // lcd.print("   CONSTRUCTION!");
  // delay(3000);
  matric_no = promptInput(F("Enter Matric No: "));
  Serial.println("Matriculation Number: " + matric_no);
  
  
  if (waitingFlag_2) {
    return;
  } 
	student_validation(current_filename, matric_no);
  if (waitingFlag_2) {
    return;
  }
  fingerprint_reg();  
  if (waitingFlag_2) {
    return;
  }
  student_log_data(current_filename, storedUID, matric_no);
  
  current_filename = ".UID-key.jsonl";
  student_log_data(current_filename, storedUID, matric_no);
  // funcState = DEFAULT_STATE;
 
  waitingFlag_2 = true;
  trig_mainMenu = '0';

}


//=====ATTENDANCE CAPTURE CATEGORY======
void capturing_fingerprint() {
  lcd.clear();
  status_bar();
}

//====Scanning Students' Fingerprint====
void attendance_capture() {
  lcd.clear();
  status_bar();
  lcd.setCursor(0, 2);
  lcd.print("PLACE YOUR HAND ON");
  lcd.setCursor(0, 3);
  lcd.print("THE SCANNER");
  lcd.setCursor(19, 3);
  lcd.blink();
}


//====Page for taking attendance====
void attendancePage() {
  lcd.clear();
  Serial.println(">> Inside Attendance Page...");
  status_bar();
  lcd.setCursor(0, 1);
  lcd.print("1. LECTURER SIGN IN");
  lcd.setCursor(0, 2);
  lcd.print("2. STUDENT SIGN IN");
  lcd.setCursor(0, 3);
  lcd.print("3. SYSTEM SETTINGS");
}


//====Lecturers' sign in page====
void lecturer_sign_in() {
  const char *current_file = "lecturer_data.jsonl";
  // subState = LECTURER_SIGN_IN;
  waitingFlag_2 = false;
  // waitingFlag_4 = false;
  waitingFlag_prompting = true;
  funcState = COURSE_CODE_FUNCTIONALITY;
  input_course_code = promptInput(F("ENTER COURSE CODE: "));
  Serial.println(">> Course Code: " + input_course_code);
  if (waitingFlag_2) {
    return;
  }
  course_validation(current_file, input_course_code);
  funcState =  DEFAULT_STATE;
  if (waitingFlag_2) {
    return;
  }
  waitingFlag_prompting = true;
  funcState = PASSWD_FUNCTIONALITY;
  input_course_passwd = promptInput(F("INPUT PASSWORD: "));
  Serial.println(">> Course Protection Code: " + input_course_passwd);
  funcState = DEFAULT_STATE;
  if (waitingFlag_2) {
    return;
  }
  // DateTime now = rtc.now();
  // char file_date [11]; 
  // sprintf(file_date, "%02d-%02d-%02d", now.day(), now.month(), now.year());
  course_validation(current_file, input_course_code, input_course_passwd);
  current_session = input_course_code + "_" + String(date);
  bool create_file_state = create_file((current_session  + ".csv").c_str());
  // Serial.println("Current session file: " + String(stored_filename));
  if (create_file_state) {
    Serial.println(">> Current Session: " + current_session);
  }
  else {
    Serial.println(">> Attendance has been taken for this course today");
    return;
  }
  // lcd.clear();
  // Serial.println(">> Lecturer sign in page running...");
  // status_bar();
}



void attendancePage_mod() {
  lcd.clear();
  Serial.println(">> Inside Attendance Mod Page...");
  status_bar();
  lcd.setCursor(0, 1);
  lcd.print("1. CURRENT SESSION");
  lcd.setCursor(0, 2);
  lcd.print("2. STUDENT SIGN IN");
  lcd.setCursor(0, 3);
  lcd.print("3. SYSTEM SETTINGS");
}

//====Student sign in page====
void student_sign_in() {
  waitingFlag_2 = false;
  const char *current_filename = ".UID-key.jsonl";
  Serial.println(">> Student sign in page running...");
  subState = STUDENT_SIGN_IN;
  lcd.clear();
  status_bar();
  // lcd.setCursor(0, 2);
  // lcd.print("     PAGE UNDER");
  // lcd.setCursor(0, 3); 
  // lcd.print("   CONSTRUCTION!");
  // delay(3000);
  fingerprint_recog();  
	if (waitingFlag_2) {
    attendancePage_mod();
    subState = ATTENDANCE_PAGE_MENU;
		return;
  }
  lcd.setCursor(0, 1);
  lcd.print("                   ");
  lcd.setCursor(0, 2);
  lcd.print("     VERIFYING");
  lcd.setCursor(0, 3);
  lcd.print("   STUDENTSHIP...");
  Serial.println(">> Retrieved UID: " + retrievedUID);
  student_validation(current_filename,"" ,retrievedUID);
  Serial.println(">> Recroded matricualtion number: " + recorded_matric_no);
  
  String attendance_filename = current_session + ".csv";
  Serial.println(">> Current session: " + attendance_filename);
  attendance_log_data((attendance_filename).c_str(), recorded_matric_no);
  // delay(2000);
  attendancePage_mod();
  subState = ATTENDANCE_PAGE_MENU;
}


//=====SYSTEM SETTINGS CATEGORY=====
void system_settings(){
  lcd.clear();
  Serial.println(">> Settings Menu Displayed");
  status_bar();
  // subState = SYSTEM_SETTINGS;
  if (!hotspot_state) {
    lcd.setCursor(0, 1);
    lcd.print("1. HOTSPOT       OFF");
  }
  else if (hotspot_state) {
    lcd.setCursor(0, 1);
    lcd.print("1. HOTSPOT        ON");
  }
  lcd.setCursor(0, 2);
  lcd.print("2. SET TIMER");
  lcd.setCursor(0, 3);
  if (!keypad_enable) {
    lcd.print("3. END SESSION");
  }
  else {
  lcd.print("3. OTHER SETTINGS");  
  }

  refresh_screen = false;  
}

//====Bluetooth settings for tansferring the marked attendance====
void hotspot_settings() {
  Serial.println(">> Inside hotspot settings...");
  lcd.clear();
  status_bar();
  subState = HOTSPOT_SETTINGS;
  
  lcd.setCursor(0, 2);
  lcd.print("1. [ON]    2. [OFF]");



  // if (funcState == HOTSPOT_OFF) {
  //   lcd.setCursor(0, 2);
  //   lcd.print("1. [ON]");
  //   lcd.print("  2. [OFF]");
  // }
  // else if (funcState == HOTSPOT_ON) {
  //   lcd.setCursor(0, 2);
  //   lcd.print("1. [OFF]");
  //   lcd.print(" 2. [BACK]");
  // }
  // else {
  //   return;
  // }
}

//====For setting the duration in which attendance can be taken====
void set_timer() {
  Serial.println(">> Inside timer settings");
  // subState = TIMER;  //Redunadnt : using promptInput and funcState 
  waitingFlag_prompting = true;
  funcState = TIMER_FUCNTIONALITY;
  // set_timer();
  timer_input = promptInput("SET TIMER IN MINS: ");
  Serial.println(">> Stored timer: " + String(timer_input));
  int timer_input_int = timer_input.toInt();
  Serial.println(">> Converter stored timer: " + String(timer_input_int));
  timer = timer_input_int * 60000;
  Serial.println(">> Calculated Final Timer: " + String(timer));
  timer_prev = timer/60000;
  Serial.println(">> Previously set time: " + String(timer_prev));
  Serial.println(">> Timer set to " + String(timer/60000) + "mins");
  system_settings();
  subState = SYS_SETTINGS_MENU;
}


void end_session() {
  Serial.println(">> Inside cancel session settings...");
  lcd.clear();
  status_bar();
  subState = END_SESSION;
  lcd.setCursor(0, 2);
  lcd.print("   ARE YOU SURE?   ");
  lcd.setCursor(0, 3); 
  lcd.print("1. [YES]    2. [NO]");
	Serial.println(">> Do you want to end the current session?");
	Serial.println(">> Presss '1' to accept, Press '2' to cancel");
}


//====Contains Date & time and Change password settings=====
void other_settings() {
  Serial.println(">> Inside OTHER settings...");
  lcd.clear();
  status_bar();
  // subState = OTHER_SETTINGS;
  lcd.setCursor(0, 1);
  lcd.print("1. DATE AND TIME");
  lcd.setCursor(0, 2);
  lcd.print("2. CHANGE PASSWORD");
  // refresh_screen = false; 
}


void date_and_time () {
  Serial.println(">> Inside  date and time settings");
  lcd.clear();
  status_bar();
  subState = DATE_TIME;
  lcd.setCursor(0, 2);
  lcd.print("     PAGE UNDER");
  lcd.setCursor(0, 3); 
  lcd.print("   CONSTRUCTION!");
}


void change_passwd() {
  Serial.println(">> Inside  password settings");
  lcd.clear();
  status_bar();
  subState = PASSWORD;
  lcd.setCursor(0, 2);
  lcd.print("     PAGE UNDER");
  lcd.setCursor(0, 3); 
  lcd.print("   CONSTRUCTION!");
}



