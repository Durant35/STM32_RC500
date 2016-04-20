#ifndef __LED_H__
#define __LED_H__

/*****************************************
 *			硬件连接：
 *				PB0	--	LED1
 *				PC4	--	LED2
 *				PC3	--	LED3
 *****************************************/

#include "stm32f10x.h"

#define ON 	0
#define OFF 1

//“\”续行符的语法要求极其严格，在它的后面不能有空格、注释等一切“杂物”
#define LED1(a)	if(a)	\
				GPIO_SetBits(GPIOB, GPIO_Pin_0);\
				else	\
				GPIO_ResetBits(GPIOB, GPIO_Pin_0)

#define LED2(a)	if(a)	\
				GPIO_SetBits(GPIOC, GPIO_Pin_4);\
				else	\
				GPIO_ResetBits(GPIOC, GPIO_Pin_4)

#define LED3(a)	if(a)	\
				GPIO_SetBits(GPIOC, GPIO_Pin_3);\
				else	\
				GPIO_ResetBits(GPIOC, GPIO_Pin_3)
				
// 直接操作寄存器的方法控制IO
#define digitalHigh(p,i) {p->BSRR = i;}			// 设置为高电平
#define digitalLow(p,i) {p->BRR = i;}			// 设置为低电平
#define digitalToggle(p,i) {p->ODR ^= i;}		// 设置为反转电平

#define LED1_TOGGLE digitalToggle(GPIOB,GPIO_Pin_0)
#define LED1_OFF 	digitalHigh(GPIOB,GPIO_Pin_0)
#define LED1_ON 	digitalLow(GPIOB,GPIO_Pin_0)

#define LED2_TOGGLE digitalToggle(GPIOC,GPIO_Pin_4)
#define LED2_OFF 	digitalHigh(GPIOC,GPIO_Pin_4)
#define LED2_ON 	digitalLow(GPIOC,GPIO_Pin_4)

#define LED3_TOGGLE digitalToggle(GPIOC,GPIO_Pin_3)
#define LED3_OFF 	digitalHigh(GPIOC,GPIO_Pin_3)
#define LED3_ON 	digitalLow(GPIOC,GPIO_Pin_3)
				
void LED_GPIO_Config(void);

#endif
