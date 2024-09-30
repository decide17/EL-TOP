/*
 * tm1639.c
 *
 *  Created on: Aug 8, 2024
 *      Author: USER
 */

#include "tm1639.h"
#include "cmsis_os.h"

#define CLK_PORT GPIOB
#define DIO_PORT GPIOB
#define CLK_PIN GPIO_PIN_8
#define DIO_PIN GPIO_PIN_9

#define STB_PORT GPIOC
#define STB_PIN GPIO_PIN_3

#define SegmentNo 3

void initTM1639(Tm1639_t *tm1639) {  //uint8_t dataPin1, uint8_t clockPin1, uint8_t strobePin1) {
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(tm1639->dio_gpio, tm1639->dio_pin, GPIO_PIN_SET);
  osDelay(1);

///////////////////////////////////////
  sendCommand(tm1639, 0x44);
  sendCommand(tm1639, 0b10000111);  // 펄스 폭을 14/16으로 설정
////////////////////////////////////////
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_RESET);
  send(tm1639, 0x00);
  for (int i = 0; i < 16; i++) {
    send(tm1639, 0x00);
  }
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
}

void sendCommand(Tm1639_t *tm1639, uint8_t cmd) {
//  digitalWrite(strobePin, LOW);
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_RESET);
//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_RESET);
  send(tm1639, cmd);
//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
//  digitalWrite(strobePin, HIGH);
}

void send(Tm1639_t *tm1639, uint8_t data) {
  for (int i = 0; i < 8; i++) {
//    digitalWrite(clockPin, LOW);
//    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_RESET);
//    digitalWrite(dataPin, data & 1 ? HIGH : LOW);
//    HAL_GPIO_WritePin(DIO_PORT, DIO_PIN, data & 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(tm1639->dio_gpio, tm1639->dio_pin, data & 1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    data >>= 1;
//    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_SET);
    osDelay(1);
  }
}

//void sendData(uint8_t address, uint8_t data) {
////  digitalWrite(strobePin, LOW);
//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_RESET);
//  send(0x00 | address);
//  send(data);
//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_SET);
////  digitalWrite(strobePin, HIGH);
//}

//void printTM1639(uint8_t a1, uint8_t b1, uint8_t a2, uint8_t b2, uint8_t a3, uint8_t b3, uint8_t a4, uint8_t b4, uint8_t a5, uint8_t b5, uint8_t a6, uint8_t b6,
//                 uint8_t a7, uint8_t b7, uint8_t a8, uint8_t b8) {
//  uint8_t info[] = { 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 };  //Âñïîìîãàòåëüíûé ìàññèâ äëÿ âûâîäà äàííûõ íà äèñïëåè
//  uint8_t data[] = { 0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110, 0b00000000 };  //Ñîñòîÿíèå ñåãìåíòîâ äëÿ èíäèêàöèè öèôð îò 0 äî 9
//
//  for (int j = 0; j < 8; j++) {  //Îáíóëÿåì âñïîìîãàòåëüíûé ìàññèâ äëÿ âûâîäà äàííûõ
//    info[j] = 0b00000000;
//  }
//
//  if (a1 > 10)
//    a1 = 10;
//  if (a2 > 10)
//    a2 = 10;
//  if (a3 > 10)
//    a3 = 10;
//  if (a4 > 10)
//    a4 = 10;
//  if (a5 > 10)
//    a5 = 10;
//  if (a6 > 10)
//    a6 = 10;
//  if (a7 > 10)
//    a7 = 10;
//  if (a8 > 10)
//    a8 = 10;
//
//  for (int i = 0; i < 8; i++) {
//    for (int bit = 0; bit < 8; bit++) {
//      info[i] = (info[i] & ~(1 << bit)) | (((data[a1 + bit] >> (7 - i)) & 1) << bit);
//    }
//    //    info[i] = (info[i] & ~(1 << 0)) | ((data[a1] >> (7 - i)) & 1) << 0;
////    info[i] = (info[i] & ~(1 << 1)) | ((data[a1] >> (7 - i)) & 1) << 1;
////    info[i] = (info[i] & ~(1 << 2)) | ((data[a1] >> (7 - i)) & 1) << 2;
////    info[i] = (info[i] & ~(1 << 3)) | ((data[a1] >> (7 - i)) & 1) << 3;
////    info[i] = (info[i] & ~(1 << 4)) | ((data[a1] >> (7 - i)) & 1) << 4;
////    info[i] = (info[i] & ~(1 << 5)) | ((data[a1] >> (7 - i)) & 1) << 5;
////    info[i] = (info[i] & ~(1 << 6)) | ((data[a1] >> (7 - i)) & 1) << 6;
////    info[i] = (info[i] & ~(1 << 7)) | ((data[a1] >> (7 - i)) & 1) << 7;
////    bitWrite(info[i], 0, bitRead(data[a1], 7 - i));
////    bitWrite(info[i], 1, bitRead(data[a2], 7 - i));
////    bitWrite(info[i], 2, bitRead(data[a3], 7 - i));
////    bitWrite(info[i], 3, bitRead(data[a4], 7 - i));
////    bitWrite(info[i], 4, bitRead(data[a5], 7 - i));
////    bitWrite(info[i], 5, bitRead(data[a6], 7 - i));
////    bitWrite(info[i], 6, bitRead(data[a7], 7 - i));
////    bitWrite(info[i], 7, bitRead(data[a8], 7 - i));
//  }
//
//  if (b1 != 0)
//    info[7] |= (1 << 0);
//  if (b2 != 0)
//    info[7] |= (1 << 1);
//  if (b3 != 0)
//    info[7] |= (1 << 2);
//  if (b4 != 0)
//    info[7] |= (1 << 3);
//  if (b5 != 0)
//    info[7] |= (1 << 4);
//  if (b6 != 0)
//    info[7] |= (1 << 5);
//  if (b7 != 0)
//    info[7] |= (1 << 6);
//  if (b8 != 0)
//    info[7] |= (1 << 7);
//
////  if (b1 != 0)
////    bitWrite(info[7], 0, 1);
////  if (b2 != 0)
////    bitWrite(info[7], 1, 1);
////  if (b3 != 0)
////    bitWrite(info[7], 2, 1);
////  if (b4 != 0)
////    bitWrite(info[7], 3, 1);
////  if (b5 != 0)
////    bitWrite(info[7], 4, 1);
////  if (b6 != 0)
////    bitWrite(info[7], 5, 1);
////  if (b7 != 0)
////    bitWrite(info[7], 6, 1);
////  if (b8 != 0)
////    bitWrite(info[7], 7, 1);
//
//  for (int i = 0; i < 8; i++) {
//    int pos = 0b11000000;
//    int data;
//    int data2;
////    for (int a = 0; a < 4; a++) {
//////      bitWrite(data, a, bitRead(info[a], i));
////    }
////    for (int a = 4; a < 8; a++) {
//////      bitWrite(data2, a - 4, bitRead(info[a], i));
////    }
//    for (int a = 0; a < 4; a++) {
//      data = (data & ~(1 << a)) | (((info[a] >> i) & 1) << a);
//    }
//    for (int a = 4; a < 8; a++) {
//      data2 = (data2 & ~(1 << (a - 4))) | (((info[a] >> i) & 1) << (a - 4));
//    }
//    sendData((pos + i * 2), data);
//    sendData((pos + i * 2 + 1), data2);
//  }
//  sendCommand(0x40);
//  sendCommand(0b10001010);
//}

void lightTM1639(Tm1639_t *tm1639, uint8_t light1) {
  uint8_t light[] = { 0b10001000, 0b10001001, 0b10001010, 0b10001011, 0b10001100, 0b10001101, 0b10001110, 0b10001111 };
  if (light1 == 0)
    light1 = 1;
  if (light1 > 8)
    light1 = 8;
  sendCommand(tm1639, light[light1 - 1]);
}

//void controlTM1639(bool a) {
//  if (a == 1)
//    sendCommand(0b10001011);
//  if (a == 0)
//    sendCommand(0b10000000);
//}

//uint8_t buttonstatusTM1639(bool b) {
//HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_RESET);
////  digitalWrite(strobePin, LOW);
//  send(0x42);  //Êîìàíäà íà çàïðîñ ñîñòîÿíèÿ êíîïîê
//  pinMode(dataPin, INPUT);
//
//  uint8_t buttonstatus1 = 0b00000000;
//  uint8_t buttonstatus2 = 0b00000000;
//
//  for (int i = 0; i < 16; i++) {  //Ïåðåáèðàåì 2 áàéòà
//    digitalWrite(clockPin, LOW);
//    if (i < 8)
//      bitWrite(buttonstatus1, i, digitalRead(dataPin));  //Ñ÷èòûâàåì ïåðâûé áàéò
//    else
//      bitWrite(buttonstatus2, i - 8, digitalRead(dataPin));  //Ñ÷èòûâàåì âòîðîé áàéò
//    digitalWrite(clockPin, HIGH);
//  }
//
//  pinMode(dataPin, OUTPUT);
//  digitalWrite(strobePin, HIGH);
//HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_SET);
//
//  if (b == 0)
//    return buttonstatus1;
//  if (b == 1)
//    return buttonstatus2;
//}

#include <stdio.h>
#include <string.h>

//#define STB_PIN 10
//#define CLK_PIN 11
//#define DIO_PIN 12

//typedef unsigned char u8;

//uint8_t code_table[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };

//void pinMode(int pin, int mode);
//void digitalWrite(int pin, int value);
void delay_us(int us);

void send_byte(Tm1639_t *tm1639, uint8_t data);
//void Addr_Static_Mode(uint8_t *addr, uint8_t *data, uint8_t len);
//void Addr_Inc_Mode(Tm1639_t *tm1639, uint8_t *data, uint8_t light_level);
//void Initial_Tm1639(int state);
//int Display(uint8_t which, uint8_t number);

//void Initial_Tm1639(int state) {
//  uint8_t code_1[16] = { 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x88, 0x83, 0xc6, 0xa1, 0x86, 0x8e };
//  uint8_t code_2[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };
//
//  if (state)
//    memcpy(code_table, code_1, 16);
//  else
//    memcpy(code_table, code_2, 16);
//
//  for (int i = 0; i < 16; i++)
//    printf("%x ", code_table[i]);
//}

//int Display(uint8_t which, uint8_t number) {
//  uint8_t p = (which - 1) * 2;
//  uint8_t Base_Addr[2] = { 0xc0 | p, 0xc0 | (p + 1) };
//  uint8_t Ready2Write[2] = { code_table[number] & 0x0f, (code_table[number] & 0xf0) >> 4 };
//
//  if (number > 9 || which > 8 || number < 0 || which < 1)
//    return 0;
//
//  Addr_Static_Mode(Base_Addr, Ready2Write, 2);
//  return 1;
//}

//void Addr_Static_Mode(u8 *addr, u8 *data, u8 len) {
//  u8 i;
//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_SET);
//  HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);
//  delay_us(1);
//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_SET);
//  send_byte(0x44);
//  for (i = 0; i < len; i++) {
//    HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_SET);
//    send_byte(*(addr + i));
//    send_byte(*(data + i));
//  }
//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_SET);
//  send_byte(0x8f);
//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_SET);
//}

void Full_Reset(Tm1639_t *tm1639) {

  for (uint8_t i = 0; i < 16; i++) {
    tm1639->data.BYTE_FIELD[i] = 0;
  }
  HAL_GPIO_WritePin(tm1639->dio_gpio, tm1639->dio_pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, 0x40);

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, 0xc0);
  for (uint8_t i = 0; i < 16; i++) {
    send_byte(tm1639, *(tm1639->data.BYTE_FIELD + i));
  }
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
}

void FND_Reset(Tm1639_t *tm1639) {  //uint8_t dataPin1, uint8_t clockPin1, uint8_t strobePin1) {
///////////////////////////////////////
  uint8_t _data[16] = { 0, 0x0c, 0, 0x0c, 0, 0x0c, 0, 0x0c, 0, 0x0c, 0, 0x0c, 0, 0x0c, 0, 0x0c, };

  for (uint8_t i = 0; i < 16; i++) {
    tm1639->data.BYTE_FIELD[i] = _data[i] & tm1639->data.BYTE_FIELD[i];
  }
  HAL_GPIO_WritePin(tm1639->dio_gpio, tm1639->dio_pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, 0x40);

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, 0xc0);
  for (uint8_t i = 0; i < 16; i++) {
    send_byte(tm1639, *(tm1639->data.BYTE_FIELD + i));
  }
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);

}

void Addr_Inc_Mode_full(Tm1639_t *tm1639, uint8_t light_level) {
  HAL_GPIO_WritePin(tm1639->dio_gpio, tm1639->dio_pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, 0x40);

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, 0xc0);
  for (uint8_t i = 0; i < 16; i++) {
    send_byte(tm1639, 0xff);
  }

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, light_level);
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
}

void Addr_Inc_Mode(Tm1639_t *tm1639, uint8_t light_level) {
  HAL_GPIO_WritePin(tm1639->dio_gpio, tm1639->dio_pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_SET);
  delay_us(1);
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, 0x40);

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, 0xc0);
  for (uint8_t i = 0; i < 16; i++) {
    send_byte(tm1639, *(tm1639->data.BYTE_FIELD + i));
  }

  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
  send_byte(tm1639, light_level);
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
}

void send_byte(Tm1639_t *tm1639, uint8_t data) {
  uint8_t i;
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_RESET);
  for (i = 0; i < 8; i++) {
    HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_RESET);
    if ((data >> i) & 0x01)

//      digitalWrite(DIO_PIN, 1);
      HAL_GPIO_WritePin(tm1639->dio_gpio, tm1639->dio_pin, GPIO_PIN_SET);
    else
//      digitalWrite(DIO_PIN, 0);
      HAL_GPIO_WritePin(tm1639->dio_gpio, tm1639->dio_pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_SET);
    // CLK HIGH/LOW toggle logic should be here
  }
  delay_us(1);
}
//uint8_t inputbuf1[16] = { 0 };
uint8_t buttonstatusTM1639(Tm1639_t *tm1639) {
  uint8_t retval1 = 0;
  uint8_t retval2 = 0;
  uint8_t buttonstatus = 0;
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(tm1639->dio_gpio, tm1639->dio_pin, GPIO_PIN_RESET);

//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_RESET);
//  HAL_GPIO_WritePin(DIO_PORT, DIO_PIN, GPIO_PIN_RESET);
  send(tm1639, 0x42);
//  pinMode(dataPin, INPUT);
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };
  GPIO_InitStruct.Pin = tm1639->dio_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(tm1639->dio_gpio, &GPIO_InitStruct);
//  osDelay(1);
  delay_us(10);

  for (uint8_t bit = 0; bit < 16; bit++) {
//    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_SET);
    if (bit < 8) {
      retval1 <<= 1;
      if (HAL_GPIO_ReadPin(tm1639->dio_gpio, tm1639->dio_pin) == 1) {
        retval1 |= 0x01;
      }
    } else {
      retval2 <<= 1;
      if (HAL_GPIO_ReadPin(tm1639->dio_gpio, tm1639->dio_pin) == 1) {
        retval2 |= 0x01;
      }
    }
//    if (HAL_GPIO_ReadPin(DIO_PORT, DIO_PIN) == 1) {
//      inputbuf1[bit] = 0x01;
//    } else
//      inputbuf1[bit] = 0;
//    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(tm1639->clk_gpio, tm1639->clk_pin, GPIO_PIN_RESET);
    delay_us(1);

  }

//  for (int i = 0; i < 16; i++) {  //Ïåðåáèðàåì 2 áàéòà
//    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
//    if (i < 8)
//      bitWrite(buttonstatus1, i, digitalRead(dataPin));  //Ñ÷èòûâàåì ïåðâûé áàéò
//    else
//      bitWrite(buttonstatus2, i - 8, digitalRead(dataPin));  //Ñ÷èòûâàåì âòîðîé áàéò
//    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);
//  }

  GPIO_InitStruct.Pin = tm1639->dio_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(tm1639->dio_gpio, &GPIO_InitStruct);

//  pinMode(dataPin, OUTPUT);
//  HAL_GPIO_WritePin(STB_PORT, STB_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(tm1639->stb_gpio, tm1639->stb_pin, GPIO_PIN_SET);
//  digitalWrite(strobePin, HIGH);

//  if (b == 0)
//    return buttonstatus1;
//  else
  if (retval1 & 0x10)
    buttonstatus = 0x01;
  if (retval2 & 0x80)
    buttonstatus |= 0x02;
  return buttonstatus;
}

void delay_us(int us) {
  for (; us > 0; us--)
    ;
}

uint8_t _tm1639Convert(char l) {
  switch (l) {
    case '0':
      return 0x3f;
    case '1':
      return 0x06;
    case '2':
      return 0x5b;
    case '3':
      return 0x4f;
    case '4':
      return 0x66;
    case '5':
      return 0x6d;
    case '6':
      return 0x7d;
    case '7':
      return 0x07;
    case '8':
      return 0x7f;
    case '9':
      return 0x6f;
    case 'A':
      return 0x77;
    case 'a':
      return 0x77;
    case 'B':
      return 0x7C;
    case 'b':
      return 0x7C;
    case 'C':
      return 0x39;
    case 'c':
      return 0x58;
    case 'D':
      return 0x5E;
    case 'd':
      return 0x5E;
    case 'E':
      return 0x79;
    case 'e':
      return 0x79;
    case 'F':
      return 0x71;
    case 'f':
      return 0x71;
    case 'G':
      return 0x3d;
    case 'g':
      return 0x3d;
    case 'H':
      return 0x76;
    case 'h':
      return 0x74;
    case 'I':
      return 0x30;
    case 'i':
      return 0x30;
    case 'J':
      return 0x1E;
    case 'j':
      return 0x1E;
    case 'L':
      return 0x38;
    case 'l':
      return 0x38;
    case 'M':
      return 0x55;
    case 'm':
      return 0x55;
    case 'N':
      return 0x37;
    case 'n':
      return 0x54;
    case 'O':
      return 0x3F;
    case 'o':
      return 0x5C;
    case 'P':
      return 0x73;
    case 'p':
      return 0x73;
    case 'Q':
      return 0x67;
    case 'q':
      return 0x67;
    case 'R':
      return 0x50;
    case 'r':
      return 0x50;
    case 'S':
      return 0x6D;
    case 's':
      return 0x6D;
    case 'T':
      return 0x78;
    case 't':
      return 0x78;
    case 'U':
      return 0x3E;
    case 'u':
      return 0x1C;
    case 'W':
      return 0x6a;
    case 'w':
      return 0x6a;
    case 'Y':
      return 0x6E;
    case 'y':
      return 0x6E;
    case '-':
      return 0x40;
    default:
      return 0x00;
  }
}

//uint8_t bufff[16] = { 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };  // 1 0x06
#include <stdbool.h>
void processSegmentMaps(bool on, uint8_t *bufMap, uint8_t *segmentMap) {
  if (on) {
    bufMap[0] = (((segmentMap[0] & 0x01) << 3) | (bufMap[0] & 0xF7));
    bufMap[2] = (((segmentMap[0] & 0x02) << 2) | (bufMap[2] & 0xF7));
    bufMap[4] = (((segmentMap[0] & 0x04) << 1) | (bufMap[4] & 0xF7));
    bufMap[6] = (((segmentMap[0] & 0x08)) | (bufMap[6] & 0xF7));
    bufMap[8] = (((segmentMap[0] & 0x10) >> 1) | (bufMap[8] & 0xF7));
    bufMap[10] = (((segmentMap[0] & 0x20) >> 2) | (bufMap[10] & 0xF7));
    bufMap[12] = (((segmentMap[0] & 0x40) >> 3) | (bufMap[12] & 0xF7));
    bufMap[14] = (((segmentMap[0] & 0x80) >> 4) | (bufMap[14] & 0xF7));
    //  for (int i = 0; i < 8; i++) {
    //    int shift = 3 - i;  // 각 비트의 시프트 값을 계산
    //    bufMap[2 * i] |= (shift > 0 ? (segmentMap[3] & (1 << i)) << shift : (segmentMap[3] & (1 << i)) >> -shift) | (bufMap[2 * i] & 0xF7);
    //  }

    bufMap[1] = ((segmentMap[1] & 0x01) | (bufMap[1] & 0xFE));
    bufMap[3] = (((segmentMap[1] & 0x02) >> 1) | (bufMap[3] & 0xFE));
    bufMap[5] = (((segmentMap[1] & 0x04) >> 2) | (bufMap[5] & 0xFE));
    bufMap[7] = (((segmentMap[1] & 0x08) >> 3) | (bufMap[7] & 0xFE));
    bufMap[9] = (((segmentMap[1] & 0x10) >> 4) | (bufMap[9] & 0xFE));
    bufMap[11] = (((segmentMap[1] & 0x20) >> 5) | (bufMap[11] & 0xFE));
    bufMap[13] = (((segmentMap[1] & 0x40) >> 6) | (bufMap[13] & 0xFE));
    bufMap[15] = (((segmentMap[1] & 0x80) >> 7) | (bufMap[15] & 0xFE));

    //  for (int i = 0; i < 8; i++) {
    //    int shift = 4 - i;  // 각 비트의 시프트 값을 계산
    //    bufMap[2 * i] |= (shift > 0 ? (segmentMap[4] & (1 << i)) << shift : (segmentMap[4] & (1 << i)) >> -shift) | (bufMap[2 * i] & 0xEF);
    //  }

    bufMap[1] = (((segmentMap[2] & 0x01) << 1) | (bufMap[1] & 0xFD));
    bufMap[3] = (((segmentMap[2] & 0x02)) | (bufMap[3] & 0xFD));
    bufMap[5] = (((segmentMap[2] & 0x04) >> 1) | (bufMap[5] & 0xFD));
    bufMap[7] = (((segmentMap[2] & 0x08) >> 2) | (bufMap[7] & 0xFD));
    bufMap[9] = (((segmentMap[2] & 0x10) >> 3) | (bufMap[9] & 0xFD));
    bufMap[11] = (((segmentMap[2] & 0x20) >> 4) | (bufMap[11] & 0xFD));
    bufMap[13] = (((segmentMap[2] & 0x40) >> 5) | (bufMap[13] & 0xFD));
    bufMap[15] = (((segmentMap[2] & 0x80) >> 6) | (bufMap[15] & 0xFD));
    //  for (int i = 0; i < 8; i++) {
    //    int shift = 5 - i;  // 각 비트의 시프트 값을 계산
    //    bufMap[2 * i] |= (shift > 0 ? (segmentMap[5] & (1 << i)) << shift : (segmentMap[5] & (1 << i)) >> -shift) | (bufMap[2 * i] & 0xDF);
    //  }
  } else {
    bufMap[0] = ((segmentMap[0] & 0x01) | (bufMap[0] & 0xFE));
    bufMap[2] = (((segmentMap[0] & 0x02) >> 1) | (bufMap[2] & 0xFE));
    bufMap[4] = (((segmentMap[0] & 0x04) >> 2) | (bufMap[4] & 0xFE));
    bufMap[6] = (((segmentMap[0] & 0x08) >> 3) | (bufMap[6] & 0xFE));
    bufMap[8] = (((segmentMap[0] & 0x10) >> 4) | (bufMap[8] & 0xFE));
    bufMap[10] = (((segmentMap[0] & 0x20) >> 5) | (bufMap[10] & 0xFE));
    bufMap[12] = (((segmentMap[0] & 0x40) >> 6) | (bufMap[12] & 0xFE));
    bufMap[14] = (((segmentMap[0] & 0x80) >> 7) | (bufMap[14] & 0xFE));

//  for (int i = 0; i < 8; i++) {
//    bufMap[2 * i] |= ((segmentMap[0] >> i) & 0x01) | (bufMap[2 * i] & 0xFE);
//  }

//  for (int i = 0; i < 16; i++) {
//    printf("%x, ", bufMap[i]);
//  }
//  printf("\n");

    bufMap[0] = (((segmentMap[1] & 0x01) << 1) | (bufMap[0] & 0xFD));
    bufMap[2] = (((segmentMap[1] & 0x02)) | (bufMap[2] & 0xFD));
    bufMap[4] = (((segmentMap[1] & 0x04) >> 1) | (bufMap[4] & 0xFD));
    bufMap[6] = (((segmentMap[1] & 0x08) >> 2) | (bufMap[6] & 0xFD));
    bufMap[8] = (((segmentMap[1] & 0x10) >> 3) | (bufMap[8] & 0xFD));
    bufMap[10] = (((segmentMap[1] & 0x20) >> 4) | (bufMap[10] & 0xFD));
    bufMap[12] = (((segmentMap[1] & 0x40) >> 5) | (bufMap[12] & 0xFD));
    bufMap[14] = (((segmentMap[1] & 0x80) >> 6) | (bufMap[14] & 0xFD));
//  for (int i = 0; i < 8; i++) {
//    bufMap[2 * i] |= ((segmentMap[1] & (0x01 << i)) >> (i ? i - 1 : 0)) | (bufMap[2 * i] & 0xFD);
//  }

    bufMap[0] = (((segmentMap[2] & 0x01) << 2) | (bufMap[0] & 0xFB));
    bufMap[2] = (((segmentMap[2] & 0x02) << 1) | (bufMap[2] & 0xFB));
    bufMap[4] = (((segmentMap[2] & 0x04)) | (bufMap[4] & 0xFB));
    bufMap[6] = (((segmentMap[2] & 0x08) >> 1) | (bufMap[6] & 0xFB));
    bufMap[8] = (((segmentMap[2] & 0x10) >> 2) | (bufMap[8] & 0xFB));
    bufMap[10] = (((segmentMap[2] & 0x20) >> 3) | (bufMap[10] & 0xFB));
    bufMap[12] = (((segmentMap[2] & 0x40) >> 4) | (bufMap[12] & 0xFB));
    bufMap[14] = (((segmentMap[2] & 0x80) >> 5) | (bufMap[14] & 0xFB));
//  for (int i = 0; i < 8; i++) {
//    int shift = 2 - i;  // 각 비트의 시프트 값을 계산
//    if (shift > 0) {
//      bufMap[2 * i] |= ((segmentMap[2] & (1 << i)) << shift) | (bufMap[2 * i] & 0xFB);
//    } else {
//      bufMap[2 * i] |= ((segmentMap[2] & (1 << i)) >> (-shift)) | (bufMap[2 * i] & 0xFB);
//    }
//  }
  }
}

void tm1639Display_led(Tm1639_t *tm1639) {
  Addr_Inc_Mode(tm1639, /*tm1639->data, */0x8f);
}

void tm1639Display_str(Tm1639_t *tm1639, bool on, char c[]) {
  int mc = 0;

  uint8_t digitArr[SegmentNo];
  for (int jt = 0; (jt - mc) < SegmentNo; jt++) {
    if (c[jt + mc] == '.') {
      digitArr[jt - 1] |= 0x80;
      mc++;
    }
    if (c[jt + mc])
      digitArr[jt] = _tm1639Convert(c[jt + mc]);
    else
      digitArr[jt] = 0x00;
  };
  processSegmentMaps(on, tm1639->data.BYTE_FIELD, digitArr);

  Addr_Inc_Mode(tm1639, /*tm1639->data, */0x8f);
}

void tm1639Display_float(Tm1639_t *tm1639, bool on, float num) {
  int mc = 0;
  char c[20] = { 0 };

//  if (num >= 100) {
  sprintf(c, "%.2f", num);
//  } else if (num >= 10) {
//    sprintf(c, "%.2f", num);
//  } else if (num >= 0) {
//    sprintf(c, "%.2f", num);
//  } else if (num >= -10) {
//    sprintf(c, "%.2f", num);
//  } else {
//    sprintf(c, "%.2f", num);
//  }
  if (c[0] == '-' && c[1] == '0') {
    c[1] = c[2];  // 1번 인덱스에 '.'을 넣음
    c[2] = c[3];  // 2번 인덱스에 '4'를 넣음
    c[3] = c[4];  // 3번 인덱스에 '5'를 넣음
    c[4] = c[5];  // 4번 인덱스에 NULL 문자 삽입
  }

  uint8_t digitArr[SegmentNo];
  for (int jt = 0; (jt - mc) < SegmentNo; jt++) {
    if (c[jt + mc] == '.') {
      digitArr[jt - 1] |= 0x80;
      mc++;
    }
    if (c[jt + mc])
      digitArr[jt] = _tm1639Convert(c[jt + mc]);
    else
      digitArr[jt] = 0x00;
  };
  processSegmentMaps(on, tm1639->data.BYTE_FIELD, digitArr);

  Addr_Inc_Mode(tm1639, /*tm1639->data, */0x8f);
}

void tm1639Display_num(Tm1639_t *tm1639, bool on, int num) {
  int mc = 0;
  char c[20];

  if (num >= 100) {
    sprintf(c, "%d", num);
  } else if (num >= 10) {
    sprintf(c, " %d", num);
  } else if (num >= 0) {
    sprintf(c, "  %d", num);
  } else if (num > -10) {
    sprintf(c, " %d", num);
  } else {
    sprintf(c, "%d", num);
  }

  uint8_t digitArr[SegmentNo];
  for (int jt = 0; (jt - mc) < SegmentNo; jt++) {
    if (c[jt + mc] == '.') {
      digitArr[jt - 1] |= 0x80;
      mc++;
    }
    if (c[jt + mc])
      digitArr[jt] = _tm1639Convert(c[jt + mc]);
    else
      digitArr[jt] = 0x00;
  };
  processSegmentMaps(on, tm1639->data.BYTE_FIELD, digitArr);

  Addr_Inc_Mode(tm1639, /*tm1639->data, */0x8f);
}

