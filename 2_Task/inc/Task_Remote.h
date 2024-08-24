/*
 * Task_Remote.h
 *
 *  Created on: Aug 24, 2024
 *      Author: USER
 */

#ifndef INC_TASK_REMOTE_H_
#define INC_TASK_REMOTE_H_
#include "com_gpio.h"
#include "cmsis_os.h"

#define _USE_REMOTE

void REMOTE_Create_Thread(void);
#endif /* INC_TASK_REMOTE_H_ */
