#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>

// WiFi Definitions
const char* _ssid = "Lock Wifi 2";
const char* password_serial_number = "123456789"; // has to be longer than 7 chars
HTTPClient httpSubscriber;
int i = 0;
int val = 0;
int last_val=HIGH;
uint8_t TurnCounter = 0;
int doorDirection = 1;
struct tys_Status{
  byte battery_status;
  byte wifi_status;
  bool lock_status;
  bool door_status;
  bool Connection_status;  
};

#define reedSwitch_Pin 13
#define MotorEncoder1 4
#define MotorEncoder2 5
#define MotorPin1 12
#define MotorPin2 14


tys_Status DoorData;
ESP8266WebServer server(80);

/*----------------------------------------------------------------------*/
/*                              Setup                                   */
/*----------------------------------------------------------------------*/
void setup(){

   Serial.begin(115200);  
   delay(100);
   
   ESP.wdtDisable();
   ESP.wdtEnable(WDTO_8S);
   delay(100);
   pinMode(LED_BUILTIN, OUTPUT);
   
   pinMode(MotorPin1,OUTPUT);
   pinMode(MotorPin2,OUTPUT);

   pinMode(MotorEncoder1,INPUT);
   pinMode(MotorEncoder2,INPUT);
   
   pinMode(reedSwitch_Pin, INPUT);
   delay(100);
   WiFi.mode(WIFI_AP_STA);
   WiFi.softAP(_ssid, password_serial_number, 1, 0);
   delay(100);
   server.on("/", handleRoot);
   server.on("/connect", handleConnect);
   server.on("/disconnect", handledisConnect);
   
   server.on("/led/on", handleLEDon);
   server.on("/led/off", handleLEDoff);
   
   server.on("/counter", handleCounter);
   server.on("/check", handleCheck);
   server.on("/networks", handleSendAccessedNetworks);
   
   server.on("/lock", handleLock);
   server.on("/unlock", handleUnlock);
   server.on("/stop", handleStop);
   server.on("/toggle",handleToggle);
   
   server.on("/getauthentication", handlegetAuthentication);
   server.on("/signalstrenght", handleSignalStrenght);
   server.on("/doorstatus", handleDoorStatus);
   server.on("/lockstatus", handleLockStatus);
   server.on("/batterystatus", handleBatteryStatus);
   server.on("/serialnumber", handleGetSerialNumber);
   server.on("/errors", handleError);
   server.on("/setSubscriber",setSubscriber);
   digitalWrite(LED_BUILTIN, LOW);
    
   server.onNotFound(handleNotFound); 
   server.begin(); //Start the server
   delay(100);
   //Serial.println("Server listening");
   //setSubscriber();
}

/*----------------------------------------------------------------------*/
/*                              Loop                                    */
/*----------------------------------------------------------------------*/
void loop() {
    server.handleClient();
    ESP.wdtFeed();
    
    val = digitalRead(MotorEncoder2);
    if(val != last_val){
      if(val == LOW){
       TurnCounter++;
      }
       Serial.println(TurnCounter);
    }
    last_val = val;
    if(doorDirection == 1){
      if(TurnCounter >= 4){
        digitalWrite(MotorPin1, LOW);
        digitalWrite(MotorPin2, LOW);
        TurnCounter =0;
        Serial.println("motorstop");
      }  
    }
    else{
      if(TurnCounter >= 4){
        digitalWrite(MotorPin1, LOW);
        digitalWrite(MotorPin2, LOW);
        TurnCounter =0;
        Serial.println("motorstop");
      }
    }
//  int httpCode;
//  httpCode = httpSubscriber.GET();
//  String thisString = httpSubscriber.getString();
//  delay(1000);
//    if (httpCode == 200 && thisString != "{\"messages\":[]}")
//    {
//      const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + 310;
//      DynamicJsonBuffer jsonBuffer(bufferSize);
//      
//      char jsonValue[sizeof(thisString)];
//      thisString.toCharArray(jsonValue, sizeof(jsonValue));
//      
//      JsonObject& root = jsonBuffer.parseObject(thisString);
//            
//      JsonObject& messages0 = root["messages"][0];
//
//      String messages0_data = messages0["data"];
//      
//          if (messages0_data == "toggle")
//        {
//          if(doorDirection == 0){
//          doorDirection = 1;
//    
//          digitalWrite(MotorPin1, LOW);
//          digitalWrite(MotorPin2, HIGH);
//          handlepublish();
//          }
//          else{
//          doorDirection = 0;
//          
//          digitalWrite(MotorPin1, HIGH);
//          digitalWrite(MotorPin2, LOW);
//          handlepublish();
//          }
//        }
//    }
}

/*----------------------------------------------------------------------*/
/*                              Root                                    */
/*----------------------------------------------------------------------*/
void handleRoot() {
      server.send(200, "text/plain", "Validation Successful");
}

/*----------------------------------------------------------------------*/
/*                              handleConnect                           */
/*----------------------------------------------------------------------*/
void handleConnect() {
  if (server.hasArg("needed_ssid") == true){


    WiFi.begin((server.arg("needed_ssid")).c_str(), (server.arg("needed_password")).c_str());

    Serial.println((server.arg("needed_ssid")).c_str());
    Serial.println(server.arg("needed_password"));
    Serial.println("Connecting");

    int delay_value = 500;
    
    while (delay_value < 5000 and WiFi.status() != WL_CONNECTED)
    {
      delay_value += 500;
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED){
      server.send(200, "text/plain", "Connected Successfully To " + server.arg("needed_ssid") + " Ip Address is : " + WiFi.localIP());
      handleSaveStatusInServer();
    } else {
      server.send(200, "text/plain", "Can not Connect, Try again.");
    }
  }
  
  server.send(400, "text/plain", "Bad Request.");

  Serial.println(WiFi.localIP());
}
/*----------------------------------------------------------------------*/
/*                              handleConnect                           */
/*----------------------------------------------------------------------*/
void handledisConnect() {
  WiFi.disconnect(); 
    server.send(200, "text/plain", "disConnected Successfully To "+ WiFi.localIP()+ String(WiFi.status()));
}
/*----------------------------------------------------------------------*/
/*                                LED on                                */
/*----------------------------------------------------------------------*/
void handleLEDon(){
  digitalWrite(LED_BUILTIN, HIGH);
}

/*----------------------------------------------------------------------*/
/*                                LED off                               */
/*----------------------------------------------------------------------*/
void handleLEDoff(){
  digitalWrite(LED_BUILTIN, LOW);
}

/*----------------------------------------------------------------------*/
/*                              handle Counter                          */
/*----------------------------------------------------------------------*/
void handleCounter(){
  i = i + 1;
  server.send(200, "text/plain", String(i));
}

/*----------------------------------------------------------------------*/
/*                              handle Check                            */
/*----------------------------------------------------------------------*/
void handleCheck(){
    const size_t bufferSize = JSON_OBJECT_SIZE(6);
    DynamicJsonBuffer jsonBuffer(bufferSize);

    JsonObject& root = jsonBuffer.createObject();
    
    get_Status(&DoorData);
    
    root["battery_status"] = 4;
    root["wifi_status"]       = DoorData.wifi_status;
    root["lock_status"]       = DoorData.lock_status;
    root["door_status"]       = DoorData.door_status;
    root["connection_status"] = DoorData.Connection_status;
    root["message"] = ":D";

    String output;
    root.prettyPrintTo(output);

    server.send(200, "text/plain", output);
}

/*----------------------------------------------------------------------*/
/*                        handlesendAccessedNetwork                     */
/*----------------------------------------------------------------------*/
void handleSendAccessedNetworks(){
  Serial.println("Searching");
  WiFi.scanNetworks(); //will return the number of networks found
  int n = WiFi.scanNetworks();

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& availableWifi = jsonBuffer.createObject();
  availableWifi["data"] = String(n) + " wifi networks detected.";
  JsonArray& networksName = availableWifi.createNestedArray("networksSSID");
  
  if (n == 0)
    server.send(200, "text/plain", "No Networks Found");
  else
  {
    for (int i = 0; i < n; ++i)
    {  
      networksName.add(WiFi.SSID(i));
      Serial.println(WiFi.SSID(i));
      delay(10);
    }

    String output;
    availableWifi.prettyPrintTo(output);
    Serial.println(output);

    server.send(200, "text/plain", output);
  }
}

/*----------------------------------------------------------------------*/
/*                              handle Lock                             */
/*----------------------------------------------------------------------*/
void handleLock(){
  if(TurnCounter < 1){
    digitalWrite(MotorPin1, HIGH);
    digitalWrite(MotorPin2, LOW);  
  }
  else{
    digitalWrite(MotorPin1, LOW);
    digitalWrite(MotorPin2, LOW);  
  }
  
  val = digitalRead(5);
  if(val == 1){
    TurnCounter++;
  }
  server.send(200, "text/plain", "Lock Successfully...");
}

/*----------------------------------------------------------------------*/
/*                              handle UnLock                           */
/*----------------------------------------------------------------------*/
void handleUnlock(){
  digitalWrite(MotorPin1, LOW);
  digitalWrite(MotorPin2, HIGH);
  server.send(200, "text/plain", "unlock Successfully...");
}

/*----------------------------------------------------------------------*/
/*                              handle Stop                             */
/*----------------------------------------------------------------------*/
void handleStop(){
  digitalWrite(MotorPin1, LOW);
  digitalWrite(MotorPin2, LOW);
  server.send(200, "text/plain", "Stop Successfully...");
}

/*----------------------------------------------------------------------*/
/*                              handle toggle                             */
/*----------------------------------------------------------------------*/
void handleToggle(){
  if(doorDirection == 0){
    doorDirection = 1;
    
    digitalWrite(MotorPin1, LOW);
    digitalWrite(MotorPin2, HIGH);
    } 
   else {
    doorDirection = 0;
    digitalWrite(MotorPin1, HIGH);
    digitalWrite(MotorPin2, LOW);
  }
  server.send(200, "text/plain", "200: Ok");
  handleSaveStatusInServer();
}
/*----------------------------------------------------------------------*/
/*                        handlegetAuthentication                       */
/*----------------------------------------------------------------------*/
void handlegetAuthentication(){
  server.send(200,"text/plain" ,server.arg("ssid") + server.arg("pass"));
}

/*----------------------------------------------------------------------*/
/*                              signalstrenght                          */
/*----------------------------------------------------------------------*/
void handleSignalStrenght() {
  long rssi = WiFi.RSSI();
  server.send(200, "text/plain", "RSSI:" + rssi);
  Serial.print("RSSI:");
  Serial.println(rssi);
}

/*----------------------------------------------------------------------*/
/*                                Door Status                                */
/*----------------------------------------------------------------------*/
void handleDoorStatus(){
  digitalWrite(LED_BUILTIN, HIGH);
}

/*----------------------------------------------------------------------*/
/*                                Lock Status                                */
/*----------------------------------------------------------------------*/
void handleLockStatus(){
  digitalWrite(LED_BUILTIN, HIGH);
}

/*----------------------------------------------------------------------*/
/*                                Battery Status                                */
/*----------------------------------------------------------------------*/
void handleBatteryStatus(){
  digitalWrite(LED_BUILTIN, HIGH);
}

/*----------------------------------------------------------------------*/
/*                                get Serial Number                     */
/*----------------------------------------------------------------------*/
void handleGetSerialNumber(){
  digitalWrite(LED_BUILTIN, HIGH);
}

/*----------------------------------------------------------------------*/
/*                                Error                                 */
/*----------------------------------------------------------------------*/
void handleError(){
  digitalWrite(LED_BUILTIN, HIGH);
}

/*----------------------------------------------------------------------*/
/*                             handle not found                         */
/*----------------------------------------------------------------------*/
void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

/*----------------------------------------------------------------------*/
/*                             handle not found                         */
/*----------------------------------------------------------------------*/
void setSubscriber(){
  if(WiFi.status()== WL_CONNECTED)
  {
    //Check WiFi connection status
    httpSubscriber.begin("http://api.backendless.com/8178E63C-1936-5C74-FFC6-B90735872B00/BA7F64EC-A219-CA3F-FFB7-B81B1BBC5300/messaging/toggle/subscribe");
    httpSubscriber.addHeader("Content-Type", "application/json");
    String postMessage = String("{\"subtopic\":\"123456789\"}");
    int httpCode = httpSubscriber.POST(postMessage);
    String payload = httpSubscriber.getString();
    httpSubscriber.end();

    httpSubscriber.begin("http://api.backendless.com/8178E63C-1936-5C74-FFC6-B90735872B00/BA7F64EC-A219-CA3F-FFB7-B81B1BBC5300/messaging/toggle/" + payload.substring(19, 55));
    httpSubscriber.addHeader("Content-Type", "application/json");
    httpCode = httpSubscriber.GET();
    String thisString = httpSubscriber.getString();
    server.send(200, "text/plain", "200: Ok" + httpCode);
    if (httpCode == 200 && thisString != "{\"messages\":[]}")
    {
      const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + 310;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      
      char jsonValue[sizeof(thisString)];
      thisString.toCharArray(jsonValue, sizeof(jsonValue));
      
      JsonObject& root = jsonBuffer.parseObject(thisString);
            
      JsonObject& messages0 = root["messages"][0];

      String g; 
      messages0.prettyPrintTo(g);
      Serial.println(g);
      String messages0_data = messages0["data"];
      
          if (messages0_data == "toggle")
        {
         if(doorDirection == 0){
          doorDirection = 1;
    
          digitalWrite(MotorPin1, LOW);
          digitalWrite(MotorPin2, HIGH);
          }
          else{
          doorDirection = 0;
          
          digitalWrite(MotorPin1, HIGH);
          digitalWrite(MotorPin2, LOW);
          }
//          handleSaveStatusInServer();
//          handlepublish();
        }
    }
  }
  else
  {
    server.send(400, "text/plain", "400: error");
  }
  httpSubscriber.end();
}

void handlepublish(){
  HTTPClient publishHttp;
  publishHttp.begin("http://api.backendless.com/8178E63C-1936-5C74-FFC6-B90735872B00/BA7F64EC-A219-CA3F-FFB7-B81B1BBC5300/messaging/reponse_toggle");
  publishHttp.addHeader("Content-Type", "application/json");
  String postMessage = String("{\"message\":\"ok\", \"subtopic\":\"123456789\"}");
  int httpCode = publishHttp.POST(postMessage);
  String payload = publishHttp.getString();
  publishHttp.end();
}

void handleSaveStatusInServer(){
  get_Status(&DoorData);
  HTTPClient http;
  http.begin("http://api.backendless.com/8178E63C-1936-5C74-FFC6-B90735872B00/BA7F64EC-A219-CA3F-FFB7-B81B1BBC5300/data/Lock/347F7393-36C8-543C-FFED-C363960EA700");
  http.addHeader("Content-Type", "application/json");
  String postMessage = String("{\"wifi_status\" :" + String(DoorData.wifi_status) + ",\"battery_status\": 3 ,\"lock_status\" : " + String(DoorData.lock_status) + ",\"door_status\" : " + String(DoorData.door_status) + ",\"connection_status\" : "+ String(DoorData.Connection_status) +"}");
  int httpCode = http.PUT(postMessage);
  String payload = http.getString();
  http.end();
}
/*----------------------------------------------------------------------------------------------------------------------------------------*/
/*                                                            alternative function                                                        */
/*----------------------------------------------------------------------------------------------------------------------------------------*/
byte get_BatteryStatus (){
  int val;
  val= analogRead(A0);
  return(val);
}

byte get_WifiStatus(){
  byte WifiStatus;
  long rssi;
  if (WiFi.status() == WL_CONNECTED){
    rssi= WiFi.RSSI();
    if (rssi > -55) { 
      WifiStatus = 5;
    } 
    else if (rssi < -55 & rssi > -65) {
      WifiStatus = 4;
    } 
    else if (rssi < -65 & rssi > -70) {
      WifiStatus = 3;
    } 
    else if (rssi < -70 & rssi > -78) {
      WifiStatus = 2;
    } 
    else if (rssi < -78 & rssi > -82) {
      WifiStatus = 1;
    } 
    else {
      WifiStatus = 0;
    }
  }
  else{
    WifiStatus = 0;
  } 
  
  return(WifiStatus);
}

bool get_lockStatus(){
  bool lockStatus;
  if(doorDirection == 1){
    lockStatus = true;
  }
  else{
    lockStatus = false;
  }
  return(lockStatus);
}

bool get_doorStatus(){
  bool doorStatus;
  doorStatus = digitalRead(reedSwitch_Pin);
  if(doorStatus == 0){
    doorStatus = true;
  }
  else{
    doorStatus  = false;
  }
  return(doorStatus);
}

bool ConnectionStatus(){
  bool ConnectionStatus;
  if (WiFi.status() == WL_CONNECTED){
    ConnectionStatus = true;
  }
  else{
    ConnectionStatus = false;
  }
  return(ConnectionStatus);
}

void get_Status(tys_Status *pvS){
  pvS->battery_status = get_BatteryStatus();
  pvS->wifi_status    = get_WifiStatus();
  pvS->lock_status    = get_lockStatus();
  pvS->Connection_status = ConnectionStatus();
  pvS->door_status = get_doorStatus();
}

