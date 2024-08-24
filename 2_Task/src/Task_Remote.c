/*
 * Task_Remote.c
 *
 *  Created on: Aug 24, 2024
 *      Author: USER
 */
#include "database.h"
#include "Task_Input.h"
#include "Task_Cli.h"
#include "com_flash.h"
#include "tm1639.h"

#define IO4_AB_MO_FND 0
#define IO4_B_LEVEL_FND 1
#define IO3_AB_BR_FND 0
#define IO3_A_LEVEL_FND 1

#ifdef _USE_CLI
static void cliRemote(uint8_t argc, const char **argv);
#endif

osThreadId_t task_Remote_Handle;
const osThreadAttr_t taskRemote_attributes = { .name = "Remote Thread", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };

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

void RESET_BUTTON(_SYSTEM_t *SystemData) {
  SystemData->buttonVaule.NAME_FIELD.Button_UP_CHK = 0;
  SystemData->buttonVaule.NAME_FIELD.Button_DOWN_CHK = 0;
  SystemData->buttonVaule.NAME_FIELD.Button_SET_CHK = 0;
  SystemData->buttonVaule.NAME_FIELD.Button_RESET_CHK = 0;
  SystemData->buttonVaule.BYTE_FIELD[2] = 0;
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

void channel_display(SET_DATA_t *ELTop) {
  switch (ELTop->remoteData.read_data) {
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
      ELTop->userData.channel = 0;
      ELTop->remoteData.read_data = ELTop->userData.channel;
      break;
  }
}

void led_Control(_SYSTEM_t *pSystem, SET_DATA_t *ELTop) {
  static uint8_t ledStep = 0;
  static uint32_t pre_time;
  if (ELTop->remoteData.setData == STAND_BY || ELTop->remoteData.setData == INIT_VIEW) {
    switch (ledStep) {
      case 0:
        pSystem->setValue.NAME_FIELD.BMO_GREEN_TOGGLE = 1;
        pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE = 1;
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
            tm1639Display_led(&tm1639_io4);

            pre_time = HAL_GetTick();
          } else if (pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED1_RED)
              tm1639_io4.data.NAME_FIELD.LED1_RED = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED1_RED = 1;
            tm1639Display_led(&tm1639_io4);

            pre_time = HAL_GetTick();
          }
          if (pSystem->setValue.NAME_FIELD.BMO_GREEN_TOGGLE) {
            if (tm1639_io4.data.NAME_FIELD.LED2_GREEN)
              tm1639_io4.data.NAME_FIELD.LED2_GREEN = 0;
            else
              tm1639_io4.data.NAME_FIELD.LED2_GREEN = 1;
            tm1639Display_led(&tm1639_io4);

            pre_time = HAL_GetTick();
          }
        }
        break;
    }
  }
}

void con(_SYSTEM_t *pSystem, SET_DATA_t *ELTop) {
  if (pSystem->buttonVaule.NAME_FIELD.Button_SET5s) {
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    ELTop->remoteData.tempStep = 1;
    ELTop->remoteData.setData = USER_CHANNEL_SELECTION;
    ELTop->remoteData.read_data = ELTop->userData.channel;
    channel_display(ELTop);
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    ELTop->remoteData.tempStep = 1;
    ELTop->remoteData.read_data = ELTop->tempData.aMoTemp;
    tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
    tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, ELTop->remoteData.read_data);
    cliPrintf("read_data : %d\n", ELTop->remoteData.read_data);
    ELTop->remoteData.setData = USER_TEMP_SET;
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_DN5s) {
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    ELTop->sdData.sdFlag = !ELTop->sdData.sdFlag;
    ELTop->remoteData.stepCnt = 0;
    ELTop->remoteData.setData = SD_CARD_SET;
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_DN10s) {
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    ELTop->remoteData.tempStep = 1;
    ELTop->remoteData.read_data = ELTop->calData.a1_LowOffSet;
    cliPrintf("read_temp : %d, tempStep : %d \n", ELTop->remoteData.read_data, ELTop->remoteData.tempStep);
    ELTop->remoteData.setData = FACTORY_CALIBRATION;
    RESET_BUTTON(pSystem);
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A1L");
    tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_UP5s) {
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    ELTop->remoteData.tempStep = 1;
    ELTop->remoteData.read_data = ELTop->levData.selectedSensorA;
    cliPrintf("read_temp : %d, tempStep : %d \n", ELTop->remoteData.read_data, ELTop->remoteData.tempStep);
    ELTop->remoteData.setData = USER_LEVEL_SET;
    RESET_BUTTON(pSystem);
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Asr");
    tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    ELTop->remoteData.tempStep = 1;
    tm1639Display_num(&tm1639_io4, 0, ELTop->levData.aStopMeterSet);
    tm1639Display_num(&tm1639_io4, 1, ELTop->levData.aStartMeterSet);
    ELTop->remoteData.setData = A_B_L_H_SETCHK;
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    ELTop->remoteData.tempStep = 1;
    ELTop->remoteData.read_data = pSystem->rtcValue.rtc.Year;
    ELTop->remoteData.setData = RTC_SET;
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Y");
    tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN5s) {
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    ELTop->remoteData.tempStep = 1;
    ELTop->remoteData.read_data = ELTop->userData.AutoReset;
    ELTop->remoteData.setData = USER_MENU_SET;
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "rSt");
    if (ELTop->remoteData.read_data)
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
    else
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP5s) {
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
    BUZZER_Control(ON, 100);
    ELTop->remoteData.tempStep = 1;
    ELTop->remoteData.read_data = ELTop->userData.rs485Id;
    ELTop->remoteData.setData = RS485_SET;
    tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ID-");
    tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
  }
}

void Remote_Control(_SYSTEM_t *pSystem, SET_DATA_t *ELTop) {
//  SET_DATA_t *ELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();

  switch (ELTop->remoteData.setData) {
    case INIT_VIEW:
      switch (ELTop->remoteData.tempStep) {
        case 0:
          tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "EL-");
          tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "TOP");
          ELTop->remoteData.stepCnt = HAL_GetTick();
          ELTop->remoteData.tempStep++;
          break;
        case 1:
          if (HAL_GetTick() - ELTop->remoteData.stepCnt >= 5000) {
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ID-");
            tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->userData.rs485Id);
            ELTop->remoteData.stepCnt = HAL_GetTick();
            ELTop->remoteData.tempStep++;
          }
          break;
        case 2:
          if (HAL_GetTick() - ELTop->remoteData.stepCnt >= 5000) {
            ELTop->remoteData.setData = STAND_BY;
            ELTop->remoteData.stepCnt = 0;
            ELTop->remoteData.tempStep = 0;
            FND_Reset(&tm1639_io3);
            FND_Reset(&tm1639_io4);
          }
          break;
      }
      con(pSystem, ELTop);
      break;
    case STAND_BY:
      con(pSystem, ELTop);
      break;
    case USER_MENU_SET:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        BUZZER_Control(ON, 100);
        ELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            ELTop->remoteData.read_data = circularValue(1, 0, ++ELTop->remoteData.read_data);
            break;
          case 2:
            ELTop->remoteData.read_data = circularValue(60, 0, ++ELTop->remoteData.read_data);
            break;
          case 3:
          case 4:
          case 5:
          case 6:
            ELTop->remoteData.read_data = circularValue(10, -50, ++ELTop->remoteData.read_data);
            break;
        }

        if (ELTop->remoteData.tempStep != 1)
          tm1639Display_num(&tm1639_io4, 0, ELTop->remoteData.read_data);
        else {
          if (ELTop->remoteData.read_data)
            tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
          else
            tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
        }
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            ELTop->remoteData.read_data = circularValue(1, 0, --ELTop->remoteData.read_data);
            break;
          case 2:
            ELTop->remoteData.read_data = circularValue(60, 0, --ELTop->remoteData.read_data);
            break;
          case 3:
          case 4:
          case 5:
          case 6:
            ELTop->remoteData.read_data = circularValue(10, -50, --ELTop->remoteData.read_data);
            break;
        }
        if (ELTop->remoteData.tempStep != 1)
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
        else {
          if (ELTop->remoteData.read_data)
            tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
          else
            tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
        }
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "trd");
            ELTop->userData.AutoReset = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->userData.TripOnDelay;
            break;
          case 2:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AJ1");
            ELTop->userData.TripOnDelay = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->userData.aMoLineResAdj;
            break;
          case 3:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AJ2");
            ELTop->userData.aMoLineResAdj = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->userData.aBrLineResAdj;
            break;
          case 4:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bJ1");
            ELTop->userData.aBrLineResAdj = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->userData.bMoLineResAdj;
            break;
          case 5:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bJ2");
            ELTop->userData.bMoLineResAdj = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->userData.bBrLineResAdj;
            break;
          case 6:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "rST");
            ELTop->userData.bBrLineResAdj = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->userData.AutoReset;
            if (ELTop->remoteData.read_data)
              tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
            else
              tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
            break;
        }
        UserSettingDataFlashSave();
        ELTop->remoteData.tempStep++;
        if (ELTop->remoteData.tempStep > 6)
          ELTop->remoteData.tempStep = 1;
        RESET_BUTTON(pSystem);
        BUZZER_Control(ON, 100);
        if (ELTop->remoteData.tempStep != 1)
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
      }
      break;
    case RS485_SET:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        BUZZER_Control(ON, 100);
        ELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            ELTop->remoteData.read_data = circularValue(64, 1, ++ELTop->remoteData.read_data);
            break;
          case 2:
            ELTop->remoteData.read_data = circularValue(3, 1, ++ELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            ELTop->remoteData.read_data = circularValue(64, 1, --ELTop->remoteData.read_data);
            break;
          case 2:
            ELTop->remoteData.read_data = circularValue(3, 1, --ELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bPS");
            ELTop->userData.rs485Id = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->userData.rs485Bps;
            break;
          case 2:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ID-");
            ELTop->userData.rs485Bps = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->userData.rs485Id;
            break;
        }
        UserSettingDataFlashSave();
        ELTop->remoteData.tempStep++;
        if (ELTop->remoteData.tempStep > 2)
          ELTop->remoteData.tempStep = 1;
        RESET_BUTTON(pSystem);
        BUZZER_Control(ON, 100);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
      }
      break;
    case A_B_L_H_SETCHK:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        BUZZER_Control(ON, 100);
        ELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        if (ELTop->remoteData.tempStep == 1) {
          tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, ELTop->levData.bStopMeterSet);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->levData.bStartMeterSet);
          BUZZER_Control(ON, 100);
          ELTop->remoteData.setData = A_B_L_H_SETCHK;
          RESET_BUTTON(pSystem);
          ELTop->remoteData.tempStep = 0;
        } else {
          tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, ELTop->levData.aStopMeterSet);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->levData.aStartMeterSet);
          BUZZER_Control(ON, 100);
          ELTop->remoteData.setData = A_B_L_H_SETCHK;
          RESET_BUTTON(pSystem);
          ELTop->remoteData.tempStep = 1;
        }
      }
      break;
    case SD_CARD_SET:
      if (ELTop->remoteData.stepCnt >= 100) {
        ELTop->remoteData.setData = STAND_BY;
        ELTop->remoteData.stepCnt = 0;
        FND_Reset(&tm1639_io3);
        FND_Reset(&tm1639_io4);
      } else {
        if (ELTop->sdData.sdFlag) {
          tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "SD-");
          tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
        } else {
          tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "SD-");
          tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
        }
      }
      ELTop->remoteData.stepCnt++;
      break;
    case RTC_SET:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        pSystem->rtcValue.rtcStatus = 1;
        ELTop->remoteData.remoteCnt = 0;
        BUZZER_Control(ON, 100);
        ELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            ELTop->remoteData.read_data = circularValue(99, 0, ++ELTop->remoteData.read_data);
            break;
          case 2:
            ELTop->remoteData.read_data = circularValue(12, 1, ++ELTop->remoteData.read_data);
            break;
          case 3:
            ELTop->remoteData.read_data = circularValue(31, 1, ++ELTop->remoteData.read_data);
            break;
          case 4:
            ELTop->remoteData.read_data = circularValue(7, 1, ++ELTop->remoteData.read_data);
            break;
          case 5:
            ELTop->remoteData.read_data = circularValue(23, 0, ++ELTop->remoteData.read_data);
            break;
          case 6:
          case 7:
            ELTop->remoteData.read_data = circularValue(59, 0, ++ELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            ELTop->remoteData.read_data = circularValue(99, 0, --ELTop->remoteData.read_data);
            break;
          case 2:
            ELTop->remoteData.read_data = circularValue(12, 1, --ELTop->remoteData.read_data);
            break;
          case 3:
            ELTop->remoteData.read_data = circularValue(31, 1, --ELTop->remoteData.read_data);
            break;
          case 4:
            ELTop->remoteData.read_data = circularValue(7, 1, --ELTop->remoteData.read_data);
            break;
          case 5:
            ELTop->remoteData.read_data = circularValue(23, 0, --ELTop->remoteData.read_data);
            break;
          case 6:
          case 7:
            ELTop->remoteData.read_data = circularValue(59, 0, --ELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "t");
            pSystem->rtcValue.rtc.Year = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = pSystem->rtcValue.rtc.Month;
            break;
          case 2:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "D");
            pSystem->rtcValue.rtc.Month = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = pSystem->rtcValue.rtc.Date;
            break;
          case 3:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "E");
            pSystem->rtcValue.rtc.Date = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = pSystem->rtcValue.rtc.DaysOfWeek;
            break;
          case 4:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "H");
            pSystem->rtcValue.rtc.DaysOfWeek = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = pSystem->rtcValue.rtc.Hour;
            break;
          case 5:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "i");
            pSystem->rtcValue.rtc.Hour = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = pSystem->rtcValue.rtc.Min;
            break;
          case 6:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "S");
            pSystem->rtcValue.rtc.Min = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = pSystem->rtcValue.rtc.Sec;
            break;
          case 7:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Y");
            pSystem->rtcValue.rtc.Sec = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = pSystem->rtcValue.rtc.Year;
            break;
        }
        TempSettingDataFlashSave();
        ELTop->remoteData.tempStep++;
        if (ELTop->remoteData.tempStep > 7)
          ELTop->remoteData.tempStep = 1;
        RESET_BUTTON(pSystem);
        BUZZER_Control(ON, 100);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
      }
      break;
    case USER_TEMP_SET:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        BUZZER_Control(ON, 100);
        ELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        if (ELTop->remoteData.tempStep <= 4)
          ELTop->remoteData.read_data = circularValue(280, -20, ++ELTop->remoteData.read_data);
        else
          ELTop->remoteData.read_data = circularValue(50, 0, ++ELTop->remoteData.read_data);
        cliPrintf("Button_UP_CHK read_data : %d tempStep : %d\n", ELTop->remoteData.read_data, ELTop->remoteData.tempStep);
        if (ELTop->remoteData.tempStep % 2 == 1) {
          Full_Reset(&tm1639_io3);
          tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, ELTop->remoteData.read_data);
        } else {
          Full_Reset(&tm1639_io4);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
        }
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        if (ELTop->remoteData.tempStep <= 4)
          ELTop->remoteData.read_data = circularValue(280, -20, --ELTop->remoteData.read_data);
        else
          ELTop->remoteData.read_data = circularValue(50, 0, --ELTop->remoteData.read_data);
        cliPrintf("Button_DOWN_CHK read_data : %d, tempStep : %d\n", ELTop->remoteData.read_data, ELTop->remoteData.tempStep);
        if (ELTop->remoteData.tempStep % 2 == 1) {
          Full_Reset(&tm1639_io3);
          tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, ELTop->remoteData.read_data);
        } else {
          Full_Reset(&tm1639_io4);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
        }
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            ELTop->tempData.aMoTemp = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->tempData.aBrTemp;
            break;
          case 2:
            ELTop->tempData.aBrTemp = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->tempData.bMoTemp;
            break;
          case 3:
            ELTop->tempData.bMoTemp = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->tempData.bBrTemp;
            break;
          case 4:
            ELTop->tempData.bBrTemp = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->tempData.aHtOnTemp;
            break;
          case 5:
            ELTop->tempData.aHtOnTemp = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->tempData.aHtOffTemp;
            break;
          case 6:
            ELTop->tempData.aHtOffTemp = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->tempData.bHtOnTemp;
            break;
          case 7:
            ELTop->tempData.bHtOnTemp = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->tempData.bHtOffTemp;
            break;
          case 8:
            ELTop->tempData.bHtOffTemp = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->tempData.aMoTemp;
            break;
        }
        TempSettingDataFlashSave();
        ELTop->remoteData.tempStep++;
        if (ELTop->remoteData.tempStep > 8)
          ELTop->remoteData.tempStep = 1;
        RESET_BUTTON(pSystem);
        BUZZER_Control(ON, 100);
        if (ELTop->remoteData.tempStep % 2 == 1) {
          Full_Reset(&tm1639_io3);
          tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, ELTop->remoteData.read_data);
        } else {
          Full_Reset(&tm1639_io4);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
        }
      }
      break;
    case USER_CHANNEL_SELECTION:
      if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        if (ELTop->remoteData.read_data <= 1)
          ELTop->remoteData.read_data++;
        else
          ELTop->remoteData.read_data = 0;
        channel_display(ELTop);
        ELTop->remoteData.remoteCnt = 0;
        BUZZER_Control(ON, 100);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        ELTop->userData.channel = ELTop->remoteData.read_data;
        UserSettingDataFlashSave();
        ELTop->remoteData.remoteCnt = 0;
        BUZZER_Control(ON, 100);
        ELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        FND_Reset(&tm1639_io3);
        FND_Reset(&tm1639_io4);
      }
      break;
    case FACTORY_CALIBRATION:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        BUZZER_Control(ON, 100);
        ELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        ELTop->remoteData.read_data = circularValue(999, 0, ++ELTop->remoteData.read_data);
        cliPrintf("Button_UP_CHK read_data : %d tempStep : %d\n", ELTop->remoteData.read_data, ELTop->remoteData.tempStep);
        tm1639Display_num(&tm1639_io4, 0, ELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        ELTop->remoteData.read_data = circularValue(999, 0, --ELTop->remoteData.read_data);
        cliPrintf("Button_DOWN_CHK read_data : %d, tempStep : %d\n", ELTop->remoteData.read_data, ELTop->remoteData.tempStep);
        tm1639Display_num(&tm1639_io4, 0, ELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A1H");
            ELTop->calData.a1_LowOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.a1_HighOffSet;
            break;
          case 2:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A2L");
            ELTop->calData.a1_HighOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.a2_LowOffSet;
            break;
          case 3:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A2H");
            ELTop->calData.a2_LowOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.a2_HighOffSet;
            break;
          case 4:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b1L");
            ELTop->calData.a2_HighOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.b1_LowOffSet;
            break;
          case 5:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b1H");
            ELTop->calData.b1_LowOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.b1_HighOffSet;
            break;
          case 6:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b2L");
            ELTop->calData.b1_HighOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.b2_LowOffSet;
            break;
          case 7:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b2H");
            ELTop->calData.b2_LowOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.b2_HighOffSet;
            break;
          case 8:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "c1L");
            ELTop->calData.b2_HighOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.c1_LowOffSet;
            break;
          case 9:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "c1H");
            ELTop->calData.c1_LowOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.c1_HighOffSet;
            break;
          case 10:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "c2L");
            ELTop->calData.c1_HighOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.c2_LowOffSet;
            break;
          case 11:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "c2H");
            ELTop->calData.c2_LowOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.c2_HighOffSet;
            break;
          case 12:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "d1L");
            ELTop->calData.c2_HighOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.d1_LowOffSet;
            break;
          case 13:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "d1H");
            ELTop->calData.d1_LowOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.d1_HighOffSet;
            break;
          case 14:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "d2L");
            ELTop->calData.d1_HighOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.d2_LowOffSet;
            break;
          case 15:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "d2H");
            ELTop->calData.d2_LowOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.d2_HighOffSet;
            break;
          case 16:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A1L");
            ELTop->calData.d2_HighOffSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->calData.a1_LowOffSet;
            break;
        }
        CalSettingDataFlashSave();
        ELTop->remoteData.tempStep++;
        if (ELTop->remoteData.tempStep > 16)
          ELTop->remoteData.tempStep = 1;
        RESET_BUTTON(pSystem);
        BUZZER_Control(ON, 100);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
      }

      break;
    case USER_LEVEL_SET:
      if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        BUZZER_Control(ON, 100);
        ELTop->remoteData.setData = STAND_BY;
        RESET_BUTTON(pSystem);
        Full_Reset(&tm1639_io3);
        Full_Reset(&tm1639_io4);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 2:
          case 10:
            ELTop->remoteData.read_data = circularValue(100, -99, ++ELTop->remoteData.read_data);
            break;
          case 9:
          case 1:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
          case 11:
          case 12:
          case 13:
          case 14:
          case 15:
            ELTop->remoteData.read_data = circularValue(999, 0, ++ELTop->remoteData.read_data);
            break;
          case 8:
          case 16:
            ELTop->remoteData.read_data = circularValue(180, 0, ++ELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io4, 0, ELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 2:
          case 10:
            ELTop->remoteData.read_data = circularValue(100, -99, --ELTop->remoteData.read_data);
            break;
          case 9:
          case 1:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
          case 11:
          case 12:
          case 13:
          case 14:
          case 15:
            ELTop->remoteData.read_data = circularValue(999, 0, --ELTop->remoteData.read_data);
            break;
          case 8:
          case 16:
            ELTop->remoteData.read_data = circularValue(180, 0, --ELTop->remoteData.read_data);
            break;
        }
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
        RESET_BUTTON(pSystem);
      } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
        ELTop->remoteData.remoteCnt = 0;
        switch (ELTop->remoteData.tempStep) {
          case 1:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ASr");
            ELTop->levData.selectedSensorA = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.aMeterCal;
          case 2:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AoC");
            ELTop->levData.aMeterCal = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.aStopMeterSet;
            break;
          case 3:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A-L");
            ELTop->levData.aStopMeterSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.aStartMeterSet;
            break;
          case 4:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A-H");
            ELTop->levData.aStartMeterSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.aUpLimitMeterSet;
            break;
          case 5:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ALL");
            ELTop->levData.aUpLimitMeterSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.aDownLimitMeterSet;
            break;
          case 6:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AHH");
            ELTop->levData.aDownLimitMeterSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.aPumpSwitchTimeSet;
            break;
          case 7:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "APC");
            ELTop->levData.aPumpSwitchTimeSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.aPumpDelaySet;
            break;
          case 8:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Adt");
            ELTop->levData.aPumpDelaySet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.selectedSensorB;
            break;
          case 9:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bSr");
            ELTop->levData.selectedSensorB = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.bMeterCal;
          case 10:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "boC");
            ELTop->levData.bMeterCal = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.bStopMeterSet;
            break;
          case 11:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b-L");
            ELTop->levData.bStopMeterSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.bStartMeterSet;
            break;
          case 12:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b-H");
            ELTop->levData.bStartMeterSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.bUpLimitMeterSet;
            break;
          case 13:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bLL");
            ELTop->levData.bUpLimitMeterSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.bDownLimitMeterSet;
            break;
          case 14:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bHH");
            ELTop->levData.bDownLimitMeterSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.bPumpSwitchTimeSet;
            break;
          case 15:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bPC");
            ELTop->levData.bPumpSwitchTimeSet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.bPumpDelaySet;
            break;
          case 16:
            tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bdt");
            ELTop->levData.bPumpDelaySet = ELTop->remoteData.read_data;
            ELTop->remoteData.read_data = ELTop->levData.selectedSensorA;
            break;
        }
        LevelSettingDataFlashSave();
        ELTop->remoteData.tempStep++;
        if (ELTop->remoteData.tempStep > 16)
          ELTop->remoteData.tempStep = 1;
        RESET_BUTTON(pSystem);
        BUZZER_Control(ON, 100);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, ELTop->remoteData.read_data);
      }
      break;
  }
//  if (ELTop->remoteData.setData != STAND_BY && ELTop->remoteData.setData != INIT_VIEW) {
//    if (ELTop->remoteData.remoteCnt > 50) {
//      ELTop->remoteData.remoteCnt = 0;
//      cliPrintf("STAND_BY Set\n");
//      ELTop->remoteData.setData = STAND_BY;
//      RESET_BUTTON(pSystem);
//    } else
//      ELTop->remoteData.remoteCnt++;
//  }
}

bool buttonClickedInput(_SYSTEM_t *pSystem) {
  bool ret = false;

  tm1639_io3.button = buttonstatusTM1639(&tm1639_io3);
  tm1639_io4.button = buttonstatusTM1639(&tm1639_io4);

  pSystem->buttonVaule.BYTE_FIELD[0] = (tm1639_io3.button & 0x03) | ((tm1639_io4.button & 0x03) << 2);
  static uint8_t button_step = 0;
  static uint8_t btn;
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
        if (HAL_GetTick() - pre_time >= pressed_time) {
          ret = true;  // 버튼 클릭됨
          button_step = 2;
          pre_time = HAL_GetTick();
        }
      } else {
        button_step = 0;
      }
      break;
    case 2:
      if (btn == pSystem->buttonVaule.BYTE_FIELD[0] && btn) {
        if (HAL_GetTick() - pre_time >= repeat_time) {
          button_step = 1;
          pre_time = HAL_GetTick();
        }
      } else {
        button_step = 0;
      }
      break;
  }
  return ret;
}

uint8_t buttonGetInput(_SYSTEM_t *pSystem) {
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
        if (btn == 0x03 || btn == 0x04 || btn == 0x05 || btn == 0x02 || btn == 0x01) {
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
      } else {
        button_step = 0;
      }
      break;
  }
  return ret;
}

void Remote_Task(void *argument) {
  _SYSTEM_t *pSystem = (_SYSTEM_t*) DataBase_Get_pInfo_Data();
  SET_DATA_t *ELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  Buzzer_ClearTimer_ID = osTimerNew(Control_Oneshot_Timer_Callback, osTimerOnce, (void*) CONTROL_TIMER_BUZZER_CLEAR, NULL);
  bool ret;
  uint8_t res;

#ifdef _USE_CLI
  cliAdd("remote", cliRemote);
#endif

  tm1639_io3.clk_gpio = GPIOB;
  tm1639_io3.clk_pin = GPIO_PIN_8;
  tm1639_io3.dio_gpio = GPIOB;
  tm1639_io3.dio_pin = GPIO_PIN_9;
  tm1639_io3.stb_gpio = GPIOC;
  tm1639_io3.stb_pin = GPIO_PIN_14;
//  tm1639_io3.stb_gpio = GPIOC;
//  tm1639_io3.stb_pin = GPIO_PIN_3;
  initTM1639(&tm1639_io3);

  tm1639_io4.clk_gpio = GPIOA;
  tm1639_io4.clk_pin = GPIO_PIN_11;
  tm1639_io4.dio_gpio = GPIOA;
  tm1639_io4.dio_pin = GPIO_PIN_12;
  tm1639_io4.stb_gpio = GPIOC;
  tm1639_io4.stb_pin = GPIO_PIN_15;
//  tm1639_io4.stb_gpio = GPIOC;
//  tm1639_io4.stb_pin = GPIO_PIN_5;
  initTM1639(&tm1639_io4);

  while (1) {
    if (ELTop->remoteData.setData == STAND_BY || ELTop->remoteData.setData == INIT_VIEW || pSystem->buttonVaule.NAME_FIELD.RES2) {
      res = buttonGetInput(pSystem);
      if (res == 1) {
        pSystem->buttonVaule.NAME_FIELD.RES2 = (tm1639_io3.button & 0x03) | ((tm1639_io4.button & 0x03) << 2);
      } else if (res == 2) {
        switch (pSystem->buttonVaule.BYTE_FIELD[0]) {
          case 0x04:
            pSystem->buttonVaule.NAME_FIELD.Button_SET5s = 1;
            break;
          case 0x03:
            pSystem->buttonVaule.NAME_FIELD.Button_UP_DN5s = 1;
            break;
          case 0x06:
            pSystem->buttonVaule.NAME_FIELD.Button_SET_DN10s = 1;
            break;
          case 0x05:
            pSystem->buttonVaule.NAME_FIELD.Button_SET_UP5s = 1;
            break;
          case 0x02:
            pSystem->buttonVaule.NAME_FIELD.Button_DOWN5s = 1;
            break;
          case 0x01:
            pSystem->buttonVaule.NAME_FIELD.Button_UP5s = 1;
            break;
        }
      } else {
        if (ELTop->remoteData.setData == STAND_BY || ELTop->remoteData.setData == INIT_VIEW) {
          pSystem->buttonVaule.BYTE_FIELD[1] = pSystem->buttonVaule.NAME_FIELD.RES2;
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
    led_Control(pSystem, ELTop);
    Remote_Control(pSystem, ELTop);
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
