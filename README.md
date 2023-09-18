# M5-energyUse
A companion app for [smartMeterLogger-esp32](https://github.com/CelliesProjects/smartMeterLogger-esp32). Does not function without a connection to the logger device!

M5-energyUse is an app that shows your current electricity and total energy use for today. 
 
You will need [smartMeterLogger-esp32](https://github.com/CelliesProjects/smartMeterLogger-esp32) connected to your smartmeter to be able to use this app. SmartMeterLogger acts as a serial to websocket bridge and this app connects to the logger to get the energy use data.
### Runs on
-  M5Stack Grey
-  M5Stack Fire
-  TTGO-TM-ESP32<br><img src="https://github.com/LilyGO/TTGO-TM-ESP32/raw/master/Image/TMV1.0.1.jpg" alt="image" width="300px"><br>https://github.com/LilyGO/TTGO-TM-ESP32

## Screenshot
In real life the blooming on the TTGO-TM-ESP32 is not as bad as on the image.<br>
<img src="https://github.com/CelliesProjects/M5-energyUse/assets/24290108/0abcaa00-fc34-4cd9-947d-e6390023ed2c" width="400px"><br>
The 3d printed case for the TTGO board by Jekyll555 fits quite nice and can be found on Thingiverse: https://www.thingiverse.com/thing:3672585

## Setup

Open `wifisetup.h` and

-  Set your WiFi credentials.
-  Set `WS_BRIDGE_HOST` to the IP address of your SmartMeterLogger.
-  You can set a static IP address by changing `SET_STATIC_IP` from `true` to `false`.
If you set this you will also have to set your desired static IP, the gateway address, the subnet and at least one DNS server.
-  Set `NTP_POOL` and `TIMEZONE` to the desired settings.

Now open `M5-energyUse.ino` and

-  Set the #define on line 1 to `LGFX_TTGO_TM` for the TTGO-TM-ESP32 board and to `LGFX_AUTODETECT` for everything else.

Compile and flash the app to your M5Stack.
M5-energyUse should now be able to use your logger to get the data.

## License

This software is released under the MIT license. See [LICENSE.md](LICENSE.md).
