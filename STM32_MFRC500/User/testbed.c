#include "testbed.h"
#include "ErrCode.h"
#include "Apps.h"
#include "USART.h"
#include "Led.h"
#include "SysTick.h"
#include "Utils.h"
#include "ParallelPort.h"
#include "MFRC500.h"

// 获取卡号 UID 测试范例
void testGetUID(void){
	unsigned char len = 0;
	unsigned char uid[10];					// 卡号，最多 10 个字节(tripple UID Size)
	signed char status = MI_OK;
	
	if(MI_OK == (status = ComM1RequestA(uid, &len))){
		USART_Transmit(uid, len);
		
		//成功获取卡号LED1常亮，否则闪烁
		for(;;){
			LED1_ON;
		}
	}else{
		USART_TransmitOne(0xFF);
		USART_TransmitOne(0x00);
		USART_TransmitOne(status);
		USART_TransmitOne(0x00);
		USART_TransmitOne(0xFF);
		
		// 操作异常 LED1 闪烁
		for(;;){
			LED1_ON;
			Delay_ms(500);
			LED1(OFF);
			Delay_ms(500);
		}
	}
}


// 读块操作测试范例
void testBlockRead(void){
	signed char status = MI_OK;
	
	unsigned char keys[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char keyAB = 0xA;
	unsigned char rdata[16] = {0};
	unsigned char sector = 15;
	unsigned char block;
	
	for(block=0; block<4; block++){
		if(MI_OK == (status = ComM1BlockRead(sector, block, keyAB, keys, rdata))){
			USART_TransmitOne(block+1);
			USART_Transmit(rdata, 16);
		}
		else{
			break;
		}
	}
	if(block == 4){
		// 成功整个扇区数据，LED1常亮，否则闪烁
		for(;;){
			LED1_ON;
		}
	}else{
		USART_TransmitOne(0xFF);
		USART_TransmitOne(0x00);
		USART_TransmitOne(status);	
		USART_TransmitOne(0x00);
		USART_TransmitOne(0xFF);
		
		// 操作异常 LED1 闪烁
		for(;;){
			LED1_ON;
			Delay_ms(500);
			LED1(OFF);
			Delay_ms(500);
		}
	}
}

// 写块操作测试范例
void testBlockWrite(void){
	signed char status = MI_OK;
	
	unsigned char keys[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char keyAB = 0xA;
	unsigned char wdata[3][16] = {
		{	
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
		},
		{
			0x19, 0x93, 0x00, 0x00, 0x10, 0x09, 0x00, 0x00,
			0x19, 0x93, 0x00, 0x00, 0x08, 0x24, 0x00, 0x00
		},
		{
			0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
			0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
		}
	};
	unsigned char sector = 15;
	unsigned char block;
	
	for(block=0; block<3; block++){
		if(MI_OK == (status = ComM1BlockWrite(sector, block, keyAB, keys, wdata[2-block]))){
			
		}
		else{
			break;
		}
	}
		
	if(block == 3){
		// 成功写入数据
		testBlockRead();
	}else{
		USART_TransmitOne(0xFF);
		USART_TransmitOne(0x00);
		USART_TransmitOne(status);
		USART_TransmitOne(0x00);
		USART_TransmitOne(0xFF);
	
		// 操作异常 LED1 闪烁
		for(;;){
			LED1_ON;
			Delay_ms(500);
			LED1(OFF);
			Delay_ms(500);
		}
	}
}

void test_long2bytes(void)
{
	long value;
	unsigned char bytes[4] = {0};
	
	value = 100;
	long2bytes(value, bytes);
	USART_Transmit(bytes, 4);
	
	value = -100;
	long2bytes(value, bytes);
	USART_Transmit(bytes, 4);
	
	value = 1;
	long2bytes(value, bytes);
	USART_Transmit(bytes, 4);
	
	value = -1;
	long2bytes(value, bytes);
	USART_Transmit(bytes, 4);
	
}

void testWalletApps(void)
{
	signed char status = MI_OK;
	
	unsigned char keys[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char keyAB = 0xA;
	unsigned char sector = 15;
	unsigned char block = 0;
	
	long money_long = 100;
	unsigned char money_byte[4] = {0};
	unsigned char checkVal[4] = {0};
	unsigned char DecreVal[4] = {0};
	unsigned char IncreVal[4] = {0};
	
	long2bytes(money_long, money_byte);
	USART_TransmitOne(0x01);
	USART_Transmit(money_byte, 4);
	
	// 初始化钱包余额为 100
	status = ComM1WalletInit(sector, block, keyAB, keys, money_byte);
	
	if(MI_OK == status){
		status = ComM1WalletCheck(sector, block, keyAB, keys, checkVal);
		
		if(MI_OK == status){
			USART_TransmitOne(0x02);
			USART_Transmit(checkVal, 4);
			
			long2bytes(25, IncreVal);
			USART_TransmitOne(0x03);
			USART_Transmit(IncreVal, 4);
			
			// 充值 25
			status = ComM1WalletRecharge(sector, block, keyAB, keys, IncreVal);
			
			if(MI_OK == status){
				ComM1WalletCheck(sector, block, keyAB, keys, checkVal);
				
				USART_TransmitOne(0x04);
				USART_Transmit(checkVal, 4);
				
				long2bytes(75, DecreVal);
				USART_TransmitOne(0x05);
				USART_Transmit(DecreVal, 4);
				
				// 消费 75
				status = ComM1WalletDebit(sector, block, keyAB, keys, DecreVal);
				
				if(MI_OK == status){
					ComM1WalletCheck(sector, block, keyAB, keys, checkVal);
					
					USART_TransmitOne(0x06);
					USART_Transmit(checkVal, 4);
				}
				else{
					goto test;
				}
			}
			else{
				goto test;
			}
		}
		else{
			goto test;
		}
	}
	test:
	if(MI_OK == status){
		for(;;){
			LED1_ON;
		}
	}
	else{
		USART_TransmitOne(0xFF);
		USART_TransmitOne(0x00);
		USART_TransmitOne(status);
		USART_TransmitOne(0x00);
		USART_TransmitOne(0xFF);
	
		// 操作异常 LED1 闪烁
		for(;;){
			LED1_ON;
			Delay_ms(500);
			LED1(OFF);
			Delay_ms(500);
		}
	}
}

void testHW(){
	unsigned char data[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
	unsigned char i;
	
	for(i=0; i<16; i++){
		WriteRawRC(RegFIFOData, data[i]);
	}
	while(ReadRawRC(RegFIFOLength)){
		USART_TransmitOne(ReadRawRC(RegFIFOData));
	}
}
