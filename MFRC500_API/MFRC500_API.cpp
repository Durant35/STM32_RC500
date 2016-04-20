#define RC500_API extern "C" _declspec(dllexport)

#include "MFRC500_API.h"
#include "SerialPort.h"


CSerialPort mySerialPort;

/** 计算 BCC 校验位
*
*  @param:  const unsigned char *data: 计算校验位数据数组
*			unsigned char len:		   数据长度
*  @return: unsigned char  校验位
*  @note:
*/
unsigned char BCC_Calculate(const unsigned char *data, unsigned char len){
	unsigned char bcc = 0;
	unsigned char i;
	for (i = 0; i < len; i++){
		bcc ^= data[i];
	}

	return bcc;
}

/** 打开串口
*
*  @param:  int portNum 串口编号,默认值为1,即COM1,注意,尽量不要大于9
*  @return: COM_STATE  初始化结果
*  @note:   
*/
signed char OpenPort(int portNum){
	bool success = true;

	success = mySerialPort.InitPort(portNum);
	if (success){
		success = mySerialPort.OpenListenThread();
	}
	
	if (success){
		return COM_OK;
	}
	else{
		return COM_ERR_PORT;
	}
}

/** 关闭串口
*
*  @param:  int portNum 串口编号,默认值为1,即COM1,注意,尽量不要大于9
*  @return: COM_STATE  初始化结果
*  @note:   
*/
signed char ClosePort(){
	bool success = true;

	success = mySerialPort.StopPort();
	
	if (success){
		return COM_OK;
	}
	else{
		return COM_ERR_PORT;
	}
}

/** 获取卡号 UID 函数
*
*  @param:  unsigned char *pUID: UID 返回值，可能4、7、10字节
*			unsigned char &len:	 UID 长度
*  @return: COM_STATE  操作状态
*  @note:
*/
signed char Card_GetUID(unsigned char *pUID, unsigned char &len){
	unsigned char TxData[64] = {0};
	unsigned int TxLength = 0;

	TxData[TxLength++] = BYTE_START;
	TxData[TxLength++] = UID_GET;
	TxData[TxLength++] = 0x00;
	TxData[TxLength++] = BCC_Calculate(TxData, TxLength);
	TxData[TxLength++] = BYTE_END;

	if(!mySerialPort.WriteData(TxData, TxLength)){
		return COM_ERR_PORT;
	}

	unsigned char RxData[32] = { 0 };
	unsigned char RxLength = 0;

	// Rx timeout
	if(!mySerialPort.GetRxData(RxData, &RxLength)){
		return COM_ERR_TIMEOUT;
	}

	// 假设一次接收完成
	if ((RxData[0] != BYTE_START)){
		return COM_ERR_NO_START;
	}
	if ((RxData[RxLength - 1] != BYTE_END)){
		return COM_ERR_NO_END;
	}

	unsigned char bcc_check = BCC_Calculate(RxData, RxLength - 2);
	if (bcc_check != RxData[RxLength - 2]){
		return COM_ERR_BCC;
	}

	if (RxData[1] != COM_OK){
		return RxData[1];
	}
	len = RxData[2];
	if (len != (RxLength - 5)){						// substract BYTE_START、BYTE_STATE、BYTE_LEN、BYTE_BCC、BYTE_END
													// 4 byte for UID(or most 7、10)
		return COM_ERR_RXNOMATCH;
	}
	memcpy(pUID, &RxData[3], len);

	return COM_OK;
}

/** 扇区密钥验证函数
*
*  @param:  unsigned char sectorNum:	扇区号(0-15)
*			unsigned char keyType:		密钥类型(0x0A Type-A; 0x0B Type-B)
*			const unsigned char *pKey:	密钥(6 字节)
*  @return: signed char 验证结果
*  @note:
*/
signed char Card_Authentication(unsigned char sectorNum, unsigned char keyType, const unsigned char *pKey){
	unsigned char i;
	unsigned char TxData[64] = { 0 };
	unsigned int TxLength = 0;

	TxData[TxLength++] = BYTE_START;
	TxData[TxLength++] = CARD_AUTHENT;

	TxData[TxLength++] = 0x08;													// 8 bytes for 3 parameters
	TxData[TxLength++] = sectorNum;
	TxData[TxLength++] = keyType;
	for (i = 0; i < 6; i++){
		TxData[TxLength++] = pKey[i];
	}

	TxData[TxLength++] = BCC_Calculate(TxData, TxLength);
	TxData[TxLength++] = BYTE_END;

	mySerialPort.WriteData(TxData, TxLength);

	unsigned char RxData[32] = { 0 };
	unsigned char RxLength = 0;

	// Rx timeout
	if(!mySerialPort.GetRxData(RxData, &RxLength)){
		return COM_ERR_TIMEOUT;
	}

	// 假设一次接收完成
	if ((RxData[0] != BYTE_START)){
		return COM_ERR_NO_START;
	}
	if ((RxData[RxLength - 1] != BYTE_END)){
		return COM_ERR_NO_END;
	}

	unsigned char bcc_check = BCC_Calculate(RxData, RxLength - 2);
	if (bcc_check != RxData[RxLength - 2]){
		return COM_ERR_BCC;
	}

	if (RxData[1] != COM_OK){
		return RxData[1];
	}
	unsigned char len = RxData[2];
	if (len != (RxLength - 5)){						// substract BYTE_START、BYTE_STATE、BYTE_LEN、BYTE_BCC、BYTE_END
													// only 1 byte for CARD_AUTHENT
		return COM_ERR_RXNOMATCH;
	}

	if (RxData[3] == keyType){
		return COM_OK;
	}
	else{
		return COM_ERR_AUTHENT;
	}
}

/** 读块数据函数
*
*  @param:  unsigned char sectorNum:	扇区号(0-15)
*			unsigned char blockNum:		块号(0-3)
*			unsigned char keyType:		密钥类型(0x0A Type-A; 0x0B Type-B)
*			const unsigned char *pKey:	密钥(6 字节)
*			unsigned char *prdata:		读取的块数据(16 字节)
*  @return: 操作状态，是否成功
*  @note:
*/
signed char Card_Read(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, unsigned char *prdata){
	unsigned char i;
	unsigned char TxData[64] = { 0 };
	unsigned int TxLength = 0;

	TxData[TxLength++] = BYTE_START;
	TxData[TxLength++] = BLOCK_READ;

	TxData[TxLength++] = 0x09;													// 9 bytes for 4 parameters
	TxData[TxLength++] = sectorNum;
	TxData[TxLength++] = blockNum;
	TxData[TxLength++] = keyType;
	for (i = 0; i < 6; i++){
		TxData[TxLength++] = pKey[i];
	}

	TxData[TxLength++] = BCC_Calculate(TxData, TxLength);
	TxData[TxLength++] = BYTE_END;

	mySerialPort.WriteData(TxData, TxLength);

	unsigned char RxData[32] = { 0 };
	unsigned char RxLength = 0;

	// Rx timeout
	if(!mySerialPort.GetRxData(RxData, &RxLength)){
		return COM_ERR_TIMEOUT;
	}

	// 假设一次接收完成
	if ((RxData[0] != BYTE_START)){
		return COM_ERR_NO_START;
	}
	if ((RxData[RxLength - 1] != BYTE_END)){
		return COM_ERR_NO_END;
	}

	unsigned char bcc_check = BCC_Calculate(RxData, RxLength - 2);
	if (bcc_check != RxData[RxLength - 2]){
		return COM_ERR_BCC;
	}

	if (RxData[1] != COM_OK){
		return RxData[1];
	}

	unsigned char len = RxData[2];
	if (len != (RxLength - 5)){						// substract BYTE_START、BYTE_STATE、BYTE_LEN、BYTE_BCC、BYTE_END
													// 16 bytes for a block data
		return COM_ERR_RXNOMATCH;
	}

	memcpy(prdata, &RxData[3], 16);

	return COM_OK;
}

/** 写块数据函数
*
*  @param:  unsigned char sectorNum:	扇区号(0-15)
*			unsigned char blockNum:		块号(0-3)
*			unsigned char keyType:		密钥类型(0x0A Type-A; 0x0B Type-B)
*			const unsigned char *pKey:	密钥(6 字节)
*			unsigned char *pwdata:		写入的块数据(16 字节)
*  @return: 操作状态，是否成功
*  @note:
*/
signed char Card_Write(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, const unsigned char *pwdata){
	unsigned char i;
	unsigned char TxData[64] = { 0 };
	unsigned int TxLength = 0;

	TxData[TxLength++] = BYTE_START;
	TxData[TxLength++] = BLOCK_WRITE;

	TxData[TxLength++] = 0x19;													// 25 bytes for 5 parameters
	TxData[TxLength++] = sectorNum;
	TxData[TxLength++] = blockNum;
	TxData[TxLength++] = keyType;
	for (i = 0; i < 6; i++){
		TxData[TxLength++] = pKey[i];
	}
	for (i = 0; i < 16; i++){
		TxData[TxLength++] = pwdata[i];
	}

	TxData[TxLength++] = BCC_Calculate(TxData, TxLength);
	TxData[TxLength++] = BYTE_END;

	mySerialPort.WriteData(TxData, TxLength);

	unsigned char RxData[32] = { 0 };
	unsigned char RxLength = 0;

	// Rx timeout
	if(!mySerialPort.GetRxData(RxData, &RxLength)){
		return COM_ERR_TIMEOUT;
	}

	// 假设一次接收完成
	if ((RxData[0] != BYTE_START)){
		return COM_ERR_NO_START;
	}
	if ((RxData[RxLength - 1] != BYTE_END)){
		return COM_ERR_NO_END;
	}

	unsigned char bcc_check = BCC_Calculate(RxData, RxLength - 2);
	if (bcc_check != RxData[RxLength - 2]){
		return COM_ERR_BCC;
	}

	if (RxData[1] != COM_OK){
		return RxData[1];
	}

	unsigned char len = RxData[2];
	if (len != (RxLength - 5)){						// substract BYTE_START、BYTE_STATE、BYTE_LEN、BYTE_BCC、BYTE_END
													// 0 for BLOCK_WRITE
		return COM_ERR_RXNOMATCH;
	}

	return COM_OK;
}

/** 电子钱包初始化
*
*  @param:  unsigned char sectorNum:	扇区号(0-15)
*			unsigned char blockNum:		块号(0-3)
*			unsigned char keyType:		密钥类型(0x0A Type-A; 0x0B Type-B)
*			const unsigned char *pKey:	密钥(6 字节)
*			unsigned long initVal:		初始化金额(满足 4 个字节)
*  @return: 操作状态，是否成功
*  @note:
*/
signed char Card_WalletInit(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, signed long initVal){
	if (initVal <= 0){
		return COM_ERR_MONEY;
	}

	unsigned char money[4] = { 0 };
	if (!long2bytes(initVal, money)){
		return COM_ERR_DATATYPE;
	}

	unsigned char i;
	unsigned char TxData[64] = { 0 };
	unsigned int TxLength = 0;

	TxData[TxLength++] = BYTE_START;
	TxData[TxLength++] = WALLET_INIT;

	TxData[TxLength++] = 0x0D;													// 13 bytes for 5 parameters
	TxData[TxLength++] = sectorNum;
	TxData[TxLength++] = blockNum;
	TxData[TxLength++] = keyType;
	for (i = 0; i < 6; i++){
		TxData[TxLength++] = pKey[i];
	}
	for (i = 0; i < 4; i++){
		TxData[TxLength++] = money[i];
	}

	TxData[TxLength++] = BCC_Calculate(TxData, TxLength);
	TxData[TxLength++] = BYTE_END;

	mySerialPort.WriteData(TxData, TxLength);

	unsigned char RxData[32] = { 0 };
	unsigned char RxLength = 0;

	// Rx timeout
	if(!mySerialPort.GetRxData(RxData, &RxLength)){
		return COM_ERR_TIMEOUT;
	}

	// 假设一次接收完成
	if ((RxData[0] != BYTE_START)){
		return COM_ERR_NO_START;
	}
	if ((RxData[RxLength - 1] != BYTE_END)){
		return COM_ERR_NO_END;
	}

	unsigned char bcc_check = BCC_Calculate(RxData, RxLength - 2);
	if (bcc_check != RxData[RxLength - 2]){
		return COM_ERR_BCC;
	}

	if (RxData[1] != COM_OK){
		//printf("%d\n", (signed char)RxData[1]);
		//printf("%02X\n", RxData[1]);
		return RxData[1];
	}

	unsigned char len = RxData[2];
	if (len != (RxLength - 5)){						// substract BYTE_START、BYTE_STATE、BYTE_LEN、BYTE_BCC、BYTE_END
													// 4 bytes for money
		return COM_ERR_RXNOMATCH;
	}

	for (i = 0; i < 4; i++){
		if (RxData[3 + i] != money[i]){
			break;
		}
	}
	if (i == 4){
		return COM_OK;
	}
	else{
		return COM_ERR_WALLET;
	}

}

/** 电子钱包余额查询
*
*  @param:  unsigned char sectorNum:	扇区号(0-15)
*			unsigned char blockNum:		块号(0-3)
*			unsigned char keyType:		密钥类型(0x0A Type-A; 0x0B Type-B)
*			const unsigned char *pKey:	密钥(6 字节)
*			unsigned long &remVal:		钱包金额(满足 4 个字节)
*  @return: 操作状态，是否成功
*  @note:
*/
signed char Card_WalletCheck(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, signed long &remVal){
	unsigned char money[4] = { 0 };

	unsigned char i;
	unsigned char TxData[64] = { 0 };
	unsigned int TxLength = 0;

	TxData[TxLength++] = BYTE_START;
	TxData[TxLength++] = WALLET_CHECK;

	TxData[TxLength++] = 0x09;													// 9 bytes for 4 parameters
	TxData[TxLength++] = sectorNum;
	TxData[TxLength++] = blockNum;
	TxData[TxLength++] = keyType;
	for (i = 0; i < 6; i++){
		TxData[TxLength++] = pKey[i];
	}

	TxData[TxLength++] = BCC_Calculate(TxData, TxLength);
	TxData[TxLength++] = BYTE_END;

	mySerialPort.WriteData(TxData, TxLength);

	unsigned char RxData[32] = { 0 };
	unsigned char RxLength = 0;

	// Rx timeout
	if(!mySerialPort.GetRxData(RxData, &RxLength)){
		return COM_ERR_TIMEOUT;
	}

	// 假设一次接收完成
	if ((RxData[0] != BYTE_START)){
		return COM_ERR_NO_START;
	}
	if ((RxData[RxLength - 1] != BYTE_END)){
		return COM_ERR_NO_END;
	}

	unsigned char bcc_check = BCC_Calculate(RxData, RxLength - 2);
	if (bcc_check != RxData[RxLength - 2]){
		return COM_ERR_BCC;
	}

	if (RxData[1] != COM_OK){
		return RxData[1];
	}

	unsigned char len = RxData[2];
	if (len != (RxLength - 5)){						// substract BYTE_START、BYTE_STATE、BYTE_LEN、BYTE_BCC、BYTE_END
													// 4 bytes for money
		return COM_ERR_RXNOMATCH;
	}

	if (bytes2long(remVal, &RxData[3])){
		return COM_OK;
	}
	else{
		return COM_ERR_DATATYPE;
	}
}

/** 电子钱包充值操作
*
*  @param:  unsigned char sectorNum:	扇区号(0-15)
*			unsigned char blockNum:		块号(0-3)
*			unsigned char keyType:		密钥类型(0x0A Type-A; 0x0B Type-B)
*			const unsigned char *pKey:	密钥(6 字节)
*			unsigned long &value:		充值金额/充值后余额(满足 4 个字节)
*  @return: 操作状态，是否成功
*  @note:
*/
signed char Card_WalletRecharge(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, signed long &value){
	if (value <= 0){
		return COM_ERR_MONEY;
	}

	unsigned char money[4] = { 0 };
	if (!long2bytes(value, money)){
		return COM_ERR_DATATYPE;
	}

	signed long remainning = 0;
	signed char status = Card_WalletCheck(sectorNum, blockNum, keyType, pKey, remainning);
	if (status != COM_OK){
		//printf("status = %d\n", status);
		return status;
	}
	signed long tmp = remainning + value;
	if (tmp <= 0){
		return COM_ERR_MONEY_OVERFLOW;
	}

	unsigned char i;
	unsigned char TxData[64] = { 0 };
	unsigned int TxLength = 0;

	TxData[TxLength++] = BYTE_START;
	TxData[TxLength++] = WALLET_RECHARGE;

	TxData[TxLength++] = 0x0D;													// 13 bytes for 4 parameters
	TxData[TxLength++] = sectorNum;
	TxData[TxLength++] = blockNum;
	TxData[TxLength++] = keyType;
	for (i = 0; i < 6; i++){
		TxData[TxLength++] = pKey[i];
	}
	for (i = 0; i < 4; i++){
		TxData[TxLength++] = money[i];
	}

	TxData[TxLength++] = BCC_Calculate(TxData, TxLength);
	TxData[TxLength++] = BYTE_END;

	mySerialPort.WriteData(TxData, TxLength);

	unsigned char RxData[32] = { 0 };
	unsigned char RxLength = 0;

	// Rx timeout
	if(!mySerialPort.GetRxData(RxData, &RxLength)){
		return COM_ERR_TIMEOUT;
	}

	// 假设一次接收完成
	if ((RxData[0] != BYTE_START)){
		return COM_ERR_NO_START;
	}
	if ((RxData[RxLength - 1] != BYTE_END)){
		return COM_ERR_NO_END;
	}

	unsigned char bcc_check = BCC_Calculate(RxData, RxLength - 2);
	if (bcc_check != RxData[RxLength - 2]){
		return COM_ERR_BCC;
	}

	if (RxData[1] != COM_OK){
		//printf("%d\n", (signed char)RxData[1]);
		//printf("%02X\n", RxData[1]);
		return RxData[1];
	}

	unsigned char len = RxData[2];
	if (len != (RxLength - 5)){						// substract BYTE_START、BYTE_STATE、BYTE_LEN、BYTE_BCC、BYTE_END
													// 4 bytes for remaining
		return COM_ERR_RXNOMATCH;
	}

	if (bytes2long(value, &RxData[3])){
		return COM_OK;
	}
	else{
		return COM_ERR_DATATYPE;
	}
	return COM_OK;
}

/** 电子钱包扣款操作
*
*  @param:  unsigned char sectorNum:	扇区号(0-15)
*			unsigned char blockNum:		块号(0-3)
*			unsigned char keyType:		密钥类型(0x0A Type-A; 0x0B Type-B)
*			const unsigned char *pKey:	密钥(6 字节)
*			unsigned long &value:		消费金额/消费后余额(满足 4 个字节)
*  @return: 操作状态，是否成功
*  @note:
*/
signed char Card_WalletDebit(unsigned char sectorNum, unsigned char blockNum, unsigned char keyType,
	const unsigned char *pKey, signed long &value){
	if (value <= 0){
		return COM_ERR_MONEY;
	}

	unsigned char money[4] = { 0 };
	if (!long2bytes(value, money)){
		return COM_ERR_DATATYPE;
	}

	signed long remainning = 0;
	signed char status = Card_WalletCheck(sectorNum, blockNum, keyType, pKey, remainning);
	if (status != COM_OK){
		return status;
	}
	if (remainning < value){
		return COM_ERR_MONEY_LACK;
	}

	unsigned char i;
	unsigned char TxData[64] = { 0 };
	unsigned int TxLength = 0;

	TxData[TxLength++] = BYTE_START;
	TxData[TxLength++] = WALLET_DEBIT;

	TxData[TxLength++] = 0x0D;													// 13 bytes for 4 parameters
	TxData[TxLength++] = sectorNum;
	TxData[TxLength++] = blockNum;
	TxData[TxLength++] = keyType;
	for (i = 0; i < 6; i++){
		TxData[TxLength++] = pKey[i];
	}
	for (i = 0; i < 4; i++){
		TxData[TxLength++] = money[i];
	}

	TxData[TxLength++] = BCC_Calculate(TxData, TxLength);
	TxData[TxLength++] = BYTE_END;

	mySerialPort.WriteData(TxData, TxLength);

	unsigned char RxData[32] = { 0 };
	unsigned char RxLength = 0;

	// Rx timeout
	if(!mySerialPort.GetRxData(RxData, &RxLength)){
		return COM_ERR_TIMEOUT;
	}

	// 假设一次接收完成
	if ((RxData[0] != BYTE_START)){
		return COM_ERR_NO_START;
	}
	if ((RxData[RxLength - 1] != BYTE_END)){
		return COM_ERR_NO_END;
	}

	unsigned char bcc_check = BCC_Calculate(RxData, RxLength - 2);
	if (bcc_check != RxData[RxLength - 2]){
		return COM_ERR_BCC;
	}

	if (RxData[1] != COM_OK){
		return RxData[1];
	}

	unsigned char len = RxData[2];
	if (len != (RxLength - 5)){						// substract BYTE_START、BYTE_STATE、BYTE_LEN、BYTE_BCC、BYTE_END
													// 4 bytes for remaining
		return COM_ERR_RXNOMATCH;
	}

	if (bytes2long(value, &RxData[3])){
		return COM_OK;
	}
	else{
		return COM_ERR_DATATYPE;
	}
}

/** 打印错误信息
*
*  @param:  signed char errCode:	操作错误码
*  @return: char*:					错误信息字符串，建议长度为char[40]
*  @note:
*/
char* PrintErrInfos(signed char errCode){
	if(errCode == COM_OK){
		return "no error.";
	}

	char* infos;
	switch(errCode){
	case COM_ERR_PORT:
		infos = "port error, maybe open or close.";
		break;
	case COM_ERR_UID:
		infos = "get card uid error.";
		break;
	case COM_ERR_AUTHENT:
		infos = "sector key authentication error.";
		break;
	case COM_ERR_READ:
		infos = "read block data error.";
		break;
	case COM_ERR_WRITE:
		infos = "write block data error.";
		break;
	case COM_ERR_NO_START:
		infos = "no start byte from STM32.";
		break;
	case COM_ERR_NO_END:
		infos = "no end byte from STM32.";
		break;
	case COM_ERR_BCC:
		infos = "bcc error for data from STM32.";
		break;
	case COM_ERR_RXNOMATCH:
		infos = "data from STM32 not integrity.";
		break;
	case COM_ERR_DATATYPE:
		infos = "wallet money long-type error.";
		break;
	case COM_ERR_WALLET:
		infos = "wallet operations error.";
		break;
	case COM_ERR_MONEY:
		infos = "wallet operated money less than or equal 0.";
		break;
	case COM_ERR_MONEY_OVERFLOW:
		infos = "wallet recharge will cause overflow.";
		break;
	case COM_ERR_MONEY_LACK:
		infos = "no enough money for debitting.";
		break;
	case COM_WALLET_NOTINIT:
		infos = "block for wallet not init.";
		break;
	case COM_ERR_TIMEOUT:
		infos = "port receive time out.";
		break;
	case COM_NOTAGERR:
		infos = "no tag or cannot select.";
		break;
	case COM_CRCERR:
		infos = "crc error for rc500.";
		break;
	case COM_AUTHERR:
		infos = "authentication error for rc500.";
		break;
	case COM_PARITYERR:
		infos = "parity error for rc500.";
		break;
	case COM_CODEERR:
		infos = "coder error for rc500.";
		break;
	case COM_NOTAUTHERR:
		infos = "3 pass authen error for rc500.";
		break;
	case COM_BITCOUNTERR:
		infos = "wrong data return from tag.";
		break;
	case COM_COLLERR:
		infos = "anti-collision loop error.";
		break;
	case COM_ERR:
		infos = "over time for rc500.";
		break;
	}

	return infos;
}

/** 带符号 4 字节 long 值 ==> 字节数据(低位字节在前)
*
*  @param:  signed long lVal:		带符号 4 字节值
*			unsigned char *bytes:	4 字节数据(低位字节在前)
*  @return: sizeof(signed long)	是否为 4 字节 
*  @note:
*/
bool long2bytes(signed long lVal, unsigned char *bytes){
	if (sizeof(signed long) != 4){
		return false;
	}
	unsigned char i;
	for (i = 0; i < 4; i++){
		bytes[i] = (lVal >> (8 * i)) & 0xFF;
	}
	return true;
}

/** 4 字节数据(低位字节在前) ==> 带符号 4 字节值
*
*  @param:  signed long &lVal:		带符号 4 字节值
*			unsigned char *bytes:	4 字节数据(低位字节在前)
*  @return: sizeof(signed long)是否为 4 字节
*  @note:
*/
bool bytes2long(signed long &lVal, const unsigned char *bytes){
	if (sizeof(signed long) != 4){
		return false;
	}

	unsigned char i;
	unsigned long tmp;

	lVal = 0x0;
	for (i = 0; i < 4; i++){
		tmp = bytes[i];
		lVal |= tmp << (8 * i);
	}
	return true;
}

