#ifndef MEMORY_H
#define MEMORY_H

#define EEPROM_CHIPID_ADDR 0
#define EEPROM_CHIPID_SIZE 4

#define EEPROM_RESTART_FLG_ADDR 8

#define EEPROM_MANUAL_MODE_ADDR 9

#define EEPROM_ESP_STATE_ADDR 10

#define EEPROM_BRIGHTNESS_PCT_ADDR 11

#define EEPROM_TIMEZONE_ADDRESS 12

#define EEPROM_NTP_SERVER_ADDR 13
#define EEPROM_NTP_SERVER_SIZE 32

#define EEPROM_MQTT_ENABLED_ADDR 49

#define EEPROM_MQTT_HOST_ADDR 50
#define EEPROM_MQTT_HOST_SIZE 32

#define EEPROM_MQTT_PORT_ADDR 82
#define EEPROM_MQTT_PORT_SIZE 2

#define EEPROM_MQTT_CLIENTID_ADDR 84
#define EEPROM_MQTT_CLIENTID_SIZE 32

#define EEPROM_MQTT_USER_ADDR 116
#define EEPROM_MQTT_USER_SIZE 32

#define EEPROM_MQTT_PWD_ADDR 148
#define EEPROM_MQTT_PWD_SIZE 32

#define EEPROM_MQTT_QOSSUB_ADDR 180
#define EEPROM_MQTT_QOSPUB_ADDR 181

#define EEPROM_TIMESTAMP_ADDR 182
#define EEPROM_TIMESTAMP_SIZE 4

void Memory_init();

void Memory_reset();

void Memory_read(char *target, uint8_t address, uint8_t size_u8);

void Memory_write(char *data, uint8_t address, uint8_t size_u8);

#endif