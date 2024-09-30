/*
 * com_gpio.c
 *
 *  Created on: Aug 17, 2023
 *      Author: sjpark
 */

#include "com_gpio.h"
#include "database.h"

typedef struct {
  GPIO_TypeDef *port;
  uint32_t pin;
  GPIO_PinState on_state;
} input_t;

typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
  GPIO_PinState on_state;
  GPIO_PinState off_state;
} output_t;

input_t input[INPUT_MAX_CH] = { { GPIOC, GPIO_PIN_0, GPIO_PIN_RESET }, { GPIOC, GPIO_PIN_1, GPIO_PIN_RESET }, { GPIOC, GPIO_PIN_6, GPIO_PIN_RESET }, { GPIOA,
GPIO_PIN_15, GPIO_PIN_RESET }, { GPIOC, GPIO_PIN_11, GPIO_PIN_RESET }, { GPIOC, GPIO_PIN_12, GPIO_PIN_RESET }, { GPIOC, GPIO_PIN_13, GPIO_PIN_RESET }, };

output_t output[OUTPUT_MAX_CH] = { { GPIOC, GPIO_PIN_2, GPIO_PIN_RESET, GPIO_PIN_SET }, { GPIOC, GPIO_PIN_3, GPIO_PIN_RESET, GPIO_PIN_SET }, { GPIOC,
GPIO_PIN_5, GPIO_PIN_RESET, GPIO_PIN_SET }, { GPIOC, GPIO_PIN_4, GPIO_PIN_RESET, GPIO_PIN_SET }, { GPIOC, GPIO_PIN_7, GPIO_PIN_RESET, GPIO_PIN_SET }, {
GPIOC, GPIO_PIN_8, GPIO_PIN_RESET, GPIO_PIN_SET }, { GPIOC, GPIO_PIN_10, GPIO_PIN_RESET, GPIO_PIN_SET }, { GPIOC, GPIO_PIN_9, GPIO_PIN_RESET, GPIO_PIN_SET }, {
GPIOB, GPIO_PIN_0, GPIO_PIN_SET, GPIO_PIN_RESET } };

void gpio_on(out_enum_t ch) {
  if (ch >= OUTPUT_MAX_CH)
    return;

  HAL_GPIO_WritePin(output[ch].port, output[ch].pin, output[ch].on_state);
}

void gpio_off(out_enum_t ch) {
  if (ch >= OUTPUT_MAX_CH)
    return;

  HAL_GPIO_WritePin(output[ch].port, output[ch].pin, output[ch].off_state);
}

void gpio_toggle(out_enum_t ch) {
  if (ch >= OUTPUT_MAX_CH)
    return;

  HAL_GPIO_TogglePin(output[ch].port, output[ch].pin);
}

bool gpio_Init(void) {
  bool ret = true;

  return ret;
}

bool buttonGetPressed(uint8_t ch) {
  bool ret = false;

  if (ch >= INPUT_MAX_CH) {
    return false;
  }

  if (HAL_GPIO_ReadPin(input[ch].port, input[ch].pin) == input[ch].on_state) {
    ret = true;
  }

  return ret;
}

void buttonObjCreate(button_obj_t *p_obj, uint8_t ch, uint32_t repeat_time) {
  p_obj->ch = ch;
  p_obj->state = 0;
  p_obj->pre_time = HAL_GetTick();
  p_obj->repeat_time = repeat_time;
}

bool buttonObjGetClicked(button_obj_t *p_obj, uint32_t pressed_time) {
  bool ret = false;

  switch (p_obj->state) {
    case 0:
      if (buttonGetPressed(p_obj->ch) == true) {
        p_obj->state = 1;
        p_obj->pre_time = HAL_GetTick();
      }
      break;

    case 1:
      if (buttonGetPressed(p_obj->ch) == true) {
        if (HAL_GetTick() - p_obj->pre_time >= pressed_time) {
          ret = true;  // 버튼 클릭됨
          p_obj->state = 2;
          p_obj->pre_time = HAL_GetTick();
        }
      } else {
        p_obj->state = 0;
      }
      break;

    case 2:
      if (buttonGetPressed(p_obj->ch) == true) {
        if (HAL_GetTick() - p_obj->pre_time >= p_obj->repeat_time) {
          p_obj->state = 1;
          p_obj->pre_time = HAL_GetTick();
        }
      } else {
        p_obj->state = 0;
      }
      break;
  }

  return ret;
}

bool InPutObjGetClicked(button_obj_t *p_obj, uint32_t pressed_time) {
  bool ret = false;

  switch (p_obj->state) {
    case 0:
      if (buttonGetPressed(p_obj->ch) == true) {
        p_obj->state = 1;
        p_obj->pre_time = HAL_GetTick();
      }
      break;

    case 1:
      if (buttonGetPressed(p_obj->ch) == true) {
        if (HAL_GetTick() - p_obj->pre_time >= pressed_time) {
          ret = true;  // 버튼 클릭됨
        }
      } else {
        p_obj->state = 0;
      }
      break;
  }

  return ret;
}

void AMo_Control(ONOFF_t on) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  if (on == ON) {
    pSystem->outputValue.NAME_FIELD.aMoRelayFlag = 1;
    gpio_on(A_MO_CON);
  } else {
    pSystem->outputValue.NAME_FIELD.aMoRelayFlag = 0;
    gpio_off(A_MO_CON);
  }
}

void BMo_Control(ONOFF_t on) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  if (on == ON) {
    pSystem->outputValue.NAME_FIELD.bMoRelayFlag = 1;
    gpio_on(B_MO_CON);
  } else {
    pSystem->outputValue.NAME_FIELD.bMoRelayFlag = 0;
    gpio_off(B_MO_CON);
  }
}

void AHt_Control(ONOFF_t on) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  if (on == ON) {
    pSystem->outputValue.NAME_FIELD.aHtRelayFlag = 1;
    gpio_on(A_HT_CON);
  } else {
    pSystem->outputValue.NAME_FIELD.aHtRelayFlag = 0;
    gpio_off(A_HT_CON);
  }
}

void BHt_Control(ONOFF_t on) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  if (on == ON) {
    pSystem->outputValue.NAME_FIELD.bHtRelayFlag = 1;
    gpio_on(B_HT_CON);
  } else {
    pSystem->outputValue.NAME_FIELD.bHtRelayFlag = 0;
    gpio_off(B_HT_CON);
  }
}

void ALLHH_Control(ONOFF_t on) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  if (on == ON) {
    pSystem->outputValue.NAME_FIELD.aLLHHRelayFlag = 1;
    gpio_on(A_LL_HH_CON);
  } else {
    pSystem->outputValue.NAME_FIELD.aLLHHRelayFlag = 0;
    gpio_off(A_LL_HH_CON);
  }
}

void BLLHH_Control(ONOFF_t on) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  if (on == ON) {
    pSystem->outputValue.NAME_FIELD.bLLHHRelayFlag = 1;
    gpio_on(B_LL_HH_CON);
  } else {
    pSystem->outputValue.NAME_FIELD.bLLHHRelayFlag = 0;
    gpio_off(B_LL_HH_CON);
  }
}

void APt_Control(ONOFF_t on) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  if (on == ON) {
    pSystem->outputValue.NAME_FIELD.aPtRelayFlag = 1;
    gpio_on(A_PT_CON);
  } else {
    pSystem->outputValue.NAME_FIELD.aPtRelayFlag = 0;
    gpio_off(A_PT_CON);
  }
}

void BPt_Control(ONOFF_t on) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  if (on == ON) {
    pSystem->outputValue.NAME_FIELD.bPtRelayFlag = 1;
    gpio_on(B_PT_CON);
  } else {
    pSystem->outputValue.NAME_FIELD.bPtRelayFlag = 0;
    gpio_off(B_PT_CON);
  }
}

/**
 * @brief   부저 동작
 * @note
 * @param On or Off
 * @retval
 */
void BUZ_Control(ONOFF_t on) {
  SYSTEM_t *pSystem = (SYSTEM_t*) DataBase_Get_pInfo_Data();
  if (on == ON) {
    pSystem->outputValue.NAME_FIELD.buzzerFlag = 1;
    gpio_on(BUZZER);
  } else {
    pSystem->outputValue.NAME_FIELD.buzzerFlag = 0;
    gpio_off(BUZZER);
  }
}

