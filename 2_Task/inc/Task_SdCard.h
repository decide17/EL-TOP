/*
 * Task_SdCard.h
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#ifndef INCLUDE_SDCARD_H_
#define INCLUDE_SDCARD_H_

#include "com_uart.h"
#include "cmsis_os.h"

#define _USE_SDCARD

void SDCard_Create_Thread(void);

#endif /* INCLUDE_CLI_H_ */
