#ifndef USB_COMM_H
#define USB_COMM_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// Function declarations
int usbinit(void);
void dataSend(int32_t* data)

#endif /* USB_COMM_H */