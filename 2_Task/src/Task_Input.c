/*
 * Task_input.c
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#include<stdlib.h>
#include "database.h"
#include "com_gpio.h"

#include "Task_Input.h"
#include "Task_Cli.h"

#include "mcp4728.h"
#include "MAX31865.h"
#include "stm32_ds3231.h"

#ifdef _USE_CLI
static void cliInput(uint8_t argc, const char **argv);
#endif

osThreadId_t task_Input_Handle;
const osThreadAttr_t taskInput_attributes = { .name = "Input Thread", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern SPI_HandleTypeDef hspi1;
Max31865_t max31865[4];
__IO uint16_t ADC_data[2];

//dacChannelConfig output1;

void rtc_flow(_SYSTEM_t *pSystem) {
  static uint8_t rtcStep = 0;
  _RTC rtc;
  bool ret;

  if (pSystem->rtcValue.rtcStatus == 1) {
    ret = DS3231_SetTime(&pSystem->rtcValue.rtc);
    if (ret != true) {
      pSystem->rtcValue.errorCnt++;
      if (pSystem->rtcValue.errorCnt >= 10)
        pSystem->rtcValue.rtcStatus = 0xff;
    } else {
      pSystem->rtcValue.errorCnt = 0;
      pSystem->rtcValue.rtcStatus = 0;
    }
    return;
  }
  switch (rtcStep) {
    case 0:
      DS3231_Init(&hi2c1);
      pSystem->rtcValue.errorCnt = 0;
      rtcStep++;
      break;
    case 1:
      DS3231_ClearAlarm1();
      ret = DS3231_SetAlarm1(ALARM_MODE_SEC_MATCHED, 0, 0, 0, 30);
      if (ret != true) {
        pSystem->rtcValue.errorCnt++;
        if (pSystem->rtcValue.errorCnt >= 10)
          rtcStep = 99;
      } else {
        rtcStep++;
        pSystem->rtcValue.errorCnt = 0;
      }
      break;
    case 2:
      ret = DS3231_GetTime(&rtc);
      if (ret != true) {
        pSystem->rtcValue.errorCnt++;
        if (pSystem->rtcValue.errorCnt >= 10)
          rtcStep = 99;
      } else {
        if (rtc.Year >= 24) {
          pSystem->rtcValue.rtc = rtc;
        }
        rtcStep++;
        pSystem->rtcValue.errorCnt = 0;
      }
      break;
    case 3:
      ret = DS3231_ReadTemperature(&pSystem->rtcValue.rtcTemp);
      if (ret != true) {
        pSystem->rtcValue.errorCnt++;
        if (pSystem->rtcValue.errorCnt >= 10)
          rtcStep = 99;
      } else {
        rtcStep++;
        pSystem->rtcValue.errorCnt = 0;
        pSystem->rtcValue.rtcCnt = 0;
      }
      break;
    case 4:
      pSystem->rtcValue.rtcCnt++;
      if (pSystem->rtcValue.rtcCnt >= 50)
        rtcStep = 2;
      break;
    case 99:
      pSystem->rtcValue.rtcStatus = 0xff;
      // todo error sequence make
      break;
  }
  //  DS3231_SetRateSelect(DS3231_1HZ);
  //Set time.
  //  DS3231_SetFullTime(23, 59, 50);
  //Set date.
  //  DS3231_SetFullDate(10, 11, 2, 2020);

}

void pt100Flow(_SYSTEM_t *pSystem) {
  static uint8_t pt100Step = 0;
  bool ret = false;

  switch (pt100Step) {
    case 0:
      Max31865_init(&max31865[0], &hspi1, GPIOB, GPIO_PIN_1, 2, 0);
      Max31865_init(&max31865[1], &hspi1, GPIOA, GPIO_PIN_6, 2, 0);
      Max31865_init(&max31865[2], &hspi1, GPIOA, GPIO_PIN_7, 2, 0);
      Max31865_init(&max31865[3], &hspi1, GPIOA, GPIO_PIN_8, 2, 0);
      for (uint8_t i = 0; i < 4; i++) {
        pSystem->pt100Value.errorCnt[i] = 0;
      }
      pt100Step++;
      break;
    case 1:
      for (uint8_t i = 0; i < 4; i++) {
        ret = Max31865_readTempC(&max31865[i], &pSystem->pt100Value.pt100[i]);
        ret = Max31865_readTempC(&max31865[i], &pSystem->pt100Value.pt100[i]);
        ret = Max31865_readTempC(&max31865[i], &pSystem->pt100Value.pt100[i]);
        ret = Max31865_readTempC(&max31865[i], &pSystem->pt100Value.pt100[i]);

        if (ret != true) {
          pSystem->pt100Value.errorCnt[i]++;
          if (pSystem->pt100Value.errorCnt[i] >= 10)
            ;
          //;error 구조체선언
        } else {
          pSystem->pt100Value.errorCnt[i] = 0;
        }
      }
      break;
  }
}
void input_init() {
  button_obj_t water1;
  button_obj_t water2;
  button_obj_t drdy_1;
  button_obj_t drdy_2;
  button_obj_t drdy_3;
  button_obj_t drdy_4;
  button_obj_t sdcard_detect;
  buttonObjCreate(&water1, WATER1, 500);
  buttonObjCreate(&water2, WATER2, 500);
  buttonObjCreate(&sdcard_detect, SDCARD_DETECT, 500);
  buttonObjCreate(&drdy_1, MAX31865_DRDY1, 500);
  buttonObjCreate(&drdy_2, MAX31865_DRDY2, 500);
  buttonObjCreate(&drdy_3, MAX31865_DRDY3, 500);
  buttonObjCreate(&drdy_4, MAX31865_DRDY4, 500);
}

void input_value(_SYSTEM_t *pSystem) {
  pSystem->inputValue.NAME_FIELD.sdCardDetect = buttonGetPressed(SDCARD_DETECT);
  pSystem->inputValue.NAME_FIELD.waterSen1 = buttonGetPressed(WATER1);
  pSystem->inputValue.NAME_FIELD.waterSen2 = buttonGetPressed(WATER2);
  pSystem->inputValue.NAME_FIELD.max31865_drdy1 = buttonGetPressed(MAX31865_DRDY1);
  pSystem->inputValue.NAME_FIELD.max31865_drdy2 = buttonGetPressed(MAX31865_DRDY2);
  pSystem->inputValue.NAME_FIELD.max31865_drdy3 = buttonGetPressed(MAX31865_DRDY3);
  pSystem->inputValue.NAME_FIELD.max31865_drdy4 = buttonGetPressed(MAX31865_DRDY4);

  pSystem->inputValue.NAME_FIELD.levelVolt[0] = (float) ADC_data[0] / 4096 * 3.3;
  pSystem->inputValue.NAME_FIELD.levelVolt[1] = (float) ADC_data[1] / 4096 * 3.3;
}

void MCP4728Flow(_SYSTEM_t *pSystem) {
  static uint8_t MCP4728Step = 0;

  switch (MCP4728Step) {
    case 0:
      memset(&pSystem->dacValue.mcp4728, 0, sizeof(pSystem->dacValue.mcp4728));
      MCP4728_Init(&hi2c2, pSystem->dacValue.mcp4728);
      MCP4728Step++;
      break;
    case 1:
      MCP4728_Write_AllChannels_Diff(&hi2c2, pSystem->dacValue.mcp4728);
//      MCP4728Step++;
      break;
//    case 2:
//      pSystem->dacValue.mcp4728.channel_Val[0] = 1000;
//      pSystem->dacValue.mcp4728.channel_Val[1] = 2000;
//      pSystem->dacValue.mcp4728.channel_Val[2] = 3000;
//      pSystem->dacValue.mcp4728.channel_Val[3] = 4095;
//      MCP4728Step--;
//      break;
  }
}

void Input_Task(void *argument) {
  _SYSTEM_t *pSystem = (_SYSTEM_t*) DataBase_Get_pInfo_Data();

//  y = 0.0048x + 0.0046;
  input_init();
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADC_data[0], 2);

#ifdef _USE_CLI
  cliAdd("input", cliInput);
#endif
  while (1) {
    rtc_flow(pSystem);
    input_value(pSystem);
    pt100Flow(pSystem);
    MCP4728Flow(pSystem);
    osDelay(100);

  }
}

void INPUT_Create_Thread(void) {
  task_Input_Handle = osThreadNew(Input_Task, NULL, &taskInput_attributes);
}

#ifdef _USE_CLI
void cliInput(uint8_t argc, const char **argv) {
  bool ret = false;

  if (argc == 1 && cliIsStr(argv[0], "show")) {
    while (cliKeepLoop()) {
      cliPrintf("\n");
      osDelay(100);
    }

    ret = true;
  }

  if (ret != true) {
    cliPrintf("input show\n");
  }
}
#endif

