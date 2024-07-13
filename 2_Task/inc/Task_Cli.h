/*
 * Task_Cli.h
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#ifndef INCLUDE_CLI_H_
#define INCLUDE_CLI_H_

#include "string.h"
#include "stdlib.h"

#include "com_uart.h"
#include "cmsis_os.h"

#define _USE_CLI

void CLI_Create_Thread(void);

bool cliInit(void);
bool cliMain(void);
bool cliAdd(const char *cmd_str, void (*cmd_func)(uint8_t argc, const char **argv));
void cliPrintf(const char *fmt, ...);
bool cliIsStr(const char *p_arg, const char *p_str);
bool cliKeepLoop(void);

int32_t cliGetData(const char *p_arg, const int base);

#endif /* INCLUDE_CLI_H_ */
