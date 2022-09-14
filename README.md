# M5-energyUse
A companion app for [smartMeterLogger-esp32](https://github.com/CelliesProjects/smartMeterLogger-esp32).

M5-energyUse is an app for a M5Stack V1 that shows your current and total energy use for today. 
 
You will need [smartMeterLogger-esp32](https://github.com/CelliesProjects/smartMeterLogger-esp32) connected to your smartmeter to be able to use this app. SmartMeterLogger acts as a serial to websocket bridge and this app connects to the logger to get the energy use data.

## Screenshot
![M5-Energy-Use](https://user-images.githubusercontent.com/24290108/190137984-4887e4e1-6e80-4dc4-bcf6-7d4d85cc3f45.JPG)

## Setup

Open `wifisetup.h` and

-  Set your WiFi credentials.
-  Set `WS_BRIDGE_HOST` to the IP address of your SmartMeterLogger.
-  You can set a static ip address by changing `SET_STATIC_IP` from `true` to `false`.
If you set this you will also have to set your desired static IP, the gateway address, the subnet and at least one DNS server.
-  Set `NTP_POOL` and `TIMEZONE` to the desired settings.

Compile and flash the app to your M5Stack.
M5-energyUse should now be able to use your logger to get the data.
