#ifndef __ISO14443A_H__
#define __ISO14443A_H__

//****************************************** MIFARE Command Sets ******************************************//
// ��ϸ�ο���MF RC500 Basic Function Library(Ӣ��API����)����2��
#define PICC_REQIDL			0x26		// Ѱ��������δ��������״̬�Ŀ�(����:None; ����:Tag Type)
#define PICC_REQALL			0x52		// Ѱ��������ȫ����(����: None; ����:Tag Type)
#define PICC_ANTICOLL1		0x93		// һ������ͻ
										// ����: optional parts of the card's serial number; 
										// ����:rest of) card'sserial number
#define PICC_ANTICOLL2		0x95		// ��������ͻ
#define PICC_ANTICOLL3		0x97		// ��������ͻ
#define PICC_SEL			0x93		// ѡ��(����: Card serial number; ����:Answer to select)
#define PICC_AUTHENT1A		0x60		// ��֤A��Կ(����: Block address; ����:Acknowledge)
#define PICC_AUTHENT1B		0x61		// ��֤B��Կ(����: Block address; ����:Acknowledge)
#define PICC_READ			0x30		// ����(����: Block address; ����:16-byte data block)
#define PICC_WRITE			0xA0		// д��(����: Block address and 16-byte data block; ����:Acknowledge)
#define PICC_DECREMENT		0xC0		// �ۿ�(����: Block address and 4-byte value; ����:Acknowledge)
#define PICC_INCREMENT		0xC1		// ��ֵ(����: Block address and 4-byte value; ����:Acknowledge)
#define PICC_RESTORE		0xC2		// ��Ǯ����������
										// ����: Block address and 4-byte dummy value; 
										// ����: Acknowledge
#define PICC_TRANSFER		0xB0		// ���滺����������(����: Block address; ����:Acknowledge)
#define PICC_HALT			0x50		// ����(����: Dummy address; ����:None)
//****************************************** MIFARE Command Sets ******************************************//

// Ѱ��
signed char PiccRequest(unsigned char req_code, unsigned char *pTagType);

// ��������ͻ
signed char PiccCascAnticollision(unsigned char *pUid, unsigned char *pLen);
// ����ͻѭ��
signed char PiccAnticollisionLoop(unsigned char *pSnr, unsigned char SelType);
// ѡ��
signed char PiccSelect(unsigned char *pSnr, unsigned char *pSAK, unsigned char SelType);

// 3 Pass Authentication
signed char PcdAuthState(unsigned char auth_mode, unsigned char blockNum, unsigned char *pSnr);

// ��д�����
signed char PiccRead(unsigned char addr, unsigned char *pReaddata);
signed char PiccWrite(unsigned char addr, unsigned char *pWritedata);

// ֵ����
signed char PiccValues(unsigned char dd_mode, unsigned char addr, unsigned char *pValue);

signed char PiccHalt(void);

#endif
