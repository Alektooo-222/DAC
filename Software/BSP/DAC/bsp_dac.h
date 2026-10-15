#ifndef BSP_DAC_H
#define BSP_DAC_H

#include <cstdint>
#include "bsp.h"

namespace BSP
{
    class DacGenerator
    {
    public:
        enum class CONST : uint16_t
        {
            number_of_channel = 4,
            sine_samples = 1000,
            sample_rate = 4000, // Hz
            max_out_voltage_mV = 5000,
        };

        enum class CHANNEL : uint8_t
        {
            CHANNEL_1 = 0,
            CHANNEL_2,
            CHANNEL_3,
            CHANNEL_4,
        };

        struct signal_param_type
        {
            uint8_t freq;
            float phase_step;
            uint16_t mid_point_mV;
            uint16_t amp_mV;
            CHANNEL ch;
        };

        static void init(dmaCallbackType dmaCallback);
        static void set_sine(uint8_t freq, uint16_t amp_mV, uint16_t mid_point_mV, CHANNEL ch);
        static void set_voltage(uint16_t voltage_mV, CHANNEL ch);
        static void begin(CHANNEL ch);
        static void stop(CHANNEL ch);
        static void draw_buf1();
        static void draw_buf2();

        static inline dmaCallbackType dmaCallback = nullptr;

    private:
        static inline uint16_t signals[static_cast<uint16_t>(CONST::sine_samples) * static_cast<uint16_t>(CONST::number_of_channel)] = {
            0,
        };

        static inline float phase_1 = 0.0f;
        static inline float phase_2 = 0.0f;
        static inline float phase_3 = 0.0f;
        static inline float phase_4 = 0.0f;

        static inline float val = 0.0f;
        
        static inline float TWO_PI = 6.28318530717958647692f;

        static inline signal_param_type signals_param[static_cast<uint16_t>(CONST::number_of_channel)] =
            {
                {43, TWO_PI * 43 / static_cast<float>(CONST::sample_rate), 2500, 2500, CHANNEL::CHANNEL_1},
                {20, TWO_PI * 20 / static_cast<float>(CONST::sample_rate), 2500, 2500, CHANNEL::CHANNEL_2},
                {30, TWO_PI * 30 / static_cast<float>(CONST::sample_rate), 2500, 2500, CHANNEL::CHANNEL_3},
                {40, TWO_PI * 40 / static_cast<float>(CONST::sample_rate), 2500, 2500, CHANNEL::CHANNEL_4}};
    };
}
#endif