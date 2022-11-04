#include "Arduino_FreeRTOS.h"
#include "Definitions.h"
#include "Input.h"
#include "Output.h"

#define TASK_20MS_DELAY 20 / portTICK_PERIOD_MS
#define TASK_50MS_DELAY 50 / portTICK_PERIOD_MS
#define TASK_250MS_DELAY 250 / portTICK_PERIOD_MS
#define TASK_1000MS_DELAY 1000 / portTICK_PERIOD_MS

void task20ms(void *pvParameters);
void task50ms(void *pvParameters);
void task250ms(void *pvParameters);
void task1000ms(void *pvParameters);

void setup()
{
   Input_init();
   Output_init();

   xTaskCreate(task20ms, (const char *)"task20ms", 128, NULL, 3, NULL);
   xTaskCreate(task50ms, (const char *)"task50ms", 128, NULL, 3, NULL);
   xTaskCreate(task250ms, (const char *)"task250ms", 128, NULL, 3, NULL);
   xTaskCreate(task1000ms, (const char *)"task1000ms", 128, NULL, 3, NULL);
}

/**
 * @brief Implementation of function that handle the 18ms requests.
 *
 * Implementation of function that handle the 18ms requests.
 * @return void
 */
void task20ms(void *pvParameters)
{
   while (true)
   {
      Input_20ms_task();
#if defined(FLEURIE)
      Enable_Display();
#endif
      vTaskDelay(TASK_20MS_DELAY);
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

void task1000ms(void *pvParameters)
{
   while (true)
   {
      Input_1000ms_task();
      vTaskDelay(TASK_1000MS_DELAY);
   }
}

void loop()
{
   /* do nothing */
}
