/*
 * Task_Input.h
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_
#include "com_gpio.h"
#include "cmsis_os.h"

#define _USE_CONTROL

extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;

void CONTROL_Create_Thread(void);
#endif /* INC_INPUT_H_ */
