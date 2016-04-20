#include "ISO14443A.h"
#include "MFRC500.h"
#include "ErrCode.h"
#include "ParallelPort.h"
#include <string.h>

/**
  * @brief  Ѱ��
  * @param  
  *		@req_code[IN]:	Ѱ����ʽ PICC_REQALL/PICC_REQIDL
  * 		0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
  *         0x26 = Ѱδ��������״̬�Ŀ�
  *		@pTagType[OUT]:	��Ƭ���ʹ���
  *        	0x0002 = Mifare_One(S70)
  *       	0x0004 = Mifare_One(S50) 	�׿�
  8       	0x0008 = Mifare_Pro			У԰��
  *      	0x0010 = Mifare_Light
  *      	0x0044 = Mifare_UltraLight
  *      	0x0304 = Mifare_ProX
  *        	0x0344 = Mifare_DesFire
  *		1. ATQA ��ʵ��������������
  *			һ���Ǹ��߶�д���Լ��Ƿ���ѭ����ͻ���ƣ�
  *			��Σ�ͨ�� ATQA ���Ի�֪��Ƭ�����кţ���UID���ĳ���
  *     2. �� ATQA �жϿ�Ƭ�������ǲ�׼ȷ��
  *		3. �� ATQA �� 0044H �� 0344H���������� Mifare UltraLight �� Mifare Desfire��
  *		   ����һ���µ�7�ֽڵ�Mifare S50����
  *	@ret �ɹ����� MI_OK/ ������� MI_BITCOUNTERR��MI_COLLERR
  * @global none
  * @attention: 
  *		���� PcdCmdProcess()�뿨ͨѶ
  */
signed char PiccRequest(unsigned char req_code, unsigned char *pTagType)
{
	signed char status;
	struct TranSciveBuffer MfComData;
	struct TranSciveBuffer *pi;
	pi = &MfComData;

	//****************************** initialize ******************************
	WriteRawRC(RegChannelRedundancy,0x03);	// ��У��(RxCRC and TxCRC disable, parity enable)
											// �� ISO14443A �½�һ������
	ClearBitMask(RegControl,0x08);			// ���ݼ��ܣ�disable Crypto-1 unit
	WriteRawRC(RegBitFraming,0x07);		    // ���һ���ֽڷ�����λ��0x52/0x26���λ��Ϊ0
	SetBitMask(RegTxControl,0x03);			// �ܽ�TX1/TX2�ϵ�����źŽ����ݵ��Ƶ�13.56MHz�����ز�
											// Tx2RF-En, Tx1RF-En enable

	PcdSetTmo(4);		                    // Ѱ����ʱ��ֵ4.83ms��ͨ����ʱ��־λ�ж��Ƿ�ʱ
											// ISO14443A ������Ϊ PcdSetTmo(1)

	MfComData.MfCommand = PCD_TRANSCEIVE;	// ���Ͳ���������
	MfComData.MfLength  = 1;				// �������ݳ���1
	MfComData.MfData[0] = req_code;			// M1��Ѱ�������� PICC_REQIDL�� PICC_REQALL

	status = PcdCmdProcess(pi);			// ���Ͳ����գ��뿨����ͨѶ
	
	#ifdef __MYDEBUG__
	USART_TransmitOne(0xdd);
	USART_TransmitOne(status&0xFF);
	USART_TransmitOne(MfComData.mfcurrent&0xFF);
	for(i=0; i<MfComData.mfcurrent; i++){
		USART_TransmitOne(MfComData.MfData[i]);
	}
	#endif

	if (MI_OK == status){    
		if (MfComData.MfLength != 0x10){   	// ��Ƭ���ʹ��룬16 bits
			return MI_BITCOUNTERR;   
		}
		*pTagType     = MfComData.MfData[0];
		*(pTagType+1) = MfComData.MfData[1];
	}
	 
	return status;
}


/**
  * @brief  14443A ��������ͻ���̣����̷�������: ANTICOLLISION �� SELECT
  *			M1 ������ͻ����[SEL NVB ]�� SEL Ϊ 0x93��NVB Ϊ 1byte ���ݣ�
  *		�� 4 λ��ʾ���δ��������ݵ���Ч�ֽ������� 4 λΪ�����������һ���ֽڵ���Чλ���� 
  *		�ɹ����յ���д�����͵ķ���ͻָ�����Ч��Χ�ڵ����п����������к���Ӧ��
  *		һ��������ͻ����Ӧ��ȡ��Ӧ�Ĵ���(CollPos�� 0x0B )��ֵȷ����ͻλ�� ֮��ͨ�����ϸ��� NVB
  *		��ֵ����յ�����Чλ������յ�����Ч���������·���ͻָ��͵����ݣ�ֱ�����޳�ͻ������
  * @param  
  *		unsigned char *pUid:	��������ͻ����ʱ��ȡ�� A ���� UID
  *		unsigned char *pLen:	A ���� UID �ĳ���
  *	@ret �ɹ����� MI_OK/ ������� MI_BITCOUNTERR��MI_COLLERR
  * @global none
  * @attention: 
  *		���� PiccAnticollisionLoop()�� PiccSelect()����
  */
signed char PiccCascAnticollision(unsigned char *pUid, unsigned char *pLen)
{   
	unsigned char SAK;
	signed char status;
	unsigned char i;

	status = PiccAnticollisionLoop(pUid, PICC_ANTICOLL1);					// ���� PICC_ANTICOLL1 ��ȡ UID CL1
	
	#ifdef __MYDEBUG__
	USART_TransmitOne((unsigned char)0xab);
	USART_TransmitOne(status);
	#endif
	
	if(MI_OK == status){	
		status = PiccSelect(pUid, &SAK, PICC_ANTICOLL1);		// �״η��� SELECT �����ȡ UID CL1
		
		if(MI_OK == status){
			*pLen = 4;											// Single UID Size(4 bytes)
			
			if(SAK & 0x20){										// ֧��14443-4Э��
				//Flag14443_4  =1;						
			}
			else{
				//Flag14443_4  =0;								// ��֧��14443-4Э��
			}
			// ���ݱ�׼��SAK �� bit3 λ��ʾUID �Ƿ���������λΪ 1 ��ʾ������
			if( SAK&0x04 ){ 			
				for(i=0; i<3; i++){								// �������������� 1 ���ֽ�
					pUid[i] = pUid[i+1];  
				}
				
				status = PiccAnticollisionLoop(pUid+3, PICC_ANTICOLL2);	// ���� PICC_ANTICOLL2 ��ȡ UID CL2
				if(MI_OK ==status){
					status = PiccSelect(pUid+3, &SAK, PICC_ANTICOLL2);
					
				 	if(MI_OK == status){
					 	*pLen = 7;								// Double UID Size(7 bytes)
						if(SAK & 0x20){							// ֧��14443-4Э��
							//Flag14443_4  =1;						
						}
						else{
							//Flag14443_4  =0;					// ��֧��14443-4Э��
						} 		
						if( SAK&0x04 ){
						 	for (i=0; i<3; i++){				// �������������� 1 ���ֽ�
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
										//Flag14443_4  =0;												//��֧��14443-4Э��
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
  * @brief  14443A ����ͻѭ�� ANTICOLLISION ����
  * @param  
  *		unsigned char *pSnr:	ÿ����������� UID CLn
  *		unsigned char SelType:	ANTICOLLISION ����� SEL ���֣�
  *								0x93 Ϊ����һ��0x95 Ϊ�����,0x97 Ϊ������
  *	@ret �ɹ����� MI_OK/ ������� MI_BITCOUNTERR��MI_COLLERR
  * @global none
  * @attention: 
  *		���� PcdCmdProcess()�뿨ͨѶ
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
		unsigned char MfCommand;		// MFRC500�����ֻ� M1��������
		unsigned int  MfLength;			// �������ݳ���(/byte)��������ݳ���(/bit)
		unsigned char MfData[128];		// �������ݻ����������ʱ������
		unsigned int mfcurrent;			// �����ֽ���
	   };
	 */
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
	memset(MfComData.MfData, 0, 128);	// initialize buffer to zeros.
    pi = &MfComData;
    
	
	// ****************************** Registers Initialisation ******************************
    WriteRawRC(RegDecoderControl, 0x28);			// ���� ZeroAfterColl
													// ��һ��λ��ͻ֮����κ�λ������Ϊ 0��
													// �� ISO14443A ����ķ���ͻ���ƽ��д���
    ClearBitMask(RegControl,0x08);					// disable crypto 1 unit
    WriteRawRC(RegChannelRedundancy,0x03);			// RxCRC and TxCRC disable, parity enable
    PcdSetTmo(3);									// medium timeout (4.833 ms)


	// ****************************** Anticollision Loop ******************************
    do
    {
		// -------------------- ���� ucBits��ucBytes������ RegBitFraming --------------------
        ucBits = (ucCollPosition) % 8;				// remaining number of bits
		// �������ֽ�����⴦��
        if (ucBits != 0){
            ucBytes = ucCollPosition / 8 + 1;		// ����һ���ֽ�Ҳ��Ҫ��һ���ֽڼ���
			
			// RxAlign[6:4] ����λ��ʽ֡�Ľ��գ������˽��յĵ�һ��λ�洢�� FIFO ��λ�ã�
			// 		�����λ�洢�ں����λλ�á�
			// TxLastBits[2:0] ����λ��ʽ֡�ķ��ͣ�����Ҫ���͵����һ���ֽڵ�λ��Ŀ��
            WriteRawRC(RegBitFraming, (ucBits << 4) + ucBits);		// TxLastBits/RxAlign
			
			// in order to solve an inconsistancy in the anticollision sequence
			// (will be solved soon), the case of 7 bits has to be treated in a
			// separate way - �ٷ��ĵ���ǿ�ҽ��鲻Ҫʹ�� RxAlign=7 �Է�ֹ���ݶ�ʧ����			
            if (ucBits == 7){
                WriteRawRC(RegBitFraming, ucBits); 	// reset RxAlign to zero��TxLastBits=ucBits
            }
        }
        else{
             ucBytes = ucCollPosition / 8;			// û�в���һ���ֽڵ����
        }
	
		
		// -------------------------- ��������������Ϣ --------------------------
        MfComData.MfCommand = PCD_TRANSCEIVE;
        MfComData.MfData[0] = SelType;				// PICC_ANTICOLL1
		/*
		 * NVB ��ʼֵΪ 0x20����ʾ������ֻ���� 2 ���ֽڣ���"0x93+0x20"������ UID ���ݣ�
		 * 		MIFARE ���뷵��ȫ�� UID �ֽ���Ϊ��Ӧ��
		 * �����ص� UID ������λ��ͻ���������������ݳ�ͻλ�ø��� NVB ֵ��
		 * ������ѭ���У����� UID ��֪�������ļ��룬NVB �������ӣ�ֱ�� 0x70 Ϊֹ��
		 *		����ʾ����"0x93+0x70"���������ֽ��⣬���� UID0��UID3 �� BCC 5��UID�����ֽڡ�
		 * ��ʱ�����ֽڹ��� 7 ��������ͻ����ת��Ϊ��Ƭѡ�����
		 * BCC ֻ���� UID CLn Ϊ 40bit ���У���ǰ�� 5 ���ֽڵ���򣡣���
		 */
        MfComData.MfData[1] = 0x20 + ((ucCollPosition / 8) << 4) + (ucBits & 0x0F);
        for (i=0; i<ucBytes; i++){					// ���ͻ���������
	        MfComData.MfData[i + 2] = ucSnr[i];
	    }
	    MfComData.MfLength = ucBytes + 2;			// ���������ֽ���
	
	    status = PcdCmdProcess(pi);
		
		#ifdef __MYDEBUG__
		USART_TransmitOne((unsigned char)0xcc);
		USART_TransmitOne(status);					// �˴�ӦΪ MI_OK/MI_COLLERR
		USART_TransmitOne(MfComData.MfLength);
		USART_TransmitOne(MfComData.mfcurrent);
		for(i=0; i<MfComData.mfcurrent; i++){
			USART_TransmitOne(MfComData.MfData[i]);
		}
		#endif
		
		
		// -------------------------- ��������Ԥ���� --------------------------
		/* RxAlign=7 ���ݿ��ܻᶪʧ����λλ�� 7��15��23��31��39(CollPos) 
		 * 		��⵽��λ��ͻ����ͨ�� RxAlign �������Ҫ�����ʵ��
		 */
		if(ucBits == 7){
			// xxxx xxx?  ---- ---[x]  ==> 3 bytes��
			if( (MfComData.MfLength%8)==0 ){
				MfComData.mfcurrent += 1;
			}
			
            dummyShift1 = 0x00;								// reorder received bits
			/*
			 *	���ʵ�� RxAlign=7 ʱ�������ݵ���λ�ָ�
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
	
		
		// -------------------------- ���µ�ǰ UID ��Ϣ --------------------------
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
 
	
	// ****************************** ��ʼ����λ���� ******************************  	
    ClearBitMask(RegDecoderControl,0x20);					// ZeroAfterColl disable
	
    return status;
}

/**
  * @brief  14443A ��������ͻ���� SELECT ����
  * @param  
  *		unsigned char *pSnr:	ÿ����������� UID CLn
  *		unsigned char *pSAK:	��Ƭ SAK
  *		unsigned char SelType:	ANTICOLLISION ����� SEL ���֣�
  *								0x93 Ϊ����һ��0x95 Ϊ�����,0x97 Ϊ������
  *	@ret ִ��״̬
  * @global none
  * @attention: 
  *		���� PcdCmdProcess()�뿨ͨѶ
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
	 * 	�� ѭ������ͻ������PCD ָ�� NVB Ϊ 0x70����ֵ��ʾ PCD ������������ UID
	 *	�� PCD ���� SEL �� NVB�����ŷ��� 40bit �� UID���������� Xor CheckSum
	 *	�� �� 40bit �� UID ƥ��� PICC���� SAK ��ΪӦ��
	 * 	�� ��� UID �������ģ�PICC �����ʹ��в��λΪ "0" �� SAK��ͬʱ�� READY תΪ ACTIVE ״̬
	 *	�� ��� ���λΪ "1"��������һ������ͻѭ�� 
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
        if (MfComData.MfLength != 0x8){			// һ���ֽڵ� SAK
			status = MI_BITCOUNTERR;
		}
    }
	
	*pSAK = MfComData.MfData[0];
	
    return status;
}

/**
  * @brief  ���� PCD/PICC 3 Pass Authentication
  * @param   
  *		auth_mode:	��֤��ʽ		0x60:��֤ A ��Կ
  *								0x61:��֤ B ��Կ
  *		blockNum:	Ҫ��֤�ľ��Կ��(0-63)
  *		pSnr:		���к��׵�ַ
  * @return 
  *		ִ��״̬
  * @attention
  *		���� PcdCmdProcess()�뿨ͨѶ
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
				// RxCRCEn������֡�����һ���ֽڱ�����ΪCRC�ֽڣ�CRC ��ȷ��CRC �ֽڲ����� FIFO��
				// 											   CRC ����CRCErr ��־��λ
				// TxCRCEn���Է��͵����ݽ��� CRC ���㲢�� CRC �ֽڼӵ�������������
				// ParityOdd��������������żУ�飬��� ISO14443A
				// ParityEn����żУ��λ��ÿ���ֽں���뷢���������в�������ڽ�����������ÿ���ֽں�
    PcdSetTmo(3);										// medium timeout (4.833 ms)
	
	/*
	 *	AUTHENT1��ִ�� Crypto1 ����֤�ĵ�һ����
	 *	���������� Auth ���Ҫ��֤�Ŀ��ľ��Կ��
	 *		  �������к�����ֽڡ�...���������к�����ֽ�
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
														// �������ֽڱ��������ֽ���Ч
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
  * @brief  �� Mifare_one ����һ��(block)����(16 �ֽ�)
  * @param  
  *		addr:		Ҫ���ľ��Կ��(0-63)
  *		pReaddata:	���������� 16 �ֽ�
  * @return 
  *		����״̬��
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
														// follow Typical Transaction Time(��2.5ms)
	
    WriteRawRC(RegChannelRedundancy,0x0F);				// RxCRC, TxCRC, Parity enable
				// RxCRCEn������֡�����һ���ֽڱ�����ΪCRC�ֽڣ�CRC ��ȷ��CRC �ֽڲ����� FIFO��
				// 											   CRC ����CRCErr ��־��λ
				// TxCRCEn���Է��͵����ݽ��� CRC ���㲢�� CRC �ֽڼӵ�������������
				// ParityOdd��������������żУ�飬��� ISO14443A
				// ParityEn����żУ��λ��ÿ���ֽں���뷢���������в�������ڽ�����������ÿ���ֽں�
	
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
  * @brief  �� Mifare_one ����һ��(block)����(16 �ֽ�)
  * @param  
  *		addr:		Ҫд�ľ��Կ��(0-63)
  *		pWritedata:	Ҫд������� 16 �ֽ�
  * @return 
  *		����״̬��
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
				// RxCRCEn������֡�����һ���ֽڱ�����ΪCRC�ֽڣ�CRC ��ȷ��CRC �ֽڲ����� FIFO��
				// 											   CRC ����CRCErr ��־��λ
				// TxCRCEn���Է��͵����ݽ��� CRC ���㲢�� CRC �ֽڼӵ�������������
				// ParityOdd��������������żУ�飬��� ISO14443A
				// ParityEn����żУ��λ��ÿ���ֽں���뷢���������в�������ڽ�����������ÿ���ֽں�

    MfComData.MfCommand = PCD_TRANSCEIVE;
    MfComData.MfLength  = 2;
    MfComData.MfData[0] = PICC_WRITE;					// Write command code
    MfComData.MfData[1] = addr;

    status = PcdCmdProcess(pi);

	/*
	 *	1. �ȷ��� PICC_WRITE ���������������ȴ�������Ӧ�ź�(1010)
	 *	2. ���ŷ��� 16 �ֽڵ����ݣ��ٴεȴ���Ӧ�ź�(1010)
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
														// follow Typical Transaction Time(��6.0ms)
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
  * @brief  �� Mifare_one �����������ݿ�(ֵ��)����ֵ(�ۿ�/��ֵ)����
  * @param  
  *		dd_mode:	����ѡ�����
  *			PICC_DECREMENT(0xC0):	�ۿ�	
  *			PICC_INCREMENT(0xC1):	��ֵ
  *		addr:		Ҫ�����ľ��Կ��(0-63)
  *		pValue:		4 �ֽ���(��)ֵ�׵�ַ��������16����������λ��ǰ
  * @return 
  *		����״̬��
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
		WriteRawRC(RegChannelRedundancy,0x07);		// ��� FM17XX оƬ�Ĳ�ͬ��
	#else
    	WriteRawRC(RegChannelRedundancy,0x07);		// RxCRC, TxCRC, Parity enable
				// RxCRCEn������֡�����һ���ֽڱ�����ΪCRC�ֽڣ�CRC ��ȷ��CRC �ֽڲ����� FIFO��
				// 											   CRC ����CRCErr ��־��λ
				// TxCRCEn���Է��͵����ݽ��� CRC ���㲢�� CRC �ֽڼӵ�������������
				// ParityOdd��������������żУ�飬��� ISO14443A
				// ParityEn����żУ��λ��ÿ���ֽں���뷢���������в�������ڽ�����������ÿ���ֽں�
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
			switch (MfComData.MfData[0]){			// ��Ӧ�ź�(1010)
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

