#ifndef CLOCK_H_
#define CLOCK_H_

typedef enum
{
    CLOCK_STATE_START,
    CLOCK_STATE_IP,
    CLOCK_STATE_VALID,
    CLOCK_STATE_SERVER_DOWN,
    CLOCK_STATE_AP,
    CLOCK_STATE_OTA
} clock_states_t;

typedef enum
{
    LIGHT_MODE_MANUAL,
    LIGHT_MODE_AUTO,
    LIGHT_MODE_OFF
} light_modes_t;

typedef union
{
    uint8_t state;
    struct
    {
        clock_states_t clockState : 3;
        light_modes_t lightMode : 2;
        uint8_t dummy : 3;
        uint8_t lightBrightness;
    };
} esp_states_t;

typedef union
{
    uint32_t timestamp;
    struct
    {
        uint8_t timestamp_byte0;
        uint8_t timestamp_byte1;
        uint8_t timestamp_byte2;
        uint8_t timestamp_byte3;
    };
} timestamp_t;

void Clock_init();
void Clock_task_1000ms();

void Clock_skip_ip();

esp_states_t Get_esp_states();
timestamp_t Get_timestamp();
String Get_light_mode_str();
String Get_clock_state_str();
boolean Get_manual_mode();
String Get_ntp_server();
int8 Get_ntp_timezone();
void Set_manual_mode(boolean value);
void Set_clock_state(uint8_t value);
void Set_lightMode(uint8_t value);
void Set_lightBrightness(uint8_t value);
void Set_ntp_server(String server);
void Set_timezone(int8 value);
void Set_timestamp(uint8 state, uint32 value);

#endif /* CLOCK_H_ */
