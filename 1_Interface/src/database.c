/*
 * database.c
 *
 *  Created on: Jan 11, 2024
 *      Author: sjpark
 */

#include "database.h"
#include "Task_Modbus.h"
#include "com_flash.h"
SLOT_DATA_t MySlot;

uint32_t DataBase_Get_pMySlot(void) {
  return (uint32_t) &MySlot;
}

#define _MODEL_NUMBER_ "STD v1.0"
#define _SW_RELEASEDATE_ "2024.08.03"  // Max 28 charactor

struct structManufacture_Data {
  char Manufactory[10];    // 제조일자
  char Serial_Number[10];  // 시리얼 넘버
  char *Model_Number;      // 모델 넘버
  char *SW_ReleaseDate;    // FW 배포일
  char *SW_Version;        // FW 버전
  char *HW_Version;        // HW 버전
} Manufacture_Data;

#define SERIAL_NUMBER_SIZE (32)
#define MANUFACTORY_SIZE (16)
#define BATTERY_ID_SIZE (32)
#define BATTERY_ID_USED_SIZE (17)
#define CELL_MAKER_SIZE (4)
#define PACK_SUPPLIER_SIZE (4)
#define MODEL_NUMBER_SIZE (32)
#define SW_RELEASE_DATE_SIZE (12)
#define SW_VERSION_SIZE (10)
#define HW_VERSION_SIZE (10)

struct stIncubated {
  uint8_t Serial_Number[SERIAL_NUMBER_SIZE];     // 시리얼 넘버
  uint8_t Manufactory[MANUFACTORY_SIZE];         // 제조일자
  uint8_t Battery_ID[BATTERY_ID_SIZE];           // 배터리 ID
  uint8_t Cell_Maker[CELL_MAKER_SIZE];           // Cell 제조사
  uint8_t Pack_Supplier[PACK_SUPPLIER_SIZE];     // Pack 공급자
  uint8_t Model_Number[MODEL_NUMBER_SIZE];       // 모델 넘버
  uint8_t SW_ReleaseDate[SW_RELEASE_DATE_SIZE];  // FW 배포일
  uint8_t SW_Version[SW_VERSION_SIZE];           // FW 버전
  uint8_t HW_Version[HW_VERSION_SIZE];           // HW 버전
} Incubated_Battery;
//============================================
//    DATA Pointer Initailize Function
//--------------------------------------------------------------------------------
SET_DATA_t ELTopData;
uint32_t temp_addr = FLASH_TEMP_START_ADDR;

void TempSettingDataFlashSave() {
  FlashErase(FLASH_TEMP_START_ADDR);
  FlashSave(FLASH_TEMP_START_ADDR, ELTopData.tempData.aMoTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 4, ELTopData.tempData.aBrTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 8, ELTopData.tempData.bMoTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 12, ELTopData.tempData.bBrTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 16, ELTopData.tempData.aHtOnTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 20, ELTopData.tempData.aHtOffTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 24, ELTopData.tempData.bHtOnTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 28, ELTopData.tempData.bHtOffTemp);
}

static void initTempDataFactory(void) {
  FlashErase(FLASH_TEMP_START_ADDR);
  ELTopData.tempData.aMoTemp = 100;
  ELTopData.tempData.aBrTemp = 100;
  ELTopData.tempData.bMoTemp = 100;
  ELTopData.tempData.bBrTemp = 100;
  ELTopData.tempData.aHtOnTemp = 20;
  ELTopData.tempData.aHtOffTemp = 30;
  ELTopData.tempData.bHtOnTemp = 20;
  ELTopData.tempData.bHtOffTemp = 30;
  FlashSave(FLASH_TEMP_START_ADDR, ELTopData.tempData.aMoTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 4, ELTopData.tempData.aBrTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 8, ELTopData.tempData.bMoTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 12, ELTopData.tempData.bBrTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 16, ELTopData.tempData.aHtOnTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 20, ELTopData.tempData.aHtOffTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 24, ELTopData.tempData.bHtOnTemp);
  FlashSave(FLASH_TEMP_START_ADDR + 28, ELTopData.tempData.bHtOffTemp);
}

static void DataBaseTempInit(void) {
  uint16_t data;

  data = *(__IO int16_t*) (FLASH_TEMP_START_ADDR);
  if (data == 0xffff) {
    initTempDataFactory();
  } else {
    ELTopData.tempData.aMoTemp = data;
  }
  data = *(__IO int16_t*) (FLASH_TEMP_START_ADDR + 4);
  if (data == 0xffff) {
    initTempDataFactory();
  } else {
    ELTopData.tempData.aBrTemp = data;
  }
  data = *(__IO int16_t*) (FLASH_TEMP_START_ADDR + 8);
  if (data == 0xffff) {
    initTempDataFactory();
  } else {
    ELTopData.tempData.bMoTemp = data;
  }
  data = *(__IO int16_t*) (FLASH_TEMP_START_ADDR + 12);
  if (data == 0xffff) {
    initTempDataFactory();
  } else {
    ELTopData.tempData.bBrTemp = data;
  }
  data = *(__IO int16_t*) (FLASH_TEMP_START_ADDR + 16);
  if (data == 0xffff) {
    initTempDataFactory();
  } else {
    ELTopData.tempData.aHtOnTemp = data;
  }
  data = *(__IO int16_t*) (FLASH_TEMP_START_ADDR + 20);
  if (data == 0xffff) {
    initTempDataFactory();
  } else {
    ELTopData.tempData.aHtOffTemp = data;
  }
  data = *(__IO int16_t*) (FLASH_TEMP_START_ADDR + 24);
  if (data == 0xffff) {
    initTempDataFactory();
  } else {
    ELTopData.tempData.bHtOnTemp = data;
  }
  data = *(__IO int16_t*) (FLASH_TEMP_START_ADDR + 28);
  if (data == 0xffff) {
    initTempDataFactory();
  } else {
    ELTopData.tempData.bHtOffTemp = data;
  }
}

void UserSettingDataFlashSave() {
  FlashErase(FLASH_USER_START_ADDR);
  FlashSave(FLASH_USER_START_ADDR, ELTopData.userData.channel);
  FlashSave(FLASH_USER_START_ADDR + 4, ELTopData.userData.rs485Id);
  FlashSave(FLASH_USER_START_ADDR + 8, ELTopData.userData.rs485Bps);
  FlashSave(FLASH_USER_START_ADDR + 12, ELTopData.userData.AutoReset);
  FlashSave(FLASH_USER_START_ADDR + 16, ELTopData.userData.TripOnDelay);
  FlashSave(FLASH_USER_START_ADDR + 20, ELTopData.userData.aMoLineResAdj);
  FlashSave(FLASH_USER_START_ADDR + 24, ELTopData.userData.aBrLineResAdj);
  FlashSave(FLASH_USER_START_ADDR + 28, ELTopData.userData.bMoLineResAdj);
  FlashSave(FLASH_USER_START_ADDR + 32, ELTopData.userData.bBrLineResAdj);
}

static void initUserDataFactory(void) {
  FlashErase(FLASH_USER_START_ADDR);
  ELTopData.userData.channel = 0;
  ELTopData.userData.rs485Id = 1;
  ELTopData.userData.rs485Bps = 1;
  ELTopData.userData.AutoReset = 0;
  ELTopData.userData.TripOnDelay = 10;
  ELTopData.userData.aMoLineResAdj = 0;
  ELTopData.userData.aBrLineResAdj = 0;
  ELTopData.userData.bMoLineResAdj = 0;
  ELTopData.userData.bBrLineResAdj = 0;
  FlashSave(FLASH_USER_START_ADDR, ELTopData.userData.channel);
  FlashSave(FLASH_USER_START_ADDR + 4, ELTopData.userData.rs485Id);
  FlashSave(FLASH_USER_START_ADDR + 8, ELTopData.userData.rs485Bps);
  FlashSave(FLASH_USER_START_ADDR + 12, ELTopData.userData.AutoReset);
  FlashSave(FLASH_USER_START_ADDR + 16, ELTopData.userData.TripOnDelay);
  FlashSave(FLASH_USER_START_ADDR + 20, ELTopData.userData.aMoLineResAdj);
  FlashSave(FLASH_USER_START_ADDR + 24, ELTopData.userData.aBrLineResAdj);
  FlashSave(FLASH_USER_START_ADDR + 28, ELTopData.userData.bMoLineResAdj);
  FlashSave(FLASH_USER_START_ADDR + 32, ELTopData.userData.bBrLineResAdj);
}

static void DataBaseUserInit(void) {
  uint16_t data;

  data = *(__IO uint8_t*) (FLASH_USER_START_ADDR);
  if (data == 0xff) {
    initUserDataFactory();
  } else {
    ELTopData.userData.channel = data;
  }
  data = *(__IO uint8_t*) (FLASH_USER_START_ADDR + 4);
  if (data == 0xff) {
    initUserDataFactory();
  } else {
    ELTopData.userData.rs485Id = data;
  }
  data = *(__IO uint8_t*) (FLASH_USER_START_ADDR + 8);
  if (data == 0xff) {
    initUserDataFactory();
  } else {
    ELTopData.userData.rs485Bps = data;
  }
  data = *(__IO uint8_t*) (FLASH_USER_START_ADDR + 12);
  if (data == 0xff) {
    initUserDataFactory();
  } else {
    ELTopData.userData.AutoReset = data;
  }
  data = *(__IO uint8_t*) (FLASH_USER_START_ADDR + 16);
  if (data == 0xff) {
    initUserDataFactory();
  } else {
    ELTopData.userData.TripOnDelay = data;
  }
  data = *(__IO int8_t*) (FLASH_USER_START_ADDR + 20);
  if (data == 0xff) {
    initUserDataFactory();
  } else {
    ELTopData.userData.aMoLineResAdj = data;
  }
  data = *(__IO int8_t*) (FLASH_USER_START_ADDR + 24);
  if (data == 0xff) {
    initUserDataFactory();
  } else {
    ELTopData.userData.aBrLineResAdj = data;
  }
  data = *(__IO int8_t*) (FLASH_USER_START_ADDR + 28);
  if (data == 0xff) {
    initUserDataFactory();
  } else {
    ELTopData.userData.bMoLineResAdj = data;
  }
  data = *(__IO int8_t*) (FLASH_USER_START_ADDR + 32);
  if (data == 0xff) {
    initUserDataFactory();
  } else {
    ELTopData.userData.bBrLineResAdj = data;
  }
}

void CalSettingDataFlashSave() {
  FlashErase(FLASH_CAL_START_ADDR);
  FlashSave(FLASH_CAL_START_ADDR, ELTopData.calData.a1_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 4, ELTopData.calData.a1_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 8, ELTopData.calData.a2_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 12, ELTopData.calData.a2_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 16, ELTopData.calData.b1_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 20, ELTopData.calData.b1_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 24, ELTopData.calData.b2_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 28, ELTopData.calData.b2_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 32, ELTopData.calData.c1_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 36, ELTopData.calData.c1_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 40, ELTopData.calData.c2_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 44, ELTopData.calData.c2_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 48, ELTopData.calData.d1_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 52, ELTopData.calData.d1_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 56, ELTopData.calData.d2_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 60, ELTopData.calData.d2_HighOffSet);
}

static void initCalDataFactory(void) {
  FlashErase(FLASH_CAL_START_ADDR);
  ELTopData.calData.a1_LowOffSet = 0;
  ELTopData.calData.a1_HighOffSet = 0;
  ELTopData.calData.a2_LowOffSet = 0;
  ELTopData.calData.a2_HighOffSet = 0;
  ELTopData.calData.b1_LowOffSet = 0;
  ELTopData.calData.b1_HighOffSet = 0;
  ELTopData.calData.b2_LowOffSet = 0;
  ELTopData.calData.b2_HighOffSet = 0;
  ELTopData.calData.c1_LowOffSet = 0;
  ELTopData.calData.c1_HighOffSet = 0;
  ELTopData.calData.c2_LowOffSet = 0;
  ELTopData.calData.c2_HighOffSet = 0;
  ELTopData.calData.d1_LowOffSet = 0;
  ELTopData.calData.d1_HighOffSet = 0;
  ELTopData.calData.d2_LowOffSet = 0;
  ELTopData.calData.d2_HighOffSet = 0;
  FlashSave(FLASH_CAL_START_ADDR, ELTopData.calData.a1_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 4, ELTopData.calData.a1_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 8, ELTopData.calData.a2_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 12, ELTopData.calData.a2_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 16, ELTopData.calData.b1_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 20, ELTopData.calData.b1_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 24, ELTopData.calData.b2_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 28, ELTopData.calData.b2_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 32, ELTopData.calData.c1_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 36, ELTopData.calData.c1_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 40, ELTopData.calData.c2_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 44, ELTopData.calData.c2_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 48, ELTopData.calData.d1_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 52, ELTopData.calData.d1_HighOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 56, ELTopData.calData.d2_LowOffSet);
  FlashSave(FLASH_CAL_START_ADDR + 60, ELTopData.calData.d2_HighOffSet);
}

static void DataBaseCalInit(void) {
  uint16_t data;

  data = *(__IO uint16_t*) (FLASH_CAL_START_ADDR);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.a1_LowOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 4);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.a1_HighOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 8);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.a2_LowOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 12);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.a2_HighOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 16);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.b1_LowOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 20);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.b1_HighOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 24);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.b2_LowOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 28);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.b2_HighOffSet = data;
  }

  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 32);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.c1_LowOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 36);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.c1_HighOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 40);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.c2_LowOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 44);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.c2_HighOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 48);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.d1_LowOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 52);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.d1_HighOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 56);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.d2_LowOffSet = data;
  }
  data = *(__IO int16_t*) (FLASH_CAL_START_ADDR + 60);
  if (data == 0xffff) {
    initCalDataFactory();
  } else {
    ELTopData.calData.d2_HighOffSet = data;
  }
}

void LevelSettingDataFlashSave() {
  FlashErase(FLASH_LEVEL_START_ADDR);
  FlashSave(FLASH_LEVEL_START_ADDR, ELTopData.levData.selectedSensorA);
  FlashSave(FLASH_LEVEL_START_ADDR + 4, ELTopData.levData.aMeterCal);
  FlashSave(FLASH_LEVEL_START_ADDR + 8, ELTopData.levData.aStopMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 12, ELTopData.levData.aStartMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 16, ELTopData.levData.aUpLimitMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 20, ELTopData.levData.aDownLimitMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 24, ELTopData.levData.aPumpSwitchTimeSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 28, ELTopData.levData.aPumpDelaySet);
  FlashSave(FLASH_LEVEL_START_ADDR + 32, ELTopData.levData.selectedSensorB);
  FlashSave(FLASH_LEVEL_START_ADDR + 36, ELTopData.levData.bMeterCal);
  FlashSave(FLASH_LEVEL_START_ADDR + 40, ELTopData.levData.bStopMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 44, ELTopData.levData.bStartMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 48, ELTopData.levData.bUpLimitMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 52, ELTopData.levData.bDownLimitMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 56, ELTopData.levData.bPumpSwitchTimeSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 60, ELTopData.levData.bPumpDelaySet);
}

static void initLevelDataFactory(void) {
  FlashErase(FLASH_LEVEL_START_ADDR);
  ELTopData.levData.selectedSensorA = 500;
  ELTopData.levData.aMeterCal = 0;
  ELTopData.levData.aStopMeterSet = 0;
  ELTopData.levData.aStartMeterSet = 0;
  ELTopData.levData.aUpLimitMeterSet = 0;
  ELTopData.levData.aDownLimitMeterSet = 0;
  ELTopData.levData.aPumpSwitchTimeSet = 0;
  ELTopData.levData.aPumpDelaySet = 0;
  ELTopData.levData.selectedSensorB = 500;
  ELTopData.levData.bMeterCal = 0;
  ELTopData.levData.bStopMeterSet = 0;
  ELTopData.levData.bStartMeterSet = 0;
  ELTopData.levData.bUpLimitMeterSet = 0;
  ELTopData.levData.bDownLimitMeterSet = 0;
  ELTopData.levData.bPumpSwitchTimeSet = 0;
  ELTopData.levData.bPumpDelaySet = 0;
  FlashSave(FLASH_LEVEL_START_ADDR, ELTopData.levData.selectedSensorA);
  FlashSave(FLASH_LEVEL_START_ADDR + 4, ELTopData.levData.aMeterCal);
  FlashSave(FLASH_LEVEL_START_ADDR + 8, ELTopData.levData.aStopMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 12, ELTopData.levData.aStartMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 16, ELTopData.levData.aUpLimitMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 20, ELTopData.levData.aDownLimitMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 24, ELTopData.levData.aPumpSwitchTimeSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 28, ELTopData.levData.aPumpDelaySet);
  FlashSave(FLASH_LEVEL_START_ADDR + 32, ELTopData.levData.selectedSensorB);
  FlashSave(FLASH_LEVEL_START_ADDR + 36, ELTopData.levData.bMeterCal);
  FlashSave(FLASH_LEVEL_START_ADDR + 40, ELTopData.levData.bStopMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 44, ELTopData.levData.bStartMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 48, ELTopData.levData.bUpLimitMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 42, ELTopData.levData.bDownLimitMeterSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 56, ELTopData.levData.bPumpSwitchTimeSet);
  FlashSave(FLASH_LEVEL_START_ADDR + 60, ELTopData.levData.bPumpDelaySet);

}

static void DataBaseLevelInit(void) {
  uint16_t data;

  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.selectedSensorA = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 4);
//  if (data == 0xffff) {
//    initLevelDataFactory();
//  } else {
  ELTopData.levData.aMeterCal = data;
//  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 8);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.aStopMeterSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 12);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.aStartMeterSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 16);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.aUpLimitMeterSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 20);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.aDownLimitMeterSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 24);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.aPumpSwitchTimeSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 28);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.aPumpDelaySet = data;
  }

  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 32);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.selectedSensorB = data;
  }

  data = *(__IO int16_t*) (FLASH_LEVEL_START_ADDR + 36);
//  if (data == 0xffff) {
//    initLevelDataFactory();
//  } else {
  ELTopData.levData.bMeterCal = data;
//  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 40);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.bStopMeterSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 44);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.bStartMeterSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 48);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.bUpLimitMeterSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 52);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.bDownLimitMeterSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 56);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.bPumpSwitchTimeSet = data;
  }
  data = *(__IO uint16_t*) (FLASH_LEVEL_START_ADDR + 60);
  if (data == 0xffff) {
    initLevelDataFactory();
  } else {
    ELTopData.levData.bPumpDelaySet = data;
  }

}

void DataBaseInit(void) {
  DataBaseTempInit();
  DataBaseUserInit();
  DataBaseCalInit();
  DataBaseLevelInit();
}
//============================================
//    ROOT DATA Function
//--------------------------------------------------------------------------------
uint32_t DataBase_Get_Factory_Setting_Data_Address(FATORY_SETTING_DATA_t pos) {
  uint32_t ret_value;
  switch (pos) {
    case MANUFACTORING_DATE:
      ret_value = (uint32_t) &(Manufacture_Data.Manufactory[0]);
      break;
    case SERIAL_NUMBER:
      ret_value = (uint32_t) &(Manufacture_Data.Serial_Number[0]);
      break;
    case MODEL_NUMBER:
      ret_value = (uint32_t) (Manufacture_Data.Model_Number);
      break;
    case SW_RELEASE_DATE:
      ret_value = (uint32_t) (Manufacture_Data.SW_ReleaseDate);
      break;
    case SW_VERSION:
      ret_value = (uint32_t) (Manufacture_Data.SW_Version);
      break;
    case HW_VERSION:
      ret_value = (uint32_t) (Manufacture_Data.HW_Version);
      break;
    default:
      break;
  }
  return ret_value;
}

MODBUS_MAP_t MODBUS_Data;

uint32_t DataBase_Get_pMODBUS_Data(void) {
  return (uint32_t) &MODBUS_Data;
}

_SYSTEM_t SystemData;

uint32_t DataBase_Get_pInfo_Data(void) {
  return (uint32_t) &SystemData;
}

uint32_t DataBase_Get_Setting_Data(void) {
  return (uint32_t) &ELTopData;
}

