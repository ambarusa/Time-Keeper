#include "Definitions.h"
#include "Input.h"
#include "Output.h"

#define CONVERT_KHZ_TO_HZ 1000

void setup()
{
   Input_init();
   Output_init();

   cli();
   // EN_SOD_VFBLANK PWM pin is already connected to Timer1
   // set timer2 interrupt at 1kHz
   TCCR2A = 0; // set entire TCCR2A register to 0
   TCCR2B = 0; // same for TCCR2B
   TCNT2 = 0;  // initialize counter value to 0
   // set compare match register for 1khz increments
   OCR2A = 249; // = (16*10^6) / (1000*64) - 1 (must be <256)
   // turn on CTC mode
   TCCR2A |= (1 << WGM21);
   // Set CS21 bit for 64 prescaler
   TCCR2B |= (1 << CS22) | (0 << CS21) | (0 << CS20);
   // enable timer compare interrupt
   TIMSK2 |= (1 << OCIE2A);

   sei();
}

void loop()
{
   static uint32_t task20Millis, prev20Millis;
   static uint32_t task50Millis, prev50Millis;
   static uint32_t task250Millis, prev250Millis;

   task20Millis = millis();
   if (task20Millis - prev20Millis >= 20)
   {
      Input_20ms_task();
#if defined(FLEURIE)
      Enable_Display();
#endif
      prev20Millis = task20Millis;
   }

   task50Millis = millis();
   if (task50Millis - prev50Millis >= 50)
   {
      ClockPacked();
      prev50Millis = task50Millis;
   }

   task250Millis = millis();
   if (task250Millis - prev250Millis >= 250)
   {
      PackedData();
      prev250Millis = task250Millis;
   }
}

static volatile uint8_t led_u8;
static volatile uint16_t counter_u16;

ISR(TIMER2_COMPA_vect)
{
   if (counter_u16++ >= CONVERT_KHZ_TO_HZ)
   {
      counter_u16 = 0;
      led_u8 = !led_u8;
      digitalWrite(LED_BUILTIN, led_u8);
      Input_1000ms_task();
   }
}