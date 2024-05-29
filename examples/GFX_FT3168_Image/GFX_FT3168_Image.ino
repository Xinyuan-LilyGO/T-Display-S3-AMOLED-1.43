/*
 * @Description: GFX屏幕显示+触摸切换图片
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-09-06 10:58:19
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-05-29 10:44:34
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include "Material_16Bit_466x466px.h"

static uint8_t Image_Flag = 0;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_GFX *gfx = new Arduino_SH8601(bus, LCD_RST /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(IIC_Bus, FT3168_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, TP_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void)
{
    FT3168->IIC_Interrupt_Flag = true;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);

    while (FT3168->begin() == false)
    {
        Serial.println("FT3168 initialization fail");
        delay(2000);
    }
    Serial.println("FT3168 initialization successfully");

    gfx->begin();
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

    Serial.printf("ID: %#X \n\n", (int32_t)FT3168->IIC_Read_Device_ID());
    delay(1000);
}

void loop()
{
    // Serial.printf("System running time: %d\n\n", (uint32_t)millis() / 1000);

    if (FT3168->IIC_Interrupt_Flag == true)
    {
        FT3168->IIC_Interrupt_Flag = false;

        int32_t touch_x = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
        int32_t touch_y = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
        uint8_t fingers_number = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

        if (fingers_number > 0)
        {
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

            Serial.printf("[1] point x: %d  point y: %d \r\n", touch_x, touch_y);

            gfx->setCursor(touch_x, touch_y);
            gfx->setTextColor(RED);
            gfx->printf("[1] point x: %d  point y: %d \r\n", touch_x, touch_y);
        }
    }
}
