#ifndef HAL_DAC_H
#define HAL_DAC_H

namespace HAL
{
    using dmaCallbackType = void(*)();

    template <class HwModule, class CONSTType, class CHANNELType>
    class Dac
    {
    public:
        static void init(dmaCallbackType dmaCallback)
        {
            HwModule::init(dmaCallback);
        }
        static void set_sine(uint8_t freq, uint16_t amp_mV, uint16_t mid_point_mV, CHANNELType ch)
        {
            HwModule::set_sine(freq, amp_mV, mid_point_mV, ch);
        }
        static void set_voltage(uint16_t voltage_mV, CHANNELType ch)
        {
            HwModule::set_voltage(voltage_mV, ch);
        }
        static void begin(CHANNELType ch)
        {
            HwModule::begin(ch);
        }
        static void stop(CHANNELType ch)
        {
            HwModule::stop(ch);
        }
        static void draw_buf1()
        {
            HwModule::draw_buf1();
        }
        static void draw_buf2()
        {
            HwModule::draw_buf2();
        }
    };
}
#endif