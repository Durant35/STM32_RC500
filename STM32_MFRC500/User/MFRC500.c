#include "MFRC500.h"
#include "ErrCode.h"
#include "SysTick.h"
#include "ParallelPort.h"
#include <string.h>

/**
  * @brief  复位并初始化 MF RC500
  * @param  none
  * @return none
  * @global none
  * @attention: 
  *		调用 PcdConfigISOType()进行相关场配置
  *		RC500 上电后应延时 500ms 才能可靠初始化
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
	
	//	配置 ISO/IEC Type-A 场并重启发射天线
	if(MI_OK == status){
		status = PcdConfigISOType('A');
	}
	
    return status;      
}

/**
  * @brief  复位 RC500
  * @param  none
  * @return 复位操作状态
  * @global none
  * @attention: 
  *		复位后进行并行接口配置并初始化为线性地址
  */
signed char PcdReset()
{
	signed char status = MI_OK;
	unsigned int i = 18000;			// 复位等待超时计数
	
									// 初始状态控制信号
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
	// 在整个启动阶段中，Command值读出为3FH
	while ((((ReadRawRC(RegCommand) & 0x3F)) != 0x3F) && i>0)
		i--;
	
	// while reset sequence in progress
	// 在初始化阶段的结束，MF RC500自动输入Idle命令，结果Command值变为00H
	while ((ReadRawRC(RegCommand) & 0x3F) && i>0)
		;

	if (i == 0){
	   status = MI_RESETERR;
	}
	else{
	   WriteRawRC(RegPage, 0x80);		// 将80H写入Page寄存器以初始化微处理器接口
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
  * @brief  配置 RC500 芯片的场调制
  * @param  
  *		@type 场类型
  *			type = 'A'：设置为TYPE_A方式
  *         type = 'B'：设置为TYPE_B方式
  *         type = 'r'：设置为AT88RF020卡方式
  *         type = 's'：设置为ST卡方式
  *         type = '1'：设置为ISO15693方式
  * @return 无
  * @global 无
  * @attention: 
  *			配置寄存器						目的				   配置值
  *		ClockQControl(0x1F)			使Q时钟复位后自动校准			0x40
  *			BitPhase(0x1B)			发送器和接收器时钟位相位			0xad(复位值)
  *		  RxThreshold(0x1C)		解码器输入端可接受的最小信号强度		0xff(复位值)
  *		  RxControl2(0x1E) 	  接收器始终有效，关闭接收器电路自动关闭  0x01
  *			FIFOLevel(0x29)	设置FIFO缓冲区上下限阈值(HiAlert/LoAlert)0x1a
  *		  TimerControl(0x2B)	当数据发送结束时，定时器自动停止		0x02
  *		 IRqPinConfig(0x2D)			IRQ低电平有效，CMOS输出			0x03
  */
signed char PcdConfigISOType(unsigned char type)
{
	if (type == 'A'){                 			//ISO14443_A
	    // test clock Q calibration(Q时钟自动校准) - value in the range of 0x46 expected
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
		WriteRawRC(RegBitPhase,0xad);    			// 复位值  

		// initialize minlevel
		WriteRawRC(RegRxThreshold,0xff);     		// 复位值  

		// disable auto power down
		WriteRawRC(RegRxControl2,0x01);				// 复位值 0x41

		// Depending on the processing speed of the
		// operation environment, the waterlevel 
		// can be adapted. (not very critical for
		// mifare applications)
		// initialize waterlevel to value 1a
  		WriteRawRC(RegFIFOLevel,0x1a);				// 复位值 0x08

		//Timer configuration
		WriteRawRC(RegTimerControl,0x02);  	// TStopRxEnd=0,TStopRxBeg=0,
											// TStartTxEnd=1,TStartTxBeg=0  
											//		数据发送结束时定时器自动停止
											// timer must be stopped manually
											// 复位值 0x06

		WriteRawRC(RegIRQPinConfig,0x03); 	// interrupt active low enable, push-pull
											// 复位值 0x02
		
		
//************************************************************* Parameters from SYSU-ZD
		// Page 2: Transmitter and control
		WriteRawRC(RegTxControl,0x5b);			// TX1/TX2 setting
		//WriteRawRC(RegCoderControl,0x19); 	// mifare 106kbd,ISO/IEC 14443 A,
												// mifare,ISO/IEC 14443A,miller coded
												// register only in CL RC632
		WriteRawRC(RegModWidth,0x13);        	// 复位值    
		//WriteRawRC(RegModWidthSOF,0x3f);   	// 0x3f: MIFARE and ISO/IEC 14443; 
												// modulation width SOF = 9.44 ms
												// register only in CL RC632												
		//WriteRawRC(RegTypeBFraming,0x3B);   	// 0x3b:EOF长度10个ETU,EGT长度6个ETU，SOF为11个ETU和3个ETU HIGH
												// 0x23, 定义ISO14443B帧格式
												// register only in CL RC632

		// Page 3: Receiver and decoder control
		WriteRawRC(RegRxControl1,0x73);        	// 复位值
		WriteRawRC(RegDecoderControl,0x08);		// 复位值
		//WriteRawRC(RegBitPhase,0xAD);        	// 复位值 follow cxdianzi
		//WriteRawRC(RegRxThreshold,0xAA);			// follow cxdianzi
		//WriteRawRC(RegBPSKDemControl,0);		// register only in CL RC632
		//WriteRawRC(RegRxControl2,0x01);        	// follow cxdianzi

		// Page 4: RF Timing and channel redundancy
		//WriteRawRC(RegRxWait,0x08);				// 复位值 0x06
		//WriteRawRC(RegChannelRedundancy,0x0F);	// 复位值 0x03
		WriteRawRC(RegCRCPresetLSB,0x63);		// 复位值
		WriteRawRC(RegCRCPresetMSB,0x63);		// 复位值
		//WriteRawRC(RegTimeSlotPeriod,0x00);	// register only in CL RC632
		//WriteRawRC(RegMFOUTSelect,0x02);		// not used
		//WriteRawRC(RegPreSet27,0x00);   	      

		// Page 5: FIFO, timer and IRQ pin configuration
		//WriteRawRC(RegFIFOLevel,0x3F);			// follow cxdianzi
		WriteRawRC(RegTimerClock,0x07);			//  复位值
		WriteRawRC(RegTimerReload,0x0A);		//  复位值
		//WriteRawRC(RegTimerControl,0x06);		// follow cxdianzi
		//WriteRawRC(RegIRQPinConfig,0x02);		// follow cxdianzi   
		//WriteRawRC(RegPreSet2E,0x00);
		//WriteRawRC(RegPreSet2F,0x00);	
		

//*************************************************************
		PcdSetTmo(1);               		// short timeout (1,0 ms)
		
											// Rf - reset and enable output driver
		PcdAntennaOff();
		Delay_ms(1);						// 启动或关闭天线发射之间至少要有1ms的间隔
		PcdAntennaOn();
	}else{ 
		return -1; 
	}
	
	return MI_OK;
}

/**
  * @brief  开启 MFRC500 天线
  * @param  none
  * @return 
  *		@MI_OK	成功开启
  *	@attention
 *		每次启动或关闭天线发射之间应至少有1ms的间隔
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
										// 管脚Tx1上的输出信号将传递由发送数据调制的13.56MHz能量载波
										// 管脚Tx2等同，且默认将传递一个反相的能量载波
        return MI_OK;
    }
}

/**
  * @brief  关闭 MFRC500 天线
  * @param  none
  * @return 
  *		@MI_OK	成功关闭
  */
signed char PcdAntennaOff(void)
{
    ClearBitMask(RegTxControl, 0x03);	// Tx2RF-En, Tx1RF-En disable
    return MI_OK;
}

/**
  * @brief  设置定时器单元定时周期
  * @param  定时周期选项
  * @return none
  * @attention: 
  *		通过修改 TimerClock 和 TimerReload 寄存器实现功能
  */
void PcdSetTmo(unsigned char tmoLength)
{
	switch(tmoLength){  						// timer clock frequency 13,56 MHz
												// 定时器始时钟 Timer=13.56MHz/(2^TPrescale)
												// 定时周期 TReloadVal/Timer
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
  * @brief  PCD 命令处理函数，主要是通过 RC500 和 ISO14443 卡通讯
  * @param  
  *		pi->MfCommand = RC500命令字
  *		pi->MfLength  = 参数的数据长度(单位: 字节)
  *		pi->MfData[]  = 参数数据
  * @return 
  *		pi->MfLength  = 返回数据长度(单位: bit)
  *		pi->MfData[]  = 返回数据
  *		signed char 处理结果(MI_OK、MI_COLLERR、MI_PARITYERR、MI_CRCERR、MI_NOTAGERR、MI_COM_ERR)
  * @attention: 需要修改 InterruptEn、InterruptRq、RegCommand、Control、FIFOData 寄存器
  */
signed char PcdCmdProcess(struct TranSciveBuffer *pi){
	unsigned char need_rece = 0; 				// 是否需要接收数据
	
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
			irqEn   = 0x11;		// LoAlert and TxIRq ==> IRQ引脚低电平触发
			waitFor = 0x10;		// TxIRq WriteE2命令，所有数据都已编程，不会自动停止
			break;
		case PCD_READE2:		
			irqEn   = 0x07;		// HiAlert, LoAlert and IdleIRq ==> IRQ引脚低电平触发
			waitFor = 0x04;		// PCD_READE2 命令数据传输完毕，自动停止
			need_rece = 1;		// 读取 E2PROM 需要接收指定长度的数据
			break;
		case PCD_LOADCONFIG:	
		case PCD_LOADKEYE2:	
			irqEn   = 0x05;		// LoAlert and IdleIRq ==> IRQ引脚低电平触发
			waitFor = 0x04;		// PCD_LOADCONFIG/PCD_LOADKEYE2/PCD_AUTHENT1 命令由其自身终止
			break;			
		case PCD_AUTHENT1:		
			irqEn   = 0x05;		// LoAlert and IdleIRq ==> IRQ引脚低电平触发
			waitFor = 0x0C;		// PCD_LOADCONFIG/PCD_LOADKEYE2/PCD_AUTHENT1 命令由其自身终止
			//need_rece = 1;      // Acknowledge
			break;
		case PCD_CALCCRC: 		// LoAlert and TxIRq
			irqEn   = 0x11;		// LoAlert and TxIRq ==> IRQ引脚低电平触发
			waitFor = 0x10;		// TxIRq CalCRC命令，所有数据都已处理，不会自动停止
			break;
		case PCD_AUTHENT2:		
			irqEn   = 0x04;		// IdleIRq ==> IRQ引脚低电平触发
			waitFor = 0x04;		// PCD_AUTHENT2 命令由其自身终止
			//need_rece = 1;      // Acknowledge
			break;
		case PCD_RECEIVE:
			irqEn   = 0x06;		// HiAlert and IdleIRq ==> IRQ引脚低电平触发
			waitFor = 0x04;		// PCD_RECEIVE 命令由其自身终止，置位RxIRq指示操作终止
			need_rece = 1;		// 返回数据流
			break;
		case PCD_LOADKEY:		// IdleIRq
			irqEn   = 0x05;		// LoAlert and IdleIRq ==> IRQ引脚低电平触发
			waitFor = 0x04;		// 密钥从 FIFO 缓冲区复制到密钥缓冲区后，自动停止，IdleIRq
			break;
		case PCD_TRANSMIT:
			irqEn   = 0x05;		// LoAlert and IdleIRq ==> IRQ引脚低电平触发
			waitFor = 0x04;		// PCD_TRANSMIT 命令由其自身终止，置位标志TxIRq 0x14
			break;
		case PCD_TRANSCEIVE:	// TimerIRq，
			irqEn   = 0x3D;		// TxIRq、RxIRq、IdleIRq and LoAlertIRq ==> IRQ引脚低电平触发 0x1D
			waitFor = 0x08;		// PCD_TRANSCEIVE 命令接收终止时 RxIRq 置位
			need_rece = 1;
			break;
		default:
			status = MI_UNKNOWN_COMMAND;
			break;
	}

	// 合法RC500命令字
	if (MI_OK == status){
		// 中断、缓冲区等状态清空
		WriteRawRC(RegInterruptEn, 0x7F);		// disable all interrupts
		WriteRawRC(RegInterruptRq, 0x7F);		// reset interrupt requests
		WriteRawRC(RegCommand, PCD_IDLE);		// terminate probably running command
		PcdFlushFIFO();

		
		// Initialize uC Timer for global Timeout management
		irqEn |= 0x20;                        	// always enable timout irq
		//waitFor |= 0x20;                      	// always wait for timeout 
		
		WriteRawRC(RegInterruptEn, irqEn|0x80);	// necessary interrupts are enabled(SetIRq)
		
	  
		// 先将发送数据前64字节写入FIFO
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
	  
		// 执行命令
		WriteRawRC(RegCommand, pi->MfCommand);		//start command
		SetBitMask(RegControl, 0x02);				//start MF RC500's timer now
		
		// 将超过64的后续字节数据依次写入FIFO ....... 此处可以考虑是否使用中断
		while((pi->mfcurrent) < (pi->MfLength)){
			if(ReadRawRC(RegFIFOLength) < 62){
				WriteRawRC(RegFIFOData, pi->MfData[pi->mfcurrent]);
				pi->mfcurrent++;
			}
		}
		
		//数据发送完毕，接收字节数清零
		pi->mfcurrent = 0;
		
		// 等待数据发送完毕(wait for cmd completion or timeout)
		// a command is completed, if the corresponding interrupt occurs or a timeout is signaled
		j = 0x3500;
		#ifdef __TRANSCEIVE_DEBUG__
		USART_TransmitOne(0xba);
		#endif
		do{
			j--;
			n = ReadRawRC(RegInterruptRq);
			if(n & 0x10){								// TxIRq ==> 所有参数数据都已处理
				// 接收数据 >32 先处理 ==> Transceive 命令
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
										// MF RC500's 定时器没超时置位或等待信号未置位或未软件超时
		#ifdef __TRANSCEIVE_DEBUG__
		USART_TransmitOne(0xba);
		USART_TransmitOne(n);
		USART_TransmitOne(waitFor);
		USART_TransmitOne(irqEn);
		#endif
		
		status = MI_COM_ERR;
		// 等待事件发生
		// 且数据发送结束时定时器不超时
		if( (n&waitFor) && (j>0) ){
			//for(i=15;i>0;i--);			// 缓冲时间
			for(i=30;i>0;i--);				// 缓冲时间
			
			// 查询是否出错
			erroflags = ReadRawRC(RegErrorFlag);
			
			#ifdef __WALLETDEBUG__
			USART_TransmitOne(0xee);
			USART_TransmitOne(erroflags);
			#endif
			
			// 无任何错误
			if(!(erroflags & 0x1F)){		// KeyErr/AccessErr需在启动相关命令时才会清零
				status = MI_OK;
				
				// M1卡命令需要返回数据
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
					
					// 判断最后接收字节的有效位个数(number of bits in the last byte)
					lastBits = ReadRawRC(RegSecondaryStatus) & 0x07;
					
					#ifdef __WALLETDEBUG__
					USART_TransmitOne(lastBits);
					#endif
					
					// 以下 pi->MfLength 为 bit 数
					if(lastBits){
						pi->MfLength = (pi->mfcurrent-1)*8 + lastBits;
					}
					else{				// RxLastBits[2:0]为 0 表示最后接收字节整个字节有效
						pi->MfLength = (pi->mfcurrent)*8;
					}
				} // end of if (need_rece)
			} // end of if (!(erroflags & 0x1F))
			else if(erroflags & 0x01){						// 检测到冲突
				status = MI_COLLERR;
				
				pi->mfcurrent = 1;
				pi->MfData[0] = ReadRawRC(RegCollPos);
				
				#ifdef __MYDEBUG__
				// 输出冲突位置
				USART_TransmitOne((unsigned char)0xbb);
				USART_TransmitOne(pi->MfData[0]);
				#endif
				
				// M1卡命令需要返回数据
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
					
					// 判断最后接收字节的有效位个数(number of bits in the last byte)
					lastBits = ReadRawRC(RegSecondaryStatus) & 0x07;
					
					// 以下 pi->MfLength 为 bit 数
					if(lastBits){
						pi->MfLength = (pi->mfcurrent-1)*8 + lastBits;
					}
					else{				// RxLastBits[2:0]为 0 表示最后接收字节整个字节有效
						pi->MfLength = (pi->mfcurrent)*8;
					}
				} // end of if (need_rece)
			} // end of else if (erroflags & 0x01)
			else if(erroflags & 0x02){						// 奇偶校验失败
				status = MI_PARITYERR;
				
				if(need_rece){
					n = ReadRawRC(RegFIFOLength);
					
					for (i=0; i<n; i++){
						pi->MfData[pi->mfcurrent+i] = ReadRawRC(RegFIFOData);
					}
					pi->mfcurrent += n;
					
					// 判断最后接收字节的有效位个数(number of bits in the last byte)
					lastBits = ReadRawRC(RegSecondaryStatus) & 0x07;
					
					// 以下 pi->MfLength 为 bit 数
					if(lastBits){
						pi->MfLength = (pi->mfcurrent-1)*8 + lastBits;
					}
					else{				// RxLastBits[2:0]为 0 表示最后接收字节整个字节有效
						pi->MfLength = (pi->mfcurrent)*8;
					}
				} // end of if (need_rece)
				
			}// end of else if(erroflags & 0x02)
			else if(erroflags & 0x04){						// SOF 不正确
				status = MI_FRAMINGERR;
			}
			else if(erroflags & 0x08){						// CRC 校验失败
				status = MI_CRCERR;
				
				if(need_rece){
					n = ReadRawRC(RegFIFOLength);
					
					for (i=0; i<n; i++){
						pi->MfData[pi->mfcurrent+i] = ReadRawRC(RegFIFOData);
					}
					pi->mfcurrent += n;
					
					// 判断最后接收字节的有效位个数(number of bits in the last byte)
					lastBits = ReadRawRC(RegSecondaryStatus) & 0x07;
					
					// 以下 pi->MfLength 为 bit 数
					if(lastBits){
						pi->MfLength = (pi->mfcurrent-1)*8 + lastBits;
					}
					else{				// RxLastBits[2:0]为 0 表示最后接收字节整个字节有效
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
			else{											// 未知状态
				//status = MI_UnknownERR;
			}
		}//end of if( (n&waitFor) && (j>0) )	
		else if(n & irqEn & 0x20){ 							// MF RC500's 定时器超时 
			status = MI_NOTAGERR;
		}
		else if(!(ReadRawRC(RegErrorFlag)&0x17)){   		// ErrorFlag 没有任何错误，通讯超时
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
  * @brief  将已转换格式后的密钥经 FIFO 缓冲区送到 RC500 的密匙缓冲区中
  * @param  
  *		pKeys:	已转换格式后的 12 字节密钥
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
	 *	LOADKEY：从 FIFO 缓冲区读出密匙字节并将其放入密匙缓冲区
	 *			注：密匙必须以指定的格式准备
	 *	经由 FIFO 通过的数据：密匙字节0(低)...密匙字节11(高)
	 *	LOADKEY 命令只能由微处理器启动，将密匙从 FIFO 缓冲区复制到密匙缓冲区后，自动终止
	 */
    MfComData.MfCommand = PCD_LOADKEY;
    MfComData.MfLength  = 12;
    memcpy(&MfComData.MfData[0], pKeys, 12);    

    status = PcdCmdProcess(pi);
 
	return status;
}

/**
  * @brief  读 RC500 EEPROM
  * @param  
  *		unsigned short startaddr:	EEPROM 地址(低位在前)
  *		unsigned char length:		读字节数
  *		unsigned char *readdata:	读出的数据
  * @return 
  *		执行状态
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
    MfComData.MfData[0] = startaddr&0xFF;					// 起始地址低字节
    MfComData.MfData[1] = (startaddr >> 8) & 0xFF;			// 起始地址高字节
    MfComData.MfData[2] = length;							// 数据字节个数

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
  * @brief  控制 RC500 清空 FIFO 缓冲区，为传输做准备
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
  * @brief  置 RC500 寄存器位
  * @param  
  *		@regAddr:	寄存器地址
  *		@mask:		置位值
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
  * @brief  清 RC500 寄存器位
  * @param  
  *		@regAddr:	寄存器地址
  *		@mask:		清位值
  *	@ret none
  * @global none
  */
void ClearBitMask(unsigned char regAddr, unsigned char mask)  
{
   char tmp = 0x00 ; 
   tmp = ReadRawRC(regAddr);
   WriteRawRC(regAddr, tmp & ~mask);  // clear bit mask
} 
