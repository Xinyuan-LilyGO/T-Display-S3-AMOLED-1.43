/*
 * @Description(CN):
 *      该程序用于测试TQT-C6正常工作时的最高功耗
 *
 * @Description(EN):
 *      This program is used to test the maximum power consumption of
 *  TQT-C6 during normal operation.
 *
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-08-18 15:26:31
 * @LastEditors: LILYGO_L
 * @LastEditTime: 2024-04-23 13:50:49
 * @License: GPL 3.0
 */
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define WIFI_SSID "xinyuandianzi"
#define WIFI_PASSWORD "AA15994823428"
// #define WIFI_SSID "LilyGo-AABB"
// #define WIFI_PASSWORD "xinyuandianzi"

#define WIFI_CONNECT_WAIT_MAX 5000

#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define GMT_OFFSET_SEC 8 * 3600 // Time zone setting function, written as 8 * 3600 in East Eighth Zone (UTC/GMT+8:00)
#define DAY_LIGHT_OFFSET_SEC 0  // Fill in 3600 for daylight saving time, otherwise fill in 0

bool Wifi_Connection_State;
static size_t CycleTime = 0;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

Arduino_GFX *gfx = new Arduino_SH8601(bus, LCD_RST /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

// void Arduino_IIC_Touch_Interrupt(void);

// std::unique_ptr<Arduino_IIC> CST816T(new Arduino_CST816x(IIC_Bus, CST816T_DEVICE_ADDRESS,
//                                                          TP_RST, TP_INT, Arduino_IIC_Touch_Interrupt));

// std::unique_ptr<Arduino_IIC> ETA4662(new Arduino_ETA4662(IIC_Bus, ETA4662_DEVICE_ADDRESS,
//                                                          DRIVEBUS_DEFAULT_VALUE, DRIVEBUS_DEFAULT_VALUE));

std::unique_ptr<Arduino_IIC> SY6970(new Arduino_SY6970(IIC_Bus, SY6970_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, DRIVEBUS_DEFAULT_VALUE));

// void Arduino_IIC_Touch_Interrupt(void)
// {
//     CST816T->IIC_Interrupt_Flag = true;
// }

void WIFI_STA_Test_Loop(void)
{
    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 0);
    gfx->setTextSize(2);
    gfx->setTextColor(BLACK);

    String temp;
    int wifi_num = 0;
    uint64_t last_tick;

    temp = "Scanning wifi";
    gfx->print(temp);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    wifi_num = WiFi.scanNetworks();
    if (wifi_num == 0)
    {
        temp = "\nWiFi scan complete !\nNo wifi discovered.\n";
    }
    else
    {
        temp = "\nWiFi scan complete !\n";
        temp += wifi_num;
        temp += " wifi discovered.\n\n";

        for (int i = 0; i < wifi_num; i++)
        {
            temp += (i + 1);
            temp += ": ";
            temp += WiFi.SSID(i);
            temp += " (";
            temp += WiFi.RSSI(i);
            temp += ")";
            temp += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " \n" : "*\n";
            delay(10);
        }
    }

    gfx->print(temp);

    temp = "Connecting to ";
    temp += WIFI_SSID;
    temp += "\n";

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    last_tick = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        temp += ".";

        if (millis() - last_tick > WIFI_CONNECT_WAIT_MAX)
        {
            Wifi_Connection_State = false;
            break;
        }
        else
        {
            Wifi_Connection_State = true;
        }
        delay(500); // 这个延时必须要有
    }

    if (Wifi_Connection_State == true)
    {
        temp += "\nThe connection was successful ! \nTakes: ";
        temp += (String)(millis() - last_tick);
        temp += " ms";
    }
    else
    {
        temp += "\nThe connection was fail !";
    }

    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 0);
    gfx->print(temp);

    CycleTime = millis() + 3000;
}

void WIFI_Time(void)
{
    String temp;
    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 10);

    if (Wifi_Connection_State == true)
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo, 10000))
        {
            temp = "Failed to obtain time!";
        }
        else
        {
            temp = "Get time success";
            temp += "\nYear/Month: ";
            temp += (String)(timeinfo.tm_year + 1900);
            temp += "/";
            temp += (String)(timeinfo.tm_mon + 1);
            temp += "/";
            temp += (String)timeinfo.tm_mday;
            temp += "\nTime: ";
            temp += (String)timeinfo.tm_hour;
            temp += " : ";
            temp += (String)timeinfo.tm_min;
            temp += " : ";
            temp += (String)timeinfo.tm_sec;
        }
    }
    else
    {
        temp += "\n\nNot connected to the network";
    }

    gfx->print(temp);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);

    if (SY6970->begin() == false)
    {
        Serial.println("SY6970 initialization fail");
        delay(2000);
    }
    else
    {
        Serial.println("SY6970 initialization successfully");
    }

    // 开启ADC测量功能
    if (SY6970->IIC_Write_Device_State(SY6970->Arduino_IIC_Power::Device::POWER_DEVICE_ADC_MEASURE,
                                       SY6970->Arduino_IIC_Power::Device_State::POWER_DEVICE_ON) == false)
    {
        Serial.println("SY6970 ADC Measure ON fail");
        delay(2000);
    }
    else
    {
        Serial.println("SY6970 ADC Measure ON successfully");
    }
    // 禁用看门狗定时器喂狗功能
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_WATCHDOG_TIMER, 0);
    // 热调节阈值设置为60度
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_THERMAL_REGULATION_THRESHOLD, 60);
    // 充电目标电压电压设置为4224mV
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_CHARGING_TARGET_VOLTAGE_LIMIT, 4224);
    // 最小系统电压限制为3600mA
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_MINIMUM_SYSTEM_VOLTAGE_LIMIT, 3600);
    // 设置OTG电压为5062mV
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_OTG_VOLTAGE_LIMIT, 5062);
    // 输入电流限制设置为2100mA
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_INPUT_CURRENT_LIMIT, 2100);
    // 快速充电电流限制设置为2112mA
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_FAST_CHARGING_CURRENT_LIMIT, 2112);
    // 预充电电流限制设置为192mA
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_PRECHARGE_CHARGING_CURRENT_LIMIT, 192);
    // 终端充电电流限制设置为320mA
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_TERMINATION_CHARGING_CURRENT_LIMIT, 320);
    // OTG电流限制设置为500mA
    SY6970->IIC_Write_Device_Value(SY6970->Arduino_IIC_Power::Device_Value::POWER_DEVICE_OTG_CHARGING_LIMIT, 500);

    gfx->begin();
    gfx->fillScreen(WHITE);

    for (int i = 0; i <= 255; i++)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }

    WIFI_STA_Test_Loop();

    delay(2000);

    if (Wifi_Connection_State == true)
    {
        // Obtain and set the time from the network time server
        // After successful acquisition, the chip will use the RTC clock to update the holding time
        configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);

        delay(3000);
    }

    gfx->fillRect(0, 60, 128, 64, WHITE);
    gfx->setCursor(10, 60);
    gfx->setTextSize(2);
    gfx->setTextColor(MAGENTA);
}

void loop()
{
    if (millis() > CycleTime)
    {
        WIFI_STA_Test_Loop();
    }

    if (Wifi_Connection_State == true)
    {
        WIFI_Time();
    }
    else
    {
        gfx->fillRect(0, 0, 128, 60, WHITE);
        gfx->setCursor(0, 10);
        gfx->print("Wifi Connection Fail");
    }

    delay(1000);
}
