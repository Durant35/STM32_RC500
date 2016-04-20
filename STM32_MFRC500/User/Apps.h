#ifndef __APPS_H__
#define __APPS_H__

#define UID_GET 		(0x01)
#define CARD_AUTHENT 	(0x03)

#define BLOCK_READ 		(0x05)
#define BLOCK_WRITE 	(0x07)

#define WALLET_INIT 	(0x09)
#define WALLET_CHECK 	(0x0B)
#define WALLET_RECHARGE (0x0D)
#define WALLET_DEBIT 	(0x0F)


signed char ComM1RequestA(unsigned char* TypeA_Uid, unsigned char* len);

signed char ComM1Authentication(unsigned char SectorNum, unsigned char KeyAB, unsigned char *pKeys);

signed char ComM1BlockRead(unsigned char SectorNum, unsigned char BlockNum,
				unsigned char KeyAB, unsigned char *pKeys, unsigned char *pReadData);

signed char ComM1BlockWrite(unsigned char SectorNum, unsigned char BlockNum, 
				unsigned char KeyAB, unsigned char *pKeys, unsigned char *pWriteData);

signed char ComM1WalletInit(unsigned char SectorNum, unsigned char BlockNum, 
				unsigned char KeyAB, unsigned char *pKeys, unsigned char *pInitVal);

signed char ComM1WalletCheck(unsigned char SectorNum, unsigned char BlockNum, 
				unsigned char KeyAB, unsigned char *pKeys, unsigned char *pCheckVal);

signed char ComM1WalletRecharge(unsigned char SectorNum, unsigned char BlockNum, 
				unsigned char KeyAB, unsigned char *pKeys, unsigned char *pIncreVal);

signed char ComM1WalletDebit(unsigned char SectorNum, unsigned char BlockNum, 
				unsigned char KeyAB, unsigned char *pKeys, unsigned char *pDecreVal);

#endif
