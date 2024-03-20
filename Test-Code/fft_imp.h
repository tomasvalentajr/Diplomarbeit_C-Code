#ifndef FFT_IMP_H
#define FFT_IMP_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "kiss_fft/kiss_fft.h"

#define N 1024 // Number of data points

// Function to calculate FFT results
int fft(int32_t* datapoints1, int32_t* datapoints2, int32_t* datapoints3, int32_t* datapoints4, float* frequencySpectrum1, float* frequencySpectrum2, float* frequencySpectrum3, float* frequencySpectrum4);

#endif // FFT_IMP_H