#include <Arduino.h>
#include <EEPROM.h>
#include "Ticker.h"
#include "hw.h"
#include "memory.h"
#include "network.h"

void task20ms();
void task100ms();
void task1000ms();

Ticker task20msTicker(task20ms, 20);
Ticker task100msTicker(task100ms, 100);
Ticker task1000msTicker(task1000ms, 1000);

void setup()
{
    Serial.begin(9600);

    /* Intentional order, memories initialized before used. */
    Memory_init();
    Hardware_init();
    Network_init();
    task20msTicker.start();
    task100msTicker.start();
    task1000msTicker.start();
}

void task20ms()
{
    Hardware_20ms_task();
}

void task100ms()
{
    Network_100ms_task();
}

void task1000ms()
{
    /* Intentional order, time evaluated before displayed. */
    Network_1000ms_task();
    Hardware_1000ms_task();
}

void loop()
{
    task20msTicker.update();
    task100msTicker.update();
    task1000msTicker.update();
}
