#ifndef CLOCK_H_
#define CLOCK_H_

typedef enum
{
  CLOCK_STATE_START,
  CLOCK_STATE_IP,
  CLOCK_STATE_VALID,
  CLOCK_STATE_SERVER_DOWN,
  CLOCK_STATE_AP
} clock_states_t;

typedef enum
{
  LIGHT_MODE_AUTO,
  LIGHT_MODE_MANUAL,
  LIGHT_MODE_OFF
} light_modes_t;

typedef union {
  uint8 state;
  struct
  {
    clock_states_t clockState : 3;
    light_modes_t lightMode : 2;
    uint8 dummy : 3;
    uint8 lightBrightness; /**<1 byte for the requested brightness value, requested by the user, in percent.*/
    uint8 checkSum;              /**<Variable for calculating a checksum for each package */
  };
} esp_states_t;

typedef union {
  uint32_t timestamp;
  struct
  {
    uint8 timestamp_byte0;
    uint8 timestamp_byte1;
    uint8 timestamp_byte2;
    uint8 timestamp_byte3;
  };
} timestamp_t;

void Clock_init();
void Clock_task_1000ms();

void Clock_skip_ip();

esp_states_t Get_esp_states();
timestamp_t Get_timestamp();
String Get_light_mode_str();
String Get_clock_state_str();
String Get_time_formatted();
boolean Get_manual_mode();
String Get_ntp_server();
int8 Get_ntp_timezone();
void Set_manual_mode(boolean value);
void Set_clock_state(uint8 value);
void Set_lightMode(uint8 value);
void Set_lightBrightness(uint8 value);
void Set_ntp_server(const char *server);
void Set_timezone(int8 value);
void Set_timestamp(uint32 value);

#endif /* CLOCK_H_ */
