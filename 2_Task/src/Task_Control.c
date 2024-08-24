/*
 * Task_input.c
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */
#include "database.h"
#include "Task_Input.h"
#include "Task_Cli.h"
#include "com_flash.h"
#include "tm1639.h"

#ifdef _USE_CLI
static void cliControl(uint8_t argc, const char **argv);
#endif

osThreadId_t task_Control_Handle;
const osThreadAttr_t taskControl_attributes = { .name = "Control Thread", .stack_size = 256 * 4, .priority = (osPriority_t) osPriorityNormal, };

/**
 * @brief   A 모터 동작
 * @note
 * @param On or Off
 * @retval
 */
void A_MO_Control(ONOFF_t on) {
  if (on == ON) {
    gpio_on(B_PT_CON);
  } else {
    gpio_off(B_PT_CON);
  }
}

void Control_Task(void *argument) {

#ifdef _USE_CLI
  cliAdd("control", cliControl);
#endif

  while (1) {
    osDelay(10);
  }
}

void CONTROL_Create_Thread(void) {
  task_Control_Handle = osThreadNew(Control_Task, NULL, &taskControl_attributes);
}

#ifdef _USE_CLI
void cliControl(uint8_t argc, const char **argv) {
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

