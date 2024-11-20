# 1 "C:\\Users\\16578\\AppData\\Local\\Temp\\tmpy0kc6m19"
#include <Arduino.h>
# 1 "D:/Information/Data/VS_Data/VSCode/PlatformIO-IDE/Arduino/ESP32-S3/16.T-Display-AMOLED-1.43/examples/Lvgl_benchmark/Lvgl_benchmark.ino"







#include "lvgl.h"
#include "lv_demo_benchmark.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include "TouchDrvCST92xx.h"
#include "SensorWireHelper.h"

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

volatile int8_t IIC_Interrupt_Flag;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS , LCD_SCLK , LCD_SDIO0 , LCD_SDIO1 ,
    LCD_SDIO2 , LCD_SDIO3 );






Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RST ,
                                      0 , false , LCD_WIDTH, LCD_HEIGHT,
                                      6 , 0 , 0 , 0 );

TouchDrvCST92xx CST9217;
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
void my_rounder_cb(lv_disp_drv_t *disp_drv, lv_area_t *area);
void lvgl_initialization(void);
void setup();
void loop();
#line 37 "D:/Information/Data/VS_Data/VSCode/PlatformIO-IDE/Arduino/ESP32-S3/16.T-Display-AMOLED-1.43/examples/Lvgl_benchmark/Lvgl_benchmark.ino"
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


    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.rounder_cb = my_rounder_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1;
    lv_disp_drv_register(&disp_drv);


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
    gfx->fillScreen(BLACK);

    lvgl_initialization();

    lv_demo_benchmark();

    for (int i = 0; i <= 255; i++)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }
}

void loop()
{
    lv_timer_handler();
    delay(5);
}