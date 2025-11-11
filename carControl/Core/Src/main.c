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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

typedef union {
    struct {
        int speed; 
        char direction[4]; 
        int lights; 
        int horn;
    } data;
    uint8_t buffer[sizeof(int) + 4 + sizeof(int) * 2]; 
} CarControl;


typedef struct {
    uint8_t type; 
    CarControl control;
} CarMessage;

uint8_t rxBuffer[sizeof(CarMessage)];
CarMessage *receveing = (CarMessage*)rxBuffer;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define EVENT0 (1UL << 0)
#define EVENT1 (1UL << 1)
#define EVENT2 (1UL << 2)
#define EVENT3 (1UL << 3)
#define EVENT4 (1UL << 4)


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

osThreadId Task_driver_motHandle;
osThreadId myTask_LedHandle;
osThreadId myTask_HornHandle;
osThreadId Get_DataHandle;
/* USER CODE BEGIN PV */

EventGroupHandle_t MyEventGroupHandle; 
EventBits_t BitsToWaitFor; 

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
void StartTask_driver_motor(void const * argument);
void StartTask_Led(void const * argument);
void StartTask_Horn(void const * argument);
void Get_Data_Task(void const * argument);

/* USER CODE BEGIN PFP */

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
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart2, rxBuffer, sizeof(rxBuffer));
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Task_driver_mot */
  osThreadDef(Task_driver_mot, StartTask_driver_motor, osPriorityHigh, 0, 128);
  Task_driver_motHandle = osThreadCreate(osThread(Task_driver_mot), NULL);

  /* definition and creation of myTask_Led */
  osThreadDef(myTask_Led, StartTask_Led, osPriorityLow, 0, 64);
  myTask_LedHandle = osThreadCreate(osThread(myTask_Led), NULL);

  /* definition and creation of myTask_Horn */
  osThreadDef(myTask_Horn, StartTask_Horn, osPriorityBelowNormal, 0, 64);
  myTask_HornHandle = osThreadCreate(osThread(myTask_Horn), NULL);

  /* definition and creation of Get_Data */
  osThreadDef(Get_Data, Get_Data_Task, osPriorityRealtime, 0, 128);
  Get_DataHandle = osThreadCreate(osThread(Get_Data), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

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

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);

  /*Configure GPIO pins : PA0 PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance == USART2) { 
		if(receveing->type == 0x01){
			BaseType_t xHigher = xTaskResumeFromISR(Get_DataHandle); 
			portEND_SWITCHING_ISR(xHigher); 
		}
  }
	HAL_UART_Receive_IT(&huart2, rxBuffer, sizeof(rxBuffer));
}

void Change_PWM_Mode(uint8_t channel, uint8_t mode) {
    if (channel == 1) {
        TIM1->CCMR1 &= ~(7 << TIM_CCMR1_OC1M_Pos); 
        TIM1->CCMR1 |= (mode << TIM_CCMR1_OC1M_Pos); 
    } else if (channel == 2) {
        TIM1->CCMR1 &= ~(7 << TIM_CCMR1_OC2M_Pos);
        TIM1->CCMR1 |= (mode << TIM_CCMR1_OC2M_Pos);
    } else if (channel == 3) {
        TIM1->CCMR2 &= ~(7 << TIM_CCMR2_OC3M_Pos);
        TIM1->CCMR2 |= (mode << TIM_CCMR2_OC3M_Pos);
    } else if (channel == 4) {
        TIM1->CCMR2 &= ~(7 << TIM_CCMR2_OC4M_Pos);
        TIM1->CCMR2 |= (mode << TIM_CCMR2_OC4M_Pos);
    }
}


/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartTask_driver_motor */
/**
  * @brief  Function implementing the Task_driver_mot thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTask_driver_motor */
void StartTask_driver_motor(void const * argument)
{
  /* USER CODE BEGIN 5 */
	MyEventGroupHandle = xEventGroupCreate(); 
	//TIM1->CCMR1 = receveing->control.data.speed; 
  /* Infinite loop */
  for(;;)
  {
		BitsToWaitFor = xEventGroupWaitBits(MyEventGroupHandle,EVENT0|EVENT1|EVENT2|EVENT3|EVENT4,pdTRUE,pdFALSE,osWaitForever); 
		if((BitsToWaitFor & EVENT0) != 0){

			Change_PWM_Mode(2, 6); 
			Change_PWM_Mode(4, 6); 
			TIM1->CCR2 = receveing->control.data.speed; 
			TIM1->CCR4 = receveing->control.data.speed; 
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2); 
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_4);
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_3);			
		}
		else if((BitsToWaitFor & EVENT1) != 0){
			Change_PWM_Mode(1, 6); 
			Change_PWM_Mode(3, 6); 
			TIM1->CCR1 = receveing->control.data.speed; 
			TIM1->CCR3 = receveing->control.data.speed; 
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1); 
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_2);
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_4);
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3); 			
		}
		else if((BitsToWaitFor & EVENT2) != 0){
			Change_PWM_Mode(2, 6); 
			Change_PWM_Mode(1, 7); 
			TIM1->CCR1 = 95; 
			TIM1->CCR2 = 95; 
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2); 
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1); 
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_3); 
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_4); 
		}
		else if((BitsToWaitFor & EVENT3) != 0){
			Change_PWM_Mode(3, 7); 
			Change_PWM_Mode(4, 6); 
			TIM1->CCR3 = 95; 
			TIM1->CCR4 = 95; 
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3); 
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_4); 
	
		}
		else if((BitsToWaitFor & EVENT4) != 0){
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1); 
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_2); 
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_3); 
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_4); 
		}
		
		
		
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTask_Led */
/**
* @brief Function implementing the myTask_Led thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_Led */
void StartTask_Led(void const * argument)
{
  /* USER CODE BEGIN StartTask_Led */
  /* Infinite loop */
  for(;;)
  {
		GPIOB->ODR |= (1<<0); 
		GPIOB->ODR &= ~(1<<1); 
		osDelay(75);
		GPIOB->ODR &= ~(1<<0); 
		GPIOB->ODR |= (1<<1); 
		osDelay(75);  
  }
  /* USER CODE END StartTask_Led */
}

/* USER CODE BEGIN Header_StartTask_Horn */
/**
* @brief Function implementing the myTask_Horn thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_Horn */
void StartTask_Horn(void const * argument)
{
  /* USER CODE BEGIN StartTask_Horn */
  /* Infinite loop */
  for(;;)
  {
		if(receveing->control.data.horn == 0){
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,1);
		}
		else if(receveing->control.data.horn == 1){
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,0);
		}
		else if(receveing->control.data.speed <= 20){
			
		}
		osDelay(1); 
  }
  /* USER CODE END StartTask_Horn */
}

/* USER CODE BEGIN Header_Get_Data_Task */
/**
* @brief Function implementing the Get_Data thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Get_Data_Task */
void Get_Data_Task(void const * argument)
{
  /* USER CODE BEGIN Get_Data_Task */
  /* Infinite loop */
  for(;;)
  {
		vTaskSuspend(NULL); 
		if(!strcmp(receveing->control.data.direction,"up")){
			xEventGroupSetBits(MyEventGroupHandle,EVENT0); 
		}
		else if(!strcmp(receveing->control.data.direction,"down")){
			xEventGroupSetBits(MyEventGroupHandle,EVENT1); 
		}
		else if(!strcmp(receveing->control.data.direction,"left")){
			xEventGroupSetBits(MyEventGroupHandle,EVENT2); 
		}
		else if(!strcmp(receveing->control.data.direction,"righ")){
			xEventGroupSetBits(MyEventGroupHandle,EVENT3); 
		}
		else if(!strcmp(receveing->control.data.direction,"stop")){
			xEventGroupSetBits(MyEventGroupHandle,EVENT4); 
		}

		
    osDelay(1);
  }
  /* USER CODE END Get_Data_Task */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
