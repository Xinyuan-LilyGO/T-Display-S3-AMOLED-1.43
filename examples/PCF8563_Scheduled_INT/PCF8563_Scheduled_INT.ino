/*
 * @Description: PCF8567计划中断示例程序
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2024-03-27 10:02:26
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-05-29 10:46:40
 * @License: GPL 3.0
 */
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"

static size_t CycleTime = 0;

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> PCF8563(new Arduino_PCF8563(IIC_Bus, PCF8563_DEVICE_ADDRESS,
                                                         DRIVEBUS_DEFAULT_VALUE, PCF8563_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void)
{
    PCF8563->IIC_Interrupt_Flag = true;
}

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

    Serial.printf("ID: %#X \n\n", (int32_t)PCF8563->IIC_Read_Device_ID());

    // 关闭RTC
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_CLOCK_RTC,
                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_OFF);

    // 清除定时器标志AF
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_TIMER_FLAG_AF,
                                    0);

    // 时钟传感器设置秒
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_SECONDS_DATA,
                                    50);
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

    // 设置分钟计划报警的值为00报警
    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_ALARM_MINUTES_DATA,
                                    0);
    // 开启分钟计划报警器
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_AE_MINUTE_ALARM,
                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

    // 设置小时计划报警的值为00报警
    // PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_ALARM_HOUR_DATA,
    //                                 0);
    // // 开启分钟计划报警器
    // PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_AE_HOUR_ALARM,
    //                                 PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

    // 设置天计划报警的值为01报警
    // PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_ALARM_DAY_DATA,
    //                                 1);
    // // 开启分钟计划报警器
    // PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_AE_DAY_ALARM,
    //                                 PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

    // 开启计划报警中断
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_INT_AIE,
                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

    // 开启RTC
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_CLOCK_RTC,
                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

    Serial.println("Timer Start");
}

void loop()
{
    if (millis() > CycleTime)
    {
        Serial.printf("--------------------PCF8563--------------------\n");

        Serial.printf("System running time: %d\n\n", (uint32_t)millis() / 1000);
        Serial.printf("IIC_Bus.use_count(): %d\n\n", (int32_t)IIC_Bus.use_count());

        Serial.printf("ID: %#X \n", (int32_t)PCF8563->IIC_Read_Device_ID());

        Serial.printf("PCF8563  Weekday: %s\n",
                      PCF8563->IIC_Read_Device_State(PCF8563->Arduino_IIC_RTC::Status_Information::RTC_WEEKDAYS_DATA).c_str());
        Serial.printf("PCF8563  Year: %d\n",
                      (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_YEARS_DATA));
        Serial.printf("PCF8563 Date: %d.%d\n",
                      (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_MONTHS_DATA),
                      (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_DAYS_DATA));
        Serial.printf("PCF8563 Time: %d:%d:%d\n",
                      (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_HOURS_DATA),
                      (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_MINUTES_DATA),
                      (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_SECONDS_DATA));

        Serial.printf("--------------------PCF8563--------------------\n");

        CycleTime = millis() + 1000;
    }

    if (PCF8563->IIC_Interrupt_Flag == true)
    {
        PCF8563->IIC_Interrupt_Flag = false;

        Serial.println("Flag triggered 1");
        delay(1000);
    }

    if ((int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_ALARM_FLAG_AF_INFORMATION) == 1)
    {
        Serial.println("Flag triggered 2");

        // 清除定时器标志TF
        PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_TIMER_FLAG_AF,
                                        0);
        delay(1000);
    }
}