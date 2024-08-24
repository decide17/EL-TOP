/*
 * com_flash.c
 *
 *  Created on: Jul 20, 2024
 *      Author: sjpark
 */
#include "database.h"
#include <stdlib.h>
#include "Task_Input.h"
#include "Task_Cli.h"
#include "TM1640.h"
#include "com_flash.h"

static FLASH_EraseInitTypeDef EraseInitStruct;

void FlashErase(uint32_t addr) {
  uint32_t PageError;

//  __IO uint32_t data32 = 0, MemoryProgramStatus = 0;

  HAL_FLASH_Unlock();

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = addr;
  EraseInitStruct.NbPages = 1;
  EraseInitStruct.Banks = FLASH_BANK_1;

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
    Error_Handler();
  }
  HAL_FLASH_Lock();
}

void FlashSave(uint32_t Add, uint32_t data) {
  HAL_FLASH_Unlock();

  if (HAL_FLASH_Program(FLASH_PROC_MASSERASE, Add, data) == HAL_OK) {
    // printf("Flash write : ");
  } else {
    Error_Handler();
  }
  HAL_FLASH_Lock();
}
