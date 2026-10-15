
#ifndef CAN_H_
#define CAN_H_

#include <assert.h>

#include "hal_can.h"
#include "bsp_can.h"
#include "status.h"

#include  "CAN_Handlers.h"

extern "C"
{
/* FreeRTOS */
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
}

extern CommandHandler CANHandlers[NUMBER_OF_CAN_HANDLERS];

class Can
{
public:
    static void init(void)
    {
        /* Create transmit queue */
        txQueue = xQueueCreate(txQueueLength, sizeof(CanMessage));
        assert(txQueue != nullptr);

        /* Create receive queue */
        rxQueue = xQueueCreate(rxQueueLength, sizeof(CanMessage));
        assert(rxQueue != nullptr);

        /* Create transmit thread */
        if (pdPASS != xTaskCreate(txTask, "CANTx", configMINIMAL_STACK_SIZE,
                                  NULL, tskIDLE_PRIORITY + 4, NULL))
        {
            assert(false);
        }

        /* Create transmit thread */
        if (pdPASS != xTaskCreate(rxTask, "CANRx", configMINIMAL_STACK_SIZE,
                                  NULL, tskIDLE_PRIORITY + 4, NULL))
        {
            assert(false);
        }

        /* Init can module */
        CanPort::init(canRxCallback, nullptr);
    };

    /* Config the speed and start */
    static Status begin(CanBaubrate speed)
    {
        Status status = Status::OK;
        CanPort::begin(speed);
        return status;
    };

    /* Send the message */
    static Status send(CanMessage *message)
    {
        Status status = Status::OK;

        assert(txQueue != nullptr);

        /* Call from IRQ or Thread */
        if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0U)
        {
            if (xQueueSend(txQueue, (void *)message, (TickType_t)0U) != pdPASS)
            {
                /* Failed to post the message */
                status = Status::BUSY;
            }
        }
        else
        {
            portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

            if (xQueueSendFromISR(txQueue, (void *)message, &xHigherPriorityTaskWoken) != pdPASS)
            {
                /* Failed to post the message */
                status = Status::BUSY;
            }

            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

        return status;
    };

    /* Return count of received messages */
    static uint32_t getCountOfReceivedMessages(void)
    {
        assert(rxQueue != nullptr);

        return uxQueueMessagesWaiting(rxQueue);
    };

    /* Receive the message */
    static Status receive(CanMessage *message)
    {
        Status status = Status::OK;

        assert(rxQueue != nullptr);

        /* Call from IRQ or Thread */
        if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0U)
        {
            if (xQueueReceive(rxQueue, (void *)message, (TickType_t)0U) != pdPASS)
            {
                /* Failed to post the message */
                status = Status::EMPTY;
            }
        }
        else
        {
            portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

            if (xQueueReceiveFromISR(rxQueue, (void *)message, &xHigherPriorityTaskWoken) != pdPASS)
            {
                /* Failed to post the message */
                status = Status::EMPTY;
            }

            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

        return status;
    };

private:
    using CanPort = HAL::Can<BSP::Can, CanBaubrate>;

    static constexpr uint32_t txQueueLength = 32U;
    static constexpr uint32_t rxQueueLength = 32U;

    static inline QueueHandle_t txQueue = nullptr;
    static inline QueueHandle_t rxQueue = nullptr;

    /* CAN receive callback */
    static void canRxCallback(CanMessage *message)
    {
        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        CanMessage dummyMessage;

        /* Check available space */
        if (uxQueueMessagesWaitingFromISR(rxQueue) == rxQueueLength)
        {
            /* Delete older messge from queue */
            xQueueReceiveFromISR(rxQueue, (void *)&dummyMessage, &xHigherPriorityTaskWoken);
        }

        /* Put the received message in to queue */
        xQueueSendFromISR(rxQueue, (void *)message, &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    };

    /* Transmit task */
    static void txTask(void *pvParameters)
    {
        CanMessage txMessage = {0};

        for (;;)
        {
            /* Wait tx message */
            xQueueReceive(txQueue, &txMessage, portMAX_DELAY);
            /* Send message */
            while (CanPort::send(&txMessage) != Status::OK)
            {
                vTaskDelay(1U);
            }
        }
    }

    /* Transmit task */
    static void rxTask(void *pvParameters)
    {
        CanMessage rxMessage = {};

        for (;;)
        {
            /* Wait tx message */
            xQueueReceive(rxQueue, &rxMessage, portMAX_DELAY);

            for (uint8_t i = 0; i < NUMBER_OF_CAN_HANDLERS; i++)
            {
                if (rxMessage.id == CANHandlers[i].id)
                {
                    CANHandlers[i].handler(&rxMessage);
                    continue;
                }
            }
        }
    }
};
#endif /* CAN1_H_ */
