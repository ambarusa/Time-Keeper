#ifndef CUSTOMTYPES_H_
#define CUSTOMTYPES_H_

#include <Arduino.h>

#if (defined(SIX_DIGITS) && defined(FOUR_DIGITS)) || (!defined(SIX_DIGITS) && !defined(FOUR_DIGITS))
#error "Please define either SIX_DIGITS or FOUR_DIGITS"
#endif

#if (defined(FLEURIE) && defined(PIXIE)) || (!defined(FLEURIE) && !defined(PIXIE))
#error "Please define either FLEURIE or PIXIE"
#endif

#define BRIGHTNESS_MAX  0   /**< Value for the maximum light intensity */
#define BRIGHTNESS_MIN  240 /**< Value for the minimum light intensity */
#define BRIGHTNESS_ZERO 255 /**< Value for the Off light */

typedef enum
{
    LIGHT_MODE_MANUAL,
    LIGHT_MODE_AUTO,
    LIGHT_MODE_OFF
} light_modes_t;

typedef enum
{
    CLOCK_STATE_START,
    CLOCK_STATE_IP,
    CLOCK_STATE_VALID,
    CLOCK_STATE_SERVER_DOWN,
    CLOCK_STATE_AP
} clock_states_t;

typedef union {
  uint32_t timestamp;
  struct
  {
    uint8_t timestamp_byte0;
    uint8_t timestamp_byte1;
    uint8_t timestamp_byte2;
    uint8_t timestamp_byte3;
  };
} timestamp_t;

typedef union {
  uint8_t state;
  struct
  {
    clock_states_t clockState : 3;
    light_modes_t lightMode : 2;
    uint8_t dummy : 3;
    uint8_t lightBrightness;
  };
} esp_states_t;

/**
 * @brief Datatype of the main global buffer.
 *
 * Enum of the LM393 light sensor's states.
 */
typedef enum
{
    LIGHT_SENSOR_OFF,
    LIGHT_SENSOR_ON,
    LIGHT_SENSOR_ERROR
} light_sensor_t;

/**
 * @brief Datatype of the main global buffer.
 *
 */
typedef struct
{
    uint8_t hour_ten;
    uint8_t hour_unit;
    uint8_t minute_ten;
    uint8_t minute_unit;
    uint8_t second_ten;
    uint8_t second_unit;
    uint8_t brightness;
    uint32_t timestamp;
    esp_states_t esp_states;
} main_buffer_t;

extern main_buffer_t mainBuffer; /**< Main global buffer. */

#if defined(FLEURIE)
/**
 * @brief Define structure for digit buffer.
 *
 * Make package to sent at display.
 */
typedef union
{
    struct
    {
        uint8_t byteH2; // For the Hours modul byte High
        uint8_t byteM2; //                     byte Middle
        uint8_t byteL2; //                     byte Low
        uint8_t byteH1; // For the Minutes modul byte High
        uint8_t byteM1; //                       byte Middle
        uint8_t byteL1; //                       byte Low
        uint8_t byteH0; // For the Seconds modul byte High
        uint8_t byteM0; //                       byte Middle
        uint8_t byteL0; //                       byte Low
    };
    struct
    {
        /* Make package for Hours modul*/
        uint8_t header2 : 4;
        uint8_t ledHU : 1;
        uint8_t hour_unit_digit_st_ui8 : 8;
        uint8_t G_hour_unit : 1;
        uint8_t ledHT : 1;
        uint8_t hour_ten_digit_st_ui8 : 8;
        uint8_t G_hour_ten : 1;
        /* Make package for Minutes modul*/
        uint8_t header1 : 4;
        uint8_t ledMU : 1;
        uint8_t minute_unit_digit_st_ui8 : 8;
        uint8_t G_minute_unit : 1;
        uint8_t ledMT : 1;
        uint8_t minute_ten_digit_st_ui8 : 8;
        uint8_t G_minute_ten : 1;
        /*Make package for Seconds modul*/
        uint8_t header0 : 4;
        uint8_t ledSU : 1;
        uint8_t second_unit_digit_st_ui8 : 8;
        uint8_t G_second_unit : 1;
        uint8_t ledST : 1;
        uint8_t second_ten_digit_st_ui8 : 8;
        uint8_t G_second_ten : 1;
    };
} PACKED_DISPLAY_DATA;

/**
 * @brief Enumeration of logical outputs
 *
 * Enumeration of logical outputs.
 */
typedef enum
{
    EN_SOD_VFDOUT = 6, /**< Data digital output */
    EN_SOD_VFCLK,      /**< Clock digital output */
    EN_SOD_VFLOAD,     /**< Load digital output */
    EN_SOD_VFBLANK     /**< Blankig digital output 4 */
} EN_OUTPUT_PINS;

typedef enum
{
    EN_START_BIT_SEND_4 = 4, /**< Start send from bit 4 to 7 */
    EN_START_BIT_SEND_7 = 7  /**< Start send from bit 0 to 7 */
} EN_START_BIT_SEND;

/**
 * @brief Implementation table for segment of digit
 *
 */
const uint8_t digit_seg_ui8[] = {
    B11111100, // 0 (O)
    B01100000, // 1 (I)
    B11011010, // 2
    B11110010, // 3
    B01100110, // 4
    B10110110, // 5 (S)
    B10111110, // 6
    B11100000, // 7
    B11111110, // 8
    B11110110, // 9
    B00000000, // 10 (blank)
    B00011110, // 11 (t)
    B00101110, // 12 (h)
    B01111100, // 13 (d)
    B01101110, // 14 (H)
    B10011110, // 15 (E)
    B00011100, // 16 (L)
    B11001110, // 17 (P)
    B10011100, // 18 (C)
    B10001110, // 19 (F)
    B10111100, // 20 (G)
    B01100110, // 21 (Y)
    B11000110, // 22 (o grade)
    B00000010, // 23 (-)
    B01100001, // 24 (! with point)
    B00101010, // 25 (n)
    B00111010, // 26 (o little)
    B11101110, // 27 (A)
    B00000001  // 28 (.)
};

#elif defined(PIXIE)

typedef enum
{
#if defined(FOUR_DIGITS)
    EN_SHDN = 4,
    EN_COLON = 6,
    EN_MINUTE_DATA = 8,
#elif defined(SIX_DIGITS)
    EN_SECOND_DATA = 2,
    EN_SECOND_CLOCK,
    EN_SECOND_LATCH,
    EN_MINUTE_DATA,
#endif
    EN_MINUTE_CLOCK,
    EN_MINUTE_LATCH,
    EN_HOUR_DATA,
    EN_HOUR_CLOCK,
    EN_HOUR_LATCH
} EN_OUTPUT_PINS;

/**
 * @brief Define structure for digit buffer.
 *
 * Make package to sent at display.
 */
typedef union
{
    struct
    {
        uint8_t hour_data;
        uint8_t minute_data;
        uint8_t second_data;
    };
    struct
    {
        uint8_t hour_data_ten : 4;
        uint8_t hour_data_unit : 4;
        uint8_t minute_data_ten : 4;
        uint8_t minute_data_unit : 4;
        uint8_t second_data_ten : 4;
        uint8_t second_data_unit : 4;
    };
} DISPLAY_DATA;

#endif

#endif /*CUSTOMTYPES_H_*/
