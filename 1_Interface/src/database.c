/*
 * database.c
 *
 *  Created on: Jan 11, 2024
 *      Author: sjpark
 */

#include "database.h"
#include "Task_Modbus.h"
SLOT_DATA_t MySlot;

uint32_t DataBase_Get_pMySlot(void) {
  return (uint32_t) &MySlot;
}

#define _MODEL_NUMBER_ "STD v1.0 Slot"
#define _SW_RELEASEDATE_ "2023.01.04"  // Max 28 charactor

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
void DataBase_Init(void) {
  MySlot.Incubated.pSerial_Number = &Incubated_Battery.Serial_Number[0];
  MySlot.Incubated.pManufactory = &Incubated_Battery.Manufactory[0];
  MySlot.Incubated.pBattery_ID = &Incubated_Battery.Battery_ID[0];
  MySlot.Incubated.pCell_Maker = &Incubated_Battery.Cell_Maker[0];
  MySlot.Incubated.pPack_Supplier = &Incubated_Battery.Pack_Supplier[0];
  MySlot.Incubated.pModel_Number = &Incubated_Battery.Model_Number[0];
  MySlot.Incubated.pSW_ReleaseDate = &Incubated_Battery.SW_ReleaseDate[0];
  MySlot.Incubated.pSW_Version = &Incubated_Battery.SW_Version[0];
  MySlot.Incubated.pHW_Version = &Incubated_Battery.HW_Version[0];

  Manufacture_Data.Model_Number = _MODEL_NUMBER_;
  Manufacture_Data.SW_ReleaseDate = _SW_RELEASEDATE_;
//  Manufacture_Data.SW_Version = _SW_VERSION_;
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

