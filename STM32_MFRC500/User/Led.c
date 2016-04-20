#include "Led.h"

/*
 *	函数名：	LED_GPIO_Config
 *	功能：	配置LED用到的I/O口
 *	输入：	无
 *	输出：	无
 */
void LED_GPIO_Config(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 
	 * SYSCLK 由 SystemInit()配置好了,默认是72MHz
	 * GPIO 所用的时钟 PCLK2 我们采用默认值，也为 72MHz
	 * 采用默认值可以不修改分频器，但外设时钟默认是处在关闭状态
	 * 所以外设时钟一般会在初始化外设的时候设置为开启。
	 * 注意： 
	 *		如果我们用到了I/O的引脚复用功能，还要开启其复用功能时钟
	 *		还有的时钟源是不同的，是挂载在低速外设总线APB1上，使用PCLK1时钟，设置API也需要改变
	 */
	// 开启GPIOB和GPIOC的外设时钟
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// 选择要控制的GPIOC引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	
	// 设置引脚模式为推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	
	// 设置引脚速率为50MHz
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	// 调用库函数，初始化GPIOB
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/**/
	// 选择要控制的GPIOC引脚
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	
	// 调用库函数，初始化GPIOC
	//GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// 让这几个引脚输出高电平，使三盏 LED 初始化后都处于灭状态
	GPIO_SetBits(GPIOB, GPIO_Pin_0);
	//GPIO_SetBits(GPIOC, GPIO_Pin_3 | GPIO_Pin_4);
	
	/*
	//GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11);
	
	GPIOC->ODR &= ~(1 << 8);
	GPIOC->ODR &= ~(1 << 10);
	GPIOC->ODR |= (1 << 9);
	GPIOC->ODR |= (1 << 11);*/
}
