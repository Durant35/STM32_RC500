#include "SysTick.h"

static __IO u32 TimingDelay;

/**
  * @brief  启动系统滴答定时器 SysTick
  * @param  无
  * @return 无
  */
void SysTick_Init(void){
	/*
	 *	SystemFrequency/1000		1ms中断1次
	 *	SystemFrequency/100000		10us中断1次
	 *	SystemFrequency/1000000		1us中断1次(最小为1/72 μs)
	 *		使用时钟源为 AHB时钟，其频率(SystemFrequency)被配置为 72MHz。调用函数时，把ticks 赋值为: 
	 *						ticks = SystemFrequency/10000 = 720，表示 720个时钟周期中断一次；
	 *		1/f 是时钟周期的时间，此时1/f =1/72 μs，所以最终定时总时间: 
	 *						T=720×（1/72），为720个时钟周期，正好是 10μs。
	 */
	// 若调用SysTick_Config()配置SysTick不成功，则进入死循环
	// 初始化SysTick成功后，先关闭定时器，在需要的时候再开启
	if(SysTick_Config(SystemCoreClock/1000000)){
		while(1);
	}
	// 关闭滴答定时器，在需要的时候再开启
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

/**
  * @brief  us延时程序
  * @param  
  *		@arg uTime: 延时uTime(us)
  * @return 无
  * @attention: 
  *		一旦我们调用了Delay_us()函数，SysTick定时器就被开启，按照设定好的定时周期
  *	递减计数，当SysTick的计数寄存器的值减为 0 时，就进入中断函数，当中断函数执行完
  * 毕之后重新计时，如此循环，除非它被关闭.
  *		最小计时周期1/72000000 秒
  */
void Delay_us(__IO u32 uTime){
	TimingDelay = uTime;
	// 使能滴答定时器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	
	while(TimingDelay>0)
		;
}

/**
  * @brief  获取节拍程序
  * @param  无
  * @return 无
  * @attention: 
  *		在SysTick中断函数SysTick_Handler函数中调用
  */
void TimingDelay_Decrement(void){
	if(TimingDelay != 0x00){
		TimingDelay--;
	}
}

/**
  * @brief  ns延时程序
  * @param  无
  * @return 无
  * @attention: 
  *		__nop执行需要13.9ns
  */
void Delay_14ns(__IO u32 ns){
  u32 i;
  for(i=0;i<ns;i++){
    __nop();
  }
}

