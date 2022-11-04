#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include "Definitions.h"

void Output_init();

/**
 * @brief Function implementation for processing output buffer
  *
  * Function pack data for processing output buffer @ 1000ms
 */
void ClockPacked();

/**
 * @brief Function implementation for display output buffer
  *
  * Function send data to display (60 bits) @ 500ms
 */
void PackedData();

/**
 * @brief Function display data
  *
  * Function display data with PWM and blink display @ 500ms if b_flag_Blink is set
 */
void Enable_Display();

#endif /*_OUTPUT_H_*/
