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
static void draw_static_elements(void);

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
  
  /* 绘制静态元素（坐标轴、刻度、标签），只绘制一次 */
  draw_static_elements();

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
 */
#define LCD_WIDTH 240
#define LCD_HEIGHT 320

#define LEFT_MARGIN 50                     /* 左侧边距，用于Y轴标签显示 */
#define LABEL_MARGIN 5                     /* 标签距离屏幕左边的边距 */

static float TIME_WINDOW_S = 1.0f;        /* 时间窗口：显示1秒内的波形 */
static float VOLTAGE_RANGE_V = 4.0f;      /* 电压范围：±2V */
static float WAVE_PERIOD_S = 0.5f;        /* 波形周期：0.5秒（2Hz） */
static float WAVE_AMPLITUDE_V = 1.0f;     /* 波形幅度：1V */

static const int TOP_MARGIN = 80;         /* 顶部边距，Y轴从更下方开始 */
static const int BOTTOM_MARGIN = 15;      /* 底部边距，避免X轴标签被截断 */

/**
 * @brief 绘制静态元素（坐标轴、刻度、标签），只需调用一次
 */
static void draw_static_elements(void)
{
  /* 计算可绘制区域 */
  const int drawable_x0 = LEFT_MARGIN;
  const int drawable_x1 = LCD_WIDTH - 1;
  const int drawable_w = drawable_x1 - drawable_x0 + 1;
  const int drawable_y0 = TOP_MARGIN;
  const int drawable_y1 = LCD_HEIGHT - 1 - BOTTOM_MARGIN;
  const int drawable_h = drawable_y1 - drawable_y0 + 1;

  /* 清屏一次 */
  lcd_clear(WHITE);

  /* 绘制坐标轴 */
  lcd_draw_line(drawable_x0, drawable_y0, drawable_x0, drawable_y1, BLACK); /* Y轴 */
  lcd_draw_line(drawable_x0, drawable_y1, drawable_x1, drawable_y1, BLACK); /* X轴在底部 */

  /* 绘制X轴刻度（不绘制时间标签，因为时间是动态的） */
  const int x_ticks = 5;
  for (int i = 0; i < x_ticks; i++) {
    float frac = (float)i / (float)(x_ticks - 1);
    int tx = drawable_x0 + (int)(frac * (drawable_w - 1));
    lcd_draw_line(tx, drawable_y1 - 4, tx, drawable_y1 + 4, BLACK);
  }

  /* 绘制Y轴刻度和电压标签（静态） */
  const int y_ticks = 5;
  for (int i = 0; i < y_ticks; i++) {
    float frac = (float)i / (float)(y_ticks - 1);
    int ty = drawable_y0 + (int)(frac * (drawable_h - 1));
    lcd_draw_line(drawable_x0 - 4, ty, drawable_x0 + 4, ty, BLACK);

    float v_label = (1.0f - frac) * (VOLTAGE_RANGE_V / 2.0f) - frac * (VOLTAGE_RANGE_V / 2.0f);
    char vstr[16];
    snprintf(vstr, sizeof(vstr), "%.1fV", v_label);
    lcd_show_string(LABEL_MARGIN, ty - 6, 40, 12, 12, vstr, BLACK);
  }
}

/**
 * @brief 绘制正弦波函数（优化版：局部更新，减少频闪）
 * @param time_offset 当前时间偏移量（秒）
 * @param color 波形颜色（16位RGB颜色）
 * 
 * 优化策略：
 * 1. 不再使用lcd_fill清除整个区域，避免大面积白色帧
 * 2. 只擦除上一帧的波形线（用背景色重绘）
 * 3. 然后绘制新波形
 */
static void draw_sine_wave_time(float time_offset, uint16_t color)
{
  const float two_pi = 2.0f * 3.14159265359f;

  /* 计算可绘制区域 */
  const int drawable_x0 = LEFT_MARGIN;
  const int drawable_x1 = LCD_WIDTH - 1;
  const int drawable_w = drawable_x1 - drawable_x0 + 1;
  const int drawable_y0 = TOP_MARGIN;
  const int drawable_y1 = LCD_HEIGHT - 1 - BOTTOM_MARGIN;
  const int drawable_h = drawable_y1 - drawable_y0 + 1;

  /* 使用静态变量保存上一帧波形数据用于擦除 */
  static int16_t prev_wave_y[240];  /* 存储上一帧每个x对应的y值 */
  static int prev_wave_valid = 0;   /* 上一帧数据是否有效 */

  /* 波形绘制区域：从Y轴右侧开始，避免擦除Y轴 */
  const int wave_x0 = drawable_x0 + 1;  /* Y轴右侧开始 */
  const int wave_w = drawable_w - 1;    /* 波形宽度 */

  /* 如果有上一帧数据，先用背景色擦除上一帧波形（直接重绘线段，高效） */
  if (prev_wave_valid) {
    int prev_x = 0;
    int16_t prev_y = 0;
    int has_prev = 0;
    for (int i = 0; i < wave_w; i++) {
      int x = wave_x0 + i;
      int16_t y = prev_wave_y[i];
      if (has_prev) {
        lcd_draw_line(prev_x, prev_y, x, y, WHITE);  /* 用白色重绘上一帧波形 */
      } else {
        has_prev = 1;
      }
      prev_x = x;
      prev_y = y;
    }
  }

  /* 重绘X轴线（底部） */
  lcd_draw_line(drawable_x0, drawable_y1, drawable_x1, drawable_y1, BLACK);
  
  /* 重绘Y轴刻度线（可能被波形擦除） */
  const int y_ticks = 5;
  for (int i = 0; i < y_ticks; i++) {
    float frac = (float)i / (float)(y_ticks - 1);
    int ty = drawable_y0 + (int)(frac * (drawable_h - 1));
    lcd_draw_line(drawable_x0 - 4, ty, drawable_x0 + 4, ty, BLACK);
  }

  /* 计算并绘制新波形，同时保存当前帧数据 */
  int prev_x = 0;
  int16_t prev_y = 0;
  int has_prev = 0;
  
  for (int i = 0; i < wave_w; i++) {
    int x = wave_x0 + i;
    float t = time_offset - TIME_WINDOW_S * (1.0f - ((float)i / (float)(wave_w - 1)));
    float value_v = WAVE_AMPLITUDE_V * sinf(two_pi * t / WAVE_PERIOD_S);
    float pixels_per_volt = (drawable_h / 2.0f) / (VOLTAGE_RANGE_V / 2.0f);
    int16_t y = (int16_t)(drawable_y0 + drawable_h / 2 - (value_v * pixels_per_volt));

    if (y < drawable_y0) y = drawable_y0;
    if (y > drawable_y1) y = drawable_y1;

    /* 保存当前点的y值 */
    prev_wave_y[i] = y;

    if (has_prev) {
      lcd_draw_line(prev_x, prev_y, x, y, color);
    } else {
      has_prev = 1;
    }

    prev_x = x;
    prev_y = y;
  }

  prev_wave_valid = 1;
  
  /* 绘制横坐标时间标签（动态，每帧更新） */
  const int x_ticks = 5;
  for (int i = 0; i < x_ticks; i++) {
    float frac = (float)i / (float)(x_ticks - 1);
    int tx = wave_x0 + (int)(frac * (wave_w - 1));
    
    /* 计算对应的时间标签 */
    float t_label = time_offset - TIME_WINDOW_S * (1.0f - frac);
    char lbl[16];
    snprintf(lbl, sizeof(lbl), "%.2fs", t_label);
    /* 先用白色清除旧标签区域，再绘制新标签（X轴下方） */
    lcd_fill(tx - 20, drawable_y1 + 2, tx + 19, drawable_y1 + 14, WHITE);
    lcd_show_string(tx - 20, drawable_y1 + 2, 40, 12, 12, lbl, BLACK);
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
