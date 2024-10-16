/*
 * Task_input.c
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#include "i2c.h"

#include "database.h"
#include "Task_Input.h"
#include "Task_Cli.h"
#include "com_flash.h"
#include "tm1639.h"

#ifdef _USE_CLI
static void cliControl(uint8_t argc, const char **argv);
#endif

osThreadId_t task_Control_Handle;
const osThreadAttr_t taskControl_attributes = { .name = "Control Thread", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };

extern Tm1639_t tm1639_io3;
extern Tm1639_t tm1639_io4;

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
    sDate.WeekDay = pSystem->rtcValue.rtc.DaysOfWeek - 1;
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
        sDate.WeekDay = pSystem->rtcValue.rtc.DaysOfWeek - 1;
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
          sDate.WeekDay = pSystem->rtcValue.rtc.DaysOfWeek - 1;
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
      if (pSystem->rtcValue.rtcCnt >= 10 * 60 * 60 * 12)
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
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sDate.Year = 24;
  sDate.Month = 1;
  sDate.Date = 1;
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
}

void APtControl(SET_DATA_t *pELTop) {
  static uint8_t AptOnFlag = 0;
  static uint32_t ptTimeSave = 0;
  if (pELTop->levData.aPumpSwitchTimeSet) {
    switch (AptOnFlag) {
      case 0:
        AptOnFlag = 1;
        APt_Control(OFF);
        ptTimeSave = pELTop->levData.aPumpSwitchTimeSet * 60 * 60 * 10;
        break;
      case 1:
        if (ptTimeSave) {
          ptTimeSave--;
        } else {
          AptOnFlag = 2;
          ptTimeSave = pELTop->levData.aPumpSwitchTimeSet * 60 * 60 * 10;
          APt_Control(ON);
        }
        break;
      case 2:
        if (ptTimeSave) {
          ptTimeSave--;
        } else {
          AptOnFlag = 1;
          ptTimeSave = pELTop->levData.aPumpSwitchTimeSet * 60 * 60 * 10;
          APt_Control(OFF);
        }
        break;
    }
  } else {
    AptOnFlag = 0;
    APt_Control(OFF);
  }
}

void BPtControl(SET_DATA_t *pELTop) {
  static uint8_t BptOnFlag = 0;
  static uint32_t ptTimeSave = 0;
  if (pELTop->levData.bPumpSwitchTimeSet) {
    switch (BptOnFlag) {
      case 0:
        BptOnFlag = 1;
        BPt_Control(OFF);
        ptTimeSave = pELTop->levData.bPumpSwitchTimeSet * 60 * 60 * 10;
        break;
      case 1:
        if (ptTimeSave) {
          ptTimeSave--;
        } else {
          BptOnFlag = 2;
          ptTimeSave = pELTop->levData.bPumpSwitchTimeSet * 60 * 60 * 10;
          BPt_Control(ON);
        }
        break;
      case 2:
        if (ptTimeSave) {
          ptTimeSave--;
        } else {
          BptOnFlag = 1;
          ptTimeSave = pELTop->levData.bPumpSwitchTimeSet * 60 * 60 * 10;
          BPt_Control(OFF);
        }
        break;
    }
  } else {
    BptOnFlag = 0;
    BPt_Control(OFF);
  }
}

void AMoControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  static uint16_t AMoDelay = 0;

  if (pSystem->outputValue.NAME_FIELD.aMoFlag == 1) {
    if (pELTop->userData.AutoReset == 0)
      return;
  }
  if (pSystem->pt100Value.pt100Cal[0] >= 280) {
    pSystem->outputValue.NAME_FIELD.aMoFlag = 0;
    AMoDelay = pELTop->userData.TripOnDelay * 10;
  } else if (pSystem->pt100Value.pt100Cal[0] <= -99) {
    if (AMoDelay) {
      AMoDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.aMoFlag = 1;
  } else if (pSystem->pt100Value.pt100Cal[0] > pELTop->tempData.aMoTemp) {
    if (AMoDelay) {
      AMoDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.aMoFlag = 1;
  } else {
    pSystem->outputValue.NAME_FIELD.aMoFlag = 0;
    AMoDelay = pELTop->userData.TripOnDelay * 10;
  }
}

void ABrControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  static uint16_t ABrDelay = 0;
  if (pSystem->outputValue.NAME_FIELD.aBrFlag == 1) {
    if (pELTop->userData.AutoReset == 0)
      return;
  }
  if (pSystem->pt100Value.pt100Cal[1] >= 280) {
    pSystem->outputValue.NAME_FIELD.aBrFlag = 0;
    ABrDelay = pELTop->userData.TripOnDelay * 10;
  } else if (pSystem->pt100Value.pt100Cal[1] <= -99) {
    if (ABrDelay) {
      ABrDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.aBrFlag = 1;
  } else if (pSystem->pt100Value.pt100Cal[1] > pELTop->tempData.aBrTemp) {
    if (ABrDelay) {
      ABrDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.aBrFlag = 1;
  } else {
    pSystem->outputValue.NAME_FIELD.aBrFlag = 0;
    ABrDelay = pELTop->userData.TripOnDelay * 10;
  }
}

void BMoControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  static uint16_t BMoDelay = 0;
  if (pSystem->outputValue.NAME_FIELD.bMoFlag == 1) {
    if (pELTop->userData.AutoReset == 0)
      return;
  }
  if (pSystem->pt100Value.pt100Cal[2] >= 280) {
    pSystem->outputValue.NAME_FIELD.bMoFlag = 0;
    BMoDelay = pELTop->userData.TripOnDelay * 10;
  } else if (pSystem->pt100Value.pt100Cal[2] <= -99) {
    if (BMoDelay) {
      BMoDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.bMoFlag = 1;
  } else if (pSystem->pt100Value.pt100Cal[2] > pELTop->tempData.bMoTemp) {
    if (BMoDelay) {
      BMoDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.bMoFlag = 1;
  } else {
    pSystem->outputValue.NAME_FIELD.bMoFlag = 0;
    BMoDelay = pELTop->userData.TripOnDelay * 10;
  }
}

void BBrControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  static uint16_t BBrDelay = 0;
  if (pSystem->outputValue.NAME_FIELD.bBrFlag == 1) {
    if (pELTop->userData.AutoReset == 0)
      return;
  }
  if (pSystem->pt100Value.pt100Cal[3] >= 280) {
    pSystem->outputValue.NAME_FIELD.bBrFlag = 0;
    BBrDelay = pELTop->userData.TripOnDelay * 10;
  } else if (pSystem->pt100Value.pt100Cal[3] <= -99) {
    if (BBrDelay) {
      BBrDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.bBrFlag = 1;
  } else if (pSystem->pt100Value.pt100Cal[3] > pELTop->tempData.bBrTemp) {
    if (BBrDelay) {
      BBrDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.bBrFlag = 1;
  } else {
    pSystem->outputValue.NAME_FIELD.bBrFlag = 0;
    BBrDelay = pELTop->userData.TripOnDelay * 10;
  }
}

void AHtControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  if (pSystem->pt100Value.pt100Cal[0] <= -99 || pSystem->pt100Value.pt100Cal[0] >= 300) {
    AHt_Control(OFF);
  } else if (pELTop->tempData.aHtOnTemp >= pELTop->tempData.aHtOffTemp) {
    AHt_Control(OFF);
  } else if (pELTop->tempData.aHtOnTemp > pSystem->pt100Value.pt100Cal[0]) {
    AHt_Control(ON);
  } else if (pELTop->tempData.aHtOffTemp < pSystem->pt100Value.pt100Cal[0]) {
    AHt_Control(OFF);
  }
}

void BHtControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  if (pSystem->pt100Value.pt100Cal[2] <= -99 || pSystem->pt100Value.pt100Cal[2] >= 300) {
    BHt_Control(OFF);
  } else if (pELTop->tempData.bHtOnTemp >= pELTop->tempData.bHtOffTemp) {
    BHt_Control(OFF);
  } else if (pELTop->tempData.bHtOnTemp > pSystem->pt100Value.pt100Cal[2]) {
    BHt_Control(ON);
  } else if (pELTop->tempData.bHtOffTemp < pSystem->pt100Value.pt100Cal[2]) {
    BHt_Control(OFF);
  }
}

uint16_t RelayALevelUpLimitCnt;
uint16_t RelayALevelDownLimitCnt;

void ALevelLimitControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  if (pSystem->adcValue.levelSensorCal[0] > 0 && pELTop->levData.selectedSensorA >= pSystem->adcValue.levelSensorCal[0]) {
    if (pELTop->levData.aDownLimitMeterSet > pSystem->adcValue.levelSensorCal[0]) {
      if (RelayALevelDownLimitCnt) {
        RelayALevelDownLimitCnt--;
        return;
      }
      pSystem->outputValue.NAME_FIELD.aLLFlag = 1;
      pSystem->outputValue.NAME_FIELD.aHHFlag = 0;
      ALLHH_Control(ON);
    } else if ((pELTop->levData.aUpLimitMeterSet != 0) && pELTop->levData.aUpLimitMeterSet < pSystem->adcValue.levelSensorCal[0]) {
      if (RelayALevelUpLimitCnt) {
        RelayALevelUpLimitCnt--;
        return;
      }
      pSystem->outputValue.NAME_FIELD.aLLFlag = 0;
      pSystem->outputValue.NAME_FIELD.aHHFlag = 1;
      ALLHH_Control(ON);
    } else if (pSystem->outputValue.NAME_FIELD.aLLFlag && (pELTop->levData.aStartMeterSet < pSystem->adcValue.levelSensorCal[0])) {
      RelayALevelUpLimitCnt = pELTop->levData.aPumpDelaySet * 10;
      RelayALevelDownLimitCnt = pELTop->levData.aPumpDelaySet * 10;
      pSystem->outputValue.NAME_FIELD.aLLFlag = 0;
      pSystem->outputValue.NAME_FIELD.aHHFlag = 0;
      ALLHH_Control(OFF);
    } else if (pSystem->outputValue.NAME_FIELD.aHHFlag && (pELTop->levData.aStopMeterSet > pSystem->adcValue.levelSensorCal[0])) {
      RelayALevelUpLimitCnt = pELTop->levData.aPumpDelaySet * 10;
      RelayALevelDownLimitCnt = pELTop->levData.aPumpDelaySet * 10;
      pSystem->outputValue.NAME_FIELD.aLLFlag = 0;
      pSystem->outputValue.NAME_FIELD.aHHFlag = 0;
      ALLHH_Control(OFF);
    } else {
      RelayALevelUpLimitCnt = pELTop->levData.aPumpDelaySet * 10;
      RelayALevelDownLimitCnt = pELTop->levData.aPumpDelaySet * 10;
    }
  } else {
    RelayALevelUpLimitCnt = pELTop->levData.aPumpDelaySet * 10;
    RelayALevelDownLimitCnt = pELTop->levData.aPumpDelaySet * 10;
    pSystem->outputValue.NAME_FIELD.aLLFlag = 0;
    pSystem->outputValue.NAME_FIELD.aHHFlag = 0;
    ALLHH_Control(OFF);
  }
}

uint16_t RelayBLevelUpLimitCnt;
uint16_t RelayBLevelDownLimitCnt;

void BLevelLimitControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  if (pSystem->adcValue.levelSensorCal[1] > 0 && pELTop->levData.selectedSensorB >= pSystem->adcValue.levelSensorCal[1]) {
    if (pELTop->levData.bDownLimitMeterSet > pSystem->adcValue.levelSensorCal[1]) {
      if (RelayBLevelDownLimitCnt) {
        RelayBLevelDownLimitCnt--;
        return;
      }
      pSystem->outputValue.NAME_FIELD.bLLFlag = 1;
      pSystem->outputValue.NAME_FIELD.bHHFlag = 0;
      BLLHH_Control(ON);
    } else if ((pELTop->levData.bUpLimitMeterSet != 0) && pELTop->levData.bUpLimitMeterSet < pSystem->adcValue.levelSensorCal[1]) {
      if (RelayBLevelUpLimitCnt) {
        RelayBLevelUpLimitCnt--;
        return;
      }
      pSystem->outputValue.NAME_FIELD.bLLFlag = 0;
      pSystem->outputValue.NAME_FIELD.bHHFlag = 1;
      BLLHH_Control(ON);
    } else if (pSystem->outputValue.NAME_FIELD.bLLFlag && (pELTop->levData.bStartMeterSet < pSystem->adcValue.levelSensorCal[1])) {
      RelayBLevelUpLimitCnt = pELTop->levData.bPumpDelaySet * 10;
      RelayBLevelDownLimitCnt = pELTop->levData.bPumpDelaySet * 10;
      pSystem->outputValue.NAME_FIELD.bLLFlag = 0;
      pSystem->outputValue.NAME_FIELD.bHHFlag = 0;
      BLLHH_Control(OFF);
    } else if (pSystem->outputValue.NAME_FIELD.bHHFlag && (pELTop->levData.bStopMeterSet > pSystem->adcValue.levelSensorCal[1])) {
      RelayBLevelUpLimitCnt = pELTop->levData.bPumpDelaySet * 10;
      RelayBLevelDownLimitCnt = pELTop->levData.bPumpDelaySet * 10;
      pSystem->outputValue.NAME_FIELD.bLLFlag = 0;
      pSystem->outputValue.NAME_FIELD.bHHFlag = 0;
      BLLHH_Control(OFF);
    } else {
      RelayBLevelUpLimitCnt = pELTop->levData.bPumpDelaySet * 10;
      RelayBLevelDownLimitCnt = pELTop->levData.bPumpDelaySet * 10;
    }
  } else {
    RelayBLevelUpLimitCnt = pELTop->levData.bPumpDelaySet * 10;
    RelayBLevelDownLimitCnt = pELTop->levData.bPumpDelaySet * 10;
    pSystem->outputValue.NAME_FIELD.bLLFlag = 0;
    pSystem->outputValue.NAME_FIELD.bHHFlag = 0;
    BLLHH_Control(OFF);
  }
}

void ALevelControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  if (pSystem->adcValue.levelSensorCal[0] <= 0 || pELTop->levData.selectedSensorA < pSystem->adcValue.levelSensorCal[0]) {
    pSystem->outputValue.NAME_FIELD.aMoLevelFlag = 0;
  } else {
    if (pELTop->levData.aStartMeterSet >= pELTop->levData.aStopMeterSet) {
      pSystem->outputValue.NAME_FIELD.aMoLevelFlag = 0;
    } else if (pELTop->levData.aStartMeterSet > pSystem->adcValue.levelSensorCal[0]) {
      pSystem->outputValue.NAME_FIELD.aMoLevelFlag = 1;
    } else if (pELTop->levData.aStopMeterSet < pSystem->adcValue.levelSensorCal[0]) {
      pSystem->outputValue.NAME_FIELD.aMoLevelFlag = 0;
    } else {
//      RelayALevelOnCnt = pELTop->levData.aPumpDelaySet * 10;
    }
  }
}

void BLevelControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  if (pSystem->adcValue.levelSensorCal[1] <= 0 || pELTop->levData.selectedSensorB < pSystem->adcValue.levelSensorCal[1]) {
    pSystem->outputValue.NAME_FIELD.bMoLevelFlag = 0;
  } else {
    if (pELTop->levData.bStartMeterSet >= pELTop->levData.bStopMeterSet) {
      pSystem->outputValue.NAME_FIELD.bMoLevelFlag = 0;
    } else if (pELTop->levData.bStartMeterSet > pSystem->adcValue.levelSensorCal[1]) {
      pSystem->outputValue.NAME_FIELD.bMoLevelFlag = 1;
      if (pELTop->levData.bDownLimitMeterSet > pSystem->adcValue.levelSensorCal[0]) {
      }
    } else if (pELTop->levData.bStopMeterSet < pSystem->adcValue.levelSensorCal[1]) {
      pSystem->outputValue.NAME_FIELD.bMoLevelFlag = 0;
    } else {
//      RelayBLevelOnCnt = pELTop->levData.bPumpDelaySet * 10;
    }
  }
}

void Water1InPutControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  static uint16_t water1AMoDelay = 0;
  if (pSystem->outputValue.NAME_FIELD.aMoWater1Flag == 1) {
    if (pELTop->userData.AutoReset == 0)
      return;
  }
  if (pSystem->inputValue.NAME_FIELD.waterSen1) {
    if (water1AMoDelay) {
      water1AMoDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.aMoWater1Flag = 1;
  } else {
    pSystem->outputValue.NAME_FIELD.aMoWater1Flag = 0;
    water1AMoDelay = pELTop->userData.TripOnDelay * 10;
  }
}

void Water2InPutControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  static uint16_t water2BMoDelay = 0;
  if (pSystem->outputValue.NAME_FIELD.bMoWater1Flag == 1) {
    if (pELTop->userData.AutoReset == 0)
      return;
  }
  if (pSystem->inputValue.NAME_FIELD.waterSen2) {
    if (water2BMoDelay) {
      water2BMoDelay--;
      return;
    }
    pSystem->outputValue.NAME_FIELD.bMoWater1Flag = 1;
  } else {
    pSystem->outputValue.NAME_FIELD.bMoWater1Flag = 0;
    water2BMoDelay = pELTop->userData.TripOnDelay * 10;
  }
}

void MoRelayControl(SYSTEM_t *pSystem) {
  if (pSystem->outputValue.NAME_FIELD.aMoWater1Flag || pSystem->outputValue.NAME_FIELD.aMoLevelFlag || pSystem->outputValue.NAME_FIELD.aMoFlag
      || pSystem->outputValue.NAME_FIELD.aBrFlag)
    AMo_Control(ON);
  else
    AMo_Control(OFF);

  if (pSystem->outputValue.NAME_FIELD.bMoWater1Flag || pSystem->outputValue.NAME_FIELD.bMoLevelFlag || pSystem->outputValue.NAME_FIELD.bMoFlag
      || pSystem->outputValue.NAME_FIELD.bBrFlag)
    BMo_Control(ON);
  else
    BMo_Control(OFF);
}

void RelayLEDControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  if (pELTop->remoteData.setData == STAND_BY) {

    if (pSystem->outputValue.NAME_FIELD.aHtRelayFlag == 1) {
      pSystem->setValue.NAME_FIELD.AHT_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.AHT_RED_TOGGLE = 0;
      tm1639_io4.data.NAME_FIELD.LED4_RED = 0;
    }
    if (pSystem->outputValue.NAME_FIELD.bHtRelayFlag) {
      pSystem->setValue.NAME_FIELD.BHT_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.BHT_RED_TOGGLE = 0;
      tm1639_io3.data.NAME_FIELD.LED5_RED = 0;
    }

    if (pSystem->outputValue.NAME_FIELD.aMoWater1Flag == 1) {
      pSystem->setValue.NAME_FIELD.ALK_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.ALK_RED_TOGGLE = 0;
      tm1639_io4.data.NAME_FIELD.LED3_RED = 0;
    }
    if (pSystem->outputValue.NAME_FIELD.bMoWater1Flag == 1) {
      pSystem->setValue.NAME_FIELD.BLK_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.BLK_RED_TOGGLE = 0;
      tm1639_io3.data.NAME_FIELD.LED6_RED = 0;
    }

    if (pSystem->outputValue.NAME_FIELD.aMoLevelFlag == 1) {
      tm1639_io3.data.NAME_FIELD.LED1_GREEN = 1;
    } else {
      tm1639_io3.data.NAME_FIELD.LED1_GREEN = 0;
    }
    if (pSystem->outputValue.NAME_FIELD.bMoLevelFlag == 1) {
      tm1639_io4.data.NAME_FIELD.LED5_GREEN = 1;
    } else {
      tm1639_io4.data.NAME_FIELD.LED5_GREEN = 0;
    }

    if (pSystem->outputValue.NAME_FIELD.aMoFlag == 1) {
      pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE = 0;
      tm1639_io4.data.NAME_FIELD.LED1_RED = 0;
    }

    if (pSystem->outputValue.NAME_FIELD.aBrFlag == 1) {
      pSystem->setValue.NAME_FIELD.ABR_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.ABR_RED_TOGGLE = 0;
      tm1639_io3.data.NAME_FIELD.LED8_RED = 0;
    }

    if (pSystem->outputValue.NAME_FIELD.bMoFlag == 1) {
      pSystem->setValue.NAME_FIELD.BMO_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.BMO_RED_TOGGLE = 0;
      tm1639_io4.data.NAME_FIELD.LED2_RED = 0;
    }

    if (pSystem->outputValue.NAME_FIELD.bBrFlag == 1) {
      pSystem->setValue.NAME_FIELD.BBR_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.BBR_RED_TOGGLE = 0;
      tm1639_io3.data.NAME_FIELD.LED7_RED = 0;
    }

    if (pSystem->outputValue.NAME_FIELD.aLLFlag == 1) {
      pSystem->setValue.NAME_FIELD.A_LL_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.A_LL_RED_TOGGLE = 0;
      tm1639_io3.data.NAME_FIELD.LED2_RED = 0;
    }
    if (pSystem->outputValue.NAME_FIELD.aHHFlag == 1) {
      pSystem->setValue.NAME_FIELD.A_HH_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.A_HH_RED_TOGGLE = 0;
      tm1639_io3.data.NAME_FIELD.LED3_RED = 0;
    }

    if (pSystem->outputValue.NAME_FIELD.bLLFlag == 1) {
      pSystem->setValue.NAME_FIELD.B_LL_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.B_LL_RED_TOGGLE = 0;
      tm1639_io4.data.NAME_FIELD.LED6_RED = 0;
    }
    if (pSystem->outputValue.NAME_FIELD.bHHFlag == 1) {
      pSystem->setValue.NAME_FIELD.B_HH_RED_TOGGLE = 1;
    } else {
      pSystem->setValue.NAME_FIELD.B_HH_RED_TOGGLE = 0;
      tm1639_io4.data.NAME_FIELD.LED7_RED = 0;
    }

    if (pSystem->outputValue.NAME_FIELD.aPtRelayFlag == 1) {
      tm1639_io3.data.NAME_FIELD.LED4_GREEN = 1;
    } else {
      tm1639_io3.data.NAME_FIELD.LED4_GREEN = 0;
    }
    if (pSystem->outputValue.NAME_FIELD.bPtRelayFlag == 1) {
      tm1639_io4.data.NAME_FIELD.LED8_GREEN = 1;
    } else {
      tm1639_io4.data.NAME_FIELD.LED8_GREEN = 0;
    }
  }
}

void BuzzerControl(SET_DATA_t *pELTop, SYSTEM_t *pSystem) {
  static uint8_t BuzStep = 0;
  static uint8_t BuzCnt = 0;
  if (pELTop->remoteData.setData == STAND_BY) {
    switch (BuzStep) {
      case 0:
        if (pSystem->setValue.NAME_FIELD.ABR_RED_TOGGLE || pSystem->setValue.NAME_FIELD.BBR_RED_TOGGLE || pSystem->setValue.NAME_FIELD.BLK_RED_TOGGLE
            || pSystem->setValue.NAME_FIELD.A_HH_RED_TOGGLE || pSystem->setValue.NAME_FIELD.A_LL_RED_TOGGLE || pSystem->setValue.NAME_FIELD.B_HH_RED_TOGGLE
            || pSystem->setValue.NAME_FIELD.B_LL_RED_TOGGLE || pSystem->setValue.NAME_FIELD.ALK_RED_TOGGLE || pSystem->setValue.NAME_FIELD.BMO_RED_TOGGLE
            || pSystem->setValue.NAME_FIELD.AMO_RED_TOGGLE) {
          BuzStep = 1;
          BuzCnt = 0;
        }
        break;
      case 1:
        BUZ_Control(ON);
        if (BuzCnt++ > 5) {
          BuzCnt = 0;
          BuzStep = 2;
        }
        break;
      case 2:
        BUZ_Control(OFF);
        if (BuzCnt++ > 5) {
          BuzCnt = 0;
          BuzStep = 0;
        }
        break;
    }

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

void Control_Task(void *argument) {
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
#ifdef _USE_CLI
  cliAdd("control", cliControl);
#endif

  while (pELTop->remoteData.setData == INIT_VIEW)
    osDelay(100);
  initTime();

  while (1) {
    RelayLEDControl(pELTop, pSystem);

    Water1InPutControl(pELTop, pSystem);
    Water2InPutControl(pELTop, pSystem);
    if (pELTop->userData.channel == 0) {
      AHtControl(pELTop, pSystem);
    } else if (pELTop->userData.channel == 1) {
      BHtControl(pELTop, pSystem);
    } else {
      AHtControl(pELTop, pSystem);
      BHtControl(pELTop, pSystem);
    }
    APtControl(pELTop);
    BPtControl(pELTop);

    ALevelControl(pELTop, pSystem);
    BLevelControl(pELTop, pSystem);
    ALevelLimitControl(pELTop, pSystem);
    BLevelLimitControl(pELTop, pSystem);

    AMoControl(pELTop, pSystem);
    ABrControl(pELTop, pSystem);
    BMoControl(pELTop, pSystem);
    BBrControl(pELTop, pSystem);
    MoRelayControl(pSystem);

    MCP4728Flow(pSystem);

    if (pELTop->remoteData.remoteCnt) {
      pELTop->remoteData.remoteCnt--;
      if (pELTop->remoteData.remoteCnt == 0) {
        pELTop->remoteData.resetCnt = 1;
      }
    }

    rtc_flow(pSystem);
    BuzzerControl(pELTop, pSystem);
    osDelay(100);
  }
}

void CONTROL_Create_Thread(void) {
  task_Control_Handle = osThreadNew(Control_Task, NULL, &taskControl_attributes);
}

#ifdef _USE_CLI
void cliControl(uint8_t argc, const char **argv) {
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

