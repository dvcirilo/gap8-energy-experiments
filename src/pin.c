#include "pin.h"

void set_pin(PinName pin, int value)
{
    /* Parsing GPIO pin to get real number for port, gpio and pin*/
    uint32_t port_number = GET_GPIO_PORT(pin);
    uint32_t gpio_number = GET_GPIO_NUM(pin);
    uint32_t pin_number  = GET_GPIO_PIN_NUM(pin);

    if (value == 0)
        GPIO_WritePinOutput( gpio_addrs[port_number], gpio_number, uGPIO_LOW);
    else
        GPIO_WritePinOutput( gpio_addrs[port_number], gpio_number, uGPIO_HIGH);
}

void init_pin(PinName pin)
{
    /* Parsing GPIO pin to get real number for port, gpio and pin*/
    uint32_t port_number = GET_GPIO_PORT(pin);
    uint32_t gpio_number = GET_GPIO_NUM(pin);
    uint32_t pin_number  = GET_GPIO_PIN_NUM(pin);

    PORT_SetPinMux(port_addrs[port_number], pin_number,  uPORT_MuxGPIO);

    /* Init GPIO - OUTPUT. */
    gpio_pin_config_t gpio_config = { .pinDirection  = uGPIO_DigitalOutput,
                                      .outputLogic   = uGPIO_LOW,
                                      .pullSelect    = uGPIO_PullUpEnable,
                                      .driveStrength = uGPIO_LowDriveStrength,
                                    };

    GPIO_PinInit ( gpio_addrs[port_number], gpio_number, &gpio_config );
}
