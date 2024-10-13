/*
 * Task_Remote.c
 *
 *  Created on: Aug 24, 2024
 *      Author: USER
 */
#include <stdbool.h>

#include "database.h"

#include "Task_Input.h"
#include "Task_Cli.h"
#include "Task_Remote.h"

#include "com_flash.h"
#include "tm1639.h"
#include "User_Temp_Set.h"
#include "User_Level_Set.h"
#include "User_Menu_Set.h"
#include "Factory_Calibration.h"

#ifdef _USE_CLI
static void cliRemote(uint8_t argc, const char **argv);
#endif

osThreadId_t task_Remote_Handle;
const osThreadAttr_t taskRemote_attributes = { .name = "Remote Thread", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityAboveNormal, };

osTimerId_t Buzzer_ClearTimer_ID;
#define CONTROL_TIMER_BUZZER_CLEAR             1
static void Control_Oneshot_Timer_Callback(void *arg);

Tm1639_t tm1639_io3;
Tm1639_t tm1639_io4;

/**
 * @brief   부저 동작
 * @note
 * @param On or Off
 * @retval
 */
void BUZZER_Control(ONOFF_t on, uint16_t time) {
  if (on == ON) {
    gpio_on(BUZZER);
    osTimerStart(Buzzer_ClearTimer_ID, time);
  } else {
    gpio_off(BUZZER);
  }
}

void RESET_BUTTON(SYSTEM_t *SystemData) {
  SystemData->buttonVaule.NAME_FIELD.Button_UP_CHK = 0;
  SystemData->buttonVaule.NAME_FIELD.Button_DOWN_CHK = 0;
  SystemData->buttonVaule.NAME_FIELD.Button_SET_CHK = 0;
  SystemData->buttonVaule.NAME_FIELD.Button_RESET_CHK = 0;
  SystemData->buttonVaule.BYTE_FIELD[2] = 0;
  SystemData->buttonVaule.BYTE_FIELD[3] = 0;
  SystemData->buttonVaule.BYTE_FIELD[4] = 0;
  memset(&SystemData->setValue, 0, sizeof(SystemData->setValue));
}

int circularValue(int max, int min, int value) {
  if (value > max) {
    return min;
  } else if (value < min) {
    return max;
  } else {
    return value;
  }
}

void StandByLedDisplay(SYSTEM_t *pSystem, uint8_t displayCase) {
  if (displayCase) {
    if (pSystem->pt100Value.pt100Cal[2] < -99) {
      tm1639_io4.data.NAME_FIELD.LED2_GREEN = 0;
      pSystem->setValue.NAME_FIELD.BMO_RED_TOGGLE = 1;
    } else if (pSystem->pt100Value.pt100Cal[2] > 280) {
      tm1639_io4.data.NAME_FIELD.LED2_RED = 0;
      tm1639_io4.data.NAME_FIELD.LED2_GREEN = 1;
      pSystem->setValue.NAME_FIELD.BMO_RED_TOGGLE = 0;
    } else {
      if (pSystem->outputValue.NAME_FIELD.bMoFlag) {
        tm1639_io4.data.NAME_FIELD.LED2_GREEN = 0;
        pSystem->setValue.NAME_FIELD.BMO_RED_TOGGLE = 1;
      } else {
        pSystem->setValue.NAME_FIELD.BMO_RED_TOGGLE = 0;
        tm1639_io4.data.NAME_FIELD.LED2_RED = 0;
        tm1639_io4.data.NAME_FIELD.LED2_GREEN = 1;
      }
    }
    if (pSystem->pt100Value.pt100Cal[3] < -99) {
      tm1639_io3.data.NAME_FIELD.LED7_GREEN = 0;
      pSystem->setValue.NAME_FIELD.BBR_RED_TOGGLE = 1;
    } else if (pSystem->pt100Value.pt100Cal[3] > 280) {
      tm1639_io3.data.NAME_FIELD.LED7_RED = 0;
      tm1639_io3.data.NAME_FIELD.LED7_GREEN = 1;
      pSystem->setValue.NAME_FIELD.BBR_RED_TOGGLE = 0;
    } else {
      if (pSystem->outputValue.NAME_FIELD.bBrFlag) {
        tm1639_io3.data.NAME_FIELD.LED7_GREEN = 0;
        pSystem->setValue.NAME_FIELD.BBR_RED_TOGGLE = 1;
      } else {
        pSystem->setValue.NAME_FIELD.BBR_RED_TOGGLE = 0;
        tm1639_io3.data.NAME_FIELD.LED7_RED = 0;
        tm1639_io3.data.NAME_FIELD.LED7_GREEN = 1;
      }
    }
    tm1639_io4.data.NAME_FIELD.LED1_GREEN = 0;
    tm1639_io3.data.NAME_FIELD.LED8_GREEN = 0;
  } else {
    if (pSystem->pt100Value.pt100Cal[0] < -99) {
      tm1639_io4.data.NAME_FIELD.LED1_GREEN = 0;
      pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE = 1;
    } else if (pSystem->pt100Value.pt100Cal[0] > 280) {
      tm1639_io4.data.NAME_FIELD.LED1_RED = 0;
      tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
      pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE = 0;
    } else {
      if (pSystem->outputValue.NAME_FIELD.aMoFlag) {
        tm1639_io4.data.NAME_FIELD.LED1_GREEN = 0;
        pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE = 1;
      } else {
        pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE = 0;
        tm1639_io4.data.NAME_FIELD.LED1_RED = 0;
        tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
      }
    }
    if (pSystem->pt100Value.pt100Cal[1] < -99) {
      tm1639_io3.data.NAME_FIELD.LED8_GREEN = 0;
      pSystem->setValue.NAME_FIELD.ABR_RED_TOGGLE = 1;
    } else if (pSystem->pt100Value.pt100Cal[1] > 280) {
      tm1639_io3.data.NAME_FIELD.LED8_RED = 0;
      tm1639_io3.data.NAME_FIELD.LED8_GREEN = 1;
      pSystem->setValue.NAME_FIELD.ABR_RED_TOGGLE = 0;
    } else {
      if (pSystem->outputValue.NAME_FIELD.aBrFlag) {
        tm1639_io3.data.NAME_FIELD.LED8_GREEN = 0;
        pSystem->setValue.NAME_FIELD.ABR_RED_TOGGLE = 1;
      } else {
        pSystem->setValue.NAME_FIELD.ABR_RED_TOGGLE = 0;
        tm1639_io3.data.NAME_FIELD.LED8_RED = 0;
        tm1639_io3.data.NAME_FIELD.LED8_GREEN = 1;
      }
    }
    tm1639_io4.data.NAME_FIELD.LED2_GREEN = 0;
    tm1639_io3.data.NAME_FIELD.LED7_GREEN = 0;
  }
}

void StandByTempDisplay(SYSTEM_t *pSystem, uint8_t displayCase) {
  char read_str[10];

  if (pSystem->pt100Value.pt100Cal[displayCase * 2] <= -21) {
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Sht");
  } else if (pSystem->pt100Value.pt100Cal[displayCase * 2] <= 281) {
    if (pSystem->pt100Value.pt100Cal[displayCase * 2] >= 0 && pSystem->pt100Value.pt100Cal[displayCase * 2] < 10) {
      snprintf(read_str, sizeof(read_str), "  %d", (int) pSystem->pt100Value.pt100Cal[displayCase * 2]);
    } else if (pSystem->pt100Value.pt100Cal[displayCase * 2] >= 10 && pSystem->pt100Value.pt100Cal[displayCase * 2] < 100) {
      snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100Cal[displayCase * 2]);
    } else if (pSystem->pt100Value.pt100Cal[displayCase * 2] > -10 && pSystem->pt100Value.pt100Cal[displayCase * 2] <= -1) {
      snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100Cal[displayCase * 2]);
    } else {
      snprintf(read_str, sizeof(read_str), "%d", (int) pSystem->pt100Value.pt100Cal[displayCase * 2]);
    }
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, read_str);
  } else {
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "---");
  }
  if (pSystem->pt100Value.pt100Cal[displayCase * 2 + 1] <= -21) {
    tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "Sht");
  } else if (pSystem->pt100Value.pt100Cal[displayCase * 2 + 1] <= 281) {
    if (pSystem->pt100Value.pt100Cal[displayCase * 2 + 1] >= 0 && pSystem->pt100Value.pt100Cal[displayCase * 2 + 1] < 10) {
      snprintf(read_str, sizeof(read_str), "  %d", (int) pSystem->pt100Value.pt100Cal[displayCase * 2 + 1]);
    } else if (pSystem->pt100Value.pt100Cal[displayCase * 2 + 1] >= 10 && pSystem->pt100Value.pt100Cal[displayCase * 2 + 1] < 100) {
      snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100Cal[displayCase * 2 + 1]);
    } else if (pSystem->pt100Value.pt100Cal[displayCase * 2 + 1] > -10 && pSystem->pt100Value.pt100Cal[displayCase * 2 + 1] <= -1) {
      snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100Cal[displayCase * 2 + 1]);
    } else {
      snprintf(read_str, sizeof(read_str), "%d", (int) pSystem->pt100Value.pt100Cal[displayCase * 2 + 1]);
    }
    tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, read_str);
  } else {
    tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "---");
  }
}

void StandByLevelDisplay(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  char read_str[10] = { 0 };

  if (pELTop->userData.channel == 0) {
    if (pSystem->adcValue.levelSensorCur[0] >= 4) {
      if (pSystem->adcValue.levelSensorCal[0] >= pELTop->levData.selectedSensorA) {
        snprintf(read_str, sizeof(read_str), "%.2f", ((double) pELTop->levData.selectedSensorA / 100));
        tm1639Display_str(&tm1639_io3, IO3_A_LEVEL_FND, read_str);
      } else {
        snprintf(read_str, sizeof(read_str), "%.2f", ((double) pSystem->adcValue.levelSensorCal[0] / 100));
        tm1639Display_str(&tm1639_io3, IO3_A_LEVEL_FND, read_str);
      }
      ////////////////////////////////////////
      /////////////////////////////// 디버깅용
      ///////////////////////////////////////
      // snprintf(read_str, sizeof(read_str), "%.2f", (double) pSystem->adcValue.levelVoltAvg[0]);
      // tm1639Display_str(&tm1639_io4, IO4_B_LEVEL_FND, read_str);
      //////////////////////////////////////////
    } else {
      tm1639Display_str(&tm1639_io3, IO3_A_LEVEL_FND, "---");
    }
  } else if (pELTop->userData.channel == 1) {
    if (pSystem->adcValue.levelSensorCur[1] >= 4) {
      if (pSystem->adcValue.levelSensorCal[1] >= pELTop->levData.selectedSensorB) {
        snprintf(read_str, sizeof(read_str), "%.2f", ((double) pELTop->levData.selectedSensorB / 100));
        tm1639Display_str(&tm1639_io4, IO4_B_LEVEL_FND, read_str);
      } else {
        snprintf(read_str, sizeof(read_str), "%.2f", ((double) pSystem->adcValue.levelSensorCal[1] / 100));
        tm1639Display_str(&tm1639_io4, IO4_B_LEVEL_FND, read_str);
      }
    } else {
      tm1639Display_str(&tm1639_io4, IO4_B_LEVEL_FND, "---");
    }
  } else {
    if (pSystem->adcValue.levelSensorCur[0] >= 4) {
      if (pSystem->adcValue.levelSensorCal[0] >= pELTop->levData.selectedSensorA) {
        snprintf(read_str, sizeof(read_str), "%.2f", ((double) pELTop->levData.selectedSensorA / 100));
        tm1639Display_str(&tm1639_io3, IO3_A_LEVEL_FND, read_str);
      } else {
        snprintf(read_str, sizeof(read_str), "%.2f", ((double) pSystem->adcValue.levelSensorCal[0] / 100));
        tm1639Display_str(&tm1639_io3, IO3_A_LEVEL_FND, read_str);
      }
    } else {
      tm1639Display_str(&tm1639_io3, IO3_A_LEVEL_FND, "---");
    }
    if (pSystem->adcValue.levelSensorCur[1] >= 4) {
      if (pSystem->adcValue.levelSensorCal[1] >= pELTop->levData.selectedSensorB) {
        snprintf(read_str, sizeof(read_str), "%.2f", ((double) pELTop->levData.selectedSensorB / 100));
        tm1639Display_str(&tm1639_io4, IO4_B_LEVEL_FND, read_str);
      } else {
        snprintf(read_str, sizeof(read_str), "%.2f", ((double) pSystem->adcValue.levelSensorCal[1] / 100));
        tm1639Display_str(&tm1639_io4, IO4_B_LEVEL_FND, read_str);
      }
    } else {
      tm1639Display_str(&tm1639_io4, IO4_B_LEVEL_FND, "---");
    }
  }
}

void channel_display(SET_DATA_t *pELTop) {
  switch (pELTop->remoteData.read_data) {
    case 0:
      tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A- ");
      break;
    case 1:
      tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "B- ");
      break;
    case 2:
      tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "a-b");
      break;
    default:
      tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "nuh");
      pELTop->userData.channel = 0;
      pELTop->remoteData.read_data = pELTop->userData.channel;
      break;
  }
}

uint8_t displayCase = 0;
uint8_t displayCnt = 0;
/**
 * @brief 대기상태에서 전면 화면 제어
 * @details 버튼으로 설정화면에 들어가지않은 상태, LED, FND 표시
 * @param args SYSTEM_t 구조체 SET_DATA_t 구조체
 * @return none
 */
void led_Control(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  static uint8_t ledStep = 0;
  static uint32_t pre_time;

  if (pELTop->remoteData.setData == STAND_BY) {
    switch (ledStep) {
      case 0:
        pre_time = HAL_GetTick();
        ledStep++;
        break;
      case 1:
        if (HAL_GetTick() - pre_time >= 500) {
          if (pSystem->setValue.NAME_FIELD.AMO_GREEN_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED1_GREEN)
              tm1639_io4.data.NAME_FIELD.LED1_GREEN = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED1_RED) {
              tm1639_io4.data.NAME_FIELD.LED1_RED = 0;
            } else {
              tm1639_io4.data.NAME_FIELD.LED1_RED = 1;
            }
          }
          if (pSystem->setValue.NAME_FIELD.BMO_GREEN_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED2_GREEN)
              tm1639_io4.data.NAME_FIELD.LED2_GREEN = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED2_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.BMO_RED_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED2_RED) {
              tm1639_io4.data.NAME_FIELD.LED2_RED = 0;
            } else {
              tm1639_io4.data.NAME_FIELD.LED2_RED = 1;
            }
          }
          if (pSystem->setValue.NAME_FIELD.ALK_GREEN_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED3_GREEN)
              tm1639_io4.data.NAME_FIELD.LED3_GREEN = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED3_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.ALK_RED_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED3_RED) {
              tm1639_io4.data.NAME_FIELD.LED3_RED = 0;
            } else {
              tm1639_io4.data.NAME_FIELD.LED3_RED = 1;
            }
          }
          if (pSystem->setValue.NAME_FIELD.AHT_GREEN_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED4_GREEN)
              tm1639_io4.data.NAME_FIELD.LED4_GREEN = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED4_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.AHT_RED_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED4_RED)
              tm1639_io4.data.NAME_FIELD.LED4_RED = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED4_RED = 1;
          }
          if (pSystem->setValue.NAME_FIELD.B_LH_GREEN_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED5_GREEN)
              tm1639_io4.data.NAME_FIELD.LED5_GREEN = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED5_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.B_LH_RED_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED5_RED)
              tm1639_io4.data.NAME_FIELD.LED5_RED = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED5_RED = 1;
          }
          if (pSystem->setValue.NAME_FIELD.B_LL_GREEN_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED6_GREEN)
              tm1639_io4.data.NAME_FIELD.LED6_GREEN = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED6_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.B_LL_RED_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED6_RED) {
              tm1639_io4.data.NAME_FIELD.LED6_RED = 0;
            } else {
              tm1639_io4.data.NAME_FIELD.LED6_RED = 1;
            }
          }
          if (pSystem->setValue.NAME_FIELD.B_HH_GREEN_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED7_GREEN)
              tm1639_io4.data.NAME_FIELD.LED7_GREEN = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED7_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.B_HH_RED_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED7_RED) {
              tm1639_io4.data.NAME_FIELD.LED7_RED = 0;
            } else {
              tm1639_io4.data.NAME_FIELD.LED7_RED = 1;
            }
          }
          if (pSystem->setValue.NAME_FIELD.B_PC_GREEN_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED8_GREEN)
              tm1639_io4.data.NAME_FIELD.LED8_GREEN = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED8_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.B_PC_RED_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED8_RED)
              tm1639_io4.data.NAME_FIELD.LED8_RED = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED8_RED = 1;
          }
          if (pSystem->setValue.NAME_FIELD.A_LH_GREEN_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED1_GREEN)
              tm1639_io3.data.NAME_FIELD.LED1_GREEN = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED1_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.A_LH_RED_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED1_RED)
              tm1639_io3.data.NAME_FIELD.LED1_RED = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED1_RED = 1;
          }
          if (pSystem->setValue.NAME_FIELD.A_LL_GREEN_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED2_GREEN)
              tm1639_io3.data.NAME_FIELD.LED2_GREEN = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED2_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.A_LL_RED_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED2_RED) {
              tm1639_io3.data.NAME_FIELD.LED2_RED = 0;
            } else {
              tm1639_io3.data.NAME_FIELD.LED2_RED = 1;
            }
          }
          if (pSystem->setValue.NAME_FIELD.A_HH_GREEN_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED3_GREEN)
              tm1639_io3.data.NAME_FIELD.LED3_GREEN = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED3_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.A_HH_RED_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED3_RED) {
              tm1639_io3.data.NAME_FIELD.LED3_RED = 0;
            } else {
              tm1639_io3.data.NAME_FIELD.LED3_RED = 1;
            }
          }
          if (pSystem->setValue.NAME_FIELD.A_PC_GREEN_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED4_GREEN)
              tm1639_io3.data.NAME_FIELD.LED4_GREEN = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED4_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.A_PC_RED_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED4_RED)
              tm1639_io3.data.NAME_FIELD.LED4_RED = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED4_RED = 1;
          }
          if (pSystem->setValue.NAME_FIELD.BHT_GREEN_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED5_GREEN)
              tm1639_io3.data.NAME_FIELD.LED5_GREEN = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED5_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.BHT_RED_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED5_RED)
              tm1639_io3.data.NAME_FIELD.LED5_RED = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED5_RED = 1;
          }
          if (pSystem->setValue.NAME_FIELD.BLK_GREEN_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED6_GREEN)
              tm1639_io3.data.NAME_FIELD.LED6_GREEN = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED6_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.BLK_RED_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED6_RED) {
              tm1639_io3.data.NAME_FIELD.LED6_RED = 0;
            } else {
              tm1639_io3.data.NAME_FIELD.LED6_RED = 1;
            }
          }
          if (pSystem->setValue.NAME_FIELD.BBR_GREEN_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED7_GREEN)
              tm1639_io3.data.NAME_FIELD.LED7_GREEN = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED7_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.BBR_RED_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED7_RED) {
              tm1639_io3.data.NAME_FIELD.LED7_RED = 0;
            } else {
              tm1639_io3.data.NAME_FIELD.LED7_RED = 1;
            }
          }
          if (pSystem->setValue.NAME_FIELD.ABR_GREEN_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED8_GREEN)
              tm1639_io3.data.NAME_FIELD.LED8_GREEN = 0;
            else
              tm1639_io3.data.NAME_FIELD.LED8_GREEN = 1;
          } else if (pSystem->setValue.NAME_FIELD.ABR_RED_TOGGLE) {
            if (tm1639_io3.data.NAME_FIELD.LED8_RED) {
              tm1639_io3.data.NAME_FIELD.LED8_RED = 0;
            } else {
              tm1639_io3.data.NAME_FIELD.LED8_RED = 1;
            }
          }
          pre_time = HAL_GetTick();
          if (pELTop->userData.channel == 0) {
            displayCase = 0;
          } else if (pELTop->userData.channel == 1) {
            displayCase = 1;
          } else {
            displayCnt++;
            if (displayCnt > 4) {
              displayCnt = 0;
              displayCase ^= 1;
            }
          }
          StandByLedDisplay(pSystem, displayCase);
          StandByTempDisplay(pSystem, displayCase);
          StandByLevelDisplay(pSystem, pELTop);
        }
        break;
    }
  }
}

/**
 * @brief 첫 버튼 이 눌릴때의 제어
 * @details INIT_VIEW 나 STAND_BY 에서 버튼이 눌렸을때 각 메뉴로 들어가게 된다.
 * @param args SYSTEM_t 구조체 SET_DATA_t 구조체
 * @return none
 */
void firstSetup(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  if (pSystem->buttonVaule.NAME_FIELD.Button_SET5s) {
    pELTop->remoteData.remoteCnt = 200;
    pELTop->remoteData.setData = USER_CHANNEL_SELECTION;
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    pELTop->remoteData.tempStep = 1;
    pELTop->remoteData.read_data = pELTop->userData.channel;
    channel_display(pELTop);
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
    UserTempStartFunc(pSystem, pELTop);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_DN5s) {
    pELTop->remoteData.remoteCnt = 200;
    pELTop->remoteData.setData = SD_CARD_SET;
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    if (pELTop->sdData.sdFlag) {
      tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "SD-");
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
    } else {
      tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "SD-");
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
    }
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_DN10s) {
    FactoryCalibrationStartFunc(pSystem, pELTop);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_UP5s) {
    UserLevelStartFunc(pSystem, pELTop);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP5chk) {
    pELTop->remoteData.remoteCnt = 200;
    pELTop->remoteData.setData = A_B_L_H_SETCHK;
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    pELTop->remoteData.tempStep = 1;
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A-L");
    tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->levData.aStartMeterSet / 100));
    // tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->levData.aStartMeterSet);
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN5chk) {
    pELTop->remoteData.setData = RTC_SET;
    pSystem->rtcValue.rtcStatus = 2;
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    pELTop->remoteData.tempStep = 1;
    pELTop->remoteData.read_data = pSystem->rtcValue.rtc.Year;
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Yer");
    tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN5s) {
    UserMenuStartFunc(pSystem, pELTop);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP5s) {
    pELTop->remoteData.remoteCnt = 200;
    pELTop->remoteData.setData = RS485_SET;
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    pELTop->remoteData.tempStep = 1;
    pELTop->remoteData.read_data = pELTop->userData.rs485Id;
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ID-");
    tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
    NVIC_SystemReset();
  }
}

/**
 * @brief 버튼 이 눌릴때의 제어
 * @details 버튼이 눌리면 다양한 메뉴로 들어가게 된다
 * @param args SYSTEM_t 구조체 SET_DATA_t 구조체
 * @return none
 */
void Remote_Control(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {

  switch (pELTop->remoteData.setData) {
    case INIT_VIEW:
      switch (pELTop->remoteData.tempStep) {
        case 0:
          tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "EL-");
          tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "TOP");
          pELTop->remoteData.stepCnt = HAL_GetTick();
          pELTop->remoteData.tempStep++;
          break;
        case 1:
          if (HAL_GetTick() - pELTop->remoteData.stepCnt >= 3000) {
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ID-");
            tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->userData.rs485Id);
            pELTop->remoteData.stepCnt = HAL_GetTick();
            pELTop->remoteData.tempStep++;
          }
          break;
        case 2:
          if (HAL_GetTick() - pELTop->remoteData.stepCnt >= 3000) {
            pELTop->remoteData.stepCnt = HAL_GetTick();
            BUZZER_Control(ON, 200);
            FND_Reset(&tm1639_io3);
            FND_Reset(&tm1639_io4);
            pELTop->remoteData.tempStep++;
          }
          break;
        case 3:
          if (HAL_GetTick() - pELTop->remoteData.stepCnt >= 200) {
            pELTop->remoteData.setData = STAND_BY;
            pELTop->remoteData.stepCnt = 0;
            pELTop->remoteData.tempStep = 0;
            FND_Reset(&tm1639_io3);
            FND_Reset(&tm1639_io4);
          }
          break;
      }
      firstSetup(pSystem, pELTop);
      break;
    case STAND_BY:
      firstSetup(pSystem, pELTop);
      break;
    case USER_MENU_SET:
      UserMenuSetFunc(pSystem, pELTop);
      break;
    case RS485_SET:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        pELTop->remoteData.remoteCnt = 0;
        pELTop->remoteData.resetCnt = 0;
        BUZZER_Control(ON, 100);
        pELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        pELTop->remoteData.remoteCnt = 200;
        switch (pELTop->remoteData.tempStep) {
          case 1:
            pELTop->remoteData.read_data = circularValue(64, 1, ++pELTop->remoteData.read_data);
            break;
          case 2:
            pELTop->remoteData.read_data = circularValue(3, 1, ++pELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
        pELTop->remoteData.remoteCnt = 200;
        switch (pELTop->remoteData.tempStep) {
          case 1:
            pELTop->remoteData.read_data = circularValue(64, 1, --pELTop->remoteData.read_data);
            break;
          case 2:
            pELTop->remoteData.read_data = circularValue(3, 1, --pELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        pELTop->remoteData.remoteCnt = 200;
        switch (pELTop->remoteData.tempStep) {
          case 1:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bPS");
            pELTop->userData.rs485Id = pELTop->remoteData.read_data;
            pELTop->remoteData.read_data = pELTop->userData.rs485Bps;
            UserSettingDataFlashSave();
            break;
          case 2:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ID-");
            pELTop->userData.rs485Bps = pELTop->remoteData.read_data;
            pELTop->remoteData.read_data = pELTop->userData.rs485Id;
            UserSettingDataFlashSave();
            break;
        }
        pELTop->remoteData.tempStep++;
        if (pELTop->remoteData.tempStep > 2)
          pELTop->remoteData.tempStep = 1;
        RESET_BUTTON(pSystem);
        BUZZER_Control(ON, 100);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
      } else if (pELTop->remoteData.resetCnt == 1) {
        switch (pELTop->remoteData.tempStep) {
          case 1:
            pELTop->userData.rs485Id = pELTop->remoteData.read_data;
            UserSettingDataFlashSave();
            break;
          case 2:
            pELTop->userData.rs485Bps = pELTop->remoteData.read_data;
            UserSettingDataFlashSave();
            break;
        }
        pELTop->remoteData.resetCnt = 0;
        cliPrintf("STAND_BY Set\n");
        BUZZER_Control(ON, 100);
        pELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      }
      break;
    case A_B_L_H_SETCHK:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        pELTop->remoteData.remoteCnt = 0;
        pELTop->remoteData.resetCnt = 0;
        BUZZER_Control(ON, 100);
        pELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        pELTop->remoteData.remoteCnt = 200;
        switch (pELTop->remoteData.tempStep) {
          case 1:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A-H");
            // tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->levData.aStopMeterSet);
            tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->levData.aStopMeterSet / 100));
            pELTop->remoteData.tempStep++;
            break;
          case 2:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b-L");
            // tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->levData.bStartMeterSet);
            tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->levData.bStartMeterSet / 100));
            pELTop->remoteData.tempStep++;
            break;
          case 3:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b-H");
            // tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->levData.bStopMeterSet);
            tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->levData.bStopMeterSet / 100));
            pELTop->remoteData.tempStep++;
            break;
          case 4:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A-L");
            // tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->levData.aStartMeterSet);
            tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->levData.aStartMeterSet / 100));
            pELTop->remoteData.tempStep = 1;
            break;
        }
        RESET_BUTTON(pSystem);
      } else if (pELTop->remoteData.resetCnt == 1) {
        pELTop->remoteData.resetCnt = 0;
        cliPrintf("STAND_BY Set\n");
        BUZZER_Control(ON, 100);
        pELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      }
      break;
    case SD_CARD_SET:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        pELTop->remoteData.remoteCnt = 0;
        pELTop->remoteData.resetCnt = 0;
        BUZZER_Control(ON, 100);
        pELTop->remoteData.setData = STAND_BY;
        UserSettingDataFlashSave();
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        pELTop->remoteData.remoteCnt = 200;
        pELTop->sdData.sdFlag = !pELTop->sdData.sdFlag;
        if (pELTop->sdData.sdFlag) {
          tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "SD-");
          tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
        } else {
          tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "SD-");
          tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
        }
      } else if (pELTop->remoteData.resetCnt == 1) {
        pELTop->remoteData.resetCnt = 0;
        cliPrintf("STAND_BY Set\n");
        BUZZER_Control(ON, 100);
        pELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      }
      break;
    case RTC_SET:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        pSystem->rtcValue.rtcStatus = 1;
        BUZZER_Control(ON, 100);
        pELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        switch (pELTop->remoteData.tempStep) {
          case 1:
            pELTop->remoteData.read_data = circularValue(99, 0, ++pELTop->remoteData.read_data);
            break;
          case 2:
            pELTop->remoteData.read_data = circularValue(12, 1, ++pELTop->remoteData.read_data);
            break;
          case 3:
            pELTop->remoteData.read_data = circularValue(31, 1, ++pELTop->remoteData.read_data);
            break;
          case 4:
            pELTop->remoteData.read_data = circularValue(7, 1, ++pELTop->remoteData.read_data);
            break;
          case 5:
            pELTop->remoteData.read_data = circularValue(23, 0, ++pELTop->remoteData.read_data);
            break;
          case 6:
          case 7:
            pELTop->remoteData.read_data = circularValue(59, 0, ++pELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
        switch (pELTop->remoteData.tempStep) {
          case 1:
            pELTop->remoteData.read_data = circularValue(99, 0, --pELTop->remoteData.read_data);
            break;
          case 2:
            pELTop->remoteData.read_data = circularValue(12, 1, --pELTop->remoteData.read_data);
            break;
          case 3:
            pELTop->remoteData.read_data = circularValue(31, 1, --pELTop->remoteData.read_data);
            break;
          case 4:
            pELTop->remoteData.read_data = circularValue(7, 1, --pELTop->remoteData.read_data);
            break;
          case 5:
            pELTop->remoteData.read_data = circularValue(23, 0, --pELTop->remoteData.read_data);
            break;
          case 6:
          case 7:
            pELTop->remoteData.read_data = circularValue(59, 0, --pELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        switch (pELTop->remoteData.tempStep) {
          case 1:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "NNo");
            pSystem->rtcValue.rtc.Year = pELTop->remoteData.read_data;
            pELTop->remoteData.read_data = pSystem->rtcValue.rtc.Month;
            break;
          case 2:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "day");
            pSystem->rtcValue.rtc.Month = pELTop->remoteData.read_data;
            pELTop->remoteData.read_data = pSystem->rtcValue.rtc.Date;
            break;
          case 3:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "UUE");
            pSystem->rtcValue.rtc.Date = pELTop->remoteData.read_data;
            pELTop->remoteData.read_data = pSystem->rtcValue.rtc.DaysOfWeek;
            break;
          case 4:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Hor");
            pSystem->rtcValue.rtc.DaysOfWeek = pELTop->remoteData.read_data;
            pELTop->remoteData.read_data = pSystem->rtcValue.rtc.Hour;
            break;
          case 5:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "NNI");
            pSystem->rtcValue.rtc.Hour = pELTop->remoteData.read_data;
            pELTop->remoteData.read_data = pSystem->rtcValue.rtc.Min;
            break;
          case 6:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "SEc");
            pSystem->rtcValue.rtc.Min = pELTop->remoteData.read_data;
            pELTop->remoteData.read_data = pSystem->rtcValue.rtc.Sec;
            break;
          case 7:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Yer");
            pSystem->rtcValue.rtc.Sec = pELTop->remoteData.read_data;
            pELTop->remoteData.read_data = pSystem->rtcValue.rtc.Year;
            break;
        }
        TempSettingDataFlashSave();
        pELTop->remoteData.tempStep++;
        if (pELTop->remoteData.tempStep > 7)
          pELTop->remoteData.tempStep = 1;
        RESET_BUTTON(pSystem);
        BUZZER_Control(ON, 100);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
      }
      break;
    case USER_TEMP_SET:
      UserTempSetFunc(pSystem, pELTop);
      break;
    case USER_CHANNEL_SELECTION:
      if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        pELTop->remoteData.remoteCnt = 200;
        if (pELTop->remoteData.read_data <= 1)
          pELTop->remoteData.read_data++;
        else
          pELTop->remoteData.read_data = 0;
        channel_display(pELTop);
        BUZZER_Control(ON, 100);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        pELTop->remoteData.remoteCnt = 0;
        pELTop->remoteData.resetCnt = 0;
        pELTop->userData.channel = pELTop->remoteData.read_data;
        UserSettingDataFlashSave();
        BUZZER_Control(ON, 100);
        pELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        FND_Reset(&tm1639_io3);
        FND_Reset(&tm1639_io4);
      } else if (pELTop->remoteData.resetCnt == 1) {
        pELTop->userData.channel = pELTop->remoteData.read_data;
        UserSettingDataFlashSave();
        pELTop->remoteData.resetCnt = 0;
        cliPrintf("STAND_BY Set\n");
        BUZZER_Control(ON, 100);
        pELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      }
      break;
    case FACTORY_CALIBRATION:
      FactoryCalibrationSetFunc(pSystem, pELTop);
      break;
    case USER_LEVEL_SET:
      UserLevelSetFunc(pSystem, pELTop);
      break;
  }
}

bool buttonClickedInput(SYSTEM_t *pSystem) {
  bool ret = false;

  tm1639_io3.button = buttonstatusTM1639(&tm1639_io3);
  tm1639_io4.button = buttonstatusTM1639(&tm1639_io4);
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();

  pSystem->buttonVaule.BYTE_FIELD[0] = (tm1639_io3.button & 0x03) | ((tm1639_io4.button & 0x03) << 2);
  static uint8_t button_step = 0;
  static uint8_t btn;
  const uint32_t up_down_pressed_time = 50;
  const uint32_t up_down_repeat_time = 20;
  const uint32_t pressed_time = 100;
  const uint32_t repeat_time = 50;
  static uint32_t pre_time;

  switch (button_step) {
    case 0:
      if (btn != pSystem->buttonVaule.BYTE_FIELD[0]) {
        btn = pSystem->buttonVaule.BYTE_FIELD[0];
        button_step = 1;
        pre_time = HAL_GetTick();
      }
      break;
    case 1:
      if (btn == pSystem->buttonVaule.BYTE_FIELD[0] && btn) {
        if (btn == 0x09 && pELTop->remoteData.setData == FACTORY_CALIBRATION) {
          if (HAL_GetTick() - pre_time >= pressed_time) {
            ret = false;  // 버튼 클릭됨
            button_step = 0;
            pSystem->buttonVaule.NAME_FIELD.Button_SET_DN = 1;
          }
        } else if ((pSystem->buttonVaule.BYTE_FIELD[0] == 0x01) || (pSystem->buttonVaule.BYTE_FIELD[0] == 0x02)) {
          if (pELTop->remoteData.setData == SD_CARD_SET || pELTop->remoteData.setData == USER_MENU_SET) {
            if (HAL_GetTick() - pre_time >= pressed_time) {
              ret = true;  // 버튼 클릭됨
              button_step = 2;
              pre_time = HAL_GetTick();
            }
          } else if (pELTop->remoteData.setData == A_B_L_H_SETCHK) {
            if (HAL_GetTick() - pre_time >= pressed_time) {
              ret = true;  // 버튼 클릭됨
              button_step = 0;
              pre_time = HAL_GetTick();
            }
          } else {
            if (HAL_GetTick() - pre_time >= up_down_pressed_time) {
              ret = true;  // 버튼 클릭됨
              button_step = 2;
              pre_time = HAL_GetTick();
            }
          }
        } else {
          if (HAL_GetTick() - pre_time >= pressed_time) {
            ret = true;  // 버튼 클릭됨
            button_step = 2;
            pre_time = HAL_GetTick();
          }
        }

      } else {
        button_step = 0;
      }
      break;
    case 2:
      if (btn == pSystem->buttonVaule.BYTE_FIELD[0] && btn) {
        if (((btn & pSystem->buttonVaule.BYTE_FIELD[0]) == 0x01) || ((btn & pSystem->buttonVaule.BYTE_FIELD[0]) == 0x02)) {
          if (HAL_GetTick() - pre_time >= up_down_repeat_time) {
            button_step = 1;
            pre_time = HAL_GetTick();
          }
        } else {
          if (HAL_GetTick() - pre_time >= repeat_time) {
            button_step = 1;
            pre_time = HAL_GetTick();
          }
        }
      } else {
        button_step = 0;
      }
      break;
  }
  return ret;
}

/**
 * @brief TM1639 버튼 입력
 * @details 최초 버튼 클릭 및 연속버튼 입력을 받음
 * @param args SYSTEM_t 구조체
 * @return 인식없음 0 버튼이 클릭되면 1, 연속버튼이클릭되면 2
 */
uint8_t buttonGetInput(SYSTEM_t *pSystem) {
  uint8_t ret = false;

  tm1639_io3.button = buttonstatusTM1639(&tm1639_io3);
  tm1639_io4.button = buttonstatusTM1639(&tm1639_io4);

  pSystem->buttonVaule.BYTE_FIELD[0] = (tm1639_io3.button & 0x03) | ((tm1639_io4.button & 0x03) << 2);
  static uint8_t button_step = 0;
  static uint8_t btn;
  const uint32_t pressed_time = 100;
  const uint32_t continuousInput10s = 10000;
  const uint32_t continuousInput5s = 5000;
  static uint32_t pre_time;

  switch (button_step) {
    case 0:
      if (btn != pSystem->buttonVaule.BYTE_FIELD[0]) {
        btn = pSystem->buttonVaule.BYTE_FIELD[0];
        button_step = 1;
        pre_time = HAL_GetTick();
      }
      break;
    case 1:
      if (btn == pSystem->buttonVaule.BYTE_FIELD[0]) {
        if (HAL_GetTick() - pre_time >= pressed_time) {
          ret = 1;  // 버튼 클릭됨
          button_step = 2;
          pre_time = HAL_GetTick();
        }
      } else {
        button_step = 0;
      }
      break;
    case 2:
      if (btn == pSystem->buttonVaule.BYTE_FIELD[0]) {
        ret = 1;  // 버튼 클릭됨
        if (btn == 0x01 || btn == 0x02 || btn == 0x03 || btn == 0x04 || btn == 0x08 || btn == 0x09 || btn == 0x0A) {
          if (HAL_GetTick() - pre_time >= continuousInput5s) {
            ret = 2;  // 버튼 클릭됨
            pre_time = HAL_GetTick();
          }
        } else if (btn == 0x01 || btn == 0x06) {
          if (HAL_GetTick() - pre_time >= continuousInput10s) {
            ret = 2;  // 버튼 클릭됨
            pre_time = HAL_GetTick();
          }
        } else {
          button_step = 0;
        }
      } else if ((btn == 0x01 && 0x03 == pSystem->buttonVaule.BYTE_FIELD[0]) || (btn == 0x02 && 0x03 == pSystem->buttonVaule.BYTE_FIELD[0])
          || (btn == 0x01 && 0x09 == pSystem->buttonVaule.BYTE_FIELD[0]) || (btn == 0x08 && 0x09 == pSystem->buttonVaule.BYTE_FIELD[0])
          || (btn == 0x02 && 0x0A == pSystem->buttonVaule.BYTE_FIELD[0]) || (btn == 0x08 && 0x0A == pSystem->buttonVaule.BYTE_FIELD[0])) {
        btn = pSystem->buttonVaule.BYTE_FIELD[0];
        pre_time = HAL_GetTick();
        ret = 1;
      } else {
        button_step = 0;
      }
      break;
  }
  return ret;
}

/**
 * @brief TM1639 연속버튼 입력
 * @details 연속 버튼 입력시간이 지난후 버튼 등록
 * @param args SYSTEM_t 구조체
 * @return none
 */
void ContinuousButtonPress(SYSTEM_t *pSystem) {
  switch (pSystem->buttonVaule.BYTE_FIELD[0]) {
    case 0x08:
      pSystem->buttonVaule.NAME_FIELD.Button_SET5s = 1;
      break;
    case 0x03:
      pSystem->buttonVaule.NAME_FIELD.Button_UP_DN5s = 1;
      break;
    case 0x09:
      pSystem->buttonVaule.NAME_FIELD.Button_SET_DN10s = 1;
      break;
    case 0x0A:
      pSystem->buttonVaule.NAME_FIELD.Button_SET_UP5s = 1;
      break;
    case 0x01:
      pSystem->buttonVaule.NAME_FIELD.Button_DOWN5s = 1;
      break;
    case 0x02:
      pSystem->buttonVaule.NAME_FIELD.Button_UP5s = 1;
      break;
    case 0x04:
      pSystem->buttonVaule.NAME_FIELD.Button_RESET5s = 1;
      break;
  }
}

/**
 * @brief TM1639 초기화
 * @details TM1639 GPIO 초기화 및 Init
 * @param args none
 * @return none
 */
void initTm1639(void) {
  tm1639_io3.clk_gpio = GPIOB;
  tm1639_io3.clk_pin = GPIO_PIN_8;
  tm1639_io3.dio_gpio = GPIOB;
  tm1639_io3.dio_pin = GPIO_PIN_9;
  tm1639_io3.stb_gpio = GPIOC;
  tm1639_io3.stb_pin = GPIO_PIN_14;
  initTM1639(&tm1639_io3);

  tm1639_io4.clk_gpio = GPIOA;
  tm1639_io4.clk_pin = GPIO_PIN_11;
  tm1639_io4.dio_gpio = GPIOA;
  tm1639_io4.dio_pin = GPIO_PIN_12;
  tm1639_io4.stb_gpio = GPIOC;
  tm1639_io4.stb_pin = GPIO_PIN_15;
  initTM1639(&tm1639_io4);

}

void Remote_Task(void *argument) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  Buzzer_ClearTimer_ID = osTimerNew(Control_Oneshot_Timer_Callback, osTimerOnce, (void*) CONTROL_TIMER_BUZZER_CLEAR, NULL);
  bool ret;
  uint8_t res;

#ifdef _USE_CLI
  cliAdd("remote", cliRemote);
#endif

  initTm1639();

  while (1) {
    if (pELTop->remoteData.setData == STAND_BY || pELTop->remoteData.setData == INIT_VIEW || pSystem->buttonVaule.NAME_FIELD.RES2) {
      res = buttonGetInput(pSystem);
      if (res == 1) {
        pSystem->buttonVaule.NAME_FIELD.RES2 = (tm1639_io3.button & 0x03) | ((tm1639_io4.button & 0x03) << 2);
      } else if (res == 2) {
        ContinuousButtonPress(pSystem);
      } else {
        if (pELTop->remoteData.setData == STAND_BY || pELTop->remoteData.setData == INIT_VIEW) {
          pSystem->buttonVaule.BYTE_FIELD[1] = pSystem->buttonVaule.NAME_FIELD.RES2;
          if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK == 1) {
            pSystem->buttonVaule.NAME_FIELD.Button_DOWNcnt++;
            pSystem->buttonVaule.NAME_FIELD.RES2 = 0;
            if (pSystem->buttonVaule.NAME_FIELD.Button_DOWNcnt >= 5) {
              pSystem->buttonVaule.NAME_FIELD.Button_DOWNcnt = 0;
              pSystem->buttonVaule.NAME_FIELD.Button_DOWN5chk = 1;
            }
          }
          if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK == 1) {
            pSystem->buttonVaule.NAME_FIELD.Button_UPcnt++;
            pSystem->buttonVaule.NAME_FIELD.RES2 = 0;
            if (pSystem->buttonVaule.NAME_FIELD.Button_UPcnt >= 5) {
              pSystem->buttonVaule.NAME_FIELD.Button_UPcnt = 0;
              pSystem->buttonVaule.NAME_FIELD.Button_UP5chk = 1;
            }
          }
        } else
          pSystem->buttonVaule.NAME_FIELD.RES2 = 0;
      }
    } else {
      ret = buttonClickedInput(pSystem);
      if (ret == true) {
        pSystem->buttonVaule.BYTE_FIELD[1] = (tm1639_io3.button & 0x03) | ((tm1639_io4.button & 0x03) << 2);
      } else {
        pSystem->buttonVaule.BYTE_FIELD[1] = 0;
      }
    }
    led_Control(pSystem, pELTop);
    Remote_Control(pSystem, pELTop);
    osDelay(10);
  }
}

void REMOTE_Create_Thread(void) {
  task_Remote_Handle = osThreadNew(Remote_Task, NULL, &taskRemote_attributes);
}

static void Control_Oneshot_Timer_Callback(void *arg) {
  uint32_t check = (uint32_t) arg;

  if (check == CONTROL_TIMER_BUZZER_CLEAR) {
    BUZZER_Control(OFF, 0);
  }
}

#ifdef _USE_CLI
void cliRemote(uint8_t argc, const char **argv) {
  bool ret = false;

  if (argc == 1 && cliIsStr(argv[0], "show")) {
    while (cliKeepLoop()) {
      cliPrintf("\n");
      osDelay(100);
    }

    ret = true;
  }

  if (ret != true) {
    cliPrintf("control show\n");
  }
}
#endif
