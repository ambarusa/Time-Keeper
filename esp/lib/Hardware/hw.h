#ifndef HW_H
#define HW_H

#include <ESP8266WiFi.h>
#include <String.h>

#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__);
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTF(...)
#endif

void Hardware_init();
void Hardware_20ms_task();
void Hardware_1000ms_task();

void Restart_device(boolean soft_b);

extern boolean Get_reset_button_state();
extern String Get_reset_button_state_str();

#endif