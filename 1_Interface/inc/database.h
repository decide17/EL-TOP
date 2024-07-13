/*
 * database.h
 *
 *  Created on: Jan 11, 2024
 *      Author: sjpark
 */

#ifndef INC_DATABASE_H_
#define INC_DATABASE_H_

#include "cmsis_os.h"
#include "com_uart.h"

#define _SW_VERSION_MAJOR 1
#define _SW_VERSION_MINOR 0
#define _SW_VERSION_SUBMINOR 2

#define DATE_YEAR 24
#define DATE_MONTH 7
#define DATE_DAY 3

typedef struct stSlotStatus {
  uint32_t Battery_Inserted :1;    // 배터리 삽입 검출
  uint32_t Battery_Locked :1;      // 배터리 Lock 수행
  uint32_t Door_Closed :1;         // Door 닫힘 검출
  uint32_t Door_Unlocked :1;       // Door Lock 수행
  uint32_t Battery_Wakeup :1;      // 배터리 wakeup 수행
  uint32_t Slot_Heater :1;         // 슬롯 히터 수행
  uint32_t Discharger_Output :1;   // 충전기 입력 제어 수행
  uint32_t Charger_Line_Output :1;  // 충전기 출력 제어 수행
  uint32_t Door_Selected :1;       // Door 열림 표시
  uint32_t Now_Charging :1;        // 충전 중
  uint32_t Full_Charged :1;        // 만충 상태
  uint32_t Balancing_Excute :1;    // 밸런싱 보정중
  uint32_t Battery_Check_OK :1;    // 배터리 OK. 충전 가능 상태
} SLOT_STATUS_t;

// typedef struct stSlotError {
//   uint32_t BMS_Cell_Over_Voltage :2;             // 셀 과충전 보호 -> BMS 값 토대로 계산
//   uint32_t BMS_Cell_Under_Voltage :2;            // 셀 과방전 보호 -> BMS 값 토대로 계산
//   uint32_t BMS_Cell_Imbalance :2;              // 셀 불균형 보호 -> BMS 값 토대로 계산
//   uint32_t BMS_Charge_Over_Temperature :2;         // 충전 과온 보호 -> BMS 값 토대로 계산
//   uint32_t BMS_Temperature_Imbalance :2;           // 온도 불균형 보호 -> BMS 값 토대로 계산
//   uint32_t BMS_Pack_Over_Voltage :2;             // 팩 과전압 보호 -> BMS 값 토대로 계산
//   uint32_t BMS_Pack_Under_Voltage :2;            // 팩 저전압 보호 -> BMS 값 토대로 계산
//   uint32_t BMS_Charge_Over_Current :2;           // 충전 과전류보호 -> BMS 값 토대로 계산
//   uint32_t SLOT_Chassis_S_Over_Temperature :2;       // Slot 과온 보호 -> SLOT 측정 값 토대로 계산
//   uint32_t SLOT_Chassis_C_Over_Temperature :2;       // 충전기 Case 과온 보호 -> SLOT 측정 값 토대로 계산
//   uint32_t SLOT_Pack_Over_Voltage :2;            // 과전압 보호 -> SLOT 측정 값 토대로 계산
//   uint32_t SLOT_Pack_Under_Voltage :2;           // 저전압 보호 -> SLOT 측정 값 토대로 계산
//   uint32_t SLOT_Charge_Over_Current :2;            // 충전 과전류보호 -> SLOT 측정 값 토대로 계산
//   uint32_t SLOT_Charge_Current_NoFlow :1;          // 충전 전류가 흐르지 않음
//   uint32_t Reserved :5;      // 32bit 정렬
//   uint32_t HeartBeat_No_Update :1;             // BMS로부터 통신이 전달 되지 않는다
//   uint32_t HeartBeat_No_Recieved :1;             // BMS로부터 통신 값이 갱신되지 않는다
//   uint32_t Fault_Status_In_BMS :1;             // BMS의 Fault Status가 Set되어 있다
//   uint32_t BAT_ID_Format_Difference :1;            // 배터리 ID가 올바르지 않다
//   uint32_t BAT_ID_No_Recieved :1;              // 배터리 ID가 수신되지 않는다
//   uint32_t BMS_Voltage_Invalid :1;             // 배터리 전압이 유효하지 않다.
// } SLOT_ERROR_t;

typedef struct stSystemRequest {
  uint8_t BeQuiet :1;
  uint8_t CurrentCalib :1;
} SYSTEM_REQUEST_t;

typedef struct stSlotData {
  struct {
    uint8_t *pSerial_Number;
    uint8_t *pManufactory;
    uint8_t *pBattery_ID;
    uint8_t *pCell_Maker;
    uint8_t *pPack_Supplier;
    uint8_t *pModel_Number;
    uint8_t *pSW_ReleaseDate;
    uint8_t *pSW_Version;
    uint8_t *pHW_Version;
  } Incubated;  // 탑재한 배터리의 제조 정보
  uint8_t BMS_HeartBeat;
  int32_t ChargeCurrent;  // unit : A, resolution : 0.001
  int32_t BatteryVoltage;  // unit : V, resolution : 0.001
  //  int16_t SlotTemperature;         // unit : ℃, resolution : 0.1
  //  int16_t ChargerTemperature;        // unit : ℃, resolution : 0.1
  uint32_t WorkSequence;
  SLOT_STATUS_t SlotStatus;
  //  SLOT_ERROR_t ErrorStatus;
  uint8_t TowerNumber;
  uint8_t SlotNumber;
  SYSTEM_REQUEST_t SystemRequest;
} SLOT_DATA_t;

typedef enum enumFactory_Setting_Data {
  MANUFACTORING_DATE,
  SERIAL_NUMBER,
  MODEL_NUMBER,
  SW_RELEASE_DATE,
  SW_VERSION,
  HW_VERSION,
  FACTORY_SETTING_LAST_DATA
} FATORY_SETTING_DATA_t;

//uint32_t DataBase_Get_pCAN_Data(void);
//uint32_t DataBase_Get_pMySlot(void);
//uint32_t DataBase_Get_pCAN_Charge(void);
//uint32_t DataBase_Get_pCAN_Control(void);
//uint32_t DataBase_Get_pAnnounce(void);
//uint32_t DataBase_Get_pCAN_SlotInfo(void);
//uint32_t DataBase_Get_Factory_Setting_Data_Address(FATORY_SETTING_DATA_t pos);
//uint32_t DataBase_Get_pCharger_CAN_Data(void);
uint32_t DataBase_Get_pMODBUS_Data(void);

void DataBase_Init(void);

#endif /* INC_DATABASE_H_ */
