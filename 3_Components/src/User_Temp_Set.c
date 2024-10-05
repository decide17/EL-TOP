/*
 * User_Temp_Set.c
 *
 *  Created on: Sep 7, 2024
 *      Author: USER
 */

#include "database.h"

#include "Task_Cli.h"
#include "Task_Remote.h"

#include "com_gpio.h"
#include "tm1639.h"

extern void BUZZER_Control(ONOFF_t on, uint16_t time);
extern void RESET_BUTTON(SYSTEM_t *SystemData);
extern int circularValue(int max, int min, int value);
extern Tm1639_t tm1639_io3;
extern Tm1639_t tm1639_io4;

void UserTempStartFunc(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  pELTop->remoteData.remoteCnt = 200;
  pELTop->remoteData.setData = USER_TEMP_SET;
  Full_Reset(&tm1639_io3);
  Full_Reset(&tm1639_io4);
  BUZZER_Control(ON, 100);

  if (pELTop->userData.channel == 1) {
    pELTop->remoteData.tempStep = 3;
    tm1639_io4.data.NAME_FIELD.LED2_GREEN = 1;
    pELTop->remoteData.read_data = pELTop->tempData.bMoTemp;
  } else {
    pELTop->remoteData.tempStep = 1;
    tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
    pELTop->remoteData.read_data = pELTop->tempData.aMoTemp;
  }

  tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, pELTop->remoteData.read_data);
  cliPrintf("read_data : %d\n", pELTop->remoteData.read_data);
  RESET_BUTTON(pSystem);
}

void UserTempSetFunc(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
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
    if (pELTop->remoteData.tempStep <= 4) {
      pELTop->remoteData.read_data = circularValue(250, 0, ++pELTop->remoteData.read_data);
      if (pELTop->remoteData.tempStep % 2 == 1) {
        Full_Reset(&tm1639_io3);
        tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, pELTop->remoteData.read_data);
      } else {
        Full_Reset(&tm1639_io4);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
      }
    } else {
      pELTop->remoteData.read_data = circularValue(50, 0, ++pELTop->remoteData.read_data);
      tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
    }
    cliPrintf("Button_UP_CHK read_data : %d tempStep : %d\n", pELTop->remoteData.read_data, pELTop->remoteData.tempStep);
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
    pELTop->remoteData.remoteCnt = 200;
    if (pELTop->remoteData.tempStep <= 4) {
      pELTop->remoteData.read_data = circularValue(250, 0, --pELTop->remoteData.read_data);
      if (pELTop->remoteData.tempStep % 2 == 1) {
        Full_Reset(&tm1639_io3);
        tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, pELTop->remoteData.read_data);
      } else {
        Full_Reset(&tm1639_io4);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
      }
    } else {
      pELTop->remoteData.read_data = circularValue(50, 0, --pELTop->remoteData.read_data);
      tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
    }
    cliPrintf("Button_DOWN_CHK read_data : %d, tempStep : %d\n", pELTop->remoteData.read_data, pELTop->remoteData.tempStep);
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
    pELTop->remoteData.remoteCnt = 200;
    if (pELTop->userData.channel == 0) {
      //1256
      switch (pELTop->remoteData.tempStep) {
        case 1:
          pELTop->remoteData.tempStep = 2;
          break;
        case 2:
        case 3:
        case 4:
          pELTop->remoteData.tempStep = 5;
          break;
        case 5:
          pELTop->remoteData.tempStep = 6;
          break;
        case 6:
        case 7:
        case 8:
          pELTop->remoteData.tempStep = 9;
          break;
      }
    } else if (pELTop->userData.channel == 1) {
      // 3478
      switch (pELTop->remoteData.tempStep) {
        case 1:
        case 2:
        case 8:
          pELTop->remoteData.tempStep = 3;
          break;
        case 3:
          pELTop->remoteData.tempStep = 4;
          break;
        case 4:
        case 5:
        case 6:
          pELTop->remoteData.tempStep = 7;
          break;
        case 7:
          pELTop->remoteData.tempStep = 8;
          break;
      }
    } else {
      pELTop->remoteData.tempStep++;
    }
    switch (pELTop->remoteData.tempStep) {
      case 2:
        tm1639_io4.data.NAME_FIELD.LED1_GREEN = 0;
        tm1639_io3.data.NAME_FIELD.LED8_GREEN = 1;
        pELTop->tempData.aMoTemp = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->tempData.aBrTemp;
        break;
      case 3:
        tm1639_io4.data.NAME_FIELD.LED2_GREEN = 1;
        if (pELTop->userData.channel == 1) {
          pELTop->tempData.bHtOffTemp = pELTop->remoteData.read_data;
        } else {
          tm1639_io3.data.NAME_FIELD.LED8_GREEN = 0;
          pELTop->tempData.aBrTemp = pELTop->remoteData.read_data;
        }
        pELTop->remoteData.read_data = pELTop->tempData.bMoTemp;
        break;
      case 4:
        tm1639_io4.data.NAME_FIELD.LED2_GREEN = 0;
        tm1639_io3.data.NAME_FIELD.LED7_GREEN = 1;
        pELTop->tempData.bMoTemp = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->tempData.bBrTemp;
        break;
      case 5:
        tm1639_io3.data.NAME_FIELD.LED7_GREEN = 0;
        tm1639_io4.data.NAME_FIELD.LED4_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AHo");
        if (pELTop->userData.channel == 0) {
          tm1639_io3.data.NAME_FIELD.LED8_GREEN = 0;
          pELTop->tempData.aBrTemp = pELTop->remoteData.read_data;
        } else {
          pELTop->tempData.bBrTemp = pELTop->remoteData.read_data;
        }
        pELTop->remoteData.read_data = pELTop->tempData.aHtOnTemp;
        break;
      case 6:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AHF");
        pELTop->tempData.aHtOnTemp = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->tempData.aHtOffTemp;
        break;
      case 7:
        tm1639_io4.data.NAME_FIELD.LED4_GREEN = 0;
        tm1639_io3.data.NAME_FIELD.LED5_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bHo");
        if (pELTop->userData.channel == 1) {
          tm1639_io3.data.NAME_FIELD.LED7_GREEN = 0;
          pELTop->tempData.bBrTemp = pELTop->remoteData.read_data;
        } else {
          pELTop->tempData.aHtOffTemp = pELTop->remoteData.read_data;
        }
        pELTop->remoteData.read_data = pELTop->tempData.bHtOnTemp;
        break;
      case 8:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bHF");
        pELTop->tempData.bHtOnTemp = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->tempData.bHtOffTemp;
        break;
      case 9:
        tm1639_io3.data.NAME_FIELD.LED5_GREEN = 0;
        tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
        if (pELTop->userData.channel == 0) {
          tm1639_io4.data.NAME_FIELD.LED4_GREEN = 0;
          pELTop->tempData.aHtOffTemp = pELTop->remoteData.read_data;
        } else {
          pELTop->tempData.bHtOffTemp = pELTop->remoteData.read_data;
        }
        pELTop->remoteData.read_data = pELTop->tempData.aMoTemp;
        break;
    }
    TempSettingDataFlashSave();

    if (pELTop->remoteData.tempStep > 8)
      pELTop->remoteData.tempStep = 1;
    RESET_BUTTON(pSystem);
    BUZZER_Control(ON, 100);
    if (pELTop->remoteData.tempStep <= 4) {
      if (pELTop->remoteData.tempStep % 2 == 1) {
        Full_Reset(&tm1639_io3);
        tm1639Display_num(&tm1639_io4, IO4_AB_MO_FND, pELTop->remoteData.read_data);
      } else {
        Full_Reset(&tm1639_io4);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
      }
    } else {
      tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
    }
  } else if (pELTop->remoteData.resetCnt == 1) {
    switch (pELTop->remoteData.tempStep) {
      case 1:
        pELTop->tempData.aMoTemp = pELTop->remoteData.read_data;
        break;
      case 2:
        pELTop->tempData.aBrTemp = pELTop->remoteData.read_data;
        break;
      case 3:
        pELTop->tempData.bMoTemp = pELTop->remoteData.read_data;
        break;
      case 4:
        if (pELTop->userData.channel == 0) {
          pELTop->tempData.aBrTemp = pELTop->remoteData.read_data;
        } else {
          pELTop->tempData.bBrTemp = pELTop->remoteData.read_data;
        }
        break;
      case 5:
        pELTop->tempData.aHtOnTemp = pELTop->remoteData.read_data;
        break;
      case 6:
        if (pELTop->userData.channel == 1) {
          pELTop->tempData.bBrTemp = pELTop->remoteData.read_data;
        } else {
          pELTop->tempData.aHtOffTemp = pELTop->remoteData.read_data;
        }
        break;
      case 7:
        pELTop->tempData.bHtOnTemp = pELTop->remoteData.read_data;
        break;
      case 8:
        if (pELTop->userData.channel == 0) {
          pELTop->tempData.aHtOffTemp = pELTop->remoteData.read_data;
        } else {
          pELTop->tempData.bHtOffTemp = pELTop->remoteData.read_data;
        }
        break;
    }
    TempSettingDataFlashSave();
    pELTop->remoteData.resetCnt = 0;
    cliPrintf("STAND_BY Set\n");
    BUZZER_Control(ON, 100);
    pELTop->remoteData.setData = STAND_BY;
    RESET_BUTTON(pSystem);
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
  }
}
