#include "adc.h"
#include "fft_imp.h"

#include "math.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/spi.h"


// ADC word length = 24 bit

#define int24(addr)             (*addr << 16 | *(addr + 1) << 8 | *(addr + 2))
#define switch_endian(val)      (((val & 0x00ff) << 8) | ((val & 0xff00) >> 8))

#define PIN_SCLK 10
#define PIN_DIN 11
#define PIN_DOUT 12
#define PIN_CS 13
#define PIN_DRDY 14
#define PIN_RST 15
#define PIN_CLKIN 21
#define MAX_ADC_VALUE 0x7FFFFF
#define MIN_NEG_ADC_VALUE 0xFFFFFF
#define MAX_NEG_ADC_VALUE 0x800000
#define MAX_VOLTAGE 1.2
#define MIN_VOLTAGE -1.2

uint8_t tx_data[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t rx_data[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


int32_t adc_data[4];
int len = 18;

static const uint8_t REG_MODE = 0x02;
static const uint8_t REG_CLOCK = 0x03;
//static const uint8_t REG_STATUS = 0x01;
//static const uint8_t REG_THRSHLD_LSB = 0x08;
//static const uint8_t REG_CH0_CFG = 0x09;
//static const uint8_t REG_CH1_CFG = 0x0E;
//static const uint8_t REG_CH2_CFG = 0x13;
//static const uint8_t REG_CH3_CFG = 0x18;

uint16_t ADC_CONFIG_CLOCK = 
        0b0000 << 12  |   // 15:12     Reserved
        0b1111 << 8  |   // 11:8    Enable all channels
        0b0   << 7  |   // 7        Reserved
        0b0   << 6  |   // 6        Reserved
        0b0   << 5  |   // 5        No TurboMode
        0b010 << 2  |   // 4:2      Oversampling ratio (0b010 = 512) -> könnte die Geschwindigkeit beeinflussen
        0b10;           // 1:0      Power mode (0b10 = high resolution)

uint16_t ADC_CONFIG_MODE = 
        0b00  << 14 |   // 15:14    Reserved
        0b0   << 13 |   // 13       Register Map CRC enable
        0b0   << 12 |   // 12       SPI input CRC enable
        0b0   << 11 |   // 11       SPI and register map CRC type
        0b0   << 10 |   // 10       Reset
        0b01  << 8  |   // 9:8      Data word length selection: 24 bit
        0b000 << 5  |   // 7:5      Reserved
        0b0   << 4  |   // 4        SPI Timeout enable
        0b00  << 2  |   // 3:2      DRDY pin signal source selection - 00b = most lagging enabled channel
        0b0   << 1  |   // 1        DRDY pin state when conversion data is not available - high
        0b0;            // 0        DRDY signal format when conversion data is available - low when data available till all data shifted

/*uint16_t ADC_CONFIG_THRSHLD_LSB = 
        0b00000000  << 8 |   // 15:8    Reserved
        0b0000 << 4  |   // 7:4      Reserved
        0b0000;          // 0        DC block filter disabled
*/
/*uint16_t ADC_CONFIG_CH0_CFG = 
        0x0000000000  << 6  |   // 15:6     Chanel 0 phase delay
        0b000   << 3  |   // 5:3        Reserved
        0b1 << 2  |   // 2      Disable DC Block filter
        0b00;           // 1:0      Chanel 0 input selection - positive DC test signal

uint16_t ADC_CONFIG_CH1_CFG = 
        0x0000000000  << 6  |   // 15:6     Chanel 1 phase delay
        0b000   << 3  |   // 5:3        Reserved
        0b1 << 2  |   // 2      Disable DC Block filter
        0b00;           // 1:0      Chanel 1 input selection - positive DC test signal

uint16_t ADC_CONFIG_CH2_CFG = 
        0x0000000000  << 6  |   // 15:6     Chanel 2 phase delay
        0b000   << 3  |   // 5:3        Reserved
        0b1 << 2  |   // 2      Disable DC Block filter
        0b00;           // 1:0      Chanel 2 input selection - positive DC test signal

uint16_t ADC_CONFIG_CH3_CFG = 
        0x0000000000  << 6  |   // 15:6     Chanel 3 phase delay
        0b000   << 3  |   // 5:3        Reserved
        0b1 << 2  |   // 2      Disable DC Block filter
        0b00;           // 1:0      Chanel 3 input selection - positive DC test signal
*/

static uint32_t toInt24(uint8_t* data) {

    uint32_t ret = (*data << 16 | *(data + 1) << 8 | *(data + 2));

    // For negative numbers (bit 23 set): Convert from 24-bit 1s complement to 32-bit by setting the top byte to 0xff
    printf("%x \n", ret);
    if (ret & (1 << 23)) ret |= (0xff << 24);
    printf("%x \n", ret);

    return ret;

}

static void ADC_SetCommand(uint16_t command)
{
    tx_data[0] = command >> 8 & 0xff;
    tx_data[1] = command & 0xff;
}

static void ADC_SetRegisterValue(uint16_t value)
{
    tx_data[3] = value >> 8 & 0xff;
    tx_data[4] = value & 0xff;
}

static uint16_t ADC_GetResponse(void)
{
    return rx_data[0] << 8 | rx_data[1];
}

static void ADC_ExchangeData(int len)
{
    gpio_put(PIN_CS, 0);       // 8,33 ns per instruction vs. 16 ns min delay
    sleep_us(1);
    spi_write_read_blocking(spi1, tx_data, rx_data, len);
    for(int x = 0; x < len; x += 3) {
            printf("%x %x %x\n", rx_data[x], rx_data[x+1], rx_data[x+2]);
    };
    sleep_us(1);
    gpio_put(PIN_CS, 1);
}

bool ADC_ConvertResults(void)
{
    uint8_t* offset = rx_data + 3;
    //printf("%x \n", offset);
    for (int ch = 0; ch < 4; ch++)
    {
        adc_data[ch] = toInt24(offset);
        //adc_data[ch] = int24(offset);
        //printf("%d \n", adc_data[ch]);
        offset +=3;
    }
    return true;
}


void ADC_Init(void)
{
    spi_init(spi1, 1050420); 

    // Start external clock for ADC
    clock_gpio_init(PIN_CLKIN, CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, 119);       // 125 MHz sys clock / 24 = 5,2 MHz
    gpio_set_function(PIN_CLKIN, GPIO_FUNC_GPCK);

    gpio_set_function(PIN_SCLK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_DIN, GPIO_FUNC_SPI);
    gpio_set_function(PIN_DOUT, GPIO_FUNC_SPI);

    // Configure the CS, DRDY, RST, and CLKIN pins
    gpio_init(PIN_CS);
    gpio_init(PIN_DRDY);
    gpio_init(PIN_RST);

    // Set the pins as outputs, except for DRDY (input)
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DRDY, GPIO_IN);
    gpio_set_dir(PIN_RST, GPIO_OUT);
    gpio_pull_up(PIN_DRDY);

    gpio_put(PIN_CS, 1);
    sleep_ms(10);
    ADC_WriteRegister(REG_MODE, ADC_CONFIG_MODE); //Configure ADC
    ADC_WriteRegister(REG_CLOCK, ADC_CONFIG_CLOCK); //Configure ADC
    //ADC_WriteRegister(REG_THRSHLD_LSB, ADC_CONFIG_THRSHLD_LSB); //disable DC-Block Filter
    //ADC_WriteRegister(REG_CH0_CFG, ADC_CONFIG_CH0_CFG); //set channel 0 input selection to positive DC test signal
    //sleep_ms(1);
    //ADC_WriteRegister(REG_CH1_CFG, ADC_CONFIG_CH1_CFG); //set channel 1 input selection to positive DC test signal
    //sleep_ms(1);
    //ADC_WriteRegister(REG_CH2_CFG, ADC_CONFIG_CH2_CFG); //set channel 2 input selection to positive DC test signal
    //sleep_ms(1);
    //ADC_WriteRegister(REG_CH3_CFG, ADC_CONFIG_CH3_CFG); //set channel 3 input selection to positive DC test signal
    sleep_ms(1);
    
    //Resync by pulsing shorter than t_W(RSL) (2048 * t_clkin = 2 ms) but longer than one clkin (950 ns @ 1,049 MHz) -> 200 us should be ok
    gpio_put(PIN_RST, 0);
    sleep_us(200);
    gpio_put(PIN_RST, 1);
    printf("Resync Done \n");
    //Reset by pulsing longer than t_W(RSL) (2048 * t_clkin = 2 ms) -> 150 ms should be ok
    gpio_put(PIN_RST, 0);
    sleep_ms(150);
    gpio_put(PIN_RST, 1);
    printf("Reset Done \n");
    
    gpio_put(PIN_CS, 0);
    ADC_SetCommand(COMMAND_UNLOCK);
    ADC_ExchangeData(len);
}

uint16_t ADC_ReadRegister(uint8_t reg)
{
    ADC_SetCommand(COMMAND_READ(reg, 1));
    ADC_ExchangeData(len);
    ADC_SetCommand(COMMAND_NULL);
    ADC_ExchangeData(len);    

    return ADC_GetResponse();
}

bool ADC_WriteRegister(uint8_t reg, uint16_t val)
{
    ADC_SetCommand(COMMAND_WRITE(reg, 1));
    ADC_SetRegisterValue(val);

    ADC_ExchangeData(len);

    return true;
}


bool ADC_CheckData(void)
{   
    if (gpio_get(PIN_DRDY) == true) {
        return false;
    }
    else {
    ADC_SetCommand(COMMAND_NULL);
    ADC_ExchangeData(len);
    ADC_ConvertResults();
    
    return true;
    }
}


int32_t ADC_GetRaw(size_t ch)
{
    return adc_data[ch];
}

void ADC_CollectData(int32_t* arr1, int32_t* arr2, int32_t* arr3, int32_t* arr4, int amount)
{   
    //printf("ADC_CollectData started \n");
    //Resync by pulsing shorter than t_W(RSL) (2048 * t_clkin = 2 ms) but longer than one clkin (950 ns @ 1,049 MHz) -> 200 us should be ok
    /*gpio_put(PIN_RST, 0);
    sleep_us(200);
    gpio_put(PIN_RST, 1);
    sleep_ms(100);
    gpio_put(PIN_RST, 0);
    sleep_ms(150);
    gpio_put(PIN_RST, 1);*/
    for (int i = 0; i < amount + 1; i++)
    {
        while (gpio_get(PIN_DRDY) == true) //&& x < 40000000 
        {
            //wait for new data
            //x++ ;
            sleep_ms(200);
        }
       
        ADC_SetCommand(COMMAND_NULL);
        ADC_ExchangeData(len);
        ADC_ConvertResults();
        //int32_t ph1 = ADC_GetRaw(0);
        int32_t ph2 = ADC_GetRaw(1);
        int32_t ph3 = ADC_GetRaw(2);
        int32_t ph4 = ADC_GetRaw(3);
        int32_t ph1 = ADC_GetRaw(0);
        ph1 = ADC_toVoltage(ph1);
        ph2 = ADC_toVoltage(ph2);
        ph3 = ADC_toVoltage(ph3);
        ph4 = ADC_toVoltage(ph4);
        printf("ph1: %d uV, ph2: %d uV, ph3: %d uV, ph4: %d uV;\n", ph1, ph2, ph3, ph4);
        if (i == amount){ //the first received date is thrown away
            arr1[0] = ph1;
            arr2[0] = ph2;
            arr3[0] = ph3;
            arr4[0] = ph4;
        }else{
            arr1[i] = ph1;
            arr2[i] = ph2;
            arr3[i] = ph3;
            arr4[i] = ph4;
        }     
    }
}

float ADC_toVoltage (int32_t adc_value){
    float voltage;
    //Check if the value is positive - between 000000h and 7FFFFFh
    if(adc_value <= MAX_ADC_VALUE){
        voltage = (adc_value / (float)MAX_ADC_VALUE) * MAX_VOLTAGE * 1000000;
    } else {
        //Convert negative value to positive and then calculate voltage. Negative values are from FFFFFFh to 800000h
        voltage = (((adc_value - (float)(MIN_NEG_ADC_VALUE)) / (float)(MAX_ADC_VALUE)) * MAX_VOLTAGE) * 1000000; //taking the real value  received from the ADC - negative numbers allowed
        //voltage = (((adc_value - (float)(MAX_ADC_VALUE)) / (float)(MAX_ADC_VALUE)) * MAX_VOLTAGE) * 1000000; //taking the abolute value - no negative numbers
    }
    return voltage;
}