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

#include "rtc.h"
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

void rtc_flow(SYSTEM_t *pSystem) {
  static uint8_t rtcStep = 0;
  _RTC rtc;
  bool ret;

  if (pSystem->rtcValue.rtcStatus == 1) {
    ret = DS3231_SetTime(&pSystem->rtcValue.rtc);
    sTime.Hours = pSystem->rtcValue.rtc.Hour;
    sTime.Minutes = pSystem->rtcValue.rtc.Min;
    sTime.Seconds = pSystem->rtcValue.rtc.Sec;
    sDate.Year = pSystem->rtcValue.rtc.Year;
    sDate.Month = pSystem->rtcValue.rtc.Month;
    sDate.Date = pSystem->rtcValue.rtc.Date;
    sDate.WeekDay = pSystem->rtcValue.rtc.DaysOfWeek;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    if (ret != true) {
      pSystem->rtcValue.errorCnt++;
      if (pSystem->rtcValue.errorCnt >= 10)
        pSystem->rtcValue.rtcStatus = 0xff;
    } else {
      pSystem->rtcValue.errorCnt = 0;
      pSystem->rtcValue.rtcStatus = 0;
    }
    return;
  } else if (pSystem->rtcValue.rtcStatus == 2) {
    ret = DS3231_GetTime(&rtc);
    if (ret != true) {
      pSystem->rtcValue.errorCnt++;
      if (pSystem->rtcValue.errorCnt >= 10)
        rtcStep = 99;
    } else {
      if (rtc.Year >= 24) {
        pSystem->rtcValue.rtc = rtc;
        sTime.Hours = pSystem->rtcValue.rtc.Hour;
        sTime.Minutes = pSystem->rtcValue.rtc.Min;
        sTime.Seconds = pSystem->rtcValue.rtc.Sec;
        sDate.Year = pSystem->rtcValue.rtc.Year;
        sDate.Month = pSystem->rtcValue.rtc.Month;
        sDate.Date = pSystem->rtcValue.rtc.Date;
        sDate.WeekDay = pSystem->rtcValue.rtc.DaysOfWeek;
        HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
      }
      pSystem->rtcValue.errorCnt = 0;
      pSystem->rtcValue.rtcCnt = 0;
    }
    if (ret != true) {
      pSystem->rtcValue.errorCnt++;
      if (pSystem->rtcValue.errorCnt >= 10)
        pSystem->rtcValue.rtcStatus = 0xff;
    } else {
      pSystem->rtcValue.errorCnt = 0;
      pSystem->rtcValue.rtcStatus = 0;
    }
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
          sTime.Hours = pSystem->rtcValue.rtc.Hour;
          sTime.Minutes = pSystem->rtcValue.rtc.Min;
          sTime.Seconds = pSystem->rtcValue.rtc.Sec;
          sDate.Year = pSystem->rtcValue.rtc.Year;
          sDate.Month = pSystem->rtcValue.rtc.Month;
          sDate.Date = pSystem->rtcValue.rtc.Date;
          sDate.WeekDay = pSystem->rtcValue.rtc.DaysOfWeek;
          HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
          HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
        }
        rtcStep++;
        pSystem->rtcValue.errorCnt = 0;
        pSystem->rtcValue.rtcCnt = 0;
      }
      break;
    case 3:
      HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
      HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
      pSystem->rtcValue.rtcCnt++;
      if (pSystem->rtcValue.rtcCnt >= 100 * 60 * 60 * 6)
        rtcStep = 2;
      break;
    case 99:
      HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
      pSystem->rtcValue.rtcStatus = 0xff;
      // todo error sequence make
      break;
  }
}

void initTime(void) {
  sTime.Hours = 1;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sDate.Year = 24;
  sDate.Month = 1;
  sDate.Date = 1;
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
}

void pt100Flow(SYSTEM_t *pSystem) {
  static uint8_t pt100Step = 0;
  bool ret = false;
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();

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
        if (ret != true) {
          pSystem->pt100Value.errorCnt[i]++;
          if (pSystem->pt100Value.errorCnt[i] >= 10)
            ;
          //;error 구조체선언
        } else {
          pSystem->pt100Value.errorCnt[i] = 0;
        }
      }
      pSystem->pt100Value.pt100Cal[0] = pSystem->pt100Value.pt100[0] + pELTop->userData.aMoLineResAdj + ((float) pELTop->calData.a1_LowOffSet / 10);
      pSystem->pt100Value.pt100Cal[1] = pSystem->pt100Value.pt100[1] + pELTop->userData.aBrLineResAdj + ((float) pELTop->calData.a2_LowOffSet / 10);
      pSystem->pt100Value.pt100Cal[2] = pSystem->pt100Value.pt100[2] + pELTop->userData.bMoLineResAdj + ((float) pELTop->calData.b1_LowOffSet / 10);
      pSystem->pt100Value.pt100Cal[3] = pSystem->pt100Value.pt100[3] + pELTop->userData.bBrLineResAdj + ((float) pELTop->calData.b2_LowOffSet / 10);
      break;
  }
}

button_obj_t water1;
button_obj_t water2;
button_obj_t drdy_1;
button_obj_t drdy_2;
button_obj_t drdy_3;
button_obj_t drdy_4;
button_obj_t sdcard_detect;

void input_init() {
  buttonObjCreate(&water1, WATER1, 500);
  buttonObjCreate(&water2, WATER2, 500);
  buttonObjCreate(&sdcard_detect, SDCARD_DETECT, 500);
  buttonObjCreate(&drdy_1, MAX31865_DRDY1, 500);
  buttonObjCreate(&drdy_2, MAX31865_DRDY2, 500);
  buttonObjCreate(&drdy_3, MAX31865_DRDY3, 500);
  buttonObjCreate(&drdy_4, MAX31865_DRDY4, 500);
}

void input_value(SYSTEM_t *pSystem) {
  pSystem->inputValue.NAME_FIELD.sdCardDetect = buttonGetPressed(SDCARD_DETECT);
  pSystem->inputValue.NAME_FIELD.max31865_drdy1 = buttonGetPressed(MAX31865_DRDY1);
  pSystem->inputValue.NAME_FIELD.max31865_drdy2 = buttonGetPressed(MAX31865_DRDY2);
  pSystem->inputValue.NAME_FIELD.max31865_drdy3 = buttonGetPressed(MAX31865_DRDY3);
  pSystem->inputValue.NAME_FIELD.max31865_drdy4 = buttonGetPressed(MAX31865_DRDY4);

  pSystem->inputValue.NAME_FIELD.waterSen1 = InPutObjGetClicked(&water1, 3000);
  pSystem->inputValue.NAME_FIELD.waterSen2 = InPutObjGetClicked(&water2, 3000);
}

int compare(const void *a, const void *b) {
  if (*(int*) a > *(int*) b)
    return 1;
  else if (*(int*) a < *(int*) b)
    return -1;
  else
    return 0;
}

void LevelAdcFlow(SYSTEM_t *pSystem) {
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  static uint8_t LevelAdcStep = 0;
//  static uint8_t AdcCnt = 0;

  switch (LevelAdcStep) {
    case 0:
      memset(pSystem->adcValue.levelVolt, 0, sizeof(pSystem->adcValue.levelVolt));
//      AdcCnt = 0;
      LevelAdcStep++;
      break;
    case 1:
      for (uint8_t i = 0; i < 10; i++) {
        pSystem->adcValue.levelVolt[0][i] = (double) ADC_data[0] / 4096 * 3.3;
        pSystem->adcValue.levelVolt[1][i] = (double) ADC_data[1] / 4096 * 3.3;
      }
//      AdcCnt++;
//      if (AdcCnt >= 10)
      LevelAdcStep++;
      break;
    case 2:
      qsort(pSystem->adcValue.levelVolt[0], 10, sizeof(pSystem->adcValue.levelVolt[0][0]), compare);
      qsort(pSystem->adcValue.levelVolt[1], 10, sizeof(pSystem->adcValue.levelVolt[1][0]), compare);
      pSystem->adcValue.levelVoltAvg[0] = (pSystem->adcValue.levelVolt[0][4] + pSystem->adcValue.levelVolt[0][5] + pSystem->adcValue.levelVolt[0][6]
          + pSystem->adcValue.levelVolt[0][7]) / 4;
      pSystem->adcValue.levelVoltAvg[1] = (pSystem->adcValue.levelVolt[1][4] + pSystem->adcValue.levelVolt[1][5] + pSystem->adcValue.levelVolt[1][6]
          + pSystem->adcValue.levelVolt[1][7]) / 4;
      LevelAdcStep++;
      break;
    case 3:
      for (uint8_t i = 0; i < 2; i++) {
//        pSystem->adcValue.levelSensorCur[i] = 12.085 * pSystem->adcValue.levelVoltAvg[i] - 1.3776;
        pSystem->adcValue.levelSensorCur[i] = 8.9553 * pSystem->adcValue.levelVoltAvg[i] - 0.0127;
      }

      pSystem->adcValue.levelSensor[0] = (0.0625 * pSystem->adcValue.levelSensorCur[0] - 0.25) * ((double) pELTop->levData.selectedSensorA / 100);
      pSystem->adcValue.levelSensorCal[0] = (uint16_t) ((pSystem->adcValue.levelSensor[0] * 100) + pELTop->levData.aMeterCal);

      pSystem->adcValue.levelSensor[1] = (0.0625 * pSystem->adcValue.levelSensorCur[1] - 0.25) * ((double) pELTop->levData.selectedSensorB / 100);
      pSystem->adcValue.levelSensorCal[1] = (uint16_t) ((pSystem->adcValue.levelSensor[1] * 100) + pELTop->levData.bMeterCal);
      LevelAdcStep = 0;
      break;
  }
}

void MCP4728Flow(SYSTEM_t *pSystem) {
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  static uint8_t MCP4728Step = 0;
  float calBuf;
  float buff;

  switch (MCP4728Step) {
    case 0:
      memset(&pSystem->dacValue.mcp4728, 0, sizeof(pSystem->dacValue.mcp4728));
      MCP4728_Init(&hi2c2, pSystem->dacValue.mcp4728);
      MCP4728Step++;
      break;
    case 1:
      calBuf = (float) (4095 - (1080 - pELTop->calData.c1_HighOffSet) - 600 - pELTop->calData.c1_LowOffSet) / 2500;
      buff = (pSystem->pt100Value.pt100Cal[0] * 10 * calBuf) + pELTop->calData.c1_LowOffSet + 600;  //) * 4.1;
      if (buff >= 4095)
        buff = 4095;
      pSystem->dacValue.mcp4728.channel_Val[1] = (uint16_t) buff;

      calBuf = (float) (4095 - (1080 - pELTop->calData.c2_HighOffSet) - 600 - pELTop->calData.c2_LowOffSet) / 2500;
      buff = (pSystem->pt100Value.pt100Cal[1] * 10 * calBuf) + pELTop->calData.c2_LowOffSet + 600;  //) * 4.1;
      if (buff >= 4095)
        buff = 4095;
      pSystem->dacValue.mcp4728.channel_Val[0] = (uint16_t) buff;

      calBuf = (float) (4095 - (1080 - pELTop->calData.d1_HighOffSet) - 600 - pELTop->calData.d1_LowOffSet) / 2500;
      buff = (pSystem->pt100Value.pt100Cal[2] * 10 * calBuf) + pELTop->calData.d1_LowOffSet + 600;  //) * 4.1;
      if (buff >= 4095)
        buff = 4095;
      pSystem->dacValue.mcp4728.channel_Val[3] = (uint16_t) buff;

      calBuf = (float) (4095 - (1080 - pELTop->calData.d2_HighOffSet) - 600 - pELTop->calData.d2_LowOffSet) / 2500;
      buff = (pSystem->pt100Value.pt100Cal[3] * 10 * calBuf) + pELTop->calData.d2_LowOffSet + 600;  //) * 4.1;
      if (buff >= 4095)
        buff = 4095;
      pSystem->dacValue.mcp4728.channel_Val[2] = (uint16_t) buff;
      MCP4728_Write_AllChannels_Diff(&hi2c2, pSystem->dacValue.mcp4728);
      break;
  }
}

void Input_Task(void *argument) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  input_init();
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADC_data[0], 2);

  initTime();

#ifdef _USE_CLI
  cliAdd("input", cliInput);
#endif
  while (1) {
    rtc_flow(pSystem);
    input_value(pSystem);
    pt100Flow(pSystem);
    MCP4728Flow(pSystem);
    LevelAdcFlow(pSystem);
    osDelay(10);

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
      osDelay(20);
    }

    ret = true;
  }

  if (ret != true) {
    cliPrintf("input show\n");
  }
}
#endif

