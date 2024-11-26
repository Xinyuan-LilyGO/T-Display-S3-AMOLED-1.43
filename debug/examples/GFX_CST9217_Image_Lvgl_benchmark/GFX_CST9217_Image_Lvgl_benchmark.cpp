/*
 * @Description: GFX屏幕显示+触摸切换图片
 * @version: V1.0.0
 * @Author: LILYGO_L
 * @Date: 2023-09-06 10:58:19
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-11-26 16:23:03
 * @License: GPL 3.0
 */
#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "Material_16Bit_466x466px.h"
#include "TouchDrvCST92xx.h"
#include "SensorWireHelper.h"
#include "lvgl.h"
#include "lv_demo_benchmark.h"

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

static uint8_t Image_Flag = 0;

volatile int8_t IIC_Interrupt_Flag;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
    LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

// DO0143FAT01
// Arduino_GFX *gfx = new Arduino_SH8601(bus, LCD_RST /* RST */,
//                                       0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);

// H0175Y003AM
Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RST /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT,
                                      6 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

TouchDrvCST92xx CST9217;

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    if (IIC_Interrupt_Flag == true)
    {
        IIC_Interrupt_Flag = false;

        int16_t touch_x[5];
        int16_t touch_y[5];
        uint8_t fingers_number = 0;

        if (CST9217.getPoint(touch_x, touch_y, 2) == true)
        {
            touch_x[0] = LCD_WIDTH - touch_x[0];
            touch_y[0] = LCD_HEIGHT - touch_y[0];

            data->state = LV_INDEV_STATE_PR;

            /*Set the coordinates*/
            data->point.x = touch_x[0];
            data->point.y = touch_y[0];
        }
        else
        {
            data->state = LV_INDEV_STATE_REL;
        }
    }
}

void my_rounder_cb(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    if (area->x1 % 2 != 0)
        area->x1 += 1;
    if (area->y1 % 2 != 0)
        area->y1 += 1;

    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    if (w % 2 != 0)
        area->x2 -= 1;
    if (h % 2 != 0)
        area->y2 -= 1;
}

void lvgl_initialization(void)
{
    lv_init();

    lv_color_t *buf_1 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * LCD_WIDTH * 40, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    lv_color_t *buf_2 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * LCD_WIDTH * 40, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);

    while ((!buf_1) || (!buf_2))
    {
        Serial.println("LVGL disp_draw_buf allocate failed!");
        delay(1000);
    }

    lv_disp_draw_buf_init(&draw_buf, buf_1, buf_2, LCD_WIDTH * 40);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.rounder_cb = my_rounder_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1; // 双缓冲全像素刷新
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Ciallo");

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);

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

    gfx->begin(80000000);
    gfx->fillScreen(WHITE);

    lvgl_initialization();

    for (int i = 0; i <= 255; i++)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }

    gfx->setCursor(100, 100);
    gfx->setTextSize(2);
    gfx->setTextColor(BLACK);
    gfx->printf("Ciallo");

    delay(1000);
}

void loop()
{
    // Serial.printf("System running time: %d\n\n", (uint32_t)millis() / 1000);

    if (IIC_Interrupt_Flag == true)
    {
        IIC_Interrupt_Flag = false;

        int16_t touch_x[5];
        int16_t touch_y[5];
        uint8_t fingers_number = 0;

        if (CST9217.getPoint(touch_x, touch_y, 2) == true)
        {
            delay(300);

            touch_x[0] = LCD_WIDTH - touch_x[0];
            touch_y[0] = LCD_HEIGHT - touch_y[0];

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
            case 4:

                lv_demo_benchmark();

                while (1)
                {
                    lv_timer_handler(); /* let the GUI do its work */
                                        // delay(5);
                }

                break;

            default:
                break;
            }

            Image_Flag++;

            if (Image_Flag > 4)
            {
                Image_Flag = 0;
            }

            gfx->setTextSize(2);
            gfx->setTextColor(MAROON);
            gfx->fillCircle(touch_x[0], touch_y[0], 3, RED);
            gfx->setCursor(touch_x[0], touch_y[0]);
            gfx->printf(" Touch X1:%d Y1:%d", touch_x[0], touch_y[0]);
        }
    }
}
