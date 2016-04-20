#include "ParallelPort.h"
#include "stm32f10x.h"

/**
  * @brief  初始化 STM32 与 RC500 连接 并行接口
  * @param  none
  * @return none
  * @global none
  * @attention: 
  *		必须开启时钟，输出引脚配置为推挽输出
  */
void ParallelPortInit(){
	/*********************************** 硬件连接 ***********************************
	 *
	 *			复用地址/数据总线D0...D7		<---->	PC0...PC7(in/out)	浮空输入/推挽输出
	 *						NCS片选信号		<---->	PC8(out)			推挽输出
	 *					  ALE地址锁存信号	<---->	PB8(out)			推挽输出
	 *					  NRD读使能信号		<---->	PC10(out)			推挽输出
	 *					  NWR写使能信号		<---->	PC11(out)			推挽输出
	 *					  	RST复位信号		<---->	PB12(out)			推挽输出
	 *
	 ********************************************************************************/
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// 速度配置太高，会导致噪声过大
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  	// 推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  	// 推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// 是否需要将PC0...PC7预设为浮空输入，避免干扰...
}

/**
  * @brief  读取 RC500 寄存器数据
  * @param  寄存器线性地址
  * @return 存储在寄存器中的数据
  * @global none
  * @attention: 
  *		推挽输出输出寄存器线性地址
  *		浮空输入读取存储在寄存器中的数据
  */
unsigned char ReadRawRC(unsigned char Address)
{
	unsigned char result;
	unsigned int i;

	// 初始化并口为推挽输出模式输出地址   	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3
								 |GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 	
															// 选中7个引脚改变输入输出方向
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  		// 推挽输出，输出地址
	GPIO_Init(GPIOC, &GPIO_InitStructure);	   					

	NCS_H;      											// 初始控制信号位无效电平
	ALE_L; 
	NWR_H; 
	NRD_H;

	//这种方式不好: GPIOC->ODR |= (Address&0x00FF);   		// PC7~PC0输出地址
	GPIO_SetBits(GPIOC, (Address&0x00FF)) ;
	GPIO_ResetBits(GPIOC, ((~Address)&0x00FF));
	
	for(i=10;i!=0;i--);										// 适当延迟以保持地址

	ALE_H;													// 地址锁存拉高，锁存地址
	for(i=10;i!=0;i--);										// ALE脉宽≥20ns
	ALE_L;													// 地址锁存释放，锁存完成
	
	for(i=10;i!=0;i--);										// ALE低之后的地址保持时间≥8ns
	
	NCS_L;													// ALE无效之后使能CS, 片选拉低
	
	// 初始化并口为浮空输入获取数据 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3
								 |GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 
															// 选中7个引脚改变输入输出方向
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// 输入模式没有最大输出速度限制
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  	// 浮空输入

	GPIO_Init(GPIOC, &GPIO_InitStructure);	   				// 高阻态更改引脚方向
	
	for(i=10;i!=0;i--);										// ALE低到读写有效≥15ns
	
	NRD_L;   
	for(i=10;i!=0;i--);										// 读写有效脉宽≥65ns
	
	result = GPIOC->IDR & 0x00FF;							// 低八位数据，获取的数据建立时间≤65ns

	NRD_H;													// 无效读信号
	
															// NRD高到NCS高≥0
	
	NCS_H;													// 片选信号复位
	
	return result; 
}  

/**
  * @brief  RC500 寄存器写入数据
  * @param  
  *		@Address 寄存器线性地址
  *		@value	 写入数据
  * @return none
  * @global none
  * @attention: 
  *		推挽输出输出寄存器线性地址和写入的数据
  */
void WriteRawRC(unsigned char Address, unsigned char value)
{  
	unsigned int i;
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3
								 |GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 
																// PC0-PC7
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  			// 推挽输出，输出地址
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);	   					// 初始化输出地址   

	NCS_H;      												// 初始信号
	ALE_L; 
	NWR_H; 
	NRD_H;

	//GPIOC->ODR |= (Address&0x00FF) ;   						// PC7-PC0输出地址   
	GPIO_SetBits(GPIOC, (Address&0x00FF));
	GPIO_ResetBits(GPIOC, (~Address&0x00FF));
	
	for(i=10;i!=0;i--);											// 适当延迟以保持地址

	ALE_H;														// 地址锁存拉高
	for(i=10;i!=0;i--);											// ALE脉宽≥20ns
	ALE_L;														// 地址锁存释放，锁存完成
	
	for(i=10;i!=0;i--);											// ALE低之后的地址保持时间≥8ns
	
	NCS_L; 														// 选中芯片，然后给数据

	// 无需更改引脚输入输出方向，一直输出数据 
	//GPIOC->ODR |= (value&0x00FF);								// PC7~PC0输出数据
	GPIO_SetBits(GPIOC, (value&0x00FF)); 
	GPIO_ResetBits(GPIOC, (~value&0x00FF));
	
	for(i=10;i!=0;i--);											// ALE低到读写有效≥15ns
	//for(i=10;i!=0;i--);										// NCS低到NRD/NWR低≥0
	
	NWR_L;														// 读写信号有效脉宽≥65ns
	for(i=10;i!=0;i--);											// 等待写入
	
	NWR_H;														// 写入完成
	for(i=10;i!=0;i--);											// NWR高之后保持时间≥8ns
	NCS_H;
}
