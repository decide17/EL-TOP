/*
 * com_uart.h
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#ifndef INC_COM_UART_H_
#define INC_COM_UART_H_

#include "usart.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define _DEF_CH1 0
#define _DEF_CH2 1

#define UART_MAX_CH 1

bool uartInit(uint8_t ch);
uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t length);
uint32_t uartAvailable(uint8_t ch);
uint8_t uartRead(uint8_t ch);
uint32_t uartPrintf(uint8_t ch, const char *fmt, ...);
uint32_t uartVPrintf(uint8_t ch, const char *fmt, va_list arg);

#endif /* INC_UART_H_ */
