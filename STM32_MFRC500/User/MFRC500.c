#include "MFRC500.h"
#include "ErrCode.h"
#include "SysTick.h"
#include "ParallelPort.h"
#include <string.h>

/**
  * @brief  ��λ����ʼ�� MF RC500
  * @param  none
  * @return none
  * @global none
  * @attention: 
  *		���� PcdConfigISOType()������س�����
  *		RC500 �ϵ��Ӧ��ʱ 500ms ���ܿɿ���ʼ��
  */
signed char RC500Init(void)
{
	signed char status;

    Delay_ms(100);

    status = PcdReset();
    if(status != MI_OK){
        Delay_ms(10);
        status = PcdReset();
    } 
    if(status != MI_OK){
        Delay_ms(10);
        status = PcdReset();
    }
	
	//	���� ISO/IEC Type-A ����������������
	if(MI_OK == status){
		status = PcdConfigISOType('A');
	}
	
    return status;      
}

/**
  * @brief  ��λ RC500
  * @param  none
  * @return ��λ����״̬
  * @global none
  * @attention: 
  *		��λ����в��нӿ����ò���ʼ��Ϊ���Ե�ַ
  */
signed char PcdReset()
{
	signed char status = MI_OK;
	unsigned int i = 18000;			// ��λ�ȴ���ʱ����
	
									// ��ʼ״̬�����ź�
	NCS_H; 
	ALE_L; 
	NWR_H; 
	NRD_H; 

	NCS_L; 
	RST_L; 							// clear reset pin
	Delay_ms(25); 					// wait for 25ms
	RST_H; 							// hard power-down phase 
	Delay_us(2500); 				// wait for 2.5ms
	RST_L; 							// clear reset pin
	//NCS_L; 
	//Delay_ms(5); 
   
	// wait until reset command recognized
	// �����������׶��У�Commandֵ����Ϊ3FH
	while ((((ReadRawRC(RegCommand) & 0x3F)) != 0x3F) && i>0)
		i--;
	
	// while reset sequence in progress
	// �ڳ�ʼ���׶εĽ�����MF RC500�Զ�����Idle������Commandֵ��Ϊ00H
	while ((ReadRawRC(RegCommand) & 0x3F) && i>0)
		;

	if (i == 0){
	   status = MI_RESETERR;
	}
	else{
	   WriteRawRC(RegPage, 0x80);		// ��80Hд��Page�Ĵ����Գ�ʼ��΢�������ӿ�
										// Dummy access in order to determine the bus 
										// configuration
	   
	   // necessary read access 
	   // after first write access, the returned value
	   // should be zero ==> interface recognized
	   if (ReadRawRC(RegCommand) != 0x00){
		   status = MI_INTERFACEERR;
	   }
	   
	   WriteRawRC(RegPage,0x00); 		// configure to linear address mode
   }
   
	return status;
}

/**
  * @brief  ���� RC500 оƬ�ĳ�����
  * @param  
  *		@type ������
  *			type = 'A'������ΪTYPE_A��ʽ
  *         type = 'B'������ΪTYPE_B��ʽ
  *         type = 'r'������ΪAT88RF020����ʽ
  *         type = 's'������ΪST����ʽ
  *         type = '1'������ΪISO15693��ʽ
  * @return ��
  * @global ��
  * @attention: 
  *			���üĴ���						Ŀ��				   ����ֵ
  *		ClockQControl(0x1F)			ʹQʱ�Ӹ�λ���Զ�У׼			0x40
  *			BitPhase(0x1B)			�������ͽ�����ʱ��λ��λ			0xad(��λֵ)
  *		  RxThreshold(0x1C)		����������˿ɽ��ܵ���С�ź�ǿ��		0xff(��λֵ)
  *		  RxControl2(0x1E) 	  ������ʼ����Ч���رս�������·�Զ��ر�  0x01
  *			FIFOLevel(0x29)	����FIFO��������������ֵ(HiAlert/LoAlert)0x1a
  *		  TimerControl(0x2B)	�����ݷ��ͽ���ʱ����ʱ���Զ�ֹͣ		0x02
  *		 IRqPinConfig(0x2D)			IRQ�͵�ƽ��Ч��CMOS���			0x03
  */
signed char PcdConfigISOType(unsigned char type)
{
	if (type == 'A'){                 			//ISO14443_A
	    // test clock Q calibration(Qʱ���Զ�У׼) - value in the range of 0x46 expected
		WriteRawRC(RegClockQControl,0x0);
		WriteRawRC(RegClockQControl,0x40);
		Delay_us(100);  						// wait approximately 100 us 
												//	---- calibration in progress
		ClearBitMask(RegClockQControl,0x40); 	// clear bit ClkQCalib for 
												// further calibration
		
		
//************************************************************* Parameters from cxdianzi												
		// The following values for RegBitPhase and
		// RegRxThreshold represents an optimal
		// value for our demo package(cxdianzi). For user
		// implementation some changes could be necessary
		// initialize bit phase
		WriteRawRC(RegBitPhase,0xad);    			// ��λֵ  

		// initialize minlevel
		WriteRawRC(RegRxThreshold,0xff);     		// ��λֵ  

		// disable auto power down
		WriteRawRC(RegRxControl2,0x01);				// ��λֵ 0x41

		// Depending on the processing speed of the
		// operation environment, the waterlevel 
		// can be adapted. (not very critical for
		// mifare applications)
		// initialize waterlevel to value 1a
  		WriteRawRC(RegFIFOLevel,0x1a);				// ��λֵ 0x08

		//Timer configuration
		WriteRawRC(RegTimerControl,0x02);  	// TStopRxEnd=0,TStopRxBeg=0,
											// TStartTxEnd=1,TStartTxBeg=0  
											//		���ݷ��ͽ���ʱ��ʱ���Զ�ֹͣ
											// timer must be stopped manually
											// ��λֵ 0x06

		WriteRawRC(RegIRQPinConfig,0x03); 	// interrupt active low enable, push-pull
											// ��λֵ 0x02
		
		
//************************************************************* Parameters from SYSU-ZD
		// Page 2: Transmitter and control
		WriteRawRC(RegTxControl,0x5b);			// TX1/TX2 setting
		//WriteRawRC(RegCoderControl,0x19); 	// mifare 106kbd,ISO/IEC 14443 A,
												// mifare,ISO/IEC 14443A,miller coded
												// register only in CL RC632
		WriteRawRC(RegModWidth,0x13);        	// ��λֵ    
		//WriteRawRC(RegModWidthSOF,0x3f);   	// 0x3f: MIFARE and ISO/IEC 14443; 
												// modulation width SOF = 9.44 ms
												// register only in CL RC632												
		//WriteRawRC(RegTypeBFraming,0x3B);   	// 0x3b:EOF����10��ETU,EGT����6��ETU��SOFΪ11��ETU��3��ETU HIGH
												// 0x23, ����ISO14443B֡��ʽ
												// register only in CL RC632

		// Page 3: Receiver and decoder control
		WriteRawRC(RegRxControl1,0x73);        	// ��λֵ
		WriteRawRC(RegDecoderControl,0x08);		// ��λֵ
		//WriteRawRC(RegBitPhase,0xAD);        	// ��λֵ follow cxdianzi
		//WriteRawRC(RegRxThreshold,0xAA);			// follow cxdianzi
		//WriteRawRC(RegBPSKDemControl,0);		// register only in CL RC632
		//WriteRawRC(RegRxControl2,0x01);        	// follow cxdianzi

		// Page 4: RF Timing and channel redundancy
		//WriteRawRC(RegRxWait,0x08);				// ��λֵ 0x06
		//WriteRawRC(RegChannelRedundancy,0x0F);	// ��λֵ 0x03
		WriteRawRC(RegCRCPresetLSB,0x63);		// ��λֵ
		WriteRawRC(RegCRCPresetMSB,0x63);		// ��λֵ
		//WriteRawRC(RegTimeSlotPeriod,0x00);	// register only in CL RC632
		//WriteRawRC(RegMFOUTSelect,0x02);		// not used
		//WriteRawRC(RegPreSet27,0x00);   	      

		// Page 5: FIFO, timer and IRQ pin configuration
		//WriteRawRC(RegFIFOLevel,0x3F);			// follow cxdianzi
		WriteRawRC(RegTimerClock,0x07);			//  ��λֵ
		WriteRawRC(RegTimerReload,0x0A);		//  ��λֵ
		//WriteRawRC(RegTimerControl,0x06);		// follow cxdianzi
		//WriteRawRC(RegIRQPinConfig,0x02);		// follow cxdianzi   
		//WriteRawRC(RegPreSet2E,0x00);
		//WriteRawRC(RegPreSet2F,0x00);	
		

//*************************************************************
		PcdSetTmo(1);               		// short timeout (1,0 ms)
		
											// Rf - reset and enable output driver
		PcdAntennaOff();
		Delay_ms(1);						// ������ر����߷���֮������Ҫ��1ms�ļ��
		PcdAntennaOn();
	}else{ 
		return -1; 
	}
	
	return MI_OK;
}

/**
  * @brief  ���� MFRC500 ����
  * @param  none
  * @return 
  *		@MI_OK	�ɹ�����
  *	@attention
 *		ÿ��������ر����߷���֮��Ӧ������1ms�ļ��
  */
signed char PcdAntennaOn(void)
{
    unsigned char i;
    i = ReadRawRC(RegTxControl);
    if (i & 0x03){  
		return MI_OK;
	}
    else{
        SetBitMask(RegTxControl, 0x03);	// Tx2RF-En, Tx1RF-En enable
										// �ܽ�Tx1�ϵ�����źŽ������ɷ������ݵ��Ƶ�13.56MHz�����ز�
										// �ܽ�Tx2��ͬ����Ĭ�Ͻ�����һ������������ز�
        return MI_OK;
    }
}

/**
  * @brief  �ر� MFRC500 ����
  * @param  none
  * @return 
  *		@MI_OK	�ɹ��ر�
  */
signed char PcdAntennaOff(void)
{
    ClearBitMask(RegTxControl, 0x03);	// Tx2RF-En, Tx1RF-En disable
    return MI_OK;
}

/**
  * @brief  ���ö�ʱ����Ԫ��ʱ����
  * @param  ��ʱ����ѡ��
  * @return none
  * @attention: 
  *		ͨ���޸� TimerClock �� TimerReload �Ĵ���ʵ�ֹ���
  */
void PcdSetTmo(unsigned char tmoLength)
{
	switch(tmoLength){  						// timer clock frequency 13,56 MHz
												// ��ʱ��ʼʱ�� Timer=13.56MHz/(2^TPrescale)
												// ��ʱ���� TReloadVal/Timer
		case 1:                       			// short timeout (1,0 ms)
			WriteRawRC(RegTimerClock,0x07); 	// TAutoRestart=0,TPrescale=2^7=128
			WriteRawRC(RegTimerReload,0x6a);	// TReloadVal = 'h6a =106(dec) 
			break;
		case 2:                       			// short timeout (1,5 ms)
			WriteRawRC(RegTimerClock,0x07); 	// TAutoRestart=0,TPrescale=2^7=128
			WriteRawRC(RegTimerReload,0xa0);	// TReloadVal = 'ha0 =160(dec) 
			break;
		case 3:                         		// medium timeout (4.833 ms) 
			WriteRawRC(RegTimerClock,0x09); 	// TAutoRestart=0,TPrescale=2^9=4*128
			WriteRawRC(RegTimerReload,0x81);	// TReloadVal = 'h81 =129(dec) 
			break;
		case 4:                       			// medium timeout (6 ms)
			WriteRawRC(RegTimerClock,0x09); 	// TAutoRestart=0,TPrescale=2^9=4*128
			WriteRawRC(RegTimerReload,0xa0);	// TReloadVal = 'ha0 =160(dec) 
			break;
		case 5:                       			// medium timeout (9.6 ms)
			WriteRawRC(RegTimerClock,0x09); 	// TAutoRestart=0,TPrescale=2^9=4*128
			WriteRawRC(RegTimerReload,0xff);	// TReloadVal = 'hff =255(dec) 
			break;
		case 6:                       			// long timeout (38.5 ms)
			WriteRawRC(RegTimerClock,0x0b); 	// TAutoRestart=0,TPrescale=2^11=16*128
			WriteRawRC(RegTimerReload,0xff);	// TReloadVal = 'hff =255(dec) 
			break;
		case 7:                       			// long timeout (154 ms)
			WriteRawRC(RegTimerClock,0x0d); 	// TAutoRestart=0,TPrescale=2^13=64*128
			WriteRawRC(RegTimerReload,0xff);	// TReloadVal = 'hff =255(dec) 
			break;
		case 8:                       			// very long timeout (616.2 ms)
			WriteRawRC(RegTimerClock,0x0f); 	// TAutoRestart=0,TPrescale=2^15=256*128
			WriteRawRC(RegTimerReload,0xff);	// TReloadVal = 'hff =255(dec) 
			break;
		default:                       
			WriteRawRC(RegTimerClock,0x07); 		// TAutoRestart=0,TPrescale=2^7=128
			WriteRawRC(RegTimerReload,tmoLength);	// TReloadVal = tmoLength(dec) 
			break;
	}
}

/**
  * @brief  PCD �����������Ҫ��ͨ�� RC500 �� ISO14443 ��ͨѶ
  * @param  
  *		pi->MfCommand = RC500������
  *		pi->MfLength  = ���������ݳ���(��λ: �ֽ�)
  *		pi->MfData[]  = ��������
  * @return 
  *		pi->MfLength  = �������ݳ���(��λ: bit)
  *		pi->MfData[]  = ��������
  *		signed char ������(MI_OK��MI_COLLERR��MI_PARITYERR��MI_CRCERR��MI_NOTAGERR��MI_COM_ERR)
  * @attention: ��Ҫ�޸� InterruptEn��InterruptRq��RegCommand��Control��FIFOData �Ĵ���
  */
signed char PcdCmdProcess(struct TranSciveBuffer *pi){
	unsigned char need_rece = 0; 				// �Ƿ���Ҫ��������
	
	int status = MI_OK;
	
	unsigned char erroflags;
	unsigned char irqEn   = 0x00;
	unsigned char waitFor = 0x00;
	unsigned char lastBits;
	unsigned char n;
	unsigned int i,j;
	unsigned int templength;
	
	// depending on the command code, appropriate interrupts are enabled (irqEn)
	// and the commit interrupt is choosen (waitFor).
	switch (pi->MfCommand){
		case PCD_IDLE:			// nothing else required
			irqEn   = 0x00;
			waitFor = 0x00;
			break;
		case PCD_WRITEE2:		
			irqEn   = 0x11;		// LoAlert and TxIRq ==> IRQ���ŵ͵�ƽ����
			waitFor = 0x10;		// TxIRq WriteE2����������ݶ��ѱ�̣������Զ�ֹͣ
			break;
		case PCD_READE2:		
			irqEn   = 0x07;		// HiAlert, LoAlert and IdleIRq ==> IRQ���ŵ͵�ƽ����
			waitFor = 0x04;		// PCD_READE2 �������ݴ�����ϣ��Զ�ֹͣ
			need_rece = 1;		// ��ȡ E2PROM ��Ҫ����ָ�����ȵ�����
			break;
		case PCD_LOADCONFIG:	
		case PCD_LOADKEYE2:	
			irqEn   = 0x05;		// LoAlert and IdleIRq ==> IRQ���ŵ͵�ƽ����
			waitFor = 0x04;		// PCD_LOADCONFIG/PCD_LOADKEYE2/PCD_AUTHENT1 ��������������ֹ
			break;			
		case PCD_AUTHENT1:		
			irqEn   = 0x05;		// LoAlert and IdleIRq ==> IRQ���ŵ͵�ƽ����
			waitFor = 0x0C;		// PCD_LOADCONFIG/PCD_LOADKEYE2/PCD_AUTHENT1 ��������������ֹ
			//need_rece = 1;      // Acknowledge
			break;
		case PCD_CALCCRC: 		// LoAlert and TxIRq
			irqEn   = 0x11;		// LoAlert and TxIRq ==> IRQ���ŵ͵�ƽ����
			waitFor = 0x10;		// TxIRq CalCRC����������ݶ��Ѵ��������Զ�ֹͣ
			break;
		case PCD_AUTHENT2:		
			irqEn   = 0x04;		// IdleIRq ==> IRQ���ŵ͵�ƽ����
			waitFor = 0x04;		// PCD_AUTHENT2 ��������������ֹ
			//need_rece = 1;      // Acknowledge
			break;
		case PCD_RECEIVE:
			irqEn   = 0x06;		// HiAlert and IdleIRq ==> IRQ���ŵ͵�ƽ����
			waitFor = 0x04;		// PCD_RECEIVE ��������������ֹ����λRxIRqָʾ������ֹ
			need_rece = 1;		// ����������
			break;
		case PCD_LOADKEY:		// IdleIRq
			irqEn   = 0x05;		// LoAlert and IdleIRq ==> IRQ���ŵ͵�ƽ����
			waitFor = 0x04;		// ��Կ�� FIFO ���������Ƶ���Կ���������Զ�ֹͣ��IdleIRq
			break;
		case PCD_TRANSMIT:
			irqEn   = 0x05;		// LoAlert and IdleIRq ==> IRQ���ŵ͵�ƽ����
			waitFor = 0x04;		// PCD_TRANSMIT ��������������ֹ����λ��־TxIRq 0x14
			break;
		case PCD_TRANSCEIVE:	// TimerIRq��
			irqEn   = 0x3D;		// TxIRq��RxIRq��IdleIRq and LoAlertIRq ==> IRQ���ŵ͵�ƽ���� 0x1D
			waitFor = 0x08;		// PCD_TRANSCEIVE ���������ֹʱ RxIRq ��λ
			need_rece = 1;
			break;
		default:
			status = MI_UNKNOWN_COMMAND;
			break;
	}

	// �Ϸ�RC500������
	if (MI_OK == status){
		// �жϡ���������״̬���
		WriteRawRC(RegInterruptEn, 0x7F);		// disable all interrupts
		WriteRawRC(RegInterruptRq, 0x7F);		// reset interrupt requests
		WriteRawRC(RegCommand, PCD_IDLE);		// terminate probably running command
		PcdFlushFIFO();

		
		// Initialize uC Timer for global Timeout management
		irqEn |= 0x20;                        	// always enable timout irq
		//waitFor |= 0x20;                      	// always wait for timeout 
		
		WriteRawRC(RegInterruptEn, irqEn|0x80);	// necessary interrupts are enabled(SetIRq)
		
	  
		// �Ƚ���������ǰ64�ֽ�д��FIFO
		pi->mfcurrent = 0;
		if(pi->MfLength > FIFO_LENGTH){
			templength = FIFO_LENGTH;
		}
		else{
			templength = pi->MfLength;
		}
		#ifdef __MYDEBUG__
		USART_TransmitOne(0xaa);
		USART_TransmitOne(templength&0xFF);
		#endif
		for (i=0; i<templength; i++){
			WriteRawRC(RegFIFOData, pi->MfData[i]);
			#ifdef __MYDEBUG__
			USART_TransmitOne(pi->MfData[i]);
			#endif
		}
		pi->mfcurrent += templength;
	  
		// ִ������
		WriteRawRC(RegCommand, pi->MfCommand);		//start command
		SetBitMask(RegControl, 0x02);				//start MF RC500's timer now
		
		// ������64�ĺ����ֽ���������д��FIFO ....... �˴����Կ����Ƿ�ʹ���ж�
		while((pi->mfcurrent) < (pi->MfLength)){
			if(ReadRawRC(RegFIFOLength) < 62){
				WriteRawRC(RegFIFOData, pi->MfData[pi->mfcurrent]);
				pi->mfcurrent++;
			}
		}
		
		//���ݷ�����ϣ������ֽ�������
		pi->mfcurrent = 0;
		
		// �ȴ����ݷ������(wait for cmd completion or timeout)
		// a command is completed, if the corresponding interrupt occurs or a timeout is signaled
		j = 0x3500;
		#ifdef __TRANSCEIVE_DEBUG__
		USART_TransmitOne(0xba);
		#endif
		do{
			j--;
			n = ReadRawRC(RegInterruptRq);
			if(n & 0x10){								// TxIRq ==> ���в������ݶ��Ѵ���
				// �������� >32 �ȴ��� ==> Transceive ����
				if(ReadRawRC(RegFIFOLength)>32){
					for(i=0; i<32; i++){
						pi->MfData[pi->mfcurrent+i] = ReadRawRC(RegFIFOData);
					}
					pi->mfcurrent += 32;
				}
			}
			#ifdef __TRANSCEIVE_DEBUG__
			USART_TransmitOne(n);
			#endif
		}while ( (!(n & irqEn & 0x20)) && (!(n & waitFor)) && (j > 0) );
										// MF RC500's ��ʱ��û��ʱ��λ��ȴ��ź�δ��λ��δ�����ʱ
		#ifdef __TRANSCEIVE_DEBUG__
		USART_TransmitOne(0xba);
		USART_TransmitOne(n);
		USART_TransmitOne(waitFor);
		USART_TransmitOne(irqEn);
		#endif
		
		status = MI_COM_ERR;
		// �ȴ��¼�����
		// �����ݷ��ͽ���ʱ��ʱ������ʱ
		if( (n&waitFor) && (j>0) ){
			//for(i=15;i>0;i--);			// ����ʱ��
			for(i=30;i>0;i--);				// ����ʱ��
			
			// ��ѯ�Ƿ����
			erroflags = ReadRawRC(RegErrorFlag);
			
			#ifdef __WALLETDEBUG__
			USART_TransmitOne(0xee);
			USART_TransmitOne(erroflags);
			#endif
			
			// ���κδ���
			if(!(erroflags & 0x1F)){		// KeyErr/AccessErr���������������ʱ�Ż�����
				status = MI_OK;
				
				// M1��������Ҫ��������
				if (need_rece){
					n = ReadRawRC(RegFIFOLength);
					
					#ifdef __WALLETDEBUG__
					USART_TransmitOne(0xcd);
					USART_TransmitOne(n);
					#endif
					
					for (i=0; i<n; i++){
						pi->MfData[pi->mfcurrent+i] = ReadRawRC(RegFIFOData);
						
						#ifdef __WALLETDEBUG__
						USART_TransmitOne(pi->MfData[pi->mfcurrent+i]);
						#endif
					}
					pi->mfcurrent += n;
					
					// �ж��������ֽڵ���Чλ����(number of bits in the last byte)
					lastBits = ReadRawRC(RegSecondaryStatus) & 0x07;
					
					#ifdef __WALLETDEBUG__
					USART_TransmitOne(lastBits);
					#endif
					
					// ���� pi->MfLength Ϊ bit ��
					if(lastBits){
						pi->MfLength = (pi->mfcurrent-1)*8 + lastBits;
					}
					else{				// RxLastBits[2:0]Ϊ 0 ��ʾ�������ֽ������ֽ���Ч
						pi->MfLength = (pi->mfcurrent)*8;
					}
				} // end of if (need_rece)
			} // end of if (!(erroflags & 0x1F))
			else if(erroflags & 0x01){						// ��⵽��ͻ
				status = MI_COLLERR;
				
				pi->mfcurrent = 1;
				pi->MfData[0] = ReadRawRC(RegCollPos);
				
				#ifdef __MYDEBUG__
				// �����ͻλ��
				USART_TransmitOne((unsigned char)0xbb);
				USART_TransmitOne(pi->MfData[0]);
				#endif
				
				// M1��������Ҫ��������
				if(need_rece){
					n = ReadRawRC(RegFIFOLength);
					
					#ifdef __MYDEBUG__
					USART_TransmitOne(0xce);
					USART_TransmitOne(n);
					#endif
					
					for (i=0; i<n; i++){
						pi->MfData[pi->mfcurrent+i] = ReadRawRC(RegFIFOData);
					}
					pi->mfcurrent += n;
					
					// �ж��������ֽڵ���Чλ����(number of bits in the last byte)
					lastBits = ReadRawRC(RegSecondaryStatus) & 0x07;
					
					// ���� pi->MfLength Ϊ bit ��
					if(lastBits){
						pi->MfLength = (pi->mfcurrent-1)*8 + lastBits;
					}
					else{				// RxLastBits[2:0]Ϊ 0 ��ʾ�������ֽ������ֽ���Ч
						pi->MfLength = (pi->mfcurrent)*8;
					}
				} // end of if (need_rece)
			} // end of else if (erroflags & 0x01)
			else if(erroflags & 0x02){						// ��żУ��ʧ��
				status = MI_PARITYERR;
				
				if(need_rece){
					n = ReadRawRC(RegFIFOLength);
					
					for (i=0; i<n; i++){
						pi->MfData[pi->mfcurrent+i] = ReadRawRC(RegFIFOData);
					}
					pi->mfcurrent += n;
					
					// �ж��������ֽڵ���Чλ����(number of bits in the last byte)
					lastBits = ReadRawRC(RegSecondaryStatus) & 0x07;
					
					// ���� pi->MfLength Ϊ bit ��
					if(lastBits){
						pi->MfLength = (pi->mfcurrent-1)*8 + lastBits;
					}
					else{				// RxLastBits[2:0]Ϊ 0 ��ʾ�������ֽ������ֽ���Ч
						pi->MfLength = (pi->mfcurrent)*8;
					}
				} // end of if (need_rece)
				
			}// end of else if(erroflags & 0x02)
			else if(erroflags & 0x04){						// SOF ����ȷ
				status = MI_FRAMINGERR;
			}
			else if(erroflags & 0x08){						// CRC У��ʧ��
				status = MI_CRCERR;
				
				if(need_rece){
					n = ReadRawRC(RegFIFOLength);
					
					for (i=0; i<n; i++){
						pi->MfData[pi->mfcurrent+i] = ReadRawRC(RegFIFOData);
					}
					pi->mfcurrent += n;
					
					// �ж��������ֽڵ���Чλ����(number of bits in the last byte)
					lastBits = ReadRawRC(RegSecondaryStatus) & 0x07;
					
					// ���� pi->MfLength Ϊ bit ��
					if(lastBits){
						pi->MfLength = (pi->mfcurrent-1)*8 + lastBits;
					}
					else{				// RxLastBits[2:0]Ϊ 0 ��ʾ�������ֽ������ֽ���Ч
						pi->MfLength = (pi->mfcurrent)*8;
					}
				} // end of if (need_rece)
				
			} // end of else if(erroflags & 0x08)
			else if(erroflags & 0x10){						// FIFOvfl
				status = MI_FIFOERR;
			}
			else if(erroflags & 0x20){						// AccessErr
				if(PCD_WRITEE2 == pi->MfCommand || PCD_READE2 == pi->MfCommand)
					status = MI_ACCESSERR;
			}
			else if(erroflags & 0x80){						// KeyErr
				if(PCD_LOADKEY == pi->MfCommand || PCD_LOADKEYE2 == pi->MfCommand)
					status = MI_KEYERR;
			}
			else{											// δ֪״̬
				//status = MI_UnknownERR;
			}
		}//end of if( (n&waitFor) && (j>0) )	
		else if(n & irqEn & 0x20){ 							// MF RC500's ��ʱ����ʱ 
			status = MI_NOTAGERR;
		}
		else if(!(ReadRawRC(RegErrorFlag)&0x17)){   		// ErrorFlag û���κδ���ͨѶ��ʱ
			status = MI_ACCESSTIMEOUT;   
		}
		else{   
			status = MI_COM_ERR;    
		}
		
		WriteRawRC(RegInterruptEn, 0x7F);					// disable all interrupts
		WriteRawRC(RegInterruptRq, 0x7F);					// clear all interrupt requests
		SetBitMask(RegControl, 0x04);           			// stop MF RC500's timer now
		WriteRawRC(RegCommand, PCD_IDLE); 					// reset command register
	} // end of if (status == MI_OK)
	#ifdef __TRANSCEIVE_DEBUG__
	USART_TransmitOne(status);
	#endif
	return status;
}

/**
  * @brief  ����ת����ʽ�����Կ�� FIFO �������͵� RC500 ���ܳ׻�������
  * @param  
  *		pKeys:	��ת����ʽ��� 12 �ֽ���Կ
  * @return 
  *		error flags of the key load: MI_KEYERR/MI_AUTHERR(generic authentication error)
  * @attention
  *		none
  * @global 
  *		none
  */
signed char PcdAuthKey(unsigned char *pKeys)
{
    signed char status;
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;

    PcdSetTmo(3);							// medium timeout (4.833 ms)
	
	/*
	 *	LOADKEY���� FIFO �����������ܳ��ֽڲ���������ܳ׻�����
	 *			ע���ܳױ�����ָ���ĸ�ʽ׼��
	 *	���� FIFO ͨ�������ݣ��ܳ��ֽ�0(��)...�ܳ��ֽ�11(��)
	 *	LOADKEY ����ֻ����΢���������������ܳ״� FIFO ���������Ƶ��ܳ׻��������Զ���ֹ
	 */
    MfComData.MfCommand = PCD_LOADKEY;
    MfComData.MfLength  = 12;
    memcpy(&MfComData.MfData[0], pKeys, 12);    

    status = PcdCmdProcess(pi);
 
	return status;
}

/**
  * @brief  �� RC500 EEPROM
  * @param  
  *		unsigned short startaddr:	EEPROM ��ַ(��λ��ǰ)
  *		unsigned char length:		���ֽ���
  *		unsigned char *readdata:	����������
  * @return 
  *		ִ��״̬
  * @attention
  *		none
  * @global 
  *		none
  */
signed char RC500_ReadE2(unsigned short startaddr, unsigned char length, unsigned char *readdata)
{
    char status = MI_OK;
	
    struct TranSciveBuffer MfComData;
    struct TranSciveBuffer *pi;
    pi = &MfComData;

    MfComData.MfCommand = PCD_READE2;
    MfComData.MfLength  = 3;
    MfComData.MfData[0] = startaddr&0xFF;					// ��ʼ��ַ���ֽ�
    MfComData.MfData[1] = (startaddr >> 8) & 0xFF;			// ��ʼ��ַ���ֽ�
    MfComData.MfData[2] = length;							// �����ֽڸ���

    status = PcdCmdProcess(pi);

    if (status == MI_OK){   
		memcpy(readdata, &MfComData.MfData[0], length);    
	}
	else{
		readdata[0] = 0;
	}
	
    return status;
}

/**
  * @brief  ���� RC500 ��� FIFO ��������Ϊ������׼��
  * @param  
  *		none
  * @return 
  *		none
  * @attention
  *		none
  * @global 
  *		none
  */
void PcdFlushFIFO(void)
{
	SetBitMask(RegControl, 0x01);			// flush FIFO buffer(0x09)
}

/**
  * @brief  �� RC500 �Ĵ���λ
  * @param  
  *		@regAddr:	�Ĵ�����ַ
  *		@mask:		��λֵ
  *	@ret none
  * @global none
  */
void SetBitMask(unsigned char regAddr, unsigned char mask)  
{
   char tmp = 0x00; 
   tmp = ReadRawRC(regAddr);
   WriteRawRC(regAddr, tmp | mask);  // set bit mask
}

/**
  * @brief  �� RC500 �Ĵ���λ
  * @param  
  *		@regAddr:	�Ĵ�����ַ
  *		@mask:		��λֵ
  *	@ret none
  * @global none
  */
void ClearBitMask(unsigned char regAddr, unsigned char mask)  
{
   char tmp = 0x00 ; 
   tmp = ReadRawRC(regAddr);
   WriteRawRC(regAddr, tmp & ~mask);  // clear bit mask
} 
