/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fft.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MIC_BUF_LEN 512
#define N_SAMPLES 2048
#define FFT_SAMPLES 2048
#define FS 1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s2;
DMA_HandleTypeDef hdma_spi2_rx;

SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN PV */
char TxBuffer[250];
UART_HandleTypeDef huart2;
int toReceive = 0;
int32_t micBuffer[MIC_BUF_LEN];
volatile uint8_t micHalfDataReady = 0;
volatile uint8_t micFullDataReady = 0;
uint32_t fft_count = 0;
float signal_input[N_SAMPLES + 2];
float magnitude[N_SAMPLES/2];
static float data[2048 + 2];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2S2_Init(void);
static void MX_SPI3_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

void compute_fft_and_print(void);
static void SD_LoadFFT_4096(const char *filename);
static void SD_SaveFFT(const char *filename);
static void MX_LPUART1_UART_Init(void);
int _write(int file, char *ptr, int len);
void send_uart(const char *msg);
void NR_to_MATLAB_scale(float signal[], float mag[], uint32_t N);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
void send_uart(const char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S2_Init();
  MX_SPI3_Init();
  if (MX_FATFS_Init() != APP_OK) {
    Error_Handler();
  }
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  MX_LPUART1_UART_Init();
  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)micBuffer, MIC_BUF_LEN);
  //SD_Card_Test();
  //SD_ReadTextFile("nowey.txt");
  memset(signal_input, 0, sizeof(signal_input));
  fft_count = 0;
  SD_LoadFFT_4096("zniszczoney_v4.txt");
  compute_fft_and_print();

  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
//	  if (micHalfDataReady){
//	  		  micHalfDataReady = 0;
//	  	      for (int i = 0; i < MIC_BUF_LEN/2; i++)
//	  	      {
//	  	    	printf("%ld ", micBuffer[i]);
//	  	      }
//	  	      printf("\r\n");
//	  	  }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x40B285C2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2S2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B_EXTENDED;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_16K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */

  /* USER CODE END I2S2_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	micHalfDataReady = 1;
}
static void MX_LPUART1_UART_Init(void)
{
    huart2.Instance = LPUART1;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
}

static void SD_LoadFFT_4096(const char *filename)
{
	FATFS fs;
	    FIL fp;
	    char line[128];

	    fft_count = 0;

	    // Mount card
	    if (f_mount(&fs, "", 1) != FR_OK) {
	        send_uart("mount failed\r\n");
	        return;
	    }

	    // Open file
	    if (f_open(&fp, filename, FA_READ) != FR_OK) {
	        send_uart("open failed\r\n");
	        return;
	    }

	    send_uart("Reading FIRST 2048 samples...\r\n");

	    // Read FIRST 2048 rows
	    while (fft_count < FFT_SAMPLES && f_gets(line, sizeof(line), &fp))
	    {
	        // Find comma → keep only first column
	        char *comma = strchr(line, ',');
	        if (comma) *comma = '\0';

	        // Convert to float
	        float value = atof(line);
	        signal_input[fft_count++] = value;

	        // Debug print each sample
	        char dbg[64];
	        snprintf(dbg, sizeof(dbg),
	                 "sample[%lu] = %f\r\n",
	                 (unsigned long)(fft_count - 1), value);
	        send_uart(dbg);
	    }

	    f_close(&fp);
	    f_mount(NULL, "", 0);

	    char msg[64];
	    snprintf(msg, sizeof(msg),
	             "Loaded %lu samples.\r\n",
	             (unsigned long)fft_count);
	    send_uart(msg);
}

static void SD_SaveFFT(const char *filename)
{
    FATFS FatFs;
    FIL fil;
    FRESULT res;
    UINT written;
    char buf[64];
    send_uart("Entering SD_SaveFFT...\r\n");

    if (disk_initialize(0) != 0) {
    	send_uart("disk init failed!\r\n");
        return;
    }

    if (f_mount(&FatFs, "", 1) != FR_OK) {
    	send_uart("mount failed!\r\n");
        return;
    }

    res = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
    	send_uart("open failed (%d)\r\n");
        return;
    }

    for (int k = 0; k < N_SAMPLES/2; k++) {
        float freq = (FS / (float)N_SAMPLES) * k;
        snprintf(buf, sizeof(buf), "%.4f, %.6f\r\n", freq, magnitude[k]);
        f_write(&fil, buf, strlen(buf), &written);
    }

    f_close(&fil);
    f_mount(NULL, "", 0);

    printf("FFT saved to %s\r\n", filename);
}

void compute_magnitude(float signal[], float mag[], uint32_t N)
{
	float scale = 1.0f / (float)N;

	    // DC bin (k = 0), no factor 2
	    mag[0] = fabsf(signal[0]) * scale;

	    // Bins 1 .. N/2 - 1 : multiply by 2 for single-sided spectrum
	    for (uint32_t k = 1; k < N/2; k++)
	    {
	        float re = signal[2*k];
	        float im = signal[2*k + 1];
	        float m = sqrtf(re*re + im*im);
	        mag[k] = 2.0f * m * scale;
	    }

}

static void SD_SaveFullFFT(const char *filename, float *data, uint32_t N)
{
    FATFS FatFs;
    FIL fil;
    FRESULT res;
    UINT written;
    char buf[48];

    send_uart("Saving full FFT array...\r\n");

    if (disk_initialize(0) != 0) {
        send_uart("disk init failed!\r\n");
        return;
    }

    if (f_mount(&FatFs, "", 1) != FR_OK) {
        send_uart("mount failed!\r\n");
        return;
    }

    res = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        send_uart("open failed!\r\n");
        return;
    }

    for (uint32_t i = 0; i < N + 2; i++)
    {
        // save index and value
        int len = snprintf(buf, sizeof(buf), "%lu, %.7f\r\n",
                           (unsigned long)i, signal_input[i]);
        f_write(&fil, buf, len, &written);
    }

    f_close(&fil);
    f_mount(NULL, "", 0);

    send_uart("Full FFT array saved.\r\n");
}

void compute_fft_and_print(void)
{
    char msg[128];

//    for (int i = 0; i < N_SAMPLES; i++)
//        data[i+1] = signal_input[i];

    realft(signal_input, N_SAMPLES, 1);
    for (int k = 1; k < N_SAMPLES/2; k++) {
        float re = signal_input[2*k];
        float im = signal_input[2*k + 1];
        magnitude[k] = sqrtf(re*re + im*im);
    }

    // Peaks
    int start_bin = 1;   // skips 0–4 (≈0–2 Hz), adjust if needed

    float max1 = 0.0f, max2 = 0.0f;
    int idx1 = 0, idx2 = 0;

    for (int k = start_bin; k < N_SAMPLES/2; k++)
    {
        if (magnitude[k] > max1) {
            max2 = max1; idx2 = idx1;
            max1 = magnitude[k]; idx1 = k;
        }
        else if (magnitude[k] > max2) {
            max2 = magnitude[k]; idx2 = k;
        }
    }
    // Normalize so the largest peak = 1
    if (max1 > 0.0f) {
        for (int k = 0; k < N_SAMPLES/2; k++) {
            magnitude[k] /= max1;
        }
    }

    float freq1 = ((float)FS / (float)N_SAMPLES) * idx1;
    float freq2 = ((float)FS / (float)N_SAMPLES) * idx2;

    snprintf(msg, sizeof(msg),
        "Peak1: %.1f Hz (%.3f)\r\nPeak2: %.1f Hz (%.3f)\r\n",
        freq1, max1/max1, freq2, max2/max1
    );
    send_uart(msg);

    // Save FFT to SD
    SD_SaveFFT("fft_output_3.txt");
    SD_SaveFullFFT("fft_full_raw.txt", signal_input, N_SAMPLES);
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
