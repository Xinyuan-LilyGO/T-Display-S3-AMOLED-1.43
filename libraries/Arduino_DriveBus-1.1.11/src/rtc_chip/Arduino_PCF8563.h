/*
 * @Description(CN):
 *      基于Arduino_DriveBus库的PCF8563芯片库
 *      编写了主要功能 部分功能未列出
 *      以下是操作PCF8563的相关枚举（控制PCF8563芯片时请调用以下枚举的参数）：
 *
 * @version: V1.0.0
 * @Author: Xk_w
 * @Date: 2023-11-16 15:42:22
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-04-05 08:55:43
 * @License: GPL 3.0
 */
#pragma once

#include "../Arduino_IIC.h"

#define PCF8563_DEVICE_ADDRESS 0x51

#define PCF8563_RD_WR_DEVICE_00 0x00
#define PCF8563_RD_WR_DEVICE_02 0x02
#define PCF8563_RD_WR_DEVICE_03 0x03
#define PCF8563_RD_WR_DEVICE_04 0x04
#define PCF8563_RD_WR_DEVICE_05 0x05
#define PCF8563_RD_WR_DEVICE_06 0x06
#define PCF8563_RD_WR_DEVICE_07 0x07
#define PCF8563_RD_WR_DEVICE_08 0x08
#define PCF8563_RD_WR_DEVICE_0A 0x0A
#define PCF8563_RD_DEVICE_0B 0x0B
#define PCF8563_RD_DEVICE_0C 0x0C
#define PCF8563_RD_DEVICE_0E 0x0E
#define PCF8563_RD_DEVICE_0F 0x0F
#define PCF8563_RD_DEVICE_10 0x10
#define PCF8563_RD_DEVICE_11 0x11
#define PCF8563_RD_DEVICE_12 0x12
#define PCF8563_RD_DEVICE_13 0x13

#define PCF8563_RD_WR_CONTROL_STATUS_1 0x00
#define PCF8563_RD_WR_CONTROL_STATUS_2 0x01
#define PCF8563_RD_WR_VL_SECONDS 0x02
#define PCF8563_RD_WR_MINUTES 0x03
#define PCF8563_RD_WR_HOURS 0x04
#define PCF8563_RD_WR_DAYS 0x05
#define PCF8563_RD_WR_WEEKDAY 0x06
#define PCF8563_RD_WR_CENTURY_MONTHS 0x07
#define PCF8563_RD_WR_YEARS 0x08
#define PCF8563_RD_WR_MINUTE_ALARM 0x09
#define PCF8563_RD_WR_HOUR_ALARM 0x0A
#define PCF8563_RD_WR_DAY_ALARM 0x0B
#define PCF8563_RD_WR_WEEKDAY_ALARM 0x0C
#define PCF8563_RD_WR_CLKOUT_CONTROL 0x0D
#define PCF8563_RD_WR_TIMER_CONTROL 0x0E
#define PCF8563_RD_WR_TIMER 0x0F

#define PCF8563_RD_DEVICE_ID 0x00 // Device ID Register

static const uint8_t PCF8563_Initialization_BufferOperations[] = {
    // BO_BEGIN_TRANSMISSION,
    // BO_WRITE_C8_D8, PCF8563_RD_WR_DEVICE_02, 0B11011101, // 开启ADC测量功能
    // BO_END_TRANSMISSION,

    BO_DELAY, 100};

class Arduino_PCF8563 : public Arduino_IIC
{
public:
    Arduino_PCF8563(std::shared_ptr<Arduino_IIC_DriveBus> bus, uint8_t device_address,
                    int8_t rst = DRIVEBUS_DEFAULT_VALUE, int8_t iqr = DRIVEBUS_DEFAULT_VALUE,
                    void (*Interrupt_Function)() = nullptr);

    bool begin(int32_t speed = DRIVEBUS_DEFAULT_VALUE) override;
    int32_t IIC_Read_Device_ID(void) override;
    bool IIC_Write_Device_State(uint32_t device, uint8_t state) override;
    bool IIC_Write_Device_Value(uint32_t device, int64_t value) override;
    String IIC_Read_Device_State(uint32_t information) override;
    double IIC_Read_Device_Value(uint32_t information) override;

protected:
    bool IIC_Initialization(void) override;
};