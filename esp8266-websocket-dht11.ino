#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include "DHT.h"
#include <ArduinoJson.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
DHT dht(DHTPIN, DHTTYPE);
StaticJsonDocument<200> jData;
DeserializationError error;
#define led1 15
#define led2 13
#define led3 12
#define led4 14
#define led5 16

String bufClient;
String bufWs;
int tTempr;
int tHumi;
int ledStatus;
unsigned long preMil = 0;
const long interval = 10000; 

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED: {
			USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

			// send message to server when Connected
			webSocket.sendTXT("Connected");
		}
			break;
		case WStype_TEXT:
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      bufWs = String((char*) payload);
      error = deserializeJson(jData, payload);
      if (error) {
        Serial.println("Bukan format JSON!!");
        return;
      } else {
        String led = jData["pin"];
        String value = jData["status"];
        //int ledPin = led
        if (value == "ON" || value == "HIGH"){
          ledStatus = 1;
        } else {
          ledStatus = 0;
        }
        if(led == "led1"){
          digitalWrite(15, ledStatus);
        }
        else if(led == "led2"){
          digitalWrite(13, ledStatus);
        }
        else if(led == "led3"){
          digitalWrite(12, ledStatus);
        }
         else if(led == "led4"){
          digitalWrite(14, ledStatus);
        }
         else if(led == "led5"){
          digitalWrite(16, ledStatus);
        }
        else if(led == "ALL"){
          digitalWrite(15, ledStatus);
          digitalWrite(13, ledStatus);
          digitalWrite(12, ledStatus);
          digitalWrite(14, ledStatus);
          digitalWrite(16, ledStatus);
        }
        Serial.println("Led: " + led + " - " + value);
        
      }
      
			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
        case WStype_PING:
            // pong will be send automatically
            USE_SERIAL.printf("[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            USE_SERIAL.printf("[WSc] get pong\n");
            break;
    }

}

int cekTemp(String var){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int ret;
  if (isnan(h) || isnan(t)) {
    ret = -1;
  } 
  if (var == "temperatur"){
    ret = t;
  } 
  else if (var == "humidity"){
    ret = h;
  } else {
    ret = -1;
  }
  return ret;
}

void setup() {
	USE_SERIAL.begin(115200);
	USE_SERIAL.setDebugOutput(true);
	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	for(uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	WiFiMulti.addAP("PERRY PLATYPUS", "fauziahcantik");
  while(WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL ("server", port, "/url")
	webSocket.begin("websocket-server.bangjii.repl.co", 80);

	// event handler
	webSocket.onEvent(webSocketEvent);

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);
  
  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);  
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
  digitalWrite(led5, LOW);
}

void loop() {
  webSocket.loop();
  unsigned long curMil = millis();
  if ((curMil - preMil) >= interval) { //timer tiap 3 detik
      preMil = curMil;
      int tempr = cekTemp("temperatur"); 
      int humi = cekTemp("humidity"); 
      if (tempr > 0 || humi > 0){
        if(tempr != tTempr || humi != tHumi){
          webSocket.sendTXT("{\"Temperature\": \"" + String(tempr) + "\", \"Humidity\": \"" + String(humi) + "\"}");        
          //Serial.println("{\"Temperature\": \"" + String(tempr) + "\", \"Humidity\": \"" + String(humi) + "\"}");
        }
      }
  } 
  if (Serial.available()) {
    bufClient = "";
    while (Serial.available()) {
      bufClient += Serial.readString();       
    }
  }
  if (bufClient != ""){
    bufClient.trim();
    webSocket.sendTXT(bufClient);
    Serial.println(bufClient);
    bufClient = "";
  }
}
