#include "USART.h"
#include "stm32f10x.h"
#include "misc.h"

 /**
  * @brief USART GPIO ���ã��ж����ã�����ģʽ���á�115200 8-N-1
  * @param ��
  * @retval ��
  */
void USART_Config(unsigned int baudRate){
	/*
	 *	1. ʹ�� USART1 ��ʱ�ӡ�
	 *	2. ���� USART1 �� I/O��
	 *	3. ���� USART1 �Ĺ���ģʽ������Ϊ������Ϊ115200��8������λ��1��ֹͣλ����Ӳ��������(115200 8-N-1)��
	 */
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStruct;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;// Configure the NVIC Preemption Priority Bits  
	
	// Configure USART1 clock����ʹ�ø��ù��ܵ�ʱ��Ҫ������Ӧ�Ĺ���ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	// UASRT GPIO configure
	// Configure Tx(PA9) as alternate function push-pull(�����������)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// Configure Rx(PA10) as as input floating 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// Configure USART1 Mode
	USART_InitStructure.USART_BaudRate = baudRate;	// ����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	// 8 λ����
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	// ��֡��β���� 1 ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;	// ������żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	// Ӳ��������ʧ��
	// ����˫��ȫ˫��ͨ�ţ���Ҫ�� Rx �� Tx ģʽ������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	// ���� USART1 ʱ��
	USART_ClockInitStruct.USART_Clock = USART_Clock_Disable;  // ʱ�ӵ͵�ƽ�
	USART_ClockInitStruct.USART_CPOL = USART_CPOL_Low;  // SLCK ������ʱ������ļ���->�͵�ƽ
	USART_ClockInitStruct.USART_CPHA = USART_CPHA_2Edge;  // ʱ�ӵڶ������ؽ������ݲ���
	USART_ClockInitStruct.USART_LastBit = USART_LastBit_Disable; // ���һλ���ݵ�ʱ�����岻�� SCLK ���
	
	// ��Ĵ�����д�����ò���
	USART_Init(USART1, &USART_InitStructure);
	USART_ClockInit(USART1, &USART_ClockInitStruct);
	
	//ʹ�� USART3 �����ж�
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	// Enable the USARTy Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	// �� STM32 �����úô���֮�󣬷������ݣ���һ�������Ƿ�����ȥ�ģ���ʱ����Cortex-M3���������
	USART_ClearFlag(USART1, USART_FLAG_TC);
	// ��ʹ������ʱ������Ҫʹ����ʱ�ӣ���Ҫʹ������ſ�������ʹ��
	USART_Cmd(USART1, ENABLE);
}

 /**
  * @brief printf��һ���꣬���յ�����fputc()�������,����C�⺯��printf��USART
  * @param 
  * @retval 
  */
int fputc(int ch, FILE *f){
	//	����һ���ֽ����ݵ�USART
	USART_SendData(USART1, (unsigned char)ch);
	//	�ȴ��������
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
	
	if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET){			// ���
			// ע�⣡����ʹ��if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)���ж�
			// http://bbs.21ic.com/icview-160999-1-1.html
        USART_ReceiveData(USART1);
    }
	
	// �����ж� (���ռĴ����ǿ�) 
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
			else if(RxCounter >= 28){	// Ŀǰ��಻����28(д��)���ֽ�
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
