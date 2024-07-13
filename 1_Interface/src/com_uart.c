/*
 * uart.c
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#include "com_uart.h"

#define UART_Q_BUF_MAX 256

static uint16_t q_in[UART_MAX_CH];
static uint16_t q_out[UART_MAX_CH];
static uint8_t q_buf[UART_MAX_CH][UART_Q_BUF_MAX];

bool uartInit(uint8_t ch) {
  HAL_StatusTypeDef hal_ret;

  switch (ch) {
    case _DEF_CH1:
      hal_ret = HAL_UART_Receive_DMA(&huart1, &q_buf[ch][0], UART_Q_BUF_MAX);
      break;
//    case _DEF_CH2:
//      hal_ret = HAL_UART_Receive_DMA(&huart2, &q_buf[ch][0], UART_Q_BUF_MAX);
//      break;
  }

  return hal_ret;
}

uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t length) {
  uint32_t ret = 0;
  HAL_StatusTypeDef hal_ret;

  switch (ch) {
    case _DEF_CH1:
      hal_ret = HAL_UART_Transmit(&huart1, p_data, length, 100);
      if (hal_ret == HAL_OK) {
        ret = length;
      }
      break;
//    case _DEF_CH2:
//      hal_ret = HAL_UART_Transmit(&huart2, p_data, length, 100);
//      if (hal_ret == HAL_OK) {
//        ret = length;
//      }
//      break;
  }

  return ret;
}

uint32_t uartAvailable(uint8_t ch) {
  uint32_t ret = 0;

  switch (ch) {
    case _DEF_CH1:
      q_in[ch] = (UART_Q_BUF_MAX - huart1.hdmarx->Instance->CNDTR) % UART_Q_BUF_MAX;
      ret = (UART_Q_BUF_MAX + q_in[ch] - q_out[ch]) % UART_Q_BUF_MAX;
      break;
//    case _DEF_CH2:
//      q_in[ch] = (UART_Q_BUF_MAX - huart2.hdmarx->Instance->CNDTR) % UART_Q_BUF_MAX;
//      ret = (UART_Q_BUF_MAX + q_in[ch] - q_out[ch]) % UART_Q_BUF_MAX;
//      break;
  }

  return ret;
}

uint8_t uartRead(uint8_t ch) {
  uint8_t ret = 0;

  switch (ch) {
    case _DEF_CH1:
      if (q_out[ch] != q_in[ch]) {
        ret = q_buf[ch][q_out[ch]];
        q_out[ch] = (q_out[ch] + 1) % UART_Q_BUF_MAX;
      }
      break;
    case _DEF_CH2:
      if (q_out[ch] != q_in[ch]) {
        ret = q_buf[ch][q_out[ch]];
        q_out[ch] = (q_out[ch] + 1) % UART_Q_BUF_MAX;
      }
      break;
  }

  return ret;
}

uint32_t uartPrintf(uint8_t ch, const char *fmt, ...) {
  uint32_t ret = 0;
  va_list arg;
  char print_buf[256];

  va_start(arg, fmt);

  int len;
  len = vsnprintf(print_buf, 256, fmt, arg);
  va_end(arg);

  if (len > 0) {
    ret = uartWrite(ch, (uint8_t*) print_buf, len);
  }

  return ret;
}

uint32_t uartVPrintf(uint8_t ch, const char *fmt, va_list arg) {
  uint32_t ret = 0;
  char print_buf[256];

  int len;
  len = vsnprintf(print_buf, 256, fmt, arg);

  if (len > 0) {
    ret = uartWrite(ch, (uint8_t*) print_buf, len);
  }

  return ret;
}
