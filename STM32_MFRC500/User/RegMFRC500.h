#ifndef __REG_MF_RC500_H__
#define __REG_MF_RC500_H__

/***********************************************************
 *
 *	FileName: 	RegMFRC500.h
 *	Desription:	MFRC500 �Ĵ�������
 * 		��ϸ�ο���MFRC500 Datasheet(������ϸ��)����5��
 *
 ***********************************************************/
 
//***************** MFRC500 Register Sets *****************//
// ҳ#0 �����״̬
#define 	RegPage 				0x00				// ѡ��Ĵ���ҳ
#define 	RegCommand 				0x01				// ������ֹͣ�����ִ��
#define 	RegFIFOData 			0x02				// 64�ֽ�FIFO��������������
#define 	RegPrimaryStatus 		0x03				// �������ͷ������Լ�FIFO������״̬��־
#define 	RegFIFOLength 			0x04				// FIFO�л�����ֽ���
#define 	RegSecondaryStatus		0x05				// ��ͬ��״̬��־
#define 	RegInterruptEn 			0x06				// ʹ�ܺͽ�ֹ�ж�����ͨ���Ŀ���λ
#define 	RegInterruptRq 			0x07				// �ж������־
// ҳ#1 ���ƺ�״̬
//#define 	RegPage 				0x08
#define 	RegControl 				0x09				// ��ͬ�Ŀ��Ʊ�־,����:��ʱ�����ڵ�
#define 	RegErrorFlag 			0x0A				// ��ʾ�ϴ�����ִ�д���״̬�Ĵ����־
#define 	RegCollPos 				0x0B				// RF�ӿڼ�⵽�ĵ�һ����ͻλ��λ��
#define 	RegTimerValue 			0x0C				// ��ʱ����ʵ��ֵ
#define 	RegCRCResultLSB 		0x0D				// CRCЭ�������Ĵ��������λ
#define 	RegCRCResultMSB 		0x0E				// CRCЭ�������Ĵ��������λ
#define 	RegBitFraming 			0x0F				// λ��ʽ֡�ĵ���
// ҳ#2 �������ͱ���������
//#define 	RegPage 				0x10				// 
#define 	RegTxControl 			0x11				// ����������TX1��TX2���߼�״̬����
#define 	RegCWConductance 		0x12				// ѡ������������TX1��TX2�ĵ絼��
#define 	RegPreSet13 			0x13				// ��ֵ����ı�
#define 	RegPreSet14 			0x14				// ��ֵ����ı�
#define 	RegModWidth 			0x15				// ѡ���������Ŀ��
#define 	RegPreSet16 			0x16				// ��ֵ����ı�
#define 	RegPreSet17 			0x17				// ��ֵ����ı�
// ҳ#3 �������ͽ������
//#define 	RegPage 				0x18
#define 	RegRxControl1 			0x19				// ���ƽ�����״̬
#define 	RegDecoderControl 		0x1A				// ���ƽ�����״̬
#define 	RegBitPhase 			0x1B				// ѡ�������ͽ�����ʱ��֮���λ��λ
#define 	RegRxThreshold 			0x1C				// ѡ��λ�������ķ�ֵ
#define 	RegPreSet1D 			0x1D				// ��ֵ����ı�
#define 	RegRxControl2 			0x1E				// ���ƽ�����״̬�Ͷ��������������Դ
#define 	RegClockQControl 		0x1F				// ����ʱ�Ӳ�������90�����Ƶ�Q�ŵ�ʱ��
// ҳ#4 ʱ����ŵ�����
//#define 	RegPage 				0x20
#define 	RegRxWait 				0x21				// ѡ���ͺ����������ǰ��ʱ����
#define 	RegChannelRedundancy 	0x22				// ѡ��RF�ŵ������������Լ������ͺ�ģʽ
#define 	RegCRCPresetLSB 		0x23				// CRC�Ĵ���Ԥ��ֵ�ĵ��ֽ�
#define 	RegCRCPresetMSB 		0x24				// CRC�Ĵ���Ԥ��ֵ�ĸ��ֽ�
#define 	RegPreSet25 			0x25				// ��ֵ����ı�
#define 	RegMFOUTSelect 			0x26				// ѡ��������ܽ�MFOUT���ڲ��ź�
#define 	RegPreSet27 			0x27				// ��ֵ����ı�
// ҳ#5 FIFO����ʱ����IRQ����
//#define 	RegPage 				0x28
#define 	RegFIFOLevel 			0x29				// ����FIFO��������羯�����
#define 	RegTimerClock 			0x2A				// ѡ��ʱ��ʱ�ӵķ�Ƶ��
#define 	RegTimerControl 		0x2B				// ѡ��ʱ������ʼ��ֹͣ����
#define 	RegTimerReload 			0x2C				// ���嶨ʱ����Ԥװֵ
#define 	RegIRQPinConfig 		0x2D				// ����IRQ�ŵ����״̬
#define 	RegPreSet2E 			0x2E				// ��ֵ����ı�
#define 	RegPreSet2F 			0x2F				// ��ֵ����ı�
// ҳ#6 RFU
//#define 	RegPage 				0x30
//#define 	RegRFU 					0x31				// ��������֮��
//#define 	RegRFU 					0x32
//#define 	RegRFU 					0x33
//#define 	RegRFU 					0x34
//#define 	RegRFU 					0x35
//#define 	RegRFU 					0x36
//#define 	RegRFU 					0x37
// ҳ#7 ���ԺͿ���
//#define 	RegPage 				0x38
//#define 	RegRFU 					0x39
#define 	RegTestAnaSelect 		0x3A				// ѡ��ģ�����ģʽ
#define 	RegPreSet3B 			0x3B				// ��ֵ����ı�
#define 	RegPreSet3C 			0x3C				// ��ֵ����ı�
#define 	RegTestDigiSelect 		0x3D				// ѡ�����ֲ���ģʽ
//#define 	RegRFU 					0x3E
//#define 	RegRFU 					0x3F

#endif
