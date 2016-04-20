#include "Utils.h"
#include "ErrCode.h"

/**
  * @brief  将 Mifare_One 卡密钥转换为 RC500 接收格式
  * @param  
  *		pUncoded:	6 字节未转换的密钥
  *		pCoded:		12 字节转换后的密钥
  * @return 
  *		操作状态码
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ChangeCodeKey(unsigned char *pUncoded,unsigned char *pCoded)
{
   unsigned char cnt = 0;
   unsigned char ln = 0;				// low nibble，低半字节
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
  * @brief  带符号 4 字节值转化为 4 字节数据(低位在前)
  * @param  
  *		lVal:	带符号 4 字节值(long)
  *		bytes:	4 字节数据(低位在前)
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
