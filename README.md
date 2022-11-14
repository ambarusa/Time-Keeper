# Introduction
# Flashing

## Arduino
### Serial
![](./docs/screenshots/avrdudess.png)

1. Download this UI: [AVRDUDESS](https://github.com/ZakKemble/AVRDUDESS/releases).
2. Download the latest precompiled [`firmware.hex`](https://github.com/ambarusa/Time-Keeper/tree/master/build) file from one of the `build/atm328p_xxx` directories (depending on your clock type).
3. Open the flasher tool, and load the **Arduino Nano (ATmega328P)** preset (as it's on the right side of the screenshot above). This will set up the tool properly.
4. Choose the right COM port at the top.
5. Add the previously downloaded `firmware.hex` file under the Flash section.
5. Hit Program!, and wait!

## ESP8266
## Serial
Download the latest [`combined.bin`](https://github.com/ambarusa/Time-Keeper/tree/master/build) file  from one of the `build/esp_xxx` directories (depending on your clock type). Then choose a UI, to flash the device, for example:

- Tasmotizer ([Releases](https://github.com/tasmota/tasmotizer/releases))
- NodeMCU PyFlasher ([Releases](https://github.com/marcelstoer/nodemcu-pyflasher/releases))

### OTA Flashing
You can flash your device with OTA, if the current firmware running supports it (an older Time Keeper firmware, or just the BasicOTA.ino example sketch).

![](./docs/screenshots/ota_ui.png)

1. Download this UI: [ESP-Ota-UI](https://github.com/Nikfinn99/ESP-Ota-UI/releases). You will need to run the application twice, once for the firmware and once for the filesystem.
2. With OTA, you can't flash `combined.bin`, because it's too large. Use the latest precompiled [`firmware.bin`](https://github.com/ambarusa/Time-Keeper/tree/master/build) and [`littlefs.bin`](https://github.com/ambarusa/Time-Keeper/tree/master/build) files instead, from the directory matching your clock type.
3. Open the Ota UI, go to Devices tab, and add your device by typing a name, and it's IP Address (see left side of the screenshot above).

**PLEASE BE CAREFUL AT THE FOLLOWING STEP**

4. Go back to the main Upload tab (right side of the screenshot above), choose the binary you want to flash, choose the device you added previously, and finally choose what type of binary you want to flash ([`firmware.bin`] = Program, [`littlefs.bin`] = SPIFFS).
5. Hit Upload, and wait!