#ifndef _sram_H_
#define _sram_H_

#include "stm32yyxx_hal_conf.h"
#include "stm32f4xx_ll_fsmc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_sram.h"
#include "stm32f4xx_hal.h"

extern DMA_HandleTypeDef hdma_fsmc;
extern SRAM_HandleTypeDef hsram1;

extern volatile uint16_t *fsmcData;
extern volatile uint16_t *fsmcCommand;

void fsmc_lcd_init(void);
static void FSMC_Init(void);
static void DMA_Init(void);

#endif