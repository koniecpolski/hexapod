#ifndef PWMLIB_H_
#define PWMLIB_H_

#include "stm32f1xx.h"

#define PWM_write			0xe0
#define PWM_read			0xe1
#define MODE1				0x00
#define MODE2				0x01

#define LED0_ON_L			0x06
#define LED0_ON_H			0x07
#define LED0_OFF_L			0x08
#define LED0_OFF_H			0x09
#define LED1_ON_L			0x0a
#define LED1_ON_H			0x0b
#define LED1_OFF_L			0x0c
#define LED1_OFF_H			0x0d
#define LED2_ON_L			0x0e
#define LED2_ON_H			0x0f
#define LED2_OFF_L			0x10
#define LED2_OFF_H			0x11
#define LED3_ON_L			0x12
#define LED3_ON_H			0x13
#define LED3_OFF_L			0x14
#define LED3_OFF_H			0x15
#define LED4_ON_L			0x16
#define LED4_ON_H			0x17
#define LED4_OFF_L			0x18
#define LED4_OFF_H			0x19
#define LED5_ON_L			0x1a
#define LED5_ON_H			0x1b
#define LED5_OFF_L			0x1c
#define LED5_OFF_H			0x1d
#define LED6_ON_L			0x1e
#define LED6_ON_H			0x1f
#define LED6_OFF_L			0x20
#define LED6_OFF_H			0x21
#define LED7_ON_L			0x22
#define LED7_ON_H			0x23
#define LED7_OFF_L			0x24
#define LED7_OFF_H			0x25
#define LED8_ON_L			0x26
#define LED8_ON_H			0x27
#define LED8_OFF_L			0x28
#define LED8_OFF_H			0x29
#define LED9_ON_L			0x2a
#define LED9_ON_H			0x2b
#define LED9_OFF_L			0x2c
#define LED9_OFF_H			0x2d
#define LED10_ON_L			0x2e
#define LED10_ON_H			0x2f
#define LED10_OFF_L			0x30
#define LED10_OFF_H			0x31
#define LED11_ON_L			0x32
#define LED11_ON_H			0x33
#define LED11_OFF_L			0x34
#define LED11_OFF_H			0x35


#define ALL_LED_ON_L		0xfa
#define ALL_LED_ON_H		0xfb
#define ALL_LED_OFF_L		0xfc
#define ALL_LED_OFF_H		0xfd

#define PRE_SCALE			0xfe

#define okres_default		20
#define opoznienie_default	2

#define def 90
#define down 140
#define up 115
#define forw 80//75
#define back 100//105
#define wait 300//500
#define del 1

extern I2C_HandleTypeDef i2c;
extern HAL_StatusTypeDef ret;
extern uint8_t bufor[49];
extern uint8_t actu[12];
//basic
uint8_t pwm_odczyt_rej(uint8_t rejestr);
void pwm_zapis_rej(uint8_t rejestr, uint8_t wartosc);

//config
void pwm_conf(void);
void pwm_sleep_off(void);
void pwm_auto_inc(void);
void pwm_prescale(uint32_t okres_ms);
void pwm_turnoff_all(void);
//utility
void oblicz_okres_pwm(uint8_t* led_ON, uint8_t* led_OFF, float opoznienie, float okres_pwm, uint32_t okres_ms);
float pwm_kat_okres(uint8_t kat);
//wykorzystujace bufor

void pwm_aktualizuj(void);
void pwm_zmiana(uint8_t pwm_nr, uint8_t kat);
void pwm_zmiana_pick(uint8_t* pwm_nr, uint8_t rozmiar, uint8_t kat);
void pwm_zmiana_array(uint8_t* pwm_nr, uint8_t rozmiar, uint8_t* kat);
void pwm_zmiana_slow(uint8_t* pwm_nr, uint8_t rozmiar, uint8_t* kat);
void pwm_inicjalizacja(void);
#endif /* PWMLIB_H_ */
