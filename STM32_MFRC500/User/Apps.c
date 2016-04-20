#include "Apps.h"
#include "ISO14443A.h"
#include "ErrCode.h"
#include "MFRC500.h"
#include "Utils.h"

//#define __RECHARGE_DEBUG__


 /**
  * @brief   寻 Mifare_One 卡，冲突检查，获取其 UID
  * @param  
  *		unsigned char* TypeA_Uid:	Mifare_One 卡 UID
  *		unsigned char* len:			UID 对应的长度
  * @return 
  *		执行状态
  * @attention
  *		none
  * @global 
  *		none
  */
signed char ComM1RequestA(unsigned char* TypeA_Uid, unsigned char* len)
{
    unsigned char atq[2];
	signed char status;		// 不能声明为 char 不然比较时会出现混乱
				  
	/*
	 *  寻卡主要用于搜寻天线射频场一定范围内是否存在 M1 卡。 
	 *	当 Mifare 卡处在读写器的天线工作范围之内时，
	 *		程序控制射频芯片向卡片发出 REQUEST  ALL(或 REQUEST  IDLE)命令，此时卡片的 ATR 将启
	 *		动，并将卡片 Block0 中的卡片类型(TagType)，共两个字节数据传送到读写器，
	 *	从而建立卡片与读写器的第一步通信联络
	 */
	status = PiccRequest(PICC_REQALL, atq);		// 一般第一次就已经成功(无冲突)
	if (MI_OK != status){
		
		#ifdef __MYDEBUG__
		USART_TransmitOne((unsigned char)0x01);
		USART_TransmitOne(status);
		#endif
		
		if(MI_COLLERR == status){				// 此处冲突时产生 MI_COLLERR，直接进入 级联防冲突
												// 提示信息为 寻卡冲突
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
	
	// 没有卡片的话将产生 MI_BITCOUNTERR，请求16-bit的TagType错误
	if(MI_OK == status){
		CascAnticoll:
		// atq[0] .. LSByte; atq[1] .. MSByte
		// M1 卡数据返回，先传输低字节数据
		// 串口传输，先发送高位字节，再发送低位字节
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
  * @brief   Mifare_One 卡扇区密钥验证
  * @param  
  *		SectorNum:	扇区号(0x0-0x0F)
  *		KeyAB:		密钥类型，0x0A 表示 Type-A；0x0B 表示 Type-B
  *		pKeys:		密钥(6 Bytes)
  * @return 
  *		操作状态码
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

	status = PiccRequest(PICC_REQALL, atq);							// 寻所有 A 卡
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// 防冲突，获取 UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// 转换密钥格式 上位机发送的6个字节为原始密钥，转换后存入 ，共12字节
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// 传送密钥到 RC500 密匙缓冲区
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
				
				for(BlockNum = 0; BlockNum < 4; BlockNum++){
					block = SectorNum * 4 + BlockNum;
					status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// 验证密钥
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
  * @brief  读取 Mifare_One 卡中指定块数据
  * @param  
  *		SectorNum:	扇区号(0x0-0x0F)
  *		BlockNum:	块号(0x0-0x03)
  *		KeyAB:		密钥类型，0x0A 表示 Type-A；0x0B 表示 Type-B
  *		pKeys:		密钥(6 Bytes)
  *		pReadData:	读取的块数据(16 Bytes)
  * @return 
  *		操作状态码
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

	status = PiccRequest(PICC_REQALL, atq);							// 寻所有 A 卡
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// 防冲突，获取 UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// 转换密钥格式 上位机发送的6个字节为原始密钥，转换后存入 ，共12字节
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// 传送密钥到 RC500 密匙缓冲区
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// 验证密钥

				if(MI_OK == status){
					status = PiccRead(block, pReadData);		 
													// 读 Mifare_one卡上一块(block)数据(16字节)
				}
        	}	
		}
	}
	
	return status;
}
			
/**
  * @brief  Mifare_One 卡中指定块写入数据
  * @param  
  *		SectorNum:	扇区号(0-15)
  *		BlockNum:	块号(0-3)
  *		KeyAB:		密钥类型，0x0A 表示 Type-A；0x0B 表示 Type-B
  *		pKeys:		密钥(6 Bytes)
  *		pWriteData:	要写入的块数据(16 Bytes)
  * @return 
  *		操作状态码
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

	status = PiccRequest(PICC_REQALL, atq);							// 寻所有 A 卡
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// 防冲突，获取 UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// 转换密钥格式 上位机发送的6个字节为原始密钥，转换后存入 ，共12字节
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// 传送密钥到 RC500 密匙缓冲区
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// 验证密钥

				if(MI_OK == status){
					status = PiccWrite(block, pWriteData);		 
													// 将数据(16字节)写入 Mifare_one 卡上一块(block)
				}
        	}	
		}
	}
	
	return status;
}
			
/**
  * @brief  初始化钱包命令
  * @param  
  *		SectorNum:	扇区号(0-15)
  *		BlockNum:	块号(0-3)
  *		KeyAB:		密钥类型，0x0A 表示 Type-A；0x0B 表示 Type-B
  *		pKeys:		密钥(6 Bytes)
  *		pInitVal:	初始化金额(4 Bytes)
  * @return 
  *		操作状态码
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

	status = PiccRequest(PICC_REQALL, atq);						// 寻所有A卡
	if(status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);		// 防冲突，获取 UID

	if( MI_OK == status ){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// 转换密钥格式 上位机发送的6个字节为原始密钥，转换后存入 ，共12字节

		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   // 传送密钥到 RC500 密匙缓冲区
			
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
				
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);
																// 验证密钥
				if(MI_OK == status){
					/*	特殊数据块内数据存储格式
					 *	字节号	0 1 2 3 | 4 5 6 7 | 8 9 10 11 |	12   13    14   15
					 * 	说明	  数值	  数值取反     数值	   Adr Adr取反 Adr Adr取反
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
								// 将符合特殊数据块格式的 16 字节数据写入 Mifare_one 卡上一块(block)  
				}
        	}	
		}
	}
	
	return status;
}

/**
  * @brief  响应钱包余额查询命令
  * @param  
  *		SectorNum:	扇区号(0-15)
  *		BlockNum:	块号(0-3)
  *		KeyAB:		密钥类型，0x0A 表示 Type-A；0x0B 表示 Type-B
  *		pKeys:		密钥(6 Bytes)
  *		pInitVal:	初始化金额(4 Bytes)
  * @return 
  *		操作状态码
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

	status = PiccRequest(PICC_REQALL, atq);						// 寻所有A卡
	if(status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);		// 防冲突，获取 UID

	if( MI_OK == status ){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// 转换密钥格式 上位机发送的6个字节为原始密钥，转换后存入 ，共12字节

		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   // 传送密钥到 RC500 密匙缓冲区
			
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
				
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);
																// 验证密钥
				if(MI_OK == status){
					
					status = PiccRead(block, rdata);		 
								// 读取 Mifare_one 卡上一块 16 字节数据，判断是否符合特殊数据块格式

					
					/*	特殊数据块内数据存储格式
					 *	字节号	0 1 2 3 | 4 5 6 7 | 8 9 10 11 |	12   13    14   15
					 * 	说明	  数值	  数值取反     数值	   Adr Adr取反 Adr Adr取反
					 */
					//数据为钱包格式则返回
					if( (rdata[0]== rdata[8]) &&(rdata[1]== rdata[9])
					  &&(rdata[2]== rdata[10]) &&(rdata[3]== rdata[11]) ){
						  rdata[0] = ~rdata[0];
						  rdata[1] = ~rdata[1];
						  rdata[2] = ~rdata[2];
						  rdata[3] = ~rdata[3];
						  
						  if( (rdata[4]==rdata[0]) && (rdata[5]==rdata[1]) 
						  && (rdata[6]==rdata[2]) && (rdata[7]==rdata[3])){	// 回应数值
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
  * @brief  响应钱包充值命令
  * @param  
  *		SectorNum:	扇区号(0-15)
  *		BlockNum:	块号(0-3)
  *		KeyAB:		密钥类型，0x0A 表示 Type-A；0x0B 表示 Type-B
  *		pKeys:		密钥(6 Bytes)
  *		pIncreVal:	充值金额(4 Bytes)
  * @return 
  *		操作状态码
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

	status = PiccRequest(PICC_REQALL, atq);							// 寻所有 A 卡
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// 防冲突，获取 UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// 转换密钥格式 上位机发送的6个字节为原始密钥，转换后存入 ，共12字节
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// 传送密钥到 RC500 密匙缓冲区
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// 验证密钥

				if(MI_OK == status){
					#ifdef __RECHARGE_DEBUG__
					USART_Transmit(pIncreVal, 4);
					#endif
					status = PiccValues(PICC_INCREMENT, block, pIncreVal);		 
																	// 调用值操作函数执行充值
				}
        	}	
		}
	}
	
	return status;
}

/**
  * @brief  响应钱包消费命令
  * @param  
  *		SectorNum:	扇区号(0-15)
  *		BlockNum:	块号(0-3)
  *		KeyAB:		密钥类型，0x0A 表示 Type-A；0x0B 表示 Type-B
  *		pKeys:		密钥(6 Bytes)
  *		pDecreVal:	消费金额(4 Bytes)
  * @return 
  *		操作状态码
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

	status = PiccRequest(PICC_REQALL, atq);							// 寻所有 A 卡
	if (status != MI_OK){    
		status = PiccRequest(PICC_REQALL, atq);  
	}

	status = PiccCascAnticollision(TypeA_Uid, &Uid_len);			// 防冲突，获取 UID

	if(MI_OK == status){
		status = ChangeCodeKey(pKeys, pCodedKeys);	  
						// 转换密钥格式 上位机发送的6个字节为原始密钥，转换后存入 ，共12字节
		
		if(MI_OK == status){
			status =  PcdAuthKey(pCodedKeys);                   	// 传送密钥到 RC500 密匙缓冲区
			if (MI_OK == status){
				auth_mode = PICC_AUTHENT1A;
				if(KeyAB == 0x0B){
					auth_mode = PICC_AUTHENT1B;
				}
             	status = PcdAuthState(auth_mode, block, TypeA_Uid);     
																	// 验证密钥

				if(MI_OK == status){
					status = PiccValues(PICC_DECREMENT, block, pDecreVal);		 
																	// 调用值操作函数执行扣款
				}
        	}	
		}
	}
	
	return status;
}

