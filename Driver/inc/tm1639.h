/*
 * tm1639.h
 *
 *  Created on: Aug 8, 2024
 *      Author: USER
 */

#ifndef INC_TM1639_H_
#define INC_TM1639_H_

#include "main.h"
#include <stdbool.h>

//uint8_t bufff[16] = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, }; 1 fnd
//uint8_t bufff[16] = { 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, }; 2 fnd
//uint8_t bufff[16] = { 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, }; 3 fnd
//uint8_t bufff[16] = { 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, }; 4 fnd
//uint8_t bufff[16] = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, }; 5 fnd
//uint8_t bufff[16] = { 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, }; 6 fnd
//uint8_t bufff[16] = { 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 4, }; 하위 녹색
//uint8_t bufff[16] = { 0, 4, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, }; 하위 적색
//uint8_t bufff[16] = { 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 8, }; 상위 녹색
//uint8_t bufff[16] = { 0, 8, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, }; 상위 적색
typedef union {
  uint8_t BYTE_FIELD[16];
  struct {
    uint8_t fnd1 :8;
    uint8_t fnd2 :2;
    uint8_t LED5_RED :1;
    uint8_t LED1_RED :1;
    uint8_t led1 :4;
    uint8_t fnd3 :8;
    uint8_t fnd4 :2;
    uint8_t LED5_GREEN :1;
    uint8_t LED1_GREEN :1;
    uint8_t led2 :4;
    uint8_t fnd5 :8;
    uint8_t fnd6 :2;
    uint8_t LED6_RED :1;
    uint8_t LED2_RED :1;
    uint8_t led3 :4;
    uint8_t fnd7 :8;
    uint8_t fnd8 :2;
    uint8_t LED6_GREEN :1;
    uint8_t LED2_GREEN :1;
    uint8_t led4 :4;
    uint8_t fnd9 :8;
    uint8_t fnd10 :2;
    uint8_t LED7_RED :1;
    uint8_t LED3_RED :1;
    uint8_t led5 :4;
    uint8_t fnd11 :8;
    uint8_t fnd12 :2;
    uint8_t LED7_GREEN :1;
    uint8_t LED3_GREEN :1;
    uint8_t led6 :4;
    uint8_t fnd13 :8;
    uint8_t fnd14 :2;
    uint8_t LED8_RED :1;
    uint8_t LED4_RED :1;
    uint8_t led7 :4;
    uint8_t fnd15 :8;
    uint8_t fnd16 :2;
    uint8_t LED8_GREEN :1;
    uint8_t LED4_GREEN :1;
    uint8_t led8 :4;
  } NAME_FIELD;
} _DATA_T;

typedef struct {
  GPIO_TypeDef *dio_gpio;
  uint16_t dio_pin;
  GPIO_TypeDef *clk_gpio;
  uint16_t clk_pin;
  GPIO_TypeDef *stb_gpio;
  uint16_t stb_pin;
  uint8_t button;
  _DATA_T data;
} Tm1639_t;

void initTM1639(Tm1639_t *tm1639);  //, uint8_t dataPin1, uint8_t clockPin1, uint8_t strobePin1);
//void sendCommand(uint8_t cmd);
void sendCommand(Tm1639_t *tm1639, uint8_t cmd);
//void send(uint8_t data);
void send(Tm1639_t *tm1639, uint8_t data);
void sendData(uint8_t address, uint8_t data);
void printTM1639(uint8_t a1, uint8_t b1, uint8_t a2, uint8_t b2, uint8_t a3, uint8_t b3, uint8_t a4, uint8_t b4, uint8_t a5, uint8_t b5, uint8_t a6, uint8_t b6,
                 uint8_t a7, uint8_t b7, uint8_t a8, uint8_t b8);
void lightTM1639(Tm1639_t *tm1639, uint8_t light1);
void controlTM1639(bool a);
uint8_t buttonstatusTM1639(Tm1639_t *tm1639);
void tm1639Display_str(Tm1639_t *tm1639, bool on, char c[]);
void tm1639Display_num(Tm1639_t *tm1639, bool on, int num);
void tm1639Display_float(Tm1639_t *tm1639, bool on, float num);
void Addr_Inc_Mode(Tm1639_t *tm1639, uint8_t light_level);
void FND_Reset(Tm1639_t *tm1639);
void Full_Reset(Tm1639_t *tm1639);
void tm1639Display_led(Tm1639_t *tm1639);
void Addr_Inc_Mode_full(Tm1639_t *tm1639, uint8_t light_level);
#endif /* INC_TM1639_H_ */
