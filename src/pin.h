#ifndef PIN_H
#define PIN_H

#include "gap_common.h"

/* Variables used. */
/* Array of PORT and GPIO peripheral base address. */
static GPIO_Type *const gpio_addrs[] = GPIO_BASE_PTRS;
static PORT_Type *const port_addrs[] = PORT_BASE_PTRS;

void set_pin(PinName pin, int value);
void init_pin(PinName pin);

#endif
