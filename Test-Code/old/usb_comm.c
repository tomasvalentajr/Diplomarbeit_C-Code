#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define BAUD_RATE 9600
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int usbinit() {
    stdio_init_all();

    // Initialize UART with specified pins and baud rate
    uart_init(UART_ID, BAUD_RATE);
    uart_set_pin(UART_ID, UART_TX_PIN, UART_RX_PIN);


    // Enable UART FIFOs, clear them, and enable the UART
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, false);
    uart_set_hw_flow(UART_ID, false, false);

    printf("UART communication initialized\n");

    
    

    return 0;
}

void dataSend(int32_t* data) {
    // Send the entire array at once
    uart_write_blocking(UART_ID, (const uint8_t *)data, sizeof(data));
    //sleep_ms(1000);  // Wait for 1 second
}