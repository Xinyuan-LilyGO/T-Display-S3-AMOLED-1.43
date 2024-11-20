/*
 * @Description: 出厂测试程序
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-09-06 10:58:19
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-11-04 15:54:47
 * @License: GPL 3.0
 */

#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "Wire.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include "Arduino_DriveBus_Library.h"
#include "Material_16Bit_466x466px.h"
#include <SD.h>
#include "TouchDrvCST92xx.h"

// 44.1 KHz
#define IIS_SAMPLE_RATE 44100 // 采样速率
#define IIS_DATA_BIT 16       // 数据位数

#define WIFI_SSID "xinyuandianzi"
#define WIFI_PASSWORD "AA15994823428"
// #define WIFI_SSID "LilyGo-AABB"
// #define WIFI_PASSWORD "xinyuandianzi"

#define WIFI_CONNECT_WAIT_MAX (5000)

#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define GMT_OFFSET_SEC 8 * 3600 // Time zone setting function, written as 8 * 3600 in East Eighth Zone (UTC/GMT+8:00)
#define DAY_LIGHT_OFFSET_SEC 0  // Fill in 3600 for daylight saving time, otherwise fill in 0

bool Wifi_Connection_Failure_Flag = false;

volatile int8_t IIC_Interrupt_Flag;

static size_t CycleTime = 0;
static size_t CycleTime_2 = 0;

static uint8_t Image_Flag = 0;

uint8_t OTG_Mode = 0;

SPIClass SPI_2(HSPI);

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

// H0175Y003AMV0
Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RST /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT,
                                      6 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::unique_ptr<Arduino_IIC> SY6970(new Arduino_SY6970(IIC_Bus, SY6970_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, DRIVEBUS_DEFAULT_VALUE));

std::unique_ptr<Arduino_IIC> PCF8563(new Arduino_PCF8563(IIC_Bus, PCF8563_DEVICE_ADDRESS,
                                                         DRIVEBUS_DEFAULT_VALUE, DRIVEBUS_DEFAULT_VALUE));

TouchDrvCST92xx CST9217;

void Wifi_STA_Test(void)
{
    String text;
    int wifi_num = 0;

    gfx->fillScreen(BLACK);
    gfx->setCursor(0, 0);
    gfx->setTextSize(2);
    gfx->setTextColor(GREEN);

    Serial.println("\nScanning wifi");
    gfx->printf("\n\n\nScanning wifi\n");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    wifi_num = WiFi.scanNetworks();
    if (wifi_num == 0)
    {
        text = "\nWiFi scan complete !\nNo wifi discovered.\n";
    }
    else
    {
        text = "\nWiFi scan complete !\n";
        text += wifi_num;
        text += " wifi discovered.\n\n";

        for (int i = 0; i < wifi_num; i++)
        {
            text += (i + 1);
            text += ": ";
            text += WiFi.SSID(i);
            text += " (";
            text += WiFi.RSSI(i);
            text += ")";
            text += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " \n" : "*\n";
            delay(10);
        }
    }

    Serial.println(text);
    gfx->println(text);

    delay(3000);
    text.clear();
    gfx->fillScreen(BLACK);
    gfx->setCursor(0, 50);

    Serial.print("Connecting to ");
    gfx->printf("Connecting to\n");

    Serial.print(WIFI_SSID);
    gfx->printf("%s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t last_tick = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        gfx->printf(".");
        delay(100);

        if (millis() - last_tick > WIFI_CONNECT_WAIT_MAX)
        {
            Wifi_Connection_Failure_Flag = true;
            break;
        }
    }

    if (!Wifi_Connection_Failure_Flag)
    {
        Serial.print("\nThe connection was successful ! \nTakes ");
        gfx->printf("\nThe connection was successful ! \nTakes ");
        Serial.print(millis() - last_tick);
        gfx->print(millis() - last_tick);
        Serial.println(" ms\n");
        gfx->printf(" ms\n");

        gfx->setTextColor(GREEN);
        gfx->printf("\nWifi test passed!");
    }
    else
    {
        gfx->setTextColor(RED);
        gfx->printf("\nWifi test error!\n");
    }
}

void PrintLocalTime(void)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 3000))
    {
        Serial.println("Failed to obtain time");
        gfx->setCursor(100, 200);
        gfx->setTextColor(RED);
        gfx->print("Failed to obtain time!");
        return;
    }
    Serial.println("Get time success");
    Serial.println(&timeinfo, "%A,%B %d %Y %H:%M:%S"); // Format Output
    gfx->setCursor(100, 200);
    gfx->setTextColor(ORANGE);
    gfx->print(&timeinfo, " %Y");
    gfx->setCursor(100, 220);
    gfx->print(&timeinfo, "%B %d");
    gfx->setCursor(100, 240);
    gfx->print(&timeinfo, "%H:%M:%S");
}

void GFX_Print_Battery_Info_Loop()
{
    gfx->fillRect(0, 0, LCD_WIDTH, 233, WHITE);
    gfx->setTextSize(2);
    gfx->setTextColor(BLACK);

    gfx->setCursor(50, 100);
    gfx->printf("Battery ADC ");

    gfx->setCursor(50, 120);
    gfx->printf("Charging Status: ");
    gfx->setCursor(50, 140);
    gfx->setTextColor(ORANGE);
    gfx->printf("%s ",
                (SY6970->IIC_Read_Device_State(SY6970->Arduino_IIC_Power::Status_Information::POWER_CHARGING_STATUS)).c_str());
    gfx->setTextColor(BLACK);

    gfx->setCursor(50, 160);
    gfx->printf("SY6970: %d mV",
                (uint32_t)SY6970->IIC_Read_Device_Value(SY6970->Arduino_IIC_Power::Value_Information::POWER_BATTERY_VOLTAGE));
    gfx->setCursor(50, 180);
    gfx->printf("Hardware: %d mV", analogReadMilliVolts(BATTERY_VOLTAGE_ADC_DATA) * 2);
}

void GFX_Print_RTC_Info_Loop()
{
    gfx->fillRect(0, 0, LCD_WIDTH, 200, WHITE);
    gfx->setTextSize(2);
    gfx->setTextColor(BLACK);

    gfx->setCursor(50, 100);
    gfx->printf("RTC Information");

    gfx->setCursor(50, 120);
    gfx->printf("PCF8563  Weekday: %s\n",
                PCF8563->IIC_Read_Device_State(PCF8563->Arduino_IIC_RTC::Status_Information::RTC_WEEKDAYS_DATA).c_str());
    gfx->setCursor(50, 140);
    gfx->printf("PCF8563  Year: %d\n",
                (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_YEARS_DATA));
    gfx->setCursor(50, 160);
    gfx->printf("PCF8563 Date: %d.%d\n",
                (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_MONTHS_DATA),
                (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_DAYS_DATA));
    gfx->setCursor(50, 180);
    gfx->printf("PCF8563 Time: %d:%d:%d\n",
                (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_HOURS_DATA),
                (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_MINUTES_DATA),
                (int32_t)PCF8563->IIC_Read_Device_Value(PCF8563->Arduino_IIC_RTC::Value_Information::RTC_SECONDS_DATA));
}

void GFX_Print_SD_Info_Loop()
{
    gfx->fillRect(0, 0, LCD_WIDTH, 200, WHITE);
    gfx->setTextSize(2);
    gfx->setTextColor(BLACK);

    gfx->setCursor(50, 100);
    gfx->printf("SD Information");

    gfx->setCursor(50, 120);

    if (!SD.begin(SD_CS, SPI_2, 40000000)) // SD boots
    {
        gfx->print("SD card initialization failed !");
    }
    else
    {
        gfx->print("SD card initialization successful !");

        uint8_t cardType = 0;
        uint64_t cardSize = 0;
        uint8_t numSectors = 0;

        cardType = SD.cardType();
        cardSize = SD.cardSize() / (1024 * 1024);
        numSectors = SD.numSectors();

        gfx->setCursor(50, 140);
        switch (cardType)
        {
        case CARD_NONE:
            gfx->print("No SD card attached");
            break;
        case CARD_MMC:
            gfx->print("SD Card Type: MMC");
            break;
        case CARD_SD:
            gfx->print("SD Card Type: SDSC");
            break;
        case CARD_SDHC:
            gfx->print("SD Card Type: SDHC");
            break;
        default:
            gfx->println("UNKNOWN");
            break;
        }

        gfx->setCursor(50, 160);
        gfx->printf("SD Card Size: %lluMB\n", cardSize);
    }
    SD.end();
}

void GFX_Print_OTG_Switch_Info(uint8_t mode)
{
    switch (mode)
    {
    case 0:
        gfx->fillRect(150, 150, 166, 60, DARKGREY);
        gfx->setTextSize(2);
        gfx->setTextColor(WHITE);

        gfx->setCursor(190, 170);
        gfx->printf("OTG OFF");

        SY6970->IIC_Write_Device_State(SY6970->Arduino_IIC_Power::Device::POWER_DEVICE_OTG_MODE,
                                       SY6970->Arduino_IIC_Power::Device_State::POWER_DEVICE_OFF);
        break;

    case 1:
        gfx->fillRect(150, 150, 166, 60, OLIVE);
        gfx->setTextSize(2);
        gfx->setTextColor(WHITE);

        gfx->setCursor(190, 170);
        gfx->printf("OTG ON");

        SY6970->IIC_Write_Device_State(SY6970->Arduino_IIC_Power::Device::POWER_DEVICE_OTG_MODE,
                                       SY6970->Arduino_IIC_Power::Device_State::POWER_DEVICE_ON);
        break;

    default:
        break;
    }
}

void GFX_Print_RTC_Switch_Info()
{
    gfx->fillRect(150, 210, 166, 60, PINK);
    gfx->drawRect(150, 210, 166, 60, RED);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);

    gfx->setCursor(170, 230);
    gfx->printf("RTC Reset");
}

void GFX_Print_Touch_Info_Loop(int32_t touch_x, int32_t touch_y, int32_t fingers_number)
{
    gfx->fillRect(0, 0, 416, 200, WHITE);
    gfx->setTextSize(2);
    gfx->setTextColor(BLACK);

    // gfx->setCursor(100, 50);
    // gfx->printf("ID: %#X ", (int32_t)FT3168->IIC_Read_Device_ID());

    gfx->setCursor(100, 70);
    gfx->printf("Fingers Number:%d ", fingers_number);

    gfx->setCursor(100, 90);
    gfx->printf("Touch X:%d Y:%d ", touch_x, touch_y);
}

void GFX_Print_Voice_Speaker_Info_Loop(int16_t left_channel, int16_t right_channel)
{
    gfx->fillRect(30, 30, 130, 30, WHITE);
    gfx->setTextSize(1);
    gfx->setTextColor(BLACK);

    gfx->setCursor(30, 30);
    gfx->printf("Voice Data:");

    gfx->setCursor(30, 40);
    gfx->printf("Left Channel:%d ", left_channel);

    gfx->setCursor(30, 50);
    gfx->printf("Right Channel:%d", right_channel);
}

void GFX_Print_Time_Info_Loop()
{
    gfx->fillRect(130, 30, 206, 120, WHITE);

    if (!Wifi_Connection_Failure_Flag)
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo, 3000))
        {
            Serial.println("Failed to obtain time");
            gfx->setCursor(140, 45);
            gfx->setTextColor(RED);
            gfx->setTextSize(2);
            gfx->print("Time error");
            return;
        }
        Serial.println("Get time success");
        Serial.println(&timeinfo, "%A,%B %d %Y %H:%M:%S"); // Format Output
        gfx->setCursor(140, 45);
        gfx->setTextColor(ORANGE);
        gfx->setTextSize(2);
        gfx->print(&timeinfo, " %Y");
        gfx->setCursor(140, 65);
        gfx->print(&timeinfo, "%B %d");
        gfx->setCursor(140, 85);
        gfx->print(&timeinfo, "%H:%M:%S");
    }
    else
    {
        gfx->setCursor(140, 45);
        gfx->setTextSize(2);
        gfx->setTextColor(RED);
        gfx->print("Network error");
    }

    gfx->setCursor(140, 105);
    gfx->printf("SYS Time:%d", (uint32_t)millis() / 1000);
}

void GFX_Print_1()
{
    gfx->fillRect(80, 300, 143, 60, ORANGE);
    gfx->drawRect(80, 300, 143, 60, PURPLE);
    gfx->fillRect(243, 300, 143, 60, PURPLE);
    gfx->drawRect(243, 300, 143, 60, ORANGE);
    gfx->setTextSize(2);
    gfx->setTextColor(WHITE);
    gfx->setCursor(100, 320);
    gfx->printf("Try Again");
    gfx->setCursor(263, 320);
    gfx->printf("Next Test");
}

void GFX_Print_TEST(String s)
{
    gfx->fillScreen(WHITE);
    gfx->setCursor(190, 100);
    gfx->setTextSize(4);
    gfx->setTextColor(PINK);
    gfx->printf("TEST");

    gfx->setCursor(30, 150);
    gfx->setTextSize(2);
    gfx->setTextColor(BLACK);
    gfx->print(s);

    gfx->setCursor(210, 250);
    gfx->setTextSize(6);
    gfx->setTextColor(RED);
    gfx->printf("3");
    delay(1000);
    gfx->fillRect(210, 250, 100, 60, WHITE);
    gfx->setCursor(210, 250);
    gfx->printf("2");
    delay(1000);
    gfx->fillRect(210, 250, 100, 60, WHITE);
    gfx->setCursor(210, 250);
    gfx->printf("1");
    delay(1000);
}

void GFX_Print_FINISH()
{
    gfx->setCursor(160, 100);
    gfx->setTextSize(4);
    gfx->setTextColor(ORANGE);
    gfx->printf("FINISH");
}

void GFX_Print_START()
{
    gfx->setCursor(150, 220);
    gfx->setTextSize(6);
    gfx->setTextColor(RED);
    gfx->printf("START");
}

void Original_Test_1()
{
    GFX_Print_TEST("1.Touch Test");

    gfx->fillScreen(WHITE);

    int16_t temp_touch_x[5];
    int16_t temp_touch_y[5];
    uint8_t fingers_number = 1;

    CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

    int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
    int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

    GFX_Print_Touch_Info_Loop(touch_x, touch_y, fingers_number);

    GFX_Print_1();
}

void Original_Test_2()
{
    GFX_Print_TEST("2.LCD Edge Detection Test");

    gfx->fillScreen(WHITE);
    gfx->drawRect(1, 1, LCD_WIDTH - 2, LCD_HEIGHT - 2, RED);

    GFX_Print_FINISH();

    GFX_Print_1();
}

void Original_Test_3()
{
    GFX_Print_TEST("3.OLED Backlight Test");

    gfx->fillScreen(WHITE);

    GFX_Print_START();

    for (int i = 255; i > 0; i--)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }
    delay(3000);
    for (int i = 0; i <= 255; i++)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }

    delay(1000);

    gfx->fillScreen(WHITE);

    GFX_Print_FINISH();

    GFX_Print_1();
}

void Original_Test_4()
{
    GFX_Print_TEST("4.OLED Color Test");

    gfx->fillScreen(RED);
    delay(3000);
    gfx->fillScreen(GREEN);
    delay(3000);
    gfx->fillScreen(BLUE);
    delay(3000);

    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_1, LCD_WIDTH, LCD_HEIGHT);
    delay(3000);

    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)gImage_2, LCD_WIDTH, LCD_HEIGHT);
    delay(3000);

    GFX_Print_FINISH();

    GFX_Print_1();
}

void Original_Test_5()
{
    GFX_Print_TEST("5.OTG Test");

    gfx->fillScreen(WHITE);

    GFX_Print_OTG_Switch_Info(0);
    GFX_Print_1();
}

void Original_Test_6()
{
    GFX_Print_TEST("6.Battery Voltage Detection Test");

    // OTG关
    SY6970->IIC_Write_Device_State(SY6970->Arduino_IIC_Power::Device::POWER_DEVICE_OTG_MODE,
                                   SY6970->Arduino_IIC_Power::Device_State::POWER_DEVICE_OFF);

    // 充电开
    SY6970->IIC_Write_Device_State(SY6970->Arduino_IIC_Power::Device::POWER_DEVICE_CHARGING_MODE,
                                   SY6970->Arduino_IIC_Power::Device_State::POWER_DEVICE_ON);

    gfx->fillScreen(WHITE);

    GFX_Print_1();
}

void Original_Test_7()
{
    GFX_Print_TEST("7.RTC Test");

    gfx->fillScreen(WHITE);

    GFX_Print_RTC_Switch_Info();

    GFX_Print_1();
}

void Original_Test_8()
{
    GFX_Print_TEST("8.SD Test");

    gfx->fillScreen(WHITE);

    GFX_Print_1();
}

void Original_Test_9()
{
    GFX_Print_TEST("9.WIFI STA Test");

    Wifi_STA_Test();

    delay(2000);

    if (!Wifi_Connection_Failure_Flag)
    {
        // Obtain and set the time from the network time server
        // After successful acquisition, the chip will use the RTC clock to update the holding time
        configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);

        delay(3000);

        PrintLocalTime();
    }
    else
    {
        gfx->setCursor(20, 200);
        gfx->setTextColor(RED);
        gfx->print("Not connected to the network");
    }
    delay(5000);

    gfx->fillScreen(WHITE);

    GFX_Print_FINISH();

    GFX_Print_1();
}

void Original_Test_Loop()
{
    Original_Test_1();

    while (1)
    {
        bool temp = false;

        if (IIC_Interrupt_Flag == true)
        {

            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];
            uint8_t fingers_number = 1;

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

            GFX_Print_Touch_Info_Loop(touch_x, touch_y, fingers_number);

            if (fingers_number > 0)
            {
                if (touch_x > 80 && touch_x < 223 && touch_y > 300 && touch_y < 360)
                {
                    Original_Test_1();
                }
                if (touch_x > 243 && touch_x < 386 && touch_y > 300 && touch_y < 360)
                {
                    temp = true;
                }
            }

            delay(300);
            IIC_Interrupt_Flag = false;
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_2();

    while (1)
    {
        bool temp = false;

        if (IIC_Interrupt_Flag == true)
        {
            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];
            uint8_t fingers_number = 1;

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

            if (fingers_number > 0)
            {
                if (touch_x > 80 && touch_x < 223 && touch_y > 300 && touch_y < 360)
                {
                    Original_Test_2();
                }
                if (touch_x > 243 && touch_x < 386 && touch_y > 300 && touch_y < 360)
                {
                    temp = true;
                }
            }

            delay(300);
            IIC_Interrupt_Flag = false;
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_3();

    while (1)
    {
        bool temp = false;

        if (IIC_Interrupt_Flag == true)
        {
            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];
            uint8_t fingers_number = 1;

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

            if (fingers_number > 0)
            {
                if (touch_x > 80 && touch_x < 223 && touch_y > 300 && touch_y < 360)
                {
                    Original_Test_3();
                }
                if (touch_x > 243 && touch_x < 386 && touch_y > 300 && touch_y < 360)
                {
                    temp = true;
                }
            }

            delay(300);
            IIC_Interrupt_Flag = false;
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_4();

    while (1)
    {
        bool temp = false;

        if (IIC_Interrupt_Flag == true)
        {
            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];
            uint8_t fingers_number = 1;

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

            if (fingers_number > 0)
            {
                if (touch_x > 80 && touch_x < 223 && touch_y > 300 && touch_y < 360)
                {
                    Original_Test_4();
                }
                if (touch_x > 243 && touch_x < 386 && touch_y > 300 && touch_y < 360)
                {
                    temp = true;
                }
            }

            delay(300);
            IIC_Interrupt_Flag = false;
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_5();

    while (1)
    {
        bool temp = false;

        if (IIC_Interrupt_Flag == true)
        {
            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];
            uint8_t fingers_number = 1;

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

            if (fingers_number > 0)
            {
                if (touch_x > 80 && touch_x < 223 && touch_y > 300 && touch_y < 360)
                {
                    Original_Test_5();
                }
                if (touch_x > 243 && touch_x < 386 && touch_y > 300 && touch_y < 360)
                {
                    temp = true;
                }
                if (touch_x > 150 && touch_x < 316 && touch_y > 150 && touch_y < 210)
                {
                    OTG_Mode = !OTG_Mode;
                    GFX_Print_OTG_Switch_Info(OTG_Mode);
                    delay(300);
                }
            }

            delay(300);
            IIC_Interrupt_Flag = false;
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_6();

    while (1)
    {
        bool temp = false;

        if (millis() > CycleTime)
        {
            GFX_Print_Battery_Info_Loop();
            CycleTime = millis() + 1000;
        }

        if (IIC_Interrupt_Flag == true)
        {
            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];
            uint8_t fingers_number = 1;

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

            if (fingers_number > 0)
            {
                if (touch_x > 80 && touch_x < 223 && touch_y > 300 && touch_y < 360)
                {
                    Original_Test_6();
                }
                if (touch_x > 243 && touch_x < 386 && touch_y > 300 && touch_y < 360)
                {
                    temp = true;
                }
            }

            delay(300);
            IIC_Interrupt_Flag = false;
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_7();

    while (1)
    {
        bool temp = false;

        if (millis() > CycleTime)
        {
            GFX_Print_RTC_Info_Loop();
            CycleTime = millis() + 1000;
        }

        if (IIC_Interrupt_Flag == true)
        {
            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];
            uint8_t fingers_number = 1;

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

            if (fingers_number > 0)
            {
                if (touch_x > 80 && touch_x < 223 && touch_y > 300 && touch_y < 360)
                {
                    Original_Test_7();
                }
                if (touch_x > 243 && touch_x < 386 && touch_y > 300 && touch_y < 360)
                {
                    temp = true;
                }
                if (touch_x > 150 && touch_x < 316 && touch_y > 210 && touch_y < 270)
                {
                    // 关闭RTC
                    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_CLOCK_RTC,
                                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_OFF);
                    // 时钟传感器设置秒
                    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_SECOND_DATA,
                                                    58);
                    // 时钟传感器设置分
                    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_MINUTE_DATA,
                                                    59);
                    // 时钟传感器设置时
                    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_HOUR_DATA,
                                                    23);
                    // 时钟传感器设置天
                    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_DAY_DATA,
                                                    31);
                    // 时钟传感器设置月
                    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_MONTH_DATA,
                                                    12);
                    // 时钟传感器设置
                    PCF8563->IIC_Write_Device_Value(PCF8563->Arduino_IIC_RTC::Device_Value::RTC_SET_YEAR_DATA,
                                                    99);
                    // 开启RTC
                    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_CLOCK_RTC,
                                                    PCF8563->Arduino_IIC_RTC::Device_State::RTC_DEVICE_ON);

                    delay(300);
                }
            }

            delay(300);
            IIC_Interrupt_Flag = false;
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_8();

    while (1)
    {
        bool temp = false;

        if (millis() > CycleTime)
        {
            GFX_Print_SD_Info_Loop();
            CycleTime = millis() + 1000;
        }

        if (IIC_Interrupt_Flag == true)
        {
            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];
            uint8_t fingers_number = 1;

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

            if (fingers_number > 0)
            {
                if (touch_x > 80 && touch_x < 223 && touch_y > 300 && touch_y < 360)
                {
                    Original_Test_8();
                }
                if (touch_x > 243 && touch_x < 386 && touch_y > 300 && touch_y < 360)
                {
                    temp = true;
                }
            }

            delay(300);
            IIC_Interrupt_Flag = false;
        }

        if (temp == true)
        {
            break;
        }
    }

    Original_Test_9();

    while (1)
    {
        bool temp = false;

        if (IIC_Interrupt_Flag == true)
        {
            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];
            uint8_t fingers_number = 1;

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

            if (fingers_number > 0)
            {
                if (touch_x > 80 && touch_x < 223 && touch_y > 300 && touch_y < 360)
                {
                    Original_Test_9();
                }
                if (touch_x > 243 && touch_x < 386 && touch_y > 300 && touch_y < 360)
                {
                    temp = true;
                }
            }

            delay(300);
            IIC_Interrupt_Flag = false;
        }

        if (temp == true)
        {
            break;
        }
    }
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

    attachInterrupt(TP_INT, []()
                    { IIC_Interrupt_Flag = true; }, FALLING);

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

    if (PCF8563->begin() == false)
    {
        Serial.println("PCF8563 initialization fail");
        delay(2000);
    }
    else
    {
        Serial.println("PCF8563 initialization successfully");
    }

    // 关闭时钟输出
    PCF8563->IIC_Write_Device_State(PCF8563->Arduino_IIC_RTC::Device::RTC_CLOCK_OUTPUT_VALUE,
                                    PCF8563->Arduino_IIC_RTC::Device_Mode::RTC_CLOCK_OUTPUT_OFF);

    gfx->begin();
    gfx->fillScreen(WHITE);

    for (int i = 0; i <= 255; i++)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }

    SPI_2.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS); // SPI boots

    Original_Test_Loop();

    gfx->fillScreen(PINK);
}

void loop()
{
    if (millis() > CycleTime_2)
    {
        GFX_Print_Time_Info_Loop();
        CycleTime_2 = millis() + 1000;
    }

    if (IIC_Interrupt_Flag == true)
    {
        uint8_t fingers_number = 1;

        if (fingers_number > 0)
        {
            int16_t temp_touch_x[5];
            int16_t temp_touch_y[5];

            CST9217.getPoint(temp_touch_x, temp_touch_y, 1);

            int32_t touch_x = LCD_WIDTH - temp_touch_x[0];
            int32_t touch_y = LCD_HEIGHT - temp_touch_y[0];

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
                gfx->setCursor(150, 200);
                gfx->setTextColor(YELLOW);
                gfx->setTextSize(2);
                gfx->println("Ciallo1~(L *##*L)^**");
                break;

            default:
                break;
            }

            Image_Flag++;

            if (Image_Flag > 2)
            {
                Image_Flag = 0;
            }

            Serial.printf("[1] point x: %d  point y: %d \r\n", touch_x, touch_y);

            gfx->setTextSize(2);
            gfx->setCursor(touch_x, touch_y);
            gfx->setTextColor(RED);
            gfx->printf("[1] point x: %d  point y: %d \r\n", touch_x, touch_y);
        }

        delay(300);
        IIC_Interrupt_Flag = false;
    }
}
