/*
 * Task_input.c
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#include <stdlib.h>
#include <string.h>
#include "i2c.h"

#include "database.h"
#include "com_gpio.h"

#include "Task_Input.h"

#ifdef _USE_CLI
static void cliInput(uint8_t argc, const char **argv);
#endif

osThreadId_t task_Input_Handle;
const osThreadAttr_t taskInput_attributes = { .name = "Input Thread", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };

extern SPI_HandleTypeDef hspi1;
__IO uint16_t ADC_data[2];

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
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*) &ADC_data[0], 2);

#ifdef _USE_CLI
  cliAdd("input", cliInput);
#endif
  while (1) {
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

