/*
 * input.c
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */
#include <math.h>
#include "database.h"
#include<stdlib.h>
#include "Task_Input.h"
#include "Task_Cli.h"
#include "TM1640.h"
#include "tm1637.h"
#include "mcp4728.h"
#include "fatfs.h"

#ifdef _USE_CLI
static void cliinput(uint8_t argc, const char **argv);
#endif

uint8_t btn;
bool button1;
bool button2;
//////////////////////////////////////////////////
////////////////////////////////////////
/////////////////////////////
FATFS fs;
FATFS *pfs;
FIL fil;
FRESULT fres;
DWORD fre_clust;
uint32_t total, free1;
char buffer[100];
uint8_t none;
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

/**
 * @brief   A 모터 동작
 * @note
 * @param On or Off
 * @retval
 */
void BUZZER_Control(ONOFF_t on, uint16_t time) {
  if (on == ON) {
    gpio_on(BUZZER);
    osDelay(time);
  } else {
    gpio_off(BUZZER);
  }
}

void file_system_test() {
  if (f_mount(&fs, "", 0) != FR_OK)
    none = 1;
//  _Error_Handler(__FILE__, __LINE__);

  /* Open file to write */
  if (f_open(&fil, "first.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) != FR_OK)
    none = 1;
//  _Error_Handler(__FILE__, __LINE__);

  /* Check free space */
  if (f_getfree("", &fre_clust, &pfs) != FR_OK)
    none = 1;
//    _Error_Handler(__FILE__, __LINE__);

  total = (uint32_t) ((pfs->n_fatent - 2) * pfs->csize * 0.5);
  free1 = (uint32_t) (fre_clust * pfs->csize * 0.5);

  /* Free space is less than 1kb */
  if (free1 < 1)
    none = 1;
//    _Error_Handler(__FILE__, __LINE__);

  /* Writing text */
  f_puts("STM32 SD Card I/O Example via SPI\n", &fil);
  f_puts("Save the world!!!", &fil);

  /* Close file */
  if (f_close(&fil) != FR_OK)
    none = 1;
//    _Error_Handler(__FILE__, __LINE__);

  /* Open file to read */
  if (f_open(&fil, "first.txt", FA_READ) != FR_OK)
    none = 1;
//    _Error_Handler(__FILE__, __LINE__);

  while (f_gets(buffer, sizeof(buffer), &fil)) {
    //printf("%s", buffer);
  }

  /* Close file */
  if (f_close(&fil) != FR_OK)
    none = 1;
//    _Error_Handler(__FILE__, __LINE__);

  /* Unmount SDCARD */
  if (f_mount(NULL, "", 1) != FR_OK)
    none = 1;
//    _Error_Handler(__FILE__, __LINE__);
}
extern I2C_HandleTypeDef hi2c2;
dacChannelConfig output1;

osThreadId_t task_Input_Handle;
const osThreadAttr_t taskInput_attributes = { .name = "Input Thread", .stack_size = 512 * 4, .priority = (osPriority_t) osPriorityNormal, };

void Input_Task(void *argument) {
  button_obj_t water1;
  button_obj_t water2;
  buttonObjCreate(&water1, WATER1, 500);
  buttonObjCreate(&water2, WATER2, 500);

  output1.channelVref = 0;
  output1.channel_Gain = 0;
  output1.channel_Val[0] = 4000;
  output1.channel_Val[1] = 3000;
  output1.channel_Val[2] = 2000;
  output1.channel_Val[3] = 1000;
  MCP4728_Init(&hi2c2, output1);

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
  tm1637Init();
  tm1637SetBrightness(7);
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////
  TM1640_brightness(7);
  TM1640_clearDisplay();
  TM1640_displayOnOff(1);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _USE_CLI
  cliAdd("input", cliinput);
#endif
  while (1) {
    MCP4728_Write_AllChannels_Diff(&hi2c2, output1);
    tm1637LED();
    btn = tm1637ReadKey();
    button1 = buttonGetPressed(WATER1);
    button2 = buttonGetPressed(WATER2);
    TM1640_display_byte(0, 0x3f);
    TM1640_display_byte(1, 0x06);
    TM1640_display_byte(2, 0x5b);

    osDelay(100);

  }
}

void INPUT_Create_Thread(void) {
  task_Input_Handle = osThreadNew(Input_Task, NULL, &taskInput_attributes);
}

#ifdef _USE_CLI
void cliinput(uint8_t argc, const char **argv) {
  bool ret = false;

  if (argc == 1 && cliIsStr(argv[0], "show")) {
    while (cliKeepLoop()) {
      for (int i = 0; i < INPUT_MAX_CH; i++) {

        switch (i) {
//          case DOOR_CHK: {
//            cliPrintf("DOOR_CHK : %d, ", buttonGetPressed(i));
//            break;
//          }

        }
      }
      cliPrintf("\n");

      osDelay(100);
    }

    ret = true;
  }

  if (ret != true) {
    cliPrintf("input show\n");
  }
}
#endif

