#ifndef HW_H
#define HW_H

#include <ESP8266WiFi.h>
#include <String.h>

void Hardware_init();
void Hardware_20ms_task();
void Hardware_1000ms_task();

void Restart_device();

extern boolean Get_reset_button_state();
extern String Get_reset_button_state_str();

#endif