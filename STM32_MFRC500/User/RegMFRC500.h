#ifndef __REG_MF_RC500_H__
#define __REG_MF_RC500_H__

/***********************************************************
 *
 *	FileName: 	RegMFRC500.h
 *	Desription:	MFRC500 寄存器集合
 * 		详细参考《MFRC500 Datasheet(中文详细版)》第5章
 *
 ***********************************************************/
 
//***************** MFRC500 Register Sets *****************//
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

#endif
