#ifndef DAC_H
#define DAC_H

#include <assert.h>

#include "hal_dac.h"
#include "bsp_dac.h"

extern "C"
{
/* FreeRTOS */
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
}

class Dac
{
public:
    using CHANNEL = BSP::DacGenerator::CHANNEL;
    using CONST = BSP::DacGenerator::CONST;

    static void init()
    {
        if (pdPASS != xTaskCreate(draw_bufTask, "draw_bufTask", configMINIMAL_STACK_SIZE,
                                  NULL, tskIDLE_PRIORITY + 4, &draw_bufHandle))
        {
            assert(false);
        }
        DACPort::init(dma_callback);
    }
    static void set_sine(uint8_t freq, uint16_t amp_mV, uint16_t mid_point_mV, BSP::DacGenerator::CHANNEL ch)
    {
        DACPort::set_sine(freq, amp_mV, mid_point_mV, ch);
    }
    static void set_voltage(uint16_t voltage_mV, BSP::DacGenerator::CHANNEL ch)
    {
        DACPort::set_voltage(voltage_mV, ch);
    }
    static void begin(BSP::DacGenerator::CHANNEL ch)
    {
        DACPort::begin(ch);
    }
    static void stop(BSP::DacGenerator::CHANNEL ch)
    {
        DACPort::stop(ch);
    }

private:
    using DACPort = HAL::Dac<BSP::DacGenerator, BSP::DacGenerator::CONST, BSP::DacGenerator::CHANNEL>;

    static inline TaskHandle_t draw_bufHandle;

    static void draw_bufTask(void *pvParameters)
    {
        CanMessage txMessage = {0};

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        for (;;)
        {
            DACPort::draw_buf1();
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            DACPort::draw_buf2();
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
    }

    static void dma_callback()
    {
        BaseType_t higherPriorityTaskWoken = pdFALSE;

        vTaskNotifyGiveFromISR(
            draw_bufHandle,
            &higherPriorityTaskWoken);

        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }
};

#endif