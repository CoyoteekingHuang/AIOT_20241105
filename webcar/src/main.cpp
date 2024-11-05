#include <Arduino.h>
#include <WiFi.h>
#include <iostream>
#include <sstream>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define up 1
#define down 2
#define left 3
#define right 4
#define Stop 0

#define right_motor 0
#define left_motor 1

#define forward 1
#define backward -1

const int pwmfreq = 1000 ; //1khz
const int pwmresolution = 8 ;
const int pwmspeedchannel = 4 ;

const char* ssid = "kingiotcar"; //esp32 station name
const char* password = "666666" ;

AsyncWebServer server(80);
AsyncWebSocket wscarinput("/carinput"); // udp transport  car command
struct motor_pins {
  int pinen;
  int pinin1;
  int pinin2;
};
std::vector<motor_pins> motor_PINs = {
  {22,16,17}, //right motor (ena in1 in2)
  {23,18,19}, // left motor (enb in3 in4)

};
void setuppinmodes(){
  ledcSetup(pwmspeedchannel, pwmfreq, pwmresolution);
  for (int i=0; i< motor_PINs.size(); i++){
    pinMode(motor_PINs[i].pinen, OUTPUT);
    pinMode(motor_PINs[i].pinin1, OUTPUT);
    pinMode(motor_PINs[i].pinin2, OUTPUT);
    ledcAttachPin(motor_PINs[i].pinen, pwmspeedchannel);
  }

}
void handleRoot(AsyncWebServerRequest *request){
  request->send(200, "text/html", htmlhomepage);

}
void handleNotFound(AsyncWebServerRequest *request){
  request->send(404, "text/plain", "File Not Found");

}


void setup() {
  // put your setup code here, to run once:
  setuppinmodes();
  Serial.begin(115200);
  WiFi.softAP(ssid, password) ; //set esp32 station
  IPAddress ip = WiFi.softAPIP(); //get ip
  Serial.print("AP IP Address:  ");
  Serial.println(ip);
  
  server.on("/",HTTP_GET, handleRoot) ; //web server
  server.onNotFound(handleNotFound); // function for notfound HTTP web

  wscarinput.onEvent(oncarinputwebsocketevent); // event of car command received 
  server.addHandler(&wscarinput); //串接一起
  server.begin();
  Serial.println("HTTP Server Started");


}


void loop() {
  // put your main code here, to run repeatedly:


}

