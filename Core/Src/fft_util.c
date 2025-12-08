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
    char msg2[64];
    snprintf(msg2, sizeof(msg2), "disk_initialize returned: %d\r\n", stat);
//    send_uart(msg2);

    if (stat != 0) {
//        send_uart("Disk init failed! Check connections.\r\n");
        return;
    }

    HAL_Delay(100);

    res = f_mount(&fs, "", 1);
    snprintf(msg2, sizeof(msg2), "f_mount returned: %d\r\n", res);
//    send_uart(msg2);

//    if (res != FR_OK) {
//        if (res == FR_NO_FILESYSTEM) send_uart("No FAT32 filesystem\r\n");
//        else if (res == FR_DISK_ERR) send_uart("Disk error\r\n");
//        else if (res == FR_NOT_READY) send_uart("Disk not ready\r\n");
//        else send_uart("Mount failed\r\n");
//        return;
//    }

    res = f_open(&fp, filename, FA_READ);
    if (res != FR_OK) {
        snprintf(msg2, sizeof(msg2), "f_open failed: %d\r\n", res);
//        send_uart(msg2);
//        if (res == FR_NO_FILE) send_uart("File not found\r\n");
        f_mount(NULL, "", 0);
        return;
    }

//    send_uart("File opened successfully!\r\n");

    while (fft_count < N_SAMPLES && f_gets(line, sizeof(line), &fp))
    {
        char *comma = strchr(line, ',');
        if (comma) *comma = '\0';

        float value = atof(line);
        signal_input[fft_count++] = value;

        char dbg[64];
        snprintf(dbg, sizeof(dbg),
                 "sample[%lu] = %f\r\n",
                 (unsigned long)(fft_count - 1), value);
//        send_uart(dbg);
    }

    f_close(&fp);
    f_mount(NULL, "", 0);

    char msg[64];
    snprintf(msg, sizeof(msg), "Loaded %lu samples.\r\n", (unsigned long)fft_count);
//    send_uart(msg);
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

//    send_uart("Saving FFT...\r\n");

    if (disk_initialize(0) != 0) {
//        send_uart("disk init failed!\r\n");
        return;
    }

    if (f_mount(&FatFs, "", 1) != FR_OK) {
//        send_uart("mount failed!\r\n");
        return;
    }

    res = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
//        send_uart("open failed!\r\n");
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

//    send_uart("FFT saved.\r\n");
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

//    send_uart("Saving full FFT...\r\n");

    if (disk_initialize(0) != 0) {
//        send_uart("disk init failed!\r\n");
        return;
    }

    if (f_mount(&FatFs, "", 1) != FR_OK) {
//        send_uart("mount failed!\r\n");
        return;
    }

    res = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
//        send_uart("open failed!\r\n");
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

//    send_uart("Full FFT saved.\r\n");
}

/* --------------------------------------------------------------------------
 * FFT + peak detection + saving
 * --------------------------------------------------------------------------*/
void compute_fft_and_print(void)
{
//    char msg[128];

    realft(signal_input, N_SAMPLES, 1);

    float max1 = 0, max2 = 0;
//    int idx1 = 0, idx2 = 0;

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

//    float freq1 = (FS / (float)N_SAMPLES) * idx1;
//    float freq2 = (FS / (float)N_SAMPLES) * idx2;

//    snprintf(msg, sizeof(msg),
//             "Peak1: %.1f Hz (%.3f)\r\nPeak2: %.1f Hz (%.3f)\r\n",
//             freq1, max1, freq2, max2);
//    send_uart(msg);

    SD_SaveFFT("fft_output_08_12_01.txt", max1);
    SD_SaveFullFFT("fft_raw_08_12_01.txt", signal_input, N_SAMPLES);
}
