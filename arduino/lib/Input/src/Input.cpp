#include "Input.h"
#include "Definitions.h"
#include "Output.h"

#define LIGHT_SENSOR_INPUT A0 /**<Analog input used by the LM393 sensor.*/
#define PWM_MAX_VALUE 255     /**<Maximum PWM value.*/

#define SERIAL_SYNC_FRAME 0x55 /**<First received byte to synchronize input data.*/
#define SERIAL_MESSAGE_LENGTH 7

main_buffer_t mainBuffer;

/**
 * @brief Init function of the LM393 light sensor.
 *
 * The function sets the LIGHT_SENSOR_INPUT (A0) pin to input.
 * @return void
 */
void LightSensorInit()
{
    pinMode(LIGHT_SENSOR_INPUT, INPUT);
}

/**
 * @brief Implementation of function, that processes the incoming timestamp.
 *
 * The function gets the timestamp from the serialBuffer,
 * extracts the hour, minute and second values from it,
 * and actualizes the mainBuffer.
 * The function sets the clock-related state variables in the mainBuffer as well.
 * @return void
 */
void setTime(uint32_t timestamp_u32)
{
    uint32_t hour_lu32;
    uint32_t minute_lu32;
    uint32_t second_lu32;

    switch (mainBuffer.esp_states.clockState)
    {
    case CLOCK_STATE_START:
        break;
    case CLOCK_STATE_IP:
        mainBuffer.hour_ten = (timestamp_u32 / 100000) % 10;
        mainBuffer.hour_unit = (timestamp_u32 / 10000) % 10;
        mainBuffer.minute_ten = (timestamp_u32 / 1000) % 10;
        mainBuffer.minute_unit = (timestamp_u32 / 100) % 10;
        mainBuffer.second_ten = (timestamp_u32 / 10) % 10;
        mainBuffer.second_unit = timestamp_u32 % 10;
        mainBuffer.timestamp = 0;
        break;
    case CLOCK_STATE_AP:
    case CLOCK_STATE_SERVER_DOWN:
    case CLOCK_STATE_VALID:
        hour_lu32 = (timestamp_u32 / 3600) % 24;
        minute_lu32 = (timestamp_u32 / 60) % 60;
        second_lu32 = timestamp_u32 % 60;
        mainBuffer.hour_ten = (uint8_t)(hour_lu32 / 10);
        mainBuffer.hour_unit = (uint8_t)(hour_lu32 % 10);
        mainBuffer.minute_ten = (uint8_t)(minute_lu32 / 10);
        mainBuffer.minute_unit = (uint8_t)(minute_lu32 % 10);
        mainBuffer.second_ten = (uint8_t)(second_lu32 / 10);
        mainBuffer.second_unit = (uint8_t)(second_lu32 % 10);
        mainBuffer.timestamp = timestamp_u32;
        break;
    default:
        break;
    }
}

/**
 * @brief Implementation of function that reads the LM393 light sensor value.
 *
 * The function reads the analog value, from the LIGHT_SENSOR_INPUT (A0) pin.
 * @return void
 */
uint8_t Read_light_sensor()
{
    uint16_t value_u16 = analogRead(LIGHT_SENSOR_INPUT);
    value_u16 = value_u16 >> 2;
    return (uint8_t)value_u16;
}

/**
 * @brief Implementation of function that actualizes the brightness value.
 *
 * Depending on the brightness mode state variable, demands a value from the
 * light sensor, or uses the brightness value requested by the user.
 * @return void
 */
void Set_brightness(uint8_t brightnessPercent_lu8)
{
    if (mainBuffer.esp_states.lightMode == LIGHT_MODE_OFF)
        mainBuffer.brightness = BRIGHTNESS_ZERO;
#if defined(FLEURIE)
    else if (mainBuffer.esp_states.lightMode == LIGHT_MODE_AUTO)
    {
        mainBuffer.brightness = Read_light_sensor();
        mainBuffer.brightness = (mainBuffer.brightness < BRIGHTNESS_MIN) ? BRIGHTNESS_MIN : mainBuffer.brightness;
    }
#endif
    else
    {
        uint16_t brightness_lu16 = 1 - (brightnessPercent_lu8 * PWM_MAX_VALUE) / 100;
        mainBuffer.brightness = brightness_lu16;
    }
}

/**
 * @brief Implementation of function that reads from the serial port.
 *
 * The function decides if the incoming data is valid, by calculating, and
 * comparing a checksum value. If the incoming data is valid, sets the
 * serialBuffer properly.
 * @return void
 */
void InputCyclic()
{
    static serial_buffer_t serialBuffer;
    static uint8_t temp_u8;
    static uint8_t checksum_calc_u8;
    static uint8_t bytes_received_u8;

    if (Serial.available())
    {
        temp_u8 = Serial.read();
        switch (bytes_received_u8)
        {
        case 0:
            if (temp_u8 == SERIAL_SYNC_FRAME)
                break;
            return;
        case 1:
            serialBuffer.esp_states.checkSum = temp_u8;
            break;
        case 2:
            serialBuffer.esp_states.lightBrightness = temp_u8;
            checksum_calc_u8 += temp_u8;
            break;
        case 3:
            serialBuffer.esp_states.state = temp_u8;
            checksum_calc_u8 += temp_u8;
            break;
        case 4:
            serialBuffer.timestamp.timestamp_byte3 = temp_u8;
            checksum_calc_u8 += temp_u8;
            break;
        case 5:
            serialBuffer.timestamp.timestamp_byte2 = temp_u8;
            checksum_calc_u8 += temp_u8;
            break;
        case 6:
            serialBuffer.timestamp.timestamp_byte1 = temp_u8;
            checksum_calc_u8 += temp_u8;
            break;
        case 7:
            serialBuffer.timestamp.timestamp_byte0 = temp_u8;
            checksum_calc_u8 += temp_u8;
            break;
        default:
            break;
        }
        bytes_received_u8++;
        if (bytes_received_u8 > SERIAL_MESSAGE_LENGTH)
        {
            if (checksum_calc_u8 == serialBuffer.esp_states.checkSum)
            {
                mainBuffer.esp_states.state = serialBuffer.esp_states.state;
                setTime(serialBuffer.timestamp.timestamp);
            }
            bytes_received_u8 = 0;
            checksum_calc_u8 = 0;
            Serial.flush();
        }
    }
    Set_brightness(serialBuffer.esp_states.lightBrightness);
}