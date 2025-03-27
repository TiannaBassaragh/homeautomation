//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################

 

// IMPORT ALL REQUIRED LIBRARIES
#include <rom/rtc.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include <ArduinoJson.h>
#include <SPI.h> //to interface with devices over the SPI bus


//IMPORT IMAGES
#include "lockclose.h"
#include "lockopen.h"


#ifndef _WIFI_H 
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
 


// DEFINE VARIABLES
#define BTN1 25
#define BTN2 26
#define BTN3 27

#define analogPin 33 //for potentiometer

//TFT Display Pin connections
#define TFT_CS    16
#define TFT_RST   17
#define TFT_DC    5
#define TFT_MOSI  23
#define TFT_SCK   19
#define TFT_MISO  18

//Display parameters
#define BOX_WIDTH 50
#define BOX_HEIGHT 50
#define SPACE 5
#define MARGIN 10

//TFT Display info
#define TFT_BG ILI9341_BLACK
#define TFT_BOX ILI9341_PURPLE
#define TFT_DIGIT ILI9341_CYAN


// IMPORT FONTS FOR TFT DISPLAY
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h> 


// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620162297";                    // Add your ID number here
static const char* subtopic[]    = {"620162297_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
static const char* mqtt_server   = "www.yanacreations.com";         // Broker IP address or Domain name as a String 
static uint16_t mqtt_port        = 1883;

// WIFI CREDENTIALS
const char* ssid       = "Robert"; // Add your Wi-Fi ssid
const char* password   = "paigeee:)"; // Add your Wi-Fi password 



// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL; 


// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters ); 

void digit1(uint8_t number);
void digit2(uint8_t number);
void digit3(uint8_t number);
void digit4(uint8_t number);

void checkPasscode(void);
void showLockState(void);

 

//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif


/* Initialize class objects*/

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST, TFT_MISO);

uint8_t currentDigit = 1; // Keeps track of the current digit being modified by the potentiometer
bool lockState = false; // keeps track of the Open and Close state of the lock

uint8_t num1 = 0, num2 = 0, num3 = 0, num4 = 0; // Keeps track of the value of each digit in the 4-digit passcode.

uint16_t current_value_analog = 0;
int current_value = 0;

/* Declare your functions below */
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters );
unsigned long getTimeStamp(void);
void callback(char* topic, byte* payload, unsigned int length);
bool publish(const char *topic, const char *payload);
void digit1(uint8_t number);
void digit2(uint8_t number);
void digit3(uint8_t number);
void digit4(uint8_t number);
void checkPasscode(void);
void showLockState(void);


void setup() {
  Serial.begin(115200);  // INIT SERIAL  
  tft.begin();
  tft.fillScreen(TFT_BG);
  tft.setTextColor(TFT_DIGIT);
    
  // CONFIGURE THE ARDUINO PINS OF THE 7SEG AS OUTPUT
  //Arduino pin configuration for buttons
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
 
  /* Configure all others here */
  //Display zero in all boxes on the TFT screen on start-up
  digit1(0);
  digit2(0);
  digit3(0);
  digit4(0);

  initialize();           // INIT WIFI, MQTT & NTP 
  vButtonCheckFunction(); // UNCOMMENT IF USING BUTTONS THEN ADD LOGIC FOR INTERFACING WITH BUTTONS IN THE vButtonCheck FUNCTION
}
  


void loop() {
  // put your main code here, to run repeatedly: 

  current_value_analog = analogRead(analogPin);
  current_value = map(current_value_analog, 0, 4095, 0, 9); //Map any of the possible analog values to a valid digit
  
  if (currentDigit == 1)
  {
    num1 = current_value;
    digit1(current_value);
  } 
  else if (currentDigit == 2)
  {
    num2 = current_value;
    digit2(current_value);
  }
  else if (currentDigit == 3)
  {
    num3 = current_value;
    digit3(current_value);
  }
  else if (currentDigit == 4)
  {
    num4 = current_value;
    digit4(current_value);
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);  
}



  
//####################################################################
//#                          UTIL FUNCTIONS                          #       
//####################################################################
void vButtonCheck( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );     
      
    for( ;; ) {
        // Add code here to check if a button(S) is pressed
        // then execute appropriate function if a button is pressed  

        // 1. Implement button1  functionality
        //On button1 press:
          //Increment the currentDigit variable. If the variable becomes greater than 4, reset it to 1. This ultimately selects the digit
          //that will be modified by the analog out of the potentiometer
        if (digitalRead(BTN1) == LOW) {
          delay(50);
          if (currentDigit >=4) {currentDigit = 1;}

          else {currentDigit++;}
          Serial.println("Button 1 pressed! Current digit changed");
        }

        // 2. Implement button2  functionality
        //On button2 press:
          //Invoke the checkPasscode function.
        if (digitalRead(BTN2) == LOW) {
          delay(50);
          Serial.println("Button 2 pressed! Check password");
          checkPasscode();
        }

        // 3. Implement button3  functionality
        //On button3 press:
          //Set lockstate variable to false.
          //Invoke the showLockState function.
        if (digitalRead(BTN3) == LOW) {
          delay(50);
          Serial.println("Button 3 pressed!");
          lockState = false;
          showLockState();
        }
        
        vTaskDelay(200 / portTICK_PERIOD_MS);  
    }
}

void vUpdate( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );    
 
    for( ;; ) {
          // Task code goes here.   
          // PUBLISH to topic every second.  
            
        vTaskDelay(1000 / portTICK_PERIOD_MS);  
    }
}

unsigned long getTimeStamp(void) {
          // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
          time_t now;         
          time(&now); // Retrieve time[Timestamp] from system and save to &now variable
          return now;
}

void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  char *received = new char[length + 1] {0}; 
  
  for (int i = 0; i < length; i++) { 
    received[i] = (char)payload[i];    
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

 
  // CONVERT MESSAGE TO JSON
  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, received);

  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // PROCESS MESSAGE
  const char *type = doc["type"];

  if (strcmp(type, "password") == 0)
  {
    const char *code = doc["code"];
  }

}

bool publish(const char *topic, const char *payload){   
     bool res = false;
     try{
        res = mqtt.publish(topic,payload);
        // Serial.printf("\nres : %d\n",res);
        if(!res){
          res = false;
          throw false;
        }
     }
     catch(...){
      Serial.printf("\nError (%d) >> Unable to publish message\n", res);
     }
  return res;
}

//***** Complete the util functions below ******
  
void digit1(uint8_t number){
  // CREATE BOX AND WRITE NUMBER IN THE BOX FOR THE FIRST DIGIT
  // 1. Set font to FreeSansBold18pt7b 
  // 2. Draw a filled rounded rectangle close to the bottom of the screen. Give it any colour you like 
  // 3. Set cursor to the appropriate coordinates in order to write the number in the middle of the box 
  // 4. Set the text colour of the number. Use any colour you like 
  // 5. Set font size to one 
  // 6. Print number to the screen 

  tft.setFont(&FreeSansBold18pt7b);
  tft.fillRoundRect(MARGIN, tft.height() - BOX_HEIGHT - MARGIN, BOX_WIDTH, BOX_HEIGHT, 10, TFT_BOX);
  tft.setCursor(MARGIN + 15, tft.height() - BOX_HEIGHT + 25);
  tft.setTextSize(1);
  tft.print(number);
}
 
void digit2(uint8_t number){
  // CREATE BOX AND WRITE NUMBER IN THE BOX FOR THE SECOND DIGIT
  // 1. Set font to FreeSansBold18pt7b 
  // 2. Draw a filled rounded rectangle close to the bottom of the screen. Give it any colour you like 
  // 3. Set cursor to the appropriate coordinates in order to write the number in the middle of the box 
  // 4. Set the text colour of the number. Use any colour you like 
  // 5. Set font size to one 
  // 6. Print number to the screen 
  
  tft.setFont(&FreeSansBold18pt7b);
  tft.fillRoundRect(MARGIN + 1*(BOX_WIDTH + SPACE), tft.height() - BOX_HEIGHT - MARGIN, BOX_WIDTH, BOX_HEIGHT, 10, TFT_BOX);
  tft.setCursor(MARGIN + 1*(BOX_WIDTH + SPACE) + 15, tft.height() - BOX_HEIGHT + 25);
  tft.setTextColor(TFT_DIGIT); 
  tft.setTextSize(1);
  tft.print(number);
}

void digit3(uint8_t number){
  // CREATE BOX AND WRITE NUMBER IN THE BOX FOR THE THIRD DIGIT
  // 1. Set font to FreeSansBold18pt7b 
  // 2. Draw a filled rounded rectangle close to the bottom of the screen. Give it any colour you like 
  // 3. Set cursor to the appropriate coordinates in order to write the number in the middle of the box 
  // 4. Set the text colour of the number. Use any colour you like 
  // 5. Set font size to one 
  // 6. Print number to the screen 
  
  tft.setFont(&FreeSansBold18pt7b);
  tft.fillRoundRect(MARGIN + 2*(BOX_WIDTH + SPACE), tft.height() - BOX_HEIGHT - MARGIN, BOX_WIDTH, BOX_HEIGHT, 10, TFT_BOX);
  tft.setCursor(MARGIN + 2*(BOX_WIDTH + SPACE) + 15, tft.height() - BOX_HEIGHT + 25);
  tft.setTextColor(TFT_DIGIT); 
  tft.setTextSize(1);
  tft.print(number);
}

void digit4(uint8_t number){
  // CREATE BOX AND WRITE NUMBER IN THE BOX FOR THE FOURTH DIGIT
  // 1. Set font to FreeSansBold18pt7b 
  // 2. Draw a filled rounded rectangle close to the bottom of the screen. Give it any colour you like 
  // 3. Set cursor to the appropriate coordinates in order to write the number in the middle of the box 
  // 4. Set the text colour of the number. Use any colour you like 
  // 5. Set font size to one 
  // 6. Print number to the screen 
  
  tft.setFont(&FreeSansBold18pt7b);
  tft.fillRoundRect(MARGIN + 3*(BOX_WIDTH + SPACE), tft.height() - BOX_HEIGHT - MARGIN, BOX_WIDTH, BOX_HEIGHT, 10, TFT_BOX);
  tft.setCursor(MARGIN + 3*(BOX_WIDTH + SPACE) + 15, tft.height() - BOX_HEIGHT + 25);
  tft.setTextColor(TFT_DIGIT); 
  tft.setTextSize(1);
  tft.print(number);
}
 
 
void checkPasscode(void){
    // THE APPROPRIATE ROUTE IN THE BACKEND COMPONENT MUST BE CREATED BEFORE THIS FUNCTION CAN WORK
    WiFiClient client;
    HTTPClient http;

    if(WiFi.status()== WL_CONNECTED){ 
      
      // 1. REPLACE LOCALHOST IN THE STRING BELOW WITH THE IP ADDRESS OF THE COMPUTER THAT YOUR BACKEND IS RUNNING ON
      http.begin(client, "www.yanacreations.com"); // Your Domain name with URL path or IP address with path 
 
      
      http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header      
      char message[20];  // Store the 4 digit passcode that will be sent to the backend for validation via HTTP POST
      
      // 2. Insert all four (4) digits of the passcode into a string with 'passcode=1234' format and then save this modified string in the message[20] variable created above 
       
      sprintf(message, "passcode=%d%d%d%d", num1, num2, num3, num4);
                      
      int httpResponseCode = http.POST(message);  // Send HTTP POST request and then wait for a response

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String received = http.getString();
       
        // 3. CONVERT 'received' TO JSON. 
        
        StaticJsonDocument<1000> doc;
        DeserializationError error = deserializeJson(doc, received);

        if (error)
        {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          return;
        }

        // 4. PROCESS MESSAGE. The response from the route that is used to validate the passcode
        // will be either {"status":"complete","data":"complete"}  or {"status":"failed","data":"failed"} schema.
        // (1) if the status is complete, set the lockState variable to true, then invoke the showLockState function
        // (2) otherwise, set the lockState variable to false, then invoke the showLockState function
              
              const char *status = doc["status"];

              if (strcmp(status, "success") == 0)
              {
                lockState = true;
                showLockState();
              }
              else
              {
                lockState = false;
                showLockState();
              }
      }     
        
      // Free resources
      http.end();

    }
             
 }



void showLockState(void){
  
    // Toggles the open and close lock images on the screen based on the lockState variable  
    tft.setFont(&FreeSansBold9pt7b);  
    tft.setTextSize(1);
    

    if(lockState == true){
      tft.drawRGBBitmap(68,10, lockopen, 104, 97); 
      tft.setCursor(50, 200);  
      tft.setTextColor(TFT_BG); 
      tft.printf("Access Denied"); 
      tft.setCursor(50, 200);  
      tft.setTextColor(ILI9341_GREEN); 
      tft.printf("Access Granted");
      tft.setTextColor(TFT_DIGIT); 
      
    }
    else {
      tft.drawRGBBitmap(68,10, lockclose, 104, 103); 
      tft.setCursor(50, 200);  
      tft.setTextColor(TFT_BG); 
      tft.printf("Access Granted"); 
      tft.setCursor(50, 200);  
      tft.setTextColor(ILI9341_RED); 
      tft.printf("Access Denied"); 
      tft.setTextColor(TFT_DIGIT); 
    }
    
}
 
