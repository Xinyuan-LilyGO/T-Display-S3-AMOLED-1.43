/*
 * @Description: Sleep
 * @Author: LILYGO_L
 * @Date: 2024-03-11 10:05:32
 * @LastEditTime: 2024-11-20 11:26:25
 * @License: GPL 3.0
 */
#include "Arduino.h"
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "TouchDrvCST92xx.h"
#include "SensorWireHelper.h"

#define SLEEP_WAKE_UP_INT GPIO_NUM_0

static size_t CycleTime = 0;

volatile int8_t IIC_Interrupt_Flag;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_GFX *gfx = new Arduino_SH8601(bus, LCD_RST /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);

TouchDrvCST92xx CST9217;

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);

    pinMode(SLEEP_WAKE_UP_INT, INPUT_PULLUP);

    attachInterrupt(TP_INT, []()
                    { IIC_Interrupt_Flag = true; }, FALLING);

    // Set to skip register check, used when the touch device address conflicts with other I2C device addresses [0x5A]
    CST9217.jumpCheck();

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

    gfx->begin();
    gfx->fillScreen(WHITE);

    for (int i = 0; i <= 255; i++)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }

    gfx->setCursor(0, LCD_HEIGHT / 2);
    gfx->setTextSize(2);
    gfx->setTextColor(RED);
    gfx->println("Enter deep sleep in 5 seconds");
    Serial.println("Enter deep sleep in 5 seconds");

    CycleTime = millis() + 5000;
}

void loop()
{
    if (millis() > CycleTime)
    {
        delay(300);

        gfx->Display_Brightness(0);
        gfx->displayOff();
        digitalWrite(LCD_EN, LOW);

        CST9217.sleep();

        Serial.println("Enter deep sleep");
        gpio_hold_en(GPIO_NUM_0);
        esp_sleep_enable_ext0_wakeup(SLEEP_WAKE_UP_INT, LOW);
        esp_deep_sleep_start();
    }
}
