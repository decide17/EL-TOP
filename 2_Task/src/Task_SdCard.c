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

//_RTC rtc = { .Year = 24, .Month = 7, .Date = 20, .DaysOfWeek = SATURDAY, .Hour = 14, .Min = 02, .Sec = 00 };
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
    } else
      return 1;
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

void WriteFile(_SYSTEM_t *pSystem) {
  uint32_t len;
  char bf[100] = { 0 };
  sprintf(
      bf,
      "%02d%02d%02d%02d%02d%02d:::%.2f,%.2f,%.2f,%.2f\r\n",  // 171208 213724 ER 000 rn
      pSystem->rtcValue.rtc.Year, pSystem->rtcValue.rtc.Month, pSystem->rtcValue.rtc.Date, pSystem->rtcValue.rtc.Hour, pSystem->rtcValue.rtc.Min,
      pSystem->rtcValue.rtc.Sec, pSystem->pt100Value.pt100[0], pSystem->pt100Value.pt100[1], pSystem->pt100Value.pt100[2], pSystem->pt100Value.pt100[3]);
  if (f_write(&MyFile, bf, strlen(bf), (void*) &len) != FR_OK) {
    cliPrintf("f_write error\n");
    return;
  }
}

void file_system_test() {
  _SYSTEM_t *pSystem = (_SYSTEM_t*) DataBase_Get_pInfo_Data();
  SET_DATA_t *ELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  uint8_t ret;
  char file[30];
  sprintf(file, "ELTOP-LOG_%d_%d_%d_%d.TXT", pSystem->rtcValue.rtc.Year, pSystem->rtcValue.rtc.Month, pSystem->rtcValue.rtc.Date, pSystem->rtcValue.rtc.Hour);  // 파일이름
  static uint8_t sdcard_flow = 0;
  if (ELTop->sdData.sdFlag == 0)
    return;
  switch (sdcard_flow) {
    case 0:
      if (pSystem->inputValue.NAME_FIELD.sdCardDetect == true)
        sdcard_flow++;
      break;
    case 1:
      MountSDIO();
      sdcard_flow++;
      break;
    case 2:
      ret = OpenFile(file);
      if (ret == 0) {
        sdcard_flow++;
        break;
      }
      WriteFile(pSystem);
      CloseFile();
      break;
    case 3:
      UnMountSDIO();
      sdcard_flow = 0;
      break;
  }
}
osThreadId_t task_SdCard_Handle;
const osThreadAttr_t taskSdCard_attributes = { .name = "SD Card Thread", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityHigh, };

osMutexId_t SDCardMutexHandle;
const osMutexAttr_t SDCardMutex_attributes = { .name = "SDCardMutex" };

void SD_Card_Task(void *argument) {

#ifdef _USE_CLI
  cliAdd("sdcard", clicard);
#endif
  SDCardMutexHandle = osMutexNew(&SDCardMutex_attributes);
  while (1) {
//    osMutexWait(SDCardMutexHandle, portMAX_DELAY);
    file_system_test();
//    osMutexRelease(SDCardMutexHandle);
    osDelay(5000);

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

