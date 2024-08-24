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
#include "stm32_ds3231.h"
#include "mcp4728.h"

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

typedef struct {
  int16_t aMoTemp;
  int16_t aBrTemp;
  int16_t bMoTemp;
  int16_t bBrTemp;
  int16_t aHtOnTemp;
  int16_t aHtOffTemp;
  int16_t bHtOnTemp;
  int16_t bHtOffTemp;
} _TEMP_DATA;

typedef struct {
  uint8_t channel;
  uint8_t rs485Id;
  uint8_t rs485Bps;
  uint8_t AutoReset;
  uint8_t TripOnDelay;
  int8_t aMoLineResAdj;
  int8_t aBrLineResAdj;
  int8_t bMoLineResAdj;
  int8_t bBrLineResAdj;
} _USER_DATA;

typedef struct {
  uint8_t sdFlag;
} _SD_DATA;

typedef struct {
  int16_t a1_LowOffSet;
  int16_t a1_HighOffSet;
  int16_t a2_LowOffSet;
  int16_t a2_HighOffSet;
  int16_t b1_LowOffSet;
  int16_t b1_HighOffSet;
  int16_t b2_LowOffSet;
  int16_t b2_HighOffSet;

  uint16_t c1_LowOffSet;
  uint16_t c1_HighOffSet;
  uint16_t c2_LowOffSet;
  uint16_t c2_HighOffSet;
  uint16_t d1_LowOffSet;
  uint16_t d1_HighOffSet;
  uint16_t d2_LowOffSet;
  uint16_t d2_HighOffSet;
} _CALIBRATION_DATA;

typedef struct {
  uint16_t selectedSensorA;
  int16_t aMeterCal;
  uint16_t aStopMeterSet;
  uint16_t aStartMeterSet;
  uint16_t aUpLimitMeterSet;
  uint16_t aDownLimitMeterSet;
  uint16_t aPumpSwitchTimeSet;
  uint16_t aPumpDelaySet;

  uint16_t selectedSensorB;
  int16_t bMeterCal;
  uint16_t bStopMeterSet;
  uint16_t bStartMeterSet;
  uint16_t bUpLimitMeterSet;
  uint16_t bDownLimitMeterSet;
  uint16_t bPumpSwitchTimeSet;
  uint16_t bPumpDelaySet;
} _LEVEL_DATA;

typedef enum enumREMOTE_Data {
  INIT_VIEW,
  STAND_BY,
  USER_TEMP_SET,
  SD_CARD_SET,
  USER_CHANNEL_SELECTION,
  FACTORY_CALIBRATION,
  USER_LEVEL_SET,
  A_B_L_H_SETCHK,
  USER_MENU_SET,
  RS485_SET,
  RTC_SET,
} REMOTE_SETTING_DATA_t;

typedef struct {
  REMOTE_SETTING_DATA_t setData;
  uint32_t stepCnt;
  uint8_t tempStep;
  uint8_t remoteCnt;
  int16_t read_data;
} _REMOTE_DATA;

typedef struct {
  _REMOTE_DATA remoteData;
  _TEMP_DATA tempData;
  _USER_DATA userData;
  _SD_DATA sdData;
  _CALIBRATION_DATA calData;
  _LEVEL_DATA levData;
} SET_DATA_t;

typedef enum enumFactory_Setting_Data {
  MANUFACTORING_DATE,
  SERIAL_NUMBER,
  MODEL_NUMBER,
  SW_RELEASE_DATE,
  SW_VERSION,
  HW_VERSION,
  FACTORY_SETTING_LAST_DATA
} FATORY_SETTING_DATA_t;

typedef union {
  uint8_t BYTE_FIELD[3];
  struct {
    uint8_t Button_UP :1;      // 버튼 UP 키
    uint8_t Button_DOWN :1;    // 버튼 DOWN 키
    uint8_t Button_SET :1;     // 버튼 SET 키
    uint8_t Button_RESET :1;   // 버튼 RESET 키
    uint8_t RES1 :4;    //
    uint8_t Button_UP_CHK :1;      // 버튼 UP OK
    uint8_t Button_DOWN_CHK :1;    // 버튼 DOWN OK
    uint8_t Button_SET_CHK :1;     // 버튼 SET OK
    uint8_t Button_RESET_CHK :1;   // 버튼 RESET OK
    uint8_t RES2 :4;    //
    uint8_t Button_SET5s :1;    // SET 키 5초
    uint8_t Button_UP_DN5s :1;    // 버튼 UP + DOWN 키 5초
    uint8_t Button_SET_DN10s :1;    // 버튼 SET + DOWN 키 10초
    uint8_t Button_SET_UP5s :1;    // 버튼 SET + UP 키 5초
    uint8_t Button_UP5s :1;    // 버튼 UP 키 5초
    uint8_t Button_UP10s :1;    // 버튼 UP 키 10초
    uint8_t Button_DOWN5s :1;  // 버튼 DOWN 키 5초
  } NAME_FIELD;
} _BUTTON_STATUS;

typedef union {
  uint8_t BYTE_FIELD[4];
  struct {
    uint8_t AMO_GREEN_TOGGLE :1;
    uint8_t AMO_RED_TOGGLE :1;
    uint8_t BMO_GREEN_TOGGLE :1;
    uint8_t BMO_RED_TOGGLE :1;
    uint8_t ALK_GREEN_TOGGLE :1;
    uint8_t ALK_RED_TOGGLE :1;
    uint8_t AHT_GREEN_TOGGLE :1;
    uint8_t AHT_RED_TOGGLE :1;
    uint8_t BHT_GREEN_TOGGLE :1;
    uint8_t BHT_RED_TOGGLE :1;
    uint8_t BLK_GREEN_TOGGLE :1;
    uint8_t BLK_RED_TOGGLE :1;
    uint8_t BBR_GREEN_TOGGLE :1;
    uint8_t BBR_RED_TOGGLE :1;
    uint8_t ABR_GREEN_TOGGLE :1;
    uint8_t ABR_RED_TOGGLE :1;
    uint8_t B_LH_GREEN_TOGGLE :1;
    uint8_t B_LH_RED_TOGGLE :1;
    uint8_t B_LL_GREEN_TOGGLE :1;
    uint8_t B_LL_RED_TOGGLE :1;
    uint8_t B_HH_GREEN_TOGGLE :1;
    uint8_t B_HH_RED_TOGGLE :1;
    uint8_t B_PC_GREEN_TOGGLE :1;
    uint8_t B_PC_RED_TOGGLE :1;
    uint8_t A_LH_GREEN_TOGGLE :1;
    uint8_t A_LH_RED_TOGGLE :1;
    uint8_t A_LL_GREEN_TOGGLE :1;
    uint8_t A_LL_RED_TOGGLE :1;
    uint8_t A_HH_GREEN_TOGGLE :1;
    uint8_t A_HH_RED_TOGGLE :1;
    uint8_t A_PC_GREEN_TOGGLE :1;
    uint8_t A_PC_RED_TOGGLE :1;
  } NAME_FIELD;
} _SETTING_STATUS;

typedef union {
  uint8_t BYTE_FIELD[3];
  struct {
    uint8_t sdCardDetect :1;  // sd카드 인식
    uint8_t waterSen1 :1;     // 수위센서 1
    uint8_t waterSen2 :1;     // 수위센서 2
    uint8_t max31865_drdy1 :1;  // max chip drdy
    uint8_t max31865_drdy2 :1;  // max chip drdy
    uint8_t max31865_drdy3 :1;  // max chip drdy
    uint8_t max31865_drdy4 :1;  // max chip drdy
    uint8_t res :1;
    float levelVolt[2];
  } NAME_FIELD;
} _INPUT_STATUS;

typedef struct {
  _RTC rtc;
  uint8_t rtcStep;
  uint8_t errorCnt;
  uint8_t rtcCnt;
  float rtcTemp;
  uint8_t rtcStatus;
} _RTC_DATA;

typedef struct {
  float pt100[4];
  uint8_t errorCnt[4];
} _PT100_DATA;

typedef struct {
  dacChannelConfig mcp4728;
} _DAC_DATA;

typedef struct {
  _BUTTON_STATUS buttonVaule;
  _INPUT_STATUS inputValue;
  _SETTING_STATUS setValue;
  _PT100_DATA pt100Value;
  _RTC_DATA rtcValue;
  _DAC_DATA dacValue;
} _SYSTEM_t;

uint32_t DataBase_Get_pMODBUS_Data(void);
uint32_t DataBase_Get_pInfo_Data(void);
uint32_t DataBase_Get_Setting_Data(void);
void TempSettingDataFlashSave(void);
void LevelSettingDataFlashSave(void);
void UserSettingDataFlashSave(void);
void CalSettingDataFlashSave(void);
void DataBaseInit(void);

#endif /* INC_DATABASE_H_ */
