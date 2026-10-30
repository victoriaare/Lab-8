/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include <stdio.h>
#include <stdint.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
static void Oxy_Pixel(int16_t x, int16_t y, uint16_t color);
static void Oxy_Line(int16_t x0, int16_t y0,int16_t x1, int16_t y1,uint16_t color);
static int16_t Oxy_Amplitude(uint32_t phase);
static void Draw_Heart(uint16_t x, uint16_t y);
static void Draw_Oximeter(uint16_t spo2, uint16_t pulse);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define OXY_WIDTH  LCD_W
#define OXY_HEIGHT LCD_H

static void Oxy_Pixel(int16_t x, int16_t y, uint16_t color)
{
    if (x >= 0 && y >= 0 &&
        x < OXY_WIDTH && y < OXY_HEIGHT)
    {
        ILI9341_DrawPixel(
            (uint16_t)x,
            (uint16_t)y,
            color
        );
    }
}

static void Oxy_Line(int16_t x0, int16_t y0,
                     int16_t x1, int16_t y1,
                     uint16_t color)
{
    int32_t x = x0;
    int32_t y = y0;

    int32_t dx = (int32_t)x1 - x0;
    int32_t dy = (int32_t)y1 - y0;

    int32_t sx = dx >= 0 ? 1 : -1;
    int32_t sy = dy >= 0 ? 1 : -1;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    int32_t error = dx - dy;

    for (;;)
    {
        Oxy_Pixel((int16_t)x, (int16_t)y, color);

        if (x == x1 && y == y1)
            break;

        int32_t twice = 2 * error;

        if (twice > -dy)
        {
            error -= dy;
            x += sx;
        }

        if (twice < dx)
        {
            error += dx;
            y += sy;
        }
    }
}

static int16_t Oxy_Amplitude(uint32_t phase)
{
    static const uint8_t t[] = {
        0, 10, 16, 22, 28, 36,
        43, 50, 59, 76, 90, 100
    };

    static const uint8_t a[] = {
        0, 1, 35, 95, 100, 72,
        52, 60, 36, 15, 3, 0
    };

    for (uint32_t i = 1; i < sizeof(t); ++i)
    {
        if (phase <= t[i])
        {
            return (int16_t)(
                a[i - 1] +
                ((int32_t)a[i] - a[i - 1]) *
                ((int32_t)phase - t[i - 1]) /
                (t[i] - t[i - 1])
            );
        }
    }

    return 0;
}

static void Draw_Heart(uint16_t x, uint16_t y)
{
    static const uint16_t heart[16] = {
        0x0000,
        0x1C38,
        0x3E7C,
        0x7FFE,
        0xFFFF,
        0xFFFF,
        0xFFFF,
        0x7FFE,
        0x3FFC,
        0x1FF8,
        0x0FF0,
        0x07E0,
        0x03C0,
        0x0180,
        0x0000,
        0x0000
    };

    uint16_t rojo = RGB565(255, 75, 100);

    for (uint16_t fila = 0; fila < 16; fila++)
    {
        for (uint16_t columna = 0; columna < 16; columna++)
        {
            if (heart[fila] & (0x8000U >> columna))
            {
                ILI9341_FillRect(
                    x + columna * 3,
                    y + fila * 3,
                    3, 3,
                    rojo
                );
            }
        }
    }
}

static void Draw_Oximeter(uint16_t spo2, uint16_t pulse)
{
    char value[8];

    const uint16_t bg =
        RGB565(8, 15, 27);

    const uint16_t header =
        RGB565(16, 29, 48);

    const uint16_t tile =
        RGB565(14, 27, 43);

    const uint16_t border =
        RGB565(37, 57, 74);

    const uint16_t white =
        RGB565(235, 245, 250);

    const uint16_t muted =
        RGB565(148, 174, 192);

    const uint16_t cyan =
        RGB565(52, 216, 220);

    const uint16_t green =
        RGB565(102, 234, 151);

    const uint16_t grid =
        RGB565(26, 43, 57);

    ILI9341_FillScreen(bg);

    ILI9341_FillRect(0, 0, 320, 44, header);
    ILI9341_FillRect(0, 43, 320, 2, cyan);
    ILI9341_DrawString(
            12, 8, "OXIMETRO",
            2, white, header
        );

        ILI9341_DrawString(
            12, 28, "DE PULSO",
            1, muted, header
        );

        ILI9341_FillRect(
            247, 14, 65, 17, border
        );

        ILI9341_FillRect(
               8, 53, 108, 78, tile
           );

           ILI9341_DrawRoundRect(
               8, 53, 108, 78,
               1, border
           );

           ILI9341_FillRect(
               8, 53, 3, 78, cyan
           );

           ILI9341_DrawString(
               20, 61, "SPO2",
               1, muted, tile
           );

           snprintf(
               value,
               sizeof(value),
               "%u",
               (unsigned)spo2
           );

           ILI9341_DrawString(
               20, 78, value,
               3, cyan, tile
           );

           ILI9341_DrawString(
               77, 94, "%",
               2, cyan, tile
           );

           ILI9341_DrawString(
               20, 116, "SATURACION",
               1, muted, tile
           );

           ILI9341_DrawRoundRect(
                  124, 53, 108, 78,
                  1, border
              );

              ILI9341_FillRect(
                  124, 53, 3, 78, green
              );

              ILI9341_DrawString(
                  136, 61, "PULSO",
                  1, muted, tile
              );

              snprintf(
                  value,
                  sizeof(value),
                  "%u",
                  (unsigned)pulse
              );

              ILI9341_DrawString(
                  136, 78, value,
                  3, green, tile
              );

              ILI9341_DrawString(
                  193, 99, "LPM",
                  1, green, tile
              );

              ILI9341_DrawString(
                  136, 116, "CARDIACA",
                  1, muted, tile
              );

              ILI9341_FillRect(
                      242, 55, 66, 73, header
                  );

                  Draw_Heart(251, 65);

                  ILI9341_DrawString(
                      251, 116, "HEART",
                      1, muted, header
                  );

                  ILI9341_FillRect(
                        8, 139, 304, 77, tile
                    );

                    ILI9341_DrawRoundRect(
                        8, 139, 304, 77,
                        1, border
                    );

                    ILI9341_DrawString(
                        16, 145, "ONDA PLETISMOGRAFICA",
                        1, muted, tile
                    );

                    for (uint16_t x = 16; x <= 304; x += 24)
                       {
                           ILI9341_FillRect(
                               x, 160, 1, 44, grid
                           );
                       }
                    for (uint16_t y = 160; y <= 202; y += 14)
                        {
                            ILI9341_FillRect(
                                16, y, 289, 1, grid
                            );
                        }

                    int16_t previous_y = 202;

                        for (int16_t i = 1; i <= 288; ++i)
                        {
                            uint32_t phase =
                                ((uint32_t)i * pulse * 5U / 216U) % 100U;

                            int16_t y = (int16_t)(
                                202 - Oxy_Amplitude(phase) * 35 / 100
                            );

                            Oxy_Line(
                                (int16_t)(15 + i),
                                previous_y,
                                (int16_t)(16 + i),
                                y,
                                cyan
                            );

                            Oxy_Line(
                                (int16_t)(15 + i),
                                (int16_t)(previous_y + 1),
                                (int16_t)(16 + i),
                                (int16_t)(y + 1),
                                cyan
                            );

                            previous_y = y;
                        }

                        // TIEMPO DE LA GRAFICA

                        ILI9341_DrawString(
                            16, 203, "0 S",
                            1, muted, tile
                        );

                        ILI9341_DrawString(
                            286, 203, "4 S",
                            1, muted, tile
                        );
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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  ILI9341_Init();
  Draw_Oximeter(98, 85);
  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED2);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : TFT_RST_Pin */
  GPIO_InitStruct.Pin = TFT_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TFT_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : TFT_CS_Pin */
  GPIO_InitStruct.Pin = TFT_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TFT_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TFT_DC_Pin */
  GPIO_InitStruct.Pin = TFT_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TFT_DC_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
