/*
 * fft_util.h
 *
 *  Created on: Dec 3, 2025
 *      Author: Marta
 */

#ifndef INC_FFT_UTIL_H_
#define INC_FFT_UTIL_H_

#include <stdint.h>

#define MIC_BUF_LEN 512
#define N_SAMPLES 2048
#define FS 1000

extern float signal_input[N_SAMPLES + 2];
extern uint32_t fft_count;

void SD_LoadFFT_4096(const char *filename);
void SD_SaveFFT(const char *filename, float max_val);
void SD_SaveFullFFT(const char *filename, float *data, uint32_t N);
void compute_fft_and_print(void);

void send_uart(const char *msg);


#endif /* INC_FFT_UTIL_H_ */
