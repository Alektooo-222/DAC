#ifndef BOARD_H_
#define BOARD_H_

extern "C"
{
#include "stm32f3xx_hal.h"
}

class Board
{
private:
public:
    static void init()
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        __HAL_RCC_DMA1_CLK_ENABLE();
    }
};

#endif