/*
 * User_Menu_Set.c
 *
 *  Created on: Oct 1, 2024
 *      Author: Admin
 */

#include "database.h"

#include "Task_Cli.h"

#include "com_gpio.h"
#include "tm1639.h"

extern void BUZZER_Control(ONOFF_t on, uint16_t time);
extern void RESET_BUTTON(SYSTEM_t *SystemData);
extern int circularValue(int max, int min, int value);
extern Tm1639_t tm1639_io3;
extern Tm1639_t tm1639_io4;

#define IO4_AB_MO_FND 0
#define IO4_B_LEVEL_FND 1
#define IO3_AB_BR_FND 0
#define IO3_A_LEVEL_FND 1

void UserMenuStartFunc(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  pELTop->remoteData.remoteCnt = 200;
  pELTop->remoteData.setData = USER_MENU_SET;
  Full_Reset(&tm1639_io3);
  Full_Reset(&tm1639_io4);
  BUZZER_Control(ON, 100);
  pELTop->remoteData.tempStep = 1;
  pELTop->remoteData.read_data = pELTop->userData.AutoReset;
  tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "rSt");
  if (pELTop->remoteData.read_data)
    tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
  else
    tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
}

void UserMenuSetFunc(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
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
        pELTop->remoteData.read_data = circularValue(1, 0, ++pELTop->remoteData.read_data);
        break;
      case 2:
        pELTop->remoteData.read_data = circularValue(60, 0, ++pELTop->remoteData.read_data);
        break;
      case 3:
      case 4:
      case 5:
      case 6:
        pELTop->remoteData.read_data = circularValue(10, -50, ++pELTop->remoteData.read_data);
        break;
    }
    if (pELTop->remoteData.tempStep != 1)
      tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
    else {
      if (pELTop->remoteData.read_data)
        tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
      else
        tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
    }
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
    pELTop->remoteData.remoteCnt = 200;
    switch (pELTop->remoteData.tempStep) {
      case 2:
        pELTop->remoteData.read_data = circularValue(60, 0, --pELTop->remoteData.read_data);
        break;
      case 3:
      case 4:
      case 5:
      case 6:
        pELTop->remoteData.read_data = circularValue(10, -50, --pELTop->remoteData.read_data);
        break;
    }
    if (pELTop->remoteData.tempStep != 1)
      tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
    pELTop->remoteData.remoteCnt = 200;
    switch (pELTop->remoteData.tempStep) {
      case 1:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "trd");
        pELTop->userData.AutoReset = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->userData.TripOnDelay;
        break;
      case 2:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AJ1");
        pELTop->userData.TripOnDelay = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->userData.aMoLineResAdj;
        break;
      case 3:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AJ2");
        pELTop->userData.aMoLineResAdj = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->userData.aBrLineResAdj;
        break;
      case 4:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bJ1");
        pELTop->userData.aBrLineResAdj = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->userData.bMoLineResAdj;
        break;
      case 5:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bJ2");
        pELTop->userData.bMoLineResAdj = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->userData.bBrLineResAdj;
        break;
      case 6:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "rST");
        pELTop->userData.bBrLineResAdj = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->userData.AutoReset;
        if (pELTop->remoteData.read_data)
          tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, " On");
        else
          tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "OFF");
        break;
    }
    UserSettingDataFlashSave();
    pELTop->remoteData.tempStep++;
    if (pELTop->remoteData.tempStep > 6)
      pELTop->remoteData.tempStep = 1;
    RESET_BUTTON(pSystem);
    BUZZER_Control(ON, 100);
    if (pELTop->remoteData.tempStep != 1)
      tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
  } else if (pELTop->remoteData.resetCnt == 1) {
    switch (pELTop->remoteData.tempStep) {
      case 1:
        pELTop->userData.AutoReset = pELTop->remoteData.read_data;
        break;
      case 2:
        pELTop->userData.TripOnDelay = pELTop->remoteData.read_data;
        break;
      case 3:
        pELTop->userData.aMoLineResAdj = pELTop->remoteData.read_data;
        break;
      case 4:
        pELTop->userData.aBrLineResAdj = pELTop->remoteData.read_data;
        break;
      case 5:
        pELTop->userData.bMoLineResAdj = pELTop->remoteData.read_data;
        break;
      case 6:
        pELTop->userData.bBrLineResAdj = pELTop->remoteData.read_data;
        break;
    }
    UserSettingDataFlashSave();
    pELTop->remoteData.resetCnt = 0;
    cliPrintf("STAND_BY Set\n");
    BUZZER_Control(ON, 100);
    pELTop->remoteData.setData = STAND_BY;
    RESET_BUTTON(pSystem);
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
  }
}
