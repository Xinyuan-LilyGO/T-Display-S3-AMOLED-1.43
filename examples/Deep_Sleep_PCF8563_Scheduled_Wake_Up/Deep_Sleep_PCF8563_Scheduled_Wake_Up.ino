/*
 * @Description: 深度睡眠后PCF8563计划唤醒
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2024-03-27 10:02:26
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-05-29 10:37:04
 * @License: GPL 3.0
 */

#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::unique_ptr<Arduino_IIC> PCF8563(new Arduino_PCF8563(IIC_Bus, PCF8563_DEVICE_ADDRESS,
                                                         DRIVEBUS_DEFAULT_VALUE, DRIVEBUS_DEFAULT_VALUE));

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    while (PCF8563->begin() == false)
    {
        Serial.println("PCF8563 initialization fail");
        delay(2000);
    }
    Serial.println("PCF8563 initialization successfully");

    // 关闭时钟输出
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_CLOCK_OUTPUT,
                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_OFF);

    // 关闭RTC
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_CLOCK_RTC,
                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_OFF);
    // 时钟传感器设置秒
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_SECONDS_DATA,
                                    58);
    // 时钟传感器设置分
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_MINUTES_DATA,
                                    59);
    // 时钟传感器设置时
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_HOURS_DATA,
                                    23);
    // 时钟传感器设置天
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_DAYS_DATA,
                                    31);
    // 时钟传感器设置月
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_MONTHS_DATA,
                                    12);
    // 时钟传感器设置年
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_YEARS_DATA,
                                    99);

    // 下面设定10秒定时
    //  设置定时器频率的值为1Hz（选择的频率越高越精准）
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_TIMER_FREQUENCY_VALUE,
                                    PCF8563->Arduino_IIC_RTC::Device_Mode::RTC_CLOCK_OUTPUT_1);
    // 设置定时器的n值（定时的时间=定时器的n值/定时器频率）
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_TIMER_N_VALUE,
                                    10);

    // 开启定时中断
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_INT_TIE,
                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

    // 开启RTC
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_CLOCK_RTC,
                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

    Serial.printf("ID: %#X \n\n", (int32_t)PCF8563->IIC_Read_Device_ID());
    delay(1000);
}

void loop()
{
}