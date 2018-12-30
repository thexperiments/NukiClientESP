#include <Arduino.h>

//ESP8266  wifi stuff
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;

//the nuki library for access to the smart lock bridge
#include <NukiClientESP.h>

NukiClientESP nuki;

//configuration
//#define TEST_NUKI_CLIENT_ESP
#ifndef TEST_NUKI_CLIENT_ESP
const char * WIFI_SSID = "YourFancySSID";
const char * WIFI_PASSWORD = "YourCoolWifiPassword";
const char * BRIDGE_IP = "192.some.bridge.ip";
const int BRIDGE_PORT = 8080;
const char * BRIDGE_TOKEN = "asdf12qwe342";
const int LOCK_ID = 123456789;
#else
  #include "testconfig.h"
#endif

//helpers
void printHeap(){
  Serial.printf("FreeHeap: %i\n", ESP.getFreeHeap());
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Wait for WiFi... ");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //Set up lock
  nuki.useBridge(BRIDGE_IP, BRIDGE_PORT);
  nuki.useBridgeAuthToken(BRIDGE_TOKEN);
  nuki.useLock(LOCK_ID);
  nuki.getLockState();
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(1000);
}

