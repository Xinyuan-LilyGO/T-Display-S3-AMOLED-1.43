/***
 * Required libraries:
 * Arduino_GFX: https://github.com/moononournation/Arduino_GFX.git
 * libhelix: https://github.com/pschatzmann/arduino-libhelix.git
 * JPEGDEC: https://github.com/bitbank2/JPEGDEC.git
 */
#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <FS.h>
#include <SD.h>
#include "pin_config.h"

// #define MJPEG_FILENAME "/240x240px_60fps.mjpeg"
// #define FPS 60
// #define MJPEG_BUFFER_SIZE (240 * 240 * 2 / 10)

// #define MJPEG_FILENAME "/466x466px_60fps.mjpeg"
// #define FPS 60
// #define MJPEG_BUFFER_SIZE (466 * 466 * 2 / 10)

#define MJPEG_FILENAME "/450x450px_60fps.mjpeg"
#define FPS 20
#define MJPEG_BUFFER_SIZE (450 * 450 * 2 / 10)

#define CHART_MARGIN 64
#define LEGEND_A_COLOR 0x1BB6
#define LEGEND_B_COLOR 0xFBE1
#define LEGEND_C_COLOR 0x2D05
#define LEGEND_D_COLOR 0xD125
#define LEGEND_E_COLOR 0x9337
#define LEGEND_F_COLOR 0x8AA9
#define LEGEND_G_COLOR 0xE3B8
#define LEGEND_H_COLOR 0x7BEF
#define LEGEND_I_COLOR 0xBDE4
#define LEGEND_J_COLOR 0x15F9

/* variables */
static int next_frame = 0;
static int skipped_frames = 0;
static unsigned long start_ms, curr_ms, next_frame_ms;
static unsigned long total_read_video_ms = 0;
static unsigned long total_decode_video_ms = 0;
static unsigned long total_show_video_ms = 0;
static int time_used = 0;
static int total_frames = 0;

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */, LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

// DO0143FAT01
// Arduino_GFX *gfx = new Arduino_SH8601(bus, LCD_RST /* RST */,
//                                       0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT);

// H0175Y003AM
Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RST /* RST */,
                                      0 /* rotation */, false /* IPS */, LCD_WIDTH, LCD_HEIGHT,
                                      6 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

SPIClass SPI_2(HSPI);

/* MJPEG Video */
#include "MjpegClass.h"
static MjpegClass mjpeg;

// pixel drawing callback
static int drawMCU(JPEGDRAW *pDraw)
{
    // Serial.printf("Draw pos = (%d, %d), size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
    unsigned long s = millis();
    gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
    total_show_video_ms += millis() - s;
    return 1;
} /* drawMCU() */

void Mjpeg_Data_Statistics(void)
{
    int played_frames = total_frames - skipped_frames;
    float fps = 1000.0 * played_frames / time_used;
    total_decode_video_ms -= total_show_video_ms;
    Serial.printf("Played frames: %d\n", played_frames);
    Serial.printf("Skipped frames: %d (%0.1f %%)\n", skipped_frames, 100.0 * skipped_frames / total_frames);
    Serial.printf("Time used: %d ms\n", time_used);
    Serial.printf("Expected FPS: %d\n", FPS);
    Serial.printf("Actual FPS: %0.1f\n", fps);
    Serial.printf("Read video: %lu ms (%0.1f %%)\n", total_read_video_ms, 100.0 * total_read_video_ms / time_used);
    Serial.printf("Decode video: %lu ms (%0.1f %%)\n", total_decode_video_ms, 100.0 * total_decode_video_ms / time_used);
    Serial.printf("Show video: %lu ms (%0.1f %%)\n", total_show_video_ms, 100.0 * total_show_video_ms / time_used);

    // gfx->setCursor(0, 0);
    gfx->setTextColor(WHITE);
    gfx->printf("Played frames: %d\n", played_frames);
    gfx->printf("Skipped frames: %d (%0.1f %%)\n", skipped_frames, 100.0 * skipped_frames / total_frames);
    gfx->printf("Time used: %d ms\n", time_used);
    gfx->printf("Expected FPS: %d\n", FPS);
    gfx->printf("Actual FPS: %0.1f\n\n", fps);

    int16_t r1 = ((gfx->height() - CHART_MARGIN - CHART_MARGIN) / 2);
    int16_t r2 = r1 / 2;
    int16_t cx = gfx->width() - r1 - 10;
    int16_t cy = r1 + CHART_MARGIN;

    float arc_start3 = 0;
    float arc_end3 = arc_start3 + max(2.0, 360.0 * total_read_video_ms / time_used);
    for (int i = arc_start3 + 1; i < arc_end3; i += 2)
    {
        gfx->fillArc(cx, cy, r1, r2, arc_start3 - 90.0, i - 90.0, LEGEND_C_COLOR);
    }
    gfx->fillArc(cx, cy, r1, r2, arc_start3 - 90.0, arc_end3 - 90.0, LEGEND_C_COLOR);
    gfx->setTextColor(LEGEND_C_COLOR);
    gfx->printf("Read video: %lu ms (%0.1f %%)\n", total_read_video_ms, 100.0 * total_read_video_ms / time_used);

    float arc_start4 = arc_end3;
    float arc_end4 = arc_start4 + max(2.0, 360.0 * total_decode_video_ms / time_used);
    for (int i = arc_start4 + 1; i < arc_end4; i += 2)
    {
        gfx->fillArc(cx, cy, r1, r2, arc_start4 - 90.0, i - 90.0, LEGEND_D_COLOR);
    }
    gfx->fillArc(cx, cy, r1, r2, arc_start4 - 90.0, arc_end4 - 90.0, LEGEND_D_COLOR);
    gfx->setTextColor(LEGEND_D_COLOR);
    gfx->printf("Decode video: %lu ms (%0.1f %%)\n", total_decode_video_ms, 100.0 * total_decode_video_ms / time_used);

    float arc_start5 = arc_end4;
    float arc_end5 = arc_start5 + max(2.0, 360.0 * total_show_video_ms / time_used);
    for (int i = arc_start5 + 1; i < arc_end5; i += 2)
    {
        gfx->fillArc(cx, cy, r2, 0, arc_start5 - 90.0, i - 90.0, LEGEND_E_COLOR);
    }
    gfx->fillArc(cx, cy, r2, 0, arc_start5 - 90.0, arc_end5 - 90.0, LEGEND_E_COLOR);
    gfx->setTextColor(LEGEND_E_COLOR);
    gfx->printf("Show video: %lu ms (%0.1f %%)\n", total_show_video_ms, 100.0 * total_show_video_ms / time_used);
}

void setup()
{
    WiFi.mode(WIFI_OFF);
    Serial.begin(115200);

    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);

    gfx->begin(80000000);
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setCursor(0, LCD_WIDTH / 2);

    for (int i = 0; i <= 255; i++)
    {
        gfx->Display_Brightness(i);
        delay(3);
    }
}

void loop()
{
    /* variables */
    next_frame = 0;
    skipped_frames = 0;

    total_read_video_ms = 0;
    total_decode_video_ms = 0;
    total_show_video_ms = 0;
    time_used = 0;
    total_frames = 0;

    SPI_2.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS); // SPI boots
    while (!SD.begin(SD_CS, SPI_2, 80000000))      /* SPI bus mode */
    {
        Serial.println("ERROR: SD card mount failed!");
        gfx->println("ERROR: SD card mount failed!");
        delay(1000);
    }

    Serial.println("Init SD successful");
    // gfx->println("Init SD successful");

    // gfx->println("Open video file: " MJPEG_FILENAME);

    File vFile = SD.open(MJPEG_FILENAME);
    if (!vFile || vFile.isDirectory())
    {
        Serial.println("ERROR: Failed to open " MJPEG_FILENAME " file for reading");
        // gfx->println("ERROR: Failed to open " MJPEG_FILENAME " file for reading");

        return;
    }
    else
    {
        uint8_t *mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
        if (!mjpeg_buf)
        {
            Serial.println("mjpeg_buf malloc failed!");
            // gfx->println("mjpeg_buf malloc failed!");
        }
        else
        {
            // gfx->println("Init video");
            mjpeg.setup(
                &vFile, mjpeg_buf, drawMCU, true /* useBigEndian */,
                0 /* x */, 0 /* y */, 450 /* widthLimit */, 450 /* heightLimit */);

            // gfx->println("Start play video");
            start_ms = millis();
            curr_ms = millis();
            next_frame_ms = start_ms + (++next_frame * 1000 / FPS / 2);
            while (vFile.available() && mjpeg.readMjpegBuf()) // Read video
            {
                total_read_video_ms += millis() - curr_ms;
                curr_ms = millis();

                if (millis() < next_frame_ms) // check show frame or skip frame
                {
                    // Play video
                    mjpeg.drawJpg();
                    total_decode_video_ms += millis() - curr_ms;
                    curr_ms = millis();
                }
                else
                {
                    ++skipped_frames;
                    // Serial.println("Skip frame");
                }

                while (millis() < next_frame_ms)
                {
                    vTaskDelay(pdMS_TO_TICKS(1));
                }

                curr_ms = millis();
                next_frame_ms = start_ms + (++next_frame * 1000 / FPS);
            }
            time_used = millis() - start_ms;
            total_frames = next_frame - 1;
            Serial.println("MJPEG video end");

            vFile.close();

            free(mjpeg_buf);

            SD.end();
        }
    }
}
