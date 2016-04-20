#ifndef __SYSTICK_H__
#define __SYSTICK_H__

#include "stm32f10x.h"
#include "core_cm3.h"					//	≈‰÷√SysTick µƒ÷–∂œ

void SysTick_Init(void);
void Delay_us(__IO u32 uTime);
void TimingDelay_Decrement(void);

#define Delay_ms(mTime) Delay_us(1000*mTime)

void Delay_14ns(__IO u32 ns);

#endif

