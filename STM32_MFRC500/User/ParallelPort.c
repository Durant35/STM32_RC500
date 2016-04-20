#include "ParallelPort.h"
#include "stm32f10x.h"

/**
  * @brief  ��ʼ�� STM32 �� RC500 ���� ���нӿ�
  * @param  none
  * @return none
  * @global none
  * @attention: 
  *		���뿪��ʱ�ӣ������������Ϊ�������
  */
void ParallelPortInit(){
	/*********************************** Ӳ������ ***********************************
	 *
	 *			���õ�ַ/��������D0...D7		<---->	PC0...PC7(in/out)	��������/�������
	 *						NCSƬѡ�ź�		<---->	PC8(out)			�������
	 *					  ALE��ַ�����ź�	<---->	PB8(out)			�������
	 *					  NRD��ʹ���ź�		<---->	PC10(out)			�������
	 *					  NWRдʹ���ź�		<---->	PC11(out)			�������
	 *					  	RST��λ�ź�		<---->	PB12(out)			�������
	 *
	 ********************************************************************************/
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	// �ٶ�����̫�ߣ��ᵼ����������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  	// �������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  	// �������
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// �Ƿ���Ҫ��PC0...PC7Ԥ��Ϊ�������룬�������...
}

/**
  * @brief  ��ȡ RC500 �Ĵ�������
  * @param  �Ĵ������Ե�ַ
  * @return �洢�ڼĴ����е�����
  * @global none
  * @attention: 
  *		�����������Ĵ������Ե�ַ
  *		���������ȡ�洢�ڼĴ����е�����
  */
unsigned char ReadRawRC(unsigned char Address)
{
	unsigned char result;
	unsigned int i;

	// ��ʼ������Ϊ�������ģʽ�����ַ   	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3
								 |GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 	
															// ѡ��7�����Ÿı������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  		// ��������������ַ
	GPIO_Init(GPIOC, &GPIO_InitStructure);	   					

	NCS_H;      											// ��ʼ�����ź�λ��Ч��ƽ
	ALE_L; 
	NWR_H; 
	NRD_H;

	//���ַ�ʽ����: GPIOC->ODR |= (Address&0x00FF);   		// PC7~PC0�����ַ
	GPIO_SetBits(GPIOC, (Address&0x00FF)) ;
	GPIO_ResetBits(GPIOC, ((~Address)&0x00FF));
	
	for(i=10;i!=0;i--);										// �ʵ��ӳ��Ա��ֵ�ַ

	ALE_H;													// ��ַ�������ߣ������ַ
	for(i=10;i!=0;i--);										// ALE�����20ns
	ALE_L;													// ��ַ�����ͷţ��������
	
	for(i=10;i!=0;i--);										// ALE��֮��ĵ�ַ����ʱ���8ns
	
	NCS_L;													// ALE��Ч֮��ʹ��CS, Ƭѡ����
	
	// ��ʼ������Ϊ���������ȡ���� 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3
								 |GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 
															// ѡ��7�����Ÿı������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// ����ģʽû���������ٶ�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  	// ��������

	GPIO_Init(GPIOC, &GPIO_InitStructure);	   				// ����̬�������ŷ���
	
	for(i=10;i!=0;i--);										// ALE�͵���д��Ч��15ns
	
	NRD_L;   
	for(i=10;i!=0;i--);										// ��д��Ч�����65ns
	
	result = GPIOC->IDR & 0x00FF;							// �Ͱ�λ���ݣ���ȡ�����ݽ���ʱ���65ns

	NRD_H;													// ��Ч���ź�
	
															// NRD�ߵ�NCS�ߡ�0
	
	NCS_H;													// Ƭѡ�źŸ�λ
	
	return result; 
}  

/**
  * @brief  RC500 �Ĵ���д������
  * @param  
  *		@Address �Ĵ������Ե�ַ
  *		@value	 д������
  * @return none
  * @global none
  * @attention: 
  *		�����������Ĵ������Ե�ַ��д�������
  */
void WriteRawRC(unsigned char Address, unsigned char value)
{  
	unsigned int i;
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3
								 |GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7; 
																// PC0-PC7
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  			// ��������������ַ
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);	   					// ��ʼ�������ַ   

	NCS_H;      												// ��ʼ�ź�
	ALE_L; 
	NWR_H; 
	NRD_H;

	//GPIOC->ODR |= (Address&0x00FF) ;   						// PC7-PC0�����ַ   
	GPIO_SetBits(GPIOC, (Address&0x00FF));
	GPIO_ResetBits(GPIOC, (~Address&0x00FF));
	
	for(i=10;i!=0;i--);											// �ʵ��ӳ��Ա��ֵ�ַ

	ALE_H;														// ��ַ��������
	for(i=10;i!=0;i--);											// ALE�����20ns
	ALE_L;														// ��ַ�����ͷţ��������
	
	for(i=10;i!=0;i--);											// ALE��֮��ĵ�ַ����ʱ���8ns
	
	NCS_L; 														// ѡ��оƬ��Ȼ�������

	// ����������������������һֱ������� 
	//GPIOC->ODR |= (value&0x00FF);								// PC7~PC0�������
	GPIO_SetBits(GPIOC, (value&0x00FF)); 
	GPIO_ResetBits(GPIOC, (~value&0x00FF));
	
	for(i=10;i!=0;i--);											// ALE�͵���д��Ч��15ns
	//for(i=10;i!=0;i--);										// NCS�͵�NRD/NWR�͡�0
	
	NWR_L;														// ��д�ź���Ч�����65ns
	for(i=10;i!=0;i--);											// �ȴ�д��
	
	NWR_H;														// д�����
	for(i=10;i!=0;i--);											// NWR��֮�󱣳�ʱ���8ns
	NCS_H;
}
