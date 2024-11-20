# 1 "C:\\Users\\16578\\AppData\\Local\\Temp\\tmpf_jfnhip"
#include <Arduino.h>
# 1 "D:/Information/Data/VS_Data/VSCode/PlatformIO-IDE/Arduino/ESP32-S3/16.T-Display-AMOLED-1.43/examples/TouchDrv_CST9217_GetPoint/TouchDrv_CST9217_GetPoint.ino"
# 30 "D:/Information/Data/VS_Data/VSCode/PlatformIO-IDE/Arduino/ESP32-S3/16.T-Display-AMOLED-1.43/examples/TouchDrv_CST9217_GetPoint/TouchDrv_CST9217_GetPoint.ino"
#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include "TouchDrvCST92xx.h"
#include "SensorWireHelper.h"
#include "pin_config.h"


#ifndef SENSOR_SDA
#define SENSOR_SDA IIC_SDA
#endif

#ifndef SENSOR_SCL
#define SENSOR_SCL IIC_SCL
#endif

#ifndef SENSOR_IRQ
#define SENSOR_IRQ TP_INT
#endif

#ifndef SENSOR_RST
#define SENSOR_RST -1
#endif

TouchDrvCST92xx touch;
int16_t x[5], y[5];
bool isPressed = false;
void setup();
void loop();
#line 59 "D:/Information/Data/VS_Data/VSCode/PlatformIO-IDE/Arduino/ESP32-S3/16.T-Display-AMOLED-1.43/examples/TouchDrv_CST9217_GetPoint/TouchDrv_CST9217_GetPoint.ino"
void setup()
{
    Serial.begin(115200);
    while (!Serial);

#if SENSOR_RST != -1
    pinMode(SENSOR_RST, OUTPUT);
    digitalWrite(SENSOR_RST, LOW);
    delay(30);
    digitalWrite(SENSOR_RST, HIGH);
    delay(50);
    delay(1000);
#endif

#if defined(ARDUINO_ARCH_RP2040)
    Wire.setSCL(SENSOR_SCL);
    Wire.setSDA(SENSOR_SDA);
    Wire.begin();
#elif defined(NRF52840_XXAA) || defined(NRF52832_XXAA)
    Wire.setPins(SENSOR_SDA, SENSOR_SCL);
    Wire.begin();
#else
    Wire.begin(SENSOR_SDA, SENSOR_SCL);
#endif


    SensorWireHelper::dumpDevices(Wire);



    uint8_t touchAddress = 0x5A;



    touch.jumpCheck();


    touch.setPins(SENSOR_RST, SENSOR_IRQ);
    bool result = touch.begin(Wire, touchAddress, SENSOR_SDA, SENSOR_SCL);
    if (result == false) {
        Serial.println("touch is not online..."); while (1)delay(1000);
    }
    Serial.print("Model :"); Serial.println(touch.getModelName());

    touch.setCoverScreenCallback([](void *ptr) {
        Serial.print(millis());
        Serial.println(" : The screen is covered");
    }, NULL);
# 134 "D:/Information/Data/VS_Data/VSCode/PlatformIO-IDE/Arduino/ESP32-S3/16.T-Display-AMOLED-1.43/examples/TouchDrv_CST9217_GetPoint/TouchDrv_CST9217_GetPoint.ino"
    attachInterrupt(SENSOR_IRQ, []() {
        isPressed = true;
    }, FALLING);
}

void loop()
{
    if (isPressed) {
        isPressed = false;
        uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());
        if (touched) {
            for (int i = 0; i < touched; ++i) {
                Serial.print("X[");
                Serial.print(i);
                Serial.print("]:");
                Serial.print(x[i]);
                Serial.print(" ");
                Serial.print(" Y[");
                Serial.print(i);
                Serial.print("]:");
                Serial.print(y[i]);
                Serial.print(" ");
            }
            Serial.println();
        }
    }
    delay(30);
}