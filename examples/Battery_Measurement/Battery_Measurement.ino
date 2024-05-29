/*
 * @Description: 电池电压测量程序
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-08-29 13:34:55
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-03-30 09:26:50
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "pin_config.h"

static size_t CycleTime = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(BATTERY_VOLTAGE_ADC_DATA, INPUT_PULLDOWN);

    analogReadResolution(12);
    analogSetPinAttenuation(BATTERY_VOLTAGE_ADC_DATA, ADC_ATTENDB_MAX);
    adcAttachPin(BATTERY_VOLTAGE_ADC_DATA);
}

void loop()
{
    if (millis() > CycleTime)
    {
        Serial.printf("Battery Voltage Data: %d mV\n\n", analogReadMilliVolts(BATTERY_VOLTAGE_ADC_DATA) * 2);

        CycleTime = millis() + 3000; // 3000ms
    }
}