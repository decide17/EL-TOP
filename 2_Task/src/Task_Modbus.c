/*
 * Task_Modbus.c
 *
 *  Created on: May 30, 2024
 *      Author: sjpark
 */

#include "Task_Modbus.h"
#include "queue.h"
#include "Task_Cli.h"
#include "com_gpio.h"
#include "database.h"

#include "Task_Input.h"

osThreadId_t task_Modbus_Handle;
const osThreadAttr_t taskModbus_attributes = { .name = "MODBUS Task", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };

MODBUS_SERIAL_FORMAT_t modbusReq;
UART_DMA_RECEIVE_HANDLE_t modbusRxHndl;
UART_DMA_SEND_HANDLE_t modbusTxHndl;

int read_uart_dma(UART_HandleTypeDef *Interface, UART_DMA_RECEIVE_HANDLE_t *tDma) {
  /* DMA handle ???? update */
  tDma->RxRealGetPos = (uint16_t) __HAL_DMA_GET_COUNTER(Interface->hdmarx);
  tDma->RxCurrentPos = (UART_DMA_SIZE - tDma->RxRealGetPos) % UART_DMA_SIZE;
  tDma->RxNextPos = tDma->RxCurrentPos + 1u;
  tDma->RxCount = ((UART_DMA_SIZE + tDma->RxCurrentPos) - tDma->RxStartPos) % UART_DMA_SIZE;

  if ((tDma->RxStartFlag != 1u) && (tDma->RxCurrentPos != tDma->RxStartPos)) {
    tDma->RxStartFlag = 1u;
    tDma->RxIdle = 0u;
  } else {
  }

  if (tDma->RxStartFlag == 1u) {
    tDma->RxIdle++;
  } else {
  }

  if (tDma->RxIdle > 100u) /* 1 = 10msec */{
    tDma->RxStartPos = tDma->RxCurrentPos;
    tDma->RxStartFlag = 0u;
    tDma->RxCount = 0u;
    tDma->RxIdle = 0u;
  } else {
  }

  return tDma->RxStartFlag;
}

uint16_t MODBUS_CRC16_v3(const unsigned char *buf, unsigned int len) {
  static const uint16_t table[256] = { 0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481,
      0x0440, 0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841, 0xD801, 0x18C0,
      0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41, 0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701,
      0x17C0, 0x1680, 0xD641, 0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040, 0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
      0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441, 0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41, 0xFA01, 0x3AC0, 0x3B80,
      0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840, 0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1,
      0xEC81, 0x2C40, 0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041, 0xA001,
      0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441, 0x6C00, 0xACC1, 0xAD81, 0x6D40,
      0xAF01, 0x6FC0, 0x6E80, 0xAE41, 0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840, 0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80,
      0xBA41, 0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40, 0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 0x7200, 0xB2C1,
      0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041, 0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500,
      0x95C1, 0x9481, 0x5440, 0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
      0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41, 0x4400, 0x84C1, 0x8581,
      0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 };

  uint8_t xor = 0;
  uint16_t crc = 0xFFFF;

  while (len--) {
    xor = (*buf++) ^ crc;
    crc >>= 8;
    crc ^= table[xor];
  }

  return crc;
}

int parse_modbus_serial(MODBUS_SERIAL_FORMAT_t *tReq, UART_DMA_RECEIVE_HANDLE_t *tDma) {
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  uint8_t rxdata[UART_DMA_SIZE ];
  uint16_t i, calCrc = 0u;
  int ret = 0;

  for (i = 0u; i < UART_DMA_SIZE ; i++) {
    rxdata[i] = 0u;
  }

  tReq->u8UnitId = 0;
  tReq->u8FuncCode = 0u;
  tReq->u16RegAddr = 0u;
  tReq->u16Quantity = 0u;
  tReq->u8DataLength = 0u;
  tReq->u16Crc = 0u;
  for (i = 0u; i < 260u; i++) {
    tReq->u8Data[i] = 0u;
  }

  if (tDma->RxCount >= 8u) {
    for (i = 0u; i < tDma->RxCount; i++) {
      rxdata[i] = tDma->RxBuf[(tDma->RxStartPos + i) % UART_DMA_SIZE ];
    }

    if (pELTop->userData.rs485Id != rxdata[0])
      return 2;
    tReq->u8UnitId = rxdata[0];
    tReq->u8FuncCode = rxdata[1];
    tReq->u16RegAddr = ((uint16_t) rxdata[2] << 8) + rxdata[3];

    switch (tReq->u8FuncCode) {
      case 0x03:
      case 0x04:
        tReq->u16Quantity = ((uint16_t) rxdata[4] << 8) + rxdata[5];
        if ((tReq->u16Quantity >= 1u) || (tReq->u16Quantity <= 125u)) {
          tReq->u16Crc = ((uint16_t) rxdata[7] << 8) + rxdata[6];  // Modbus CRC : LSB first
          calCrc = MODBUS_CRC16_v3(rxdata, 6u);
          ret = 0;
        } else {
          ret = 3; /* ExceptionCode = 3 : quantity inavlid */
        }
        break;

      case 0x06:
        tReq->u8Data[0] = rxdata[4];
        tReq->u8Data[1] = rxdata[5];
        tReq->u16Crc = ((uint16_t) rxdata[7] << 8) + rxdata[6];
        calCrc = MODBUS_CRC16_v3(rxdata, 6u);
        break;

      case 0x10:
        tReq->u16Quantity = ((uint16_t) rxdata[4] << 8) + rxdata[5];
        if ((tReq->u16Quantity >= 1u) || (tReq->u16Quantity <= 123u)) {
          tReq->u8DataLength = rxdata[6];

          if (tDma->RxCount >= (tReq->u8DataLength + 9u)) {
            for (i = 0u; i < tReq->u8DataLength; i++) {
              tReq->u8Data[i] = rxdata[(uint16_t) 7u + i];
            }
            tReq->u16Crc = ((uint16_t) rxdata[8u + tReq->u8DataLength] << 8) + rxdata[7u + tReq->u8DataLength];
            calCrc = MODBUS_CRC16_v3(rxdata, (7u + tReq->u8DataLength));
          } else {
            ret = 3; /* ExceptionCode = 3 : quantity inavlid */
          }
        } else {
          ret = 3; /* ExceptionCode = 3 : quantity inavlid */
        }
        break;

      default:
        ret = 0xff; /* ExceptionCode = 1 : function code inavlid */
        break;
    }

    if (ret == 0) {
      tDma->RxStartPos = tDma->RxCurrentPos;
      tDma->RxStartFlag = 0u;

      if (tReq->u16Crc != calCrc) {
        ret = -1;
      }
    }
  } else {
    tDma->RxStartFlag = 0u;
    ret = 0xff;
  }

  return ret;
}

// RTC_DATA_t tSetRtcData;
uint8_t modbus_serial_send_packet(UART_DMA_SEND_HANDLE_t *tDmaTx, MODBUS_MAP_t *tModMap, MODBUS_SERIAL_FORMAT_t *tReq) {
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  uint16_t *pData = 0;
  uint16_t crc = 0u;
  uint16_t i = 0u;
  uint8_t TempSetting = 0;
  uint8_t UserSetting = 0;
  uint8_t CalSetting = 0;
  uint8_t LevelSetting = 0;
  pData = NULL;

  switch (tReq->u8FuncCode) {
    case 0x03: /* Read multiple registers */
    case 0x04: /* Read input registers */
      if ((tReq->u16RegAddr >= tModMap->AddrInfo[CONTROL_MAP].StartAddress) && (tReq->u16RegAddr <= tModMap->AddrInfo[CONTROL_MAP].EndAddress)) {
        if (tReq->u16Quantity <= tModMap->AddrInfo[CONTROL_MAP].AddressSize) {
          pData = (uint16_t*) &tModMap->Control;
          pData += (tReq->u16RegAddr - tModMap->AddrInfo[CONTROL_MAP].StartAddress);
        }
      }
//      else if ((tReq->u16RegAddr >= tModMap->AddrInfo[CONTROL_CMD_MAP].StartAddress) && (tReq->u16RegAddr <= tModMap->AddrInfo[CONTROL_CMD_MAP].EndAddress)) {
//        if (tReq->u16Quantity <= tModMap->AddrInfo[CONTROL_CMD_MAP].AddressSize) {
//          pData = (uint16_t*) &tModMap->Control_CMD;
//          pData += (tReq->u16RegAddr - tModMap->AddrInfo[CONTROL_CMD_MAP].StartAddress);
//        }
//      }
      tDmaTx->TxBuf[0] = tReq->u8UnitId;
      tDmaTx->TxBuf[1] = tReq->u8FuncCode;
      tDmaTx->TxBuf[2] = ((uint8_t) (tReq->u16Quantity & 0x00FF) * 2u);
      for (i = 0u; i < tReq->u16Quantity; i++) {
        tDmaTx->TxBuf[(i * 2) + 3] = (uint8_t) (*pData >> 8);
        tDmaTx->TxBuf[(i * 2) + 4] = (uint8_t) (*pData & 0x00ffu);
        pData += 1u;
      }
      crc = MODBUS_CRC16_v3(&tDmaTx->TxBuf[0], 3 + tDmaTx->TxBuf[2]); /* 3 = Unit ID(1) + Function Code(1) + DataLenth(1) */
      tDmaTx->TxBuf[tDmaTx->TxBuf[2] + 3] = ((uint8_t) (crc & 0x00FF));
      tDmaTx->TxBuf[tDmaTx->TxBuf[2] + 4] = ((uint8_t) ((crc & 0xFF00) >> 8));
      tDmaTx->TxLength = 5 + tDmaTx->TxBuf[2]; /* 5 = Unit ID(1) + Function Code(1) + DataLenth(1) + Data(n) + CRC(2) */
      break;

    case 0x06:
      /* Write Single registers */
      if ((tReq->u16RegAddr >= tModMap->AddrInfo[CONTROL_MAP].StartAddress) && (tReq->u16RegAddr <= tModMap->AddrInfo[CONTROL_MAP].EndAddress)) {
        if (tReq->u16Quantity <= tModMap->AddrInfo[CONTROL_MAP].AddressSize) {
          pData = (uint16_t*) &tModMap->Control;
          pData += (tReq->u16RegAddr - tModMap->AddrInfo[CONTROL_MAP].StartAddress);
        }
      }

      *pData = (uint16_t) (((uint16_t) tReq->u8Data[i * 2] << 8) & 0xff00u) | (uint16_t) ((uint16_t) tReq->u8Data[(i * 2) + 1u] & 0x00ffu);

      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 14)) {
        sDate.Year = tModMap->Control.Year;
        pSystem->rtcValue.rtcStatus = 1;
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 15)) {
        sDate.Month = tModMap->Control.Month;
        pSystem->rtcValue.rtcStatus = 1;
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 16)) {
        sDate.WeekDay = tModMap->Control.Week;
        pSystem->rtcValue.rtcStatus = 1;
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 17)) {
        sDate.Date = tModMap->Control.Day;
        pSystem->rtcValue.rtcStatus = 1;
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 18)) {
        sTime.Hours = tModMap->Control.Hour;
        pSystem->rtcValue.rtcStatus = 1;
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 19)) {
        sTime.Minutes = tModMap->Control.Minute;
        pSystem->rtcValue.rtcStatus = 1;
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 20)) {
        sTime.Seconds = tModMap->Control.Second;
        pSystem->rtcValue.rtcStatus = 1;
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 21)) {
        pELTop->tempData.aMoTemp = tModMap->Control.tempData[0];
        TempSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 22)) {
        pELTop->tempData.aBrTemp = tModMap->Control.tempData[1];
        TempSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 23)) {
        pELTop->tempData.bMoTemp = tModMap->Control.tempData[2];
        TempSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 24)) {
        pELTop->tempData.bBrTemp = tModMap->Control.tempData[3];
        TempSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 25)) {
        pELTop->tempData.aHtOnTemp = tModMap->Control.tempData[4];
        TempSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 26)) {
        pELTop->tempData.aHtOffTemp = tModMap->Control.tempData[5];
        TempSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 27)) {
        pELTop->tempData.bHtOnTemp = tModMap->Control.tempData[6];
        TempSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 28)) {
        pELTop->tempData.bHtOffTemp = tModMap->Control.tempData[7];
        TempSettingDataFlashSave();
      }
      ////////////////////////////////////////////////////////////////////////////////////
      else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 29)) {
        pELTop->userData.channel = tModMap->Control.userData[0];
        UserSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 30)) {
        pELTop->userData.rs485Id = tModMap->Control.userData[1];
        UserSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 31)) {
        pELTop->userData.rs485Bps = tModMap->Control.userData[2];
        UserSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 32)) {
        pELTop->userData.AutoReset = tModMap->Control.userData[3];
        UserSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 33)) {
        pELTop->userData.TripOnDelay = tModMap->Control.userData[4];
        UserSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 34)) {
        pELTop->userData.aMoLineResAdj = tModMap->Control.userData[5];
        UserSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 35)) {
        pELTop->userData.aBrLineResAdj = tModMap->Control.userData[6];
        UserSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 36)) {
        pELTop->userData.bMoLineResAdj = tModMap->Control.userData[7];
        UserSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 37)) {
        pELTop->userData.bBrLineResAdj = tModMap->Control.userData[8];
        UserSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 38)) {
        pELTop->sdData.sdFlag = tModMap->Control.userData[9];
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 39)) {
        pELTop->calData.a1_LowOffSet = tModMap->Control.calData[0];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 40)) {
        pELTop->calData.a2_LowOffSet = tModMap->Control.calData[1];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 41)) {
        pELTop->calData.b1_LowOffSet = tModMap->Control.calData[2];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 42)) {
        pELTop->calData.b2_LowOffSet = tModMap->Control.calData[3];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 43)) {
        pELTop->calData.c1_LowOffSet = tModMap->Control.calData[4];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 44)) {
        pELTop->calData.c1_HighOffSet = tModMap->Control.calData[5];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 45)) {
        pELTop->calData.c2_LowOffSet = tModMap->Control.calData[6];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 46)) {
        pELTop->calData.c2_HighOffSet = tModMap->Control.calData[7];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 47)) {
        pELTop->calData.d1_LowOffSet = tModMap->Control.calData[8];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 48)) {
        pELTop->calData.d1_HighOffSet = tModMap->Control.calData[9];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 49)) {
        pELTop->calData.d2_LowOffSet = tModMap->Control.calData[10];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 50)) {
        pELTop->calData.d2_HighOffSet = tModMap->Control.calData[11];
        CalSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 51)) {
        pELTop->levData.selectedSensorA = tModMap->Control.levelData[0];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 52)) {
        pELTop->levData.aMeterCal = tModMap->Control.levelData[1];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 53)) {
        pELTop->levData.aStartMeterSet = tModMap->Control.levelData[2];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 54)) {
        pELTop->levData.aStopMeterSet = tModMap->Control.levelData[3];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 55)) {
        pELTop->levData.aDownLimitMeterSet = tModMap->Control.levelData[4];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 56)) {
        pELTop->levData.aUpLimitMeterSet = tModMap->Control.levelData[5];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 57)) {
        pELTop->levData.aPumpSwitchTimeSet = tModMap->Control.levelData[6];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 58)) {
        pELTop->levData.aPumpDelaySet = tModMap->Control.levelData[7];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 59)) {
        pELTop->levData.selectedSensorB = tModMap->Control.levelData[8];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 60)) {
        pELTop->levData.bMeterCal = tModMap->Control.levelData[9];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 61)) {
        pELTop->levData.bStartMeterSet = tModMap->Control.levelData[10];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 62)) {
        pELTop->levData.bStopMeterSet = tModMap->Control.levelData[11];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 63)) {
        pELTop->levData.bDownLimitMeterSet = tModMap->Control.levelData[12];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 64)) {
        pELTop->levData.bUpLimitMeterSet = tModMap->Control.levelData[13];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 65)) {
        pELTop->levData.bPumpSwitchTimeSet = tModMap->Control.levelData[14];
        LevelSettingDataFlashSave();
      } else if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 66)) {
        pELTop->levData.bPumpDelaySet = tModMap->Control.levelData[15];
        LevelSettingDataFlashSave();
      }

      tDmaTx->TxBuf[0] = tReq->u8UnitId;
      tDmaTx->TxBuf[1] = tReq->u8FuncCode;
      tDmaTx->TxBuf[2] = ((uint8_t) ((tReq->u16RegAddr & 0xFF00) >> 8));
      tDmaTx->TxBuf[3] = ((uint8_t) (tReq->u16RegAddr & 0x00FF));
      tDmaTx->TxBuf[4] = ((uint8_t) ((*pData & 0xFF00) >> 8));
      tDmaTx->TxBuf[5] = ((uint8_t) (*pData & 0x00FF));
      crc = MODBUS_CRC16_v3(&tDmaTx->TxBuf[0], 6); /* 6 = Unit ID(1) + Function Code(1) + Reg.Address(2) + Quantity(2) */
      tDmaTx->TxBuf[6] = ((uint8_t) (crc & 0x00FF));
      tDmaTx->TxBuf[7] = ((uint8_t) ((crc & 0xFF00) >> 8));
      tDmaTx->TxLength = 8; /* 8 = Unit ID(1) + Function Code(1) + Reg.Address(2) + Quantity(2) + CRC(2) */
      break;

    case 0x10:
      if ((tReq->u16RegAddr >= tModMap->AddrInfo[CONTROL_MAP].StartAddress) && (tReq->u16RegAddr <= tModMap->AddrInfo[CONTROL_MAP].EndAddress)) {
        if (tReq->u16Quantity <= tModMap->AddrInfo[CONTROL_MAP].AddressSize) {
          pData = (uint16_t*) &tModMap->Control;
          pData += (tReq->u16RegAddr - tModMap->AddrInfo[CONTROL_MAP].StartAddress);
        }
      }

      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 14)) {
        sDate.Year = tModMap->Control.Year;
        pSystem->rtcValue.rtcStatus = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 15)) {
        sDate.Month = tModMap->Control.Month;
        pSystem->rtcValue.rtcStatus = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 16)) {
        sDate.WeekDay = tModMap->Control.Week;
        pSystem->rtcValue.rtcStatus = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 17)) {
        sDate.Date = tModMap->Control.Day;
        pSystem->rtcValue.rtcStatus = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 18)) {
        sTime.Hours = tModMap->Control.Hour;
        pSystem->rtcValue.rtcStatus = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 19)) {
        sTime.Minutes = tModMap->Control.Minute;
        pSystem->rtcValue.rtcStatus = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 20)) {
        sTime.Seconds = tModMap->Control.Second;
        pSystem->rtcValue.rtcStatus = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 21)) {
        pELTop->tempData.aMoTemp = tModMap->Control.tempData[0];
        TempSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 22)) {
        pELTop->tempData.aBrTemp = tModMap->Control.tempData[1];
        TempSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 23)) {
        pELTop->tempData.bMoTemp = tModMap->Control.tempData[2];
        TempSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 24)) {
        pELTop->tempData.bBrTemp = tModMap->Control.tempData[3];
        TempSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 25)) {
        pELTop->tempData.aHtOnTemp = tModMap->Control.tempData[4];
        TempSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 26)) {
        pELTop->tempData.aHtOffTemp = tModMap->Control.tempData[5];
        TempSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 27)) {
        pELTop->tempData.bHtOnTemp = tModMap->Control.tempData[6];
        TempSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 28)) {
        pELTop->tempData.bHtOffTemp = tModMap->Control.tempData[7];
        TempSetting = 1;
      }
      if (TempSetting == 1)
        TempSettingDataFlashSave();
      ////////////////////////////////////////////////////////////////////////////////////
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 29)) {
        pELTop->userData.channel = tModMap->Control.userData[0];
        UserSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 30)) {
        pELTop->userData.rs485Id = tModMap->Control.userData[1];
        UserSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 31)) {
        pELTop->userData.rs485Bps = tModMap->Control.userData[2];
        UserSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 32)) {
        pELTop->userData.AutoReset = tModMap->Control.userData[3];
        UserSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 33)) {
        pELTop->userData.TripOnDelay = tModMap->Control.userData[4];
        UserSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 34)) {
        pELTop->userData.aMoLineResAdj = tModMap->Control.userData[5];
        UserSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 35)) {
        pELTop->userData.aBrLineResAdj = tModMap->Control.userData[6];
        UserSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 36)) {
        pELTop->userData.bMoLineResAdj = tModMap->Control.userData[7];
        UserSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 37)) {
        pELTop->userData.bBrLineResAdj = tModMap->Control.userData[8];
        UserSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 38)) {
        pELTop->sdData.sdFlag = tModMap->Control.userData[9];
      }
      if (UserSetting == 1)
        UserSettingDataFlashSave();
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 39)) {
        pELTop->calData.a1_LowOffSet = tModMap->Control.calData[0];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 40)) {
        pELTop->calData.a2_LowOffSet = tModMap->Control.calData[1];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 41)) {
        pELTop->calData.b1_LowOffSet = tModMap->Control.calData[2];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 42)) {
        pELTop->calData.b2_LowOffSet = tModMap->Control.calData[3];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 43)) {
        pELTop->calData.c1_LowOffSet = tModMap->Control.calData[4];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 44)) {
        pELTop->calData.c1_HighOffSet = tModMap->Control.calData[5];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 45)) {
        pELTop->calData.c2_LowOffSet = tModMap->Control.calData[6];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 46)) {
        pELTop->calData.c2_HighOffSet = tModMap->Control.calData[7];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 47)) {
        pELTop->calData.d1_LowOffSet = tModMap->Control.calData[8];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 48)) {
        pELTop->calData.d1_HighOffSet = tModMap->Control.calData[9];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 49)) {
        pELTop->calData.d2_LowOffSet = tModMap->Control.calData[10];
        CalSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 50)) {
        pELTop->calData.d2_HighOffSet = tModMap->Control.calData[11];
        CalSetting = 1;
      }
      if (CalSetting == 1)
        CalSettingDataFlashSave();
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 51)) {
        pELTop->levData.selectedSensorA = tModMap->Control.levelData[0];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 52)) {
        pELTop->levData.aMeterCal = tModMap->Control.levelData[1];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 53)) {
        pELTop->levData.aStartMeterSet = tModMap->Control.levelData[2];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 54)) {
        pELTop->levData.aStopMeterSet = tModMap->Control.levelData[3];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 55)) {
        pELTop->levData.aDownLimitMeterSet = tModMap->Control.levelData[4];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 56)) {
        pELTop->levData.aUpLimitMeterSet = tModMap->Control.levelData[5];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 57)) {
        pELTop->levData.aPumpSwitchTimeSet = tModMap->Control.levelData[6];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 58)) {
        pELTop->levData.aPumpDelaySet = tModMap->Control.levelData[7];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 59)) {
        pELTop->levData.selectedSensorB = tModMap->Control.levelData[8];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 60)) {
        pELTop->levData.bMeterCal = tModMap->Control.levelData[9];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 61)) {
        pELTop->levData.bStartMeterSet = tModMap->Control.levelData[10];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 62)) {
        pELTop->levData.bStopMeterSet = tModMap->Control.levelData[11];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 63)) {
        pELTop->levData.bDownLimitMeterSet = tModMap->Control.levelData[12];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 64)) {
        pELTop->levData.bUpLimitMeterSet = tModMap->Control.levelData[13];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 65)) {
        pELTop->levData.bPumpSwitchTimeSet = tModMap->Control.levelData[14];
        LevelSetting = 1;
      }
      if (tReq->u16RegAddr == (tModMap->AddrInfo[CONTROL_MAP].StartAddress + 66)) {
        pELTop->levData.bPumpDelaySet = tModMap->Control.levelData[15];
        LevelSetting = 1;
      }
      if (LevelSetting == 1)
        LevelSettingDataFlashSave();

      for (i = 0u; i < (tReq->u16Quantity); i++) {
        *pData = (uint16_t) (((uint16_t) tReq->u8Data[i * 2] << 8) & 0xff00u) | (uint16_t) ((uint16_t) tReq->u8Data[(i * 2) + 1u] & 0x00ffu);
        pData += 1u;
      }

      tDmaTx->TxBuf[0] = tReq->u8UnitId;
      tDmaTx->TxBuf[1] = tReq->u8FuncCode;
      tDmaTx->TxBuf[2] = ((uint8_t) ((tReq->u16RegAddr & 0xFF00) >> 8));
      tDmaTx->TxBuf[3] = ((uint8_t) (tReq->u16RegAddr & 0x00FF));
      tDmaTx->TxBuf[4] = ((uint8_t) ((tReq->u16Quantity & 0xFF00) >> 8));
      tDmaTx->TxBuf[5] = ((uint8_t) (tReq->u16Quantity & 0x00FF));
      crc = MODBUS_CRC16_v3(&tDmaTx->TxBuf[0], 6); /* 6 = Unit ID(1) + Function Code(1) + Reg.Address(2) + Quantity(2) */
      tDmaTx->TxBuf[6] = ((uint8_t) (crc & 0x00FF));
      tDmaTx->TxBuf[7] = ((uint8_t) ((crc & 0xFF00) >> 8));
      tDmaTx->TxLength = 8; /* 8 = Unit ID(1) + Function Code(1) + Reg.Address(2) + Quantity(2) + CRC(2) */
      break;

    default:
      break;
  }

  return 1;
}

void Modbus_Process(void) {

  int packetError = 0;
  MODBUS_MAP_t *pMODBUS = (MODBUS_MAP_t*) DataBase_Get_pMODBUS_Data();

  HAL_UART_Receive_DMA(&huart2, (uint8_t*) &modbusRxHndl.RxBuf[0], UART_DMA_SIZE);

  if (read_uart_dma(&huart2, &modbusRxHndl) == 1) {
    packetError = parse_modbus_serial(&modbusReq, &modbusRxHndl);
    if (packetError == 0) { /* not error */
      modbus_serial_send_packet(&modbusTxHndl, pMODBUS, &modbusReq);

      if (modbusTxHndl.TxLength > 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        HAL_UART_Transmit(&huart2, (uint8_t*) &modbusTxHndl.TxBuf[0], modbusTxHndl.TxLength, 200);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
      } else { /* ExceptionCode = 4 : read inavlid */
      }
    } else {
      if (packetError == 1) { /* ExceptionCode = 1 : function code inavlid */
      } else if (packetError == 2) { /* ExceptionCode = 2 : data address inavlid */
      } else if (packetError == 3) { /* ExceptionCode = 3 : quantity inavlid */
      } else if (packetError == -1) { /* CRC Error */
      } else if (packetError == 0xff) { /* No data */
      } else {
      }
    }
  }
}

void Modbus_Init() {
  MODBUS_MAP_t *pMODBUS = (MODBUS_MAP_t*) DataBase_Get_pMODBUS_Data();

  pMODBUS->AddrInfo[CONTROL_MAP].AddressSize = sizeof(pMODBUS->Control);
  pMODBUS->AddrInfo[CONTROL_MAP].StartAddress = 0u;
  pMODBUS->AddrInfo[CONTROL_MAP].EndAddress = pMODBUS->AddrInfo[CONTROL_MAP].StartAddress + (pMODBUS->AddrInfo[CONTROL_MAP].AddressSize / 2u) - 1u;

//  pMODBUS->AddrInfo[CONTROL_CMD_MAP].AddressSize = sizeof(pMODBUS->Control_CMD);
//  pMODBUS->AddrInfo[CONTROL_CMD_MAP].StartAddress = pMODBUS->AddrInfo[CONTROL_MAP].EndAddress + 1;
//  pMODBUS->AddrInfo[CONTROL_CMD_MAP].EndAddress = pMODBUS->AddrInfo[CONTROL_CMD_MAP].StartAddress + (pMODBUS->AddrInfo[CONTROL_CMD_MAP].AddressSize / 2u) - 1u;

}

void Update_Modbus_Map_Control() {
  MODBUS_MAP_t *pMODBUS = (MODBUS_MAP_t*) DataBase_Get_pMODBUS_Data();
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();

  pMODBUS->Control.FWVersionMajor = 0;
  pMODBUS->Control.FWVersionMinor = 0;
  pMODBUS->Control.FWVersionPatch = 6;

  pMODBUS->Control.InputStatus = pSystem->inputValue.NAME_FIELD.waterSen1;
  pMODBUS->Control.InputStatus |= (pSystem->inputValue.NAME_FIELD.waterSen2 << 1);
  pMODBUS->Control.Pt100Value[0] = (int16_t) pSystem->pt100Value.pt100Cal[0];
  pMODBUS->Control.Pt100Value[1] = (int16_t) pSystem->pt100Value.pt100Cal[1];
  pMODBUS->Control.Pt100Value[2] = (int16_t) pSystem->pt100Value.pt100Cal[2];
  pMODBUS->Control.Pt100Value[3] = (int16_t) pSystem->pt100Value.pt100Cal[3];
  pMODBUS->Control.LevelSensor[0] = (int16_t) pSystem->adcValue.levelSensorCal[0];
  pMODBUS->Control.LevelSensor[1] = (int16_t) pSystem->adcValue.levelSensorCal[1];

  pMODBUS->Control.Year = sDate.Year;
  pMODBUS->Control.Month = sDate.Month;
  pMODBUS->Control.Week = sDate.WeekDay;
  pMODBUS->Control.Day = sDate.Date;
  pMODBUS->Control.Hour = sTime.Hours;
  pMODBUS->Control.Minute = sTime.Minutes;
  pMODBUS->Control.Second = sTime.Seconds;

  pMODBUS->Control.tempData[0] = pELTop->tempData.aMoTemp;
  pMODBUS->Control.tempData[1] = pELTop->tempData.aBrTemp;
  pMODBUS->Control.tempData[2] = pELTop->tempData.bMoTemp;
  pMODBUS->Control.tempData[3] = pELTop->tempData.bBrTemp;
  pMODBUS->Control.tempData[4] = pELTop->tempData.aHtOnTemp;
  pMODBUS->Control.tempData[5] = pELTop->tempData.aHtOffTemp;
  pMODBUS->Control.tempData[6] = pELTop->tempData.bHtOnTemp;
  pMODBUS->Control.tempData[7] = pELTop->tempData.bHtOffTemp;

  pMODBUS->Control.userData[0] = pELTop->userData.channel;
  pMODBUS->Control.userData[1] = pELTop->userData.rs485Id;
  pMODBUS->Control.userData[2] = pELTop->userData.rs485Bps;
  pMODBUS->Control.userData[3] = pELTop->userData.AutoReset;
  pMODBUS->Control.userData[4] = pELTop->userData.TripOnDelay;
  pMODBUS->Control.userData[5] = pELTop->userData.aMoLineResAdj;
  pMODBUS->Control.userData[6] = pELTop->userData.aBrLineResAdj;
  pMODBUS->Control.userData[7] = pELTop->userData.bMoLineResAdj;
  pMODBUS->Control.userData[8] = pELTop->userData.bBrLineResAdj;
  pMODBUS->Control.userData[9] = pELTop->sdData.sdFlag;

  pMODBUS->Control.calData[0] = pELTop->calData.a1_LowOffSet;
  pMODBUS->Control.calData[1] = pELTop->calData.a2_LowOffSet;
  pMODBUS->Control.calData[2] = pELTop->calData.b1_LowOffSet;
  pMODBUS->Control.calData[3] = pELTop->calData.b2_LowOffSet;
  pMODBUS->Control.calData[4] = pELTop->calData.c1_LowOffSet;
  pMODBUS->Control.calData[5] = pELTop->calData.c1_HighOffSet;
  pMODBUS->Control.calData[6] = pELTop->calData.c2_LowOffSet;
  pMODBUS->Control.calData[7] = pELTop->calData.c2_HighOffSet;
  pMODBUS->Control.calData[8] = pELTop->calData.d1_LowOffSet;
  pMODBUS->Control.calData[9] = pELTop->calData.d1_HighOffSet;
  pMODBUS->Control.calData[10] = pELTop->calData.d2_LowOffSet;
  pMODBUS->Control.calData[11] = pELTop->calData.d2_HighOffSet;

  pMODBUS->Control.levelData[0] = pELTop->levData.selectedSensorA;
  pMODBUS->Control.levelData[1] = pELTop->levData.aMeterCal;
  pMODBUS->Control.levelData[2] = pELTop->levData.aStopMeterSet;
  pMODBUS->Control.levelData[3] = pELTop->levData.aStartMeterSet;
  pMODBUS->Control.levelData[4] = pELTop->levData.aDownLimitMeterSet;
  pMODBUS->Control.levelData[5] = pELTop->levData.aUpLimitMeterSet;
  pMODBUS->Control.levelData[6] = pELTop->levData.aPumpSwitchTimeSet;
  pMODBUS->Control.levelData[7] = pELTop->levData.aPumpDelaySet;
  pMODBUS->Control.levelData[8] = pELTop->levData.selectedSensorB;
  pMODBUS->Control.levelData[9] = pELTop->levData.bMeterCal;
  pMODBUS->Control.levelData[10] = pELTop->levData.bStopMeterSet;
  pMODBUS->Control.levelData[11] = pELTop->levData.bStartMeterSet;
  pMODBUS->Control.levelData[12] = pELTop->levData.bDownLimitMeterSet;
  pMODBUS->Control.levelData[13] = pELTop->levData.bUpLimitMeterSet;
  pMODBUS->Control.levelData[14] = pELTop->levData.bPumpSwitchTimeSet;
  pMODBUS->Control.levelData[15] = pELTop->levData.bPumpDelaySet;

}

void Update_Modbus_Map(void) {
  Update_Modbus_Map_Control();
}

void MODBUS_Send_Task(void *argument) {
  osDelay(500);
  Modbus_Init();
  for (;;) {
    Update_Modbus_Map();
    Modbus_Process();
    osDelay(10);
  }
}

void MODBUS_Create_Thread(void) {
  task_Modbus_Handle = osThreadNew(MODBUS_Send_Task, NULL, &taskModbus_attributes);
}
