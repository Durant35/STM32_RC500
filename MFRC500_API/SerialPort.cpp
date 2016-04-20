#include "SerialPort.h"
#include <process.h>
#include <iostream>

// �߳��˳���־
bool CSerialPort::s_bExit = false;
// ��������¼�
HANDLE CSerialPort::m_hRxEvent = CreateEvent(NULL, true, false, NULL);

// ������������ʱ,sleep���´β�ѯ�����ʱ��,��λ:����
const UINT SLEEP_TIME_INTERVAL = 5;
// ��֤һ�ν�����������
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
	// ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ���
	if (!OpenPort(portNo)){
		return false;
	}

	// ��ʱ����,���ƶ�����ת��Ϊ�ַ�����ʽ,�Թ���DCB�ṹ
	char szDCBparam[50];
	sprintf(szDCBparam, "%d, %c, %d, %d", baud, parity, databits, stopsbits);

	// �����ٽ�� 
	EnterCriticalSection(&m_csCommunicationSync);

	// �Ƿ��д�����
	BOOL bIsSuccess = TRUE;

    /** 
	 *	�ڴ˿���������������Ļ�������С,���������,��ϵͳ������Ĭ��ֵ.
	 *  �Լ����û�������Сʱ,Ҫע�������Դ�һЩ,���⻺�������
	 */
	/*
	if (bIsSuccess)
	{	
		bIsSuccess = SetupComm(m_hComm,10,10);
	}*/

	/** ���ô��ڵĳ�ʱʱ��,����Ϊ0,��ʾ��ʹ�ó�ʱ���� */
	COMMTIMEOUTS  CommTimeouts;
	CommTimeouts.ReadIntervalTimeout         = 0;		// �����ַ�֮��ĳ�ʱ����
	CommTimeouts.ReadTotalTimeoutMultiplier  = 0;		// ������ʱ�ܵĳ�ʱϵ��
	CommTimeouts.ReadTotalTimeoutConstant    = 0;		// ������ʱ�ܵĳ�ʱ����
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;		// д����ʱ�ܵĳ�ʱϵ��
	CommTimeouts.WriteTotalTimeoutConstant   = 0;		// д����ʱ�ܵĳ�ʱ����
	if (bIsSuccess){
		bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
	}

	if (bIsSuccess){
		DCB dcb;

		// ��ȡ��ǰ�������ò���,���ҹ��촮��DCB����
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(szDCBparam, &dcb);

		bIsSuccess = SetCommState(m_hComm, &dcb);
	}
		
	// ��մ��ڻ�����
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	// �뿪�ٽ��
	LeaveCriticalSection(&m_csCommunicationSync);

	return (bIsSuccess==TRUE);
}

void CSerialPort::ClosePort()
{
	/** ����д��ڱ��򿪣��ر��� */
	if( m_hComm != INVALID_HANDLE_VALUE ){
		CloseHandle( m_hComm );
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

bool CSerialPort::OpenPort( UINT portNo )
{
	/** �����ٽ�� */ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** �Ѵ��ڵı��ת��Ϊ�豸�� */ 
    char szPort[50];
	sprintf(szPort, "COM%d", portNo);

	/** ��ָ���Ĵ��� */ 
	m_hComm = CreateFileA(szPort,		                /** �豸��,COM1,COM2�� */ 
						 GENERIC_READ | GENERIC_WRITE,  /** ����ģʽ,��ͬʱ��д */   
						 0,                             /** ����ģʽ,0��ʾ������ */ 
					     NULL,							/** ��ȫ������,һ��ʹ��NULL */ 
					     OPEN_EXISTING,					/** �ò�����ʾ�豸�������,���򴴽�ʧ�� */ 
						 0,    
						 0);    

	/** �����ʧ�ܣ��ͷ���Դ������ */ 
	if (m_hComm == INVALID_HANDLE_VALUE){
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** �˳��ٽ��� */ 
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

bool CSerialPort::StopPort(){
	/** �����ʧ�ܣ��ͷ���Դ������ */ 
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
	// ����߳��Ƿ��Ѿ�������
	if (m_hListenThread != INVALID_HANDLE_VALUE){
		// �߳��Ѿ����� 
		return false;
	}

	s_bExit = false;
	// �߳�ID
	UINT threadId;
	// �����������ݼ����߳�
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread){
		return false;
	}
	// �����̵߳����ȼ�,������ͨ�߳�
	if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL)){
		return false;
	}

	return true;
}

bool CSerialPort::CloseListenTread()
{	
	if (m_hListenThread != INVALID_HANDLE_VALUE){
		// ֪ͨ�߳��˳�
		s_bExit = true;

		// �ȴ��߳��˳� 
		Sleep(10);

		// ���߳̾����Ч 
		CloseHandle( m_hListenThread );
		m_hListenThread = INVALID_HANDLE_VALUE;
	}
	return true;
}

UINT CSerialPort::GetBytesInCOM()
{
	DWORD dwError = 0;						// ������
	COMSTAT  comstat;						// COMSTAT �ṹ��,��¼ͨ���豸��״̬��Ϣ
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;
											// �ڵ��� ReadFile �� WriteFile ֮ǰ��ͨ�������������ǰ�����Ĵ����־ 
	if ( ClearCommError(m_hComm, &dwError, &comstat) ){
		BytesInQue = comstat.cbInQue;		// ��ȡ�����뻺�����е��ֽ���
	}

	return BytesInQue;
}

UINT WINAPI CSerialPort::ListenThread( void* pParam )
{
	// �õ������ָ�� 
	CSerialPort *pSerialPort = reinterpret_cast<CSerialPort*>(pParam);

	// �߳�ѭ������ѯ��ʽ��ȡ��������
	while (!pSerialPort->s_bExit) 
	{
		UINT BytesInQue = pSerialPort->GetBytesInCOM();

		// ����������뻺������������,����Ϣһ���ٲ�ѯ
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

	// �ٽ�������
	EnterCriticalSection(&m_csCommunicationSync);

	// �ӻ�������ȡ����
	bResult = ReadFile(m_hComm, RxBuf, length, &RxCounter, NULL);
	if (!bResult){
		// ��ȡ������,���Ը��ݸô�����������ԭ��
		DWORD dwError = GetLastError();

		// ��մ��ڻ�����
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);

		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	// �뿪�ٽ���
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

	/** �ٽ������� */ 
	EnterCriticalSection(&m_csCommunicationSync);

	/** �򻺳���д��ָ���������� */ 
	bResult = WriteFile(m_hComm, pData, length, &BytesToSend, NULL);
	if (!bResult)  
	{
		DWORD dwError = GetLastError();
		/** ��մ��ڻ����� */ 
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** �뿪�ٽ��� */ 
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
	//WaitForSingleObject(m_hRxEvent, 0xffffffffL);	//���޵ȴ��¼�����
	ResetEvent(m_hRxEvent);

	// �ٽ�������
	EnterCriticalSection(&m_csCommunicationSync);
	*pLength = RxCounter;
	memcpy(pData, RxBuf, RxCounter);
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}


