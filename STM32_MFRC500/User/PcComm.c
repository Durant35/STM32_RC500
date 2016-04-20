#include "PcComm.h"
#include "Led.h"
#include "USART.h"
#include "ErrCode.h"
#include "SysTick.h"
#include "Apps.h"
#include <string.h>

unsigned char success = FALSE;
unsigned char failed = FALSE;

void Mfrc_Loop(void){
	unsigned char i;
	struct UsartData UsartDatai;
	struct UsartData *pi = &UsartDatai;
	usart_process(pi);
	
	if(success){
		success = FALSE;
		send_feedback(pi);
		
		LED1_ON;
		Delay_ms(1000);
	}
	
	if(failed){
		failed = FALSE;
		send_error(pi);
		
		for(i=0; i<3; i++){
			LED1_ON;
			Delay_ms(250);
			LED1_OFF;
			Delay_ms(250);
		}
	}
}

void usart_process(struct UsartData *pi){
	unsigned char expectedParaBytes = 0;
	unsigned char bcc_check = BYTE_START;
	unsigned char i = 0;
	unsigned char sector, block;
	unsigned char keys[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char KeyAB = 0xA;
	unsigned char wdata[16] = {0};
	unsigned char value[4] = {0};
	
	if(!RxComplete){
		return;
	}
	
	if(RxBuf[RxCounter-1] != BYTE_END){
		pi->UartErrCode = USART_NOENDERR;
		goto BuffReset;
	}
	
	pi->UartCmd = RxBuf[0];
	
	// 参数数据与命令是否匹配
	switch(pi->UartCmd){
		case UID_GET:
			expectedParaBytes = 0;
		break;
		case CARD_AUTHENT:
			expectedParaBytes = 8;
		break;
		case BLOCK_READ:
			expectedParaBytes = 9;
		break;
		case BLOCK_WRITE:
			expectedParaBytes = 25;
		break;
		case WALLET_INIT:
			expectedParaBytes = 13;
		break;
		case WALLET_CHECK:
			expectedParaBytes = 9;
		break;
		case WALLET_RECHARGE:
			expectedParaBytes = 13;
		break;
		case WALLET_DEBIT:
			expectedParaBytes = 13;
		break;
		default:
			expectedParaBytes = 0;
		break;
	}
	
	if((RxCounter-4) != expectedParaBytes){		// substract BYTE_CMD、BYTE_LEN、BYTE_XOR、BYTE_END
		pi->UartErrCode = USART_NOMATCH;
		goto BuffReset;
	}
	
	// 奇偶校验计算
	for(i=0; i<RxCounter-2; i++){			// substract BYTE_XOR、BYTE_END
		bcc_check ^= RxBuf[i];
	}
	if(bcc_check != RxBuf[RxCounter-2]){
		pi->UartErrCode = USART_CRCERR;
		goto BuffReset;
	}
	
	switch(pi->UartCmd){
		case UID_GET:
			pi->UartErrCode = ComM1RequestA(pi->UartData, &(pi->UartDataLen));
			break;
		case CARD_AUTHENT:
			sector = RxBuf[2];
			KeyAB = RxBuf[3];
			memcpy(keys, &RxBuf[4], 6);
			pi->UartErrCode = ComM1Authentication(sector, KeyAB, keys);
			
			// 此处为了兼容接收帧格式
			if((pi->UartErrCode) == MI_OK){
				pi->UartDataLen = 1;
				pi->UartData[0] = KeyAB;
			}
			break;
		case BLOCK_READ:
			sector = RxBuf[2];
			block = RxBuf[3];
			KeyAB = RxBuf[4];
			memcpy(keys, &RxBuf[5], 6);
			pi->UartErrCode = ComM1BlockRead(sector, block, KeyAB, keys, pi->UartData);
			if((pi->UartErrCode) == MI_OK){
				pi->UartDataLen = 16;
			}
			break;
		case BLOCK_WRITE:
			sector = RxBuf[2];
			block = RxBuf[3];
			KeyAB = RxBuf[4];
			memcpy(keys, &RxBuf[5], 6);
			memcpy(wdata, &RxBuf[11], 16);
			pi->UartErrCode = ComM1BlockWrite(sector, block, KeyAB, keys, wdata);
			pi->UartDataLen = 0;
			break;
		case WALLET_INIT:
			sector = RxBuf[2];
			block = RxBuf[3];
			KeyAB = RxBuf[4];
			memcpy(keys, &RxBuf[5], 6);
			memcpy(value, &RxBuf[11], 4);
			pi->UartErrCode = ComM1WalletInit(sector, block, KeyAB, keys, value);
			if((pi->UartErrCode) == MI_OK){
				pi->UartErrCode = ComM1WalletCheck(sector, block, KeyAB, keys, pi->UartData);
				pi->UartDataLen = 4;
			}
			break;
		case WALLET_CHECK:
			sector = RxBuf[2];
			block = RxBuf[3];
			KeyAB = RxBuf[4];
			memcpy(keys, &RxBuf[5], 6);
			pi->UartErrCode = ComM1WalletCheck(sector, block, KeyAB, keys, pi->UartData);
			if((pi->UartErrCode) == MI_OK){
				pi->UartDataLen = 4;
			}
			break;
		case WALLET_RECHARGE:
			sector = RxBuf[2];
			block = RxBuf[3];
			KeyAB = RxBuf[4];
			memcpy(keys, &RxBuf[5], 6);
			memcpy(value, &RxBuf[11], 4);
			pi->UartErrCode = ComM1WalletRecharge(sector, block, KeyAB, keys, value);
			if((pi->UartErrCode) == MI_OK){
				pi->UartErrCode = ComM1WalletCheck(sector, block, KeyAB, keys, pi->UartData);
				pi->UartDataLen = 4;
			}
			break;
		case WALLET_DEBIT:
			sector = RxBuf[2];
			block = RxBuf[3];
			KeyAB = RxBuf[4];
			memcpy(keys, &RxBuf[5], 6);
			memcpy(value, &RxBuf[11], 4);
			pi->UartErrCode = ComM1WalletDebit(sector, block, KeyAB, keys, value);
			if((pi->UartErrCode) == MI_OK){
				pi->UartErrCode = ComM1WalletCheck(sector, block, KeyAB, keys, pi->UartData);
				pi->UartDataLen = 4;
			}
			break;
		default:
			pi->UartErrCode = USART_CMDERR;
	}
	
BuffReset:
	if((pi->UartErrCode) == USART_OK){
		success = TRUE;
	}
	else{
		failed = TRUE;
	}
	Uart1_BufReset();
}


void send_feedback(struct UsartData *pi){
	unsigned char bcc_check = BYTE_START;
	unsigned char i = 0;
	
	bcc_check ^= pi->UartErrCode;
	bcc_check ^= pi->UartDataLen;
	for(i=0; i<(pi->UartDataLen); i++){
		bcc_check ^= pi->UartData[i];
	}
	
	USART_TransmitOne(BYTE_START);
	USART_TransmitOne(pi->UartErrCode);
	USART_TransmitOne(pi->UartDataLen);
	USART_Transmit(pi->UartData, pi->UartDataLen);
	USART_TransmitOne(bcc_check);
	USART_TransmitOne(BYTE_END);
}

void send_error(struct UsartData *pi){
	unsigned char bcc_check = BYTE_START;
	
	bcc_check ^= pi->UartErrCode;
	
	USART_TransmitOne(BYTE_START);
	USART_TransmitOne(pi->UartErrCode);
	USART_TransmitOne(bcc_check);
	USART_TransmitOne(BYTE_END);
}
