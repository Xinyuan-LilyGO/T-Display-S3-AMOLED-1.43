/*
 * @Description: Sleep
 * @Author: LILYGO_L
 * @Date: 2023-08-18 15:26:31
 * @LastEditTime: 2024-11-20 11:26:37
 * @License: GPL 3.0
 */
#include "Arduino.h"
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "TouchDrvCST92xx.h"
#include "SensorWireHelper.h"

#define SLEEP_ENTER_KEY 0
#define SLEEP_WAKE_UP_INT GPIO_NUM_9

static size_t CycleTime = 0;
static bool Light_Sleep_Exit_Initialization_Flag = false;
static bool Light_Sleep_Exit_Flag = false;

volatile int8_t IIC_Interrupt_Flag;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

// DO0143FMST02
// Arduino_GFX *gfx = new Arduino_SH8601(bus, LCD_RST /* RST */,
//                                       0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);

// H0175Y003AMV0
Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RST /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT,
                                      6 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

TouchDrvCST92xx CST9217;

void setup()
{
    // 使用浅睡眠功能建议将串口配置为UART0而不是USB
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);

    pinMode(SLEEP_ENTER_KEY, INPUT_PULLUP);
    pinMode(SLEEP_WAKE_UP_INT, INPUT_PULLUP);

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

    gfx->fillScreen(WHITE);
    gfx->setCursor(0, LCD_HEIGHT / 2);
    gfx->setTextSize(2);
    gfx->setTextColor(RED);

    Serial.println("Press the button to enter light sleep");
    gfx->println("Press the button to enter light sleep");
}

void loop()
{
    if (digitalRead(SLEEP_ENTER_KEY) == LOW)
    {
        delay(300);

        gfx->fillScreen(WHITE);
        gfx->setCursor(0, LCD_HEIGHT / 2);
        gfx->setTextSize(2);
        gfx->setTextColor(RED);

        Serial.println("Enter light sleep in 3 seconds");
        gfx->println("Enter light sleep in 3 seconds");
        Serial.println("3");
        delay(1000);
        Serial.println("2");
        delay(1000);
        Serial.println("1");
        delay(1000);

        Light_Sleep_Exit_Initialization_Flag = true;

        gfx->Display_Brightness(0);
        gfx->displayOff();
        digitalWrite(LCD_EN, LOW);

        esp_sleep_enable_ext0_wakeup(SLEEP_WAKE_UP_INT, LOW);
        esp_light_sleep_start();
    }

    if (Light_Sleep_Exit_Initialization_Flag == true)
    {
        // 浅睡眠过后使用串口工具查看信息
        Light_Sleep_Exit_Initialization_Flag = false;
        Light_Sleep_Exit_Flag = true;

        delay(1000);

        digitalWrite(LCD_EN, HIGH);

        gfx->displayOn();
        gfx->Display_Brightness(255);

        gfx->fillScreen(WHITE);
    }

    if (Light_Sleep_Exit_Flag == true)
    {
        if (millis() > CycleTime)
        {
            gfx->fillScreen(WHITE);
            gfx->setCursor(0, LCD_HEIGHT / 2);
            gfx->setTextSize(2);
            gfx->setTextColor(RED);

            Serial.println("Successfully exited light sleep");
            gfx->println("Successfully exited light sleep");
            CycleTime = millis() + 1000;
        }
    }
}
