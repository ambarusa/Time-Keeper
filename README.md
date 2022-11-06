# Introduction
# Flashing

## Arduino

Download the latest precompiled [`firmware.hex`](https://github.com/ambarusa/Time-Keeper/tree/master/build) file from one of the `atm328p_xxx` directories (depending on your clock type), and flash it with this UI:

- AVRDUDESS ([Releases](https://github.com/ZakKemble/AVRDUDESS/releases))

## ESP8266
Download the latest [`combined.bin`](https://github.com/ambarusa/Time-Keeper/tree/master/build) file  from one of the `esp_xxx` directories (depending on your clock type). Then choose a UI, to flash the device, for example:

- Tasmotizer ([Releases](https://github.com/tasmota/tasmotizer/releases))
- NodeMCU PyFlasher ([Releases](https://github.com/marcelstoer/nodemcu-pyflasher/releases))

### OTA Flashing
You can flash your device, if the current firmware running supports OTA (an older Time Keeper firmware, or just the BasicOTA.ino example sketch).

1. Download this UI: [ESP-Ota-UI](https://github.com/Nikfinn99/ESP-Ota-UI/releases). You will need to run the application twice, once for the firmware and once for the filesystem.
2. Download the latest precompiled [`firmware.bin`](https://github.com/ambarusa/Time-Keeper/tree/master/build) and [`littlefs.bin`](https://github.com/ambarusa/Time-Keeper/tree/master/build) files, from the directory matching your clock type.