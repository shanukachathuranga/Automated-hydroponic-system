#include <Arduino.h>
// Include the appropriate WiFi library based on the microcontroller being used (ESP32 or ESP8266)
#include <WiFi.h>
#include <math.h>

// firebase ----------------------------------------------------------------------------------------------
#include <Firebase_ESP_Client.h> // Firebase library for ESP microcontrollers
// Provide the token generation process info.
#include "addons/TokenHelper.h" 
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h" 

// Insert your network credentials (SSID and Password) to connect to Wi-Fi
#define WIFI_SSID "WIFI SSID"
#define WIFI_PASSWORD "WIFI PASSWORD"

// Insert Firebase project API Key (required for authentication)
#define API_KEY "api key"

// Insert Firebase Realtime Database URL (required to access the database)
#define DATABASE_URL "firebase db url" 

// Define Firebase Data object to handle communication with Firebase
FirebaseData fbdo;

// Define Firebase Authentication and Configuration objects
FirebaseAuth auth;
FirebaseConfig config;

// Variables to manage timing and data sending
unsigned long sendDataPrevMillis = 0; // Tracks the last time data was sent
bool signupOK = false; // Flag to check if Firebase sign-up was successful
//---------------------------------------------------------------------------------------------------------


// DS18B20 temp probe setup--------------------------------------------------------------------------
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 8 // pin for temp probe
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with DS18B20 temp probe
DallasTemperature sensors(&oneWire); // Pass the oneWire instance to DallasTemperature
//---------------------------------------------------------------------------------------------------

// DHT11 Humidity/Temp sensor------------------------------------------------------------------------
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 35
#define DHTTYPE    DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);
//---------------------------------------------------------------------------------------------------

NetworkServer server(80);

//sensor data declaration-----------------------------------------------------------------------------------
float tdsVal = 0;                         //total dissolved solids (ppm)
float pHVal = 0;                          //pH value
float liquidTemp = 0;                     //Liquid Temperature Value
float waterLevel = 0;                     // water level
float humidityAndTemperatureArr[] = {0,0};   //Humidity and temperature value DHT11 [temp,humidity]
float firebaseSendData[6];            // firebase send data array [ph,tds,liqTemp,airTemp,humidity,distance]
float firebaseGetData[6];             // firebase get data array  [ph,tds,liqTemp,airTemp,humidity,distance]
int dateTimeArr[4];                   // [hour,minute,month,day]
bool sysControlData[6]; // [fertilizer,growlight,phMotorDown,phMotorUp,solenoid,waterMotor]
bool isFertilizerPumpOn = false;
bool isGrowlightOn = false;
bool isPHMotorDownOn = false;
bool isPHMotorUpOn = false;
bool isSolenoidOn = false;
bool isWaterMotorOn = false;
bool sysActivityData[] = {"false","false","false","false","false","false"}; // [fertilizer,growlight,phMotorDown,phMotorUp,solenoid,waterMotor]
float presetDataArr[9];               //[ph,tdsMin,tdsMax,lcStartHr,lcEndHr,pcStartHr,pcEndHr,pcOnDuration,pcOffDuration]
//--------------------------------------------------------------------------------------------------------

// presest values--------------------------------------------------------------------------------------------

// float presetDataArr[8]; //[ph,tds,lcStartHr,lcEndHr,pcStartHr,pcEndHr,pcOnDuration,pcOffDuration];
//-----------------------------------------------------------------------------------------------------------

//system control data--------------------------------------------------------------------------------------
// bool sysControlData[6]; // [fertilizer,growlight,phMotorDown,phMotorUp,solenoid,waterMotor]
//---------------------------------------------------------------------------------------------------------

//ultrasonic sensor----------------------------------------------------------------------------------------
#include <NewPing.h>
#define TRIG_PIN 37 //  Trig
#define ECHO_PIN 38 //  Echo
#define MAX_DISTANCE 400 // Maximum distance want to measure (in cm)
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
//---------------------------------------------------------------------------------------------------------

//relay data-----------------------------------------------------------------------------------------------
#define PH_UP_PUMP 46
#define PH_DOWN_PUMP 9
#define WATER_PUMP 10
#define GROW_LIGHT 11
#define FERTILIZER_PUMP 12
#define SOLENOID 13
//---------------------------------------------------------------------------------------------------------

//DS3231 rtc module----------------------------------------------------------------------------------------
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;
#define RTC_SDA_PIN 18
#define RTC_SCL_PIN 17

//---------------------------------------------------------------------------------------------------------

//limit values---------------------------------------------------------------------------------------------
const float minWaterLevel = 20;
const float maxWaterLevel = 100;
float tdsMin;
float tdsMax;
float lcStartHr;
float lcEndHr;
float pHmin;
float pHmax;
bool isWaterFilling = false;
float pcStartHr;
float pcEndHr;
float pcOnDuration;
float pcOffDuration;
int pumpOnCountTime[2]; // for countTime() function [hr, minutes]
int pumpOffCountTime[2]; // for countTime() function [hr, minutes]
unsigned long phUpRunningInterval = 5000;  //set
unsigned long phUpEndingMillis = 0;
unsigned long phDownRunningInterval = 5000;
unsigned long phDownEndingMillis = 0;
unsigned long fertilizerPumpRunningInterval = 10000;
unsigned long fertilizerPumpEndingMillis = 0;
//---------------------------------------------------------------------------------------------------------

//relay timers---------------------------------------------------------------------------------------------


void setup() {
  Serial.begin(115200);
  pinMode(PH_UP_PUMP, OUTPUT);      // set the PH_UP_PUMP pin mode
  pinMode(PH_DOWN_PUMP, OUTPUT);    // set the PH_DOWN_PUMP pin mode
  pinMode(WATER_PUMP, OUTPUT);      // set the WATER_PUMP pin mode
  pinMode(GROW_LIGHT, OUTPUT);      // set the GROW_LIGHT pin mode
  pinMode(FERTILIZER_PUMP, OUTPUT); // set the FERTILIZER_PUMP pin mode
  pinMode(SOLENOID, OUTPUT);        // set the SOLENOID pin mode
  pinMode(5, OUTPUT);               // set the LED pin mode
  relayHandlerOFF(PH_UP_PUMP);
  relayHandlerOFF(PH_DOWN_PUMP);
  relayHandlerOFF(WATER_PUMP);
  relayHandlerOFF(GROW_LIGHT);
  relayHandlerOFF(FERTILIZER_PUMP);
  relayHandlerOFF(SOLENOID);

  sensors.begin();     // temp probe begin
  dht.begin();         // humidity sensor begin
  
  connectWifi();
  //------------------------------------------------------------------------------------
  //FIREBASE
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  // Sign up to Firebase anonymously (no email/password required)
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok"); // Print success message if sign-up is successful
    signupOK = true; // Set the flag to indicate successful sign-up
  } else {
  // If sign-up fails, print the error message
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  // Assign the callback function for handling token generation status
  config.token_status_callback = tokenStatusCallback; // See addons/TokenHelper.h
  // Initialize Firebase with the configuration and authentication details
  Firebase.begin(&config, &auth);
  // Enable automatic reconnection to Wi-Fi if the connection is lost
  Firebase.reconnectWiFi(true);
  //---------------------------------------------------------------------------------------
  // Initialize I2C with custom pins for rtc module
  Wire.begin(RTC_SDA_PIN, RTC_SCL_PIN);
  // Check if RTC is connected
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Check if the RTC lost power and needs to be reset
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    // Set the RTC to the current date and time
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  getCurrentTime(dateTimeArr);
  if(dateTimeArr[1] >= 58 && dateTimeArr[0] == 24){
    pumpOnCountTime[0] = 0;
    pumpOnCountTime[1] = 1;
    pumpOffCountTime[0] = 0;
    pumpOffCountTime[1] = 1;
  }else{
    pumpOnCountTime[0] = dateTimeArr[0];
    pumpOnCountTime[1] = dateTimeArr[1];
    pumpOffCountTime[0] = dateTimeArr[0];
    pumpOffCountTime[1] = dateTimeArr[1];
  }
  


}

void loop() {
  Serial.println("\n###################### Start cycle ############################\n");
  // float tdsVal = getTDSvalue();               
  // float pHVal = getPHValue();                 
  // float liquidTemp = getLiquidTempValue();    
  checkWifiStatus();
  getCurrentTime(dateTimeArr);
  unsigned long currentMillis = millis();

  //get system Control data;
  getSystemControlData(sysControlData);
  isFertilizerPumpOn = sysControlData[0];
  isGrowlightOn = sysControlData[1];
  isPHMotorDownOn = sysControlData[2];
  isPHMotorUpOn = sysControlData[3];
  isSolenoidOn = sysControlData[4];
  isWaterMotorOn = sysControlData[5];

  if(!isFertilizerPumpOn){
    relayHandlerOFF(FERTILIZER_PUMP);
  }
  if(!isGrowlightOn){
    relayHandlerOFF(GROW_LIGHT);
  }
  if(!isPHMotorDownOn){
    relayHandlerOFF(PH_DOWN_PUMP);
  }
  if(!isPHMotorUpOn){
    relayHandlerOFF(PH_UP_PUMP);
  }
  if(!isSolenoidOn){
    relayHandlerOFF(SOLENOID);
    isWaterFilling = false;
  }
  if(!isWaterMotorOn){
    relayHandlerOFF(WATER_PUMP);
  }

  Serial.println("----------------syscontrol data--------------------------");
  Serial.println("isFertilizerPumpOn: "+String(isFertilizerPumpOn));
  Serial.println("isGrowlightOn: "+String(isGrowlightOn));
  Serial.println("isPHMotorDownOn: "+String(isPHMotorDownOn));
  Serial.println("isPHMotorUpOn: "+String(isPHMotorUpOn));
  Serial.println("isSolenoidOn: "+String(isSolenoidOn));
  Serial.println("isWaterMotorOn: "+String(isWaterMotorOn));
  Serial.println("-----------------------------------------------------");

  //get preset data
  getPresetValue(presetDataArr);
  tdsMin = presetDataArr[1];
  tdsMax = presetDataArr[2];
  lcStartHr = presetDataArr[3];
  lcEndHr = presetDataArr[4];
  pHmin = presetDataArr[0] - 0.5;
  pHmax = presetDataArr[0] + 0.5;
  pcStartHr = presetDataArr[5];
  pcEndHr = presetDataArr[6];
  pcOnDuration = presetDataArr[7];
  pcOffDuration = presetDataArr[8];
  Serial.println("----------------PresetData--------------------------");
  Serial.println("pHMin: "+String(pHmin));
  Serial.println("pHMax: "+String(pHmax));
  Serial.println("tdsMin: "+String(tdsMin));
  Serial.println("tdsMax: "+String(tdsMax));
  Serial.println("lcStartHr: "+String(lcStartHr));
  Serial.println("lcEndHr: "+String(lcEndHr));
  Serial.println("pcStartHr: "+String(pcStartHr));
  Serial.println("pcEndHr: "+String(pcEndHr));
  Serial.println("pcOnDuration: "+String(pcOnDuration));
  Serial.println("pcOffDuration: "+String(pcOffDuration));
  Serial.println("current Hr: "+String(dateTimeArr[0]));
  Serial.println("current Min: "+String(dateTimeArr[1]));
  Serial.println("-----------------------------------------------------");

  // get the sensor data and assign to variables
  
  waterLevel = getWaterLevel();
  tdsVal = getTDSvalue();
  pHVal = getPHValue();
  liquidTemp = getLiquidTempValue();
  waterLevel = getWaterLevel();
  getHumidityAndTemp(humidityAndTemperatureArr);
  Serial.println("----------------SensorData--------------------------");
  Serial.println("pH: "+String(pHVal));
  Serial.println("waterLevel: "+String(waterLevel));
  Serial.println("tdsVal: "+String(tdsVal));
  Serial.println("liquidTemp: "+String(liquidTemp));
  Serial.println("airTemp: "+String(humidityAndTemperatureArr[0]));
  Serial.println("humidity: "+String(humidityAndTemperatureArr[1]));
  Serial.println("-----------------------------------------------------");


  // get systemControl data
  getSystemControlData(sysControlData);

  //turn off timers
  if(currentMillis >= fertilizerPumpEndingMillis){
    relayHandlerOFF(FERTILIZER_PUMP);
  }
  if(currentMillis >= phUpEndingMillis){
    relayHandlerOFF(PH_UP_PUMP);
  }
  if(currentMillis >= phDownEndingMillis){
    relayHandlerOFF(PH_DOWN_PUMP);
  }


 

  

  // manage water level
  if(waterLevel <= minWaterLevel){
    if(isSolenoidOn){
      relayHandlerON(SOLENOID);
      relayHandlerOFF(WATER_PUMP);
      isWaterFilling = true;
    }
    
  }else if(waterLevel >= maxWaterLevel){
    relayHandlerOFF(SOLENOID);
    relayHandlerON(WATER_PUMP);
    isWaterFilling = false;
  }

  //manage growlight on/off cycle
  Serial.println("Growlight Management-----------------------------");
  int lightTest1 = (dateTimeArr[0] >= lcStartHr) && (dateTimeArr[0] <= lcEndHr);
  int lightTest2 = dateTimeArr[0] >= lcEndHr;
  Serial.println("first if: "+String(lightTest1));
  Serial.println("second if: "+String(lightTest2));
  if((dateTimeArr[0] >= lcStartHr) && (dateTimeArr[0] <= lcEndHr)) {
    relayHandlerON(GROW_LIGHT);
    Serial.println("Growlight On");
  }else if(dateTimeArr[0] >= lcEndHr){
    relayHandlerOFF(GROW_LIGHT);
    Serial.println("Growlight Off");
  }
  Serial.println("--------------------------------------------------");

  Serial.println("## isWaterFilling: " + String(isWaterFilling));
  if(!isWaterFilling){
    Serial.println("## Inside !isWaterFilling Statement");
    // manage water pump cycle
    Serial.println("##Start of Water pump management");
    Serial.println("dateTimeArr[0]: "+String(dateTimeArr[0])+"\npcStartHr: "+String(pcStartHr)+"\npumpOffCountTime[0]: "+String(pumpOffCountTime[0])+"\npumpOffCountTime[1]: "+String(pumpOffCountTime[1]));
    Serial.println(String(dateTimeArr[0] >= pcStartHr)+" && "+ String(dateTimeArr[0] >= pumpOffCountTime[0])+" && "+String(dateTimeArr[1] >= pumpOffCountTime[1]));
    if(dateTimeArr[0] >= pcStartHr && dateTimeArr[0] >= pumpOffCountTime[0] && dateTimeArr[1] >= pumpOffCountTime[1]){
      relayHandlerON(WATER_PUMP);
      countTime(dateTimeArr[0], dateTimeArr[1], pcOnDuration, pumpOnCountTime);
    }else if(dateTimeArr[0] >= pcEndHr && dateTimeArr[0] >= pumpOnCountTime[0] && dateTimeArr[1] >= pumpOnCountTime[1]){
      relayHandlerOFF(WATER_PUMP);
      countTime(dateTimeArr[0], dateTimeArr[1], pcOffDuration, pumpOffCountTime);
    }
    Serial.println("##End of Water pump management");

    //manage fertilizer ppm
    Serial.println("##Start of fertilizer ppm management");
    Serial.println("tdsVal: "+String(tdsVal)+"\ntdsMin: "+String(tdsMin)+"\ncurrentMillis: "+String(currentMillis)+"\nfertilizerPumpEndingMillis: "+String(fertilizerPumpEndingMillis));
    Serial.println(String(tdsVal <= tdsMin)+" && "+ String(currentMillis >= fertilizerPumpEndingMillis));
    if(tdsVal <= tdsMin && currentMillis >= fertilizerPumpEndingMillis){
      relayHandlerON(FERTILIZER_PUMP);
      fertilizerPumpEndingMillis = currentMillis + fertilizerPumpRunningInterval;
    }else if(tdsVal >= tdsMax){
      relayHandlerOFF(FERTILIZER_PUMP);
    }
    Serial.println("##End of fertilizer ppm management");

    //manage pH levels
    Serial.println("##Start of pH levels management");
    Serial.println("pHVal: "+String(pHVal)+"\npHmin: "+String(pHmin)+"\ncurrentMillis: "+String(currentMillis)+"\nphUpEndingMillis: "+String(phUpEndingMillis));
    Serial.println(String(pHVal < pHmin)+" && "+ String(currentMillis >= phUpEndingMillis));
    if(pHVal < pHmin && currentMillis >= phUpEndingMillis){
      relayHandlerON(PH_UP_PUMP);
      phUpEndingMillis = currentMillis + phUpRunningInterval;
    }else if(pHVal > pHmax && currentMillis >= phDownEndingMillis){
      relayHandlerON(PH_DOWN_PUMP);
      phDownEndingMillis = currentMillis + phDownRunningInterval;
    }
    Serial.println("##End of pH levels management");

    Serial.println("##End of the !isWaterFilling Statement");
  }

  



  firebaseSendData[0] = pHVal;
  firebaseSendData[1] = tdsVal;
  firebaseSendData[2] = liquidTemp;
  firebaseSendData[3] = humidityAndTemperatureArr[0];
  firebaseSendData[4] = humidityAndTemperatureArr[1];
  firebaseSendData[5] = waterLevel;

  sendSensorJsonData(firebaseSendData);

  // getJsonData(firebaseGetData);

  // Serial.println("\n--------sensor data-------");
  // Serial.println("PH Value: " + String(firebaseGetData[0]));
  // Serial.println("tds: " + String(firebaseGetData[1]));
  // Serial.println("liquidTemp: " + String(firebaseGetData[2]));
  // Serial.println("airTemp: " + String(firebaseGetData[3]));
  // Serial.println("humidity: " + String(firebaseGetData[4]));
  // Serial.println("Distance: " + String(firebaseGetData[5]));
  // Serial.println("---------------------------\n"); 


  // //send system control data
  // bool boolValue = random(0, 2);
  // int num = random(0, 6);
  // sendSystemControlData(num,boolValue);

  Serial.println("\n###################### End cycle ############################\n");
  // delay(3000);
  
}

// count time
void countTime(int currentHr, int currentMin, int addMins, int arr[]){
  Serial.println("##Inside countTime function");
  if((currentMin+addMins)%60 == 0){
    arr[0] = currentHr;
    arr[1] = currentMin+addMins;
  }else{
    if(currentHr == 23){
      arr[0] = 0 + ((currentMin+addMins)/60);
      arr[1] = (currentMin+addMins)%60;
    }else{
      arr[0] = currentHr + ((currentMin+addMins)/60);
      arr[1] = (currentMin+addMins)%60;
    }
  }
  Serial.println("##End of the countTime function");
}

// relay handler off
void relayHandlerOFF(int pin){
  if(pin == PH_UP_PUMP){
    Serial.println("##Inside relayHandlerOFF function");
    Serial.println("##Turning OFF PH_UP_PUMP");
    digitalWrite(pin, HIGH);
    sendSystemActivityData(3, false);
    Serial.println("##End of the relayHandlerOFF function");
  }else if(pin == PH_DOWN_PUMP){
    Serial.println("##Inside relayHandlerOFF function");
    Serial.println("##Turning OFF PH_DOWN_PUMP");
    digitalWrite(pin, HIGH);
    sendSystemActivityData(2, false);
    Serial.println("##End of the relayHandlerOFF function");
  }else if(pin == WATER_PUMP){
    Serial.println("##Inside relayHandlerOFF function");
    Serial.println("##Turning OFF WATER_PUMP");
    digitalWrite(pin, HIGH);
    sendSystemActivityData(5, false);
    Serial.println("##End of the relayHandlerOFF function");
  }else if(pin == GROW_LIGHT){
    Serial.println("##Inside relayHandlerOFF function");
    Serial.println("##Turning OFF GROW_LIGHT");
    digitalWrite(pin, HIGH);
    sendSystemActivityData(1, false);
    Serial.println("##End of the relayHandlerOFF function");
  }else if(pin == FERTILIZER_PUMP){
    Serial.println("##Inside relayHandlerOFF function");
    Serial.println("##Turning OFF FERTILIZER_PUMP");
    digitalWrite(pin, HIGH);
    sendSystemActivityData(0, false);
    Serial.println("##End of the relayHandlerOFF function");
  }else if(pin == SOLENOID){
    Serial.println("##Inside relayHandlerOFF function");
    Serial.println("##Turning OFF SOLENOID");
    digitalWrite(pin, HIGH);
    sendSystemActivityData(4, false);
    Serial.println("##End of the relayHandlerOFF function");
  }else{
    Serial.println("##Inside relayHandlerOFF function");
    Serial.println("Cant turn OFF pin: "+ String(pin));
    Serial.println("##End of the relayHandlerOFF function");
  }
}

// relay handler on
void relayHandlerON(int pin){
  if(pin == PH_UP_PUMP && isPHMotorUpOn){
    Serial.println("##Inside relayHandlerON function");
    Serial.println("##Turning ON PH_UP_PUMP");
    digitalWrite(pin, LOW);
    sendSystemActivityData(3, true);
    Serial.println("##End of the relayHandlerON function");
  }else if(pin == PH_DOWN_PUMP && isPHMotorDownOn){
    Serial.println("##Inside relayHandlerON function");
    Serial.println("##Turning ON PH_DOWN_PUMP");
    digitalWrite(pin, LOW);
    sendSystemActivityData(2, true);
    Serial.println("##End of the relayHandlerON function");
  }else if(pin == WATER_PUMP && isWaterMotorOn){
    Serial.println("##Inside relayHandlerON function");
    Serial.println("##Turning ON WATER_PUMP");
    digitalWrite(pin, LOW);
    sendSystemActivityData(5, true);
    Serial.println("##End of the relayHandlerON function");
  }else if(pin == GROW_LIGHT && isGrowlightOn){
    Serial.println("##Inside relayHandlerON function");
    Serial.println("##Turning ON GROW_LIGHT");
    digitalWrite(pin, LOW);
    sendSystemActivityData(1, true);
    Serial.println("##End of the relayHandlerON function");
  }else if(pin == FERTILIZER_PUMP && isFertilizerPumpOn){
    Serial.println("##Inside relayHandlerON function");
    Serial.println("##Turning ON FERTILIZER_PUMP");
    digitalWrite(pin, LOW);
    sendSystemActivityData(0, true);
    Serial.println("##End of the relayHandlerON function");
  }else if(pin == SOLENOID && isSolenoidOn){
    Serial.println("##Inside relayHandlerON function");
    Serial.println("##Turning ON SOLENOID");
    digitalWrite(pin, LOW);
    sendSystemActivityData(4, true);
    Serial.println("##End of the relayHandlerON function");
  }else{
    Serial.println("##Inside relayHandlerON function");
    Serial.println("Cant turn ON pin: "+ String(pin));
    Serial.println("##End of the relayHandlerON function");
  }
  
}

//get current time
void getCurrentTime(int arr[]){ // [hour,minute,month,day]
  Serial.println("##Inside getCurrentTime function");

  DateTime now = rtc.now();    // Get the current date and time from the RTC

  // Hour, Minute, Month, and Date
  arr[0] = now.hour();   // Hour
  arr[1] = now.minute(); // Minute
  arr[2] = now.month();  // Month
  arr[3] = now.day();    // Date
  Serial.println("##End of the getCurrentTime function");
}

// get water level
float getWaterLevel(){
  Serial.println("##Inside getWaterLevel function");
  // Measure the distance using the ultrasonic sensor
  float measuredDistance = sonar.ping_cm(); 
  // Define constants for max and min distances
  const float maxDistance = 48.0; // Distance when the tank is empty
  const float minDistance = 34.0; // Distance when the tank is full

  // Calculate the water level percentage
  float waterLevel = ((maxDistance - measuredDistance) / (maxDistance - minDistance)) * 100;

  // Ensure the water level is within the valid range (0% to 100%)
  if (waterLevel < 0) {
    waterLevel = 0;
  } else if (waterLevel > 100) {
    waterLevel = 100;
  }

  return waterLevel;
  

}

//send system control data
void sendSystemActivityData(int index, bool value){
  Serial.println("##Inside sendSystemActivityData function");
  
  String dataArr[] = {"fertilizer","growLight","phMotorDown","phMotorUp","solenoid","waterMotor"};
  String path = "systemActivity/" + dataArr[index];
  Serial.println("##data path: "+ path);


  if (Firebase.ready() && signupOK) {

    // Send the JSON object to the Firebase Realtime Database path "sensorData"
    if (Firebase.RTDB.setBool(&fbdo, path, value)) {
      Serial.println("syscontrol data send PASSED"); // Print success message
    } else {
      Serial.println("syscontrol data send FAILED"); // Print failure message
      Serial.println("REASON: " + fbdo.errorReason()); // Print the reason for failure
    }
  }
  Serial.println("##End of the sendSystemActivityData function");
}

// get systemControl data from firebase
void getSystemControlData(bool arr[]){
  Serial.println("##Inside getSystemControlData function");
  // Read the JSON object from the Firebase Realtime Database path "sensorData"
  bool fertilizer;
  bool growLight;
  bool phMotorDown;
  bool phMotorUp;
  bool solenoid;
  bool waterMotor;

    if (Firebase.RTDB.getJSON(&fbdo, "systemControl")) {
      Serial.println("PASSED"); // Print success message
      Serial.println("PATH: " + fbdo.dataPath()); // Print the database path where data was read
      Serial.println("TYPE: " + fbdo.dataType()); // Print the type of data read

      // Check if the data is a JSON object
      if (fbdo.dataType() == "json") {
        FirebaseJson *json = fbdo.to<FirebaseJson *>(); // Get the JSON object

        // Create a FirebaseJsonData object to store the result
        FirebaseJsonData jsonData;

        // Extract systemControl data 
        if (json->get(jsonData, "fertilizer")) {
          fertilizer = jsonData.boolValue;
        } else {
          Serial.println("Failed to get fertilizer data");
        }
        if (json->get(jsonData, "growLight")) {
          growLight = jsonData.boolValue;
        } else {
          Serial.println("Failed to get growLight data");
        }
        if (json->get(jsonData, "phMotorDown")) {
          phMotorDown = jsonData.boolValue;
        } else {
          Serial.println("Failed to get phMotorDown data");
        }
        if (json->get(jsonData, "phMotorUp")) {
          phMotorUp = jsonData.boolValue;
        } else {
          Serial.println("Failed to get phMotorUp data");
        }
        if (json->get(jsonData, "solenoid")) {
          solenoid = jsonData.boolValue;
        } else {
          Serial.println("Failed to get solenoid data");
        }
        if (json->get(jsonData, "waterMotor")) {
          waterMotor = jsonData.boolValue;
        } else {
          Serial.println("Failed to get waterMotor data");
        }

        arr[0] = fertilizer;
        arr[1] = growLight;
        arr[2] = phMotorDown;
        arr[3] = phMotorUp;
        arr[4] = solenoid;
        arr[5] = waterMotor;

      } else {
        Serial.println("Data is not a JSON object.");
      }
    } else {
      Serial.println("FAILED"); // Print failure message
      Serial.println("REASON: " + fbdo.errorReason()); // Print the reason for failure
    }
    Serial.println("##End of the getSystemControlData function");
}

// preset fetch from firebase
void getPresetValue(float arr[]){ //arr[9]
  Serial.println("##Inside getPresetValue function");
  // Read the JSON object from the Firebase Realtime Database path "sensorData"
  float pH;
  float tdsMin;
  float tdsMax;
  float lcStartHr;
  float lcEndHr;
  float pcStartHr;
  float pcEndHr;
  float pcOnDuration;
  float pcOffDuration;

    if (Firebase.RTDB.getJSON(&fbdo, "presets")) {
      Serial.println("syscontrol data GET PASSED"); // Print success message
      Serial.println("PATH: " + fbdo.dataPath()); // Print the database path where data was read
      Serial.println("TYPE: " + fbdo.dataType()); // Print the type of data read

      // Check if the data is a JSON object
      if (fbdo.dataType() == "json") {
        FirebaseJson *json = fbdo.to<FirebaseJson *>(); // Get the JSON object

        // Create a FirebaseJsonData object to store the result
        FirebaseJsonData jsonData;

        // Extract preset data
        if (json->get(jsonData, "lightEndTime")) {
          lcEndHr = jsonData.floatValue;
        } else {
          Serial.println("Failed to get lightEndTime");
        }

        if (json->get(jsonData, "lightStartTime")) {
          lcStartHr = jsonData.floatValue;
        } else {
          Serial.println("Failed to get lightStartTime");
        }

        if (json->get(jsonData, "pH")) {
          pH = jsonData.floatValue;
        } else {
          Serial.println("Failed to get pH");
        }

        if (json->get(jsonData, "pumpEndTime")) {
          pcEndHr = jsonData.floatValue;
        } else {
          Serial.println("Failed to get pumpEndTime");
        }

        if (json->get(jsonData, "pumpOffDuration")) {
          pcOffDuration = jsonData.floatValue;
        } else {
          Serial.println("Failed to get pumpOffDuration");
        }

        if (json->get(jsonData, "pumpOnDuration")) {
          pcOnDuration = jsonData.floatValue;
        } else {
          Serial.println("Failed to get pumpOnDuration");
        }

        if (json->get(jsonData, "pumpStartTime")) {
          pcStartHr = jsonData.floatValue;
        } else {
          Serial.println("Failed to get pumpOnDuration");
        }

        if (json->get(jsonData, "tdsMax")) {
          tdsMax = jsonData.floatValue;
        } else {
          Serial.println("Failed to get tdsMax");
        }

        if (json->get(jsonData, "tdsMin")) {
          tdsMin = jsonData.floatValue;
        } else {
          Serial.println("Failed to get tdsMin");
        }

        arr[0] = pH;
        arr[1] = tdsMin;
        arr[2] = tdsMax;
        arr[3] = lcStartHr;
        arr[4] = lcEndHr;
        arr[5] = pcStartHr;
        arr[6] = pcEndHr;
        arr[7] = pcOnDuration;
        arr[8] = pcOffDuration;

      } else {
        Serial.println("Data is not a JSON object.");
      }
    } else {
      Serial.println("FAILED preset fetch"); // Print failure message
      Serial.println("REASON: " + fbdo.errorReason()); // Print the reason for failure
    }
    Serial.println("##End of the getPresetValue function");
}

//send data to firebase
void sendSensorJsonData(float arr[]){ //arr [6]
  Serial.println("##Inside sendSensorJsonData function");
  float pH = arr[0];
  float tds = arr[1];
  float liquidTemp = arr[2];
  float airTemp = arr[3];
  float humidity = arr[4];
  float distance = arr[5];

  Serial.println("##Debugging Input Data:");
  Serial.print("pH: "); Serial.println(pH);
  Serial.print("tds: "); Serial.println(tds);
  Serial.print("liquidTemp: "); Serial.println(liquidTemp);
  Serial.print("airTemp: "); Serial.println(airTemp);
  Serial.print("humidity: "); Serial.println(humidity);
  Serial.print("distance: "); Serial.println(distance);

  if (Firebase.ready() && signupOK) {

    // Create a JSON object
    FirebaseJson json;
    json.add("pH", pH);
    json.add("tds", tds);
    json.add("liquidTemp", liquidTemp);
    json.add("airTemp", airTemp);
    json.add("humidity", humidity);
    json.add("distance", distance);

    // Send the JSON object to the Firebase Realtime Database path "sensorData"
    if (Firebase.RTDB.setJSON(&fbdo, "sensorData", &json)) {
      Serial.println("syscontrol data SEND PASSED");
    } else {
      Serial.println("syscontrol data SEND FAILED"); // Print failure message
      Serial.println("REASON: " + fbdo.errorReason()); // Print the reason for failure
    }
  }
  Serial.println("##End of the sendSensorJsonData function");
}

// get json data from firebase
void getSensorJsonData(float arr[]){
  Serial.println("##Inside getSensorJsonData function");
  // Read the JSON object from the Firebase Realtime Database path "sensorData"
  float airTemp;
  float distance;
  float humidity;
  float liquidTemp;
  float pH;
  float tds;

    if (Firebase.RTDB.getJSON(&fbdo, "sensorData")) {
      Serial.println("syscontrol data GET PASSED"); // Print success message
      Serial.println("PATH: " + fbdo.dataPath()); // Print the database path where data was read
      Serial.println("TYPE: " + fbdo.dataType()); // Print the type of data read

      // Check if the data is a JSON object
      if (fbdo.dataType() == "json") {
        FirebaseJson *json = fbdo.to<FirebaseJson *>(); // Get the JSON object

        // Create a FirebaseJsonData object to store the result
        FirebaseJsonData jsonData;

        // Extract sensorData
        if (json->get(jsonData, "airTemp")) {
          airTemp = jsonData.floatValue;
        } else {
          Serial.println("Failed to get airTemp");
        }
        if (json->get(jsonData, "distance")) {
          distance = jsonData.floatValue;
        } else {
          Serial.println("Failed to get distance");
        }
        if (json->get(jsonData, "humidity")) {
          humidity = jsonData.floatValue;
        } else {
          Serial.println("Failed to get humidity");
        }
        if (json->get(jsonData, "liquidTemp")) {
          liquidTemp = jsonData.floatValue;
        } else {
          Serial.println("Failed to get liquidTemp");
        }
        if (json->get(jsonData, "pH")) {
          pH = jsonData.floatValue;
        } else {
          Serial.println("Failed to get pH");
        }
        if (json->get(jsonData, "tds")) {
          tds = jsonData.floatValue;
        } else {
          Serial.println("Failed to get tds");
        }

        arr[0] = pH;
        arr[1] = tds;
        arr[2] = liquidTemp;
        arr[3] = airTemp;
        arr[4] = humidity;
        arr[5] = distance;

      } else {
        Serial.println("Data is not a JSON object.");
      }
    } else {
      Serial.println("FAILED"); // Print failure message
      Serial.println("REASON: " + fbdo.errorReason()); // Print the reason for failure
    }
    Serial.println("##End of the getSensorJsonData function");
}

//connect to wifi
void connectWifi(){
  Serial.println("##Inside connectWifi function");
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);
    delay(500);
    Serial.print(".");
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//check wifi status
void checkWifiStatus(){
  Serial.println("##Inside checkWifiStatus function");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi is connected.");
    rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);
    delay(500);
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
    delay(500);

  } else {
    Serial.println("Wi-Fi is not connected.");
    connectWifi();
  }
  Serial.println("##End of the connectWifi function");
}

// Get total disolved solids in ppm
float getTDSvalue(){
  Serial.println("##Inside getTDSvalue function");
  // Define constants
  const int tdsPin = 4; // Analog pin connected to the TDS sensor
  const float vRef = 3.3; // Reference voltage (3.3V for ESP32, 5V for Arduino)
  const float resolution = 4095; // Conversion factor (ADC resolution / reference voltage)
  const float calibration_factor = 3.1;

  // Read the analog value from the TDS sensor
  int adcValue = analogRead(tdsPin);

  // Convert the ADC value to voltage
  float voltage = (adcValue / resolution) * vRef;

  // Calculate TDS value (approximation)
  float tdsValue = voltage * 100 * calibration_factor; // This formula may vary based on the sensor
  Serial.println("##End of the getTDSvalue function");
  return tdsValue;  //ppm

}

// get pH value
float getPHValue(){
  Serial.println("##Inside getPHValue function");

  const int pH_sensor_pin = 6;
  const float calibrationValue = 0.0067;

  int sensorValue = analogRead(pH_sensor_pin);  // Read analog value
  float voltage = sensorValue * (3.3 / 4095.0); // Convert to voltage (assuming 3.3V reference)
  float pHValue = 7.0 - ((sensorValue - 2045) * calibrationValue); // Example conversion formula (calibrate for accuracy)
  
  if(pHValue < 0 || pHValue > 14){
    pHValue = 0.0;
  }
  Serial.println("##End of the getPHValue function");
  return pHValue;

}

// get liquid temperature
float getLiquidTempValue(){
  Serial.println("##Inside getLiquidTempValue function");
  sensors.requestTemperatures(); // Request temperature reading
  float temperature = sensors.getTempCByIndex(0); // Read temperature in Celsius
  Serial.println("##End of the getLiquidTempValue function");
  // Serial.print("Temperature: ");
  return temperature;
  // Serial.println(" °C");

}

// get air humidity and temperature
void getHumidityAndTemp(float arr[]){ //[temp, humidity]
  // Delay between measurements.
  // delay(delayMS);
  // Get temperature event and print its value.
  Serial.println("##Inside getHumidityAndTemp function");
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temp = event.temperature;
  if(temp >= 0){
    arr[0] = temp;
  }else{
    arr[0] = 0.0;
  }
  

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  float hmdt = event.relative_humidity;
  if(hmdt >= 0){
    arr[1] = hmdt;
  }else{
    arr[1] = 0.0;
  }
  Serial.println("##End of the getHumidityAndTemp function");

}











