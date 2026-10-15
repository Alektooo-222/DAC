#include "CAN_Handlers.h"
#include "dac.h"

constexpr float resolution_mV_bit = 19.6f;

CommandHandler CANHandlers[NUMBER_OF_CAN_HANDLERS] = {
    {0x00000001, ID_0x00_00_00_01}};

void ID_0x00_00_00_01(CanMessage *can_rx_message)
{
    if (can_rx_message->data[3] > 4)
    {
        return;
    }

    Dac::set_sine(can_rx_message->data[0],
                  static_cast<uint16_t>(static_cast<float>(can_rx_message->data[1]) * resolution_mV_bit),
                  static_cast<uint16_t>(static_cast<float>(can_rx_message->data[2]) * resolution_mV_bit),
                  static_cast<Dac::CHANNEL>(can_rx_message->data[3] - 1U));
    return;
}
