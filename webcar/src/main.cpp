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

const char* htmlhomepage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE HTML>
<html>
   <head>
    <style>
      .arrows {
      font-size:40px;
      color:red;
    }
    td.button {
      background-color:black;
      border-radius:25%;
      box-shadow: 5px 5px #888888;
    }
    td.button:active {
      transform: translate(5px,5px);
      box-shadow: none; 
    }

    .noselect {
      -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
         -khtml-user-select: none; /* Konqueror HTML */
           -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
                user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
    }

    .slidecontainer {
      width: 100%;
    }

    .slider {
      -webkit-appearance: none;
      width: 100%;
      height: 20px;
      border-radius: 5px;
      background: #d3d3d3;
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    .slider:hover {
      opacity: 1;
    }
  
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    .slider::-moz-range-thumb {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }
    </style>
   </head>
   <body>
      <h1 style="color: teal; text-align: center;">ESP32 Web Control Car</h1>
<h2 style="text-align:center">WiFi Controler</h2>
<table id="mainTable" border="1" style="width:400px;margin:auto;table-layout:fixed;text-align: enter;" CELLSPACING=10>
  <tr>
    <td></td>
    <td class="button" ontouchstart='sendButtonInput("MoveCar", "1")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8679;</span></td>
    <td></td>
  </tr>
  <tr>
    <td class="button" ontouchstart='sendButtonInput("MoveCar", "3")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8678;</span></td>
    <td class="button"></td>
    <td class="button" ontouchstart='sendButtonInput("MoveCar", "4")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8680;</span></td>
  </tr>
  <tr>
    <td></td>
    <td class="button" ontouchstart='sendButtonInput("MoveCar", "2")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8681;</span></td>
    <td></td>
  </tr>
   <tr>
  <tr></tr>
  <tr></tr>
   </tr>
  <tr>
    <td style="text-align:center;font-size:24px"><b>Speed:</b></td>
    <td colspan=2>
      <div class="slidecontainer">
        <input id="Speed" type="range" min="0" max="255" value="125" class="slider" oninput='sendButtonInput("Speed",value)'>
      </div>
    </td>
  </tr>
</table>

      <script>
          var webSocketCarInputUrl="ws:\/\/"+window.location.hostname+"/carinput";
var  webSocketCarInput;

function sendButtomInput(key,value){
  var data = key+","+value; //Speed,120; MoveCar,1
  webSocketCarInput.send(data);
}

function initCarinputWebSocket()
{
  webSocketCarInput = new WebSocket(webSocketCarInputUrl);
  webSocketCarInput.onopen = function(event){
    var speedButton = document.getElementById("Speed");
    sendButtonInput("Speed",speedButton.value);
  };
  webSocketCarInput.onclose = function(event){setTimeout(initInputWebSocket, 2000)};
  webSocketCarInput.onmessage = function(event){};
}
window.onload=initCarinputWebSocket;
document.getElementById("mainTable").addEventListener("touchend", function(event){
  event.prentDefault();
});
      </script.
   </body>
</html>


)HTMLHOMEPAGE";

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
  request->send(200, "text/html", htmlhomepage); // USER SEND COMMAND BY HOMEPAGE

}
void handleNotFound(AsyncWebServerRequest *request){
  request->send(404, "text/plain", "File Not Found");

}

void rotateMotor(int motorNumber, int motorDirection){
  if(motorDirection == forward){
    digitalWrite(motor_PINs[motorNumber].pinin1, HIGH);
    digitalWrite(motor_PINs[motorNumber].pinin2, LOW);
  } else if (motorDirection == backward){
    digitalWrite(motor_PINs[motorNumber].pinin1, LOW);
    digitalWrite(motor_PINs[motorNumber].pinin2, HIGH);
  } else{
    digitalWrite(motor_PINs[motorNumber].pinin1, LOW);
    digitalWrite(motor_PINs[motorNumber].pinin2, LOW);
  }
}

void moveCar(int valueInt){
  Serial.printf("Got value as %d\n", valueInt);
  switch(valueInt){
    case up:
      rotateMotor(right_motor, forward);
      rotateMotor(left_motor, forward);
    break;
    case down:
      rotateMotor(right_motor, backward);
      rotateMotor(left_motor, backward);
    break;
    case left:
      rotateMotor(right_motor, forward);
      rotateMotor(left_motor, backward);
    break;
    case right:
      rotateMotor(right_motor, backward);
      rotateMotor(left_motor, forward);
    break;
    case Stop:
      rotateMotor(right_motor, Stop);
      rotateMotor(left_motor, Stop);
    break;
    default:

    break;

  }
}

void oncarinputwebsocketevent(AsyncWebSocket *server,
                              AsyncWebSocketClient *client,
                             AwsEventType type, 
                             void *arg, 
                             uint8_t *data, 
                             size_t len)
{
  switch(type){
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(),client->remoteIP().toString().c_str());
    break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n",client->id());
    break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;
      if(info->final && info->index ==0 && info->len && info->opcode==WS_TEXT){
        std::string myData = "";
        myData.assign((char *)data,len);
        std::istringstream ss(myData);
        std::string key,value;
        std::getline(ss,key,',');
        std::getline(ss,value,',');
        Serial.printf("key [%s] Value [%s]\n",key , value); //Speed,120;MoveCar,1
        int valueInt =atoi(value.c_str());
        if(key == "MoveCar"){
          moveCar(valueInt);
        }
        else if(key == "Speed"){
          ledcWrite(pwmspeedchannel,valueInt);
        }
      }
    break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
    break;
    default:
    break;
  }

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
 // wscarinput.cleanupClients();
 wscarinput.cleanupClients();

}

