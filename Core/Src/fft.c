/*
 * fft.c
 *
 *  Created on: Oct 25, 2025
 *      Author: Marta
 *      usage: realft(signal, N, 1);
 *      for (int k = 1; k < N/2; k++) {
        float re = signal[2*k];
        float im = signal[2*k + 1];
        magnitude[k] = sqrtf(re*re + im*im);
    }
 */

#include "fft.h"
#include <math.h>

void four1(float data[], uint32_t nn, int isign){
	uint32_t n = nn << 1;
	uint32_t j = 0;
	for (uint32_t i = 0; i < n; i += 2){
		if (j > 1){
			float tempr = data[j];
			data[i] = tempr;

			tempr = data[j + 1];
			data[j + 1] = data[i + 1];
			data[i + 1] = tempr;
		}
		uint32_t m = nn;
		while (m >= 2 && j >= m){
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	uint32_t mmax = 2;
	while (n > mmax){
		uint32_t istep = mmax << 1;
		double theta = isign * (2.0 * M_PI / (double)mmax);
		double wtemp = sin(0.5 * theta);
		double wpr = -2.0 * wtemp * wtemp;
		double wpi = sin(theta);
		double wr = 1.0;
		double wi = 0.0;

		for (uint32_t m = 0; m < mmax; m += 2){
			for (uint32_t i = m; i < n; i += istep){
				uint32_t j = i + mmax;
				float tempr = wr * data[j] - wi * data[j + 1];
				float tempi = wr * data[j + 1] + wi * data[j];
				data[j] = data[i] - tempr;
				data[j+1] = data[i + 1] - tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			double wtemp2 = wr;
			wr = wtemp2 * wpr - wi * wpi + wr;
			wi = wi * wpr + wtemp2 * wpi + wi;
		}
		mmax = istep;
	}
}

/* Real FFT: wraps four1() for real-valued input */
void realft(float data[], uint32_t n, int isign){
	double theta = M_PI /(double)(n >> 1);
	double wtemp = sin(0.5 * theta);
	double wpr = -2.0 * wtemp * wtemp;
	double wpi = sin(theta);
	double wr = 1.0 + wpr;
	double wi = wpi;

	float c1 = 0.5;
	float c2;
	if (isign == 1){
		c2 = -0.5f;
		four1(data, n >> 1, 1); //perform forward fft
	}
	else {
		c2 = 0.5f;
		theta = -theta;
		four1(data, n >> 1, -1); //perform inverse fft
	}
	for (uint32_t i = 1; i < (n >> 2); i++){
		uint32_t i1 = 2 * i;
		uint32_t i2 = i1 + 1;
		uint32_t i3 = n - i1;
		uint32_t i4 = i3 + 1;

		float h1r = c1 * (data[i1] + data[i3]);
		float h1i = c1 * (data[i2] - data[i4]);
		float h2r = -c2 * (data[i2] + data[i4]);
		float h2i = c2 * (data[i1] - data[i3]);

		data[i1] = h1r + (float)(wr * h2r - wi * h2i);
		data[i2] = h1i + (float)(wr * h2i + wi * h2r);
		data[i3] = h1r - (float)(wr * h2r - wi * h2i);
		data[i4] = -h1i + (float)(wr * h2i + wi * h2r);

		double wtemp2 = wr;
		wr = wtemp2 * wpr - wi * wpi + wr;
		wi = wi * wpr + wtemp2 * wpi + wi;
	}
	if (isign == 1){
		float tmp = data[0];
		data[0] = tmp + data[1];
		data[1] = tmp - data[1];
	}
	else {
		float tmp = data[0];
		data[0] = 0.5f * (tmp + data[1]);
		data[1] = 0.5f * (tmp - data[1]);
	}
}
