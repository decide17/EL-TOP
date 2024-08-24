/*
 * com_gpio.h
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#ifndef INC_COM_GPIO_H_
#define INC_COM_GPIO_H_

#include "main.h"
#include <stdbool.h>

#define INPUT_MAX_CH 7
#define OUTPUT_MAX_CH 9

typedef struct {
  uint8_t ch;
  uint8_t state;
  uint32_t repeat_time;
  uint32_t pre_time;
} button_obj_t;

typedef enum {
  A_MO_CON,
  A_LL_HH_CON,
  A_HT_CON,
  A_PT_CON,
  B_MO_CON,
  B_LL_HH_CON,
  B_HT_CON,
  B_PT_CON,
  BUZZER,
} out_enum_t;

typedef enum {
  WATER1,
  WATER2,
  SDCARD_DETECT,
  MAX31865_DRDY1,
  MAX31865_DRDY2,
  MAX31865_DRDY3,
  MAX31865_DRDY4,
} in_enum_t;

typedef enum enOnOff {
  OFF,
  ON
} ONOFF_t;

bool gpio_Init(void);
bool buttonGetPressed(uint8_t ch);

void buttonObjCreate(button_obj_t *p_obj, uint8_t ch, uint32_t repeat_time);
bool buttonObjGetClicked(button_obj_t *p_obj, uint32_t pressed_time);

void gpio_on(out_enum_t ch);
void gpio_off(out_enum_t ch);
void gpio_toggle(out_enum_t ch);
uint32_t Tower_Number_Input(void);
uint32_t Slot_Number_Input(void);

#endif /* INC_COM_GPIO_H_ */
