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
  std::vector<String> bridgeIPs;
  std::vector<int> bridgePorts;
  nuki.discoverBridges(bridgeIPs, bridgePorts);
  nuki.useBridge(bridgeIPs[0], bridgePorts[0]);
  String token;
  nuki.authenticateBridge(token);
  nuki.useBridgeAuthToken(token);
  std::vector<int> lockIDs;
  nuki.discoverLocks(lockIDs);
  nuki.useLock(lockIDs[0]);
  // interact with lock
  nuki.getLockState();
  nuki.unlock();
  delay(5000);
  nuki.getLockState();
  delay(5000);
  nuki.lock();
  nuki.getLockState();
}

void loop() {
  // put your main code here, to run repeatedly:
}

