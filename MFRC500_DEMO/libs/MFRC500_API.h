#ifndef _MFRC500_API_H_
#define _MFRC500_API_H_

#ifndef RC500_API
#define RC500_API extern "C" _declspec(dllimport)
#endif

typedef enum{
	COM_OK = 0,							// ִ�гɹ�
	COM_ERR_PORT = -124,				// ���ڲ����쳣
	COM_ERR_UID	= -123,					// ��ȡ���ų���
	COM_ERR_AUTHENT = -122,				// ������Կ��֤ʧ��
	COM_ERR_READ = -121,				// ��������ʧ��
	COM_ERR_WRITE = -120,				// д������ʧ��

	COM_ERR_NO_START = -119,			// ����֡����ʼ�ֽڴ���
	COM_ERR_NO_END = -118,				// ����֡�޽����ֽڴ���

	COM_ERR_BCC = -117,					// ����֡ BCC У���ֽڲ���ȷ

	COM_ERR_RXNOMATCH = -116,			// ����֡��������ì��

	COM_ERR_DATATYPE = -115,			// Ǯ����� long ���ʹ���

	COM_ERR_WALLET = -114,				// Ǯ������쳣

	COM_ERR_MONEY = -113,				// �Ƿ��������
	COM_ERR_MONEY_OVERFLOW = -112,		// ��ֵ����������������
	COM_ERR_MONEY_LACK = -111,			// ����

	COM_ERR_TIMEOUT = -110,				// ����ʱ

	COM_NOTAGERR = -1,					// û�п�Ƭ��Ѱ������
	COM_CRCERR = -2,					// ͨ�� CRC У��ʧ��
	COM_AUTHERR = -4,					// ͨѶ��֤ʧ��
	COM_PARITYERR = -5,					// ͨѶ��żУ��ʧ��
	COM_CODEERR = -6,					// ͨѶ�����쳣
	COM_NOTAUTHERR = -10,				// δͨ��3 pass ��֤
	COM_BITCOUNTERR = -11,				// ��������λ������
	COM_COLLERR = -24,					// ����ͻʧ��
	COM_ERR = -125						// MFRC 500 ͨѶ��ʱ
} COM_STATE;

#define MAX_MONEY		(0x7FFFFFFF)

#define BYTE_START		(0xBC)
#define BYTE_END		(0xFE)

#define UID_GET 		(0x01)
#define CARD_AUTHENT 	(0x03)

#define BLOCK_READ 		(0x05)
#define BLOCK_WRITE 	(0x07)

#define WALLET_INIT 	(0x09)
#define WALLET_CHECK 	(0x0B)
#define WALLET_RECHARGE (0x0D)
#define WALLET_DEBIT 	(0x0F)

unsigned char BCC_Calculate(const unsigned char *data, unsigned char len);
RC500_API signed char OpenPort(int portNum);
RC500_API signed char ClosePort(void);

RC500_API signed char Card_GetUID(unsigned char *pUID, unsigned char &len);
RC500_API signed char Card_Authentication(unsigned char sectorNum, unsigned char keyType, const unsigned char *pKey);

RC500_API signed char Card_Read(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
			const unsigned char *pKey, unsigned char *prdata);
RC500_API signed char Card_Write(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
			const unsigned char *pKey, const unsigned char *pwdata);

RC500_API signed char Card_WalletInit(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, signed long initVal);
RC500_API signed char Card_WalletCheck(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, signed long &remVal);
RC500_API signed char Card_WalletRecharge(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, signed long &value);
RC500_API signed char Card_WalletDebit(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, signed long &value);

RC500_API char* PrintErrInfos(signed char errCode);

bool long2bytes(signed long lVal, unsigned char *bytes);
bool bytes2long(signed long &lVal, const unsigned char *bytes);

#endif
