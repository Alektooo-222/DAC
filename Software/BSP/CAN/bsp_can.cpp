#include "bsp_can.h"
#include "can_common.h"
#include <cassert>
#include <cstring>
#include "bsp.h"

extern "C"
{
#include "stm32f3xx_hal.h"
    /* FreeRTOS */
}

void Error_Handler(void);

CAN_HandleTypeDef hcan;

CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader0;
CAN_RxHeaderTypeDef RxHeader1;
uint8_t TxData[8] = {
    0,
};
uint32_t TxMailbox = 0;

uint8_t RxData0[8] = {
    0,
};

uint8_t RxData1[8] = {
    0,
};

/* void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
} */

void BSP::Can::init(CanRxCallback rxCallback, CanTxCallback txCallback)
{
    CAN_FilterTypeDef sFilterConfig;

    rxCompleteCallback = rxCallback;
    txCompleteCallback = txCallback;

    hcan.Instance = CAN;
    hcan.Init.Prescaler = 9;
    hcan.Init.Mode = CAN_MODE_NORMAL;
    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
    hcan.Init.TimeTriggeredMode = DISABLE;
    hcan.Init.AutoBusOff = ENABLE;
    hcan.Init.AutoWakeUp = DISABLE;
    hcan.Init.AutoRetransmission = ENABLE;
    hcan.Init.ReceiveFifoLocked = DISABLE;
    hcan.Init.TransmitFifoPriority = ENABLE;
    if (HAL_CAN_Init(&hcan) != HAL_OK)
    {
        Error_Handler();
    }

    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    // sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
    {
        /* Filter configuration Error */
        Error_Handler();
    }
}

void BSP::Can::begin(CanBaubrate speed)
{
    HAL_CAN_Start(&hcan);
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_ERROR | CAN_IT_BUSOFF | CAN_IT_LAST_ERROR_CODE | CAN_IT_RX_FIFO1_MSG_PENDING);
}

bool BSP::Can::send(const CanMessage *message)
{
    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
    {
        return false;
    }

    TxHeader.StdId = 0;
    TxHeader.ExtId = message->id;
    TxHeader.RTR = CAN_RTR_DATA; // CAN_RTR_REMOTE
    TxHeader.IDE = CAN_ID_EXT;   // CAN_ID_EXT
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, message->data, &TxMailbox) != HAL_OK)
    {
        return false;
    }

    return true;
}

extern "C"
{
    void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
    {
        CanMessage rxMsg = {};

        HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader0, RxData0);

        rxMsg.id = RxHeader0.ExtId;
        rxMsg.length = RxHeader0.DLC;
        memcpy(rxMsg.data, RxData0, RxHeader0.DLC);

        if (BSP::Can::rxCompleteCallback != nullptr)
        {
            BSP::Can::rxCompleteCallback(&rxMsg);
        }

        return;
    }

    void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
    {
        CanMessage rxMsg = {};

        HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &RxHeader1, RxData1);

        rxMsg.id = RxHeader1.ExtId;
        rxMsg.length = RxHeader1.DLC;
        memcpy(rxMsg.data, RxData1, RxHeader1.DLC);

        if (BSP::Can::rxCompleteCallback != nullptr)
        {
            BSP::Can::rxCompleteCallback(&rxMsg);
        }

        return;
    }

    void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle)
    {

        GPIO_InitTypeDef GPIO_InitStruct = {0};
        if (canHandle->Instance == CAN)
        {
            /* CAN clock enable */
            __HAL_RCC_CAN1_CLK_ENABLE();

            __HAL_RCC_GPIOB_CLK_ENABLE();
            /**CAN GPIO Configuration
            PB8     ------> CAN_RX
            PB9     ------> CAN_TX
            */
            GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            /* CAN interrupt Init */
            /* HAL_NVIC_SetPriority(USB_HP_CAN_TX_IRQn, BSP::CAN_IRQ_PRIO, 0);
            HAL_NVIC_EnableIRQ(USB_HP_CAN_TX_IRQn);
            HAL_NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, BSP::CAN_IRQ_PRIO, 0);
            HAL_NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
            HAL_NVIC_SetPriority(CAN_RX1_IRQn, BSP::CAN_IRQ_PRIO, 0);
            HAL_NVIC_EnableIRQ(CAN_RX1_IRQn);
            HAL_NVIC_SetPriority(CAN_SCE_IRQn, BSP::CAN_IRQ_PRIO, 0);
            HAL_NVIC_EnableIRQ(CAN_SCE_IRQn); */

            HAL_NVIC_SetPriority(USB_HP_CAN_TX_IRQn, 5, 0);
            HAL_NVIC_EnableIRQ(USB_HP_CAN_TX_IRQn);
            HAL_NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, 5, 0);
            HAL_NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
            HAL_NVIC_SetPriority(CAN_RX1_IRQn, 5, 0);
            HAL_NVIC_EnableIRQ(CAN_RX1_IRQn);
            HAL_NVIC_SetPriority(CAN_SCE_IRQn, 5, 0);
            HAL_NVIC_EnableIRQ(CAN_SCE_IRQn);
        }
    }

    void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle)
    {

        if (canHandle->Instance == CAN)
        {
            /* Peripheral clock disable */
            __HAL_RCC_CAN1_CLK_DISABLE();

            /**CAN GPIO Configuration
            PB8     ------> CAN_RX
            PB9     ------> CAN_TX
            */
            HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);

            /* CAN interrupt Deinit */
            /* USER CODE BEGIN CAN:USB_HP_CAN_TX_IRQn disable */
            /**
             * Uncomment the line below to disable the "USB_HP_CAN_TX_IRQn" interrupt
             * Be aware, disabling shared interrupt may affect other IPs
             */
            /* HAL_NVIC_DisableIRQ(USB_HP_CAN_TX_IRQn); */
            /* USER CODE END CAN:USB_HP_CAN_TX_IRQn disable */

            /* USER CODE BEGIN CAN:USB_LP_CAN_RX0_IRQn disable */
            /**
             * Uncomment the line below to disable the "USB_LP_CAN_RX0_IRQn" interrupt
             * Be aware, disabling shared interrupt may affect other IPs
             */
            /* HAL_NVIC_DisableIRQ(USB_LP_CAN_RX0_IRQn); */
            /* USER CODE END CAN:USB_LP_CAN_RX0_IRQn disable */

            HAL_NVIC_DisableIRQ(CAN_RX1_IRQn);
            HAL_NVIC_DisableIRQ(CAN_SCE_IRQn);
        }
    }
}