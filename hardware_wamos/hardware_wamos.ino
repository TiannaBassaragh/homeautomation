
#include <SoftwareSerial.h>
// IMPORT ALL REQUIRED LIBRARIES
#include <Arduino.h> //arduino functions like digitalWrite and pinMode

#include <stdlib.h> //standard library functions in c

#include <stdio.h> //standard i/o functions like printf

#include <ArduinoJson.h> //for json messages

#include <NewPing.h> //for ultrasonic sensor

#include <math.h>
   
//**********ENTER IP ADDRESS OF SERVER******************//

#define HOST_IP     "www.yanacreations.com"       // REPLACE WITH IP ADDRESS OF SERVER ( IP ADDRESS OF COMPUTER THE BACKEND IS RUNNING ON) 
#define HOST_PORT   "8080"            // REPLACE WITH SERVER PORT (BACKEND FLASK API PORT)
#define route       "api/update"      // LEAVE UNCHANGED 
#define idNumber    "620162297"       // REPLACE WITH YOUR ID NUMBER 

// WIFI CREDENTIALS
#define SSID        "MonaConnect"     // "REPLACE WITH YOUR WIFI's SSID"   
#define password    ""  // "REPLACE WITH YOUR WiFi's PASSWORD" 

#define stay        100
 
//**********PIN DEFINITIONS******************//

 
#define espRX         10
#define espTX         11
#define espTimeout_ms 300

#define trigPin 13 //pin to output to trigger pin on sensor
#define echoPin 12 //pin to send info to echo pin on sensor
#define maxDistance 240 //maximum distance for ultrasonic sensor in cm

#define maxWaterheight 77.763 //max waterheight in inches (where the tank is at 100% capacity)
// #define maxReserveVol 1000 //US Gallons when the tank is at 100% capacity
#define sensorHeight 94.5 //inches from base of tank
#define tankDiameter 61.5 //inches

/*Variables*/
float  radar        =0; //distance in inches
double waterheight  =0; //inches
double reserve      =0; //volume filled in reserve in US Gallons
double percentage   =0; //percent of tank filled
 
/* Declare your functions below */
void espSend(char command[]);
void espUpdate(char mssg[]);
void espInit();
double calcWaterheight(double); //metres
double calcReserveVol(double); //unit US gallons
double calcPercentage(); //percentage using waterheight and max water level

SoftwareSerial esp(espRX, espTX); 
 
NewPing sonar(trigPin, echoPin, maxDistance); //setup for sensor

void setup(){

  Serial.begin(115200); 
  // Configure GPIO pins here
  //Since Im using NewPing I don't have any to configure
 
  espInit();  
 
}

void loop(){
  
  radar = sonar.ping_in(); //Continuously measure the distance between the sensor and the water in the tank. This must be considered as the ‘radar’ measurement

  waterheight = calcWaterheight(radar); //Using the radar measurement, calculate the water level/height in the tank. This must be considered as the ‘waterheight’ measurement.
  reserve = calcReserveVol(waterheight); //Calculate remaining water reserves, in US Gallons, from the radar measurement. Consider this as the ‘reserve’ measurement. That is, compute the volume of water currently in the tank with units of US Gallons.
  percentage = calcPercentage(waterheight); // Compute a percentage using the waterheight and the Maximum water level of the tank
  
  // Serial.print(radar);
  // Serial.println(" inches");
  // Serial.print(waterheight);
  // Serial.println(" inches");
  // Serial.print(reserve);
  // Serial.println(" gallons");
  // Serial.print(percentage);
  // Serial.println("%");
  // Serial.println("");

  delay(50);

  // send updates with schema ‘{"id": "student_id", "type": "ultrasonic", "radar": 0, "waterheight": 0, "reserve": 0, "percentage": 0}’
  JsonDocument doc;
  char message[290]     = {0};
  doc["id"]             = idNumber;
  doc["type"]           = "ultrasonic";
  doc["radar"]          = radar;
  doc["waterheight"]    = waterheight;
  doc["reserve"]        = reserve;
  doc["percentage"]     = percentage;

  serializeJson(doc, message);  // Seralize / Covert JSon object to JSon string and store in char* array

  espUpdate(message);


  delay(1000);  
}

 
void espSend(char command[] ){   
    esp.print(command); // send the read character to the esp    
    while(esp.available()){ Serial.println(esp.readString());}    
}


void espUpdate(char mssg[]){ 
    char espCommandString[50] = {0};
    char post[290]            = {0};

    snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",HOST_IP,HOST_PORT); 
    espSend(espCommandString);    //starts the connection to the server
    delay(stay);

    // GET REQUEST 
    // snprintf(post,sizeof(post),"GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);

    // POST REQUEST
    snprintf(post,sizeof(post),"POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);
  
    snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSEND=%d\r\n", strlen(post));
    espSend(espCommandString);    //sends post length
    delay(stay);
    Serial.println(post);
    espSend(post);                //sends POST request with the parameters 
    delay(stay);
    espSend("AT+CIPCLOSE\r\n");   //closes server connection
   }

void espInit(){
    char connection[100] = {0};
    esp.begin(115200); 
    Serial.println("Initializing"); //the word initializing was spelt wrong
    esp.println("AT"); 
    delay(1000);
    esp.println("AT+CWMODE=1");
    delay(1000);
    while(esp.available()){ Serial.println(esp.readString());} 

    snprintf(connection, sizeof(connection),"AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,password);
    esp.print(connection);

    delay(3000);  //gives ESP some time to get IP

    if(esp.available()){   Serial.print(esp.readString());}
    
    Serial.println("\nFinish Initializing");    
   
}

//***** Design and implement all util functions below ******
double calcWaterheight(double radar){
  // Ensure radar is within valid bounds
  if (radar < 0) {
    radar = 0;
  } else if (radar > sensorHeight + maxWaterheight) {
    radar = sensorHeight + maxWaterheight;
  }

  // Calculate the water height
  return sensorHeight + maxWaterheight - radar; //to account for space between max waterheight and sensor
  //ignoring the space we would have: 
  //return maxWaterheight - radar;
}

double calcReserveVol(double waterheight){
  //formula: (pi*(r^2)*h) / (231 to get US Gallons from Cubic inches)
  //Calculate remaining water reserves, in US Gallons, from the radar measurement. That is, compute the volume of water currently in the tank with units of US Gallons.
  float tankRadius = tankDiameter/2;
  return (3.1415*tankRadius*tankRadius*waterheight) / 231.0;
}
double calcPercentage(double waterheight){
  //Compute a percentage using the waterheight and the Maximum water level of the tank.
  return (waterheight/maxWaterheight)*100;
}
