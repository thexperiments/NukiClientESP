# NukiClientESP
Library for accessing the nuki smartlock with an ESP8266. 

Autodiscovery and authentication supported.

Required libraries:
* ArduinoJson 5.13.x
* ESP8266 libraries (installed if you install the esp8266 board files)

Tested with an nodeMCU 1.0

Usage:
* See Examples
* * Autodiscovering the lock and authenticating with the bridge.
* * Using a static configuration to operate a lock

Todo:
* Clean up
* Reduce use of String to use less RAM
* Support callbacks to update the lock state
* Real world testing

With some modification this should also work on other platforms. 
I am not affiliated with Nuki Home Solutions, this not an official library, NUKI is a trademark of Nuki Home Solutions.
