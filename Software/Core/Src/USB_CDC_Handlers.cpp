
#include "USB_CDC_Handlers.h"
#include "usb_cdc.h"
#include "dac.h"
#include "usb_cdc_common.h"

#include "etl/string.h"
#include "etl/string_view.h"
#include "etl/string_utilities.h"
#include "etl/to_string.h"
#include "etl/format_spec.h"

#include <stdint.h>

constexpr float resolution_mV_bit = 19.6f;

void SET_SINE_USB_CDC(etl::string<MAX_SIZE_USB_CDC_MESSAGE> &str_cmd);
void SET_VOLTAGE_USB_CDC(etl::string<MAX_SIZE_USB_CDC_MESSAGE> &str_cmd);

using HandlerFuncUsbCdc = void (*)(etl::string<MAX_SIZE_USB_CDC_MESSAGE> &cmd);

uint8_t str_inv_arg_usb_cdc[]  = "Invalid Argument\n";
uint8_t str_inv_ch_usb_cdc[]  = "Invalid Channel\n";
uint8_t str_wrong_arg_cnt_usb_cdc[] = "Wrong Argument Count\n";

etl::string<MAX_SIZE_USB_CDC_MESSAGE> response_usb_cdc;

struct CommandHandlerUsbCdc
{
    const char *id;
    HandlerFuncUsbCdc handler;
};

struct SetSineCommand
{
    uint32_t freq;
    uint32_t amp_mV;
    uint32_t mid_point_mV;
    uint32_t channel;
};

struct SetVoltageCommand
{
    uint32_t mid_point_mV;
    uint32_t channel;
};

enum class ParseResult
{
    Ok,
    InvalidCommand,
    InvalidArgument,
    WrongArgumentCount,
    Overflow
};

CommandHandlerUsbCdc UsbCdcHandlers[NUMBER_OF_USB_CDC_HANDLERS] = {
    {"set_sine", SET_SINE_USB_CDC},
    {"set_voltage", SET_VOLTAGE_USB_CDC}};

/* void to_lowercase(etl::string<MAX_SIZE_USB_CDC_MESSAGE> &str)
{
    for (char &c : str)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
} */

static bool parse_uint32(etl::string_view str, uint32_t &value)
{
    if (str.empty())
        return false;

    uint32_t result = 0;

    for (char c : str)
    {
        if (c < '0' || c > '9')
            return false;

        uint32_t digit = static_cast<uint32_t>(c - '0');

        // Проверка переполнения uint32_t
        if (result > (UINT32_MAX - digit) / 10)
            return false;

        result = result * 10 + digit;
    }

    value = result;
    return true;
}

ParseResult inline parse_set_sine(
    etl::string_view message,
    SetSineCommand &command)
{
    // Ищем пробелы и выделяем 5 токенов:
    //
    // set_sine
    // freq
    // amp_mV
    // mid_point_mV
    // channel

    etl::string_view tokens[5];

    size_t token_count = 0;
    size_t start = 0;

    for (size_t i = 0; i <= message.size(); ++i)
    {
        if (i == message.size() || message[i] == ' ' || message[i] == '\n')
        {
            if (i > start)
            {
                if (token_count >= 5)
                    return ParseResult::WrongArgumentCount;

                tokens[token_count++] =
                    message.substr(start, i - start);
            }

            start = i + 1;
        }
    }

    if (token_count != 5)
        return ParseResult::WrongArgumentCount;

    // Проверяем имя команды
    if (tokens[0] != "set_sine")
        return ParseResult::InvalidCommand;

    if (!parse_uint32(tokens[1], command.freq))
        return ParseResult::InvalidArgument;

    if (!parse_uint32(tokens[2], command.amp_mV))
        return ParseResult::InvalidArgument;

    if (!parse_uint32(tokens[3], command.mid_point_mV))
        return ParseResult::InvalidArgument;

    if (!parse_uint32(tokens[4], command.channel))
        return ParseResult::InvalidArgument;

    return ParseResult::Ok;
}

ParseResult inline parse_set_voltage(
    etl::string_view message,
    SetVoltageCommand &command)
{
    // Ищем пробелы и выделяем 3 токенов:
    //
    // mid_point_mV
    // channel

    static constexpr uint8_t number_of_tokens = 3U;

    etl::string_view tokens[3];

    size_t token_count = 0;
    size_t start = 0;

    for (size_t i = 0; i <= message.size(); ++i)
    {
        if (i == message.size() || message[i] == ' ' || message[i] == '\n')
        {
            if (i > start)
            {
                if (token_count >= number_of_tokens)
                    return ParseResult::WrongArgumentCount;

                tokens[token_count++] =
                    message.substr(start, i - start);
            }

            start = i + 1;
        }
    }

    if (token_count != number_of_tokens)
        return ParseResult::WrongArgumentCount;

    // Проверяем имя команды
    if (tokens[0] != "set_voltage")
        return ParseResult::InvalidCommand;

    if (!parse_uint32(tokens[1], command.mid_point_mV))
        return ParseResult::InvalidArgument;

    if (!parse_uint32(tokens[2], command.channel))
        return ParseResult::InvalidArgument;

    return ParseResult::Ok;
}

void SET_SINE_USB_CDC(etl::string<MAX_SIZE_USB_CDC_MESSAGE> &str_cmd)
{
    SetSineCommand parsered_cmd = {};
    UsbCdcMessage msg = {};

    ParseResult state  = parse_set_sine(str_cmd, parsered_cmd); //ParseResult::InvalidArgument; //parse_set_sine(str_cmd, parsered_cmd);

    if (state == ParseResult::InvalidArgument)
    {
        msg.size = sizeof(str_inv_arg_usb_cdc);
        memcpy(msg.data, str_inv_arg_usb_cdc, msg.size);
        Usb_Cdc::send(&msg);
        return;
    }

    if (state == ParseResult::WrongArgumentCount)
    {
        msg.size = sizeof(str_wrong_arg_cnt_usb_cdc);
        memcpy(msg.data, str_wrong_arg_cnt_usb_cdc, msg.size);
        Usb_Cdc::send(&msg);
        return;
    }

    if (parsered_cmd.channel > 4U || parsered_cmd.channel <= 0U)
    {
        msg.size = sizeof(str_inv_ch_usb_cdc);
        memcpy(msg.data, str_inv_ch_usb_cdc, msg.size);
        Usb_Cdc::send(&msg);
        return;
    }

    

    Dac::set_sine(parsered_cmd.freq,
                  parsered_cmd.amp_mV,
                  parsered_cmd.mid_point_mV,
                  static_cast<Dac::CHANNEL>(parsered_cmd.channel - 1U));
    
    response_usb_cdc += "OK. Set sine freq=";

    etl::to_string(parsered_cmd.freq, response_usb_cdc, true);
    response_usb_cdc += " amp=";
    etl::to_string(parsered_cmd.amp_mV, response_usb_cdc, true);
    response_usb_cdc += " mV mid_point=";
    etl::to_string(parsered_cmd.mid_point_mV, response_usb_cdc, true);
    response_usb_cdc += " mV channel=";
    etl::to_string(parsered_cmd.channel, response_usb_cdc, true);
    response_usb_cdc += "\n";

    msg.size = response_usb_cdc.size();
    memcpy(msg.data, response_usb_cdc.c_str(), msg.size);
    Usb_Cdc::send(&msg);
    response_usb_cdc.clear();

    return;
};

void SET_VOLTAGE_USB_CDC(etl::string<MAX_SIZE_USB_CDC_MESSAGE> &str_cmd)
{

    SetVoltageCommand parsered_cmd = {};
    UsbCdcMessage msg = {};

    ParseResult state  = parse_set_voltage(str_cmd, parsered_cmd);

    if (state == ParseResult::InvalidArgument)
    {
        msg.size = sizeof(str_inv_arg_usb_cdc);
        memcpy(msg.data, str_inv_arg_usb_cdc, msg.size);
        Usb_Cdc::send(&msg);
        return;
    }

    if (state == ParseResult::WrongArgumentCount)
    {
        msg.size = sizeof(str_wrong_arg_cnt_usb_cdc);
        memcpy(msg.data, str_wrong_arg_cnt_usb_cdc, msg.size);
        Usb_Cdc::send(&msg);
        return;
    }

    if (parsered_cmd.channel > 4U || parsered_cmd.channel <= 0U)
    {
        msg.size = sizeof(str_inv_ch_usb_cdc);
        memcpy(msg.data, str_inv_ch_usb_cdc, msg.size);
        Usb_Cdc::send(&msg);
        return;
    }

    

    Dac::set_sine(0U,
                  0U,
                  parsered_cmd.mid_point_mV,
                  static_cast<Dac::CHANNEL>(parsered_cmd.channel - 1U));

    response_usb_cdc += "OK. Set voltage=";

    etl::to_string(parsered_cmd.mid_point_mV, response_usb_cdc, true);
    response_usb_cdc += " channel=";
    etl::to_string(parsered_cmd.channel, response_usb_cdc, true);
    /* response += " mV mid_point=";
    etl::to_string(parsered_cmd.mid_point_mV, response, true);
    response += " mV channel=";
    etl::to_string(parsered_cmd.channel, response, true); */
    response_usb_cdc += "\n";

    msg.size = response_usb_cdc.size();
    memcpy(msg.data, response_usb_cdc.c_str(), msg.size);
    Usb_Cdc::send(&msg);
    response_usb_cdc.clear();

    return;
};

void UsbCdcCommmandHandler(UsbCdcMessage *usb_cdc_rx_message)
{
    etl::string<MAX_SIZE_USB_CDC_MESSAGE> str(reinterpret_cast<char *>(usb_cdc_rx_message->data), usb_cdc_rx_message->size);

    etl::string_view cmd_name;

    etl::to_lower_case(str);

    for (size_t i = 0; i <= str.size(); ++i)
    {
        if (i == str.size() || str[i] == ' ' || str[i] == '\n')
        {
            cmd_name = str.substr(0, i);
            break;
        }
    }

    for (uint8_t i = 0; i < NUMBER_OF_USB_CDC_HANDLERS; i++)
    {
        if (cmd_name.compare(reinterpret_cast<const char *>(UsbCdcHandlers[i].id)) == 0)
        {
            UsbCdcHandlers[i].handler(str);
            continue;
        }
    }
    return;
}
