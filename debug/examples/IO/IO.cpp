/*
 * @Description: None
 * @version: V1.0.0
 * @Author: None
 * @Date: 2023-08-29 13:34:55
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-03-28 09:57:49
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "pin_config.h"

uint8_t i = 0;

void setup()
{
    Serial.begin(115200);

    // pinMode(GPIO_NUM_1, OUTPUT);
    // pinMode(GPIO_NUM_42, OUTPUT);
    // pinMode(GPIO_NUM_2, OUTPUT);
    // pinMode(GPIO_NUM_40, OUTPUT);
    // pinMode(GPIO_NUM_41, OUTPUT);
    // pinMode(GPIO_NUM_4, OUTPUT);
    // pinMode(GPIO_NUM_6, OUTPUT);
    pinMode(LCD_SDIO3, OUTPUT);
    pinMode(LCD_SDIO2, OUTPUT);
    pinMode(LCD_SDIO1, OUTPUT);
    pinMode(LCD_SDIO0, OUTPUT);
    pinMode(LCD_SCLK, OUTPUT);
    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_RST, OUTPUT);
    pinMode(LCD_EN, OUTPUT);

    //     digitalWrite(GPIO_NUM_1, i);
    // digitalWrite(GPIO_NUM_42, i);
    // digitalWrite(GPIO_NUM_2, i);
    // digitalWrite(GPIO_NUM_40, i);
    // digitalWrite(GPIO_NUM_41, i);
    // digitalWrite(GPIO_NUM_4, i);
    // digitalWrite(GPIO_NUM_6, i);
    digitalWrite(LCD_SDIO3, i);
    digitalWrite(LCD_SDIO2, i);
    digitalWrite(LCD_SDIO1, i);
    digitalWrite(LCD_SDIO0, i);
    digitalWrite(LCD_SCLK, i);
    digitalWrite(LCD_CS, i);
    digitalWrite(LCD_RST, i);
    digitalWrite(LCD_EN, i);
}

void loop()
{
    // digitalWrite(GPIO_NUM_1, i);
    // digitalWrite(GPIO_NUM_42, i);
    // digitalWrite(GPIO_NUM_2, i);
    // digitalWrite(GPIO_NUM_40, i);
    // digitalWrite(GPIO_NUM_41, i);
    // digitalWrite(GPIO_NUM_4, i);
    // digitalWrite(GPIO_NUM_6, i);
    // digitalWrite(GPIO_NUM_5, i);
    // digitalWrite(GPIO_NUM_7, i);
    // digitalWrite(GPIO_NUM_16, i);
    // digitalWrite(GPIO_NUM_15, i);
    // digitalWrite(GPIO_NUM_17, i);
    // digitalWrite(GPIO_NUM_8, i);
    // digitalWrite(GPIO_NUM_18, i);

    digitalWrite(LCD_SDIO3, i);
    digitalWrite(LCD_SDIO2, i);
    digitalWrite(LCD_SDIO1, i);
    digitalWrite(LCD_SDIO0, i);
    digitalWrite(LCD_SCLK, i);
    digitalWrite(LCD_CS, i);
    digitalWrite(LCD_RST, i);
    digitalWrite(LCD_EN, i);

    i = !i;
    delay(3000);
}