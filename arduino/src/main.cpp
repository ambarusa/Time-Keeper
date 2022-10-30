#include "Arduino_FreeRTOS.h"
#include "Definitions.h"
#include "Input.h"
#include "Output.h"

#define TASK_18MS_DELAY 1
#define TASK_50MS_DELAY 3
#define TASK_250MS_DELAY 15

void task18ms(void *pvParameters);
void task50ms(void *pvParameters);
void task250ms(void *pvParameters);

void setup()
{
   Serial.begin(9600);
   LightSensorInit();
   DisplayInit();

   xTaskCreate(task18ms, (const char *)"task18ms", 128, NULL, 3, NULL);
   xTaskCreate(task50ms, (const char *)"task50ms", 128, NULL, 3, NULL);
   xTaskCreate(task250ms, (const char *)"task250ms", 128, NULL, 3, NULL);
}

/**
 * @brief Implementation of function that handle the 18ms requests.
 *
 * Implementation of function that handle the 18ms requests.
 * @return void
 */
void task18ms(void *pvParameters)
{
   while (true)
   {
      InputCyclic();
#if defined(FLEURIE)
      Enable_Display();
#endif
      vTaskDelay(TASK_18MS_DELAY);
   }
}

/**
 * @brief Implementation of function that handle the 250ms requests.
 *
 * Implementation of function that handle the 250ms requests.
 * @return void
 */
void task250ms(void *pvParameters)
{
   while (true)
   {
      PackedData();
      vTaskDelay(TASK_250MS_DELAY);
   }
}

/**
 * @brief Implementation of function that handle the 50ms requests.
 *
 * Implementation of function that handle the 50ms requests.
 * @return void
 */
void task50ms(void *pvParameters)
{
   while (true)
   {
      ClockPacked();
      vTaskDelay(TASK_50MS_DELAY);
   }
}

void loop()
{
   /* do nothing */
}
