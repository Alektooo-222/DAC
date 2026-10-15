#ifndef BSP_CAN_H
#define BSP_CAN_H

#include "bsp.h"

namespace BSP
{
    class Can
    {
    public:
        static void init(CanRxCallback rxCallback, CanTxCallback txCallback);
        static void begin(CanBaubrate speed);
        static bool send(const CanMessage *message);

        static inline CanRxCallback rxCompleteCallback = nullptr;
        static inline CanTxCallback txCompleteCallback = nullptr;
    private:
        
    };
}
#endif