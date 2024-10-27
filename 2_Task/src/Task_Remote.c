/*
 * Task_Remote.c
 *
 *  Created on: Aug 24, 2024
 *      Author: USER
 */
#include <stdbool.h>

#include "database.h"

#include "Task_Input.h"
#include "Task_Remote.h"

#include "com_flash.h"
#include "tm1639.h"

#ifdef _USE_CLI
static void cliRemote(uint8_t argc, const char **argv);
#endif

osThreadId_t task_Remote_Handle;
const osThreadAttr_t taskRemote_attributes = { .name = "Remote Thread", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityAboveNormal, };

osTimerId_t Buzzer_ClearTimer_ID;
#define CONTROL_TIMER_BUZZER_CLEAR             1
static void Control_Oneshot_Timer_Callback(void *arg);

Tm1639_t tm1639_io3;
Tm1639_t tm1639_io4;

/**
 * @brief   부저 동작
 * @note
 * @param On or Off
 * @retval
 */
void BUZZER_Control(ONOFF_t on, uint16_t time) {
  if (on == ON) {
    gpio_on(BUZZER);
    osTimerStart(Buzzer_ClearTimer_ID, time);
  } else {
    gpio_off(BUZZER);
  }
}

void StandByTempDisplay(SYSTEM_t *pSystem, uint8_t displayCase) {
  char read_str[10];

  snprintf(read_str, sizeof(read_str), "%d", (uint16_t) pSystem->adcValue.levelSensor[0] / 100);
  tm1639Display_str(&tm1639_io4, IO4_AB_MO_FND, read_str);

  snprintf(read_str, sizeof(read_str), "%d", (uint16_t) pSystem->adcValue.levelSensor[0] % 100);
  tm1639Display_str(&tm1639_io3, IO3_AB_BR_FND, read_str);

}

void StandByLevelDisplay(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  char read_str[10] = { 0 };

  snprintf(read_str, sizeof(read_str), "%d", ((uint16_t) pSystem->adcValue.levelVoltAvg[0] / 100));
  tm1639Display_str(&tm1639_io3, IO3_A_LEVEL_FND, read_str);

  snprintf(read_str, sizeof(read_str), "%d", ((uint16_t) pSystem->adcValue.levelVoltAvg[0] % 100));
  tm1639Display_str(&tm1639_io4, IO4_B_LEVEL_FND, read_str);

}

uint8_t displayCase = 0;
uint8_t displayCnt = 0;
/**
 * @brief 대기상태에서 전면 화면 제어
 * @details 버튼으로 설정화면에 들어가지않은 상태, LED, FND 표시
 * @param args SYSTEM_t 구조체 SET_DATA_t 구조체
 * @return none
 */
void led_Control(SYSTEM_t *pSystem, SET_DATA_t *pELTop) {
  static uint8_t ledStep = 0;
  static uint32_t pre_time;

  if (pELTop->remoteData.setData == STAND_BY) {
    switch (ledStep) {
      case 0:
        pre_time = HAL_GetTick();
        ledStep++;
        break;
      case 1:
        if (HAL_GetTick() - pre_time >= 500) {
          pre_time = HAL_GetTick();
          if (pELTop->userData.channel == 0) {
            displayCase = 0;
          } else if (pELTop->userData.channel == 1) {
            displayCase = 1;
          } else {
            displayCnt++;
            if (displayCnt > 4) {
              displayCnt = 0;
              displayCase ^= 1;
            }
          }
          StandByTempDisplay(pSystem, displayCase);
          StandByLevelDisplay(pSystem, pELTop);
        }
        break;
    }
  }
}

/**
 * @brief TM1639 초기화
 * @details TM1639 GPIO 초기화 및 Init
 * @param args none
 * @return none
 */
void initTm1639(void) {
  tm1639_io3.clk_gpio = GPIOB;
  tm1639_io3.clk_pin = GPIO_PIN_8;
  tm1639_io3.dio_gpio = GPIOB;
  tm1639_io3.dio_pin = GPIO_PIN_9;
  tm1639_io3.stb_gpio = GPIOC;
  tm1639_io3.stb_pin = GPIO_PIN_14;
  initTM1639(&tm1639_io3);

  tm1639_io4.clk_gpio = GPIOA;
  tm1639_io4.clk_pin = GPIO_PIN_11;
  tm1639_io4.dio_gpio = GPIOA;
  tm1639_io4.dio_pin = GPIO_PIN_12;
  tm1639_io4.stb_gpio = GPIOC;
  tm1639_io4.stb_pin = GPIO_PIN_15;
  initTM1639(&tm1639_io4);

}

void Remote_Task(void *argument) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  SET_DATA_t *pELTop = (SET_DATA_t*) DataBase_Get_Setting_Data();
  Buzzer_ClearTimer_ID = osTimerNew(Control_Oneshot_Timer_Callback, osTimerOnce, (void*) CONTROL_TIMER_BUZZER_CLEAR, NULL);

#ifdef _USE_CLI
  cliAdd("remote", cliRemote);
#endif

  initTm1639();

  while (1) {
    led_Control(pSystem, pELTop);
    osDelay(10);
  }
}

void REMOTE_Create_Thread(void) {
  task_Remote_Handle = osThreadNew(Remote_Task, NULL, &taskRemote_attributes);
}

static void Control_Oneshot_Timer_Callback(void *arg) {
  uint32_t check = (uint32_t) arg;

  if (check == CONTROL_TIMER_BUZZER_CLEAR) {
    BUZZER_Control(OFF, 0);
  }
}

#ifdef _USE_CLI
void cliRemote(uint8_t argc, const char **argv) {
  bool ret = false;

  if (argc == 1 && cliIsStr(argv[0], "show")) {
    while (cliKeepLoop()) {
      cliPrintf("\n");
      osDelay(100);
    }

    ret = true;
  }

  if (ret != true) {
    cliPrintf("control show\n");
  }
}
#endif
