#include "mqtt.h"

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include "Ticker.h"
#include "ArduinoJson.h"
#include "clock.h"
#include "memory.h"
#include "network.h"
#include "webserver.h"
#include "hw.h"

#define MQTT_RECONN_RETRIES 5
#define TOPIC_MAX_LEN 96
#define AUTODISCOVERY_MAX_LEN 32

Ticker mqtt_reconn_ticker(Mqtt_connect, 10000);

boolean mqtt_enabled_u8;
String mqtt_status = "Not enabled";

AsyncMqttClient amqtt_client;

char mqtt_host[EEPROM_MQTT_HOST_SIZE];
char mqtt_clientid[EEPROM_MQTT_CLIENTID_SIZE];
char mqtt_username[EEPROM_MQTT_USER_SIZE];
char mqtt_password[EEPROM_MQTT_PWD_SIZE];
uint16_t mqtt_port_u16;
uint8_t qossub;
uint8_t qospub;

const char *mqtt_birth_payload = "online";
const char *mqtt_will_payload = "offline";

const char availability[] = "/status";
char autodiscovery[AUTODISCOVERY_MAX_LEN] = "homeassistant";
const char mqtt_set_substr[] = "/set";

char mqtt_availability_topic[TOPIC_MAX_LEN];

char mqtt_topic[TOPIC_MAX_LEN];
char mqtt_cmd_topic[TOPIC_MAX_LEN];
char mqtt_config_topic[TOPIC_MAX_LEN];

#if defined(FLEURIE)
const char mqtt_brightness_substr[] = "/brightness";
const char mqtt_effect_substr[] = "/effect";
char mqtt_brightness_topic[TOPIC_MAX_LEN];
char mqtt_brightness_cmd_topic[TOPIC_MAX_LEN];
char mqtt_effect_topic[TOPIC_MAX_LEN];
char mqtt_effect_cmd_topic[TOPIC_MAX_LEN];
#endif

void Mqtt_discovery_publish();

static bool Topic_equals(const char *left, const char *right)
{
   while (*left != '\0' && *right != '\0')
   {
      if (*left != *right)
         return false;

      ++left;
      ++right;
   }

   return *left == *right;
}

static bool Payload_equals(const char *payload, size_t payload_len, const char *expected)
{
   size_t index = 0;
   while (index < payload_len && expected[index] != '\0')
   {
      if (payload[index] != expected[index])
         return false;

      ++index;
   }

   return index == payload_len && expected[index] == '\0';
}

#if defined(FLEURIE)
static int Parse_positive_int(const char *payload, size_t payload_len)
{
   int value = 0;
   bool has_digit = false;

   for (size_t index = 0; index < payload_len; ++index)
   {
      const char current = payload[index];
      if (current < '0' || current > '9')
         return has_digit ? value : 0;

      has_digit = true;
      value = value * 10 + (current - '0');
   }

   return has_digit ? value : 0;
}
#endif

static void Copy_string_setting(char *destination, size_t destination_size, const String &value)
{
   value.toCharArray(destination, destination_size);
}

String Get_mqtt_status()
{
   String result = mqtt_status;
   if (!WiFi.isConnected())
      result += " when offline";
   return result;
}
boolean Get_mqtt_enabled()
{
   return mqtt_enabled_u8;
}
String Get_mqtt_host()
{
   return String(mqtt_host);
}
String Get_mqtt_port()
{
   return String(mqtt_port_u16);
}
String Get_mqtt_qossub()
{
   return String(qossub);
}
String Get_mqtt_qospub()
{
   return String(qospub);
}
String Get_mqtt_clientid()
{
   return String(mqtt_clientid);
}
String Get_mqtt_username()
{
   return String(mqtt_username);
}
String Get_mqtt_autodiscovery()
{
   return autodiscovery;
}

void Set_mqtt_enabled(int enabled)
{
   /*! Checking if the MQTT enabled state is equal to the parameter is intentionally skipped.
       This allows to disable the MQTT even though the connection to an MQTT server couldn't
       be made, and the system disabled already the MQTT for this runtime. */
   mqtt_enabled_u8 = (boolean)enabled;
   if (!mqtt_enabled_u8)
      mqtt_status = "Not enabled";
   Memory_write((char *)&mqtt_enabled_u8, EEPROM_MQTT_ENABLED_ADDR, sizeof(mqtt_enabled_u8));
}
void Set_mqtt_host(String host)
{
   if (host.equals(mqtt_host))
      return;

   Copy_string_setting(mqtt_host, EEPROM_MQTT_HOST_SIZE, host);
   Memory_write(mqtt_host, EEPROM_MQTT_HOST_ADDR, EEPROM_MQTT_HOST_SIZE);
}
void Set_mqtt_port(int port)
{
   mqtt_port_u16 = (uint16_t)port;
   Memory_write((char *)&mqtt_port_u16, EEPROM_MQTT_PORT_ADDR, EEPROM_MQTT_PORT_SIZE);
}
void Set_mqtt_qossub(int sub)
{
   qossub = (uint8_t)sub;
   Memory_write((char *)&qossub, EEPROM_MQTT_QOSSUB_ADDR, sizeof(qossub));
}
void Set_mqtt_qospub(int pub)
{
   qospub = (uint8_t)pub;
   Memory_write((char *)&qospub, EEPROM_MQTT_QOSPUB_ADDR, sizeof(qospub));
}
void Set_mqtt_clientid(String clientid)
{
   if (clientid.equals(mqtt_clientid))
      return;

   Copy_string_setting(mqtt_clientid, EEPROM_MQTT_CLIENTID_SIZE, clientid);
   Memory_write((char *)mqtt_clientid, EEPROM_MQTT_CLIENTID_ADDR, EEPROM_MQTT_CLIENTID_SIZE);
}
void Set_mqtt_username(String user)
{
   if (user.equals(mqtt_username))
      return;

   Copy_string_setting(mqtt_username, EEPROM_MQTT_USER_SIZE, user);
   Memory_write((char *)mqtt_username, EEPROM_MQTT_USER_ADDR, EEPROM_MQTT_USER_SIZE);
}
void Set_mqtt_password(String pwd)
{
   if (pwd.equals(mqtt_password))
      return;

   Copy_string_setting(mqtt_password, EEPROM_MQTT_PWD_SIZE, pwd);
   Memory_write((char *)mqtt_password, EEPROM_MQTT_PWD_ADDR, EEPROM_MQTT_PWD_SIZE);
}
void Set_mqtt_autodiscovery(String autodisc)
{
   if (autodisc.equals(autodiscovery))
      return;

   Copy_string_setting(autodiscovery, AUTODISCOVERY_MAX_LEN, autodisc);
}

void onMqttConnect(bool sessionPresent)
{
   mqtt_reconn_ticker.stop();
   mqtt_status = "MQTT: Connected to MQTT";

   DEBUG_PRINTF("\nMQTT: Connected to %s:%i as %s\n", mqtt_host, mqtt_port_u16, mqtt_clientid);
   Notify_ws_clients("MQTT_STATUS", Get_mqtt_status());

   amqtt_client.subscribe(mqtt_cmd_topic, qossub);
   amqtt_client.publish(mqtt_availability_topic, qospub, true, mqtt_birth_payload);

   Mqtt_state_publish(mqtt_topic, (Get_esp_states().lightMode == LIGHT_MODE_OFF) ? "OFF" : "ON");
#if defined(FLEURIE)
   amqtt_client.subscribe(mqtt_brightness_cmd_topic, qossub);
   amqtt_client.subscribe(mqtt_effect_cmd_topic, qossub);
   Mqtt_state_publish(mqtt_brightness_topic, String(Get_esp_states().lightBrightness));
   Mqtt_state_publish(mqtt_effect_topic, Get_light_mode_str());
#endif

   Mqtt_discovery_publish();
}

void onMqttMessage(char *topic, char *payload_raw, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
   if (len == 0 || len > 100) return; // Safety check

   DEBUG_PRINTF("\nMQTT: Recieved [%s]: %.*s\n", topic, int(len), payload_raw);

   if (Topic_equals(topic, mqtt_cmd_topic))
   {
      if (Payload_equals(payload_raw, len, "OFF"))
         Set_lightMode(LIGHT_MODE_OFF);
      else if (Payload_equals(payload_raw, len, "ON"))
         if (Get_esp_states().lightMode == LIGHT_MODE_OFF)
            Set_lightMode(LIGHT_MODE_MANUAL);
   }

#if defined(FLEURIE)
   if (Topic_equals(topic, mqtt_effect_cmd_topic))
   {
      if (Payload_equals(payload_raw, len, "Manual"))
         Set_lightMode(LIGHT_MODE_MANUAL);
      else if (Payload_equals(payload_raw, len, "Automatic"))
         Set_lightMode(LIGHT_MODE_AUTO);
   }

   if (Topic_equals(topic, mqtt_brightness_cmd_topic))
      Set_lightBrightness(Parse_positive_int(payload_raw, len));
#endif
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
   DEBUG_PRINT("MQTT: Disconnected from MQTT, reason: ");
   if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT)
   {
      mqtt_status = "Bad server fingerprint";
   }
   else if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED)
   {
      mqtt_status = "TCP Disconnected";
   }
   else if (reason == AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION)
   {
      mqtt_status = "Unacceptable Protocol version";
   }
   else if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED)
   {
      mqtt_status = "MQTT Identifier rejected";
   }
   else if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE)
   {
      mqtt_status = "MQTT server unavailable";
   }
   else if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS)
   {
      mqtt_status = "MQTT malformed credentials";
   }
   else if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED)
   {
      mqtt_status = "MQTT not authorized";
   }
   else if (reason == AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE)
   {
      mqtt_status = "Not enough space on ESP";
   }
   DEBUG_PRINTLN(mqtt_status);
   if (mqtt_reconn_ticker.counter() == MQTT_RECONN_RETRIES)
   {
      mqtt_reconn_ticker.stop();
      mqtt_enabled_u8 = false;
      mqtt_status = "Couldn't connect. Turned off until a new configuration";
      DEBUG_PRINTLN("MQTT: Couldn't connect. Disabling it until a new configuration");
      Notify_ws_clients("MQTT_EN", String(Get_mqtt_enabled()));
   }
   else if (WiFi.isConnected() && mqtt_reconn_ticker.state() == STOPPED)
      mqtt_reconn_ticker.start();

   Notify_ws_clients("MQTT_STATUS", Get_mqtt_status());
}

void Mqtt_init()
{
   Memory_read((char *)&mqtt_enabled_u8, EEPROM_MQTT_ENABLED_ADDR, sizeof(mqtt_enabled_u8));
   Memory_read((char *)mqtt_host, EEPROM_MQTT_HOST_ADDR, EEPROM_MQTT_HOST_SIZE);
   Memory_read((char *)&mqtt_port_u16, EEPROM_MQTT_PORT_ADDR, EEPROM_MQTT_PORT_SIZE);
   Memory_read((char *)mqtt_clientid, EEPROM_MQTT_CLIENTID_ADDR, EEPROM_MQTT_CLIENTID_SIZE);
   Memory_read((char *)mqtt_username, EEPROM_MQTT_USER_ADDR, EEPROM_MQTT_USER_SIZE);
   Memory_read((char *)mqtt_password, EEPROM_MQTT_PWD_ADDR, EEPROM_MQTT_PWD_SIZE);
   Memory_read((char *)&qossub, EEPROM_MQTT_QOSSUB_ADDR, sizeof(qossub));
   Memory_read((char *)&qospub, EEPROM_MQTT_QOSPUB_ADDR, sizeof(qospub));

   if (!mqtt_enabled_u8)
   {
      DEBUG_PRINTLN("Network: MQTT not enabled.");
      return;
   }

   snprintf(mqtt_availability_topic, sizeof(mqtt_availability_topic), "%s%s", mqtt_clientid, availability);
   snprintf(mqtt_topic, sizeof(mqtt_topic), "%s/state", mqtt_clientid);
   snprintf(mqtt_cmd_topic, sizeof(mqtt_cmd_topic), "%s/state%s", mqtt_clientid, mqtt_set_substr);
   snprintf(mqtt_config_topic, sizeof(mqtt_config_topic), "%s/light/%s/config", autodiscovery, mqtt_clientid);

#if defined(FLEURIE)
   snprintf(mqtt_brightness_topic, sizeof(mqtt_brightness_topic), "%s%s", mqtt_clientid, mqtt_brightness_substr);
   snprintf(mqtt_brightness_cmd_topic, sizeof(mqtt_brightness_cmd_topic), "%s%s%s", mqtt_clientid, mqtt_brightness_substr, mqtt_set_substr);
   snprintf(mqtt_effect_topic, sizeof(mqtt_effect_topic), "%s%s", mqtt_clientid, mqtt_effect_substr);
   snprintf(mqtt_effect_cmd_topic, sizeof(mqtt_effect_cmd_topic), "%s%s%s", mqtt_clientid, mqtt_effect_substr, mqtt_set_substr);
#endif

   amqtt_client.setWill(mqtt_availability_topic, 2, true, mqtt_will_payload, 0);
   amqtt_client.setClientId(mqtt_clientid);
   amqtt_client.setCredentials(mqtt_username, mqtt_password);
   amqtt_client.setServer(mqtt_host, mqtt_port_u16);
   amqtt_client.onConnect(onMqttConnect);
   amqtt_client.onDisconnect(onMqttDisconnect);
   amqtt_client.onMessage(onMqttMessage);
}

void Mqtt_connect()
{
   if (!mqtt_enabled_u8)
      return;

   amqtt_client.connect();
}

void Mqtt_discovery_publish()
{
   JsonDocument root;

   root["name"] = DEVICE_NAME;
#ifdef ESP32
   root["unique_id"] = String((uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF)) + "-" + String(DEVICE_NAME);
#else
   root["unique_id"] = String(ESP.getChipId()) + "-" + String(DEVICE_NAME);
#endif
   root["icon"] = "mdi:clock-outline";
   root["availability_topic"] = mqtt_availability_topic;
   root["payload_available"] = mqtt_birth_payload;
   root["payload_not_available"] = mqtt_will_payload;
   root["state_topic"] = mqtt_topic;
   root["command_topic"] = mqtt_cmd_topic;
#if defined(FLEURIE)
   root["brightness"] = "true";
   root["brightness_scale"] = 100;
   root["brightness_state_topic"] = mqtt_brightness_topic;
   root["brightness_command_topic"] = mqtt_brightness_cmd_topic;
   root["effect"] = "true";
   root["effect_state_topic"] = mqtt_effect_topic;
   root["effect_command_topic"] = mqtt_effect_cmd_topic;
   JsonArray fx_list = root["effect_list"].to<JsonArray>();
   fx_list.add("Manual");
   fx_list.add("Automatic");
#endif
   // JsonObject device = root.createNestedObject("device");
   JsonObject device = root["device"].to<JsonObject>();
#ifdef ESP32
   device["identifiers"] = String((uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF));
#else
   device["identifiers"] = String(ESP.getChipId());
#endif
   device["manufacturer"] = "Time-Keeper";
   device["model"] = DEVICE_NAME;
   device["name"] = "Time-Keeper";
   device["sw_version"] = "1.0";

   char payload[1024];
   serializeJson(root, payload);

   uint16_t publish_id = amqtt_client.publish(mqtt_config_topic, qospub, true, payload);
   (void)publish_id;
   DEBUG_PRINTF("MQTT Disovery Send [ID %i]: [%s]: \n%s\n", publish_id, mqtt_config_topic, payload);
}

void Mqtt_state_publish(const char *topic, String data)
{
   if (!mqtt_enabled_u8)
      return;

   uint16_t publish_id = amqtt_client.publish(topic, qospub, true, data.c_str());
   (void)publish_id;
   DEBUG_PRINTF("MQTT State Send [ID %i]: [%s]: %s\n", publish_id, topic, data.c_str());
}

void Mqtt_100ms_task()
{
   if (!mqtt_enabled_u8)
      return;

   mqtt_reconn_ticker.update();
}