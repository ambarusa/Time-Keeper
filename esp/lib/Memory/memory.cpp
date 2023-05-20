#include <EEPROM.h>
#include <Arduino.h>
#include "hw.h"
#include "network.h"
#include "memory.h"

void Memory_init()
{
    EEPROM.begin(4096);
    boolean init_needed_b = false;
    uint32_t chipid_u32 = ESP.getChipId();

    char cipid_stored_u32[EEPROM_CHIPID_SIZE];
    Memory_read((char *)cipid_stored_u32, EEPROM_CHIPID_ADDR, EEPROM_CHIPID_SIZE);

    for (int i = 0; i < EEPROM_CHIPID_SIZE; i++)
    {
        uint8_t chipid_byte_u8 = (chipid_u32 >> (8 * i)) & 0xFF;
        if (cipid_stored_u32[i] != chipid_byte_u8)
        {
            cipid_stored_u32[i] = chipid_byte_u8;
            init_needed_b = true;
        }
    }
    if (init_needed_b)
    {
        Memory_write((char *)cipid_stored_u32, EEPROM_CHIPID_ADDR, EEPROM_CHIPID_SIZE);
        char buffer[32];
        uint8_t buffer_u8;

        Memory_write((char *)&(buffer_u8 = 0), EEPROM_RESTART_FLG_ADDR, sizeof(buffer_u8));
        Memory_write((char *)&(buffer_u8 = 1), EEPROM_MANUAL_MODE_ADDR, sizeof(buffer_u8));
        Memory_write((char *)&(buffer_u8 = 0), EEPROM_ESP_STATE_ADDR, sizeof(buffer_u8));
        Memory_write((char *)&(buffer_u8 = 85), EEPROM_BRIGHTNESS_PCT_ADDR, sizeof(buffer_u8));
        Memory_write((char *)&(buffer_u8 = 0), EEPROM_TIMEZONE_ADDRESS, sizeof(buffer_u8));
        strcpy(buffer, "0.europe.pool.ntp.org");
        Memory_write(buffer, EEPROM_NTP_SERVER_ADDR, EEPROM_NTP_SERVER_SIZE);
        Memory_write((char *)&(buffer_u8 = 0), EEPROM_MQTT_ENABLED_ADDR, sizeof(buffer_u8));
        Memory_write((char *)&(buffer_u8 = uint8_t(1883)), EEPROM_MQTT_PORT_ADDR, sizeof(buffer_u8));
        Memory_write((char *)&(buffer_u8 = 1883 >> 8), EEPROM_MQTT_PORT_ADDR + 1, sizeof(buffer_u8));
        strcpy(buffer, DEVICE_NAME);
        Memory_write(buffer, EEPROM_MQTT_CLIENTID_ADDR, EEPROM_MQTT_CLIENTID_SIZE);
        strcpy(buffer, "");
        Memory_write(buffer, EEPROM_MQTT_HOST_ADDR, EEPROM_MQTT_HOST_SIZE);
        Memory_write(buffer, EEPROM_MQTT_USER_ADDR, EEPROM_MQTT_USER_SIZE);
        Memory_write(buffer, EEPROM_MQTT_PWD_ADDR, EEPROM_MQTT_PWD_SIZE);
        Memory_write((char *)&(buffer_u8 = 1), EEPROM_MQTT_QOSSUB_ADDR, sizeof(buffer_u8));
        Memory_write((char *)&(buffer_u8 = 1), EEPROM_MQTT_QOSPUB_ADDR, sizeof(buffer_u8));
        for (int i = 0; i < EEPROM_TIMESTAMP_SIZE; i++)
            Memory_write((char *)&(buffer_u8 = 0), EEPROM_TIMESTAMP_ADDR + i, sizeof(buffer_u8));
        DEBUG_PRINTLN("\nMemory: Memory wasn't initialized. Initalizing now.\n");
    }
}

void Memory_reset()
{
    char chipid_empty_u32[EEPROM_CHIPID_SIZE] = {0};
    Memory_write((char *)chipid_empty_u32, EEPROM_CHIPID_ADDR, EEPROM_CHIPID_SIZE);
}

void Memory_read(char *target, uint8_t address, uint8_t size_u8)
{
    for (uint8_t counter_u8 = 0; counter_u8 < size_u8; counter_u8++)
    {
        *(target + counter_u8) = EEPROM.read(address + counter_u8);
    }
}

void Memory_write(char *data, uint8_t address, uint8_t size_u8)
{
    for (uint8_t counter_u8 = 0; counter_u8 < size_u8; counter_u8++)
    {
        char buffer;
        Memory_read((char *)&buffer, (address + counter_u8), sizeof(buffer));
        if (buffer != *(data + counter_u8))
            EEPROM.write((address + counter_u8), *(data + counter_u8));
    }
    EEPROM.commit();
}