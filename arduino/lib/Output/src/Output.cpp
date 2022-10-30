#include "Output.h"

#define MESSAGE_DISPLAY_01_SEC_TIMEOUT 20
#define MESSAGE_DISPLAY_03_SEC_TIMEOUT 60
#define MESSAGE_DISPLAY_15_SEC_TIMEOUT 300
#define MESSAGE_DISPLAY_01_MIN_TIMEOUT 1200
#define TASK_06_SEC_TIMEOUT 120

#if defined(FLEURIE)
PACKED_DISPLAY_DATA PackedDisplayData;

/**
 * @brief Flag for mask blink display
 * 0 = Display On
 * 1 = Display Off
 */
static uint8_t sb_maskBlink = 0;

/**
 * @brief Flag for blink state
 * 1 = Blink On
 */
static bool b_flag_Blink = 0;

void DisplayInit()
{
	pinMode(EN_SOD_VFDOUT, OUTPUT);
	pinMode(EN_SOD_VFCLK, OUTPUT);
	pinMode(EN_SOD_VFLOAD, OUTPUT);
	pinMode(EN_SOD_VFBLANK, OUTPUT);
	mainBuffer.brightness = BRIGHTNESS_ZERO;
}

/**
 * @brief Function implementation for processing output buffer
 *
 * Function pack data for processing output buffer @ 50ms
 */
void ClockPacked()
{
	static uint16_t MessageDisplay01SecTimeout_u16;
	static uint16_t MessageDisplay01MinTimeout_u16;
	switch (mainBuffer.esp_states.clockState)
	{
	case CLOCK_STATE_START: //    HELLO! message
	{
		PackedDisplayData.hour_ten_digit_st_ui8 = digit_seg_ui8[14];	// H
		PackedDisplayData.hour_unit_digit_st_ui8 = digit_seg_ui8[15];	// E
		PackedDisplayData.minute_ten_digit_st_ui8 = digit_seg_ui8[16];	// L
		PackedDisplayData.minute_unit_digit_st_ui8 = digit_seg_ui8[16]; // L
		PackedDisplayData.second_ten_digit_st_ui8 = digit_seg_ui8[0];	// O
		PackedDisplayData.second_unit_digit_st_ui8 = digit_seg_ui8[24]; // !
		mainBuffer.esp_states.lightMode = LIGHT_MODE_MANUAL;
		mainBuffer.brightness = BRIGHTNESS_MAX;
		b_flag_Blink = 0;
		break;
	}
	case CLOCK_STATE_AP: //  nEtoFF message
	{
		MessageDisplay01MinTimeout_u16++;
		if (MessageDisplay01MinTimeout_u16 >= MESSAGE_DISPLAY_01_MIN_TIMEOUT)
		{
			MessageDisplay01MinTimeout_u16 = 0;
			MessageDisplay01SecTimeout_u16 = 0;
		}
		if (MessageDisplay01SecTimeout_u16 < MESSAGE_DISPLAY_01_SEC_TIMEOUT)
		{
			PackedDisplayData.hour_ten_digit_st_ui8 = digit_seg_ui8[25];	// n
			PackedDisplayData.hour_unit_digit_st_ui8 = digit_seg_ui8[15];	// E
			PackedDisplayData.minute_ten_digit_st_ui8 = digit_seg_ui8[11];	// t
			PackedDisplayData.minute_unit_digit_st_ui8 = digit_seg_ui8[26]; // o
			PackedDisplayData.second_ten_digit_st_ui8 = digit_seg_ui8[19];	// F
			PackedDisplayData.second_unit_digit_st_ui8 = digit_seg_ui8[19]; // F
			MessageDisplay01SecTimeout_u16++;
		}
		else
		{
			PackedDisplayData.hour_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.hour_ten];
			PackedDisplayData.hour_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.hour_unit];
			PackedDisplayData.minute_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.minute_ten];
			PackedDisplayData.minute_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.minute_unit];
			PackedDisplayData.second_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.second_ten];
			PackedDisplayData.second_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.second_unit];
		}
		b_flag_Blink = 0;
		break;
	};
	case CLOCK_STATE_SERVER_DOWN: //  ConFIG message
	{
		MessageDisplay01MinTimeout_u16++;
		if (MessageDisplay01MinTimeout_u16 >= MESSAGE_DISPLAY_01_MIN_TIMEOUT)
		{
			MessageDisplay01MinTimeout_u16 = 0;
			MessageDisplay01SecTimeout_u16 = 0;
		}
		if (MessageDisplay01SecTimeout_u16 < MESSAGE_DISPLAY_01_SEC_TIMEOUT)
		{
			PackedDisplayData.hour_ten_digit_st_ui8 = digit_seg_ui8[18];	// C
			PackedDisplayData.hour_unit_digit_st_ui8 = digit_seg_ui8[26];	// o
			PackedDisplayData.minute_ten_digit_st_ui8 = digit_seg_ui8[25];	// n
			PackedDisplayData.minute_unit_digit_st_ui8 = digit_seg_ui8[19]; // F
			PackedDisplayData.second_ten_digit_st_ui8 = digit_seg_ui8[1];	// I
			PackedDisplayData.second_unit_digit_st_ui8 = digit_seg_ui8[20]; // G
			MessageDisplay01SecTimeout_u16++;
		}
		else
		{
			PackedDisplayData.hour_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.hour_ten];
			PackedDisplayData.hour_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.hour_unit];
			PackedDisplayData.minute_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.minute_ten];
			PackedDisplayData.minute_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.minute_unit];
			PackedDisplayData.second_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.second_ten];
			PackedDisplayData.second_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.second_unit];
		}
		b_flag_Blink = 0;
		break;
	};
	case CLOCK_STATE_VALID: // All is Ok.
	{
		PackedDisplayData.hour_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.hour_ten];
		PackedDisplayData.hour_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.hour_unit];
		PackedDisplayData.minute_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.minute_ten];
		PackedDisplayData.minute_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.minute_unit];
		PackedDisplayData.second_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.second_ten];
		PackedDisplayData.second_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.second_unit];
		MessageDisplay01MinTimeout_u16 = 0;
		MessageDisplay01SecTimeout_u16 = 0;
		b_flag_Blink = 0;
		break;
	};
	case CLOCK_STATE_IP: // Ip address.
	{
		PackedDisplayData.hour_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.hour_ten];
		PackedDisplayData.hour_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.hour_unit];
		PackedDisplayData.minute_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.minute_ten] | digit_seg_ui8[28];
		PackedDisplayData.minute_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.minute_unit];
		PackedDisplayData.second_ten_digit_st_ui8 = digit_seg_ui8[mainBuffer.second_ten];
		PackedDisplayData.second_unit_digit_st_ui8 = digit_seg_ui8[mainBuffer.second_unit] | digit_seg_ui8[28];
		mainBuffer.esp_states.lightMode = LIGHT_MODE_MANUAL;
		mainBuffer.brightness = BRIGHTNESS_MAX;
		MessageDisplay01MinTimeout_u16 = 0;
		MessageDisplay01SecTimeout_u16 = 0;
		b_flag_Blink = 0;
		break;
	};
	default: //    -HELP- message
	{
		PackedDisplayData.hour_ten_digit_st_ui8 = digit_seg_ui8[23];	// -
		PackedDisplayData.hour_unit_digit_st_ui8 = digit_seg_ui8[14];	// H
		PackedDisplayData.minute_ten_digit_st_ui8 = digit_seg_ui8[15];	// E
		PackedDisplayData.minute_unit_digit_st_ui8 = digit_seg_ui8[16]; // L
		PackedDisplayData.second_ten_digit_st_ui8 = digit_seg_ui8[17];	// P
		PackedDisplayData.second_unit_digit_st_ui8 = digit_seg_ui8[23]; // -
		b_flag_Blink = 0;
		break;
	}
	}
	return;
}

/**
 * @brief Function implementation for display output buffer
 *
 * Function send data to display (60 bits, bit-to-bit) @ 500ms
 */
void PackedData()
{
	uint8_t counter_bit_ui8;

	PackedDisplayData.G_hour_ten = 1;
	PackedDisplayData.G_hour_unit = 1;
	PackedDisplayData.G_minute_ten = 1;
	PackedDisplayData.G_minute_unit = 1;
	PackedDisplayData.G_second_ten = 1;
	PackedDisplayData.G_second_unit = 1;

	PackedDisplayData.ledHT = 1;
	PackedDisplayData.ledHU = 1;
	PackedDisplayData.ledMT = 1;
	PackedDisplayData.ledMU = 1;
	PackedDisplayData.ledST = 1;
	PackedDisplayData.ledSU = 1;

	for (counter_bit_ui8 = EN_START_BIT_SEND_4; counter_bit_ui8 <= EN_START_BIT_SEND_7; counter_bit_ui8++)
	{
		digitalWrite(EN_SOD_VFCLK, 0);
		digitalWrite(EN_SOD_VFDOUT, bitRead(PackedDisplayData.byteH2, counter_bit_ui8));
		digitalWrite(EN_SOD_VFCLK, 1);
	}

	for (counter_bit_ui8 = 0; counter_bit_ui8 <= EN_START_BIT_SEND_7; counter_bit_ui8++)
	{
		digitalWrite(EN_SOD_VFCLK, 0);
		digitalWrite(EN_SOD_VFDOUT, bitRead(PackedDisplayData.byteM2, counter_bit_ui8));
		digitalWrite(EN_SOD_VFCLK, 1);
	}

	for (counter_bit_ui8 = 0; counter_bit_ui8 <= EN_START_BIT_SEND_7; counter_bit_ui8++)
	{
		digitalWrite(EN_SOD_VFCLK, 0);
		digitalWrite(EN_SOD_VFDOUT, bitRead(PackedDisplayData.byteL2, counter_bit_ui8));
		digitalWrite(EN_SOD_VFCLK, 1);
	}

	for (counter_bit_ui8 = EN_START_BIT_SEND_4; counter_bit_ui8 <= EN_START_BIT_SEND_7; counter_bit_ui8++)
	{
		digitalWrite(EN_SOD_VFCLK, 0);
		digitalWrite(EN_SOD_VFDOUT, bitRead(PackedDisplayData.byteH1, counter_bit_ui8));
		digitalWrite(EN_SOD_VFCLK, 1);
	}

	for (counter_bit_ui8 = 0; counter_bit_ui8 <= EN_START_BIT_SEND_7; counter_bit_ui8++)
	{
		digitalWrite(EN_SOD_VFCLK, 0);
		digitalWrite(EN_SOD_VFDOUT, bitRead(PackedDisplayData.byteM1, counter_bit_ui8));
		digitalWrite(EN_SOD_VFCLK, 1);
	}

	for (counter_bit_ui8 = 0; counter_bit_ui8 <= EN_START_BIT_SEND_7; counter_bit_ui8++)
	{
		digitalWrite(EN_SOD_VFCLK, 0);
		digitalWrite(EN_SOD_VFDOUT, bitRead(PackedDisplayData.byteL1, counter_bit_ui8));
		digitalWrite(EN_SOD_VFCLK, 1);
	}

	for (counter_bit_ui8 = EN_START_BIT_SEND_4; counter_bit_ui8 <= EN_START_BIT_SEND_7; counter_bit_ui8++)
	{
		digitalWrite(EN_SOD_VFCLK, 0);
		digitalWrite(EN_SOD_VFDOUT, bitRead(PackedDisplayData.byteH0, counter_bit_ui8));
		digitalWrite(EN_SOD_VFCLK, 1);
	}

	for (counter_bit_ui8 = 0; counter_bit_ui8 <= EN_START_BIT_SEND_7; counter_bit_ui8++)
	{
		digitalWrite(EN_SOD_VFCLK, 0);
		digitalWrite(EN_SOD_VFDOUT, bitRead(PackedDisplayData.byteM0, counter_bit_ui8));
		digitalWrite(EN_SOD_VFCLK, 1);
	}

	for (counter_bit_ui8 = 0; counter_bit_ui8 <= EN_START_BIT_SEND_7; counter_bit_ui8++)
	{
		digitalWrite(EN_SOD_VFCLK, 0);
		digitalWrite(EN_SOD_VFDOUT, bitRead(PackedDisplayData.byteL0, counter_bit_ui8));
		digitalWrite(EN_SOD_VFCLK, 1);
	}

	digitalWrite(EN_SOD_VFLOAD, 1);
	digitalWrite(EN_SOD_VFCLK, 0); // reset Clock

	if (b_flag_Blink)
	{
		sb_maskBlink ^= 1;
	}
	else
	{
		sb_maskBlink = 0;
	}
	digitalWrite(EN_SOD_VFLOAD, 0);
}

/**
 * @brief Function display data
 *
 * Function display data with PWM and blink display @ 250ms if b_flag_Blink is set
 * The change intesity is modulated.
 */
void Enable_Display()
{
	static int signed lb_deltavalueLight;
	static uint8_t lub_valuebrightness = BRIGHTNESS_ZERO;

	lb_deltavalueLight = mainBuffer.brightness - lub_valuebrightness;
	if (lb_deltavalueLight > 0)
	{
		lub_valuebrightness++;
		lb_deltavalueLight++;
	}
	if (lb_deltavalueLight < 0)
	{
		lub_valuebrightness--;
		lb_deltavalueLight--;
	}
	if (sb_maskBlink)
	{
		analogWrite(EN_SOD_VFBLANK, BRIGHTNESS_ZERO); /* Blinking and blink flag = 1*/
	}
	else
	{
		analogWrite(EN_SOD_VFBLANK, lub_valuebrightness);
	}
}

#elif defined(PIXIE)
DISPLAY_DATA DisplayData;

void TurnOffDisplay()
{
	DisplayData.hour_data = 0xFF;
	DisplayData.minute_data = 0xFF;
	DisplayData.second_data = 0xFF;
}

void DisplayInit()
{
#if defined(FOUR_DIGITS)
	pinMode(EN_COLON, OUTPUT);
	pinMode(EN_SHDN, OUTPUT);
#elif defined(SIX_DIGITS)
	pinMode(EN_SECOND_DATA, OUTPUT);
	pinMode(EN_SECOND_CLOCK, OUTPUT);
	pinMode(EN_SECOND_LATCH, OUTPUT);
#endif
	pinMode(EN_MINUTE_DATA, OUTPUT);
	pinMode(EN_MINUTE_CLOCK, OUTPUT);
	pinMode(EN_MINUTE_LATCH, OUTPUT);
	pinMode(EN_HOUR_DATA, OUTPUT);
	pinMode(EN_HOUR_CLOCK, OUTPUT);
	pinMode(EN_HOUR_LATCH, OUTPUT);
	TurnOffDisplay();
}

/**
 * @brief Function implementation for processing output buffer
 *
 * Function pack data for processing output buffer @ 50ms
 */
void ClockPacked()
{
	static uint16_t MessageDisplay01SecTimeout_u16;
	static uint16_t MessageDisplay15SecTimeout_u16;

	if (mainBuffer.brightness == BRIGHTNESS_ZERO)
	{
		TurnOffDisplay();
		return;
	}
	switch (mainBuffer.esp_states.clockState)
	{
	case CLOCK_STATE_START:
	{
		DisplayData.hour_data = 0;
		DisplayData.minute_data = 0;
		DisplayData.second_data = 0;
		break;
	};
	case CLOCK_STATE_AP:
	case CLOCK_STATE_SERVER_DOWN:
	{
		MessageDisplay15SecTimeout_u16++;
		if (MessageDisplay15SecTimeout_u16 >= MESSAGE_DISPLAY_15_SEC_TIMEOUT)
		{
			MessageDisplay15SecTimeout_u16 = 0;
			MessageDisplay01SecTimeout_u16 = 0;
		}
		if (MessageDisplay01SecTimeout_u16 < MESSAGE_DISPLAY_01_SEC_TIMEOUT)
		{
			TurnOffDisplay();
			MessageDisplay01SecTimeout_u16++;
		}
		else
		{
			DisplayData.hour_data = (mainBuffer.hour_unit & 0x0F) << 4 | (mainBuffer.hour_ten & 0x0F);
			DisplayData.minute_data = (mainBuffer.minute_unit & 0x0F) << 4 | (mainBuffer.minute_ten & 0x0F);
			DisplayData.second_data = (mainBuffer.second_unit & 0x0F) << 4 | (mainBuffer.second_ten & 0x0F);
		}
		break;
	};
	case CLOCK_STATE_IP:
#if defined(FOUR_DIGITS)
	{
		static uint16_t MessageDisplay03SecTimeout_u16;

		/* The bytes of an IP address are displayed one-by-one for 3 seconds */
		if (MessageDisplay03SecTimeout_u16 < MESSAGE_DISPLAY_03_SEC_TIMEOUT)
		{
			DisplayData.hour_data = (mainBuffer.hour_unit & 0x0F) << 4 | (mainBuffer.hour_ten & 0x0F);
			DisplayData.minute_data = 0xF0 | (mainBuffer.minute_ten & 0x0F);
		}
		else
		{
			DisplayData.hour_data = (mainBuffer.second_ten & 0x0F) << 4 | (mainBuffer.minute_unit & 0x0F);
			DisplayData.minute_data = 0xF0 | (mainBuffer.second_unit & 0x0F);
		}
		(++MessageDisplay03SecTimeout_u16) %= MESSAGE_DISPLAY_03_SEC_TIMEOUT * 2 - 5;
		MessageDisplay01SecTimeout_u16 = MESSAGE_DISPLAY_01_SEC_TIMEOUT;
		MessageDisplay15SecTimeout_u16 = 0;
		break;
	};
#endif
	case CLOCK_STATE_VALID:
	{
#if defined(FOUR_DIGITS)
		analogWrite(EN_COLON, (MessageDisplay01SecTimeout_u16 < MESSAGE_DISPLAY_01_SEC_TIMEOUT) << 4);
		(++MessageDisplay01SecTimeout_u16) %= MESSAGE_DISPLAY_01_SEC_TIMEOUT * 2;
#endif
		DisplayData.hour_data = (mainBuffer.hour_unit & 0x0F) << 4 | (mainBuffer.hour_ten & 0x0F);
		DisplayData.minute_data = (mainBuffer.minute_unit & 0x0F) << 4 | (mainBuffer.minute_ten & 0x0F);
		DisplayData.second_data = (mainBuffer.second_unit & 0x0F) << 4 | (mainBuffer.second_ten & 0x0F);
		MessageDisplay15SecTimeout_u16 = 0;
		break;
	};
	default:
		break;
	}
}

/**
 * @brief Function implementation for uptading a shift register
 *
 * Function sends a byte to a shift register
 */
void UpdateShiftRegister(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t data)
{
	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, MSBFIRST, data);
	digitalWrite(latchPin, HIGH);
}

/**
 * @brief Function implementation for display output buffer
 *
 * Function send data to display @ 250ms
 */
void PackedData()
{
	UpdateShiftRegister(EN_HOUR_LATCH, EN_HOUR_CLOCK, EN_HOUR_DATA, DisplayData.hour_data);
	UpdateShiftRegister(EN_MINUTE_LATCH, EN_MINUTE_CLOCK, EN_MINUTE_DATA, DisplayData.minute_data);
/* this is a temporary solution, because turning off the tubes should have been done through SHDN pin of the source for every tube */
#if defined(FOUR_DIGITS)
	if (DisplayData.hour_data == 0xFF && DisplayData.minute_data == 0xFF)
	{
		digitalWrite(EN_COLON, LOW);
		digitalWrite(EN_SHDN, HIGH);
	}
	else
	{
		digitalWrite(EN_SHDN, LOW);
	}
#elif defined(SIX_DIGITS)
	UpdateShiftRegister(EN_SECOND_LATCH, EN_SECOND_CLOCK, EN_SECOND_DATA, DisplayData.second_data);
#endif
}
#endif