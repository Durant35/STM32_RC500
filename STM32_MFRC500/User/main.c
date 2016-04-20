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
  * @brief  ������
  * @param  ��
  * @return ��
  */
int main(void)
{
	// 	����SysTickΪÿ1us�ж�һ��
	SysTick_Init();
	
	//	USART ���� GPIO���ã�����ģʽ���ã��ж����ã�����115200 8-N-1
	USART_Config(115200);
	
	//	������ MFRC500 ���ӵĲ��нӿڣ�������������ź�
	ParallelPortInit();
	
	//	���� LED IO �ڣ�������ʾ��ʼ��״̬
	LED_GPIO_Config();

	//	�ϵ�һ��ʱ����ٽ�������������úͲ���
	Delay_ms(500);
	
	//	reset RC632��ʹ�����Ե�ַ����ʼ���нӿ����ͣ��ر�����
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
