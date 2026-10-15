#include "bsp_uart.h"
#include "uart_common.h"
#include <cstring>

extern "C"
{
#include "stm32f3xx_hal.h"
}

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (uartHandle->Instance == USART1)
    {
        /* USART1 clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PB6     ------> USART1_TX
        PB7     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* USART1 DMA Init */
        /* USART1_TX Init */
        hdma_usart1_tx.Instance = DMA1_Channel4;
        hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart1_tx.Init.Mode = DMA_NORMAL;
        hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
        if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
        {
            BSP::Error_Handler();
        }

        __HAL_LINKDMA(uartHandle, hdmatx, hdma_usart1_tx);

        /* USART1_RX Init */
        hdma_usart1_rx.Instance = DMA1_Channel5;
        hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart1_rx.Init.Mode = DMA_NORMAL;
        hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
        if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
        {
            BSP::Error_Handler();
        }

        __HAL_LINKDMA(uartHandle, hdmarx, hdma_usart1_rx);

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 10U, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{

    if (uartHandle->Instance == USART1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
        PB6     ------> USART1_TX
        PB7     ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);

        /* USART1 DMA DeInit */
        HAL_DMA_DeInit(uartHandle->hdmatx);
        HAL_DMA_DeInit(uartHandle->hdmarx);

        /* USART1 interrupt Deinit */
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
}

void BSP::Uart::init(SerialRxCallback rxCallback, SerialTxCallback txCallback)
{

    rxCompleteCallback = rxCallback;
    txCompleteCallback = txCallback;

    /* DMA1_Channel4_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 10U, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
    /* DMA1_Channel5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 10U, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

bool BSP::Uart::send(UartMessage *mes)
{
    if (HAL_UART_Transmit_DMA(&huart1, mes->data, mes->size) != HAL_OK)
    {
        return false;
    }

    return true;
}

bool BSP::Uart::begin()
{
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    HAL_UARTEx_ReceiveToIdle_DMA(
        &huart1,
        uart_rx_buf,
        MAX_SIZE_UART_MESSAGE);
    return true;
}

void BSP::Uart::copy_rx_buf(UartMessage *Msg)
{
    memcpy(Msg->data, uart_rx_buf, Msg->size);
    return;
}

extern "C"
{
    void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart,
                                    uint16_t Size)
    {
        UartMessage rxMsg = {};

        rxMsg.size = Size;
        BSP::Uart::copy_rx_buf(&rxMsg);

        if (BSP::Uart::rxCompleteCallback != nullptr)
        {
            BSP::Uart::rxCompleteCallback(&rxMsg);
        }

        if (huart->Instance == USART1)
        {
            BSP::Uart::begin();
        }
        return;
    }

    void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
    {
        if (huart->Instance == USART1)
        {
            uint32_t error = HAL_UART_GetError(huart);

            if (error & HAL_UART_ERROR_ORE)
            {
                BSP::Uart::begin();
            }
        }
        return;
    }
}