#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#define SW_DET  D6
#define SW_PIN  D8
#define LED D7

int state = 0; 
int stateCount = 0;
int firstStart = 1;

// Line config
#define LINE_TOKEN "ez4tu8UFv2Yw7ELVCeyrfSfFqOYTUlb6QqTfZJotMp3"

void Line_Notify_Send(String msg) {
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
  if(!digitalRead(SW_DET)) { //power down
    if(!state) {
      state = 1;
      digitalWrite(LED, 0); //on
      Line_Notify_Send("POWER DOWN");      
    }
    delay(1000);
  } else { //power up
    if(firstStart) {
      firstStart = 0;
      digitalWrite(LED, 0); //on
      Line_Notify_Send("FIRST START");
    } else {
      state = 0;
      digitalWrite(LED, 0); //on
      Line_Notify_Send("POWER UP");
    }    
  }
  
  ledBlink();
  delay(2000);
}
