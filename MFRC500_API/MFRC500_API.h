#ifndef _MFRC500_API_H_
#define _MFRC500_API_H_

#ifndef RC500_API
#define RC500_API extern "C" _declspec(dllimport)
#endif

typedef enum{
	COM_OK = 0,							// 执行成功
	COM_ERR_PORT = -124,				// 串口操作异常
	COM_ERR_UID	= -123,					// 获取卡号出错
	COM_ERR_AUTHENT = -122,				// 扇区密钥验证失败
	COM_ERR_READ = -121,				// 读块数据失败
	COM_ERR_WRITE = -120,				// 写块数据失败

	COM_ERR_NO_START = -119,			// 接收帧无起始字节错误
	COM_ERR_NO_END = -118,				// 接收帧无结束字节错误

	COM_ERR_BCC = -117,					// 接收帧 BCC 校验字节不正确

	COM_ERR_RXNOMATCH = -116,			// 接收帧接收数据矛盾

	COM_ERR_DATATYPE = -115,			// 钱包金额 long 类型错误

	COM_ERR_WALLET = -114,				// 钱包金额异常

	COM_ERR_MONEY = -113,				// 非法操作金额
	COM_ERR_MONEY_OVERFLOW = -112,		// 充值操作将导致余额溢出
	COM_ERR_MONEY_LACK = -111,			// 余额不足

	COM_ERR_TIMEOUT = -110,				// 请求超时

	COM_NOTAGERR = -1,					// 没有卡片或寻不到卡
	COM_CRCERR = -2,					// 通信 CRC 校验失败
	COM_AUTHERR = -4,					// 通讯验证失败
	COM_PARITYERR = -5,					// 通讯奇偶校验失败
	COM_CODEERR = -6,					// 通讯编码异常
	COM_NOTAUTHERR = -10,				// 未通过3 pass 验证
	COM_BITCOUNTERR = -11,				// 返回数据位数不够
	COM_COLLERR = -24,					// 防冲突失败
	COM_WALLET_NOTINIT = -65,			// 钱包特殊数据块未初始化
	COM_ERR = -125						// MFRC 500 通讯超时
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
