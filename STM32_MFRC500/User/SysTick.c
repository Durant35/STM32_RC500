#include "SysTick.h"

static __IO u32 TimingDelay;

/**
  * @brief  ����ϵͳ�δ�ʱ�� SysTick
  * @param  ��
  * @return ��
  */
void SysTick_Init(void){
	/*
	 *	SystemFrequency/1000		1ms�ж�1��
	 *	SystemFrequency/100000		10us�ж�1��
	 *	SystemFrequency/1000000		1us�ж�1��(��СΪ1/72 ��s)
	 *		ʹ��ʱ��ԴΪ AHBʱ�ӣ���Ƶ��(SystemFrequency)������Ϊ 72MHz�����ú���ʱ����ticks ��ֵΪ: 
	 *						ticks = SystemFrequency/10000 = 720����ʾ 720��ʱ�������ж�һ�Σ�
	 *		1/f ��ʱ�����ڵ�ʱ�䣬��ʱ1/f =1/72 ��s���������ն�ʱ��ʱ��: 
	 *						T=720����1/72����Ϊ720��ʱ�����ڣ������� 10��s��
	 */
	// ������SysTick_Config()����SysTick���ɹ����������ѭ��
	// ��ʼ��SysTick�ɹ����ȹرն�ʱ��������Ҫ��ʱ���ٿ���
	if(SysTick_Config(SystemCoreClock/1000000)){
		while(1);
	}
	// �رյδ�ʱ��������Ҫ��ʱ���ٿ���
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

/**
  * @brief  us��ʱ����
  * @param  
  *		@arg uTime: ��ʱuTime(us)
  * @return ��
  * @attention: 
  *		һ�����ǵ�����Delay_us()������SysTick��ʱ���ͱ������������趨�õĶ�ʱ����
  *	�ݼ���������SysTick�ļ����Ĵ�����ֵ��Ϊ 0 ʱ���ͽ����жϺ��������жϺ���ִ����
  * ��֮�����¼�ʱ�����ѭ�������������ر�.
  *		��С��ʱ����1/72000000 ��
  */
void Delay_us(__IO u32 uTime){
	TimingDelay = uTime;
	// ʹ�ܵδ�ʱ��
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	
	while(TimingDelay>0)
		;
}

/**
  * @brief  ��ȡ���ĳ���
  * @param  ��
  * @return ��
  * @attention: 
  *		��SysTick�жϺ���SysTick_Handler�����е���
  */
void TimingDelay_Decrement(void){
	if(TimingDelay != 0x00){
		TimingDelay--;
	}
}

/**
  * @brief  ns��ʱ����
  * @param  ��
  * @return ��
  * @attention: 
  *		__nopִ����Ҫ13.9ns
  */
void Delay_14ns(__IO u32 ns){
  u32 i;
  for(i=0;i<ns;i++){
    __nop();
  }
}

