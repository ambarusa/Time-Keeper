#ifndef NETWORK_H
#define NETWORK_H

#include "webserver.h"
#include "mqtt.h"
#include "clock.h"

#if defined(FLEURIE)
#define DEVICE_NAME "Fleurie"
#elif defined(PIXIE)
#define DEVICE_NAME "Pixie"
#elif (defined(FLEURIE) && defined(PIXIE)) || (!defined(FLEURIE) && !defined(PIXIE))
#error "Please define either FLEURIE or PIXIE"
#endif

void Network_init();
void Network_reset();
void Network_100ms_task();
void Network_1000ms_task();

String Get_wifi_ssid();
void Set_wifi_credentials(String ssid, String pwd);

#endif