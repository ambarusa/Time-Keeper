#ifndef MQTT_H
#define MQTT_H

#include "AsyncMqttClient.h"

extern char *mqtt_topic;
extern char *mqtt_effect_topic;
extern char *mqtt_brightness_topic;

void onMqttConnect(bool sessionPresent);
void onMqttMessage(char *topic, char *payload_raw, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);

void Mqtt_init();
void Mqtt_connect();
void Mqtt_state_publish(char *topic, String data);

String Get_mqtt_status();
boolean Get_mqtt_enabled();
String Get_mqtt_host();
String Get_mqtt_port();
String Get_mqtt_qossub();
String Get_mqtt_qospub();
String Get_mqtt_clientid();
String Get_mqtt_username();
String Get_mqtt_autodiscovery();

void Set_mqtt_enabled(int enabled);
void Set_mqtt_host(String host);
void Set_mqtt_port(int port);
void Set_mqtt_qossub(int sub);
void Set_mqtt_qospub(int pub);
void Set_mqtt_clientid(String clientid);
void Set_mqtt_username(String user);
void Set_mqtt_password(String pwd);
void Set_mqtt_autodiscovery(String autodisc);

void Mqtt_100ms_task();

#endif