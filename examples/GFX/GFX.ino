/*
 * @Description: GFX屏幕显示示例
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-09-06 10:58:19
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-05-29 10:43:59
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_GFX *gfx = new Arduino_SH8601(bus, LCD_RST /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);

    gfx->begin();
    gfx->fillScreen(WHITE);

    for (int i = 0; i <= 255; i++)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }

    // for (int i = 0; i < 4; i++)
    // {
    //     gfx->SetContrast(i);
    //     delay(2000);
    // }

    // gfx->SetContrast(0);

    gfx->fillScreen(RED);
    delay(1000);
    gfx->fillScreen(GREEN);
    delay(1000);
    gfx->fillScreen(BLUE);
    delay(1000);

    // gfx->fillRect(30, 30, 2, 2, RED);

    gfx->fillScreen(WHITE);
    gfx->setCursor(100, 100);
    gfx->setTextColor(BLACK);
    gfx->println("Ciallo1~(L *##*L)^**");
}

void loop()
{
    // gfx->fillScreen(PINK);
    // delay(1000);
}
