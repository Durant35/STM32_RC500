#ifndef __ISO14443A_H__
#define __ISO14443A_H__

//****************************************** MIFARE Command Sets ******************************************//
// 详细参考《MF RC500 Basic Function Library(英文API简述)》第2章
#define PICC_REQIDL			0x26		// 寻天线区内未进入休眠状态的卡(参数:None; 返回:Tag Type)
#define PICC_REQALL			0x52		// 寻天线区内全部卡(参数: None; 返回:Tag Type)
#define PICC_ANTICOLL1		0x93		// 一级防冲突
										// 参数: optional parts of the card's serial number; 
										// 返回:rest of) card'sserial number
#define PICC_ANTICOLL2		0x95		// 二级防冲突
#define PICC_ANTICOLL3		0x97		// 三级防冲突
#define PICC_SEL			0x93		// 选卡(参数: Card serial number; 返回:Answer to select)
#define PICC_AUTHENT1A		0x60		// 验证A密钥(参数: Block address; 返回:Acknowledge)
#define PICC_AUTHENT1B		0x61		// 验证B密钥(参数: Block address; 返回:Acknowledge)
#define PICC_READ			0x30		// 读块(参数: Block address; 返回:16-byte data block)
#define PICC_WRITE			0xA0		// 写块(参数: Block address and 16-byte data block; 返回:Acknowledge)
#define PICC_DECREMENT		0xC0		// 扣款(参数: Block address and 4-byte value; 返回:Acknowledge)
#define PICC_INCREMENT		0xC1		// 充值(参数: Block address and 4-byte value; 返回:Acknowledge)
#define PICC_RESTORE		0xC2		// 调钱包到缓冲区
										// 参数: Block address and 4-byte dummy value; 
										// 返回: Acknowledge
#define PICC_TRANSFER		0xB0		// 保存缓冲区中数据(参数: Block address; 返回:Acknowledge)
#define PICC_HALT			0x50		// 休眠(参数: Dummy address; 返回:None)
//****************************************** MIFARE Command Sets ******************************************//

// 寻卡
signed char PiccRequest(unsigned char req_code, unsigned char *pTagType);

// 级联防冲突
signed char PiccCascAnticollision(unsigned char *pUid, unsigned char *pLen);
// 防冲突循环
signed char PiccAnticollisionLoop(unsigned char *pSnr, unsigned char SelType);
// 选卡
signed char PiccSelect(unsigned char *pSnr, unsigned char *pSAK, unsigned char SelType);

// 3 Pass Authentication
signed char PcdAuthState(unsigned char auth_mode, unsigned char blockNum, unsigned char *pSnr);

// 读写块操作
signed char PiccRead(unsigned char addr, unsigned char *pReaddata);
signed char PiccWrite(unsigned char addr, unsigned char *pWritedata);

// 值操作
signed char PiccValues(unsigned char dd_mode, unsigned char addr, unsigned char *pValue);

signed char PiccHalt(void);

#endif
