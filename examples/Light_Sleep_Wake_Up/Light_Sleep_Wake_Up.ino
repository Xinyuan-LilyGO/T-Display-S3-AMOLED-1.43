/*
 * @Description: Sleep
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-08-18 15:26:31
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-05-29 10:45:07
 * @License: GPL 3.0
 */
#include "Arduino.h"
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"

#define SLEEP_ENTER_KEY 0
#define SLEEP_WAKE_UP_INT GPIO_NUM_9

static size_t CycleTime = 0;
static bool Light_Sleep_Exit_Initialization_Flag = false;
static bool Light_Sleep_Exit_Flag = false;

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
    // 使用浅睡眠功能建议将串口配置为UART0而不是USB
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);

    pinMode(SLEEP_ENTER_KEY, INPUT_PULLUP);
    pinMode(SLEEP_WAKE_UP_INT, INPUT_PULLUP);

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

        // 激活模式
        FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
                                       FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);

        // 触摸芯片是否自动进入Monitor模式
        FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_AUTOMATICALLY_MONITOR_MODE,
                                       FT3168->Arduino_IIC_Touch::Device_State::TOUCH_DEVICE_ON);
        // 设定触摸芯片自动进入Monitor模式的时间为3秒
        FT3168->IIC_Write_Device_Value(FT3168->Arduino_IIC_Touch::Device_Value::TOUCH_AUTOMATICALLY_MONITOR_TIME,
                                       3);

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
