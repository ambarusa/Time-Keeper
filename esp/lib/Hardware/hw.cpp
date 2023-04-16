#include "Ticker.h"
#include "hw.h"
#include "memory.h"
#include "network.h"

#define RESET_SW_PIN 5 /**<The digital input pin used by the Reset Config Button.*/

#define RESET_TIMEOUT 250 /**< Used for the Reset Switch; It's used in a 20ms task, so 250 * 20ms = 5 sec */

#define SERIAL_SYNC_FRAME 0x55
#define SERIAL_MESSAGE_LENGTH 7

Ticker device_restart_ticker([]
                             { ESP.restart(); },
                             2000, 1);

boolean reset_button_u8 = HIGH; /**<Variable that contains the Reset Button's state */
boolean send_data_b;

void Restart_device(boolean soft_b)
{
    if (soft_b)
    {
        uint8_t restart_flg_u8 = 1;
        Memory_write((char *)&restart_flg_u8, EEPROM_RESTART_FLG_ADDR, sizeof(restart_flg_u8));
    }
    Disable_WifiDisconnectHandler();
    DEBUG_PRINTLN("\nHardware: The device will now restart...\n");
    device_restart_ticker.start();
}

boolean Get_reset_button_state()
{
    return !reset_button_u8;
}

String Get_reset_button_state_str()
{
    return (reset_button_u8 == LOW) ? "ON" : "OFF";
}

void Read_inputs()
{
    static uint8_t counter_u8 = RESET_TIMEOUT;
    static uint8_t state_old_b = HIGH;

    bool state_current_b = digitalRead(RESET_SW_PIN);
    if (counter_u8 > 0)
    {
        if (state_current_b == state_old_b)
            counter_u8--;
        else
        {
            state_old_b = state_current_b;
        }
    }
    else
    {
        counter_u8 = RESET_TIMEOUT;
        if (reset_button_u8 == state_current_b)
            return;
        reset_button_u8 = state_current_b;
        DEBUG_PRINTF("\nHardware: RESET Switch was turned %s\n", Get_reset_button_state_str().c_str());
    }
}

void Serial_send_message()
{
    static uint8_t checksum_u8, checksum_old_u8, bytes_sent_u8;

    if (!send_data_b && !bytes_sent_u8)
        return;

    esp_states_t states_u24 = Get_esp_states();
    timestamp_t timestamp_u32 = Get_timestamp();
    checksum_u8 = states_u24.lightBrightness + states_u24.state +
                  timestamp_u32.timestamp_byte3 + timestamp_u32.timestamp_byte2 +
                  timestamp_u32.timestamp_byte1 + timestamp_u32.timestamp_byte0;

    if (checksum_u8 == checksum_old_u8)
        return;

    switch (bytes_sent_u8)
    {
    case 0:
        Serial.write(SERIAL_SYNC_FRAME);
        break;
    case 1:
        Serial.write(checksum_u8);
        break;
    case 2:
        Serial.write(states_u24.lightBrightness);
        break;
    case 3:
        Serial.write(states_u24.state);
        break;
    case 4:
        Serial.write(timestamp_u32.timestamp_byte3);
        break;
    case 5:
        Serial.write(timestamp_u32.timestamp_byte2);
        break;
    case 6:
        Serial.write(timestamp_u32.timestamp_byte1);
        break;
    case 7:
        Serial.write(timestamp_u32.timestamp_byte0);
        break;
    default:
        break;
    }
    bytes_sent_u8++;
    if (bytes_sent_u8 > SERIAL_MESSAGE_LENGTH)
    {
#if defined(DEBUG)
        Serial.print("\n| Mode: ");
        Serial.print(Get_light_mode_str());
        Serial.print("| Intensity: ");
        Serial.print(states_u24.lightBrightness);
        Serial.print("| Clock State: ");
        Serial.print(Get_clock_state_str());
        Serial.print("| Timestamp: ");
        Serial.print(timestamp_u32.timestamp);
        Serial.println(" |");
#endif
        bytes_sent_u8 = 0;
        send_data_b = false;
        checksum_old_u8 = checksum_u8;
    }
}

void Hardware_init()
{
    pinMode(RESET_SW_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

    uint8_t restart_flg_u8 = 0;
    Memory_read((char *)&restart_flg_u8, EEPROM_RESTART_FLG_ADDR, sizeof(uint8_t));

    if (restart_flg_u8)
    {
#if defined(DEBUG)
        Serial.println("\n\nHardware: Restoring states after the restart\n");
#endif
        Clock_skip_ip();
        Memory_write((char *)&(restart_flg_u8 = 0), EEPROM_RESTART_FLG_ADDR, sizeof(restart_flg_u8));
    }

    send_data_b = true;
}

void Hardware_20ms_task()
{
    Read_inputs();

    device_restart_ticker.update();

    if (Get_reset_button_state())
    {
        reset_button_u8 = !reset_button_u8;
        Memory_reset();
        Network_reset();
        Restart_device(true);
    }

    Serial_send_message();
}

/**
 * @brief Hardware's cyclic task @ 1000ms
 *
 * The function blinks the Built-in LED, to see if the software is not blocked anywhere.
 *
 */
void Hardware_1000ms_task()
{
    send_data_b = true;

    static boolean ledState;
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
}