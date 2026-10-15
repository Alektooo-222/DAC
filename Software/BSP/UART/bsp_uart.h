#ifndef BSP_UART_H
#define BSP_UART_H

#include "bsp.h"

namespace BSP
{
    class Uart
    {
    public:
        static void init(SerialRxCallback rxCallback, SerialTxCallback txCallback);
        static bool send(UartMessage *);
        static bool begin();
        static void copy_rx_buf(UartMessage *);
        

        static inline SerialRxCallback rxCompleteCallback = nullptr;
        static inline SerialTxCallback txCompleteCallback = nullptr;

    private:
        static inline uint8_t uart_rx_buf[MAX_SIZE_UART_MESSAGE] = {
            0,
        };
    };
}
#endif
