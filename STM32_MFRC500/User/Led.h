#ifndef __LED_H__
#define __LED_H__

/*****************************************
 *			Ӳ�����ӣ�
 *				PB0	--	LED1
 *				PC4	--	LED2
 *				PC3	--	LED3
 *****************************************/

#include "stm32f10x.h"

#define ON 	0
#define OFF 1

//��\�����з����﷨Ҫ�����ϸ������ĺ��治���пո�ע�͵�һ�С����
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
				
// ֱ�Ӳ����Ĵ����ķ�������IO
#define digitalHigh(p,i) {p->BSRR = i;}			// ����Ϊ�ߵ�ƽ
#define digitalLow(p,i) {p->BRR = i;}			// ����Ϊ�͵�ƽ
#define digitalToggle(p,i) {p->ODR ^= i;}		// ����Ϊ��ת��ƽ

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
