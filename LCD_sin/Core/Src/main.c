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
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include "lcd.h"
#include <math.h>

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static void draw_sine_wave_time(float time_offset, uint16_t color);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_FSMC_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  lcd_init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    lcd_display_on();  /* 开启LCD显示 */
    
    /* 主循环：持续绘制动态正弦波 */
    while (1)
    {
      /* 获取当前运行时间（秒） */
      float time_s = (float)HAL_GetTick() * 0.001f; /* 毫秒转换为秒 */
      
      /* 每200ms切换一次波形颜色（红色和蓝色交替） */
      uint16_t color = (((uint32_t)(HAL_GetTick() / 200)) & 1) ? RED : BLUE;
      
      /* 绘制正弦波 */
      draw_sine_wave_time(time_s, color);
      
      /* 延时40ms，控制刷新率约为25FPS */
      HAL_Delay(40);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
 * @brief LCD显示参数配置
 * 
 * LCD_WIDTH: 屏幕宽度（像素）
 * LCD_HEIGHT: 屏幕高度（像素）
 * TIME_WINDOW_S: 显示的时间窗口大小（秒）
 * VOLTAGE_RANGE_V: 电压显示范围（伏特）
 * WAVE_PERIOD_S: 正弦波周期（秒）
 * WAVE_AMPLITUDE_V: 正弦波幅度（伏特）
 * TOP_MARGIN: 顶部边距，用于显示信息文字
 */
#define LCD_WIDTH 240
#define LCD_HEIGHT 320

static float TIME_WINDOW_S = 1.0f;        /* 时间窗口：显示1秒内的波形 */
static float VOLTAGE_RANGE_V = 10.0f;     /* 电压范围：±5V，总共10V */
static float WAVE_PERIOD_S = 0.125f;       /* 波形周期：0.25秒（4Hz） */
static float WAVE_AMPLITUDE_V = 4.0f;     /* 波形幅度：4V */

static const int TOP_MARGIN = 20;         /* 顶部边距，用于显示参数信息 */

/**
 * @brief 绘制正弦波函数
 * @param time_offset 当前时间偏移量（秒）
 * @param color 波形颜色（16位RGB颜色）
 * 
 * 功能：
 * 1. 清屏并显示当前参数信息
 * 2. 绘制中心参考线
 * 3. 根据时间偏移量绘制动态正弦波
 * 4. 支持电压值到屏幕坐标的转换
 */
static void draw_sine_wave_time(float time_offset, uint16_t color)
{
  const float two_pi = 2.0f * 3.14159265359f;  /* 2π常数，用于正弦函数计算 */

  /* 计算可绘制区域 */
  const int drawable_y0 = TOP_MARGIN;           /* 可绘制区域起始Y坐标 */
  const int drawable_y1 = LCD_HEIGHT - 1;       /* 可绘制区域结束Y坐标 */
  const int drawable_h = drawable_y1 - drawable_y0 + 1;  /* 可绘制区域高度 */

  lcd_clear(WHITE);  /* 清屏为白色背景 */

  /* 显示当前波形参数信息 */
  char buf[64];
  snprintf(buf, sizeof(buf), "Twin=%.2fs P=%.3fs A=%.2fV", 
           TIME_WINDOW_S, WAVE_PERIOD_S, WAVE_AMPLITUDE_V);
  lcd_show_string(0, 0, 200, 16, 16, buf, BLUE);

  /* 绘制中心参考线（接地线） */
  int16_t y_center = drawable_y0 + drawable_h / 2;  /* 计算屏幕中心Y坐标 */
  lcd_draw_line(0, y_center, LCD_WIDTH - 1, y_center, BLACK);  /* 绘制黑色水平线 */

  /* 逐点计算并用线段连接相邻点以获得更平滑的波形 */
  int prev_x = 0;
  int16_t prev_y = 0;
  int has_prev = 0;
  for (int x = 0; x < LCD_WIDTH; x++) {
    /* 计算当前像素点对应的时间坐标 */
    float t = time_offset - TIME_WINDOW_S * (1.0f - ((float)x / (float)(LCD_WIDTH - 1)));

    /* 计算正弦波电压值 */
    float value_v = WAVE_AMPLITUDE_V * sinf(two_pi * t / WAVE_PERIOD_S);

    /* 计算电压到像素的转换比例 */
    float pixels_per_volt = (drawable_h / 2.0f) / (VOLTAGE_RANGE_V / 2.0f);

    /* 计算像素Y坐标（中心为0点） */
    int16_t y = (int16_t) (y_center - (value_v * pixels_per_volt));

    /* 边界检查，确保Y坐标在有效范围内 */
    if (y < drawable_y0) y = drawable_y0;
    if (y > drawable_y1) y = drawable_y1;

    /* 如果已有前一点，则用直线连接前一点与当前点 */
    if (has_prev) {
      lcd_draw_line(prev_x, prev_y, x, y, color);
    } else {
      has_prev = 1;
    }

    prev_x = x;
    prev_y = y;
  }
}

/**
 * @brief 重定向printf输出到串口
 * @param ch 要发送的字符
 * @param f 文件指针（未使用）
 * @return 发送的字符
 * 
 * 功能：将标准输出重定向到USART1，用于调试信息输出
 */
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
  return ch;
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
