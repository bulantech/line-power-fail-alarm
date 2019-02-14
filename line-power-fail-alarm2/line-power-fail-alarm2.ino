#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#define SW_DET  D6
#define SW_PIN  D5
#define LED D7

int state = 0; 
int stateCount = 0;
int firstStart = 1;

#define POWER_SEND_COUNT  3 //ต้องการให้แจ้งเตือน 3 ครั้ง
#define POWER_SEND_TIME   60 //ห่างกัน 60 วินาที

int powerUpSendCount = POWER_SEND_COUNT; 
int powerUpSendTime = POWER_SEND_TIME; 
int powerDownSendCount = POWER_SEND_COUNT; 
int powerDownSendTime = POWER_SEND_TIME; 

// Line config
#define LINE_TOKEN "LhVtD4p7uL879nYR9EoACUvZuC30gNonSQ1RaNC5jUP"

// https://meyerweb.com/eric/tools/dencoder/
String powerDown = "%E0%B8%95%E0%B8%AD%E0%B8%99%E0%B8%99%E0%B8%B5%E0%B9%89%E0%B9%82%E0%B8%A3%E0%B8%87%E0%B9%80%E0%B8%88%E0%B8%99%E0%B9%84%E0%B8%9F%E0%B8%9F%E0%B9%89%E0%B8%B2%E0%B9%82%E0%B8%A3%E0%B8%87%E0%B8%97%E0%B8%B5%E0%B9%884%20%E0%B9%84%E0%B8%9F%E0%B8%9F%E0%B9%89%E0%B8%B2%E0%B8%94%E0%B8%B1%E0%B8%9A%E0%B8%AD%E0%B8%A2%E0%B8%B9%E0%B9%88";
String powerUp = "%E0%B8%95%E0%B8%AD%E0%B8%99%E0%B8%99%E0%B8%B5%E0%B9%89%E0%B9%82%E0%B8%A3%E0%B8%87%E0%B9%80%E0%B8%88%E0%B8%99%E0%B9%84%E0%B8%9F%E0%B8%9F%E0%B9%89%E0%B8%B2%E0%B9%82%E0%B8%A3%E0%B8%87%E0%B8%97%E0%B8%B5%E0%B9%884%20%E0%B9%84%E0%B8%9F%E0%B8%9F%E0%B9%89%E0%B8%B2%E0%B8%A1%E0%B8%B2%E0%B8%9B%E0%B8%81%E0%B8%95%E0%B8%B4%E0%B9%81%E0%B8%A5%E0%B9%89%E0%B8%A7";

void Line_Notify_Send(String msg) {
  digitalWrite(LED, 0); //on
//  return; //test
  WiFiClientSecure client; 

  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return;   
  }

  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
  req += "Cache-Control: no-cache\r\n";
  req += "User-Agent: ESP8266\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(String("message=" + msg).length()) + "\r\n";
  req += "\r\n";
  req += "message=" + msg;
  // Serial.println(req);
  client.print(req);
    
  delay(20);

  Serial.println("-------------");
  while(client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    Serial.println(line);
  }
  Serial.println("-------------");
  digitalWrite(LED, 1); //off
}

void ledBlink() {
  digitalWrite(LED, 0); //on
  delay(10);
  digitalWrite(LED, 1); //off
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(SW_DET, INPUT_PULLUP);
  pinMode(LED, OUTPUT);  
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  Serial.println("Reset wifi config?:");
  for(int i=5; i>0; i--){
    Serial.print(String(i)+" "); 
    ledBlink();
    delay(1000);
  }
  
  //reset saved settings
  if(digitalRead(SW_PIN) == LOW) // Press button
  {
    Serial.println();
    Serial.println("Reset wifi config");
    digitalWrite(LED, 0); //on
    wifiManager.resetSettings(); 
  }    
  
  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
}

void loop() {
  ledBlink();

  if(!digitalRead(SW_DET)) { //power down
    if(!state) { //แจ้งเตือนไฟดับครั้งแรก
      state = 1;
      powerDownSendCount = POWER_SEND_COUNT; //กำหนดค่านับจำนวนกี่ครั้ง
      powerDownSendTime = 1000*POWER_SEND_TIME; //กำหนดค่าเวลาในการส่ง
      Serial.println("POWER DOWN");    
      Line_Notify_Send(powerDown);      
    } else {
      powerDownSendTime -= 5000;
      if(powerDownSendTime <= 0) {
        powerDownSendTime = 1000*POWER_SEND_TIME;
        if(powerDownSendCount) {
          --powerDownSendCount;
          Serial.print(String(powerDownSendCount));
          Serial.println(" POWER DOWN");    
          Line_Notify_Send(powerDown);    
        }
      }
    }
    delay(5000);
  } else { //power up
    if(firstStart) {
      firstStart = 0;      
      Serial.println("Smart Detector Start");
      Line_Notify_Send("Smart Detector Start");
    } else {
      if(state) { //แจ้งเตือนไฟติดครั้งแรก
        state = 0;
        powerUpSendCount = POWER_SEND_COUNT; //กำหนดค่านับจำนวนกี่ครั้ง
        powerUpSendTime = 1000*POWER_SEND_TIME; //กำหนดค่าเวลาในการส่ง
        Serial.println("POWER UP");
        Line_Notify_Send(powerUp);
      } else {
        powerUpSendTime -= 2000;
        if(powerUpSendTime <= 0) {
          powerUpSendTime = 1000*POWER_SEND_TIME;
          if(powerUpSendCount) {
            --powerUpSendCount;
            Serial.print(String(powerUpSendCount));
            Serial.println(" POWER UP");
            Line_Notify_Send(powerUp);    
          }
        }
      }     
    } 
    delay(2000);   
  }   
  
}
