#include "sram.h"

volatile uint16_t *fsmcCommand;
volatile uint16_t *fsmcData;

DMA_HandleTypeDef hdma_fsmc;
SRAM_HandleTypeDef hsram1;
FSMC_NORSRAM_TimingTypeDef Timing = {0};
FSMC_NORSRAM_TimingTypeDef ExtTiming = {0};
GPIO_InitTypeDef GPIO_InitStruct = {0};

void fsmc_lcd_init()
{
    fsmcCommand = ((uint16_t *)((uint32_t)(0x60FFFFFE)));
    fsmcData = ((uint16_t *)((uint32_t)(0x61000000)));

    FSMC_Init();
    DMA_Init();
}

static void FSMC_Init(void)
{
    /* Peripheral clock enable */
    __HAL_RCC_FSMC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /** Perform the SRAM1 memory initialization sequence*/
    hsram1.Instance = FSMC_NORSRAM_DEVICE;
    hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram1.Init */
    hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
    hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
    hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
    hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
    hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
    hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
    /* Timing */
    Timing.AddressSetupTime = 1;
    Timing.AddressHoldTime = 0;
    Timing.DataSetupTime = 15;
    Timing.BusTurnAroundDuration = 0;
    Timing.CLKDivision = 0;
    Timing.DataLatency = 0;
    Timing.AccessMode = FSMC_ACCESS_MODE_A;
    /* ExtTiming */
    ExtTiming.AddressSetupTime = 0;
    ExtTiming.AddressHoldTime = 0;
    ExtTiming.DataSetupTime = 3;
    ExtTiming.BusTurnAroundDuration = 0;
    ExtTiming.CLKDivision = 0;
    ExtTiming.DataLatency = 0;
    ExtTiming.AccessMode = FSMC_ACCESS_MODE_A;
    if (HAL_SRAM_Init(&hsram1, &Timing, &ExtTiming) != HAL_OK)
    {
    }
}

extern "C" void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_fsmc);
    // Überprüfen Sie, ob der DMA-Transfer abgeschlossen ist
    if (__HAL_DMA_GET_FLAG(&hdma_fsmc, DMA_FLAG_TCIF0_4))
    {
        __HAL_DMA_CLEAR_FLAG(&hdma_fsmc, DMA_FLAG_TCIF0_4);
        // Optional: Setzen eines Flags oder eine Callback-Funktion aufrufen
    }
}

void DMA_TransferCompleteCallback(DMA_HandleTypeDef *hdma)
{
    if (hdma == &hdma_fsmc)
    {
    }
}

void DMA_TransferErrorCallback(DMA_HandleTypeDef *hdma)
{
}

static void DMA_Init(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    HAL_DMA_RegisterCallback(&hdma_fsmc, HAL_DMA_XFER_CPLT_CB_ID, DMA_TransferCompleteCallback);
    HAL_DMA_RegisterCallback(&hdma_fsmc, HAL_DMA_XFER_ERROR_CB_ID, DMA_TransferErrorCallback);

    /* Set the parameters to be configured */
    hdma_fsmc.Instance = DMA2_Stream0;
    hdma_fsmc.Init.Channel = DMA_CHANNEL_0;
    hdma_fsmc.Init.Direction = DMA_MEMORY_TO_MEMORY;
    hdma_fsmc.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_fsmc.Init.MemInc = DMA_MINC_ENABLE;
    hdma_fsmc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_fsmc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_fsmc.Init.Mode = DMA_NORMAL;
    hdma_fsmc.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_fsmc.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_fsmc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma_fsmc.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_fsmc.Init.PeriphBurst = DMA_PBURST_SINGLE;

    if (HAL_DMA_Init(&hdma_fsmc) != HAL_OK)
    {
        uint32_t error = HAL_DMA_GetError(&hdma_fsmc);
    }
    __HAL_LINKDMA(&hsram1, hdma, hdma_fsmc);
}
