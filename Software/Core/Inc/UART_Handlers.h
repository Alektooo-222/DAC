#pragma once

#include <cstdio>

#include "uart_common.h"

void CommmandHandler(UartMessage *uart_rx_message);

#define NUMBER_OF_UART_HANDLERS 2U
