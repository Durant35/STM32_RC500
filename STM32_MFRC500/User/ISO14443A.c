#include "ISO14443A.h"
#include "MFRC500.h"
#include "ErrCode.h"
#include "ParallelPort.h"
#include <string.h>

/**
  * @brief  寻卡
  * @param  
  *		@req_code[IN]:	寻卡方式 PICC_REQALL/PICC_REQIDL
  * 		0x52 = 寻感应区内所有符合14443A标准的卡
  *         0x26 = 寻未进入休眠状态的卡
  *		@pTagType[OUT]:	卡片类型代码
  *        	0x0002 = Mifare_One(S70)
  *       	0x0004 = Mifare_One(S50) 	白卡
  8       	0x0008 = Mifare_Pro			校园卡
  *      	0x0010 = Mifare_Light
  *      	0x0044 = Mifare_UltraLight
  *      	0x0304 = Mifare_ProX
  *        	0x0344 = Mifare_DesFire
  *		1. ATQA 的实际作用有两个，
  *			一个是告诉读写器自己是否遵循防冲突机制，
  *			其次，通过 ATQA 可以获知卡片的序列号（即UID）的长度
  *     2. 以 ATQA 判断卡片的类型是不准确的
  *		3. 有 ATQA 是 0044H 和 0344H，但不属于 Mifare UltraLight 和 Mifare Desfire。
  *		   而是一种新的7字节的Mifare S50卡。
  *	@ret 成功返回 MI_OK/ 错误代码 MI_BITCOUNTERR、MI_COLLERR
  * @global none
  * @attention: 
  *		调用 PcdCmdProcess()与卡通讯
  */
signed char PiccRequest(unsigned char req_code, unsigned char *pTagType)
{
	signed char status;
	struct TranSciveBuffer MfComData;
	struct TranSciveBuffer *pi;
	pi = &MfComData;

	//****************************** initialize ******************************
	WriteRawRC(RegChannelRedundancy,0x03);	// 奇校验(RxCRC and TxCRC disable, parity enable)
											// 在 ISO14443A 下进一步配置
	ClearBitMask(RegControl,0x08);			// 数据加密，disable Crypto-1 unit
	WriteRawRC(RegBitFraming,0x07);		    // 最后一个字节发送七位，0x52/0x26最高位均为0
	SetBitMask(RegTxControl,0x03);			// 管脚TX1/TX2上的输出信号将传递调制的13.56MHz能量载波
											// Tx2RF-En, Tx1RF-En enable

	PcdSetTmo(4);		                    // 寻卡超时阈值4.83ms，通过超时标志位判断是否超时
											// ISO14443A 场配置为 PcdSetTmo(1)

	MfComData.MfCommand = PCD_TRANSCEIVE;	// 发送并接受命令
	MfComData.MfLength  = 1;				// 发送数据长度1
	MfComData.MfData[0] = req_code;			// M1卡寻卡命令字 PICC_REQIDL或 PICC_REQALL

	status = PcdCmdProcess(pi);			// 发送并接收，与卡进行通讯
	
	#ifdef __MYDEBUG__
	USART_TransmitOne(0xdd);
	USART_TransmitOne(status&0xFF);
	USART_TransmitOne(MfComData.mfcurrent&0xFF);
	for(i=0; i<MfComData.mfcurrent; i++){
		USART_TransmitOne(MfComData.MfData[i]);
	}
	#endif

	if (MI_OK == status){    
		if (MfComData.MfLength != 0x10){   	// 卡片类型代码，16 bits
			return MI_BITCOUNTERR;   
		}
		*pTagType     = MfComData.MfData[0];
		*(pTagType+1) = MfComData.MfData[1];
	}
	 
	return status;
}


/**
  * @brief  14443A 级联防冲突流程，流程分两部分: ANTICOLLISION 和 SELECT
  *			M1 卡防冲突命令[SEL NVB ]中 SEL 为 0x93，NVB 为 1byte 数据，
  *		高 4 位表示本次待发送数据的有效字节数，低 4 位为发送数据最后一个字节的有效位数。 
  *		成功接收到读写器发送的防冲突指令后，有效范围内的所有卡均以其序列号响应。
  *		一旦发生冲突，则应读取相应寄存器(CollPos， 0x0B )的值确定冲突位， 之后通过不断更新 NVB
  *		的值与接收到的有效位数与接收到的有效数据来更新防冲突指令传送的数据，直至再无冲突发生。
  * @param  
  *		unsigned char *pUid:	级联防冲突结束时获取的 A 卡的 UID
  *		unsigned char *pLen:	A 卡的 UID 的长度
  *	@ret 成功返回 MI_OK/ 错误代码 MI_BITCOUNTERR、MI_COLLERR
  * @global none
  * @attention: 
  *		调用 PiccAnticollisionLoop()和 PiccSelect()函数
  */
signed char PiccCascAnticollision(unsigned char *pUid, unsigned char *pLen)
{   
	unsigned char SAK;
	signed char status;
	unsigned char i;

	status = PiccAnticollisionLoop(pUid, PICC_ANTICOLL1);					// 发送 PICC_ANTICOLL1 获取 UID CL1
	
	#ifdef __MYDEBUG__
	USART_TransmitOne((unsigned char)0xab);
	USART_TransmitOne(status);
	#endif
	
	if(MI_OK == status){	
		status = PiccSelect(pUid, &SAK, PICC_ANTICOLL1);		// 首次发送 SELECT 命令获取 UID CL1
		
		if(MI_OK == status){
			*pLen = 4;											// Single UID Size(4 bytes)
			
			if(SAK & 0x20){										// 支持14443-4协议
				//Flag14443_4  =1;						
			}
			else{
				//Flag14443_4  =0;								// 不支持14443-4协议
			}
			// 根据标准，SAK 的 bit3 位表示UID 是否完整。此位为 1 表示不完整
			if( SAK&0x04 ){ 			
				for(i=0; i<3; i++){								// 不完整，丢弃第 1 个字节
					pUid[i] = pUid[i+1];  
				}
				
				status = PiccAnticollisionLoop(pUid+3, PICC_ANTICOLL2);	// 发送 PICC_ANTICOLL2 获取 UID CL2
				if(MI_OK ==status){
					status = PiccSelect(pUid+3, &SAK, PICC_ANTICOLL2);
					
				 	if(MI_OK == status){
					 	*pLen = 7;								// Double UID Size(7 bytes)
						if(SAK & 0x20){							// 支持14443-4协议
							//Flag14443_4  =1;						
						}
						else{
							//Flag14443_4  =0;					// 不支持14443-4协议
						} 		
						if( SAK&0x04 ){
						 	for (i=0; i<3; i++){				// 不完整，丢弃第 1 个字节
							 	pUid[i+3] = pUid[i+4];
							}
							status = PiccAnticollisionLoop(pUid+6, PICC_ANTICOLL3);
							if(MI_OK == status){
						    	status = PiccSelect(pUid+6, &SAK, PICC_ANTICOLL3);
								if(MI_OK == status){
									*pLen = 10;					// Tripple UID Size(10 bytes)
									
									if(SAK & 0x20){
										//Flag14443_4  =1;						
									}
									else{
										//Flag14443_4  =0;												//不支持14443-4协议
									}
								}
								else{
									return status;
								}
							}else{
								return status;
							}// end of PICC_ANTICOLL3
						}
					}else{
						return status;
					}// end of PICC_ANTICOLL2
				}else{
					return status;
				}// end of PICC_ANTICOLL1
			}					
		}
		return status;
	}
	return status;
}

/**
  * @brief  14443A 防冲突循环 ANTICOLLISION 命令
  * @param  
  *		unsigned char *pSnr:	每个串联级别的 UID CLn
  *		unsigned char SelType:	ANTICOLLISION 命令的 SEL 部分，
  *								0x93 为级别一，0x95 为级别二,0x97 为级别三
  *	@ret 成功返回 MI_OK/ 错误代码 MI_BITCOUNTERR、MI_COLLERR
  * @global none
  * @attention: 
  *		调用 PcdCmdProcess()与卡通讯
  */
signed char PiccAnticollisionLoop(unsigned char *pSnr, unsigned char SelType)
{
    signed char status;
    unsigned char i;
    unsigned char ucBytes;							// number of bytes known
    unsigned char ucBits;							// remaining number of bits
    unsigned char snr_check = 0;					// snr_xor vs snr_check
    unsigned char ucCollPosition = 0;				// record CollPos
    unsigned char ucTemp;
    unsigned char ucSnr[5] = {0, 0, 0, 0 ,0};		// local snr and BCC
	
    unsigned char dummyShift1;      				// dummy byte for snr shifting
    unsigned char dummyShift2;      				// dummy byte for snr shifting
	
	/*
	 * struct TranSciveBuffer{
		unsigned char MfCommand;		// MFRC500命令字或 M1卡命令字
		unsigned int  MfLength;			// 发送数据长度(/byte)或接收数据长度(/bit)
		unsigned char MfData[128];		// 发送数据或接收数据临时缓冲区
		unsigned int mfcurrent;			// 接收字节数
	   };
	 */
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
	memset(MfComData.MfData, 0, 128);	// initialize buffer to zeros.
    pi = &MfComData;
    
	
	// ****************************** Registers Initialisation ******************************
    WriteRawRC(RegDecoderControl, 0x28);			// 启用 ZeroAfterColl
													// 在一个位冲突之后的任何位都屏蔽为 0，
													// 由 ISO14443A 定义的防冲突机制进行处理
    ClearBitMask(RegControl,0x08);					// disable crypto 1 unit
    WriteRawRC(RegChannelRedundancy,0x03);			// RxCRC and TxCRC disable, parity enable
    PcdSetTmo(3);									// medium timeout (4.833 ms)


	// ****************************** Anticollision Loop ******************************
    do
    {
		// -------------------- 更新 ucBits、ucBytes，设置 RegBitFraming --------------------
        ucBits = (ucCollPosition) % 8;				// remaining number of bits
		// 不完整字节需额外处理
        if (ucBits != 0){
            ucBytes = ucCollPosition / 8 + 1;		// 不足一个字节也需要按一个字节计数
			
			// RxAlign[6:4] 用于位方式帧的接收，定义了接收的第一个位存储在 FIFO 的位置，
			// 		更多的位存储在后面的位位置。
			// TxLastBits[2:0] 用于位方式帧的发送，定义要发送的最后一个字节的位数目。
            WriteRawRC(RegBitFraming, (ucBits << 4) + ucBits);		// TxLastBits/RxAlign
			
			// in order to solve an inconsistancy in the anticollision sequence
			// (will be solved soon), the case of 7 bits has to be treated in a
			// separate way - 官方文档：强烈建议不要使用 RxAlign=7 以防止数据丢失！！			
            if (ucBits == 7){
                WriteRawRC(RegBitFraming, ucBits); 	// reset RxAlign to zero，TxLastBits=ucBits
            }
        }
        else{
             ucBytes = ucCollPosition / 8;			// 没有不足一个字节的情况
        }
	
		
		// -------------------------- 构建发送命令信息 --------------------------
        MfComData.MfCommand = PCD_TRANSCEIVE;
        MfComData.MfData[0] = SelType;				// PICC_ANTICOLL1
		/*
		 * NVB 初始值为 0x20，表示该命令只含有 2 个字节，即"0x93+0x20"，不含 UID 数据，
		 * 		MIFARE 卡须返回全部 UID 字节作为响应。
		 * 若返回的 UID 数据有位冲突的情况发生，则根据冲突位置更新 NVB 值。
		 * 在搜索循环中，随着 UID 已知比特数的加入，NVB 不断增加，直到 0x70 为止。
		 *		它表示除了"0x93+0x70"两个命令字节外，还有 UID0～UID3 和 BCC 5个UID数据字节。
		 * 此时命令字节共有 7 个，防冲突命令转变为卡片选择命令。
		 * BCC 只有在 UID CLn 为 40bit 才有，是前面 5 个字节的异或！！！
		 */
        MfComData.MfData[1] = 0x20 + ((ucCollPosition / 8) << 4) + (ucBits & 0x0F);
        for (i=0; i<ucBytes; i++){					// 发送缓冲区描述
	        MfComData.MfData[i + 2] = ucSnr[i];
	    }
	    MfComData.MfLength = ucBytes + 2;			// 发送数据字节数
	
	    status = PcdCmdProcess(pi);
		
		#ifdef __MYDEBUG__
		USART_TransmitOne((unsigned char)0xcc);
		USART_TransmitOne(status);					// 此处应为 MI_OK/MI_COLLERR
		USART_TransmitOne(MfComData.MfLength);
		USART_TransmitOne(MfComData.mfcurrent);
		for(i=0; i<MfComData.mfcurrent; i++){
			USART_TransmitOne(MfComData.MfData[i]);
		}
		#endif
		
		
		// -------------------------- 接收数据预处理 --------------------------
		/* RxAlign=7 数据可能会丢失，在位位置 7、15、23、31、39(CollPos) 
		 * 		检测到的位冲突不能通过 RxAlign 解决，需要软件来实现
		 */
		if(ucBits == 7){
			// xxxx xxx?  ---- ---[x]  ==> 3 bytes！
			if( (MfComData.MfLength%8)==0 ){
				MfComData.mfcurrent += 1;
			}
			
            dummyShift1 = 0x00;								// reorder received bits
			/*
			 *	软件实现 RxAlign=7 时接收数据的移位恢复
			 *		xxxx xxx?  ---- ---[x] **** ***[-]  0000 000[*]
			 *		?000 0000  [x]xxx xxxx [-]--- ----  [*]*** ****
			 */
			if( MfComData.MfData[0] ){
				for (i=1; i<MfComData.mfcurrent; i++){		// MfData[0] keeps the CollPos	
					dummyShift2 = MfComData.MfData[i];
					/*
					 *	xxxx xxx?(>>1) 			==> 0xxx xxxx
					 *	---- ---[x](<<7) 		==> [x]00 0000
					 *	0xxx xxxx | [x]00 0000 	==> [x]xxx xxxx
					 */
					MfComData.MfData[i] = (dummyShift1 >> 1) | (MfComData.MfData[i] << 7);
					dummyShift1 = dummyShift2;
				}
            
				//MfComData.MfLength -= MfComData.mfcurrent;	// no need to update bits&bytes
				
				// recalculation of collision position
				//MfComData.MfData[0] += 7 - (MfComData.MfData[0] + 6) / 9;
			} // end of if( MfComData.MfData[0] )
			else{
				for (i=0; i<MfComData.mfcurrent; i++){	
					dummyShift2 = MfComData.MfData[i];
					MfComData.MfData[i] = (dummyShift1 >> 1) | (MfComData.MfData[i] << 7);
					dummyShift1 = dummyShift2;
				}
			}
        } // end of if(ucBits == 7)
	
		
		// -------------------------- 更新当前 UID 信息 --------------------------
	    ucTemp = ucSnr[(ucCollPosition / 8)];
		// MI_COLLERR/MI_OK, no other occured
	    if(status == MI_COLLERR){
			for (i=0; i < MfComData.mfcurrent; i++){// MfData[0] keeps the CollPos	
		         ucSnr[i + (ucCollPosition / 8)] = MfComData.MfData[i+1];
            }
	        ucSnr[(ucCollPosition / 8)] |= ucTemp;
			
	        ucCollPosition += MfComData.MfData[0];				// Update the ucCollPosition
			#ifdef __ANTICOLL_LOOP_DEBUG__
			USART_TransmitOne(0xbc);
			USART_TransmitOne(ucCollPosition);
			#endif
        }
        else if(status == MI_OK){
            for (i=0; i < MfComData.mfcurrent; i++){
                 ucSnr[4 - i] = MfComData.MfData[MfComData.mfcurrent - i - 1];
            }
            ucSnr[(ucCollPosition / 8)] |= ucTemp;
        }
    } while(status == MI_COLLERR);

	
	// ****************************** SNR Xor check ******************************			
    if(status == MI_OK){
    	 for(i=0; i<4; i++){   
             *(pSnr+i)  = ucSnr[i];
             snr_check ^= ucSnr[i];
         }
		 
         if(snr_check != ucSnr[i]){   
			status = MI_COM_ERR;    
		 } 
    }
 
	
	// ****************************** 初始化复位设置 ******************************  	
    ClearBitMask(RegDecoderControl,0x20);					// ZeroAfterColl disable
	
    return status;
}

/**
  * @brief  14443A 级联防冲突流程 SELECT 命令
  * @param  
  *		unsigned char *pSnr:	每个串联级别的 UID CLn
  *		unsigned char *pSAK:	卡片 SAK
  *		unsigned char SelType:	ANTICOLLISION 命令的 SEL 部分，
  *								0x93 为级别一，0x95 为级别二,0x97 为级别三
  *	@ret 执行状态
  * @global none
  * @attention: 
  *		调用 PcdCmdProcess()与卡通讯
  */
signed char PiccSelect(unsigned char *pSnr, unsigned char *pSAK, unsigned char SelType)
{
    unsigned char i;
    signed char status;
    unsigned char snr_check = 0;
	
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;

    WriteRawRC(RegChannelRedundancy,0x0F);		// RxCRC,TxCRC, Parity enable
    ClearBitMask(RegControl,0x08);				// disable crypto 1 unit
    PcdSetTmo(4);
    
	// ********************************** Cmd Sequence **********************************
	/*
	 * 	① 循环防冲突结束，PCD 指定 NVB 为 0x70，此值表示 PCD 将发送完整的 UID
	 *	② PCD 发送 SEL 和 NVB，接着发送 40bit 的 UID，后面是其 Xor CheckSum
	 *	③ 与 40bit 的 UID 匹配的 PICC，以 SAK 作为应答
	 * 	④ 如果 UID 是完整的，PICC 将发送带有层叠位为 "0" 的 SAK，同时从 READY 转为 ACTIVE 状态
	 *	⑤ 如果 层叠位为 "1"，进入下一级防冲突循环 
	 */
    MfComData.MfCommand = PCD_TRANSCEIVE;
    MfComData.MfData[0] = SelType;
    MfComData.MfData[1] = 0x70;
    for(i=0; i<4; i++){
    	snr_check ^= *(pSnr+i);
    	MfComData.MfData[i+2] = *(pSnr+i);
    }
    MfComData.MfData[6] = snr_check;
    MfComData.MfLength  = 7;

    status = PcdCmdProcess(pi);
    
    if (status == MI_OK){
        if (MfComData.MfLength != 0x8){			// 一个字节的 SAK
			status = MI_BITCOUNTERR;
		}
    }
	
	*pSAK = MfComData.MfData[0];
	
    return status;
}

/**
  * @brief  开启 PCD/PICC 3 Pass Authentication
  * @param   
  *		auth_mode:	验证方式		0x60:验证 A 密钥
  *								0x61:验证 B 密钥
  *		blockNum:	要验证的绝对块号(0-63)
  *		pSnr:		序列号首地址
  * @return 
  *		执行状态
  * @attention
  *		调用 PcdCmdProcess()与卡通讯
  * @global 
  *		none
  */
signed char PcdAuthState(unsigned char auth_mode, unsigned char blockNum, unsigned char *pSnr)
{
    signed char status = MI_OK;
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;

    WriteRawRC(RegChannelRedundancy,0x0F);
				// RxCRCEn，接收帧的最后一个字节被解释为CRC字节，CRC 正确，CRC 字节不放入 FIFO；
				// 											   CRC 错误，CRCErr 标志置位
				// TxCRCEn，对发送的数据进行 CRC 计算并将 CRC 字节加到发送数据流中
				// ParityOdd，采用奇数的奇偶校验，遵从 ISO14443A
				// ParityEn，奇偶校验位在每个字节后插入发送数据流中并会出现在接收数据流的每个字节后
    PcdSetTmo(3);										// medium timeout (4.833 ms)
	
	/*
	 *	AUTHENT1：执行 Crypto1 卡验证的第一部分
	 *	参数：卡的 Auth 命令、要验证的卡的绝对块号
	 *		  卡的序列号最低字节、...、卡的序列号最高字节
	 */
    MfComData.MfCommand = PCD_AUTHENT1;
    MfComData.MfLength  = 6;
    MfComData.MfData[0] = auth_mode;					// write authentication command
    MfComData.MfData[1] = blockNum;						// write block number for authentication
    memcpy(&MfComData.MfData[2], pSnr, 4);				// write 4 bytes card serial number 
      
    status = PcdCmdProcess(pi);
	
	#ifdef __AUTH_DEBUG__
		USART_TransmitOne(0xaa);
		USART_TransmitOne(MfComData.MfLength);
		USART_TransmitOne(MfComData.MfData[0]);
	#endif

	if (status == MI_OK){
        if(ReadRawRC(RegSecondaryStatus) & 0x07){ 		// Check RxLastBits for error
														// 最后接收字节必须整个字节有效
			status = MI_BITCOUNTERR;
			
			#ifdef __AUTH_DEBUG__
				USART_TransmitOne(0xbb);
				USART_TransmitOne(MfComData.MfLength);
				USART_TransmitOne(MfComData.MfData[0]);
			#endif
		}
        else{
			MfComData.MfCommand = PCD_AUTHENT2;
			MfComData.MfLength  = 0;
			status = PcdCmdProcess(pi);
			
			#ifdef __AUTH_DEBUG__
				USART_TransmitOne(0xbb);
				USART_TransmitOne(MfComData.MfLength);
				USART_TransmitOne(MfComData.MfData[0]);
			#endif
			
			if (status == MI_OK){
				if (ReadRawRC(RegControl) & 0x08){		// Crypto1 activated
					status = MI_OK;   
				}
				else{
					status = MI_AUTHERR;   
				}
			}
		}
	}
    return status;
}

/**
  * @brief  读 Mifare_one 卡上一块(block)数据(16 字节)
  * @param  
  *		addr:		要读的绝对块号(0-63)
  *		pReaddata:	读出的数据 16 字节
  * @return 
  *		操作状态码
  * @attention
  *		none
  * @global 
  *		none
  */
signed char PiccRead(unsigned char addr, unsigned char *pReaddata)
{
    signed char status;
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;
	
    PcdSetTmo(3);										// medium timeout (4.833 ms)
														// follow Typical Transaction Time(≥2.5ms)
	
    WriteRawRC(RegChannelRedundancy,0x0F);				// RxCRC, TxCRC, Parity enable
				// RxCRCEn，接收帧的最后一个字节被解释为CRC字节，CRC 正确，CRC 字节不放入 FIFO；
				// 											   CRC 错误，CRCErr 标志置位
				// TxCRCEn，对发送的数据进行 CRC 计算并将 CRC 字节加到发送数据流中
				// ParityOdd，采用奇数的奇偶校验，遵从 ISO14443A
				// ParityEn，奇偶校验位在每个字节后插入发送数据流中并会出现在接收数据流的每个字节后
	
    MfComData.MfCommand = PCD_TRANSCEIVE;
    MfComData.MfLength  = 2;
    MfComData.MfData[0] = PICC_READ;
    MfComData.MfData[1] = addr;

    status = PcdCmdProcess(pi);
    if(status == MI_OK){
		if (MfComData.MfLength != 0x80){
			status = MI_BITCOUNTERR;
		}
        else{   
			memcpy(pReaddata, &MfComData.MfData[0], 16);  
		}
    }
	else{
		if (status != MI_NOTAGERR ){			// no timeout occured
			if (MfComData.MfLength == 4){ 		// NACK
				MfComData.MfData[0] &= 0x0f;  	// mask out upper nibble
				if((MfComData.MfData[0] & 0x0a) == 0){
					status = MI_NOTAUTHERR;
				}
				else{
					status = MI_CODEERR;
				}
			}
		}
		memcpy(pReaddata, "0000000000000000", 16);
	}
	
    return status;
}

/**
  * @brief  读 Mifare_one 卡上一块(block)数据(16 字节)
  * @param  
  *		addr:		要写的绝对块号(0-63)
  *		pWritedata:	要写入的数据 16 字节
  * @return 
  *		操作状态码
  * @attention
  *		none
  * @global 
  *		none
  */
signed char PiccWrite(unsigned char addr, unsigned char *pWritedata)
{
    signed char status;
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;
    
    PcdSetTmo(1);               						// short timeout (1,0 ms) as configured.
    WriteRawRC(RegChannelRedundancy,0x07); 				// TxCRC, Parity enable
				// RxCRCEn，接收帧的最后一个字节被解释为CRC字节，CRC 正确，CRC 字节不放入 FIFO；
				// 											   CRC 错误，CRCErr 标志置位
				// TxCRCEn，对发送的数据进行 CRC 计算并将 CRC 字节加到发送数据流中
				// ParityOdd，采用奇数的奇偶校验，遵从 ISO14443A
				// ParityEn，奇偶校验位在每个字节后插入发送数据流中并会出现在接收数据流的每个字节后

    MfComData.MfCommand = PCD_TRANSCEIVE;
    MfComData.MfLength  = 2;
    MfComData.MfData[0] = PICC_WRITE;					// Write command code
    MfComData.MfData[1] = addr;

    status = PcdCmdProcess(pi);

	/*
	 *	1. 先发送 PICC_WRITE 命令，发送完该命令后等待卡的响应信号(1010)
	 *	2. 接着发送 16 字节的数据，再次等待响应信号(1010)
	 */
    if(status != MI_NOTAGERR){							// no timeout error
        if(MfComData.MfLength != 4){  					// 4 bits are necessary
			status = MI_BITCOUNTERR;   	
		}
        else{											// 4 bit received
           MfComData.MfData[0] &= 0x0F;					// mask out upper nibble
           switch (MfComData.MfData[0]){
              case 0x00:
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0A:
                 status = MI_OK;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
           }
        }
    }
	
    if(status == MI_OK){
        PcdSetTmo(4);									// medium timeout (6.0 ms)
														// follow Typical Transaction Time(≥6.0ms)
        MfComData.MfCommand = PCD_TRANSCEIVE;
        MfComData.MfLength  = 16;
        memcpy(&MfComData.MfData[0], pWritedata, 16);
		     
        status = PcdCmdProcess(pi);

        if (status != MI_NOTAGERR){
            MfComData.MfData[0] &= 0x0F;
            switch(MfComData.MfData[0])
            {
               case 0x00:
                  status = MI_WRITEERR;
                  break;
               case 0x0A:
                  status = MI_OK;
                  break;
               default:
                  status = MI_CODEERR;
                  break;
           }
        }
    }
	
    return status;
}

/**
  * @brief  对 Mifare_one 卡上特殊数据块(值块)进行值(扣款/充值)操作
  * @param  
  *		dd_mode:	输入选择操作
  *			PICC_DECREMENT(0xC0):	扣款	
  *			PICC_INCREMENT(0xC1):	充值
  *		addr:		要操作的绝对块号(0-63)
  *		pValue:		4 字节增(减)值首地址，带符号16进制数，低位在前
  * @return 
  *		操作状态码
  * @attention
  *		none
  * @global 
  *		none
  */
signed char PiccValues(unsigned char dd_mode, unsigned char addr, unsigned char *pValue)
{
	signed char status;
	
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;
	
    PcdSetTmo(1);									// short timeout (1,0 ms)

	#ifdef FM17XX
		WriteRawRC(RegChannelRedundancy,0x07);		// 针对 FM17XX 芯片的不同点
	#else
    	WriteRawRC(RegChannelRedundancy,0x07);		// RxCRC, TxCRC, Parity enable
				// RxCRCEn，接收帧的最后一个字节被解释为CRC字节，CRC 正确，CRC 字节不放入 FIFO；
				// 											   CRC 错误，CRCErr 标志置位
				// TxCRCEn，对发送的数据进行 CRC 计算并将 CRC 字节加到发送数据流中
				// ParityOdd，采用奇数的奇偶校验，遵从 ISO14443A
				// ParityEn，奇偶校验位在每个字节后插入发送数据流中并会出现在接收数据流的每个字节后
	#endif

    MfComData.MfCommand = PCD_TRANSCEIVE;
    MfComData.MfLength  = 2;
    MfComData.MfData[0] = dd_mode;					// Inc/Dec command code
    MfComData.MfData[1] = addr;
	
	status = PcdCmdProcess(pi);

    if(status != MI_NOTAGERR){						// no timeout error
        if (MfComData.MfLength != 4){   			// 4 bits are necessary
			status = MI_BITCOUNTERR;   
		}
        else{
			#ifdef __WALLETDEBUG__
			USART_TransmitOne(0xdd);
			USART_TransmitOne(status);
			USART_TransmitOne(MfComData.MfLength);
			USART_TransmitOne(MfComData.MfData[0]);
			#endif
			
			MfComData.MfData[0] &= 0x0F;			// mask out upper nibble
			switch (MfComData.MfData[0]){			// 响应信号(1010)
              case 0x00:
                 status = MI_NOTAUTHERR;
                 break;
              case 0x0A:
                 status = MI_OK;
                 break;
              case 0x01:
                 status = MI_VALERR;
                 break;
              default:
                 status = MI_CODEERR;
                 break;
			}
		}
	}
	if(status == MI_OK){
		PcdSetTmo(3);								// medium timeout (4.833 ms)
													// follow Typical Transaction Time(both 2.5ms)
        MfComData.MfCommand = PCD_TRANSCEIVE;
        MfComData.MfLength  = 4;
        pi = &MfComData;
        memcpy(&MfComData.MfData[0], pValue, 4);
		
		status = PcdCmdProcess(pi);

		if(status == MI_OK){
			if (MfComData.MfLength != 4){   
				status = MI_BITCOUNTERR;
			}
			else{   
				status = MI_OK;           
			}
        }
        else if(status == MI_NOTAGERR){   			// no response after 4 byte value
													// transfer command has to follow
			status = MI_OK;    
		}
        else{   
			status = MI_COM_ERR;     
		}
     }
     
     if (status == MI_OK){
		 MfComData.MfCommand = PCD_TRANSCEIVE;
		 MfComData.MfLength  = 2;
		 MfComData.MfData[0] = PICC_TRANSFER;		// transfer command code
		 MfComData.MfData[1] = addr;
		 status = PcdCmdProcess(pi);
		 
		 if (status != MI_NOTAGERR){
            if(MfComData.MfLength != 4){   
				status = MI_BITCOUNTERR;    
			}
            else{
				MfComData.MfData[0] &= 0x0F;
				switch(MfComData.MfData[0]){
					case 0x00:
						status = MI_NOTAUTHERR;
					break;
					case 0x0a:
						status = MI_OK;
					break;
					case 0x01:
						status = MI_VALERR;
					break;
					default:
						status = MI_CODEERR;
					break;
				}
			}
		 }
	 }
	 
	 return status;
}

// To Be Tested
signed char PiccHalt(){
	signed char status = MI_OK;
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;

    MfComData.MfCommand = PCD_TRANSCEIVE;
    MfComData.MfLength  = 2;
    MfComData.MfData[0] = PICC_HALT;
    MfComData.MfData[1] = 0;

    status = PcdCmdProcess(pi);
    if (status){
        if (status==MI_NOTAGERR || status== MI_ACCESSTIMEOUT){
			status = MI_OK;
		}
    }
	
    WriteRawRC(RegCommand, PCD_IDLE);
	
    return status;
}

