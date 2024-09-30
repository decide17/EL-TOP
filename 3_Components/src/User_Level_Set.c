/*
 * User_Level_Set.c
 *
 *  Created on: Sep 7, 2024
 *      Author: USER
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

void UserLevelStartFunc(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  pELTop->remoteData.remoteCnt = 200;
  pELTop->remoteData.setData = USER_LEVEL_SET;
  Full_Reset(&tm1639_io3);
  Full_Reset(&tm1639_io4);
  BUZZER_Control(ON, 100);
  pELTop->remoteData.tempStep = 1;
  pELTop->remoteData.read_data = pELTop->levData.selectedSensorA;
  cliPrintf("read_temp : %d, tempStep : %d \n", pELTop->remoteData.read_data, pELTop->remoteData.tempStep);
  RESET_BUTTON(pSystem);
  tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Asr");
  tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
}

void UserLevelSetFunc(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
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
      case 2:
      case 10:
        pELTop->remoteData.read_data = circularValue(100, -99, ++pELTop->remoteData.read_data);
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 9:
      case 1:
      case 3:
      case 4:
      case 5:
      case 6:
      case 11:
      case 12:
      case 13:
      case 14:
        pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 7:
      case 15:
        pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        break;
      case 8:
      case 16:
        pELTop->remoteData.read_data = circularValue(180, 0, ++pELTop->remoteData.read_data);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        break;
    }
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
    pELTop->remoteData.remoteCnt = 200;
    switch (pELTop->remoteData.tempStep) {
      case 2:
      case 10:
        pELTop->remoteData.read_data = circularValue(100, -99, --pELTop->remoteData.read_data);
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 9:
      case 1:
      case 3:
      case 4:
      case 5:
      case 6:
      case 11:
      case 12:
      case 13:
      case 14:
        pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 7:
      case 15:
        pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        break;
      case 8:
      case 16:
        pELTop->remoteData.read_data = circularValue(180, 0, --pELTop->remoteData.read_data);
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        break;
    }
    RESET_BUTTON(pSystem);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
    pELTop->remoteData.remoteCnt = 200;
    switch (pELTop->remoteData.tempStep) {
      case 1:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AoC");
        pELTop->levData.selectedSensorA = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.aMeterCal;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 2:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A-L");
        pELTop->levData.aMeterCal = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.aStartMeterSet;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 3:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A-H");
        pELTop->levData.aStartMeterSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.aStopMeterSet;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 4:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ALL");
        pELTop->levData.aStopMeterSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.aDownLimitMeterSet;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 5:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "AHH");
        pELTop->levData.aDownLimitMeterSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.aUpLimitMeterSet;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 6:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "APC");
        pELTop->levData.aUpLimitMeterSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.aPumpSwitchTimeSet;
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        break;
      case 7:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "Adt");
        pELTop->levData.aPumpSwitchTimeSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.aPumpDelaySet;
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        break;
      case 8:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bSr");
        pELTop->levData.aPumpDelaySet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.selectedSensorB;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 9:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "boC");
        pELTop->levData.selectedSensorB = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.bMeterCal;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 10:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b-L");
        pELTop->levData.bMeterCal = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.bStartMeterSet;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 11:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b-H");
        pELTop->levData.bStartMeterSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.bStopMeterSet;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 12:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bLL");
        pELTop->levData.bStopMeterSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.bDownLimitMeterSet;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 13:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bHH");
        pELTop->levData.bDownLimitMeterSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.bUpLimitMeterSet;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
      case 14:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bPC");
        pELTop->levData.bUpLimitMeterSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.bPumpSwitchTimeSet;
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        break;
      case 15:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "bdt");
        pELTop->levData.bPumpSwitchTimeSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.bPumpDelaySet;
        tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
        break;
      case 16:
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "ASr");
        pELTop->levData.bPumpDelaySet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->levData.selectedSensorA;
        tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 100));
        break;
    }
    LevelSettingDataFlashSave();
    pELTop->remoteData.tempStep++;
    if (pELTop->remoteData.tempStep > 16)
      pELTop->remoteData.tempStep = 1;
    RESET_BUTTON(pSystem);
    BUZZER_Control(ON, 100);
  } else if (pELTop->remoteData.resetCnt == 1) {
    switch (pELTop->remoteData.tempStep) {
      case 1:
        pELTop->levData.selectedSensorA = pELTop->remoteData.read_data;
        break;
      case 2:
        pELTop->levData.aMeterCal = pELTop->remoteData.read_data;
        break;
      case 3:
        pELTop->levData.aStartMeterSet = pELTop->remoteData.read_data;
        break;
      case 4:
        pELTop->levData.aStopMeterSet = pELTop->remoteData.read_data;
        break;
      case 5:
        pELTop->levData.aDownLimitMeterSet = pELTop->remoteData.read_data;
        break;
      case 6:
        pELTop->levData.aUpLimitMeterSet = pELTop->remoteData.read_data;
        break;
      case 7:
        pELTop->levData.aPumpSwitchTimeSet = pELTop->remoteData.read_data;
        break;
      case 8:
        pELTop->levData.aPumpDelaySet = pELTop->remoteData.read_data;
        break;
      case 9:
        pELTop->levData.selectedSensorB = pELTop->remoteData.read_data;
        break;
      case 10:
        pELTop->levData.bMeterCal = pELTop->remoteData.read_data;
        break;
      case 11:
        pELTop->levData.bStartMeterSet = pELTop->remoteData.read_data;
        break;
      case 12:
        pELTop->levData.bStopMeterSet = pELTop->remoteData.read_data;
        break;
      case 13:
        pELTop->levData.bDownLimitMeterSet = pELTop->remoteData.read_data;
        break;
      case 14:
        pELTop->levData.bUpLimitMeterSet = pELTop->remoteData.read_data;
        break;
      case 15:
        pELTop->levData.bPumpSwitchTimeSet = pELTop->remoteData.read_data;
        break;
      case 16:
        pELTop->levData.bPumpDelaySet = pELTop->remoteData.read_data;
        break;
    }
    LevelSettingDataFlashSave();
    pELTop->remoteData.resetCnt = 0;
    cliPrintf("STAND_BY Set\n");
    BUZZER_Control(ON, 100);
    pELTop->remoteData.setData = STAND_BY;
    RESET_BUTTON(pSystem);
  }
}
