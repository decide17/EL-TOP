/*
 * Task_input.c
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#include<stdlib.h>
#include "i2c.h"

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

extern SPI_HandleTypeDef hspi1;
Max31865_t max31865[4];
__IO uint16_t ADC_data[2];

void pt100Flow(SYSTEM_t *pSystem) {
  static uint8_t pt100Step = 0;
  static uint8_t pt100Cnt = 0;
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
      pt100Step++;
      break;
    case 2:
      pt100Cnt++;
      if (pt100Cnt >= 100) {
        pt100Cnt = 0;
        pt100Step = 1;
      }
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

// 비교 함수
int compare(const void *a, const void *b) {
  uint16_t val1 = *(const uint16_t*) a;
  uint16_t val2 = *(const uint16_t*) b;

  if (val1 < val2)
    return -1;
  if (val1 > val2)
    return 1;
  return 0;
}

void LevelAdcFlow(SYSTEM_t *pSystem) {
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  static uint8_t LevelAdcStep = 0;
  static uint8_t AdcCnt = 0;

  switch (LevelAdcStep) {
    case 0:
      memset(pSystem->adcValue.levelVolt, 0, sizeof(pSystem->adcValue.levelVolt));
      AdcCnt = 0;
      LevelAdcStep++;
      break;
    case 1:
      pSystem->adcValue.levelVolt[0][AdcCnt] = ADC_data[0];
      pSystem->adcValue.levelVolt[1][AdcCnt] = ADC_data[1];
      AdcCnt++;
      if (AdcCnt >= 50) {
        LevelAdcStep++;
      }
      break;
    case 2:
      qsort(pSystem->adcValue.levelVolt[0], 50, sizeof(uint16_t), compare);
      qsort(pSystem->adcValue.levelVolt[1], 50, sizeof(uint16_t), compare);
      pSystem->adcValue.levelVoltAvg[0] = (double) (pSystem->adcValue.levelVolt[0][24] + pSystem->adcValue.levelVolt[0][25] + pSystem->adcValue.levelVolt[0][26]
          + pSystem->adcValue.levelVolt[0][27]) / 4 / 4096 * 3.3;
      pSystem->adcValue.levelVoltAvg[1] = (double) (pSystem->adcValue.levelVolt[1][24] + pSystem->adcValue.levelVolt[1][25] + pSystem->adcValue.levelVolt[1][26]
          + pSystem->adcValue.levelVolt[1][27]) / 4 / 4096 * 3.3;
      LevelAdcStep++;
      break;
    case 3:
      for (uint8_t i = 0; i < 2; i++) {
//        pSystem->adcValue.levelSensorCur[i] = (0.4744 * pSystem->adcValue.levelVoltAvg[i] * pSystem->adcValue.levelVoltAvg[i])
//            + (8.1836 * pSystem->adcValue.levelVoltAvg[i]) + 0.3617;

        pSystem->adcValue.levelSensorCur[i] = (-0.066 * pSystem->adcValue.levelVoltAvg[i] * pSystem->adcValue.levelVoltAvg[i])
            + (10.283 * pSystem->adcValue.levelVoltAvg[i]) - 0.0464;
      }

      pSystem->adcValue.levelSensor[0] = (0.0625 * pSystem->adcValue.levelSensorCur[0] - 0.25) * ((double) pELTop->levData.selectedSensorA / 100);
      pSystem->adcValue.levelSensorCal[0] = (uint16_t) ((pSystem->adcValue.levelSensor[0] * 100) + pELTop->levData.aMeterCal);

      pSystem->adcValue.levelSensor[1] = (0.0625 * pSystem->adcValue.levelSensorCur[1] - 0.25) * ((double) pELTop->levData.selectedSensorB / 100);
      pSystem->adcValue.levelSensorCal[1] = (uint16_t) ((pSystem->adcValue.levelSensor[1] * 100) + pELTop->levData.bMeterCal);
      LevelAdcStep = 0;
      break;
  }
}

void Input_Task(void *argument) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  input_init();
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADC_data[0], 2);

#ifdef _USE_CLI
  cliAdd("input", cliInput);
#endif
  while (1) {
    input_value(pSystem);
    pt100Flow(pSystem);
    LevelAdcFlow(pSystem);
    osDelay(1);

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

