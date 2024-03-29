/*
Diplomarbeit

Author: Tomas Valenta, Dipl.-Ing. Johannes Herold

Dieser Programmcode wurde im Rahmen einer Studienarbeit zum Thema Evaluierung einer Methode für das Bestimmen der relativen Entfernung mehrerer Sendeeinheiten erstellt.
Er ist für die Ausführung auf einem Raspberry Pi Pico W mit dem RP2040 Chip konzipiert. Der vollständige Code ist auch auf GitHub unter https://github.com/tomasvalentajr/Diplomarbeit.git verfügbar.
Für die Kompilierung dieses Programms wird empfohlen die voreingestellte Visual Studio Code-Umgebung von Raspberry Pi zu nutzen (https://www.raspberrypi.com/news/raspberry-pi-pico-windows-installer/).
*/


// Compiler headers
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "string.h"
#include "hardware/gpio.h"
#define LED_PIN 25

// Pico SDK headers
#include "pico/stdlib.h"
#include "hardware/timer.h"

// Project specific headers
#include "adc.h"
#include "fft_imp.h"

void getPosition(float* freq) {
    //calculate the relative position of the IR-sources from the relative amplitudes of their frequencies
    int binNum1 = 139; //Frequency bin of the first source
    int binNum2 = 212; //Frequency bin of the second source
    float amp1 = 0;
    float amp2 = 0;
    float distRatio = 0;

    amp1 = freq[binNum1];
    amp2 = freq[binNum2];
    distRatio = sqrt(amp2 / amp1);

    printf("l1/l2 =%f\n", distRatio);
}

void LED_ON() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
}

int main() {
    stdio_init_all();

    sleep_ms(2000);
    LED_ON();
    ADC_Init();
    //printf("adc_init() \n"); //ADC_Init done
    

    int32_t adcData1[N] = {0};   //Arrays for storing raw ADC Data
    int32_t adcData2[N] = {0};   
    int32_t adcData3[N] = {0};   
    int32_t adcData4[N] = {0};   
    float freqBins1[N] = {0};    //Arrays for storing the amplitudes of the frequency spectrum
    float freqBins2[N] = {0};    
    float freqBins3[N] = {0};        
    float freqBins4[N] = {0};        
    int distance = 100;
    int angle = 0;
    //ADC_MonitorData(); //for monitoring the ADC Values without 
    for(int q = 1; q < 11; q++) {
        ADC_CollectData(adcData1, adcData2, adcData3, adcData4, N); //collect ADC Values for FFT
        printf("Versuch %d: Abstand %d mm, Winkel %d deg", q, distance, angle);
        printf("ADC Data\n");
        printf("Sensor 1\n");
        for(int x = 0; x < N; x++) {
            printf("%d\n", adcData1[x]);
        };
        printf("Sensor 2\n");
        for(int x = 0; x < N; x++) {
            printf("%d\n", adcData2[x]);
        };
        printf("Sensor 3\n");
        for(int x = 0; x < N; x++) {
            printf("%d\n", adcData3[x]);
        };
        printf("Sensor 4\n");
        for(int x = 0; x < N; x++) {
            printf("%d\n", adcData4[x]);
        };
        fft(adcData1, adcData2, adcData3, adcData4, freqBins1, freqBins2, freqBins3, freqBins4); //calculate FFT for the ADC values
        printf("Frequenzbereich 1\n");
        for(int x = 0; x < N; x++) {
            printf("%.2f\n", freqBins1[x]);
        };
        printf("Frequenzbereich 2\n");
        for(int x = 0; x < N; x++) {
            printf("%.5f\n", freqBins2[x]);
        };
        printf("Frequenzbereich 3\n");
        for(int x = 0; x < N; x++) {
            printf("%f\n", freqBins3[x]);
        };
        printf("Frequenzbereich 4\n");
        for(int x = 0; x < N; x++) {
            printf("%f\n", freqBins4[x]);
        };
        int binNum = 139; //Frequency bin of the IR-source
        printf("Amplituden der Frequenzbins\n");
        printf("%f\n", freqBins1[binNum]);
        printf("%f\n", freqBins2[binNum]);
        printf("%f\n", freqBins3[binNum]);
        printf("%f\n", freqBins4[binNum]);

        sleep_ms(150); //wait 150ms before making a new measurement
    }
    printf("10 Meassurements were collected\n"); //sign, that 10 FFTs have been alculated
        
}