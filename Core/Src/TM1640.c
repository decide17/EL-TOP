/*

 ******************************************************************************
 * @file 			( фаил ):   TM1640.c
 * @brief 		( описание ):
 ******************************************************************************
 * @attention 	( внимание ):	author: Golinskiy Konstantin	e-mail: golinskiy.konstantin@gmail.com
 ******************************************************************************

 */

/* Includes ----------------------------------------------------------*/

#include "TM1640.h"
#include "font.h"

uint8_t Cmd_DispCtrl = BRIGHTNESS_TM1640;		// контрастность дисплея от 0 -7

int32_t x = 0;
int32_t y = 0;

uint8_t frameBuffer[SIZE_MATRIX_COL];			// массив для буфера кадра на размер матрицы

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция задержки в микросекундах
 * @param	( параметры ):	кол-во микросекунд
 * @return  ( возвращает ):

 ******************************************************************************
 */
void Delay_us(uint32_t i) {

  for (; i > 0; i--)
    ;
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция для старта ( активации ) передачи данных на модуль
 * @param	( параметры ):
 * @return  ( возвращает ):

 ******************************************************************************
 */
void TM1640_Generate_START(void) {

  CLK(HIGH);
  DIN(HIGH);
  Delay_us( DELAY_TIME);
  DIN(LOW);
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция для конца отправки данных на модуль ( вызываеться по завершению )
 * @param	( параметры ):
 * @return  ( возвращает ):

 ******************************************************************************
 */
void TM1640_Generate_STOP(void) {

  CLK(LOW);
  Delay_us( DELAY_TIME);
  DIN(LOW);
  Delay_us( DELAY_TIME);
  CLK(HIGH);
  Delay_us( DELAY_TIME);
  DIN(HIGH);
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция отправки данных на модуль 1 байт
 * @param	( параметры ):	1 байт информации
 * @return  ( возвращает ):

 ******************************************************************************
 */
void TM1640_WriteData(uint8_t oneByte) {

  Delay_us( DELAY_TIME);

  for (uint8_t i = 0; i < 8; i++) {
    CLK(LOW);

//    if (oneByte & (1 << i)) {
    if ((oneByte >> i) & 0x01) {
      DIN(HIGH);
    } else {
      DIN(LOW);
    }

    Delay_us( DELAY_TIME);
    CLK(HIGH);
    Delay_us( DELAY_TIME);
  }

  CLK(LOW);
  Delay_us( DELAY_TIME);
  DIN(LOW);
  while ( read_DIN) {
  };
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция отправляет 1 байт в указаный сегмент
 ( 8 пикселей = 8 битам ( 1 байт ) ) где 1 там вкл пиксель и наоборот
 * @param	( параметры ):	1 пар - номер сигмента, 2 пар - 1 байт для отображения
 * @return  ( возвращает ):

 ******************************************************************************
 */
void TM1640_display_byte(uint8_t Seg_N, uint8_t DispData) {

  TM1640_Generate_START();
  TM1640_WriteData(0x44);			// 1:фиксированный адресс
  TM1640_Generate_STOP();

  TM1640_Generate_START();
  TM1640_WriteData(Seg_N | 0xc0);  // устанавливаем адресс старшие 2 бита должны быть 1(0xc0)+ номер сегмента
  TM1640_WriteData(DispData);		// отправляем "кодированные" данные для одного сегмента
  TM1640_Generate_STOP();

  TM1640_Generate_START();
  TM1640_WriteData(Cmd_DispCtrl);  // включаем дисплей с выставлленой яркостью;
  TM1640_Generate_STOP();
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция включения и выключения экрана с сохранением на нем выведеной информации
 * @param	( параметры ):	байт статуса экрана = 1- вкл, 0- выкл
 * @return  ( возвращает ):

 ******************************************************************************
 */
void TM1640_displayOnOff(uint8_t OnOff) {

  TM1640_Generate_START();
  TM1640_WriteData(0x44);		// 1:фиксированный адресс
  TM1640_Generate_STOP();

  TM1640_Generate_START();
// для включения или выключения дисплея с сохранением текущей контрасности нужно ставить третий бит в 1 вкл 0 выкл
  TM1640_WriteData(OnOff ? (Cmd_DispCtrl | 1 << 3) : (Cmd_DispCtrl & ~(1 << 3)));		// 0x80 выкл     0х88 вкл
  TM1640_Generate_STOP();
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция отчистки дисплея
 * @param	( параметры ):
 * @return  ( возвращает ):

 ******************************************************************************
 */
void TM1640_clearDisplay(void) {

  for (uint8_t i = 0; i < SIZE_MATRIX_COL; i++) {
    TM1640_display_byte(i, 0x00);
  }
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция установки яркости дисплея
 * @param	( параметры ):	яркость дисплея ( значения от 0  до 7 )
 * @return  ( возвращает ):

 ******************************************************************************
 */
void TM1640_brightness(uint8_t brightness) {

  Cmd_DispCtrl = 0x88 + brightness;
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция получения буффера для работы с ним
 * @param	( параметры ):	координата X и Y
 * @return  ( возвращает ):	указатель на буффер

 ******************************************************************************
 */
uint8_t* _getBufferPtr(int8_t x, int8_t y) {

  if ((y < 0) || (y >= SIZE_MATRIX_COL)) {
    return NULL;
  }
  if ((x < 0) || (x >= (8))) {
    return NULL;
  }

  uint8_t B = x >> 3;		//byte

  return frameBuffer + y + B;
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция заполнения буффера нужными пикселями
 * @param	( параметры ):	координаты X и Y и ( 0-выкл пиксель, 1-вкл пиксель )
 * @return  ( возвращает ):

 ******************************************************************************
 */
// меняя х у меняем положение текста вертикальное или горизонтальное
void setPixel(int8_t y, int8_t x, uint8_t enabled) {

  uint8_t *p = _getBufferPtr(x, y);
  if (!p) {
    return;
  }

  uint8_t b = 7 - (x & 7);		//bit

  if (enabled) {
    *p |= (1 << b);
  } else {
    *p &= ~(1 << b);
  }
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция из массива данных преобразует в пиксели и заполняет буффер
 * @param	( параметры ):	1 пар - ссылка на массив с данными, 2 пар и 3 пар - координаты X Y ,
 3 пар и 4 пар размер фрагмента ( например 8 на 8 пикселей )
 * @return  ( возвращает ):

 ******************************************************************************
 */
void drawSprite(const uint8_t *sprite, int x, int y, int width, int height) {

// The mask is used to get the column bit from the sprite row
  uint8_t mask = 0x80;

  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {

      setPixel(x + ix, y + iy, (uint8_t) (sprite[iy] & mask));

      // shift the mask by one pixel to the right
      mask = mask >> 1;
    }
    // reset column mask
    mask = 0x80;
  }
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция для преобразования полученного символа в его нарисованный вариант в массиве
 * @param	( параметры ):	1 пар - ссылка на текст, 2 пар и 3 пар координаты X Y
 * @return  ( возвращает ):

 ******************************************************************************
 */
void TM1640_drawString_buff(char *text, int x, int y) {

  uint16_t len = strlen(text);

  uint16_t count_len = 0;

  while (len--) {

    count_len++;
    uint8_t c;

    //---------------------------------------------------------------------
    // проверка на кириллицу UTF-8, если латиница то пропускаем if
    // Расширенные символы ASCII Win-1251 кириллица (код символа 128-255)
    // проверяем первый байт из двух ( так как UTF-8 ето два байта )
    // если он больше либо равен 0xC0 ( первый байт в кириллеце будет равен 0xD0 либо 0xD1 именно в алфавите )
    if ((uint8_t) *text >= 0xC0) {	// код 0xC0 соответствует символу кириллица 'A' по ASCII Win-1251

      // проверяем какой именно байт первый 0xD0 либо 0xD1---------------------------------------------
      switch ((uint8_t) *text) {
        case 0xD0: {
          // увеличиваем массив так как нам нужен второй байт
          text++;
          // проверяем второй байт там сам символ
          if ((uint8_t) *text >= 0x90 && (uint8_t) *text <= 0xBF) {
            c = (*text) + 0x30;
          }  // байт символов А...Я а...п  делаем здвиг на +48
          else if ((uint8_t) *text == 0x81) {
            c = 0xA8;
            break;
          }		// байт символа Ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
          else if ((uint8_t) *text == 0x84) {
            c = 0xAA;
            break;
          }		// байт символа Є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
          else if ((uint8_t) *text == 0x86) {
            c = 0xB2;
            break;
          }		// байт символа І ( если нужнф еще символы добавляем тут и в функции DrawChar() )
          else if ((uint8_t) *text == 0x87) {
            c = 0xAF;
            break;
          }		// байт символа Ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
          break;
        }
        case 0xD1: {
          // увеличиваем массив так как нам нужен второй байт
          text++;
          // проверяем второй байт там сам символ
          if ((uint8_t) *text >= 0x80 && (uint8_t) *text <= 0x8F) {
            c = (*text) + 0x70;
          }  // байт символов п...я	елаем здвиг на +112
          else if ((uint8_t) *text == 0x91) {
            c = 0xB8;
            break;
          }		// байт символа ё ( если нужнф еще символы добавляем тут и в функции DrawChar() )
          else if ((uint8_t) *text == 0x94) {
            c = 0xBA;
            break;
          }		// байт символа є ( если нужнф еще символы добавляем тут и в функции DrawChar() )
          else if ((uint8_t) *text == 0x96) {
            c = 0xB3;
            break;
          }		// байт символа і ( если нужнф еще символы добавляем тут и в функции DrawChar() )
          else if ((uint8_t) *text == 0x97) {
            c = 0xBF;
            break;
          }		// байт символа ї ( если нужнф еще символы добавляем тут и в функции DrawChar() )
          break;
        }
      }
      //------------------------------------------------------------------------------------------------
    } else {
      c = *text;
    }

    text++;
    //---------------------------------------------------------------------

    // номер по ASCII - позиция в массиве  = значение которое нужно отнять

    // символы латинские буквы цифры и спецсимволы начинаються с - 32 символ по ASCII а в массиве 0 эллемент
    // значит 32 - 0 = 32 значит от текущего значения отнимаем 32
    // и делаем это для всего латинского алфавита ( так как в массиве он идет по таблице ASCII )
    // то для всего диапазона ( от 32 до 127 включительно отнимаем 32 )
    // проверяем диапазон  для латинских символов и немного спец символов
    if (c > 31 && c < 128) {
      c = c - 32;
    }

    // символ кирилица А - 192 символ по ASCII а в массиве 96 эллемент
    // значит 192 - 96 = 96 значит от текущего значения отнимаем 96
    // и делаем это для всей килилицы алфавита ( так как в массиве он идет по таблице ASCII )
    // то для всего диапазона ( от 192 до 255 включительно отнимаем 96 )
    // проверяем диапазон  для латинских символов и немного спец символов
    else if (c > 191) {
      c = c - 96;
    }

    // дополнительные символы и знаки если нужны
    // добавляем в конец массива тут делаем проверку и расчеты
    // номер по ASCII - позиция в массиве  = значение которое нужно отнять
    else if (c == 168) {
      c = c - 8;
    }		    // 168 - 160 эллемент массива ( символ Ё ) = 8
    else if (c == 184) {
      c = c - 23;
    }		// 184 - 161 эллемент массива ( символ ё ) = 23

    else if (c == 170) {
      c = c - 8;
    }  		// Є 	// 170 символ по ASCII  - 162 эллемент
    else if (c == 175) {
      c = c - 12;
    }   		// Ї 	// 175 символ по ASCII  - 163 эллемент
    else if (c == 178) {
      c = c - 14;
    }  		// І 	// 178 символ по ASCII  - 164 эллемент
    else if (c == 179) {
      c = c - 14;
    } 		// і	// 179 символ по ASCII  - 165 эллемент
    else if (c == 186) {
      c = c - 20;
    } 		// є	// 186 символ по ASCII  - 166 эллемент
    else if (c == 191) {
      c = c - 24;
    } 		// ї	// 191 символ по ASCII  - 167 эллемент

    // stop if char is outside visible area
    if (x + count_len * 8 > ( SIZE_MATRIX_COL)) {
      return;
    }

    // only draw if char is visible
    if (8 + x + count_len * 8 > 0) {
      drawSprite(disp1ay[c], x + count_len * 8, y, 8, 8);
    }
  }
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция выводит на экран все что нвходиться в буффере
 * @param	( параметры ):
 * @return  ( возвращает ):

 ******************************************************************************
 */
void TM1640_display_Draw(void) {

  for (int16_t d = 0; d != SIZE_MATRIX_COL; d++) {

    uint8_t data = frameBuffer[d];

    TM1640_Generate_START();
    TM1640_WriteData(0x44);				// 1:фиксированный адресс
    TM1640_Generate_STOP();

    TM1640_Generate_START();
    TM1640_WriteData((15 - d) | 0xc0);
    TM1640_WriteData(data);				// отправляем "кодированные" данные для одного сегмента
    TM1640_Generate_STOP();

    TM1640_Generate_START();
    TM1640_WriteData(Cmd_DispCtrl);		// включаем дисплей с выставлленой яркостью;
    TM1640_Generate_STOP();
  }
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция очистка буффера но не чистит экран
 * @param	( параметры ):
 * @return  ( возвращает ):

 ******************************************************************************
 */
void clear_buff(void) {

  memset(frameBuffer, 0, SIZE_MATRIX_COL);
}
//----------------------------------------------------------------------------------

/*
 ******************************************************************************
 * @brief	 ( описание ):  функция подсчета длины строки с учетом кириллицы ( 2 байта на символ )
 * @param	( параметры ):
 * @return  ( возвращает ):

 ******************************************************************************
 */
uint16_t strlenUTF8(char *str) {
  uint16_t i = 0;
  while (*str != '\0') {
    if ((uint8_t) *str >= 0xC0) {  // код 0xC0 соответствует символу кириллица 'A' по ASCII Win-1251
      str++;
    }

    i++;
    str++;
  }
  return i;
}
//----------------------------------------------------------------------------------

/************************ (C) COPYRIGHT GKP *****END OF FILE****/
