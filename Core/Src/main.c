/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include <string.h>
#include <stdbool.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
uint16_t pwmVal = 0; 
uint8_t uartRxBuffer[2];
uint16_t newPwmVal;
uint32_t powerOnTimeMs = 0;
int32_t addr, writeFlashData;
volatile bool newBrightness = false;
volatile bool received = false;
volatile bool newtime = false;
//volatile bool newBrightnessReceived = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void sendBrightness(uint16_t pwmVal) 
	{
  char buffer[20];
  sprintf(buffer, "brightness: %d\r\n", pwmVal/2);
  HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
		newBrightness = true;
		
			}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  if (huart == &huart1)
  {

		newPwmVal = ((uartRxBuffer[0]-(uint8_t)*"0")*10+uartRxBuffer[1]-(uint8_t)*"0")*2;
      pwmVal = newPwmVal;
      __HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,pwmVal); 
      sendBrightness(pwmVal); 
			received=true;

//memset(uartRxBuffer,0,3);
		
    HAL_UART_Receive_IT(&huart1, uartRxBuffer, 2); 
  }
}

void SysTick_Handler(void)
{
 powerOnTimeMs += 1; 
	HAL_IncTick();	
			if(powerOnTimeMs % 1000 == 0)
			{
				newtime=true;				
			}
				
}
void sendTime(uint16_t powerOnTimeMs) 
{
  char buffer[100];
    sprintf(buffer, "Power on time: %lu s\r\n\r\n", powerOnTimeMs/1000);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}
void sendlastTime(uint16_t powerOnTimeMs) 
{
  char buffer[100];
    sprintf(buffer, "Last Power on time: %lu s\r\n", powerOnTimeMs/1000);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}
void writeFlashTest(void)
{
	HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef f;
	f.TypeErase = FLASH_TYPEERASE_PAGES;
	f.PageAddress = addr;
	f.NbPages = 1;

	uint32_t PageError = 0;
	
	HAL_FLASHEx_Erase(&f, &PageError);

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,addr, writeFlashData);

	HAL_FLASH_Lock() ;
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
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	
	
	pwmVal=*(__IO uint16_t*)(0x08007000);	
	powerOnTimeMs=*(__IO uint16_t*)(0x08007200);
	sendlastTime(powerOnTimeMs);
	powerOnTimeMs=0;
	sendBrightness(pwmVal);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
	HAL_UART_Receive_IT(&huart1, uartRxBuffer, 2);
	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_2, pwmVal);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2)==GPIO_PIN_RESET & pwmVal<200)
		{
			pwmVal=pwmVal+2;
			__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,pwmVal);
			sendBrightness(pwmVal);
			sendTime(powerOnTimeMs);
			HAL_Delay(120);
		}
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==GPIO_PIN_RESET & pwmVal>0)
		{
			pwmVal=pwmVal-2;
			__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,pwmVal);
			sendBrightness(pwmVal);
			sendTime(powerOnTimeMs);
			HAL_Delay(120);
		}
		if(received)
		{
			sendTime(powerOnTimeMs);
			received=false;
		}
		if(newBrightness)
		{
			addr=0x08007500;
		  writeFlashData=	pwmVal;
		  writeFlashTest();
			newBrightness=false;
		}
		if(newtime)
		{
			addr=0x08007700;
		  writeFlashData=	powerOnTimeMs;
		  writeFlashTest();
			newtime=false;
		}
		HAL_Delay(50);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
