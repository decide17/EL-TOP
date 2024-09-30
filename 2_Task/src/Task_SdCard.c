/*
 * Task_SdCard.c
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */
#include<stdlib.h>
#include "database.h"
#include "Task_Cli.h"
#include "fatfs.h"
#include "stm32_ds3231.h"

#ifdef _USE_CLI
static void clicard(uint8_t argc, const char **argv);
#endif

osThreadId_t task_SdCard_Handle;
const osThreadAttr_t taskSdCard_attributes = { .name = "SD Card Thread", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityHigh, };

osMutexId_t SDCardMutexHandle;
const osMutexAttr_t SDCardMutex_attributes = { .name = "SDCardMutex" };

FRESULT res;
FATFS SDFatFs;
FIL MyFile;

void MountSDIO(void) {
  res = f_mount(&SDFatFs, "", 0);
  if (res != FR_OK)
    cliPrintf("MountSDIO error\n");
}

void UnMountSDIO(void) {
  res = f_mount(NULL, "", 0);
  if (res != FR_OK)
    cliPrintf("UnMountSDIO error\n");
}

uint8_t OpenFile(char *fileName) {
  if (f_open(&MyFile, fileName, FA_READ | FA_WRITE) != FR_OK) {  // 파일이 없으면?
    if (f_open(&MyFile, fileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
      cliPrintf("OpenFile error\n");
      return 0;
    } else {
      // todo
//      char *bt = 'Time,AMoSet,AMo,ABrSet,ABr,BMoSet,BMo,BBrSet,BBr,A-HTon,A-HToff,B-HTon,B-HToff,LEAK-A,LEAK-B,ALevel,ASR,A-LO,A-HI,A-LL,A-HH,BLevel,BSR,B-LO,B-HI,B-LL,B-HH,APump,BPump,Relay';
      return 1;
    }
  } else {  // 이어쓰기
    f_lseek(&MyFile, f_size(&MyFile));  // 파일 크기만큼 이동합니다 (파일 끝으로 이동합니다)
    return 1;
  }
}

void CloseFile() {
  res = f_close(&MyFile);
  if (res != FR_OK)
    cliPrintf("CloseFile error\n");
}

//void ReadFile(void) {
//  res = f_read(&MyFile, text, sizeof(text), (void*) &bytesRead);
//  if (res != FR_OK || bytesRead == 0)
//    cliPrintf("ReadFile error\n");
//}

extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
uint16_t cnt;
void WriteFile(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  uint32_t len;
  char bf[200] = { 0 };
  sprintf(bf, "%02d.%02d.%02d.%02d.%02d.%02d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,", sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes,
          sTime.Seconds, pELTop->tempData.aMoTemp, (int16_t) pSystem->pt100Value.pt100Cal[0], pELTop->tempData.aBrTemp,
          (int16_t) pSystem->pt100Value.pt100Cal[1], pELTop->tempData.bMoTemp, (int16_t) pSystem->pt100Value.pt100Cal[2], pELTop->tempData.bBrTemp,
          (int16_t) pSystem->pt100Value.pt100Cal[3], pELTop->tempData.aHtOnTemp, pELTop->tempData.aHtOffTemp, pELTop->tempData.bHtOnTemp,
          pELTop->tempData.bHtOffTemp, pSystem->inputValue.NAME_FIELD.waterSen1, pSystem->inputValue.NAME_FIELD.waterSen1);

//  sprintf(bf, "%02d.%02d.%02d.%02d.%02d.%02d,%d,%d,%d,%d,%d,%d,", sDate.Year, sDate.Month, sDate.Date, sTime.Hours, sTime.Minutes, sTime.Seconds,
//          (int16_t) pSystem->pt100Value.pt100Cal[0], (int16_t) pSystem->pt100Value.pt100Cal[1], (int16_t) pSystem->pt100Value.pt100Cal[2],
//          (int16_t) pSystem->pt100Value.pt100Cal[3], pSystem->inputValue.NAME_FIELD.waterSen1, pSystem->inputValue.NAME_FIELD.waterSen1);

  if (f_write(&MyFile, bf, strlen(bf), (void*) &len) != FR_OK) {
    cliPrintf("f_write error\n");
    return;
  }

  memset(bf, 0, sizeof(bf));

  sprintf(bf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", pELTop->levData.selectedSensorA, pSystem->adcValue.levelSensorCal[0],
          pELTop->levData.aStartMeterSet, pELTop->levData.aStopMeterSet, pELTop->levData.aDownLimitMeterSet, pELTop->levData.aUpLimitMeterSet,
          pELTop->levData.selectedSensorB, pSystem->adcValue.levelSensorCal[1], pELTop->levData.bStartMeterSet, pELTop->levData.bStopMeterSet,
          pELTop->levData.bDownLimitMeterSet, pELTop->levData.bUpLimitMeterSet, pELTop->levData.aPumpSwitchTimeSet, pELTop->levData.bPumpSwitchTimeSet,
          pSystem->outputValue.NAME_FIELD.aMoRelayFlag, pSystem->outputValue.NAME_FIELD.aLLHHRelayFlag, pSystem->outputValue.NAME_FIELD.aHtRelayFlag,
          pSystem->outputValue.NAME_FIELD.aPtRelayFlag, pSystem->outputValue.NAME_FIELD.bMoRelayFlag, pSystem->outputValue.NAME_FIELD.bLLHHRelayFlag,
          pSystem->outputValue.NAME_FIELD.bHtRelayFlag, pSystem->outputValue.NAME_FIELD.bPtRelayFlag);

//  sprintf(bf, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", pSystem->adcValue.levelSensorCal[0], pSystem->adcValue.levelSensorCal[1],
//          pSystem->outputValue.NAME_FIELD.aMoRelayFlag, pSystem->outputValue.NAME_FIELD.aLLHHRelayFlag, pSystem->outputValue.NAME_FIELD.aHtRelayFlag,
//          pSystem->outputValue.NAME_FIELD.aPtRelayFlag, pSystem->outputValue.NAME_FIELD.bMoRelayFlag, pSystem->outputValue.NAME_FIELD.bLLHHRelayFlag,
//          pSystem->outputValue.NAME_FIELD.bHtRelayFlag, pSystem->outputValue.NAME_FIELD.bPtRelayFlag);

  if (f_write(&MyFile, bf, strlen(bf), (void*) &len) != FR_OK) {
    cliPrintf("f_write error\n");
    return;
  }

//  if (f_write(&MyFile, bf, strlen(bf), (void*) &len) != FR_OK) {
//    cliPrintf("f_write error\n");
//    return;
//  }
  cnt++;
}
uint8_t buf;
void SdCardSystemSave() {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
//  static uint8_t buf = 0;
  uint8_t ret;

  static uint8_t sdcardFlow = 0;
  static uint16_t sdcardCnt = 0;
  if (pELTop->sdData.sdFlag == 0) {
    if (pSystem->inputValue.NAME_FIELD.sdCardDetect == true) {
      sdcardFlow = 4;
    } else {
      sdcardFlow = 0;
    }
  } else if (pSystem->inputValue.NAME_FIELD.sdCardDetect == false) {
    sdcardFlow = 0;
  }

  switch (sdcardFlow) {
    case 0:
      if (pSystem->inputValue.NAME_FIELD.sdCardDetect == true)
        sdcardFlow++;
      break;
    case 1:
      MountSDIO();
      sdcardFlow++;
      break;
    case 2:
      char file[30];
      sprintf(file, "ELTOP-LOG_%d_%d.csv", sDate.Year, sDate.Month);  // 파일이름
      ret = OpenFile(file);
      if (ret == 0) {
        sdcardFlow = 4;
        break;
      }
      WriteFile(pSystem, pELTop);
      CloseFile();
      sdcardCnt = 10 * 5 * 60;
      buf = pSystem->outputValue.BYTE_FIELD[2];
      sdcardFlow++;
      break;
    case 3:
      if (pSystem->outputValue.BYTE_FIELD[2] != buf) {
        buf = pSystem->outputValue.BYTE_FIELD[2];
        sdcardFlow = 2;
        sdcardCnt = 10 * 5 * 60;
      } else if (sdcardCnt) {
        sdcardCnt--;
        break;
      }
      sdcardFlow = 2;
      break;
    case 4:
      UnMountSDIO();
      sdcardFlow = 0;
      break;
  }
}

void SD_Card_Task(void *argument) {
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();

#ifdef _USE_CLI
  cliAdd("sdcard", clicard);
#endif

  while (pELTop->remoteData.setData == INIT_VIEW)
    osDelay(100);

  osDelay(2000);

  SDCardMutexHandle = osMutexNew(&SDCardMutex_attributes);
  while (1) {
//    osMutexWait(SDCardMutexHandle, portMAX_DELAY);
    SdCardSystemSave();
//    osMutexRelease(SDCardMutexHandle);
    osDelay(100);

  }
}

void SDCard_Create_Thread(void) {
  task_SdCard_Handle = osThreadNew(SD_Card_Task, NULL, &taskSdCard_attributes);
}

#ifdef _USE_CLI
void clicard(uint8_t argc, const char **argv) {
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

