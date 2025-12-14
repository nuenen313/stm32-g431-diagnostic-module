/*
 * fft_util.c
 *
 *  Created on: Dec 3, 2025
 *      Author: Marta
 */
#include "fft_util.h"
#include "fft.h"
#include "app_fatfs.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/* External buffer from main.c */
float signal_input[N_SAMPLES + 2];
uint32_t fft_count = 0;

/* --------------------------------------------------------------------------
 * SD LOAD 4096 — reads the first 2048 samples from file
 * --------------------------------------------------------------------------*/
void SD_LoadFFT_4096(const char *filename)
{
    FATFS fs;
    FIL fp;
    char line[128];
    FRESULT res;
    DSTATUS stat;

    fft_count = 0;

    stat = disk_initialize(0);
    if (stat != 0) {
        return;
    }

    HAL_Delay(100);

    res = f_open(&fp, filename, FA_READ);
    if (res != FR_OK) {
        f_mount(NULL, "", 0);
        return;
    }

    while (fft_count < N_SAMPLES && f_gets(line, sizeof(line), &fp))
    {
        char *comma = strchr(line, ',');
        if (comma) *comma = '\0';

        float value = atof(line);
        signal_input[fft_count++] = value;
    }

    f_close(&fp);
    f_mount(NULL, "", 0);
}

/* --------------------------------------------------------------------------
 * Save FFT normalized magnitudes
 * --------------------------------------------------------------------------*/
void SD_SaveFFT(const char *filename, float max_val)
{
    FATFS FatFs;
    FIL fil;
    FRESULT res;
    UINT written;
    char buf[64];

    if (disk_initialize(0) != 0) {
        return;
    }

    if (f_mount(&FatFs, "", 1) != FR_OK) {
        return;
    }

    res = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        return;
    }

    for (int k = 0; k < N_SAMPLES/2; k++)
    {
        float freq = (FS /(float) N_SAMPLES) * k;
        float re = signal_input[2*k];
        float im = signal_input[2*k + 1];
        float mag = sqrtf(re*re + im*im);

        if (max_val > 0) mag /= max_val;

        snprintf(buf, sizeof(buf), "%.4f, %.6f\r\n", freq, mag);
        f_write(&fil, buf, strlen(buf), &written);
    }

    f_close(&fil);
    f_mount(NULL, "", 0);
}

/* --------------------------------------------------------------------------
 * Save raw FFT array
 * --------------------------------------------------------------------------*/
void SD_SaveFullFFT(const char *filename, float *data, uint32_t N)
{
    FATFS FatFs;
    FIL fil;
    FRESULT res;
    UINT written;
    char buf[48];

    if (disk_initialize(0) != 0) {
        return;
    }

    if (f_mount(&FatFs, "", 1) != FR_OK) {
        return;
    }

    res = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        return;
    }

    for (uint32_t i = 0; i < N + 2; i++)
    {
        int len = snprintf(buf, sizeof(buf),
                           "%lu, %.7f\r\n", (unsigned long)i, data[i]);
        f_write(&fil, buf, len, &written);
    }

    f_close(&fil);
    f_mount(NULL, "", 0);
}

/* --------------------------------------------------------------------------
 * FFT + peak detection + saving
 * --------------------------------------------------------------------------*/
void compute_fft_and_print(const char *output_filename, const char *raw_filename)
{

    realft(signal_input, N_SAMPLES, 1);

    float max1 = 0, max2 = 0;
    int idx1 = 0, idx2 = 0;

    for (int k = 1; k < N_SAMPLES/2; k++)
    {
        float re = signal_input[2*k];
        float im = signal_input[2*k+1];
        float mag = sqrtf(re*re + im*im);

        if (mag > max1) {
            max2 = max1; idx2 = idx1;
            max1 = mag; idx1 = k;
        } else if (mag > max2) {
            max2 = mag; idx2 = k;
        }
    }

    SD_SaveFFT(output_filename, max1);
    SD_SaveFullFFT(raw_filename, signal_input, N_SAMPLES);
}

void SD_SaveMicBuffer(const char *filename, int16_t *mic_data, uint32_t num_samples)
{
    FATFS FatFs;
    FIL fil;
    FRESULT res;
    UINT written;
    char buf[32];

    if (disk_initialize(0) != 0) {
        return;
    }

    if (f_mount(&FatFs, "", 1) != FR_OK) {
        return;
    }

    res = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        f_mount(NULL, "", 0);
        return;
    }

    // Write header (optional)
    snprintf(buf, sizeof(buf), "Sample, Value\r\n");
    f_write(&fil, buf, strlen(buf), &written);

    // Write microphone samples
    for (uint32_t i = 0, n = 0; i < num_samples; i += 2, n++) {
        int16_t s = mic_data[i];
        int len = snprintf(buf, sizeof(buf), "%lu,%d\r\n",
                           (unsigned long)n, (int)s);
        f_write(&fil, buf, len, &written);
    }

    f_close(&fil);
    f_mount(NULL, "", 0);
}

void SD_SaveSignal(const char *filename, float *data, uint32_t n)
{
    FATFS fs;
    FIL fil;
    FRESULT res;
    UINT written;
    char buf[48];

    if (disk_initialize(0) != 0) return;
    if (f_mount(&fs, "", 1) != FR_OK) return;

    res = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        f_mount(NULL, "", 0);
        return;
    }

    f_write(&fil, "sample,value\r\n", 15, &written);

    for (uint32_t i = 0; i < n; i++) {
        int len = snprintf(buf, sizeof(buf),
                           "%lu,%.6f\r\n",
                           (unsigned long)i,
                           data[i]);
        f_write(&fil, buf, len, &written);
    }

    f_close(&fil);
    f_mount(NULL, "", 0);
}

