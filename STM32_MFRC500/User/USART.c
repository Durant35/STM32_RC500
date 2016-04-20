#include "USART.h"
#include "stm32f10x.h"
#include "misc.h"

 /**
  * @brief USART GPIO 配置，中断配置，工作模式配置。115200 8-N-1
  * @param 无
  * @retval 无
  */
void USART_Config(unsigned int baudRate){
	/*
	 *	1. 使能 USART1 的时钟。
	 *	2. 配置 USART1 的 I/O。
	 *	3. 配置 USART1 的工作模式，具体为波特率为115200、8个数据位、1个停止位、无硬件流控制(115200 8-N-1)。
	 */
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStruct;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;// Configure the NVIC Preemption Priority Bits  
	
	// Configure USART1 clock，在使用复用功能的时候，要开启相应的功能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	// UASRT GPIO configure
	// Configure Tx(PA9) as alternate function push-pull(复用推挽输出)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// Configure Rx(PA10) as as input floating 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// Configure USART1 Mode
	USART_InitStructure.USART_BaudRate = baudRate;	// 波特率配置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	// 8 位数据
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	// 在帧结尾传输 1 个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;	// 禁用奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// 硬件流控制失能
	// 配置双线全双工通信，需要把 Rx 和 Tx 模式都开启
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	// 配置 USART1 时钟
	USART_ClockInitStruct.USART_Clock = USART_Clock_Disable;  // 时钟低电平活动
	USART_ClockInitStruct.USART_CPOL = USART_CPOL_Low;  // SLCK 引脚上时钟输出的极性->低电平
	USART_ClockInitStruct.USART_CPHA = USART_CPHA_2Edge;  // 时钟第二个边沿进行数据捕获
	USART_ClockInitStruct.USART_LastBit = USART_LastBit_Disable; // 最后一位数据的时钟脉冲不从 SCLK 输出
	
	// 向寄存器中写入配置参数
	USART_Init(USART1, &USART_InitStructure);
	USART_ClockInit(USART1, &USART_ClockInitStruct);
	
	//使能 USART3 接收中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	// Enable the USARTy Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	// 在 STM32 中配置好串口之后，发送数据，第一个数据是发不出去的，这时由于Cortex-M3本身的问题
	USART_ClearFlag(USART1, USART_FLAG_TC);
	// 在使用外设时，不仅要使能其时钟，还要使能外设才可以正常使用
	USART_Cmd(USART1, ENABLE);
}

 /**
  * @brief printf是一个宏，最终调用了fputc()这个函数,定向C库函数printf到USART
  * @param 
  * @retval 
  */
int fputc(int ch, FILE *f){
	//	发送一个字节数据到USART
	USART_SendData(USART1, (unsigned char)ch);
	//	等待发送完毕
	while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TC));
	return (ch);
}

void USART_TransmitOne(unsigned char data){
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_SendData(USART1, data);
	while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TC))
		;
}

void USART_Transmit(unsigned char data[], unsigned char len){
	unsigned char i;
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
	for(i=0; i<len; i++){
		USART_SendData(USART1, data[i]);
		while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TC))
			;
	}
}

unsigned char RxBuf[RX_BUFFER_SIZE];
unsigned char RxCounter = 0;
unsigned char RxComplete = FALSE;
unsigned char cmdStart = FALSE;
unsigned char cmdLen = RX_BUFFER_SIZE;
/**
  * @brief  This function handles USART1 interrupt request.
  * @param  None
  * @retval None
  * @global 
  */
void Uart1_IRQHandler(void)
{
	unsigned char RxByte = 0x00;
	
	if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET){			// 溢出
			// 注意！不能使用if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)来判断
			// http://bbs.21ic.com/icview-160999-1-1.html
        USART_ReceiveData(USART1);
    }
	
	// 接收中断 (接收寄存器非空) 
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
        // Clear the USART1 Receive interrupt
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		
		// unsigned char com_data = USART1->DR;
		
		// Read one byte from the receive data register
        RxByte = USART_ReceiveData(USART1);
		
		if((cmdStart == FALSE) && (RxByte == BYTE_START)){
			cmdStart = TRUE;
			RxCounter = 0;
			cmdLen = RX_BUFFER_SIZE;
		}
		else if(cmdStart == TRUE){
			RxBuf[RxCounter++] = RxByte;
			
			if(RxCounter == 2){
				cmdLen = RxByte + 4;	// cmd code + parameter number + Xor byte + end byte
			}
			else if(RxCounter == cmdLen){
				RxComplete = TRUE;
			}
			else if(RxCounter >= 28){	// 目前最多不超过28(写块)个字节
				RxCounter = 28;
			}
		}
	}
}

/**
  * @brief  This function handles USART1 interrupt request.
  * @param  None
  * @retval None
  * @global 
  */
void Uart1_BufReset(void){
	RxComplete = FALSE;
	
	cmdStart = FALSE;
	RxCounter = 0;
	cmdLen = RX_BUFFER_SIZE;
}
