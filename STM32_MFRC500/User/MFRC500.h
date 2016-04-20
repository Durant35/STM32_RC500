#ifndef __MFRC500_H__
#define __MFRC500_H__

/***********************************************************
 *
 *	Desription:	MF RC500 Register Sets
 * 		详细参考《MF RC500 Datasheet(中文详细版)》第5章
 *
 ***********************************************************/
// 页#0 命令和状态
#define 	RegPage 				0x00				// 选择寄存器页
#define 	RegCommand 				0x01				// 启动和停止命令的执行
#define 	RegFIFOData 			0x02				// 64字节FIFO缓冲区输入和输出
#define 	RegPrimaryStatus 		0x03				// 接收器和发送器以及FIFO缓冲区状态标志
#define 	RegFIFOLength 			0x04				// FIFO中缓冲的字节数
#define 	RegSecondaryStatus		0x05				// 不同的状态标志
#define 	RegInterruptEn 			0x06				// 使能和禁止中断请求通过的控制位
#define 	RegInterruptRq 			0x07				// 中断请求标志
// 页#1 控制和状态
//#define 	RegPage 				0x08
#define 	RegControl 				0x09				// 不同的控制标志,例如:定时器、节电
#define 	RegErrorFlag 			0x0A				// 显示上次命令执行错误状态的错误标志
#define 	RegCollPos 				0x0B				// RF接口检测到的第一个冲突位的位置
#define 	RegTimerValue 			0x0C				// 定时器的实际值
#define 	RegCRCResultLSB 		0x0D				// CRC协处理器寄存器的最低位
#define 	RegCRCResultMSB 		0x0E				// CRC协处理器寄存器的最高位
#define 	RegBitFraming 			0x0F				// 位方式帧的调节
// 页#2 发送器和编码器控制
//#define 	RegPage 				0x10				// 
#define 	RegTxControl 			0x11				// 天线驱动脚TX1和TX2的逻辑状态控制
#define 	RegCWConductance 		0x12				// 选择天线驱动脚TX1和TX2的电导率
#define 	RegPreSet13 			0x13				// 该值不会改变
#define 	RegPreSet14 			0x14				// 该值不会改变
#define 	RegModWidth 			0x15				// 选择调整脉冲的宽度
#define 	RegPreSet16 			0x16				// 该值不会改变
#define 	RegPreSet17 			0x17				// 该值不会改变
// 页#3 接收器和解码控制
//#define 	RegPage 				0x18
#define 	RegRxControl1 			0x19				// 控制接收器状态
#define 	RegDecoderControl 		0x1A				// 控制解码器状态
#define 	RegBitPhase 			0x1B				// 选择发送器和接收器时钟之间的位相位
#define 	RegRxThreshold 			0x1C				// 选择位解码器的阀值
#define 	RegPreSet1D 			0x1D				// 该值不会改变
#define 	RegRxControl2 			0x1E				// 控制解码器状态和定义接收器的输入源
#define 	RegClockQControl 		0x1F				// 控制时钟产生用于90°相移的Q信道时钟
// 页#4 时序和信道冗余
//#define 	RegPage 				0x20
#define 	RegRxWait 				0x21				// 选择发送后接收器启动前的时间间隔
#define 	RegChannelRedundancy 	0x22				// 选择RF信道上数据完整性检测的类型和模式
#define 	RegCRCPresetLSB 		0x23				// CRC寄存器预设值的低字节
#define 	RegCRCPresetMSB 		0x24				// CRC寄存器预设值的高字节
#define 	RegPreSet25 			0x25				// 该值不会改变
#define 	RegMFOUTSelect 			0x26				// 选择输出到管脚MFOUT的内部信号
#define 	RegPreSet27 			0x27				// 该值不会改变
// 页#5 FIFO、定时器和IRQ脚配
//#define 	RegPage 				0x28
#define 	RegFIFOLevel 			0x29				// 定义FIFO上溢和下溢警告界限
#define 	RegTimerClock 			0x2A				// 选择定时器时钟的分频器
#define 	RegTimerControl 		0x2B				// 选择定时器的起始和停止条件
#define 	RegTimerReload 			0x2C				// 定义定时器的预装值
#define 	RegIRQPinConfig 		0x2D				// 配置IRQ脚的输出状态
#define 	RegPreSet2E 			0x2E				// 该值不会改变
#define 	RegPreSet2F 			0x2F				// 该值不会改变
// 页#6 RFU
//#define 	RegPage 				0x30
//#define 	RegRFU 					0x31				// 保留将来之用
//#define 	RegRFU 					0x32
//#define 	RegRFU 					0x33
//#define 	RegRFU 					0x34
//#define 	RegRFU 					0x35
//#define 	RegRFU 					0x36
//#define 	RegRFU 					0x37
// 页#7 测试和控制
//#define 	RegPage 				0x38
//#define 	RegRFU 					0x39
#define 	RegTestAnaSelect 		0x3A				// 选择模拟测试模式
#define 	RegPreSet3B 			0x3B				// 该值不会改变
#define 	RegPreSet3C 			0x3C				// 该值不会改变
#define 	RegTestDigiSelect 		0x3D				// 选择数字测试模式
//#define 	RegRFU 					0x3E
//#define 	RegRFU 					0x3F
//****************************************** MFRC500 Register Sets ******************************************//


/***********************************************************
 *
 *	Desription:	MF RC500 Command Sets
 * 		详细参考《MF RC500 Datasheet(中文详细版)》第16章
 *
 ***********************************************************/
#define PCD_IDLE			0x00		// 无动作:取消当前执行的命令
#define PCD_WRITEE2			0x01		// 从FIFO缓冲区获得数据并写入内部E2PROM
#define PCD_READE2			0x03		// 从内部E2PROM读出数据并将其放入FIFO缓冲区(密匙不能被读出)
#define PCD_LOADCONFIG		0x07		// 从E2PROM读取数据并初始化MF RC500寄存器
#define PCD_LOADKEYE2		0x0B		// 将EEPROM中保存的密钥调入缓存*/
#define PCD_AUTHENT1		0x0C		// 执行Crypto1卡验证密钥第一步*/
#define PCD_AUTHENT2		0x14		// 执行Crypto1卡验证密钥第二步*/
#define PCD_RECEIVE			0x16		// 接收数据(在接收器实际启动之前状态机经过寄存器RxWait配置的时间后才结束等待)
#define PCD_LOADKEY			0x19		// 从FIFO缓冲区读出密钥字节并将其放入密钥缓冲区(密钥必须以指定的格式准备)
#define PCD_TRANSMIT		0x1A		// 将数据从FIFO缓冲区发送到卡
#define PCD_TRANSCEIVE		0x1E		// 将数据从FIFO发送到卡并在发送后自动启动接收器
										// 在接收器实际启动之前状态机经过寄存器RxWait配置的时间后才结束等待
										// 该命令是发送和接收的组合
#define PCD_STARTUP			0x3F		// 运行复位和初始化阶段(该命令不能通过软件,只能通过上电或硬件复位启动)
#define PCD_CALCCRC			0x12		// CRC计算(CRC计算结果可从寄存器CRCResultLSB和CRCResultMSB中读出)
//****************************************** MFRC500 Command Sets ******************************************//


#define FIFO_LENGTH       	64			// FIFO size = 64byte


// PCD/PICC 通讯结构体
struct TranSciveBuffer{
	unsigned char MfCommand;		// MFRC500命令字或 M1卡命令字
	unsigned int  MfLength;			// 发送数据长度或接收数据长度
	unsigned char MfData[128];		// 发送数据或接收数据临时缓冲区
	unsigned int mfcurrent;			// 当前数据下标
};


// 复位并初始化 RC500
signed char RC500Init(void);
// 复位 RC500
signed char PcdReset(void);       

// 配置 RC500 芯片的场调制
signed char PcdConfigISOType(unsigned char type);
// 开启天线
signed char PcdAntennaOn(void);
// 关闭天线
signed char PcdAntennaOff(void);
// 设置 RC500 定时器
void PcdSetTmo(unsigned char tmoLength);

// PCD 命令处理函数，主要是 PCD/PICC 通讯
signed char PcdCmdProcess(struct TranSciveBuffer *pi);

// 将已转换格式后的密钥经 FIFO 缓冲区送到 RC500 的密匙缓冲区中
signed char PcdAuthKey(unsigned char *pKeys);

signed char RC500_ReadE2(unsigned short startaddr, unsigned char length, unsigned char *readdata);

// 清空 FIFO 缓冲区
void PcdFlushFIFO(void);

// RC500 寄存器置位
void SetBitMask(unsigned char reg,unsigned char mask);
// RC500 寄存器清位
void ClearBitMask(unsigned char reg,unsigned char mask);

#endif
