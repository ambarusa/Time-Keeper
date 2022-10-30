#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "NTPClient.h"
#include "memory.h"
#include "network.h"

#define HOUR_IN_SEC 3600 /**<Hour in seconds. */
#define TASK_15_MIN_TIMEOUT 900
#define TASK_30_MIN_TIMEOUT 1800
#define TASK_06_SEC_TIMEOUT 6

timestamp_t timestamp_u32;
char ntp_server[EEPROM_NTP_SERVER_SIZE];
int8 timezone_s8;

WiFiUDP ntp_udp;
NTPClient ntp_client(ntp_udp);

boolean manual_mode_b, force_sync_b, skip_ip_b;
esp_states_t esp_states_u24;

void Clock_skip_ip()
{
    skip_ip_b = true;
#ifdef DEBUG
    Serial.printf("Clock: IP displaying will be skipped\n");
#endif
}

uint32_t Get_IPAddress_fragment()
{
    static uint8 idx_u8;
    idx_u8 %= 4;

    String fragment;
    IPAddress ip = WiFi.localIP();

    for (int i = 0; i < 2; i++)
    {
        String temp = String(ip[idx_u8]);
        while (temp.length() < 3)
            temp = "0" + temp;
        fragment += temp;
        idx_u8++;
    }
    return fragment.toInt();
}
esp_states_t Get_esp_states()
{
    return esp_states_u24;
}
String Get_ntp_server()
{
    return String(ntp_server);
}
int8 Get_ntp_timezone()
{
    return timezone_s8;
}
timestamp_t Get_timestamp()
{
    return timestamp_u32;
}
String Get_light_mode_str()
{
    switch (esp_states_u24.lightMode)
    {
    case LIGHT_MODE_AUTO:
        return "Automatic";
    case LIGHT_MODE_MANUAL:
        return "Manual";
    case LIGHT_MODE_OFF:
        return "Off";
    }
    return "";
}
String Get_clock_state_str()
{
    switch (esp_states_u24.clockState)
    {
    case CLOCK_STATE_START:
        return "Start";
    case CLOCK_STATE_IP:
        return "IP";
    case CLOCK_STATE_AP:
        return "AP";
    case CLOCK_STATE_SERVER_DOWN:
        return "Server Down";
    case CLOCK_STATE_VALID:
        return "Valid";
    }
    return "";
}
String Get_time_formatted()
{
    if (esp_states_u24.clockState < CLOCK_STATE_VALID)
        return "Not set yet";
    uint8 h = (timestamp_u32.timestamp / 3600) % 24;
    uint8 m = (timestamp_u32.timestamp / 60) % 60;
    uint8 s = timestamp_u32.timestamp % 60;
    char result[9];
    sprintf(result, "%02d:%02d:%02d", h, m, s);
    return String(result);
}
boolean Get_manual_mode()
{
    return manual_mode_b;
}

void Set_manual_mode(boolean value)
{
    if (manual_mode_b == value)
        return;

    manual_mode_b = value;
#ifdef DEBUG
    Serial.printf("Clock: Manual Mode was set to %s\n", (value == 1) ? "ON" : "OFF");
#endif
    Memory_write((char *)&value, EEPROM_MANUAL_MODE_ADDR, sizeof(value));
}
void Set_lightMode(uint8 value)
{
    if (esp_states_u24.lightMode == value)
        return;

    esp_states_u24.lightMode = light_modes_t(value);
#ifdef DEBUG
    Serial.printf("Clock: Light Mode set to %s\n", Get_light_mode_str().c_str());
#endif
    Mqtt_state_publish(mqtt_topic, (value == LIGHT_MODE_OFF) ? "OFF" : "ON");
#if defined(FLEURIE)
    if (value != LIGHT_MODE_OFF)
        Mqtt_state_publish(mqtt_effect_topic, Get_light_mode_str());
#endif
    Notify_ws_clients("LIGHTMODE", Get_light_mode_str());
    // Memory_write((char *)&value, EEPROM_LIGHT_MODE_ADDR, sizeof(uint8));
}
void Set_lightBrightness(uint8 value)
{
    if (esp_states_u24.lightBrightness == value)
        return;

    esp_states_u24.lightBrightness = value % 101;
#ifdef DEBUG
    Serial.printf("Clock: Brightness set to %i\n", value);
#endif

#if defined(FLEURIE)
    Mqtt_state_publish(mqtt_brightness_topic, String(value));
#endif
    Notify_ws_clients("BRIGHTNESS", String(esp_states_u24.lightBrightness));
    // Memory_write((char *)&intensity, EEPROM_BRIGHTNESS_PCT_ADDR, sizeof(uint8));
}
void Set_clock_state(uint8 value)
{
    if (esp_states_u24.clockState == (clock_states_t)value)
        return;

    esp_states_u24.clockState = (clock_states_t)value;
#ifdef DEBUG
    Serial.printf("Clock: Clock State was set to %s\n", Get_clock_state_str().c_str());
#endif
    switch (esp_states_u24.clockState)
    {
    case CLOCK_STATE_IP:
        timestamp_u32.timestamp = Get_IPAddress_fragment();
        if (!manual_mode_b)
        {
            ntp_client.setPoolServerName(ntp_server);
            ntp_client.setUpdateInterval(TASK_30_MIN_TIMEOUT);
            ntp_client.begin();
#ifdef DEBUG
            Serial.printf("Clock: Starting NTP Client\n");
#endif
        }
        break;
    default:
        break;
    }
}
void Set_ntp_server(const char *server)
{
    strcpy(ntp_server, server);
    Memory_write(ntp_server, EEPROM_NTP_SERVER_ADDR, EEPROM_NTP_SERVER_SIZE);

    if (!ntp_client.forceUpdate())
        ntp_client.begin();
    force_sync_b = true;
}
void Set_timezone(int8 value)
{
    if (timezone_s8 == value)
        return;

    int8 offset = abs(timezone_s8 - value) * ((value > timezone_s8) ? 1 : -1);
    timestamp_u32.timestamp += offset * HOUR_IN_SEC;
    timezone_s8 = value;
    Memory_write((char *)&timezone_s8, EEPROM_TIMEZONE_ADDRESS, sizeof(timezone_s8));
}
void Set_timestamp(uint32 value)
{
    Set_clock_state(CLOCK_STATE_VALID);
    timestamp_u32.timestamp = value + timezone_s8 * HOUR_IN_SEC;
}

void Clock_init()
{
    uint8 temp_u8;
    Memory_read((char *)&manual_mode_b, EEPROM_MANUAL_MODE_ADDR, sizeof(uint8));
    Memory_read((char *)&temp_u8, EEPROM_LIGHT_MODE_ADDR, sizeof(uint8));
    Memory_read((char *)&timezone_s8, EEPROM_TIMEZONE_ADDRESS, sizeof(uint8));
    Memory_read((char *)ntp_server, EEPROM_NTP_SERVER_ADDR, EEPROM_NTP_SERVER_SIZE);
    esp_states_u24.lightMode = (light_modes_t)temp_u8;
    esp_states_u24.lightBrightness = 100;

    esp_states_u24.clockState = CLOCK_STATE_START;
}

void Clock_task_1000ms()
{
    ntp_client.update();

    switch (esp_states_u24.clockState)
    {
    case CLOCK_STATE_START:
        timestamp_u32.timestamp = 0;
    case CLOCK_STATE_IP:
        static uint8 ip_state_timeout_u8 = (!skip_ip_b) ? TASK_06_SEC_TIMEOUT * 2 - 1 : 0;
        if (ip_state_timeout_u8 == TASK_06_SEC_TIMEOUT)
            timestamp_u32.timestamp = Get_IPAddress_fragment();
        else if (!ip_state_timeout_u8)
        {
            char brightness;
            Memory_read(&brightness, EEPROM_BRIGHTNESS_PCT_ADDR, sizeof(brightness));

            Set_clock_state((manual_mode_b || ntp_client.isTimeSet()) ? CLOCK_STATE_VALID : CLOCK_STATE_SERVER_DOWN);
            Set_lightBrightness(brightness);

            timestamp_u32.timestamp = timezone_s8 * HOUR_IN_SEC;
            if (!manual_mode_b && ntp_client.isTimeSet())
                timestamp_u32.timestamp += ntp_client.getEpochTime();
            else
                force_sync_b = true;
            ip_state_timeout_u8 = TASK_06_SEC_TIMEOUT * 2;
        }
        ip_state_timeout_u8--;
        break;
    case CLOCK_STATE_SERVER_DOWN:
    case CLOCK_STATE_VALID:
    case CLOCK_STATE_AP:
        static uint16 sync_timeout_u16 = TASK_15_MIN_TIMEOUT;
        if (!manual_mode_b && (!sync_timeout_u16 || force_sync_b))
        {
            if (ntp_client.isTimeSet())
            {
                Set_clock_state(CLOCK_STATE_VALID);
                timestamp_u32.timestamp = ntp_client.getEpochTime() + timezone_s8 * HOUR_IN_SEC;
                sync_timeout_u16 = TASK_15_MIN_TIMEOUT;
                force_sync_b = false;
            }
            else
              Set_clock_state(CLOCK_STATE_SERVER_DOWN);  
        }
        if (sync_timeout_u16)
            sync_timeout_u16--;

        timestamp_u32.timestamp++;
        Notify_ws_clients("TIME", Get_time_formatted());
        break;
    default:
        break;
    }

    esp_states_u24.checkSum = timestamp_u32.timestamp_byte3 + timestamp_u32.timestamp_byte2;
    esp_states_u24.checkSum += timestamp_u32.timestamp_byte1 + timestamp_u32.timestamp_byte0;
    esp_states_u24.checkSum += esp_states_u24.state + esp_states_u24.lightBrightness;
}
