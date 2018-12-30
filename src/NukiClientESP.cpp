/*
Library for accessing the NUKI bridge.
Intended for use with ESP8266 but also should work with other boards
*/
#include "NukiClientESP.h"

NukiClientESP::NukiClientESP(){
    // set up the http timeout as the lock sometimes takes quite some time to respond
    _http.setTimeout(10000); //timout is in ms -> 10000 ms = 10 seconds
}
NukiClientESP::NukiClientESP(String bridgeIp, uint bridgePort){
    NukiClientESP();
    useBridge(bridgeIp, bridgePort);
}
NukiClientESP::NukiClientESP(String bridgeIp, uint bridgePort, String bridgeAuthToken){
    useBridgeAuthToken(bridgeAuthToken);
    NukiClientESP(bridgeIp,bridgePort);
}
NukiClientESP::NukiClientESP(String bridgeIp, uint bridgePort, String bridgeAuthToken, uint lockID){
    useLock(lockID);
    NukiClientESP(bridgeIp,bridgePort, bridgeAuthToken);
}

bool NukiClientESP::discoverBridges(std::vector<String> &bridgeIPs, std::vector<int> &bridgePorts){
    String responseString;
    StaticJsonBuffer<1024> jsonBuffer;

    Serial.printf("Discovering bridges via api.nuki.io...\n");
    int httpResponseCode = _doAPIRequest(_NUKI_API_ENDPOINT_BRIDGEDISCOVERY, responseString, _NUKI_API_ENDPOINT_BRIDGEDISCOVERY_FINGERPRINT);
    JsonObject& root = jsonBuffer.parseObject(responseString);
    JsonArray& bridges = root["bridges"];

    for (uint i = 0; i < bridges.size(); i++){
        int bridgeID = bridges[i]["bridgeId"].as<int>();
        int bridgePort = bridges[i]["port"].as<int>();
        String bridgeIP = bridges[i]["ip"];
        bridgeIPs.push_back(bridgeIP);
        bridgePorts.push_back(bridgePort);
        Serial.printf("Found bridge: %s:%u ( %u )\n", bridgeIP.c_str(), bridgePort, bridgeID);
    }
    
    if ((httpResponseCode == 200) && (root["errorCode"].as<int>() == 0)) {
        return true;
    }

    return false;
}

bool NukiClientESP::authenticateBridge(String &token){
    String request = _fillAPITemplate(_NUKI_API_ENDPOINT_BRIDGEAUTH);
    String responseString;
    StaticJsonBuffer<1024> jsonBuffer;

    Serial.printf("Authenticating with Bridge %s\n", _currentBridgeIP.c_str());
    int httpResponseCode = _doAPIRequest(request,responseString);
    JsonObject& root = jsonBuffer.parseObject(responseString);
    
    if ((httpResponseCode == 200) && (root["success"].as<bool>() == true)) {
        token = root["token"].as<String>();
        Serial.printf("Acquired token: %s\n", token.c_str());
        return true;
    }

    return false;
}

void NukiClientESP::useBridge(String bridgeIp, uint bridgePort){
    _currentBridgeIP = bridgeIp;
    _currentBridgePort = bridgePort;
}

void NukiClientESP::useBridgeAuthToken(String bridgeAuthToken){
    _currentBridgeAuthToken = bridgeAuthToken;
}

void NukiClientESP::useLock(uint lockID){
    _currentLockID = lockID;
}

bool NukiClientESP::discoverLocks(std::vector<int> &lockIDs){
    String request = _fillAPITemplate(_NUKI_API_ENDPOINT_LOCKLIST);
    String responseString;
    StaticJsonBuffer<1024> jsonBuffer;
    int httpResponseCode = _doAPIRequest(request,responseString);
    JsonArray& root = jsonBuffer.parseArray(responseString);
    for (uint i = 0; i < root.size(); i++){
        int lockID = root[i]["nukiId"].as<int>();
        String lockName = root[i]["name"];
        lockIDs.push_back(lockID);
        Serial.printf("Found lock: %s ( %u )\n", lockName.c_str(), lockID);
    }

    if (httpResponseCode == 200) {
        return true;
    }

    return false;
}

bool NukiClientESP::triggerLockAction(NUKI_lock_actions lockAction){
    String request = _fillAPITemplate(_NUKI_API_ENDPOINT_LOCKACTION);
    String responseString;
    StaticJsonBuffer<1024> jsonBuffer;

    request.replace("[ACTION]",String(lockAction));
    Serial.printf("Triggering lock action: %u\n", lockAction);
    int httpResponseCode = _doAPIRequest(request,responseString);
    JsonObject& root = jsonBuffer.parseObject(responseString);
    
    if ((httpResponseCode == 200) && (root["success"].as<bool>() == true)) {
        return true;
    }

    return false;
}

bool NukiClientESP::lock(){
    return triggerLockAction(NUKI_lock_actions::action_lock);
}
bool NukiClientESP::unlock(){
    return triggerLockAction(NUKI_lock_actions::action_unlock);
}
bool NukiClientESP::unlatch(){
    return triggerLockAction(NUKI_lock_actions::action_unlatch);
}

NukiClientESP::NUKI_lock_states NukiClientESP::getLockState(){
    String request = _fillAPITemplate(_NUKI_API_ENDPOINT_LOCKSTATUS);
    String responseString;
    StaticJsonBuffer<1024> jsonBuffer;

    Serial.printf("Getting lock state\n");
    int httpResponseCode = _doAPIRequest(request,responseString);
    JsonObject& root = jsonBuffer.parseObject(responseString);
    
    if ((httpResponseCode == 200) && (root["success"].as<bool>() == true)) {
        _currentLockState = (NUKI_lock_states)root["state"].as<int>();
        Serial.printf("Current state is: %s ( %u )\n", root["stateName"].as<char*>(), _currentLockState);
        return _currentLockState;
    }

    return NUKI_lock_states::state_undefined;
}
bool NukiClientESP::isLocked(){
    return _currentLockState == NUKI_lock_states::state_locked;
}

String NukiClientESP::_fillAPITemplate(String apiTemplate){
    apiTemplate.replace("[IP]",_currentBridgeIP);
    apiTemplate.replace("[PORT]",String(_currentBridgePort));
    apiTemplate.replace("[TOKEN]",_currentBridgeAuthToken);
    apiTemplate.replace("[LOCK_ID]",String(_currentLockID));
    apiTemplate.replace("[PORT]",String(_currentBridgePort));
    return apiTemplate;
}

int NukiClientESP::_doAPIRequest(String requestURL, String& responseString, String SSLfingerprint){
    int retries = 0;
    int httpCode = -255;
    Serial.printf("Request URL: %s\n", requestURL.c_str());
    if (SSLfingerprint == ""){
        _http.begin(requestURL);
    }
    else{
        _http.begin(requestURL,SSLfingerprint);
    }
    
    
    while(retries < _current_retries_max){
        httpCode = _http.GET();
        retries++;

        if (httpCode > 0){
            if (httpCode == HTTP_CODE_OK) {
                responseString = _http.getString();
                break;
            }
            else{
                Serial.printf("[HTTP] GET... failed, error (%i): %s\n", httpCode, _http.errorToString(httpCode).c_str());
                delay(_current_retries_timeout);
            }
        
        }
        else{
            Serial.printf("[HTTP] GET... failed, error (%i): %s\n", httpCode, _http.errorToString(httpCode).c_str());
            delay(_current_retries_timeout);
        }
    }

    _http.end();
    Serial.println(responseString.c_str());
    Serial.println(httpCode);
    return httpCode;
}