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

#define IO4_AB_MO_FND 0
#define IO4_B_LEVEL_FND 1
#define IO3_AB_BR_FND 0
#define IO3_A_LEVEL_FND 1

void REMOTE_Create_Thread(void);
#endif /* INC_TASK_REMOTE_H_ */
