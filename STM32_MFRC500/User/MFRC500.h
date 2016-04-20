#ifndef __MFRC500_H__
#define __MFRC500_H__

/***********************************************************
 *
 *	Desription:	MF RC500 Register Sets
 * 		��ϸ�ο���MF RC500 Datasheet(������ϸ��)����5��
 *
 ***********************************************************/
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
//****************************************** MFRC500 Register Sets ******************************************//


/***********************************************************
 *
 *	Desription:	MF RC500 Command Sets
 * 		��ϸ�ο���MF RC500 Datasheet(������ϸ��)����16��
 *
 ***********************************************************/
#define PCD_IDLE			0x00		// �޶���:ȡ����ǰִ�е�����
#define PCD_WRITEE2			0x01		// ��FIFO������������ݲ�д���ڲ�E2PROM
#define PCD_READE2			0x03		// ���ڲ�E2PROM�������ݲ��������FIFO������(�ܳײ��ܱ�����)
#define PCD_LOADCONFIG		0x07		// ��E2PROM��ȡ���ݲ���ʼ��MF RC500�Ĵ���
#define PCD_LOADKEYE2		0x0B		// ��EEPROM�б������Կ���뻺��*/
#define PCD_AUTHENT1		0x0C		// ִ��Crypto1����֤��Կ��һ��*/
#define PCD_AUTHENT2		0x14		// ִ��Crypto1����֤��Կ�ڶ���*/
#define PCD_RECEIVE			0x16		// ��������(�ڽ�����ʵ������֮ǰ״̬�������Ĵ���RxWait���õ�ʱ���Ž����ȴ�)
#define PCD_LOADKEY			0x19		// ��FIFO������������Կ�ֽڲ����������Կ������(��Կ������ָ���ĸ�ʽ׼��)
#define PCD_TRANSMIT		0x1A		// �����ݴ�FIFO���������͵���
#define PCD_TRANSCEIVE		0x1E		// �����ݴ�FIFO���͵������ڷ��ͺ��Զ�����������
										// �ڽ�����ʵ������֮ǰ״̬�������Ĵ���RxWait���õ�ʱ���Ž����ȴ�
										// �������Ƿ��ͺͽ��յ����
#define PCD_STARTUP			0x3F		// ���и�λ�ͳ�ʼ���׶�(�������ͨ�����,ֻ��ͨ���ϵ��Ӳ����λ����)
#define PCD_CALCCRC			0x12		// CRC����(CRC�������ɴӼĴ���CRCResultLSB��CRCResultMSB�ж���)
//****************************************** MFRC500 Command Sets ******************************************//


#define FIFO_LENGTH       	64			// FIFO size = 64byte


// PCD/PICC ͨѶ�ṹ��
struct TranSciveBuffer{
	unsigned char MfCommand;		// MFRC500�����ֻ� M1��������
	unsigned int  MfLength;			// �������ݳ��Ȼ�������ݳ���
	unsigned char MfData[128];		// �������ݻ����������ʱ������
	unsigned int mfcurrent;			// ��ǰ�����±�
};


// ��λ����ʼ�� RC500
signed char RC500Init(void);
// ��λ RC500
signed char PcdReset(void);       

// ���� RC500 оƬ�ĳ�����
signed char PcdConfigISOType(unsigned char type);
// ��������
signed char PcdAntennaOn(void);
// �ر�����
signed char PcdAntennaOff(void);
// ���� RC500 ��ʱ��
void PcdSetTmo(unsigned char tmoLength);

// PCD �����������Ҫ�� PCD/PICC ͨѶ
signed char PcdCmdProcess(struct TranSciveBuffer *pi);

// ����ת����ʽ�����Կ�� FIFO �������͵� RC500 ���ܳ׻�������
signed char PcdAuthKey(unsigned char *pKeys);

signed char RC500_ReadE2(unsigned short startaddr, unsigned char length, unsigned char *readdata);

// ��� FIFO ������
void PcdFlushFIFO(void);

// RC500 �Ĵ�����λ
void SetBitMask(unsigned char reg,unsigned char mask);
// RC500 �Ĵ�����λ
void ClearBitMask(unsigned char reg,unsigned char mask);

#endif
