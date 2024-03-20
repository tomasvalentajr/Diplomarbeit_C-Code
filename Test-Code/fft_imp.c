#include "fft_imp.h"

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "kiss_fft/kiss_fft.h"



int fft(int32_t* datapoints1, int32_t* datapoints2, int32_t* datapoints3, int32_t* datapoints4, float* frequencySpectrum1, float* frequencySpectrum2, float* frequencySpectrum3, float* frequencySpectrum4) {
    // Create an array for your 24-bit data points and Populate the data array with your 24-bit numbers (sample values)
    //printf("fft() started \n");
    int32_t data1[N] = {0};
    int32_t data2[N] = {0};
    int32_t data3[N] = {0};
    int32_t data4[N] = {0};
    for (int i = 0; i < N; i++) {
        data1[i] = datapoints1[i];
        data2[i] = datapoints2[i];
        data3[i] = datapoints3[i];
        data4[i] = datapoints4[i];
    }

     // Create kissfft configuration - N Samples, non-inverted FFT
    kiss_fft_cfg cfg = kiss_fft_alloc(N, 0, NULL, NULL);
    
    // Convert the 24-bit data to 32-bit for kissfft
    kiss_fft_cpx fft_in1[N] = {0};
    for (int i = 0; i < N; i++) {
        fft_in1[i].r = (kiss_fft_scalar)data1[i];
        fft_in1[i].i = 0.0;
    }
    // Perform the FFT
    kiss_fft_cpx fft_out1[N] = {0};
    kiss_fft(cfg, fft_in1, fft_out1);
    // Clean up
    //kiss_fft_free(cfg);
    //Save the magnitudes into an array for calculating the position
    for (int i = 0; i < N; i++) {
        frequencySpectrum1[i] = sqrt(fft_out1[i].r * fft_out1[i].r + fft_out1[i].i * fft_out1[i].i);
    }

    kiss_fft_cpx fft_in2[N] = {0};
    for (int i = 0; i < N; i++) {
        fft_in2[i].r = (kiss_fft_scalar)data2[i];
        fft_in2[i].i = 0.0;
    }
    // Perform the FFT
    kiss_fft_cpx fft_out2[N] = {0};
    kiss_fft(cfg, fft_in2, fft_out2);
    // Clean up
    //kiss_fft_free(cfg);
    //Save the magnitudes into an array for calculating the position
    for (int i = 0; i < N; i++) {
        frequencySpectrum2[i] = sqrt(fft_out2[i].r * fft_out2[i].r + fft_out2[i].i * fft_out2[i].i);
    }

    kiss_fft_cpx fft_in3[N] = {0};
    for (int i = 0; i < N; i++) {
        fft_in3[i].r = (kiss_fft_scalar)data3[i];
        fft_in3[i].i = 0.0;
    }
    // Perform the FFT
    kiss_fft_cpx fft_out3[N] = {0};
    kiss_fft(cfg, fft_in3, fft_out3);
    // Clean up
    //kiss_fft_free(cfg);
    //Save the magnitudes into an array for calculating the position
    for (int i = 0; i < N; i++) {
        frequencySpectrum3[i] = sqrt(fft_out3[i].r * fft_out3[i].r + fft_out3[i].i * fft_out3[i].i);
    }

    kiss_fft_cpx fft_in4[N] = {0};
    for (int i = 0; i < N; i++) {
        fft_in4[i].r = (kiss_fft_scalar)data4[i];
        fft_in4[i].i = 0.0;
    }
    // Perform the FFT
    kiss_fft_cpx fft_out4[N] = {0};
    kiss_fft(cfg, fft_in4, fft_out4);
    // Clean up
    kiss_fft_free(cfg);
    //Save the magnitudes into an array for calculating the position
    for (int i = 0; i < N; i++) {
        frequencySpectrum4[i] = sqrt(fft_out4[i].r * fft_out4[i].r + fft_out4[i].i * fft_out4[i].i);
    }

    /*for (int i = 0; i < N; i++) {
        float magnitude = sqrt(fft_out[i].r * fft_out[i].r + fft_out[i].i * fft_out[i].i);
        if (i == 139)
        {
            printf("%f", magnitude);
            printf(" | ");
        } else if (i == 212)
        {
            printf("%f\n", magnitude);
        }
    }*/
    
    //8388608 values for 23-Bits - 2^23 
    
    //prints used ADC Values
    /*for (int i = 0; i < N; i++) {
        printf("%d\n", data[i]);
    }*/

    return 0;
}