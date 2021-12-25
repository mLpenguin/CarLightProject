# Car Light Project
Arduino code for an esp8266 using NeoPixels for both node and controller. Light node is controlled from either a webpage or controller in the cabin. 

## Features:
* Light node and controller
* HTTP webserver to control light node
* AP allowing for control of node via browser
* Updating via OTA

## Known Issues/Bugs:
* Rainbow light program triggers soft restart on controller (Unknown if on node as well).
* Occasionally problems with OTA.
* Unable to power off node via ignition.
  * Power drain until car auto-shutoff due to low voltage on battery.

## Future:
* Node detects when car ignition is on or off (Autoshutdown for power saving).
* Add more light patterns
* Refine UI for browser control