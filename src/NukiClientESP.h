#pragma once
#include <Arduino.h>
#include <string.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

class NukiClientESP {
    public:
    /////////////////////////////////////////////////////////////////////////////////////
	// Enums for mapping responses and actions
	/////////////////////////////////////////////////////////////////////////////////////
    enum NUKI_lock_states : byte {
        state_uncalibrated = 0,
        state_locked = 1,
        state_unlocking = 2,
        state_unlocked = 3,
        state_locking = 4,
        state_unlatched = 5,
        state_unlocked_lockngo = 6,
        state_unlatching = 7,
        state_motor_blocked = 254,
        state_undefined = 255
    };

    enum NUKI_lock_actions : byte {
        action_unlock = 1,
        action_lock = 2,
        action_unlatch = 3,
        action_lockngo = 4,
        action_lockngo_with_unlatch = 5
    };

    enum NUKI_http_responses : int {
        response_invalid_action = 400,
        response_invalid_token = 401,
        response_authentication_disabled = 403,
        response_samrtlock_unknown = 404,
        response_smartlock_offline = 503
    };

    /////////////////////////////////////////////////////////////////////////////////////
	// Functions for setting up the Library
	/////////////////////////////////////////////////////////////////////////////////////
    NukiClientESP();
    NukiClientESP(String bridgeIp, uint bridgePort);
    NukiClientESP(String bridgeIp, uint bridgePort, String bridgeAuthToken);
    NukiClientESP(String bridgeIp, uint bridgePort, String bridgeAuthToken, uint lockID);
    
    bool discoverBridges(std::vector<String> &bridgeIPs, std::vector<int> &bridgePorts);
    bool authenticateBridge(String &token);
    void useBridge(String bridgeIp, uint bridgePort);
    void useBridgeAuthToken(String authToken);

    bool discoverLocks(std::vector<int> &lockIDs);
    void useLock(uint lockID);

    

    /////////////////////////////////////////////////////////////////////////////////////
	// Functions for controlling the lock
	/////////////////////////////////////////////////////////////////////////////////////
    bool triggerLockAction(NUKI_lock_actions lockAction);
    bool lock();
    bool unlock();
    bool unlatch();

    /////////////////////////////////////////////////////////////////////////////////////
	// Functions for getting the lock state
	/////////////////////////////////////////////////////////////////////////////////////
    NUKI_lock_states getLockState();
    bool isLocked();
    bool getBridgeInfo(String &bridgeInfoJSON);
    
    protected:
    /////////////////////////////////////////////////////////////////////////////////////
	// API endpoint strings
	/////////////////////////////////////////////////////////////////////////////////////
    String _NUKI_API_ENDPOINT_BRIDGEAUTH = "http://[IP]:[PORT]/auth";
    String _NUKI_API_ENDPOINT_BRIDGEINFO = "http://[IP]:[PORT]/info?token=[TOKEN]";
    String _NUKI_API_ENDPOINT_LOCKLIST = "http://[IP]:[PORT]/list?token=[TOKEN]";
    String _NUKI_API_ENDPOINT_LOCKSTATUS = "http://[IP]:[PORT]/lockState?token=[TOKEN]&nukiID=[LOCK_ID]";
    String _NUKI_API_ENDPOINT_LOCKACTION = "http://[IP]:[PORT]/lockAction?token=[TOKEN]&nukiID=[LOCK_ID]&action=[ACTION]";
    String _NUKI_API_ENDPOINT_BRIDGEDISCOVERY = "https://api.nuki.io/discover/bridges";
    String _NUKI_API_ENDPOINT_BRIDGEDISCOVERY_FINGERPRINT = "e5a7c95ba444be76cc1e9120eb1be594486e7cd6";

    /////////////////////////////////////////////////////////////////////////////////////
	// variables
	/////////////////////////////////////////////////////////////////////////////////////
    String _currentBridgeIP = "";
    uint _currentBridgePort = 0;
    String _currentBridgeAuthToken = "";
    uint _currentLockID = 0;
    NUKI_lock_states _currentLockState = NUKI_lock_states::state_undefined;
    //how often to retry if we cant connect to the bridge  or the bridge tells the lock is offline
    int _current_retries_max = 3; 
    int _current_retries_timeout = 250; //retry delay in ms

    HTTPClient _http;

    /////////////////////////////////////////////////////////////////////////////////////
	// helpers
	/////////////////////////////////////////////////////////////////////////////////////

    String _fillAPITemplate(String apiTemplate);
    int _doAPIRequest(String requestURL, String& responseString, String SSLfingerprint = "");
};