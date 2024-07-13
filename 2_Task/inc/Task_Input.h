/*
 * Task_Input.h
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#ifndef INC_INPUT_H_
#define INC_INPUT_H_
#include "com_gpio.h"
#include "com_adc.h"
#include "cmsis_os.h"

#define _USE_INPUT

#define ADC_RD_MAX_CNT        20
#define Ring_Buf_Inc(x, y)      x = (x<(y-1)) ? (x+1) : 0
#define Ring_Buf_Dec(x, y)      x = (x>0) ? (x-1) : (y-1)

void INPUT_Create_Thread(void);
#endif /* INC_INPUT_H_ */
