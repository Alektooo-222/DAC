#pragma once

#include <cstdio>

#include "can_common.h"

using HandlerFunc = void (*)(CanMessage *can_rx_message);

struct CommandHandler
{
    uint32_t id;
    HandlerFunc handler;
};

#define NUMBER_OF_CAN_HANDLERS 1U



void ID_0x00_00_00_01(CanMessage *can_rx_message);