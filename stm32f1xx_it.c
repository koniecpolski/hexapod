/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @author  Ac6
  * @version V1.0
  * @date    02-Feb-2015
  * @brief   Default Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#ifdef USE_RTOS_SYSTICK
#include <cmsis_os.h>
#endif
#include "stm32f1xx_it.h"

extern TIM_HandleTypeDef timer;
extern TIM_HandleTypeDef timer2;
extern uint8_t obs;


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void delay (uint32_t us)
{
	__HAL_TIM_SET_COUNTER(&timer, 0);
	while ((__HAL_TIM_GET_COUNTER(&timer))<us);
}

uint32_t hcsr04_read (void)
{
	uint32_t local_time=0;

	/*
	Trig_Pin  GPIO_PIN_1
	Echo_Pin  GPIO_PIN_4
	*/

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);  // reset pinu TRIGGER
	delay(2);  // 2 us
	while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4)); //czekaj az fala sie skonczy

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);  // ustaw pin TRIGGER
	delay(10);  // 10 us
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);  // reset pinu TRIGGER

	// zmierz czas przez jaki sygnal ECHO jest wysoki

	while (!(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4)));  // czekaj na fale
	while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4))    // dopoki sygnal jest wysoki
	 {
		local_time++;   // mierz czas
		delay (1);
	 }
	return local_time;
}

/******************************************************************************/
/*            	  	    Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles SysTick Handler, but only if no RTOS defines it.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
#ifdef USE_RTOS_SYSTICK
	osSystickHandler();
#endif
}

void TIM3_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&timer2); //Przekazanie obslugi przerwania do HAL
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	uint32_t sensor_time;
	float distance=0;

	sensor_time = hcsr04_read(); //wykonaj pomiar
	distance = sensor_time * 0.34/2;

	if(distance<50) //jesli przeszkoda blisko
		obs=1; //skrecanie
	else
		obs=0; //droga wolna
}
