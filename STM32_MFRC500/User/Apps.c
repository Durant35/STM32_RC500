#include "Apps.h"
#include "ISO14443A.h"
#include "ErrCode.h"
#include "MFRC500.h"
#include "Utils.h"

//#define __RECHARGE_DEBUG__


 /**
  * @brief   Ѱ Mifare_One ������ͻ��飬��ȡ�� UID
  * @param  
  *		unsigned char* TypeA_Uid:	Mifare_One �� UID
  *		unsigned char* len:			UID ��Ӧ�ĳ���
  * @return 
  *		ִ��״̬
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ComM1RequestA(unsigned char* TypeA_Uid, unsigned char* len)
{
    unsigned char atq[2];
	signed char status;		// ��������Ϊ char ��Ȼ�Ƚ�ʱ����ֻ���
				  
	/*
	 *  Ѱ����Ҫ������Ѱ������Ƶ��һ����Χ���Ƿ���� M1 ���� 
	 *	�� Mifare �����ڶ�д�������߹�����Χ֮��ʱ��
	 *		���������ƵоƬ��Ƭ���� REQUEST  ALL(�� REQUEST  IDLE)�����ʱ��Ƭ�� ATR ����
	 *		����������Ƭ Block0 �еĿ�Ƭ����(TagType)���������ֽ����ݴ��͵���д����
	 *	�Ӷ�������Ƭ���д���ĵ�һ��ͨ������
	 */
	status = PiccRequest(PICC_REQALL, atq);		// һ���һ�ξ��Ѿ��ɹ�(�޳�ͻ)
	if (MI_OK != status){
		
		#ifdef __MYDEBUG__
		USART_TransmitOne((unsigned char)0x01);
		USART_TransmitOne(status);
		#endif
		
		if(MI_COLLERR == status){				// �˴���ͻʱ���� MI_COLLERR��ֱ�ӽ��� ��������ͻ
												// ��ʾ��ϢΪ Ѱ����ͻ
			atq[0] = atq[1] = 0xFF;
			goto CascAnticoll;
		}
		else{
			status = PiccRequest(PICC_REQALL, atq);
		}
	}
	
	#ifdef __MYDEBUG__
	USART_TransmitOne((unsigned char)0x02);
	USART_TransmitOne(status);
	#endif
	
	// û�п�Ƭ�Ļ������� MI_BITCOUNTERR������16-bit��TagType����
	if(MI_OK == status){
		CascAnticoll:
		// atq[0] .. LSByte; atq[1] .. MSByte
		// M1 �����ݷ��أ��ȴ�����ֽ�����
		// ���ڴ��䣬�ȷ��͸�λ�ֽڣ��ٷ��͵�λ�ֽ�
		#ifdef __MYDEBUG__
		USART_TransmitOne(0x03);
		USART_TransmitOne(atq[1]);
		USART_TransmitOne(atq[0]);
		#endif
		
		status = PiccCascAnticollision(TypeA_Uid, len);
		
		#ifdef __MYDEBUG__
		USART_TransmitOne((unsigned char)0x04);
		USART_TransmitOne(status);
		#endif
	}
	
	return status;
}


/**
  * @brief   Mifare_One ��������Կ��֤
  * @param  
  *		SectorNum:	������(0x0-0x0F)
  *		KeyAB:		��Կ���ͣ�0x0A ��ʾ Type-A��0x0B ��ʾ Type-B
  *		pKeys:		��Կ(6 Bytes)
  * @return 
  *		����״̬��
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ComM1Authentication(unsigned char SectorNum, unsigned char KeyAB, unsigned char *pKeys){
	signed char status = MI_OK;
	unsigned char atq[2];
	unsigned char Uid_len;
	unsigned char TypeA_Uid[4];
	unsigned char pCodedKeys[12];
	unsigned char auth_mode;
	unsigned char BlockNum = 0;
	unsigned char block;

	status = PiccRequest(PICC_REQALL, atq);							// Ѱ���� A ��
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// ����ͻ����ȡ UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// ת����Կ��ʽ ��λ�����͵�6���ֽ�Ϊԭʼ��Կ��ת������� ����12�ֽ�
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// ������Կ�� RC500 �ܳ׻�����
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
				
				for(BlockNum = 0; BlockNum < 4; BlockNum++){
					block = SectorNum * 4 + BlockNum;
					status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// ��֤��Կ
					if(MI_OK != status){
						break;
					}
				}
			}
		}
	}
	return status;
}

/**
  * @brief  ��ȡ Mifare_One ����ָ��������
  * @param  
  *		SectorNum:	������(0x0-0x0F)
  *		BlockNum:	���(0x0-0x03)
  *		KeyAB:		��Կ���ͣ�0x0A ��ʾ Type-A��0x0B ��ʾ Type-B
  *		pKeys:		��Կ(6 Bytes)
  *		pReadData:	��ȡ�Ŀ�����(16 Bytes)
  * @return 
  *		����״̬��
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ComM1BlockRead(unsigned char SectorNum, unsigned char BlockNum,
			unsigned char KeyAB, unsigned char *pKeys, unsigned char *pReadData){
	signed char status = MI_OK;
	unsigned char atq[2];
	unsigned char Uid_len;
	unsigned char TypeA_Uid[4];
	unsigned char pCodedKeys[12];
	unsigned char auth_mode;
	unsigned char block = SectorNum * 4 + BlockNum;

	status = PiccRequest(PICC_REQALL, atq);							// Ѱ���� A ��
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// ����ͻ����ȡ UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// ת����Կ��ʽ ��λ�����͵�6���ֽ�Ϊԭʼ��Կ��ת������� ����12�ֽ�
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// ������Կ�� RC500 �ܳ׻�����
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// ��֤��Կ

				if(MI_OK == status){
					status = PiccRead(block, pReadData);		 
													// �� Mifare_one����һ��(block)����(16�ֽ�)
				}
        	}	
		}
	}
	
	return status;
}
			
/**
  * @brief  Mifare_One ����ָ����д������
  * @param  
  *		SectorNum:	������(0-15)
  *		BlockNum:	���(0-3)
  *		KeyAB:		��Կ���ͣ�0x0A ��ʾ Type-A��0x0B ��ʾ Type-B
  *		pKeys:		��Կ(6 Bytes)
  *		pWriteData:	Ҫд��Ŀ�����(16 Bytes)
  * @return 
  *		����״̬��
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ComM1BlockWrite(unsigned char SectorNum, unsigned char BlockNum, 
			unsigned char KeyAB, unsigned char *pKeys, unsigned char *pWriteData){
	signed char status = MI_OK;
	unsigned char atq[2];
	unsigned char Uid_len;
	unsigned char TypeA_Uid[4];
	unsigned char pCodedKeys[12];
	unsigned char auth_mode;
	unsigned char block = SectorNum * 4 + BlockNum;

	status = PiccRequest(PICC_REQALL, atq);							// Ѱ���� A ��
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// ����ͻ����ȡ UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// ת����Կ��ʽ ��λ�����͵�6���ֽ�Ϊԭʼ��Կ��ת������� ����12�ֽ�
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// ������Կ�� RC500 �ܳ׻�����
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// ��֤��Կ

				if(MI_OK == status){
					status = PiccWrite(block, pWriteData);		 
													// ������(16�ֽ�)д�� Mifare_one ����һ��(block)
				}
        	}	
		}
	}
	
	return status;
}
			
/**
  * @brief  ��ʼ��Ǯ������
  * @param  
  *		SectorNum:	������(0-15)
  *		BlockNum:	���(0-3)
  *		KeyAB:		��Կ���ͣ�0x0A ��ʾ Type-A��0x0B ��ʾ Type-B
  *		pKeys:		��Կ(6 Bytes)
  *		pInitVal:	��ʼ�����(4 Bytes)
  * @return 
  *		����״̬��
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ComM1WalletInit(unsigned char SectorNum, unsigned char BlockNum, 
					unsigned char KeyAB, unsigned char *pKeys, unsigned char *pInitVal)
{
	signed char status = MI_OK;
	unsigned char atq[2];
	unsigned char Uid_len;
	unsigned char TypeA_Uid[4];
	unsigned char pCodedKeys[12];
	unsigned char auth_mode;
	unsigned char wdata[16] = {0};
	unsigned char block = SectorNum * 4 + BlockNum;

	status = PiccRequest(PICC_REQALL, atq);						// Ѱ����A��
	if(status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);		// ����ͻ����ȡ UID

	if( MI_OK == status ){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// ת����Կ��ʽ ��λ�����͵�6���ֽ�Ϊԭʼ��Կ��ת������� ����12�ֽ�

		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   // ������Կ�� RC500 �ܳ׻�����
			
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
				
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);
																// ��֤��Կ
				if(MI_OK == status){
					/*	�������ݿ������ݴ洢��ʽ
					 *	�ֽں�	0 1 2 3 | 4 5 6 7 | 8 9 10 11 |	12   13    14   15
					 * 	˵��	  ��ֵ	  ��ֵȡ��     ��ֵ	   Adr Adrȡ�� Adr Adrȡ��
					 */
					wdata[0] = pInitVal[0];
					wdata[1] = pInitVal[1];
					wdata[2] = pInitVal[2];
					wdata[3] = pInitVal[3];
					
					wdata[4] = ~pInitVal[0];
					wdata[5] = ~pInitVal[1];
					wdata[6] = ~pInitVal[2];
					wdata[7] = ~pInitVal[3];
					
					wdata[8]  = pInitVal[0];
					wdata[9]  = pInitVal[1];
					wdata[10] = pInitVal[2];
					wdata[11] = pInitVal[3];
					
					wdata[12] = block;
					wdata[13] = ~block;
					wdata[14] = block;
					wdata[15] = ~block;
					
					status = PiccWrite(block, wdata);		 
								// �������������ݿ��ʽ�� 16 �ֽ�����д�� Mifare_one ����һ��(block)  
				}
        	}	
		}
	}
	
	return status;
}

/**
  * @brief  ��ӦǮ������ѯ����
  * @param  
  *		SectorNum:	������(0-15)
  *		BlockNum:	���(0-3)
  *		KeyAB:		��Կ���ͣ�0x0A ��ʾ Type-A��0x0B ��ʾ Type-B
  *		pKeys:		��Կ(6 Bytes)
  *		pInitVal:	��ʼ�����(4 Bytes)
  * @return 
  *		����״̬��
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ComM1WalletCheck(unsigned char SectorNum, unsigned char BlockNum, 
					unsigned char KeyAB, unsigned char *pKeys, unsigned char *pCheckVal)
{
	signed char status = MI_OK;
	unsigned char atq[2];
	unsigned char Uid_len;
	unsigned char TypeA_Uid[4];
	unsigned char pCodedKeys[12];
	unsigned char auth_mode;
	unsigned char rdata[16] = {0};
	unsigned char block = SectorNum * 4 + BlockNum;

	status = PiccRequest(PICC_REQALL, atq);						// Ѱ����A��
	if(status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);		// ����ͻ����ȡ UID

	if( MI_OK == status ){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// ת����Կ��ʽ ��λ�����͵�6���ֽ�Ϊԭʼ��Կ��ת������� ����12�ֽ�

		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   // ������Կ�� RC500 �ܳ׻�����
			
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
				
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);
																// ��֤��Կ
				if(MI_OK == status){
					
					status = PiccRead(block, rdata);		 
								// ��ȡ Mifare_one ����һ�� 16 �ֽ����ݣ��ж��Ƿ�����������ݿ��ʽ

					
					/*	�������ݿ������ݴ洢��ʽ
					 *	�ֽں�	0 1 2 3 | 4 5 6 7 | 8 9 10 11 |	12   13    14   15
					 * 	˵��	  ��ֵ	  ��ֵȡ��     ��ֵ	   Adr Adrȡ�� Adr Adrȡ��
					 */
					//����ΪǮ����ʽ�򷵻�
					if( (rdata[0]== rdata[8]) &&(rdata[1]== rdata[9])
					  &&(rdata[2]== rdata[10]) &&(rdata[3]== rdata[11]) ){
						  rdata[0] = ~rdata[0];
						  rdata[1] = ~rdata[1];
						  rdata[2] = ~rdata[2];
						  rdata[3] = ~rdata[3];
						  
						  if( (rdata[4]==rdata[0]) && (rdata[5]==rdata[1]) 
						  && (rdata[6]==rdata[2]) && (rdata[7]==rdata[3])){	// ��Ӧ��ֵ
							  pCheckVal[0] = rdata[8];
							  pCheckVal[1] = rdata[9];
							  pCheckVal[2] = rdata[10];
							  pCheckVal[3] = rdata[11];
						  }
						  else{
							  status = WALLET_NOTINIT;
						  }
					  }
					  else{
						  status = WALLET_NOTINIT;
					  }
				}
			}
		}
	}
	
	return status;
}
					
/**
  * @brief  ��ӦǮ����ֵ����
  * @param  
  *		SectorNum:	������(0-15)
  *		BlockNum:	���(0-3)
  *		KeyAB:		��Կ���ͣ�0x0A ��ʾ Type-A��0x0B ��ʾ Type-B
  *		pKeys:		��Կ(6 Bytes)
  *		pIncreVal:	��ֵ���(4 Bytes)
  * @return 
  *		����״̬��
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ComM1WalletRecharge(unsigned char SectorNum, unsigned char BlockNum, 
					unsigned char KeyAB, unsigned char *pKeys, unsigned char *pIncreVal)
{
	signed char status = MI_OK;
	unsigned char atq[2];
	unsigned char Uid_len;
	unsigned char TypeA_Uid[4];
	unsigned char pCodedKeys[12];
	unsigned char auth_mode;
	unsigned char block = SectorNum * 4 + BlockNum;

	status = PiccRequest(PICC_REQALL, atq);							// Ѱ���� A ��
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// ����ͻ����ȡ UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// ת����Կ��ʽ ��λ�����͵�6���ֽ�Ϊԭʼ��Կ��ת������� ����12�ֽ�
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// ������Կ�� RC500 �ܳ׻�����
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// ��֤��Կ

				if(MI_OK == status){
					#ifdef __RECHARGE_DEBUG__
					USART_Transmit(pIncreVal, 4);
					#endif
					status = PiccValues(PICC_INCREMENT, block, pIncreVal);		 
																	// ����ֵ��������ִ�г�ֵ
				}
        	}	
		}
	}
	
	return status;
}

/**
  * @brief  ��ӦǮ����������
  * @param  
  *		SectorNum:	������(0-15)
  *		BlockNum:	���(0-3)
  *		KeyAB:		��Կ���ͣ�0x0A ��ʾ Type-A��0x0B ��ʾ Type-B
  *		pKeys:		��Կ(6 Bytes)
  *		pDecreVal:	���ѽ��(4 Bytes)
  * @return 
  *		����״̬��
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ComM1WalletDebit(unsigned char SectorNum, unsigned char BlockNum, 
					unsigned char KeyAB, unsigned char *pKeys, unsigned char *pDecreVal)
{
	signed char status = MI_OK;
	unsigned char atq[2];
	unsigned char Uid_len;
	unsigned char TypeA_Uid[4];
	unsigned char pCodedKeys[12];
	unsigned char auth_mode;
	unsigned char block = SectorNum * 4 + BlockNum;

	status = PiccRequest(PICC_REQALL, atq);							// Ѱ���� A ��
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// ����ͻ����ȡ UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// ת����Կ��ʽ ��λ�����͵�6���ֽ�Ϊԭʼ��Կ��ת������� ����12�ֽ�
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// ������Կ�� RC500 �ܳ׻�����
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// ��֤��Կ

				if(MI_OK == status){
					status = PiccValues(PICC_DECREMENT, block, pDecreVal);		 
																	// ����ֵ��������ִ�пۿ�
				}
        	}	
		}
	}
	
	return status;
}

