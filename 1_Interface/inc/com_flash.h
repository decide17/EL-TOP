/*
 * com_flash.h
 *
 *  Created on: Jul 20, 2024
 *      Author: sjpark
 */

#ifndef INC_COM_FLASH_H_
#define INC_COM_FLASH_H_

#define ADDR_FLASH_PAGE_0     ((uint32_t)0x08000000) // Page 0, 2 Kbyte
#define ADDR_FLASH_PAGE_1     ((uint32_t)0x08000800) // Page 1, 2 Kbyte
#define ADDR_FLASH_PAGE_2     ((uint32_t)0x08001000) // Page 2, 2 Kbyte
#define ADDR_FLASH_PAGE_3     ((uint32_t)0x08001800) // Page 3, 2 Kbyte
#define ADDR_FLASH_PAGE_4     ((uint32_t)0x08002000) // Page 4, 2 Kbyte
#define ADDR_FLASH_PAGE_5     ((uint32_t)0x08002800) // Page 5, 2 Kbyte
#define ADDR_FLASH_PAGE_6     ((uint32_t)0x08003000) // Page 6, 2 Kbyte
#define ADDR_FLASH_PAGE_7     ((uint32_t)0x08003800) // Page 7, 2 Kbyte
#define ADDR_FLASH_PAGE_8     ((uint32_t)0x08004000) // Page 8, 2 Kbyte
#define ADDR_FLASH_PAGE_9     ((uint32_t)0x08004800) // Page 9, 2 Kbyte
#define ADDR_FLASH_PAGE_10    ((uint32_t)0x08005000) // Page 10, 2 Kbyte
#define ADDR_FLASH_PAGE_11    ((uint32_t)0x08005800) // Page 11, 2 Kbyte
#define ADDR_FLASH_PAGE_12    ((uint32_t)0x08006000) // Page 12, 2 Kbyte
#define ADDR_FLASH_PAGE_13    ((uint32_t)0x08006800) // Page 13, 2 Kbyte
#define ADDR_FLASH_PAGE_14    ((uint32_t)0x08007000) // Page 14, 2 Kbyte
#define ADDR_FLASH_PAGE_15    ((uint32_t)0x08007800) // Page 15, 2 Kbyte
#define ADDR_FLASH_PAGE_16    ((uint32_t)0x08008000) // Page 16, 2 Kbyte
#define ADDR_FLASH_PAGE_17    ((uint32_t)0x08008800) // Page 17, 2 Kbyte
#define ADDR_FLASH_PAGE_18    ((uint32_t)0x08009000) // Page 18, 2 Kbyte
#define ADDR_FLASH_PAGE_19    ((uint32_t)0x08009800) // Page 19, 2 Kbyte
#define ADDR_FLASH_PAGE_20    ((uint32_t)0x0800A000) // Page 20, 2 Kbyte
#define ADDR_FLASH_PAGE_21    ((uint32_t)0x0800A800) // Page 21, 2 Kbyte
#define ADDR_FLASH_PAGE_22    ((uint32_t)0x0800B000) // Page 22, 2 Kbyte
#define ADDR_FLASH_PAGE_23    ((uint32_t)0x0800B800) // Page 23, 2 Kbyte
#define ADDR_FLASH_PAGE_24    ((uint32_t)0x0800C000) // Page 24, 2 Kbyte
#define ADDR_FLASH_PAGE_25    ((uint32_t)0x0800C800) // Page 25, 2 Kbyte
#define ADDR_FLASH_PAGE_26    ((uint32_t)0x0800D000) // Page 26, 2 Kbyte
#define ADDR_FLASH_PAGE_27    ((uint32_t)0x0800D800) // Page 27, 2 Kbyte
#define ADDR_FLASH_PAGE_28    ((uint32_t)0x0800E000) // Page 28, 2 Kbyte
#define ADDR_FLASH_PAGE_29    ((uint32_t)0x0800E800) // Page 29, 2 Kbyte
#define ADDR_FLASH_PAGE_30    ((uint32_t)0x0800F000) // Page 30, 2 Kbyte
#define ADDR_FLASH_PAGE_31    ((uint32_t)0x0800F800) // Page 31, 2 Kbyte

#define ADDR_FLASH_PAGE_116   ((uint32_t)0x0803A000) // Page 116, 2 Kbyte
#define ADDR_FLASH_PAGE_117   ((uint32_t)0x0803A800) // Page 117, 2 Kbyte
#define ADDR_FLASH_PAGE_118   ((uint32_t)0x0803B000) // Page 118, 2 Kbyte
#define ADDR_FLASH_PAGE_119   ((uint32_t)0x0803B800) // Page 119, 2 Kbyte
#define ADDR_FLASH_PAGE_120   ((uint32_t)0x0803C000) // Page 120, 2 Kbytes
#define ADDR_FLASH_PAGE_121   ((uint32_t)0x0803C800) // Page 121, 2 Kbytes
#define ADDR_FLASH_PAGE_122   ((uint32_t)0x0803D000) // Page 122, 2 Kbytes
#define ADDR_FLASH_PAGE_123   ((uint32_t)0x0803D800) // Page 123, 2 Kbytes
#define ADDR_FLASH_PAGE_124   ((uint32_t)0x0803E000) // Page 124, 2 Kbytes
#define ADDR_FLASH_PAGE_125   ((uint32_t)0x0803E800) // Page 125, 2 Kbytes
#define ADDR_FLASH_PAGE_126   ((uint32_t)0x0803F000) // Page 126, 2 Kbytes
#define ADDR_FLASH_PAGE_127   ((uint32_t)0x0803F800) // Page 127, 2 Kbytes

#define FLASH_LEVEL_START_ADDR ADDR_FLASH_PAGE_124
#define FLASH_CAL_START_ADDR ADDR_FLASH_PAGE_125
#define FLASH_USER_START_ADDR ADDR_FLASH_PAGE_126
#define FLASH_TEMP_START_ADDR ADDR_FLASH_PAGE_127

// 0x0803F800 ~ 0x0803FFFF

void FlashErase();
void FlashSave(uint32_t Add, uint32_t data);

#endif /* INC_COM_FLASH_H_ */
