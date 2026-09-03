#include "esp32-hal.h"
#include "systemGlobal.hpp" .
#include <SPI.h>
#include <SdFat.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>

#define SD_MISO 27
#define SD_MOSI 32
#define SD_SCK 33
#define SD_CS 26

extern NetworkServer server; 
const char *ssid = "ATTENDANCE DEVICE";
const char *password = "12345678";

bool sdCardFlag;
bool course_passwd_matched =  false;


//======SD card objects======
extern SdFat sd;
extern File32 dataFile;
// extern const char* filename;
// extern const char* filename2;

// We use 4MHz for stability on breadboards (4000000).
#define SPI_CONFIG SdSpiConfig(SD_CS, SHARED_SPI, SD_SCK_MHZ(4))

void SD_module_check() {
  if (!sd.begin(SPI_CONFIG)) {
    sdCardFlag = true;
    lcd.setCursor(0, 1);
    lcd.print(F("Couldn't find SD"));
    digitalWrite(check_LED, HIGH);
    Serial.println(">> Couldn't find SD module");
    delay(1000);
  } 
  else {
    sdCardFlag = false;
    lcd.setCursor(0, 1);
    lcd.print(F("SD module Init..."));
    sdCardFlag = true;
    Serial.println(">> SD module Initialized successfully");
    delay(1000);
  }
}   


void hotspot_setup() {
    Serial.println(">> Starting Wi-Fi Access Point...");
    WiFi.mode(WIFI_AP);

    WiFi.softAP(ssid, password,1, 0, 1);

    // 3. CRITICAL: Give the internal network stack 100 milliseconds to wake up and build its mutex locks
    delay(100);

    GH_IP = WiFi.softAPIP();
    Serial.print(">> AP IP address: ");
    Serial.println(GH_IP);

    server.begin();
    Serial.println(">> Server started");
}

void hotspot_off() {
  if (!hotspot_state) {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
  }
}


bool create_file(const char *filename, const char *filename_2) {
  if (sdCardFlag) {
    Serial.println(">> Creating file...");
  
    if (sd.exists(filename) && subState == LECTURER_SIGN_IN) {
      lcd.clear();
      lcd.setCursor(0, 2);
      lcd.print(" ATTENDANCE ALREADY");
      lcd.setCursor(0, 3);
      lcd.print("    TAKEN FOR TODAY");
      delay(3000);
      return false;
    }
    if (dataFile.open(filename, O_RDWR | O_CREAT | O_AT_END)){
      Serial.println(">> SUCCESS: File is open and ready.");
      dataFile.sync(); 
      dataFile.close(); // Always close after setup
      Serial.println(">> SUCCESS: File synced and safely closed.");
      Serial.println((">> File name: " + String(filename)));
      
    } 
    if (dataFile.open(filename_2, O_RDWR | O_CREAT | O_AT_END)) {
      Serial.println(">> SUCCESS: File is open and ready.");
      dataFile.sync(); 
      dataFile.close(); // Always close after setup
      Serial.println(">> SUCCESS: File synced and safely closed.");
      Serial.println((">> File name: " + String(filename_2)));
    } 

  }
  else { 
    Serial.println(">> FAILURE: Could not open file.");
    sd.errorPrint(&Serial); // Prints WHY the file open failed
    return false;
  }
}

//=====Logs Course code and course password in a Json file on the Sd card=====
void lecturer_log_data(const char *filename, String course_code, String course_passwd) {
    JsonDocument lecturer_data;
    // String course_code = "220305010001";
    // String course_passwd = "12345678";

    lecturer_data[course_code] = course_passwd;

    
    JsonVariantConst variant = lecturer_data.as<JsonVariantConst>();
    serializeJson(variant, Serial);
    Serial.println("\n");
    Serial.println("Current file: " + String(filename));

    if (dataFile.open(filename, O_RDWR | O_AT_END)) {
        serializeJson(variant, dataFile);
        dataFile.println();
        //delay(100);
        dataFile.sync();
        dataFile.close();  
        lcd.setCursor(0,3);
        lcd.print(F("    INFO SAVED!    "));
        delay(2000);
        Serial.println(">> Lecturer data succefully written");
        Serial.println(">> File succefully closed");
    } 
    else { 
        lcd.setCursor(0, 3); 
        lcd.print("   SD CARD ERROR!  ");
        Serial.println(">> Lecturer data was not written");
        Serial.println(">> Check SD card");
        delay(2000);
    }
}


void student_log_data(const char *filename, String UID , String matric_number) {
    JsonDocument student_data;
    
    if (filename == "matric_number-key.jsonl") {
        student_data[matric_number] = UID;
    }
    else {
        student_data[UID] = matric_number;
    }
    JsonVariantConst variant = student_data.as<JsonVariantConst>();
    serializeJson(variant,Serial);
    Serial.println();
    
    if (dataFile.open(filename, O_RDWR | O_AT_END)) {
        serializeJson(variant, dataFile);
        dataFile.println();
        dataFile.sync();
        dataFile.close();  
        lcd.clear();
        status_bar();
        lcd.setCursor(0, 2);
        lcd.print(F("    INFO SAVED!    "));
        delay(2000);
        Serial.println(">> Student data succefully written");
        Serial.println(">> File succefully closed");
    }
    else {
        lcd.clear();
        status_bar();
        lcd.setCursor(0, 2); 
        lcd.print("   SD CARD ERROR!  ");
        Serial.println(">> Student data was not written");
        Serial.println(">> Check SD card");
        delay(2000);
    }
}



bool course_validation(const char *filename, String inputCourse, String inputPassword) {
  Serial.println(">> Running course code validation...");
  String line;
  JsonDocument lecturer_data;
  bool courseFound;   
  

  if (dataFile.open(filename, O_READ)) {

    // Loop through the file one line at a time
    while (dataFile.available()) {
      line = dataFile.readStringUntil('\n');
      
      // Parse the single line: {"PHY401": "pass123"}
      DeserializationError error = deserializeJson(lecturer_data, line);
      if (error) {
        continue; // Skip broken lines
          
      }
      // Check if this specific line contains the course we want
      if (!lecturer_data[inputCourse].isNull()) {
        if (mainState == LECTURER_SIGN_UP) {
          dataFile.close();
          lcd.clear();
          status_bar();
          lcd.setCursor(0, 2);
          lcd.print("   COURSE ALREADY");
          lcd.setCursor(0, 3);
          lcd.print("       EXISTS!");
          Serial.println(">> Course already exists!");
          delay(3000);
          waitingFlag_2 = true;
          return true;
        }
          
        courseFound = true;
        Serial.println(">> Course found in database");
        // The course is on this line! Now check the password.
        if (lecturer_data[inputCourse] == inputPassword) {
          if (subState == LECTURER_SIGN_IN) {
            dataFile.close();
            lcd.setCursor(0, 1);
            lcd.print("    COURSE FOUND   ");
            lcd.setCursor(0, 2);
            lcd.print("   ATTENDANCE IN   ");
            lcd.setCursor(0, 3); 
            lcd.print("      PROGRESS     ");
            Serial.println("Access Granted!");
            timer = 600000;
            timer_prev = 0;
            delay(3000);
            waitingFlag_4 = true;
            keypad_enable = false;
            return true;
          }
          // course_passwd_matched = true;
          dataFile.close();
          Serial.println(">> Webpage course code and course password matched with database");
          return true;

        } 
        
        else {
          if (subState == LECTURER_SIGN_IN && inputPassword == "") {
            dataFile.close();
            Serial.println(">> Signing in a course...");
            waitingFlag_4 = false; // Just to be safe
            return true;
          }
          if (subState == LECTURER_SIGN_IN) {
            dataFile.close();
            lcd.setCursor(0, 1);
            lcd.print(F("                   "));
            lcd.setCursor(0, 2);
            lcd.print(F("                   "));
            lcd.setCursor(0, 2);
            lcd.print("  WRONG PASSWORD!  ");
            dataFile.close();
            delay(2000);
            waitingFlag_4 = false; // Just to be safe 
            Serial.println("Access Denied: Wrong Password.");
            return false;
          }
          // course_passwd_matched = false;
          dataFile.close();
          Serial.println(">> Webpage course code found but password wrong");
          return false;
        }
        dataFile.close();
      }
        
    }
    if (!courseFound) {
      if (mainState == LECTURER_SIGN_UP) {
        Serial.println(">> Registering new course...");
        waitingFlag_4 = false; // Just to be safe 
        dataFile.close();
        return true;
      }
      if (subState == LECTURER_SIGN_IN) {    
        dataFile.close();
        lcd.setCursor(0, 1);
        lcd.print(F("                   "));
        lcd.setCursor(0, 2);
        lcd.print(F("                   "));
        lcd.setCursor(0, 2);
        lcd.print(" COURSE NOT FOUND  ");
        lcd.setCursor(0, 3);
        lcd.print("    IN DATABASE    ");
        delay(2000);
        waitingFlag_4 = false; // Just to be safe 
        waitingFlag_2 = true;              
      }
      Serial.println(">> Course not found in database");
      return false;   
    }

  }
  else {
    if (mainState == LECTURER_SIGN_UP || subState == LECTURER_SIGN_IN) {
      lcd.clear();
      status_bar();
      lcd.setCursor(0, 2);
      lcd.print("   CHECK SD CARD!");
      delay(3000);
      waitingFlag_2 = true;
      return false;
    }
    Serial.println("Database not found.");
    return false;
  }
}


// bool student_validation(const char *filename,String input_matric_no, String UID) {
//     Serial.println(">> Running student validation...");
//     // String line;
//     JsonDocument student_data;
//     bool matricFound = false;    
//     bool UIDfound = false;

//     if (dataFile.open(filename, O_READ)) {
//         if (String(filename) == "matric_number-key.jsonl" || String(filename) == "UID-key.jsonl") {
//             Serial.print(">> Current file open: "); 
//             Serial.println(String(filename));
//             // Loop through the file one line at a time
//             while (dataFile.available()) {
//                 // line = dataFile.readStringUntil('\n');
                
//                 // Parse the single line: {"PHY401": "pass123"}
//                 DeSerializationError error = deSerializeJson(student_data, dataFile);
//                 if (error) {
//                     continue; // Skip broken lines
//                 }
//                 // Check if this specific line contains the course we want
//                 if (String(filename) == "matric_number-key.jsonl") {
//                     if (!student_data[input_matric_no].isNull()) {
                        
//                         dataFile.close();
//                         lcd.clear();
//                         status_bar();
//                         lcd.setCursor(0, 2);
//                         lcd.print("MATRICULATION NUMBER");
//                         lcd.setCursor(0, 3);
//                         lcd.print("  ALREADY EXISTS!");
//                         Serial.print(">> Matriculation number: ");
//                         Serial.print(input_matric_no);
//                         Serial.println(" already exists!");
//                         delay(3000);
//                         waitingFlag_2 = true;
//                         return true;
//                     }
//                 }

//                 else if  (!student_data[UID].isNull()) {
//                     // const char* raw_value = student_data[String(UID)].as<const char*>();
//                     // if (raw_value != nullptr) {
//                     //     recorded_matric_no = raw_value;
//                     // }
//                     recorded_matric_no = student_data[UID];
//                     // recorded_matric_no = student_data[String(UID)].as<String>();
//                     // recorded_matric_no = String(student_data[UID].as<String>());
//                     dataFile.close();
//                     Serial.print(">> Fingerprint matched with ");
//                     Serial.print(recorded_matric_no);
//                     Serial.println(" in database");
//                     return true;
                        

//                 }

//                 // else {
//                 //     dataFile.close();
//                 //     // return true;
//                 // }
                 
//                 // else if (student_data[UID] == input_matric_no) {
//                 //     lcd.clear();
//                 //     status_bar();
//                 //     lcd.setCursor(0 , 2);
//                 //     lcd.print("   ATTENDACE TAKEN");
//                 //     Serial.println(">> Attendance has been taken");
//                 //     delay(3000);
//                 // // }
//                 // waitingFlag_2 = true;
//                 // return true; 
//                 // } 
//                 // dataFile.close();
//             }
//             if (mainState == STUDENT_SIGN_UP) {
//                 if (!matricFound) {
//                     Serial.println(">> Registering new course...");
//                     waitingFlag_4 = false; // Just to be safe 
//                     dataFile.close();
//                     return true;
//                 }
//             }
//             else if (!UIDfound) {
//                 dataFile.close();
//                 lcd.clear();
//                 status_bar();
//                 lcd.setCursor(0 , 1);
//                 lcd.print(" RECOGNITION FAILED");
//                 lcd.setCursor(0, 2);
//                 lcd.print("    TRY AGAIN OR");
//                 lcd.setCursor(0, 3);
//                 lcd.print("  GO AND REGISTER");
                
//                 delay(3000);
//                 Serial.println(">> Recognition failed, try again or go and register");
//                 waitingFlag_2 = true;
//                 return true;
//             }
//             else {
//                 dataFile.close();
//                 return true;
//             }
//         }
// 	}
// }




bool student_validation(const char *filename, String input_matric_no, String UID) {
    Serial.println(">> Running student validation...");
    JsonDocument student_data;
    bool matricFound = false;  
    bool UIDfound = false;
    
    if (dataFile.open(filename, O_READ)) {
        // Fix: Cast filename to String so it compares the text, not the memory address
        if (String(filename) == "matric_number-key.jsonl" || String(filename) == ".UID-key.jsonl") {
            Serial.print(">> Current file open: "); 
            Serial.println(filename);
            
            // Loop through the file one line at a time
            while (dataFile.available()) {
                
                // Parse the single line
                DeserializationError error = deserializeJson(student_data, dataFile);
                if (error) {
                    continue; // Skip broken lines and keep searching
                }
             
                // --- MATRIC VALIDATION CHECK ---
                if (String(filename) == "matric_number-key.jsonl") {
                    if (!student_data[input_matric_no].isNull()) {
                        
                        dataFile.close(); // Match found! Safe to close.
                        
                        lcd.clear();
                        status_bar();
                        lcd.setCursor(0, 2);
                        lcd.print("MATRICULATION NUMBER");
                        lcd.setCursor(0, 3);
                        lcd.print("  ALREADY EXISTS!");
                        
                        Serial.print(">> Matriculation number: ");
                        Serial.print(input_matric_no);
                        Serial.println(" already exists!");
                        
                        delay(3000);
                        waitingFlag_2 = true;
                        return true; 
                    }
                }

                // --- UID VALIDATION CHECK ---
                else if (!student_data[UID].isNull()) {
                    
                    const char* raw_value = student_data[String(UID)].as<const char*>();
                    if (raw_value != nullptr) {
                        recorded_matric_no = raw_value;
                    }
                    
                    dataFile.close(); // Match found! Safe to close.

                    Serial.print(">> Fingerprint matched with ");
                    Serial.print(recorded_matric_no);
                    Serial.println(" in database");
                    return true;
                }
            }
            dataFile.close();

            // Handle the "Not Found" outcomes based on the system state
            if (mainState == STUDENT_SIGN_UP) {
                if (!matricFound) {
                    Serial.println(">> Registering new student...");
                    waitingFlag_4 = false; 
                    return true; 
                }
            }
            else { // Assuming Recognition Failed
                // lcd.clear();
                // status_bar();
                // lcd.setCursor(0 , 1);
                // lcd.print(" RECOGNITION FAILED");
                // lcd.setCursor(0, 2);
                // lcd.print("    TRY AGAIN OR");
                // lcd.setCursor(0, 3);
                // lcd.print("  GO AND REGISTER");
                
                // delay(3000);
                Serial.println(">> Recognition failed, try again or go and register");
                waitingFlag_2 = true;
                return true; 
            }
            
        } else {
            Serial.println(">> File was not found");
            // Close the file if the filename didn't match the required ones
            dataFile.close();
        }
    } 
    else {
        Serial.println(">> Error: Could not open file.");
    }
    
    return false;
}

void attendance_log_data(const char *filename, String recorded_matric) {
    Serial.println(">> Current file: " + String(filename));
    String Header;
    String dataString = String(date) + "," + String(timeFull) + "," + recorded_matric;
    if (dataFile.open(filename, O_RDWR | O_AT_END)) {
        if (dataFile.fileSize() == 0) {
            Header = dataFile.println("Date,Time,Matriculation_number");
            Serial.print("Header writen: ");
            Serial.println(Header);
        }
        dataFile.println(dataString);
        dataFile.close();
        lcd.clear();
        status_bar();
        lcd.setCursor(0, 2);
        lcd.print(" MATRIC " + recorded_matric_no);
        lcd.setCursor(0 , 3);
        lcd.print("   ATTENDACE TAKEN");      
        delay(3000);
    }
}


void erasing_student_records() {
  Serial.println(">> Wiping students records...");


  if (sd.exists("/.UID-key.jsonl") && sd.exists("/matric_number-key.jsonl")) {
    sd.remove("/.UID-key.jsonl");
    sd.remove("/matric_number-key.jsonl");
    Serial.println(">> Successfully wiped students records!");
  }
  else {
    Serial.println(">> Database was already empty or missing.");
  }
}


//====for wiping a course and course password fromthe SD card===
void erasinG_lecturer_records () {
  Serial.println(">> Wiping course records...");


  if (sd.exists("/lecturer_data.jsonl")) {
    sd.remove("/lecturer_data.jsonl");
    Serial.println(">> Successfully wiped course records!");
  }
  else {
    Serial.println(">> Database was already empty or file was missing.");
  }
}


const char html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Attendance System</title>
    <style>
        :root {
            --bg-light: #ffffff;
            --bg-dark: #000000;
            --frame-bg: #0a0a0a;
            --text-main: #ffffff;
            --text-muted: #888888;
            --input-bg: #141414;
            --border-color: #2a2a2a;
            --accent: #ffffff;
            --accent-text: #000000;
        }
        
        body {
            margin: 0;
            padding: 0;
            font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            background: linear-gradient(135deg, #ffffff 0%, #737373 50%, #000000 100%);
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            animation: fadeIn 0.8s ease-out;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(-10px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .container {
            background: var(--frame-bg);
            border: 1px solid var(--border-color);
            padding: 3rem 2.5rem; 
            border-radius: 40px; 
            box-shadow: 0 30px 60px rgba(0, 0, 0, 0.4);
            width: 85%;
            max-width: 380px;
            text-align: center;
        }

        .brand-header {
            font-size: 1.8rem;
            font-weight: 900;
            color: var(--text-main);
            margin-top: 0;
            margin-bottom: 0.2rem;
            letter-spacing: 2px;
            text-transform: uppercase;
        }

        .sub-header {
            font-size: 1rem;
            color: var(--text-muted);
            margin-bottom: 2.5rem;
            font-weight: 500;
            letter-spacing: 1px;
            text-transform: uppercase;
        }

        .input-group {
            position: relative;
            margin-bottom: 20px;
        }

        .input-box {
            width: 100%;
            padding: 16px 24px;
            background: var(--input-bg);
            border: 2px solid var(--border-color);
            border-radius: 50px; 
            font-size: 1rem;
            color: var(--text-main);
            box-sizing: border-box;
            outline: none;
            transition: all 0.3s ease;
        }

        #course_password {
            padding-right: 85px;
        }

        .input-box::placeholder {
            color: #666666;
        }

        .input-box:focus {
            border-color: var(--accent);
            background: #000000;
        }

        .pw-toggle {
            position: absolute;
            right: 15px;
            top: 50%;
            transform: translateY(-50%);
            background: #222222;
            border: none;
            border-radius: 30px;
            color: var(--text-main);
            font-size: 0.7rem;
            font-weight: 800;
            letter-spacing: 1px;
            cursor: pointer;
            padding: 8px 14px;
            transition: all 0.2s ease;
            outline: none;
        }

        .pw-toggle:hover {
            background: var(--accent);
            color: var(--accent-text);
        }

        .btn {
            display: block;
            width: 100%;
            padding: 18px;
            background: var(--accent);
            color: var(--accent-text);
            border: none;
            border-radius: 50px; 
            font-size: 1.1rem;
            font-weight: 900;
            cursor: pointer;
            transition: all 0.2s ease;
            margin-top: 30px;
            text-transform: uppercase;
            letter-spacing: 1.5px;
        }

        .btn:active {
            transform: scale(0.96);
            background: #cccccc;
        }

        .secure-note {
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 0.8rem; 
            color: var(--text-muted);
            margin-top: 20px; 
            letter-spacing: 1px;
            font-weight: 700;
            text-transform: uppercase;
        }

        .secure-note svg {
            margin-right: 8px;
        }

        .footer {
            margin-top: 4rem; 
            padding-top: 1.5rem;
            border-top: 1px solid var(--border-color);
        }

        .copyright {
            display: block;
            font-size: 0.8rem;
            color: #555555;
            font-weight: 900; 
            letter-spacing: 1px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="brand-header">ATTENDANCE SYSTEM</div>
        <div class="sub-header">Secure Record Retrieval</div>
        
        <form action="/confirm" method="GET">
            <div class="input-group">
                <input type="text" name="course_code" class="input-box" placeholder="Enter course code" required autocomplete="off">
            </div>
            
   <div class="input-group">
                <input type="text" name="course_date" class="input-box" placeholder="Session date (DD-MM-YYYY)" required autocomplete="off" pattern="\d{2}-\d{2}-\d{4}" title="Format must be exactly DD-MM-YYYY">
            </div>

            <div class="input-group">
                <input type="password" name="course_password" id="course_password" class="input-box" placeholder="Enter course password" required>
                <button type="button" class="pw-toggle" id="pwToggle">SHOW</button>
            </div>
            <button type="submit" class="btn">CONFIRM</button>
            
            <div class="secure-note">
                <svg viewBox="0 0 24 24" width="14" height="14" stroke="currentColor" stroke-width="2" fill="none" stroke-linecap="round" stroke-linejoin="round">
                    <rect x="3" y="11" width="18" height="11" rx="2" ry="2"></rect>
                    <path d="M7 11V7a5 5 0 0 1 10 0v4"></path>
                </svg>
                RESTRICTED LECTURER ACCESS
            </div>
        </form>
        
        <div class="footer">
            <span class="copyright">&copy; SOFTWARE BY ASTROJOE</span>
        </div>
    </div>

    <script>
        const pwInput = document.getElementById('course_password');
        const pwToggle = document.getElementById('pwToggle');

        pwToggle.addEventListener('click', function() {
            if (pwInput.type === 'password') {
                pwInput.type = 'text';
                pwToggle.textContent = 'HIDE';
                pwToggle.style.background = '#ffffff';
                pwToggle.style.color = '#000000';
            } else {
                pwInput.type = 'password';
                pwToggle.textContent = 'SHOW';
                pwToggle.style.background = '#222222';
                pwToggle.style.color = '#ffffff';
            }
        });
    </script>
</body>
</html>
)rawliteral";

//======Handles webpage connection and display======
void webpage_connection () {
  NetworkClient client = server.accept(); 
//   Serial.println(">> Waiting for a device to connect...");

  if (client) {

    Serial.println(">> New browser connection!");  
      
    // Set a strict 2-second timeout so a dead connection doesn't lock up your ESP32
    client.setTimeout(2000); 

    while (client.connected()) {    
      if (client.available()) {     
        // Read an entire line at once from the network buffer
        String currentLine = client.readStringUntil('\n');
        Serial.println(">> HTTP GET request: " + currentLine);

        // --- THE NEW BACKEND FILE DOWNLOADER ---
        if (currentLine.indexOf("GET /confirm?") >= 0) {
            
          // 1. Slice out the Course Code
          int codeStart = currentLine.indexOf("course_code=") + 12;
          int codeEnd = currentLine.indexOf("&course_date=");
          String courseCode = currentLine.substring(codeStart, codeEnd);

          // 2. Slice out the Date
          int dateStart = currentLine.indexOf("course_date=") + 12;
          int dateEnd = currentLine.indexOf("&course_password=");
          String courseDate = currentLine.substring(dateStart, dateEnd);

          // 3. Slice out the Password
          int passStart = currentLine.indexOf("course_password=") + 16;
          int passEnd = currentLine.indexOf(" HTTP");
          String coursePassword = currentLine.substring(passStart, passEnd);

          Serial.print(">> Request for Course: ");
          Serial.print(courseCode);
          Serial.print(" on ");
          Serial.println(courseDate);

          // 3. Authenticate (You can build a more complex password system later)
          bool webpage_input_result = course_validation("lecturer_data.jsonl", courseCode, coursePassword);
          // if (coursePassword == "admin123") { 
          if (webpage_input_result) {    
            // Construct the expected filename (e.g., "/PHY401.csv")
            String filePath = "/" + courseCode + "_" + courseDate + ".csv";
            Serial.print(">> File path: ");
            Serial.println(filePath);
            
            File dataFile = sd.open(filePath);
          
            if (dataFile) {
         
              Serial.println(">> File found! Sending to phone...");
          
              // Send special HTTP headers that force the phone to download a file
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/csv");
          
              // This tells the browser the exact name to save the file as
              client.print("Content-Disposition: attachment; filename=\"");
              client.print(courseCode + "_" + courseDate);
              client.println("_Attendance.csv\"");
          
              client.println("Connection: close");
              client.println(); // Blank line means headers are done

              // CRITICAL: Stream the file byte-by-byte. 
              // Never load the whole file into a String, or the ESP32 will crash!
              while (dataFile.available()) {
                client.write(dataFile.read());
              }
            
              dataFile.close();
              Serial.println(">> Download complete.");
              
            } 
            else {
              // File does not exist on the SD card
              client.println("HTTP/1.1 404 Not Found");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              client.println("<h1 style='color:red; text-align:center;'>Error 404: File Not Found</h1>");
              client.println("<p style='text-align:center;'>No records exist for this course yet.</p>");
              client.println();
            }
          } 
          else {
            // Wrong Password
            client.println("HTTP/1.1 403 Forbidden");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<h1 style='color:red; text-align:center;'>Error 403: Access Denied</h1>");
            client.println("<p style='text-align:center;'>Incorrect course password.</p>");
            client.println();
          }
        
          break; // Stop parsing and close the connection
        }
        // ---------------------------------------

        // If the line is empty, the browser is just asking for the main page
        if (currentLine == "\r" || currentLine.length() == 0) {
          // ... (Your code that sends the html_page goes here)

        
          // Read an entire line at once from the network buffer
          String currentLine = client.readStringUntil('\n');

          // An empty line (just a carriage return remaining) means the browser is done asking.
          // Now it is our turn to send the web page back!
          if (currentLine == "\r" || currentLine.length() == 0) {
            Serial.println(">> Sending HTTP headers...");
            // 1. Send standard HTTP headers
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close"); // Tell phone to drop connection when done
          

            // 2. Send the actual HTML code for the web page

            client.println(FPSTR(html_page));
          
            // 3. The HTTP response always ends with another blank line
            client.println();
            Serial.println(">> HTML page successfully sent to browser!");
            break; // Break out immediately
          }
        }
      }
    }
    // Close the connection clean and flat
    client.stop();
    Serial.println(">> Web page sent. Client disconnected.");
  }
}






// bool student_validation(const char *filename, String UID) {
//     Serial.println(">> Running student validation...");
//     String line;
//     JsonDocument student_data; 
//     bool UIDfound = false;

//     if (dataFile.open(filename, O_READ)) {
//         if (filename == "UID-key.jsonl") {
//             Serial.println(">> Current file open: " + String(filename));
//             // Loop through the file one line at a time
//             while (dataFile.available()) {
//                 line = dataFile.readStringUntil('\n');
                
//                 // Parse the single line: {"PHY401": "pass123"}
//                 DeSerializationError error = deSerializeJson(student_data, line);
//                 if (error) {
//                     continue; // Skip broken lines
//                 }
//                 // Check if this specific line contains the course we want
//                 if (!student_data[UID].isNull()) {
//                     dataFile.close();
//                     if (student_data[UID] == input_matric_no) {
//                         lcd.clear();
//                         status_bar();
//                         lcd.setCursor(0 , 2);
//                         lcd.print("   ATTENDACE TAKEN");
//                         Serial.println(">> Attendance has been taken");
//                         delay(3000);
//                     }
//                     waitingFlag_2 = true;
//                     return true;
                    
//                 }
//             }
//             else if (!UIDfound) {
//                 lcd.clear();
//                 status_bar();
//                 lcd.setCursor(0 , 1);
//                 lcd.print(" RECOGNITION FAILED");
//                 lcd.setCursor(0, 2);
//                 lcd.print("    TRY AGAIN OR");
//                 lcd.setCursor(0, 3);
//                 lcd.print("  GO AND REGISTER");
//                 Serial.println(">> Recognition failed, try again or go and register");
//                 delay(3000);
//             }
//         }
// 	}
// }









//         if (filename == "UID-key.jsonl") {
//             while (dataFile.available()) {
//                 line = dataFile.readStringUntil('\n');
                
//                 // Parse the single line: {"PHY401": "pass123"}
//                 DeSerializationError error = deSerializeJson(student_data, line);
//                 if (error) {
//                     continue; // Skip broken lines
//                 }
//                 // Check if this specific line contains the course we want
//                 if (student_data.containsKey(input_matric_no)) {
                    
//                     dataFile.close();
//                     lcd.clear();
//                     status_bar();
//                     lcd.setCursor(0, 2);
//                     lcd.print("MATRICULATION NUMBER");
//                     lcd.setCursor(0, 3);
//                     lcd.print("  ALREADY EXISTS!");
//                     Serial.println(">> Matriculation number already exists!");
//                     delay(3000);
//                     waitingFlag_2 = true;
//                     return true;
//                 }
                
//             }
//             if (!courseFound) {
//                 Serial.println(">> Registering new course...");
//                 waitingFlag_4 = false; // Just to be safe 
//                 dataFile.close();
//                 return true;
//             } 
//         }


//     }
//     else {
//         lcd.clear();
//         status_bar();
//         lcd.setCursor(0, 2);
//         lcd.print("   CHECK SD CARD!");
//         Serial.println("Database not found.");
//         delay(3000);
//         waitingFlag_2 = true;
//         return false;
//     }
// }


// dataFile.close();
// lcd.setCursor(0, 1);
// lcd.print(F("                   "));
// lcd.setCursor(0, 2);
// lcd.print(F("                   "));
// lcd.setCursor(0, 2);
// lcd.print(" COURSE NOT FOUND  ");
// lcd.setCursor(0, 3);
// lcd.print("    IN DATABASE    ");
// delay(2000);
// Serial.println(">> Course not found in database");
// waitingFlag_4 = false; // Just to be safe 
// waitingFlag_2 = true;
// return false;   


// student_data.
// Serial.println(">> Matriculation number found in database");
// matricFound = true;
// // The course is on this line! Now check the password.
// if (student_data[Inputmatric_number] == UID) {
//     dataFile.close();
//     // lcd.setCursor(0, 1);
//     // lcd.print("MATRICULATION NUMBER F");
//     // lcd.setCursor(0, 2);
//     // lcd.print("   ATTENDANCE IN   ");
//     // lcd.setCursor(0, 3); 
//     // lcd.print("      PROGRESS     ");
//     // Serial.println("Access Granted!");

//     // delay(3000);
//     waitingFlag_4 = true;

//     return true;
// } 
// else {
//     if (subState == LECTURER_SIGN_IN && inputPassword == "") {
//         Serial.println(">> Signing in a course...");
//         waitingFlag_4 = false; // Just to be safe 
//         dataFile.close();         
//         return true;
//     }
//     lcd.setCursor(0, 1);
//     lcd.print(F("                   "));
//     lcd.setCursor(0, 2);
//     lcd.print(F("                   "));
//     lcd.setCursor(0, 2);
//     lcd.print("  WRONG PASSWORD!  ");
//     dataFile.close();
//     delay(2000);
//     waitingFlag_4 = false; // Just to be safe 
//     Serial.println("Access Denied: Wrong Password.");
//     return false;
// }
// dataFile.close();

