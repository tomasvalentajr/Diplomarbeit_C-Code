cmake_minimum_required(VERSION 3.25)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


#include(pico_sdk_import.cmake)

#project(Diplomarbeit C CXX ASM)
pico_sdk_init()

add_executable(Diplomarbeit
        main.c
        adc.c
        fft_imp.c
        kiss_fft/kiss_fft.c
)

target_include_directories(Diplomarbeit PRIVATE
        .
        kiss_fft
        
)

# pull in common dependencies and additional pwm hardware support
target_link_libraries(Diplomarbeit
        pico_stdlib
        hardware_gpio
        hardware_spi
        hardware_clocks
        hardware_timer
)


# printf only via USB
pico_enable_stdio_uart(Diplomarbeit 1)
pico_enable_stdio_usb(Diplomarbeit 1)


# create map/bin/hex file etc.
pico_add_extra_outputs(Diplomarbeit)
