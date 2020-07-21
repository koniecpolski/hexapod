#include "pwmlib.h"

//BASIC I2C
uint8_t pwm_odczyt_rej(uint8_t rejestr)
{
	uint8_t odczyt = 0;
	HAL_I2C_Mem_Read(&i2c, PWM_read, rejestr, 1, &odczyt, sizeof(odczyt), HAL_MAX_DELAY);
	return odczyt;
}

void pwm_zapis_rej(uint8_t rejestr, uint8_t wartosc)
{
	HAL_I2C_Mem_Write(&i2c, PWM_write, rejestr, 1, &wartosc, sizeof(wartosc), HAL_MAX_DELAY);
	return;
}

//CONFIGURATION
void pwm_conf(void)
{
	uint8_t konfig=0x21;// 00-1-0 000-1 AI_on & Sleep_off
	pwm_zapis_rej(MODE1,konfig);
	return;
}

void pwm_sleep_off(void)
{
	uint8_t dane = pwm_odczyt_rej(MODE1);
	uint8_t maska = 0x10;
	if((dane&maska) == maska) // bit 4 SLEEP ustawiony jest na 1
	{
		pwm_zapis_rej(MODE1,dane^maska);
	}
	return;
}

void pwm_auto_inc(void)
{
	uint8_t dane = pwm_odczyt_rej(MODE1);
	uint8_t maska = 0x20;
	if((dane&maska) != maska) // bit 5 SLEEP ustawiony jest na 1
		{
			pwm_zapis_rej(MODE1,dane^maska);
		}
	return;
}

void pwm_prescale(uint32_t okres_ms)
{
	uint32_t czestotliwosc=1000/okres_ms;
	uint8_t skala = 25000000/(4096*czestotliwosc)-1;
	pwm_zapis_rej(PRE_SCALE,skala);
	return;
}

void pwm_turnoff_all(void)
{
	pwm_zapis_rej(ALL_LED_OFF_H, 0x10);
	return;
}
//UTILITY
void oblicz_okres_pwm(uint8_t* led_ON, uint8_t* led_OFF, float opoznienie, float okres_pwm, uint32_t okres_ms)
{
	uint16_t opoznienie_h = (opoznienie/okres_ms)*4096-1;
	uint16_t wypelnienie = (okres_pwm/okres_ms)*4096;

	led_ON[0]= opoznienie_h & 0xff;//L
	led_ON[1]=(opoznienie_h >> 8) & 0xff;//H

	uint16_t wartosc = opoznienie_h+wypelnienie;

	led_OFF[0]= wartosc & 0xff;//L
	led_OFF[1]=(wartosc >> 8) & 0xff;//H
	return;
}

float pwm_kat_okres(uint8_t kat)
{
	if(kat>180)
			return 1.5; //90°
	float okres_pwm = 1.1*kat/90+0.4;
	return okres_pwm;
}



//SIM - BUFOR

void pwm_aktualizuj(void)
{
	HAL_NVIC_DisableIRQ(TIM3_IRQn);
	ret = HAL_I2C_Master_Transmit(&i2c, PWM_write, bufor, sizeof(bufor), HAL_MAX_DELAY);
	if (ret != HAL_OK)
	{
			//nie wysz³o
	}
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	return;
}


void pwm_zmiana(uint8_t pwm_nr, uint8_t kat)
{
	actu[pwm_nr]=kat;
	uint8_t pwm_ON[2];
	uint8_t pwm_OFF[2];

	if(pwm_nr<6)
		kat=180-kat;

	//POPRAWKI
	if(pwm_nr==9)
		kat=kat-7;
	if(pwm_nr==5)
		kat=kat+1;
	if((pwm_nr==4)||(pwm_nr==11))
		kat=kat+3;
	if(pwm_nr==10)
		kat=kat+9;

	oblicz_okres_pwm(pwm_ON,pwm_OFF,opoznienie_default,pwm_kat_okres(kat),okres_default);

	uint8_t i=1+pwm_nr*4;
	bufor[i]=pwm_ON[0];
	bufor[i+1]=pwm_ON[1];
	bufor[i+2]=pwm_OFF[0];
	bufor[i+3]=pwm_OFF[1];

	return;
}


void pwm_zmiana_pick(uint8_t* pwm_nr, uint8_t rozmiar, uint8_t kat)
{
	for(uint8_t i=0;i<rozmiar;i++)
			pwm_zmiana(pwm_nr[i],kat);
	return;
}

void pwm_zmiana_array(uint8_t* pwm_nr, uint8_t rozmiar, uint8_t* kat)
{
	for(uint8_t i=0;i<rozmiar;i++)
		pwm_zmiana(pwm_nr[i],kat[i]);
	return;
}

void pwm_zmiana_slow(uint8_t* pwm_nr, uint8_t rozmiar, uint8_t* kat)
{
	uint8_t kat_t[rozmiar];
	uint8_t on=1;
	uint8_t i;
	for(i=0;i<rozmiar;i++)
		kat_t[i]=actu[pwm_nr[i]];//wczytanie aktualnych katow

	while(on)//dopoki wszystkie katy nie sa wlasciwe
	{
		on=0;
		for(i=0;i<rozmiar;i++)
		{
			if(actu[pwm_nr[i]]!=kat[i])
			{
				if(actu[pwm_nr[i]]<kat[i])
					kat_t[i]=kat_t[i]+1;
				else
					kat_t[i]=kat_t[i]-1;

				on=1; //potrzeba robienia dalej
				pwm_zmiana(pwm_nr[i],kat_t[i]);
			}
		}
		pwm_aktualizuj();
		HAL_Delay(del);
	}

}

void pwm_inicjalizacja(void)
{
	pwm_auto_inc();
	pwm_prescale(20);
	pwm_sleep_off(); //wlacza uspienie
	HAL_Delay(1); //500us na wystartowanie
	pwm_turnoff_all(); //reset kanalow

	//wstawaj!
	bufor[0]=LED0_ON_L; //adres pierwszego rejestru
	uint8_t pwm_I[]={0,1,2,6,7,8}; //serwa pionowe
	uint8_t pwm_II[]={3,4,5,9,10,11}; //serwa poziome
	pwm_zmiana_pick(pwm_I,sizeof(pwm_I),down);
	pwm_zmiana_pick(pwm_II,sizeof(pwm_II),def);
	pwm_aktualizuj();

}
