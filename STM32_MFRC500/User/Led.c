#include "Led.h"

/*
 *	��������	LED_GPIO_Config
 *	���ܣ�	����LED�õ���I/O��
 *	���룺	��
 *	�����	��
 */
void LED_GPIO_Config(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 
	 * SYSCLK �� SystemInit()���ú���,Ĭ����72MHz
	 * GPIO ���õ�ʱ�� PCLK2 ���ǲ���Ĭ��ֵ��ҲΪ 72MHz
	 * ����Ĭ��ֵ���Բ��޸ķ�Ƶ����������ʱ��Ĭ���Ǵ��ڹر�״̬
	 * ��������ʱ��һ����ڳ�ʼ�������ʱ������Ϊ������
	 * ע�⣺ 
	 *		��������õ���I/O�����Ÿ��ù��ܣ���Ҫ�����临�ù���ʱ��
	 *		���е�ʱ��Դ�ǲ�ͬ�ģ��ǹ����ڵ�����������APB1�ϣ�ʹ��PCLK1ʱ�ӣ�����APIҲ��Ҫ�ı�
	 */
	// ����GPIOB��GPIOC������ʱ��
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// ѡ��Ҫ���Ƶ�GPIOC����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	
	// ��������ģʽΪ�������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	
	// ������������Ϊ50MHz
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	// ���ÿ⺯������ʼ��GPIOB
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/**/
	// ѡ��Ҫ���Ƶ�GPIOC����
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	
	// ���ÿ⺯������ʼ��GPIOC
	//GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// ���⼸����������ߵ�ƽ��ʹ��յ LED ��ʼ���󶼴�����״̬
	GPIO_SetBits(GPIOB, GPIO_Pin_0);
	//GPIO_SetBits(GPIOC, GPIO_Pin_3 | GPIO_Pin_4);
	
	/*
	//GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11);
	
	GPIOC->ODR &= ~(1 << 8);
	GPIOC->ODR &= ~(1 << 10);
	GPIOC->ODR |= (1 << 9);
	GPIOC->ODR |= (1 << 11);*/
}
