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

void INPUT_Create_Thread(void);

extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
#endif /* INC_INPUT_H_ */
