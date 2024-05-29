/*
 * @Description: Sleep 
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2024-03-11 10:05:32
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-05-29 10:42:41
 * @License: GPL 3.0
 */
#include "Arduino.h"
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"

#define SLEEP_WAKE_UP_INT GPIO_NUM_9

static size_t CycleTime = 0;

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

    pinMode(SLEEP_WAKE_UP_INT, INPUT_PULLUP);

    if (FT3168->begin() == false)
    {
        Serial.println("FT3168 initialization fail");
        delay(2000);
    }
    else
    {
        Serial.println("FT3168 initialization successfully");
    }
    // // 激活模式
    // FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
    //                                FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_ACTIVE);

    // // 触摸芯片是否自动进入Monitor模式
    // FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_AUTOMATICALLY_MONITOR_MODE,
    //                                FT3168->Arduino_IIC_Touch::Device_State::TOUCH_DEVICE_ON);
    // // 设定触摸芯片自动进入Monitor模式的时间为10秒
    // FT3168->IIC_Write_Device_Value(FT3168->Arduino_IIC_Touch::Device_Value::TOUCH_AUTOMATICALLY_MONITOR_TIME,
    //                                10);

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

        // 激活模式
        FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
                                       FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);

        // 触摸芯片是否自动进入Monitor模式
        FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_AUTOMATICALLY_MONITOR_MODE,
                                       FT3168->Arduino_IIC_Touch::Device_State::TOUCH_DEVICE_ON);
        // 设定触摸芯片自动进入Monitor模式的时间为3秒
        FT3168->IIC_Write_Device_Value(FT3168->Arduino_IIC_Touch::Device_Value::TOUCH_AUTOMATICALLY_MONITOR_TIME,
                                       3);

        Serial.println("Enter deep sleep");
        // gpio_hold_en(GPIO_NUM_0);
        esp_sleep_enable_ext0_wakeup(SLEEP_WAKE_UP_INT, LOW);
        esp_deep_sleep_start();
    }
}
