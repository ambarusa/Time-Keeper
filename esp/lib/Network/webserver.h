#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>

void Webserver_start();
void Webserver_stop();

void Notify_ws_clients(String key, String data);

#endif