// BigDemo.ino

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>

#include "unphone.h"
#include "foodflows.h"
#include "WiFiHandler.h"
#include "UIController.h"
// TODO #include "HttpFirmwareUpdate.h"
#include "IOExpander.h"

static const char *TAG = "MAIN";        // ESP logger debug tag
int loopIter = 0;                       // counter for main loop
int loopIter2 = 0;                      // counter for wifi task loop

UIController *uiCont;

void setup() {
  Wire.setClock(100000); // higher rates trigger an IOExpander bug
  UNPHONE_DBG = true;
  Serial.begin(115200);  // init the serial line

  // fire up I²C, and the unPhone's IOExpander library
  Wire.begin();
  IOExpander::begin();

  checkPowerSwitch(); // check if power switch is now off & if so shutdown

  if(!accel.begin()) // set up the accelerometer
  {
    D("Failed to start accelerometer");
    delay(3000);
  }
  
  i2s_config(); // configure the I2S bus

  pinMode(IR_LEDS, OUTPUT); // IR_LED pin
  
  D_ON
  D("unphone setup\n")
  bool noErrors = true;

  uiCont = new UIController(ui_testcard);

  if(!uiCont->begin()) {
    noErrors = false;
    E("WARNING: ui.begin failed!\n")
  }
}

void loop() {
  D("entering main loop\n")
  while(1) {
    micros(); // update overflow
    uiCont->run();
    checkPowerSwitch(); // check if power switch is now off & if so shutdown

    // allow the protocol CPU IDLE task to run periodically
    if(loopIter % 2500 == 0) {
      if(loopIter % 25000 == 0)
        D("completed loop %d, yielding 1000th time since last\n", loopIter)

      vTaskDelay(100 / portTICK_PERIOD_MS); // 100 appears min to allow IDLE
    }
    loopIter++;
  }
}

/* TODO
void wifi_task(void *pvParameter) {
  wifi_handler_startup();
  HttpFirmwareUpdate test("test", BUILD_VERSION, DEV_BUILD);
  unsigned long timer = millis();
  
  while(1) {
    wifi_handler_turn();

    if (millis() - timer > 1000000) {// don't actually want this to happen atm
      if (test.checkForUpdate()){
        test.performUpdate();
      }
      timer = millis();
    }
    
    // allow the protocol CPU IDLE task to run periodically
    if(loopIter2++ % 2500 == 0) {
      if(loopIter2++ % 25000 == 0)
        D("completed loop %d, yielding 1000th time since last\n", loopIter2 - 1)

      vTaskDelay(100 / portTICK_PERIOD_MS); // 100 appears min to allow IDLE
    }
  }
}
*/

/* TODO
extern "C" void app_main() {
  esp_log_level_set("*", ESP_LOG_DEBUG);
  esp_log_level_set(TAG, ESP_LOG_DEBUG);
  // E("this would be an oops\n")

  D("initArduino....\n")
  initArduino();
  D("....done\n");

  printf("/////////////////\n");
  printf(String(UNPHONE_VERSION).c_str());
  printf("/////////////////\n");
  printf(String(BUILD_VERSION).c_str());
  printf("/////////////////\n");
  printf(String(DEV_BUILD).c_str());
  printf("/////////////////\n");

  const BaseType_t UI_TASK_CORE = 1;
  xTaskCreatePinnedToCore(
    unphone_task, "unphone_task", 8192, NULL, 1000, NULL, UI_TASK_CORE
  );

//  //FIXME - shouldn't be running on protocol core... everything appears to work still though 
//  //TODO - read FIXME
//  // THIS IS TERRIBLE (apparently)
//  const BaseType_t WIFI_TASK_CORE = 0;
//  xTaskCreatePinnedToCore(
//    wifi_task, "wifitask", 8192, NULL, 1000, NULL, WIFI_TASK_CORE
//  );
}
*/
