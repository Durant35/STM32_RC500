#include "SerialPort.h"
#include <process.h>
#include <iostream>

// 线程退出标志
bool CSerialPort::s_bExit = false;
// 接收完成事件
HANDLE CSerialPort::m_hRxEvent = CreateEvent(NULL, true, false, NULL);

// 当串口无数据时,sleep至下次查询间隔的时间,单位:毫秒
const UINT SLEEP_TIME_INTERVAL = 5;
// 保证一次接收完整数据
const UINT SLEEP_TIME_PREPARE = 500;

CSerialPort::CSerialPort(void)
	: m_hListenThread(INVALID_HANDLE_VALUE)
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hListenThread = INVALID_HANDLE_VALUE;
	InitializeCriticalSection(&m_csCommunicationSync);

	RxCounter = 0;
}

CSerialPort::~CSerialPort(void)
{
	CloseListenTread();
	ClosePort();
	DeleteCriticalSection(&m_csCommunicationSync);
}

bool CSerialPort::InitPort( UINT portNo /*= 1*/,UINT baud /*= CBR_115200*/,char parity /*= 'N'*/,
						    UINT databits /*= 8*/, UINT stopsbits /*= 1*/,DWORD dwCommEvents /*= EV_RXCHAR*/ )
{
	// 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护
	if (!OpenPort(portNo)){
		return false;
	}

	// 临时变量,将制定参数转化为字符串形式,以构造DCB结构
	char szDCBparam[50];
	sprintf(szDCBparam, "%d, %c, %d, %d", baud, parity, databits, stopsbits);

	// 进入临界段 
	EnterCriticalSection(&m_csCommunicationSync);

	// 是否有错误发生
	BOOL bIsSuccess = TRUE;

    /** 
	 *	在此可以设置输入输出的缓冲区大小,如果不设置,则系统会设置默认值.
	 *  自己设置缓冲区大小时,要注意设置稍大一些,避免缓冲区溢出
	 */
	/*
	if (bIsSuccess)
	{	
		bIsSuccess = SetupComm(m_hComm,10,10);
	}*/

	/** 设置串口的超时时间,均设为0,表示不使用超时限制 */
	COMMTIMEOUTS  CommTimeouts;
	CommTimeouts.ReadIntervalTimeout         = 0;		// 两个字符之间的超时设置
	CommTimeouts.ReadTotalTimeoutMultiplier  = 0;		// 读操作时总的超时系数
	CommTimeouts.ReadTotalTimeoutConstant    = 0;		// 读操作时总的超时常数
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;		// 写操作时总的超时系数
	CommTimeouts.WriteTotalTimeoutConstant   = 0;		// 写操作时总的超时常数
	if (bIsSuccess){
		bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
	}

	if (bIsSuccess){
		DCB dcb;

		// 获取当前串口配置参数,并且构造串口DCB参数
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(szDCBparam, &dcb);

		bIsSuccess = SetCommState(m_hComm, &dcb);
	}
		
	// 清空串口缓冲区
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	// 离开临界段
	LeaveCriticalSection(&m_csCommunicationSync);

	return (bIsSuccess==TRUE);
}

void CSerialPort::ClosePort()
{
	/** 如果有串口被打开，关闭它 */
	if( m_hComm != INVALID_HANDLE_VALUE ){
		CloseHandle( m_hComm );
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

bool CSerialPort::OpenPort( UINT portNo )
{
	/** 进入临界段 */ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** 把串口的编号转换为设备名 */ 
    char szPort[50];
	sprintf(szPort, "COM%d", portNo);

	/** 打开指定的串口 */ 
	m_hComm = CreateFileA(szPort,		                /** 设备名,COM1,COM2等 */ 
						 GENERIC_READ | GENERIC_WRITE,  /** 访问模式,可同时读写 */   
						 0,                             /** 共享模式,0表示不共享 */ 
					     NULL,							/** 安全性设置,一般使用NULL */ 
					     OPEN_EXISTING,					/** 该参数表示设备必须存在,否则创建失败 */ 
						 0,    
						 0);    

	/** 如果打开失败，释放资源并返回 */ 
	if (m_hComm == INVALID_HANDLE_VALUE){
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** 退出临界区 */ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

bool CSerialPort::StopPort(){
	/** 如果打开失败，释放资源并返回 */ 
	if (m_hComm == INVALID_HANDLE_VALUE){
		return false;
	}
	else{
		CloseListenTread();
		ClosePort();
		return true;
	}
}

bool CSerialPort::OpenListenThread()
{
	// 检测线程是否已经开启了
	if (m_hListenThread != INVALID_HANDLE_VALUE){
		// 线程已经开启 
		return false;
	}

	s_bExit = false;
	// 线程ID
	UINT threadId;
	// 开启串口数据监听线程
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread){
		return false;
	}
	// 设置线程的优先级,高于普通线程
	if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL)){
		return false;
	}

	return true;
}

bool CSerialPort::CloseListenTread()
{	
	if (m_hListenThread != INVALID_HANDLE_VALUE){
		// 通知线程退出
		s_bExit = true;

		// 等待线程退出 
		Sleep(10);

		// 置线程句柄无效 
		CloseHandle( m_hListenThread );
		m_hListenThread = INVALID_HANDLE_VALUE;
	}
	return true;
}

UINT CSerialPort::GetBytesInCOM()
{
	DWORD dwError = 0;						// 错误码
	COMSTAT  comstat;						// COMSTAT 结构体,记录通信设备的状态信息
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;
											// 在调用 ReadFile 和 WriteFile 之前，通过本函数清除以前遗留的错误标志 
	if ( ClearCommError(m_hComm, &dwError, &comstat) ){
		BytesInQue = comstat.cbInQue;		// 获取在输入缓冲区中的字节数
	}

	return BytesInQue;
}

UINT WINAPI CSerialPort::ListenThread( void* pParam )
{
	// 得到本类的指针 
	CSerialPort *pSerialPort = reinterpret_cast<CSerialPort*>(pParam);

	// 线程循环，轮询方式读取串口数据
	while (!pSerialPort->s_bExit) 
	{
		UINT BytesInQue = pSerialPort->GetBytesInCOM();

		// 如果串口输入缓冲区中无数据,则休息一会再查询
		if ( BytesInQue == 0 ){
			Sleep(SLEEP_TIME_INTERVAL);
			continue;
		}

		if (pSerialPort->ReadData(BytesInQue) == true){
			SetEvent(m_hRxEvent);
			Sleep(SLEEP_TIME_PREPARE);
		}
	}

	return 0;
}

bool CSerialPort::ReadData(UINT length){
	BOOL  bResult = TRUE;
	if (m_hComm == INVALID_HANDLE_VALUE){
		return false;
	}

	// 临界区保护
	EnterCriticalSection(&m_csCommunicationSync);

	// 从缓冲区读取数据
	bResult = ReadFile(m_hComm, RxBuf, length, &RxCounter, NULL);
	if (!bResult){
		// 获取错误码,可以根据该错误码查出错误原因
		DWORD dwError = GetLastError();

		// 清空串口缓冲区
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);

		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	// 离开临界区
	LeaveCriticalSection(&m_csCommunicationSync);

	return (RxCounter == length);
}

bool CSerialPort::WriteData( unsigned char* pData, unsigned int length )
{
	BOOL   bResult     = TRUE;
	DWORD  BytesToSend = 0;
	if(m_hComm == INVALID_HANDLE_VALUE){
		return false;
	}

	/** 临界区保护 */ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** 向缓冲区写入指定量的数据 */ 
	bResult = WriteFile(m_hComm, pData, length, &BytesToSend, NULL);
	if (!bResult)  
	{
		DWORD dwError = GetLastError();
		/** 清空串口缓冲区 */ 
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** 离开临界区 */ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}


bool CSerialPort::GetRxData(unsigned char *pData, unsigned char *pLength){
	if (m_hComm == INVALID_HANDLE_VALUE){
		return false;
	}

	unsigned long result = WaitForSingleObject(m_hRxEvent, 3200);
	switch(result){
	case WAIT_OBJECT_0:
		break;
	case WAIT_TIMEOUT:
		printf("WAIT_TIMEOUT\n");
		return false;
		break;
	default:
		return false;
		break;
	}
	//WaitForSingleObject(m_hRxEvent, 0xffffffffL);	//无限等待事件发生
	ResetEvent(m_hRxEvent);

	// 临界区保护
	EnterCriticalSection(&m_csCommunicationSync);
	*pLength = RxCounter;
	memcpy(pData, RxBuf, RxCounter);
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}


