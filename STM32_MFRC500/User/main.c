/**
  ******************************************************************************
  * @file    main.c 
  * @author  Tarantula-7
  * @version V0.1
  * @date    2015.11.27
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
#include "SysTick.h"
#include "USART.h"
#include "ParallelPort.h"
#include "Led.h"
#include "ErrCode.h"
#include "MFRC500.h"
#include "PcComm.h"


/**
  * @brief  主函数
  * @param  无
  * @return 无
  */
int main(void)
{
	// 	配置SysTick为每1us中断一次
	SysTick_Init();
	
	//	USART 串口 GPIO配置，工作模式配置，中断配置，参数115200 8-N-1
	USART_Config(115200);
	
	//	配置与 MFRC500 连接的并行接口，用于输出控制信号
	ParallelPortInit();
	
	//	配置 LED IO 口，用于显示初始化状态
	LED_GPIO_Config();

	//	上电一段时间后再进行相关其他配置和操作
	Delay_ms(500);
	
	//	reset RC632，使用线性地址，初始并行接口类型，关闭天线
	if(MI_OK == RC500Init()){
		//testBlockWrite();
		
		//test_long2bytes();
		
		//testWalletApps();
		//USART_TransmitOne(0x23);
		while(1){
			Mfrc_Loop();
			//testBlockRead();
		}
		//testHW();
	}
	else{
	}
}
