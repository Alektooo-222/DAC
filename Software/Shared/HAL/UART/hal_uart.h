#ifndef HAL_UARTH_
#define HAL_UARTH_

#include "uart_common.h"

#include "status.h"

namespace HAL
{
    using SerialTxCallback = void (*)();
    using SerialRxCallback = void (*)(UartMessage *);

    template <class HwModule>
    class Uart
    {
    public:
        static void init(SerialRxCallback rxCallback, SerialTxCallback txCallback)
        {
            HwModule::init(rxCallback, txCallback);
        }

        static inline void begin()
        {
            HwModule::begin();
        };

        static inline Status send(UartMessage * mes)
        {
            Status status = Status::BUSY;

            if(HwModule::send(mes) == true)
            {
                status = Status::OK;
            }
            
            return status;
        }
    };
}
#endif