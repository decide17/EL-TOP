/*
 * Factroy_Calibration.c
 *
 *  Created on: Sep 7, 2024
 *      Author: USER
 */

#include "database.h"

#include "Task_Cli.h"
#include "Task_Remote.h"

#include "com_gpio.h"
#include "tm1639.h"

extern void BUZZER_Control(ONOFF_t on, uint16_t time);
extern void RESET_BUTTON(SYSTEM_t *SystemData);
extern int circularValue(int max, int min, int value);
extern Tm1639_t tm1639_io3;
extern Tm1639_t tm1639_io4;

static uint8_t sensor_view = 0;

void FactoryCalibrationStartFunc(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  pELTop->remoteData.setData = FACTORY_CALIBRATION;
  Full_Reset(&tm1639_io3);
  Full_Reset(&tm1639_io4);
  BUZZER_Control(ON, 100);
  pELTop->remoteData.tempStep = 1;
  pELTop->remoteData.read_data = pELTop->calData.a1_LowOffSet;
  tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
  sensor_view = 1;
  cliPrintf("read_temp : %d, tempStep : %d \n", pELTop->remoteData.read_data, pELTop->remoteData.tempStep);
  RESET_BUTTON(pSystem);
  tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A1 ");
}

void FactoryCalibrationSetFunc(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  if (pSystem->buttonVaule.NAME_FIELD.Button_RESET_CHK) {
    switch (pELTop->remoteData.tempStep) {
      case 1:
        sensor_view ^= 1;
        if (sensor_view == 0) {
          tm1639_io4.data.NAME_FIELD.LED1_GREEN = 0;
          tm1639Display_led(&tm1639_io4);
          tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 10));
        } else {
          tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
          tm1639Display_led(&tm1639_io4);
        }
        break;
      case 2:
        sensor_view ^= 1;
        if (sensor_view == 0) {
          tm1639_io3.data.NAME_FIELD.LED8_GREEN = 0;
          tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 10));
        } else
          tm1639_io3.data.NAME_FIELD.LED8_GREEN = 1;
        break;
      case 3:
        sensor_view ^= 1;
        if (sensor_view == 0) {
          tm1639_io4.data.NAME_FIELD.LED2_GREEN = 0;
          tm1639Display_led(&tm1639_io4);
          tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 10));
        } else
          tm1639_io4.data.NAME_FIELD.LED2_GREEN = 1;
        break;
      case 4:
        sensor_view ^= 1;
        if (sensor_view == 0) {
          tm1639_io3.data.NAME_FIELD.LED7_GREEN = 0;
          tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 10));
        } else
          tm1639_io3.data.NAME_FIELD.LED7_GREEN = 1;
        break;
      default:
        sensor_view = 0;
        break;
    }
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_UP_CHK) {
    if (sensor_view == 0) {
      switch (pELTop->remoteData.tempStep) {
        case 1:
        case 2:
        case 3:
        case 4:
          pELTop->remoteData.read_data = circularValue(100, -99, ++pELTop->remoteData.read_data);
          tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 10));
          break;
        case 5:
          pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.c1_LowOffSet = pELTop->remoteData.read_data;
          break;
        case 6:
          pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.c1_HighOffSet = pELTop->remoteData.read_data;
          break;
        case 7:
          pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.c2_LowOffSet = pELTop->remoteData.read_data;
          break;
        case 8:
          pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.c2_HighOffSet = pELTop->remoteData.read_data;
          break;
        case 9:
          pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.d1_LowOffSet = pELTop->remoteData.read_data;
          break;
        case 10:
          pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.d1_HighOffSet = pELTop->remoteData.read_data;
          break;
        case 11:
          pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.d2_LowOffSet = pELTop->remoteData.read_data;
          break;
        case 12:
          pELTop->remoteData.read_data = circularValue(999, 0, ++pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.d2_HighOffSet = pELTop->remoteData.read_data;
          break;
      }
      cliPrintf("Button_UP_CHK read_data : %d tempStep : %d\n", pELTop->remoteData.read_data, pELTop->remoteData.tempStep);
      RESET_BUTTON(pSystem);
    }
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_DOWN_CHK) {
    if (sensor_view == 0) {
      switch (pELTop->remoteData.tempStep) {
        case 1:
        case 2:
        case 3:
        case 4:
          pELTop->remoteData.read_data = circularValue(100, -99, --pELTop->remoteData.read_data);
          tm1639Display_float(&tm1639_io3, IO3_AB_BR_FND, ((float) pELTop->remoteData.read_data / 10));
          break;
        case 5:
          pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.c1_LowOffSet = pELTop->remoteData.read_data;
          break;
        case 6:
          pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.c1_HighOffSet = pELTop->remoteData.read_data;
          break;
        case 7:
          pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.c2_LowOffSet = pELTop->remoteData.read_data;
          break;
        case 8:
          pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.c2_HighOffSet = pELTop->remoteData.read_data;
          break;
        case 9:
          pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.d1_LowOffSet = pELTop->remoteData.read_data;
          break;
        case 10:
          pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.d1_HighOffSet = pELTop->remoteData.read_data;
          break;
        case 11:
          pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.d2_LowOffSet = pELTop->remoteData.read_data;
          break;
        case 12:
          pELTop->remoteData.read_data = circularValue(999, 0, --pELTop->remoteData.read_data);
          tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
          pELTop->calData.d2_HighOffSet = pELTop->remoteData.read_data;
          break;
      }
      cliPrintf("Button_UP_CHK read_data : %d tempStep : %d\n", pELTop->remoteData.read_data, pELTop->remoteData.tempStep);
      RESET_BUTTON(pSystem);
    }
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_CHK) {
    switch (pELTop->remoteData.tempStep) {
      case 1:
        sensor_view = 1;
        tm1639_io4.data.NAME_FIELD.LED1_GREEN = 0;
        tm1639_io3.data.NAME_FIELD.LED8_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A2 ");
        pELTop->calData.a1_LowOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.a2_LowOffSet;
        break;
      case 2:
        sensor_view = 1;
        tm1639_io3.data.NAME_FIELD.LED8_GREEN = 0;
        tm1639_io4.data.NAME_FIELD.LED2_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b1 ");
        pELTop->calData.a2_LowOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.b1_LowOffSet;
        break;
      case 3:
        sensor_view = 1;
        tm1639_io4.data.NAME_FIELD.LED2_GREEN = 0;
        tm1639_io3.data.NAME_FIELD.LED7_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "b2 ");
        pELTop->calData.b1_LowOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.b2_LowOffSet;
        break;
      case 4:
        sensor_view = 0;
        tm1639_io3.data.NAME_FIELD.LED7_GREEN = 0;
        tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "c1L");
        pELTop->calData.b2_LowOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.c1_LowOffSet;
        break;
      case 5:
        sensor_view = 0;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "c1H");
        pELTop->calData.c1_LowOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.c1_HighOffSet;
        break;
      case 6:
        sensor_view = 0;
        tm1639_io4.data.NAME_FIELD.LED1_GREEN = 0;
        tm1639_io3.data.NAME_FIELD.LED8_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "c2L");
        pELTop->calData.c1_HighOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.c2_LowOffSet;
        break;
      case 7:
        sensor_view = 0;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "c2H");
        pELTop->calData.c2_LowOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.c2_HighOffSet;
        break;
      case 8:
        sensor_view = 0;
        tm1639_io3.data.NAME_FIELD.LED8_GREEN = 0;
        tm1639_io4.data.NAME_FIELD.LED2_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "d1L");
        pELTop->calData.c2_HighOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.d1_LowOffSet;
        break;
      case 9:
        sensor_view = 0;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "d1H");
        pELTop->calData.d1_LowOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.d1_HighOffSet;
        break;
      case 10:
        sensor_view = 0;
        tm1639_io4.data.NAME_FIELD.LED2_GREEN = 0;
        tm1639_io3.data.NAME_FIELD.LED7_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "d2L");
        pELTop->calData.d1_HighOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.d2_LowOffSet;
        break;
      case 11:
        sensor_view = 0;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "d2H");
        pELTop->calData.d2_LowOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.d2_HighOffSet;
        break;
      case 12:
        sensor_view = 1;
        tm1639_io3.data.NAME_FIELD.LED7_GREEN = 0;
        tm1639_io4.data.NAME_FIELD.LED1_GREEN = 1;
        tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, "A1 ");
        pELTop->calData.d2_HighOffSet = pELTop->remoteData.read_data;
        pELTop->remoteData.read_data = pELTop->calData.a1_LowOffSet;
        break;
    }
    CalSettingDataFlashSave();
    pELTop->remoteData.tempStep++;
    if (pELTop->remoteData.tempStep > 12)
      pELTop->remoteData.tempStep = 1;
    RESET_BUTTON(pSystem);
    BUZZER_Control(ON, 100);
    if (sensor_view == 0)
      tm1639Display_num(&tm1639_io3, IO3_AB_BR_FND, pELTop->remoteData.read_data);
  } else if (pSystem->buttonVaule.NAME_FIELD.Button_SET_DN) {
    cliPrintf("STAND_BY Set\n");
    BUZZER_Control(ON, 100);
    pELTop->remoteData.setData = STAND_BY;
    RESET_BUTTON(pSystem);
    Full_Reset(&tm1639_io3);
    Full_Reset(&tm1639_io4);
  } else if (pELTop->remoteData.tempStep == 1 && sensor_view == 1) {
    char read_str[10];
    if (pSystem->pt100Value.pt100[0] < 280) {
      if (pSystem->pt100Value.pt100[0] >= 0 && pSystem->pt100Value.pt100[0] < 10) {
        snprintf(read_str, sizeof(read_str), "  %d", (int) pSystem->pt100Value.pt100[0]);
      } else if (pSystem->pt100Value.pt100[0] >= 10 && pSystem->pt100Value.pt100[0] < 100) {
        snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100[0]);
      } else if (pSystem->pt100Value.pt100[0] > -10 && pSystem->pt100Value.pt100[0] <= -1) {
        snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100[0]);
      } else {
        snprintf(read_str, sizeof(read_str), "%d", (int) pSystem->pt100Value.pt100[0]);
      }
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, read_str);
    } else {
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "---");
    }
  } else if (pELTop->remoteData.tempStep == 2 && sensor_view == 1) {
    char read_str[10];
    if (pSystem->pt100Value.pt100[1] < 280) {
      if (pSystem->pt100Value.pt100[1] >= 0 && pSystem->pt100Value.pt100[1] < 10) {
        snprintf(read_str, sizeof(read_str), "  %d", (int) pSystem->pt100Value.pt100[1]);
      } else if (pSystem->pt100Value.pt100[1] >= 10 && pSystem->pt100Value.pt100[1] < 100) {
        snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100[1]);
      } else if (pSystem->pt100Value.pt100[1] > -10 && pSystem->pt100Value.pt100[1] <= -1) {
        snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100[1]);
      } else {
        snprintf(read_str, sizeof(read_str), "%d", (int) pSystem->pt100Value.pt100[1]);
      }
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, read_str);
    } else {
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "---");
    }
  } else if (pELTop->remoteData.tempStep == 3 && sensor_view == 1) {
    char read_str[10];
    if (pSystem->pt100Value.pt100[2] < 280) {
      if (pSystem->pt100Value.pt100[2] >= 0 && pSystem->pt100Value.pt100[2] < 10) {
        snprintf(read_str, sizeof(read_str), "  %d", (int) pSystem->pt100Value.pt100[2]);
      } else if (pSystem->pt100Value.pt100[2] >= 10 && pSystem->pt100Value.pt100[2] < 100) {
        snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100[2]);
      } else if (pSystem->pt100Value.pt100[2] > -10 && pSystem->pt100Value.pt100[2] <= -1) {
        snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100[2]);
      } else {
        snprintf(read_str, sizeof(read_str), "%d", (int) pSystem->pt100Value.pt100[2]);
      }
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, read_str);
    } else {
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "---");
    }
  } else if (pELTop->remoteData.tempStep == 4 && sensor_view == 1) {
    char read_str[10];
    if (pSystem->pt100Value.pt100[3] < 280) {
      if (pSystem->pt100Value.pt100[3] >= 0 && pSystem->pt100Value.pt100[3] < 10) {
        snprintf(read_str, sizeof(read_str), "  %d", (int) pSystem->pt100Value.pt100[3]);
      } else if (pSystem->pt100Value.pt100[3] >= 10 && pSystem->pt100Value.pt100[3] < 100) {
        snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100[3]);
      } else if (pSystem->pt100Value.pt100[3] > -10 && pSystem->pt100Value.pt100[3] <= -1) {
        snprintf(read_str, sizeof(read_str), " %d", (int) pSystem->pt100Value.pt100[3]);
      } else {
        snprintf(read_str, sizeof(read_str), "%d", (int) pSystem->pt100Value.pt100[3]);
      }
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, read_str);
    } else {
      tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, "---");
    }
  }

}
