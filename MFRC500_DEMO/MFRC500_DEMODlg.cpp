// MFRC500_DEMODlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFRC500_DEMO.h"
#include "MFRC500_DEMODlg.h"
#include "./libs/MFRC500_API.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFRC500_DEMODlg dialog

CMFRC500_DEMODlg::CMFRC500_DEMODlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMFRC500_DEMODlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMFRC500_DEMODlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// m_font����Ϊȫ�ֱ������߳�Ա��������ָ��
	// ���� ������������֮������ɾ��֮��
	m_font.CreateFont(16, 0, 0, 0,
		FW_HEAVY,
		FALSE, FALSE,
		FALSE, GB2312_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH|FF_MODERN, _T("���Ŀ���"));

	keyType = 0x0A;

	isPortOpened = false;
	isUIDGetSuccess = false;
	isWalletOpeSuccess = false;
}

void CMFRC500_DEMODlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFRC500_DEMODlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFRC500_DEMODlg, CDialog)
	//{{AFX_MSG_MAP(CMFRC500_DEMODlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_PORT, OnBtnOpenPort)
	ON_BN_CLICKED(IDC_BUTTON2, OnBtnGetUID)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON1, OnBtnDefaultKey)
	ON_BN_CLICKED(IDC_RADIOA, OnRadioA)
	ON_BN_CLICKED(IDC_RADIOB, OnRadioB)
	ON_BN_CLICKED(IDC_BUTTON3, OnBtnAuthen)
	ON_BN_CLICKED(IDC_BUTTON6, OnBtnReadSector)
	ON_BN_CLICKED(IDC_BUTTON4, OnBtnReadBlock)
	ON_BN_CLICKED(IDC_BUTTON5, OnBtnWriteBlock)
	ON_BN_CLICKED(IDC_BUTTON7, OnBtnWalletInit)
	ON_BN_CLICKED(IDC_BUTTON8, OnBtnWalletCheck)
	ON_BN_CLICKED(IDC_BUTTON9, OnBtnWalletRecharge)
	ON_BN_CLICKED(IDC_BUTTON10, OnBtnWalletDebit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFRC500_DEMODlg message handlers

BOOL CMFRC500_DEMODlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	// Ĭ�ϴ�������
	InitPortSettings();
	// Ĭ�������� ���
	InitSectorBlock();
	// Ĭ����Կ����
	((CButton*)GetDlgItem(IDC_RADIOA))->SetCheck(true);
	// Ĭ����Կ
	((CEdit*)GetDlgItem(IDC_KEY))->SetWindowText("FFFFFFFFFFFF");

	((CStatic*)GetDlgItem(IDC_INFOS))->SetFont(&m_font);
		
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFRC500_DEMODlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFRC500_DEMODlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFRC500_DEMODlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CMFRC500_DEMODlg::InitPortSettings(){
	// COM3
	((CComboBox*)GetDlgItem(IDC_PORTNO))->SetCurSel(2);
	// 115200
	((CComboBox*)GetDlgItem(IDC_BAUDRATE))->SetCurSel(1);
	// 8 databits
	((CComboBox*)GetDlgItem(IDC_DATABITS))->SetCurSel(3);
	// 1 stopbit
	((CComboBox*)GetDlgItem(IDC_STOPBITS))->SetCurSel(0);
	// no parity
	((CComboBox*)GetDlgItem(IDC_PARITY))->SetCurSel(0);
}

void CMFRC500_DEMODlg::InitSectorBlock(){
	((CComboBox*)GetDlgItem(IDC_SECTOR_KEY))->SetCurSel(10);
	
	((CComboBox*)GetDlgItem(IDC_SECTOR_RW))->SetCurSel(10);
	((CComboBox*)GetDlgItem(IDC_BLOCK_RW))->SetCurSel(0);

	((CComboBox*)GetDlgItem(IDC_SECTOR_WALLET))->SetCurSel(10);
	((CComboBox*)GetDlgItem(IDC_BLOCK_WALLET))->SetCurSel(0);
}

void CMFRC500_DEMODlg::OnBtnOpenPort() 
{
	if(isPortOpened){
		if(!ClosePort()){
			isPortOpened = false;
			((CButton*)GetDlgItem(IDC_BTN_PORT))->SetWindowText("�򿪴���");
			PortSettingEnable(true);
			MessageBox("���ڹرճɹ�");
		}
		else{
			MessageBox("���ڹر�ʧ��");
		}
		return;
	}

	if(!OpenPort(3)){
		//((CEdit*)GetDlgItem(IDC_STATUS))->SetWindowText("�ɹ�");
		isPortOpened = true;
		((CButton*)GetDlgItem(IDC_BTN_PORT))->SetWindowText("�رմ���");
		PortSettingEnable(false);
		MessageBox("���ڴ򿪳ɹ�");
	}
	else{
		MessageBox("���ڴ�ʧ��");
	}
}

void CMFRC500_DEMODlg::PortSettingEnable(bool enable){
	((CComboBox*)GetDlgItem(IDC_PORTNO))->EnableWindow(enable);
	((CComboBox*)GetDlgItem(IDC_BAUDRATE))->EnableWindow(enable);
	((CComboBox*)GetDlgItem(IDC_DATABITS))->EnableWindow(enable);
	((CComboBox*)GetDlgItem(IDC_STOPBITS))->EnableWindow(enable);
	((CComboBox*)GetDlgItem(IDC_PARITY))->EnableWindow(enable);
}

void CMFRC500_DEMODlg::OnBtnGetUID() 
{
	((CEdit*)GetDlgItem(IDC_UID))->SetWindowText("");

	unsigned char uid[10];
	unsigned char len;

	signed char status = Card_GetUID(uid, len);

	if(!status){
		isUIDGetSuccess = true;
		CString tmp, UID;
		UID.Empty();
		for(unsigned char i=0; i<len; i++){
			tmp.Format("%02X", uid[i]);
			UID += tmp;
		}
		((CEdit*)GetDlgItem(IDC_UID))->SetWindowText(UID);
		((CEdit*)GetDlgItem(IDC_UID_STATUS))->SetWindowText("��ȡ���ųɹ�");
	}
	else{
		isUIDGetSuccess = false;

		CString errInfos = PrintErrInfos(status);
		((CEdit*)GetDlgItem(IDC_UID_STATUS))->SetWindowText(errInfos);
	}
}


void CMFRC500_DEMODlg::OnBtnDefaultKey() 
{
	((CEdit*)GetDlgItem(IDC_KEY))->SetWindowText("FFFFFFFFFFFF");
}

HBRUSH CMFRC500_DEMODlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	// ���� info ��Ϣ��ɫ
	if(pWnd->GetDlgCtrlID() == IDC_INFOS){
        pDC->SetTextColor(RGB(128, 0, 255));	
    }

	// ���� IDC_UID_STATUS ��ɫ
	if(pWnd->GetDlgCtrlID() == IDC_UID_STATUS){
		if(isUIDGetSuccess){
			pDC->SetTextColor(RGB(0, 0, 255));
		}
		else{
			pDC->SetTextColor(RGB(255, 0, 0));
		}	
    }

	// ���� IDC_WALLET_STATUS ��ɫ
	if(pWnd->GetDlgCtrlID() == IDC_WALLET_STATUS){
		if(isWalletOpeSuccess){
			pDC->SetTextColor(RGB(0, 0, 255));
		}
		else{
			pDC->SetTextColor(RGB(255, 0, 0));
		}	
    }
	// TODO: Return a different brush if the default is not desired
	return hbr;
}


void CMFRC500_DEMODlg::OnRadioA() 
{
	keyType = 0x0A;	
}

void CMFRC500_DEMODlg::OnRadioB() 
{
	keyType = 0x0B;	
}

// 16 �������ַ���ת��Ϊ�ֽ�����
void CMFRC500_DEMODlg::HexStr2CharArray(CString hexStr, unsigned char *asc, unsigned char *asc_len)
{
	*asc_len = 0;
	unsigned char len = hexStr.GetLength();

	char* temp=(char *)malloc(len+1);		// +1 ==> '\0'

	char tmp[3]={0};
	char *Hex;
	unsigned char *p;
	
	strncpy(temp, (LPCTSTR)hexStr, len+1);
	Hex = temp;
	p = asc;
	
	while(*Hex != '\0')
	{
		tmp[0] = *Hex;
		Hex++;
		tmp[1] = *Hex;
		Hex++;
		tmp[2] = '\0';
		*p = (unsigned char) strtol(tmp, NULL, 16);
		p++;
		(*asc_len)++;
	}
   free(temp);
}

void CMFRC500_DEMODlg::OnBtnAuthen() 
{
	// ��ȡ�������Կ
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);

	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_KEY))->GetCurSel();
	
	signed char status = Card_Authentication(sectorNum, keyType, psw);
	if(!status){
		MessageBox("��Կ��֤�ɹ�");
	}
	else{
		CString info = PrintErrInfos(status);
		MessageBox("��Կ��֤ʧ��: " + info);
	}
}

void CMFRC500_DEMODlg::OnBtnReadSector() 
{
	// ��ȡ�������Կ
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("��������Կ��");
		return;
	}

	ClearRWEditors();			// �����һ�ζ�д������

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);
	
	unsigned char des_data[16];

	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_RW))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("��ѡ��������");
		return;
	}

	CString str;
	for(unsigned char block=0; block<4; block++){
		signed char status = Card_Read(sectorNum, block, keyType, psw, des_data);
		if(!status) {
			str.Empty();

			if(block == 3){
				for (int k=0; k<16; k++) {
					CString s1;
					s1.Format("%02X", des_data[k]);
					str += s1;
					if(k == 5){
						((CEdit*)GetDlgItem(IDC_BLOCK3A))->SetWindowText(str);
						str.Empty();
					}
					else if(k == 9){
						((CEdit*)GetDlgItem(IDC_BLOCK3B))->SetWindowText(str);
						str.Empty();
					}
				}
				((CEdit*)GetDlgItem(IDC_BLOCK3C))->SetWindowText(str);
				continue;
			}
				
			for (int k=0; k<16; k++) {
				CString s1;
				s1.Format("%02X",des_data[k]);
				str += s1;
			}
			switch(block){
				case 0:
					((CEdit*)GetDlgItem(IDC_BLOCK0))->SetWindowText(str);
					break;
				case 1:
					((CEdit*)GetDlgItem(IDC_BLOCK1))->SetWindowText(str);
					break;
				case 2:
					((CEdit*)GetDlgItem(IDC_BLOCK2))->SetWindowText(str);
					break;
				default:
					break;
			}
		}
		else {
			CString tmp;
			CString info = PrintErrInfos(status);
			if(block > 0){
				tmp.Format("%d", block);
				MessageBox("��ȡ��" + tmp + "��Ϣʧ��: " + info);
			}
			else{
				tmp.Format("%d", sectorNum);
				MessageBox("��ȡ����" + tmp + "��Ϣʧ��: " + info);
			}

			return;			//	ֻҪ��һ�����ȡʧ�ܣ�����������ȡ��ֹ
		}
	}
}

void CMFRC500_DEMODlg::OnBtnReadBlock() 
{
	// ��ȡ�������Կ
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("��������Կ��");
		return;
	}

	ClearRWEditors();			// �����һ�ζ�д������

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);

	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_RW))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("��ѡ��������");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_RW))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("��ѡ��飡");
		return;
	}

	unsigned char des_data[16];	
	CString str;
	signed char status = Card_Read(sectorNum, blockNum, keyType, psw, des_data);
	if(!status) {
		str.Empty();
		if(blockNum == 3){
			for (int k=0; k<16; k++) {
				CString s1;
				s1.Format("%02X", des_data[k]);
				str += s1;
				if(k == 5){
					((CEdit*)GetDlgItem(IDC_BLOCK3A))->SetWindowText(str);
					str.Empty();
				}
				else if(k == 9){
					((CEdit*)GetDlgItem(IDC_BLOCK3B))->SetWindowText(str);
					str.Empty();
				}
			}
			((CEdit*)GetDlgItem(IDC_BLOCK3C))->SetWindowText(str);
		}
		else{
			for (int k=0; k<16; k++) {
				CString s1;
				s1.Format("%02X",des_data[k]);
				str += s1;
			}
			switch(blockNum){
				case 0:
					((CEdit*)GetDlgItem(IDC_BLOCK0))->SetWindowText(str);
					break;
				case 1:
					((CEdit*)GetDlgItem(IDC_BLOCK1))->SetWindowText(str);
					break;
				case 2:
					((CEdit*)GetDlgItem(IDC_BLOCK2))->SetWindowText(str);
					break;
				default:
					break;
			}
		}
	}
	else {
		CString tmp;
		CString info = PrintErrInfos(status);
		tmp.Format("%d", blockNum);
		MessageBox("��ȡ��" + tmp + "��Ϣʧ��: " + info);
	}
}

void CMFRC500_DEMODlg::OnBtnWriteBlock() 
{
	// ��ȡ�������Կ
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("��������Կ��");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_RW))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("��ѡ��������");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_RW))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("��ѡ��飡");
		return;
	}

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);
	
	//	����д�������
	CString src_data0, src_data1, src_data2;
	switch(blockNum){
		case 0:
			((CEdit*)GetDlgItem(IDC_BLOCK0))->GetWindowText(src_data0);
			break;
		case 1:
			((CEdit*)GetDlgItem(IDC_BLOCK1))->GetWindowText(src_data0);
			break;
		case 2:
			((CEdit*)GetDlgItem(IDC_BLOCK2))->GetWindowText(src_data0);
			break;
		case 3:
			((CEdit*)GetDlgItem(IDC_BLOCK3A))->GetWindowText(src_data0);
			((CEdit*)GetDlgItem(IDC_BLOCK3B))->GetWindowText(src_data1);
			((CEdit*)GetDlgItem(IDC_BLOCK3C))->GetWindowText(src_data2);
			break;
		default:
			break;
	}

	//	�û�δ�������ݸ��辯��
	if(src_data0.IsEmpty()){
		if(3 != blockNum){
			MessageBox("������д������ݣ�");
			return;
		}
		else{
			if(src_data1.IsEmpty() && src_data2.IsEmpty()){
				MessageBox("������д������ݣ�");
				return;
			}
		}
	}

	unsigned char des_data[16];
	// ����ǰ�����Ŀ�3����ȡÿ���ֶ�����
	signed char status = Card_Read(sectorNum, blockNum, keyType, psw, des_data);
	if(COM_OK != status){
		CString tmp;
		CString info = PrintErrInfos(status);
		tmp.Format("%d", blockNum);
		MessageBox("д��" + tmp + "ʧ��: " + info);
		return;
	}

	CString msrc_data0, msrc_data1, msrc_data2;
	CString s1;
	for (int k=0; k<16; k++) {
		s1.Format("%02X",des_data[k]);
		if(blockNum != 3){
			msrc_data0	+= s1;
		}
		else if(k<6){ 
			msrc_data0	+= s1;
		}
		else if(k<10){
			msrc_data1	+= s1;
		}
		else{
			msrc_data2	+= s1;
		}
	}

	// ����д��3�Ĳ�����Ҫ���ֶο���
	if(blockNum == 3){
		MessageBox("��3Ϊ��Կ�洢�飬��ȷ��Ҫ��������޸ģ�");

		int i;
		for(i=0; i<src_data0.GetLength(); i++){
			if(i == 12) 
				break;									// ��ֹ��������
			msrc_data0.SetAt(i, src_data0[i]);
		}
		for(i=0; i<src_data1.GetLength(); i++){
			if(i == 8) 
				break;
			msrc_data1.SetAt(i, src_data1[i]);
		}
		for(i=0; i<src_data2.GetLength(); i++){
			if(i == 12) 
				break;
			msrc_data2.SetAt(i, src_data2[i]);
		}

		CString dataStr = msrc_data0 + msrc_data1 + msrc_data2;

		//	 �������������д��ʧ�ܵ����
		if(dataStr.GetLength()%2 != 0){
			dataStr.Insert(dataStr.GetLength(), "0");
		}
			
		unsigned char src_data[16];
		unsigned char src_len;
		HexStr2CharArray(dataStr, src_data, &src_len);

		status = Card_Write(sectorNum, blockNum, keyType, psw, src_data);
		if(!status){
			CString sectorStr;
			CString blockStr;
			sectorStr.Format("%d", sectorNum);
			blockStr.Format("%d", blockNum);
			MessageBox("����: " + sectorStr + " ��: " + blockStr + " ����д��ɹ�");
		}
		else{
			CString sectorStr;
			CString blockStr;
			sectorStr.Format("%d", sectorNum);
			blockStr.Format("%d", blockNum);
			CString info = PrintErrInfos(status);
			MessageBox("����: " + sectorStr + " ��: " + blockStr + " ����д��ʧ��: " + info);
		}
	}
	else {
		for(int i=0; i<src_data0.GetLength(); i++){
			if(i == 32) break;
			msrc_data0.SetAt(i, src_data0[i]);
		}

		if(msrc_data0.GetLength()%2 != 0){
			msrc_data0.Insert(src_data0.GetLength(), "0");
		}

		unsigned char src_data[16];
		unsigned char src_len;
		HexStr2CharArray(msrc_data0, src_data, &src_len);

		status = Card_Write(sectorNum, blockNum, keyType, psw, src_data);
		if(!status){
			CString sectorStr;
			CString blockStr;
			sectorStr.Format("%d", sectorNum);
			blockStr.Format("%d", blockNum);
			MessageBox("����: " + sectorStr + " ��: " + blockStr + " ����д��ɹ�");
		}
		else{
			CString sectorStr;
			CString blockStr;
			sectorStr.Format("%d", sectorNum);
			blockStr.Format("%d", blockNum);
			CString info = PrintErrInfos(status);
			MessageBox("����: " + sectorStr + " ��: " + blockStr + " ����д��ʧ��: " + info);
		}

	}
}

// ******************************* ��տ����� *********************************
void CMFRC500_DEMODlg::ClearRWEditors(){
	((CEdit*)GetDlgItem(IDC_BLOCK0))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK1))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK2))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK3A))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK3B))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK3C))->SetWindowText("");
}
// ******************************* ��տ����� *********************************

void CMFRC500_DEMODlg::OnBtnWalletInit() 
{
	// ��ȡ�������Կ
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("��������Կ��");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_WALLET))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("��ѡ��������");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_WALLET))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("��ѡ��飡");
		return;
	}

	CString remainings;
	((CEdit*)GetDlgItem(IDC_REMAIN))->GetWindowText(remainings);
	if(remainings.IsEmpty()){
		isWalletOpeSuccess = false;
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("�������ʼ����");
		return;
	}
	long account = atol(remainings);

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);

	signed char status = Card_WalletInit(sectorNum, blockNum, keyType, psw, account);
	if(!status){
		isWalletOpeSuccess = true;
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("Ǯ����ʼ���ɹ�");
	}
	else{
		isWalletOpeSuccess = false;
		CString info = PrintErrInfos(status);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText(info);
	}
}

void CMFRC500_DEMODlg::OnBtnWalletCheck() 
{
	// ��ȡ�������Կ
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("��������Կ��");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_WALLET))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("��ѡ��������");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_WALLET))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("��ѡ��飡");
		return;
	}
	((CEdit*)GetDlgItem(IDC_REMAIN))->SetWindowText("");

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);

	long account;
	signed char status = Card_WalletCheck(sectorNum, blockNum, keyType, psw, account);
	if(!status){
		isWalletOpeSuccess = true;
		CString tmp;
		tmp.Format("%ld", account);
		((CEdit*)GetDlgItem(IDC_REMAIN))->SetWindowText(tmp);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("��ѯ�ɹ�");
	}
	else{
		isWalletOpeSuccess = false;
		CString info = PrintErrInfos(status);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText(info);
	}
}

void CMFRC500_DEMODlg::OnBtnWalletRecharge() 
{	
	// ��ȡ�������Կ
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("��������Կ��");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_WALLET))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("��ѡ��������");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_WALLET))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("��ѡ��飡");
		return;
	}
	((CEdit*)GetDlgItem(IDC_REMAIN))->SetWindowText("");

	CString accountStr;
	((CEdit*)GetDlgItem(IDC_RECHARGE))->GetWindowText(accountStr);
	if(accountStr.IsEmpty()){
		isWalletOpeSuccess = false;
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("�������ֵ��");
		return;
	}
	long account = atol(accountStr);

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);

	signed char status = Card_WalletRecharge(sectorNum, blockNum, keyType, psw, account);
	if(!status){
		isWalletOpeSuccess = true;
		((CEdit*)GetDlgItem(IDC_RECHARGE))->SetWindowText("");
		CString tmp;
		tmp.Format("%ld", account);
		((CEdit*)GetDlgItem(IDC_REMAIN))->SetWindowText(tmp);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("��ֵ�ɹ�");
	}
	else{
		isWalletOpeSuccess = false;
		CString info = PrintErrInfos(status);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText(info);
	}
}

void CMFRC500_DEMODlg::OnBtnWalletDebit() 
{
	// ��ȡ�������Կ
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("��������Կ��");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_WALLET))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("��ѡ��������");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_WALLET))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("��ѡ��飡");
		return;
	}
	((CEdit*)GetDlgItem(IDC_REMAIN))->SetWindowText("");

	CString accountStr;
	((CEdit*)GetDlgItem(IDC_DEBIT))->GetWindowText(accountStr);
	if(accountStr.IsEmpty()){
		isWalletOpeSuccess = false;
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("���������ѽ�");
		return;
	}
	long account = atol(accountStr);

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);

	signed char status = Card_WalletDebit(sectorNum, blockNum, keyType, psw, account);
	if(!status){
		isWalletOpeSuccess = true;
		((CEdit*)GetDlgItem(IDC_DEBIT))->SetWindowText("");
		CString tmp;
		tmp.Format("%ld", account);
		((CEdit*)GetDlgItem(IDC_REMAIN))->SetWindowText(tmp);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("�ۿ�ɹ�");
	}
	else{
		isWalletOpeSuccess = false;
		CString info = PrintErrInfos(status);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText(info);
	}
}
