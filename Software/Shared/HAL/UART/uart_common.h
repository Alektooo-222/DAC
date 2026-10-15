#ifndef UART_COMMON_H_
#define UART_COMMON_H_

#include <cstdint>
#include <cstddef>

#define MAX_SIZE_UART_MESSAGE 64U

struct UartMessage
{
    size_t size;
    uint8_t data[MAX_SIZE_UART_MESSAGE];
};

#endif /* UART_COMMON_H_ */
