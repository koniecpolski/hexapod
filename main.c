#include "stm32f1xx.h"
#include "pwmlib.h"
#include <stdlib.h> //dla randa

GPIO_InitTypeDef gpio;
I2C_HandleTypeDef i2c;
TIM_HandleTypeDef timer;
TIM_HandleTypeDef timer2;
HAL_StatusTypeDef ret;

uint8_t bufor[49];
uint8_t actu[12];

uint8_t obs=0; //przeszkoda
uint8_t prawo=1; //kierunek
div_t divresult;


void Gpio_conf(void);
void Timer_conf(void);
void I2c_conf(void);

void tripod_gate(void);
void s_prawo(void);
void s_lewo(void);

int main(void)
{
	 SystemCoreClock = 8000000; // taktowanie 8Mhz
	 HAL_Init(); //uruchomienie biblioteki HAL
	 Gpio_conf();
	 Timer_conf();
	 I2c_conf();

	 pwm_inicjalizacja(); // wstepna konfiguracja sterownika

	 HAL_Delay(5000);

	 while (1)
	 {
		 if(obs==1) //losowanie kierunku
		 {
			 divresult = div(__HAL_TIM_GET_COUNTER(&timer2),2);
			 prawo=divresult.rem;
		 }
		 while(obs==1) //jesli przeszkoda - skrecaj
		 {
			 if(prawo==1)
			 	s_prawo();
			 else
			 	s_lewo();
		 }
		 tripod_gate(); //idz do przodu
	 }
}

void Gpio_conf(void)
{
	//GPIO_InitTypeDef gpio;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	// GPIO dla I2C
	gpio.Mode = GPIO_MODE_AF_OD;
	gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;		// SCL, SDA
	gpio.Pull = GPIO_PULLUP; //PULL up bo nie ma rezystorów sterownik
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &gpio);

	//GPIO dla SONIC
	//Reset pinu TRIGGER
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

	//pin: PA1 - TRIGGER
	gpio.Pin = GPIO_PIN_1;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &gpio);

	//pin: PA4 - ECHO
	gpio.Pin = GPIO_PIN_4;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio);


}

void Timer_conf(void)
{
	//TIMER us - 1MHz
	__HAL_RCC_TIM2_CLK_ENABLE();

	timer.Instance = TIM2;
	timer.Init.Period = 0xffff - 1;
	timer.Init.Prescaler = 8 - 1;
	timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	timer.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer.Init.RepetitionCounter = 0;
	timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&timer);
	HAL_TIM_Base_Start(&timer);


	//TIMER przerwanie - 1kHz
	__HAL_RCC_TIM3_CLK_ENABLE();

	timer2.Instance = TIM3;
	timer2.Init.Period = 500 - 1;
	timer2.Init.Prescaler = 8000 - 1;
	timer2.Init.ClockDivision = 0;
	timer2.Init.CounterMode = TIM_COUNTERMODE_UP;
	timer2.Init.RepetitionCounter = 0;
	timer2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&timer2);

	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_TIM_Base_Start_IT(&timer2);
}

void I2c_conf(void)
{
	__HAL_RCC_I2C1_CLK_ENABLE();

	i2c.Instance             = I2C1;
	i2c.Init.ClockSpeed      = 100000;
	i2c.Init.DutyCycle       = I2C_DUTYCYCLE_2;
	i2c.Init.OwnAddress1     = 0xff;
	i2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	i2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	i2c.Init.OwnAddress2     = 0xff;
	i2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	i2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

	HAL_I2C_Init(&i2c);
}

void tripod_gate(void)
{
	uint8_t firo[]={1,6,8}; //grupa I outer
	uint8_t seco[]={0,2,7}; //grupa II outer
	uint8_t internal[]={4,9,11,3,5,10}; //grupa wewnetrzna
	uint8_t forward1[]={forw,forw,forw,back,back,back}; //grupa I do przodu
	uint8_t forward2[]={back,back,back,forw,forw,forw}; //grupa II do przodu

	pwm_zmiana_pick(firo,sizeof(firo),up);//do gory I
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_slow(internal,sizeof(internal),forward1);
	HAL_Delay(wait/2);

	pwm_zmiana_pick(firo,sizeof(firo),down);//na dol I
	pwm_aktualizuj();
	HAL_Delay(wait);
	pwm_zmiana_pick(seco,sizeof(seco),up);//do gory II
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_slow(internal,sizeof(internal), forward2);
	HAL_Delay(wait/2);

	pwm_zmiana_pick(seco,sizeof(seco),down);//na dol II
	pwm_aktualizuj();
	HAL_Delay(wait);
	return;
}

void s_prawo(void)
{
	uint8_t firo[]={1,6,8}; //grupa I outer
	uint8_t firi[]={4,9,11}; //grupa I inter
	uint8_t right1[]={forw,back,back};
	uint8_t seco[]={0,2,7};	//grupa II outer
	uint8_t seci[]={3,5,10}; //grupa II inter
	uint8_t right2[]={forw,forw,back};

	pwm_zmiana_pick(firo, sizeof(firo),up);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_array(firi, sizeof(firi), right1);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_pick(firo, sizeof(firo),down);
	pwm_aktualizuj();
	HAL_Delay(wait);
	//----------------------druga para

	pwm_zmiana_pick(seco,sizeof(seco),up);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_array(seci, sizeof(seci), right2);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_pick(seco, sizeof(seco),down);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_pick(firi,sizeof(firi),def);
	pwm_zmiana_pick(seci,sizeof(seci),def);
	pwm_aktualizuj();
	HAL_Delay(wait);
}

void s_lewo(void)
{
	uint8_t firo[]={1,6,8}; //grupa I outer
	uint8_t firi[]={4,9,11}; //grupa I inter
	uint8_t left1[]={back,forw,forw};
	uint8_t seco[]={0,2,7}; //grupa II outer
	uint8_t seci[]={3,5,10}; //grupa II inter
	uint8_t left2[]={back,back,forw};

	pwm_zmiana_pick(firo,sizeof(firo),up);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_array(firi, sizeof(firi), left1);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_pick(firo,sizeof(firo),down);
	pwm_aktualizuj();
	HAL_Delay(wait);
	//----------------------
	pwm_zmiana_pick(seco,sizeof(seco),up);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_array(seci, sizeof(seci), left2);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_pick(seco,sizeof(seco),down);
	pwm_aktualizuj();
	HAL_Delay(wait);

	pwm_zmiana_pick(firi,sizeof(firi),def);
	pwm_zmiana_pick(seci,sizeof(seci),def);
	pwm_aktualizuj();
	HAL_Delay(wait);
}
