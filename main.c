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
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "LCD/lcd.h"
#include "LCD/bmp.h"
#include "math.h"

#include "arm_math.h"//fft相关文件
#include "arm_const_structs.h"
void delay_init(uint8_t SYSCLK); 
#define LCD_W      320    // 屏幕宽度
#define LCD_H      240    // 屏幕高度
#define FFT_LEN    1024   // FFT点数
#define SHOW_POINT 256    // 屏幕显示256个频谱点（不卡顿）
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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t capture_Buf[2];
uint8_t adc_complete_flag = 0;
float adc_value = 0;
uint8_t capture_Flag = 0;
uint32_t high_level_time = 0;
 uint8_t RxBuffer[1]; 



#define FFT_LENGTH 1024

__IO uint8_t AdcConvEnd = 0;
uint16_t adcBuff[FFT_LENGTH];

float fft_inputbuf[FFT_LENGTH * 2];  
float fft_outputbuf[FFT_LENGTH];  

void LCD_Show_FFT(float *fft_data);
void LCD_Show_ADC_Wave(uint16_t *adc_data);

void zhang_tim5(void);
void zhang_pwm_control(void);
uint16_t signalSinBuffer[100];
uint16_t signalSquareBuffer[100];
uint16_t signalRampBuffer[100];



// 正弦波信号缓存填充
void signalSin ()
{
    for (int i=0; i<100; i++)
    {
        signalSinBuffer[i] = ((sin(i*2*PI/100) + 1)*(4096/2));
     //   printf(" signalSinBuffer==%d",signalSinBuffer[i]);
    }
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
   delay_init(168);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM14_Init();
  MX_TIM5_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_DAC_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);//led0����
  HAL_UART_Receive_IT(&huart1,RxBuffer,1);
  HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1);
  
  
  __HAL_TIM_SET_AUTORELOAD(&htim3, 99);
  HAL_TIM_Base_Start(&htim3);
  HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adcBuff,FFT_LENGTH);
  
  
  
  __HAL_TIM_SET_AUTORELOAD(&htim7, 199);
  HAL_TIM_Base_Start(&htim7);
  signalSin();
  HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1,(uint32_t*)signalSinBuffer,100,DAC_ALIGN_12B_R);
  
 // HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,1.8/(3.3/4096));

  
//  
    lcd_init();                             /* 初始化LCD */
    g_point_color = RED;
    lcd_clear(WHITE);
      lcd_show_string(10, 80, 240, 24, 24, "TFTLCD TEST", RED);
      HAL_Delay(500);
     //  show_picture(0,0,360,360,(uint16_t*)gImage_rf);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   printf("-------1----------------");
      printf("-------2----------------");
         HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10,GPIO_PIN_RESET);
   
  while (1)
  { 


    // 直接操作寄存器或 HAL 库，每秒翻转一次


//  HAL_Delay(500);
  

 //  zhang_pwm_control();//pwm呼吸灯
  //  HAL_Delay(5);
    if (capture_Flag == 2)
    {
        printf("PA0 High Level Time: %u us\r\n", high_level_time);
        
        capture_Flag = 0;
  
        HAL_Delay(100); 
    }
   if(adc_complete_flag == 1)
    {
  
        
        
        
       for (int i = 0; i < FFT_LENGTH; i++)
{
    fft_inputbuf[i * 2] = adcBuff[i] * 3.3 / 4096;//实部赋值，* 3.3 / 4096是为了将ADC采集到的值转换成实际电压
    fft_inputbuf[i * 2 + 1] = 0;//虚部赋值，固定为0.
}

arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH); 
fft_outputbuf[0] /= 1024;

for (int i = 1; i < FFT_LENGTH; i++)//输出各次谐波幅值
{
    fft_outputbuf[i] /= 512;
}

LCD_Show_FFT(fft_outputbuf);
LCD_Show_ADC_Wave(adcBuff);
printf("FFT Result:\r\n");

for (int i = 0; i < FFT_LENGTH; i++)//输出各次谐波幅值
{
	printf("%d:\t%.2f\r\n", i, fft_outputbuf[i]);
}


     
     printf("d");
       adc_complete_flag=0;


    }
    


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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
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


void HAL_DAC_ConvCpltCallback(DAC_HandleTypeDef *hdac)
{
  

    
    printf("-------3----------------");
       HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10,GPIO_PIN_RESET);  // 随便找个LED翻转
        
    
}
void LCD_Show_FFT(float *fft_data)
{
    uint16_t i;
    uint16_t x, y;
  
    float max_amp = 0.5f;  
    
    lcd_clear(WHITE);

    // 画坐标轴
    lcd_draw_line(0, LCD_H-1, LCD_W-1, LCD_H-1, BLACK);
    lcd_draw_line(0, 0, 0, LCD_H-1, BLACK);

    
    for(i=0; i<320; i++)
    {
        x = i; 
   
        y = 220 - (uint16_t)((fft_data[i] / max_amp) * 200);
        
        // 限制范围，防止溢出
        if(y > 220) y = 220;
        if(y < 20)  y = 20;
        
        // 画红色柱子
        lcd_draw_line(x, 220, x, y, RED);
    }
}
void LCD_Show_ADC_Wave(uint16_t *adc_data)
{
    uint16_t i, y1, y2;
    // 绘制ADC原始波形（蓝色）
    for(i=0; i<255; i++)  // 只画255点，适配屏幕宽度
    {
        y1 = 500 - (adc_data[i] * 200 / 4095);
        y2 = 500 - (adc_data[i+1] * 200 / 4095);
        lcd_draw_line(i, y1, i+1, y2, BLUE);
       
    } HAL_Delay(100);
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
  {HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET); // 点亮 LED
    
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
