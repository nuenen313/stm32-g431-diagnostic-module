/*
 * fft.h
 *
 *  Created on: Oct 25, 2025
 *      Author: Marta
 */

#ifndef INC_FFT_H_
#define INC_FFT_H_

#include <math.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Performs complex Fast Fourier Transform (in-place)
 *
 * @param data   Array of 2*N floats: [Re0, Im0, Re1, Im1, ...]
 * @param nn     Number of complex samples (N)
 * @param isign  +1 for forward transform, -1 for inverse
 */
void four1(float data[], uint32_t nn, int isign);

/**
 * @brief Performs real-valued FFT (optimized for real input)
 *
 * @param data   Array of N floats (real-valued signal)
 * @param n      Number of real samples (must be power of 2)
 * @param isign  +1 for forward, -1 for inverse
 */
void realft(float data[], uint32_t n, int isign);

#ifdef __cplusplus
}
#endif


#endif /* INC_FFT_H_ */
