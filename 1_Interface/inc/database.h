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

  int16_t c1_LowOffSet;
  int16_t c1_HighOffSet;
  int16_t c2_LowOffSet;
  int16_t c2_HighOffSet;
  int16_t d1_LowOffSet;
  int16_t d1_HighOffSet;
  int16_t d2_LowOffSet;
  int16_t d2_HighOffSet;
} _CALIBRATION_DATA;

typedef struct {
  uint16_t selectedSensorA;
  int16_t aMeterCal;
  uint16_t aStartMeterSet;
  uint16_t aStopMeterSet;
  uint16_t aDownLimitMeterSet;
  uint16_t aUpLimitMeterSet;
  uint16_t aPumpSwitchTimeSet;
  uint16_t aPumpDelaySet;

  uint16_t selectedSensorB;
  int16_t bMeterCal;
  uint16_t bStartMeterSet;
  uint16_t bStopMeterSet;
  uint16_t bDownLimitMeterSet;
  uint16_t bUpLimitMeterSet;
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
//  HARDWARD_TEST,
} REMOTE_SETTING_DATA_t;

typedef struct {
  REMOTE_SETTING_DATA_t setData;
  uint32_t stepCnt;
  uint8_t tempStep;
  uint16_t remoteCnt;
  int16_t read_data;
  uint8_t resetCnt;
} _REMOTE_DATA;

typedef struct {
  _REMOTE_DATA remoteData;
  _TEMP_DATA tempData;
  _USER_DATA userData;
  _SD_DATA sdData;
  _CALIBRATION_DATA calData;
  _LEVEL_DATA levData;
} SET_DATA_t;

typedef union {
  uint8_t BYTE_FIELD[5];
  struct {
    uint8_t Button_DOWN :1;    // 버튼 DOWN 키
    uint8_t Button_UP :1;      // 버튼 UP 키
    uint8_t Button_RESET :1;   // 버튼 RESET 키
    uint8_t Button_SET :1;     // 버튼 SET 키
    uint8_t RES1 :4;    //
    uint8_t Button_DOWN_CHK :1;   // 버튼 DOWN OK
    uint8_t Button_UP_CHK :1;     // 버튼 UP OK
    uint8_t Button_RESET_CHK :1;  // 버튼 RESET OK
    uint8_t Button_SET_CHK :1;    // 버튼 SET OK
    uint8_t RES2 :4;    //
    uint8_t Button_SET5s :1;      // SET 키 5초
    uint8_t Button_UP_DN5s :1;    // 버튼 UP + DOWN 키 5초
    uint8_t Button_SET_DN10s :1;  // 버튼 SET + DOWN 키 10초
    uint8_t Button_SET_UP5s :1;   // 버튼 SET + UP 키 5초
    uint8_t Button_UP5s :1;       // 버튼 UP 키 5초
    uint8_t Button_UP10s :1;      // 버튼 UP 키 10초
    uint8_t Button_DOWN5s :1;     // 버튼 DOWN 키 5초
    uint8_t Button_RESET5s :1;    // 버튼 RESET 키 5초
    uint8_t Button_SET_DN :1;     // 버튼 SET+DOWN
    uint8_t RES3 :7;    //
    uint8_t Button_DOWN5chk :1;   // 버튼 DOWN 키 5회 누름
    uint8_t Button_UP5chk :1;     // 버튼 UP 키 5회 누름
    uint8_t Button_DOWNcnt :3;    // 버튼 DOWN 키 카운트
    uint8_t Button_UPcnt :3;      // 버튼 UP키 카운트
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
  uint8_t BYTE_FIELD[1];
  struct {
    uint8_t sdCardDetect :1;  // sd카드 인식
    uint8_t waterSen1 :1;     // 수위센서 1
    uint8_t waterSen2 :1;     // 수위센서 2
    uint8_t max31865_drdy1 :1;  // max chip drdy
    uint8_t max31865_drdy2 :1;  // max chip drdy
    uint8_t max31865_drdy3 :1;  // max chip drdy
    uint8_t max31865_drdy4 :1;  // max chip drdy
    uint8_t res :1;
  } NAME_FIELD;
} _INPUT_STATUS;

typedef union {
  uint8_t BYTE_FIELD[4];
  struct {
    uint8_t aMoLevelFlag :1;    // A-MO Relay
    uint8_t aMoWater1Flag :1;
    uint8_t aMoFlag :1;
    uint8_t aBrFlag :1;
    uint8_t aLLFlag :1;    // A-LL Relay
    uint8_t aHHFlag :1;    // A-HH Relay
    uint8_t res1 :2;

    uint8_t bMoLevelFlag :1;    // B-MO Relay
    uint8_t bMoWater1Flag :1;
    uint8_t bMoFlag :1;
    uint8_t bBrFlag :1;
    uint8_t bLLFlag :1;    // B-L/H Relay
    uint8_t bHHFlag :1;    // B-LL Relay
    uint8_t res2 :2;

    uint8_t aMoRelayFlag :1;
    uint8_t aHtRelayFlag :1;    // A-HT Relay
    uint8_t aPtRelayFlag :1;    // A-PT Relay
    uint8_t aLLHHRelayFlag :1;
    uint8_t bMoRelayFlag :1;
    uint8_t bHtRelayFlag :1;    // B-HT Relay
    uint8_t bPtRelayFlag :1;    // B-PT Relay
    uint8_t bLLHHRelayFlag :1;

    uint8_t buzzerFlag :1;
  } NAME_FIELD;
} _OUTPUT_STATUS;

typedef struct {
  _RTC rtc;
  uint8_t rtcStep;
  uint8_t errorCnt;
  uint32_t rtcCnt;
  float rtcTemp;
  uint8_t rtcStatus;
} _RTC_DATA;

typedef struct {
  float pt100[4];
  float pt100Cal[4];
  uint8_t errorCnt[4];
} _PT100_DATA;

typedef struct {
  dacChannelConfig mcp4728;
} _DAC_DATA;

typedef struct {
  uint16_t levelVolt[2][50];
  double levelVoltAvg[2];
  double levelSensorCur[2];
  double levelSensor[2];
  uint16_t levelSensorCal[2];
} _ADC_DATA;

typedef struct {
  uint8_t BYTE_FIELD[1];
  struct {
    uint8_t adc1None :1;  // adc 1 인식없음
    uint8_t adc2None :1;  // adc 2 인식없음
    uint8_t adc1Over :1;  // adc 2 과전류
    uint8_t Res :5;  // adc 2 과전류
  } NAME_FIELD;
} _ERROR_DATA;

typedef struct {
  _BUTTON_STATUS buttonVaule;
  _INPUT_STATUS inputValue;
  _OUTPUT_STATUS outputValue;
  _SETTING_STATUS setValue;
  _PT100_DATA pt100Value;
  _RTC_DATA rtcValue;
  _DAC_DATA dacValue;
  _ADC_DATA adcValue;
  _ERROR_DATA errorValue;
} SYSTEM_t;

uint32_t DataBase_Get_pMODBUS_Data(void);
uint32_t DataBase_Get_pInfo_Data(void);
uint32_t DataBase_Get_Setting_Data(void);
void TempSettingDataFlashSave(void);
void LevelSettingDataFlashSave(void);
void UserSettingDataFlashSave(void);
void CalSettingDataFlashSave(void);
void DataBaseInit(void);

#endif /* INC_DATABASE_H_ */
