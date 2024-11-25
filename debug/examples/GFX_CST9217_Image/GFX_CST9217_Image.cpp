/*
 * @Description: GFX屏幕显示+触摸切换图片
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-09-06 10:58:19
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-09-06 16:21:39
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "Material_16Bit_466x466px.h"
#include "TouchDrvCST92xx.h"
#include "SensorWireHelper.h"

static uint8_t Image_Flag = 0;

volatile int8_t IIC_Interrupt_Flag;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

// DO0143FAT01
// Arduino_GFX *gfx = new Arduino_SH8601(bus, LCD_RST /* RST */,
//                                       0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);

// H0175Y003AM
Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RST /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT,
                                      6 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

TouchDrvCST92xx CST9217;

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);

    attachInterrupt(TP_INT, []()
                    { IIC_Interrupt_Flag = true; }, FALLING);

    CST9217.setPins(-1, TP_INT);
    if (CST9217.begin(Wire, 0x5A, IIC_SDA, IIC_SCL) == false)
    {
        Serial.println("CST9217 initialization failed");
    }
    else
    {
        Serial.print("Model :");
        Serial.println(CST9217.getModelName());
    }

    gfx->begin(80000000);
    gfx->fillScreen(WHITE);

    for (int i = 0; i <= 255; i++)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }

    gfx->setCursor(100, 100);
    gfx->setTextSize(2);
    gfx->setTextColor(BLACK);
    gfx->printf("Ciallo");

    delay(1000);
}

void loop()
{
    // Serial.printf("System running time: %d\n\n", (uint32_t)millis() / 1000);

    if (IIC_Interrupt_Flag == true)
    {
        IIC_Interrupt_Flag = false;

        int16_t touch_x[5];
        int16_t touch_y[5];
        uint8_t fingers_number = 0;

        if (CST9217.getPoint(touch_x, touch_y, 2) == true)
        {
            touch_x[0] = LCD_WIDTH - touch_x[0];
            touch_y[0] = LCD_HEIGHT - touch_y[0];

            switch (Image_Flag)
            {
            case 0:
                gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_1, LCD_WIDTH, LCD_HEIGHT); // RGB
                break;
            case 1:
                gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_2, LCD_WIDTH, LCD_HEIGHT); // RGB
                break;
            case 2:
                gfx->fillScreen(PINK);
                gfx->setCursor(100, 100);
                gfx->setTextColor(YELLOW);
                gfx->setTextSize(1);
                gfx->println("Ciallo1~(L *##*L)^**");

                gfx->setTextSize(2);
                break;
            case 3:
                gfx->fillScreen(PINK);
                gfx->setCursor(100, 100);
                gfx->setTextColor(YELLOW);
                gfx->setTextSize(2);
                gfx->println("Ciallo1~(L *##*L)^**");
                break;

            default:
                break;
            }

            Image_Flag++;

            if (Image_Flag > 3)
            {
                Image_Flag = 0;
            }

            gfx->setTextSize(2);
            gfx->setTextColor(MAROON);
            gfx->fillCircle(touch_x[0], touch_y[0], 3, RED);
            gfx->setCursor(touch_x[0], touch_y[0]);
            gfx->printf(" Touch X1:%d Y1:%d", touch_x[0], touch_y[0]);
        }
    }
}
