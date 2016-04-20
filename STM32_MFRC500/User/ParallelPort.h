#ifndef _PARALLEL_PORT_H_
#define _PARALLEL_PORT_H_

// 并行接口硬件操作
#define RST (1<<12) 			// PB12
#define WR (1<<11)  			// PC11
#define RD (1<<10)  			// PC10
#define CS (1<<8)   			// PC8
#define ALE (1<<8)  			// PB8

#define RST_L GPIOB->ODR &= ~RST
#define RST_H GPIOB->ODR |= RST
					  
#define NWR_L GPIOC->ODR &= ~WR
#define NWR_H GPIOC->ODR |= WR
					  
#define NRD_L GPIOC->ODR &= ~RD
#define NRD_H GPIOC->ODR |= RD
					  
#define NCS_L GPIOC->ODR &= ~CS 
#define NCS_H GPIOC->ODR |= CS
					  
#define ALE_L GPIOB->ODR &= ~ALE
#define ALE_H GPIOB->ODR |= ALE


// STM32 并行接口管脚配置
void ParallelPortInit(void);
// 写寄存器
void WriteRawRC(unsigned char Address,unsigned char value);
// 读寄存器                     
unsigned char ReadRawRC(unsigned char Address); 

#endif
