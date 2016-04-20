#ifndef __USART_H__
#define __USART_H__

#include <stdio.h>
#include "PcComm.h"

void USART_Config(unsigned int baudRate);

int fputc(int ch, FILE *f);

void USART_TransmitOne(unsigned char data);
void USART_Transmit(unsigned char data[], unsigned char len);
void Uart1_IRQHandler(void);
void Uart1_BufReset(void);

extern unsigned char RxBuf[RX_BUFFER_SIZE];
extern unsigned char RxCounter;
extern unsigned char RxComplete;

#endif
