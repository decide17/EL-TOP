/*
 * com_adc.c
 *
 *  Created on: Aug 30, 2023
 *      Author: sjpark
 */

#include "com_adc.h"

uint16_t adc_start(void) {
  HAL_ADC_Start(&hadc1);                              // ADC를 Enable시킨 후 conversion을 시작
  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);   // conversion이 완료되기를 기다림

  return HAL_ADC_GetValue(&hadc1);      // ADC 데이터를 읽어 변수에 저장
}
