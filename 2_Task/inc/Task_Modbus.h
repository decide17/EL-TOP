/*
 * Task_Modbus.h
 *
 *  Created on: May 30, 2024
 *      Author: sjpar
 */

#ifndef INC_TASK_MODBUS_H_
#define INC_TASK_MODBUS_H_

#include "com_uart.h"
#include "cmsis_os.h"

#define UART_DMA_SIZE (uint16_t)512

#define _USE_MODBUS

typedef struct {
  uint16_t FWVersionMajor;
  uint16_t FWVersionMinor;
  uint16_t FWVersionPatch;
  uint16_t InputStatus;
  int16_t Pt100Value[4];
  int16_t LevelSensor[2];
  uint16_t RelayFlag;
  uint16_t Reserved_1[3];
  uint16_t Year;
  uint16_t Month;
  uint16_t Week;
  uint16_t Day;
  uint16_t Hour;
  uint16_t Minute;
  uint16_t Second;
  int16_t tempData[8];
  int16_t userData[10];
  int16_t calData[12];
  int16_t levelData[16];
  uint16_t pt100DAC[4];
  uint16_t Reserved_2[4];
} MODBUS_CONTROL_t;

//typedef struct {
//
////  uint16_t Heater1_control;
////  uint16_t Fan1_control;
////  uint16_t Heater2_control;
////  uint16_t Fan2_control;
////  uint16_t Reset_control;
////  uint16_t Reserved_2;
////  uint16_t Set_Year;
////  uint16_t Set_Month;
////  uint16_t Set_Day;
////  uint16_t Set_Hour;
////  uint16_t Set_Minute;
////  uint16_t Set_Second;
//} MODBUS_CONTROL_CMD_t;

#pragma pack(push, 1)
typedef struct {
  uint8_t InterfaceType;
  uint8_t RxStartFlag;
  uint16_t RxStartPos;
  uint16_t RxRealGetPos;
  uint16_t RxCurrentPos;
  uint16_t RxNextPos;
  uint16_t RxCount;
  uint16_t RxIdle;
  uint8_t RxBuf[UART_DMA_SIZE ];
  uint8_t Direction;
} UART_DMA_RECEIVE_HANDLE_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
  uint8_t TxBuf[UART_DMA_SIZE ];
  uint16_t TxLength;
} UART_DMA_SEND_HANDLE_t;
#pragma pack(pop)

typedef struct {
  uint16_t AddressSize;
  uint16_t StartAddress;
  uint16_t EndAddress;
} MODBUS_MAP_INFO_t;

enum {
  CONTROL_MAP,
  CONTROL_CMD_MAP,
  END_MAP
};

typedef struct {
  MODBUS_CONTROL_t Control;
//  MODBUS_CONTROL_CMD_t Control_CMD;
  MODBUS_MAP_INFO_t AddrInfo[END_MAP];
} MODBUS_MAP_t;

#pragma pack(push, 1)
typedef struct {
  uint8_t u8UnitId;
  uint8_t u8FuncCode;
  uint16_t u16RegAddr;
  uint16_t u16Quantity;
  uint8_t u8DataLength;
  uint16_t u16Crc;
  uint8_t u8Data[260];
} MODBUS_SERIAL_FORMAT_t;
#pragma pack(pop)

void MODBUS_Create_Thread(void);

#endif /* INC_TASK_MODBUS_H_ */
