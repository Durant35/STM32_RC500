#include "Utils.h"
#include "ErrCode.h"

/**
  * @brief  �� Mifare_One ����Կת��Ϊ RC500 ���ո�ʽ
  * @param  
  *		pUncoded:	6 �ֽ�δת������Կ
  *		pCoded:		12 �ֽ�ת�������Կ
  * @return 
  *		����״̬��
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ChangeCodeKey(unsigned char *pUncoded,unsigned char *pCoded)
{
   unsigned char cnt = 0;
   unsigned char ln = 0;				// low nibble���Ͱ��ֽ�
   unsigned char hn = 0;				// high nibble

   for(cnt=0; cnt<6; cnt++){
      ln = pUncoded[cnt] & 0x0F;
      hn = pUncoded[cnt] >> 4;
      pCoded[cnt*2+1] = (~ln<<4) | ln;
      pCoded[cnt*2]   = (~hn<<4) | hn;
   }
   return MI_OK;
}

/**
  * @brief  ������ 4 �ֽ�ֵת��Ϊ 4 �ֽ�����(��λ��ǰ)
  * @param  
  *		lVal:	������ 4 �ֽ�ֵ(long)
  *		bytes:	4 �ֽ�����(��λ��ǰ)
  * @return 
  *		none
  * @attention
  *		none
  * @global 
  *		none
  */
void long2bytes(long lVal, unsigned char *bytes)
{
	unsigned char i;
	for(i=0; i<4; i++){
		bytes[i] = (lVal >> (8*i)) & 0xFF;
	}
}
