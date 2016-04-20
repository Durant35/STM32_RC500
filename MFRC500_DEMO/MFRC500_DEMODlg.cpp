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

	// m_font必须为全局变量或者成员变量或者指针
	// 避免 “创建完字体之后立马删除之”
	m_font.CreateFont(16, 0, 0, 0,
		FW_HEAVY,
		FALSE, FALSE,
		FALSE, GB2312_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FIXED_PITCH|FF_MODERN, _T("华文楷体"));

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
	// 默认串口设置
	InitPortSettings();
	// 默认扇区号 块号
	InitSectorBlock();
	// 默认密钥类型
	((CButton*)GetDlgItem(IDC_RADIOA))->SetCheck(true);
	// 默认密钥
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
			((CButton*)GetDlgItem(IDC_BTN_PORT))->SetWindowText("打开串口");
			PortSettingEnable(true);
			MessageBox("串口关闭成功");
		}
		else{
			MessageBox("串口关闭失败");
		}
		return;
	}

	if(!OpenPort(3)){
		//((CEdit*)GetDlgItem(IDC_STATUS))->SetWindowText("成功");
		isPortOpened = true;
		((CButton*)GetDlgItem(IDC_BTN_PORT))->SetWindowText("关闭串口");
		PortSettingEnable(false);
		MessageBox("串口打开成功");
	}
	else{
		MessageBox("串口打开失败");
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
		((CEdit*)GetDlgItem(IDC_UID_STATUS))->SetWindowText("获取卡号成功");
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
	// 设置 info 信息颜色
	if(pWnd->GetDlgCtrlID() == IDC_INFOS){
        pDC->SetTextColor(RGB(128, 0, 255));	
    }

	// 设置 IDC_UID_STATUS 颜色
	if(pWnd->GetDlgCtrlID() == IDC_UID_STATUS){
		if(isUIDGetSuccess){
			pDC->SetTextColor(RGB(0, 0, 255));
		}
		else{
			pDC->SetTextColor(RGB(255, 0, 0));
		}	
    }

	// 设置 IDC_WALLET_STATUS 颜色
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

// 16 进制数字符串转化为字节数据
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
	// 获取输入的密钥
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);

	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_KEY))->GetCurSel();
	
	signed char status = Card_Authentication(sectorNum, keyType, psw);
	if(!status){
		MessageBox("密钥验证成功");
	}
	else{
		CString info = PrintErrInfos(status);
		MessageBox("密钥验证失败: " + info);
	}
}

void CMFRC500_DEMODlg::OnBtnReadSector() 
{
	// 获取输入的密钥
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("请输入密钥！");
		return;
	}

	ClearRWEditors();			// 清空上一次读写块数据

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);
	
	unsigned char des_data[16];

	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_RW))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("请选择扇区！");
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
				MessageBox("读取块" + tmp + "信息失败: " + info);
			}
			else{
				tmp.Format("%d", sectorNum);
				MessageBox("读取扇区" + tmp + "信息失败: " + info);
			}

			return;			//	只要有一个块读取失败，整个扇区读取终止
		}
	}
}

void CMFRC500_DEMODlg::OnBtnReadBlock() 
{
	// 获取输入的密钥
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("请输入密钥！");
		return;
	}

	ClearRWEditors();			// 清空上一次读写块数据

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);

	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_RW))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("请选择扇区！");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_RW))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("请选择块！");
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
		MessageBox("读取块" + tmp + "信息失败: " + info);
	}
}

void CMFRC500_DEMODlg::OnBtnWriteBlock() 
{
	// 获取输入的密钥
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("请输入密钥！");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_RW))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("请选择扇区！");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_RW))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("请选择块！");
		return;
	}

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);
	
	//	处理写入块数据
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

	//	用户未输入数据给予警告
	if(src_data0.IsEmpty()){
		if(3 != blockNum){
			MessageBox("请输入写入块数据！");
			return;
		}
		else{
			if(src_data1.IsEmpty() && src_data2.IsEmpty()){
				MessageBox("请输入写入块数据！");
				return;
			}
		}
	}

	unsigned char des_data[16];
	// 读当前扇区的块3并获取每个字段数据
	signed char status = Card_Read(sectorNum, blockNum, keyType, psw, des_data);
	if(COM_OK != status){
		CString tmp;
		CString info = PrintErrInfos(status);
		tmp.Format("%d", blockNum);
		MessageBox("写块" + tmp + "失败: " + info);
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

	// 对于写块3的操作需要分字段考虑
	if(blockNum == 3){
		MessageBox("块3为密钥存储块，您确定要对其进行修改？");

		int i;
		for(i=0; i<src_data0.GetLength(); i++){
			if(i == 12) 
				break;									// 防止过长数据
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

		//	 解决奇数个数据写入失败的情况
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
			MessageBox("扇区: " + sectorStr + " 块: " + blockStr + " 数据写入成功");
		}
		else{
			CString sectorStr;
			CString blockStr;
			sectorStr.Format("%d", sectorNum);
			blockStr.Format("%d", blockNum);
			CString info = PrintErrInfos(status);
			MessageBox("扇区: " + sectorStr + " 块: " + blockStr + " 数据写入失败: " + info);
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
			MessageBox("扇区: " + sectorStr + " 块: " + blockStr + " 数据写入成功");
		}
		else{
			CString sectorStr;
			CString blockStr;
			sectorStr.Format("%d", sectorNum);
			blockStr.Format("%d", blockNum);
			CString info = PrintErrInfos(status);
			MessageBox("扇区: " + sectorStr + " 块: " + blockStr + " 数据写入失败: " + info);
		}

	}
}

// ******************************* 清空块数据 *********************************
void CMFRC500_DEMODlg::ClearRWEditors(){
	((CEdit*)GetDlgItem(IDC_BLOCK0))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK1))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK2))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK3A))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK3B))->SetWindowText("");
	((CEdit*)GetDlgItem(IDC_BLOCK3C))->SetWindowText("");
}
// ******************************* 清空块数据 *********************************

void CMFRC500_DEMODlg::OnBtnWalletInit() 
{
	// 获取输入的密钥
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("请输入密钥！");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_WALLET))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("请选择扇区！");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_WALLET))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("请选择块！");
		return;
	}

	CString remainings;
	((CEdit*)GetDlgItem(IDC_REMAIN))->GetWindowText(remainings);
	if(remainings.IsEmpty()){
		isWalletOpeSuccess = false;
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("请输入初始化余额！");
		return;
	}
	long account = atol(remainings);

	unsigned char psw[6];
	unsigned char len;
	HexStr2CharArray(getKey, psw, &len);

	signed char status = Card_WalletInit(sectorNum, blockNum, keyType, psw, account);
	if(!status){
		isWalletOpeSuccess = true;
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("钱包初始化成功");
	}
	else{
		isWalletOpeSuccess = false;
		CString info = PrintErrInfos(status);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText(info);
	}
}

void CMFRC500_DEMODlg::OnBtnWalletCheck() 
{
	// 获取输入的密钥
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("请输入密钥！");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_WALLET))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("请选择扇区！");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_WALLET))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("请选择块！");
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
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("查询成功");
	}
	else{
		isWalletOpeSuccess = false;
		CString info = PrintErrInfos(status);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText(info);
	}
}

void CMFRC500_DEMODlg::OnBtnWalletRecharge() 
{	
	// 获取输入的密钥
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("请输入密钥！");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_WALLET))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("请选择扇区！");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_WALLET))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("请选择块！");
		return;
	}
	((CEdit*)GetDlgItem(IDC_REMAIN))->SetWindowText("");

	CString accountStr;
	((CEdit*)GetDlgItem(IDC_RECHARGE))->GetWindowText(accountStr);
	if(accountStr.IsEmpty()){
		isWalletOpeSuccess = false;
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("请输入充值金额！");
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
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("充值成功");
	}
	else{
		isWalletOpeSuccess = false;
		CString info = PrintErrInfos(status);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText(info);
	}
}

void CMFRC500_DEMODlg::OnBtnWalletDebit() 
{
	// 获取输入的密钥
	CString getKey;
	((CEdit*)GetDlgItem(IDC_KEY))->GetWindowText(getKey);
	if(getKey.IsEmpty()){
		MessageBox("请输入密钥！");
		return;
	}
	
	unsigned char sectorNum = ((CComboBox*)GetDlgItem(IDC_SECTOR_WALLET))->GetCurSel();
	if(CB_ERR == sectorNum){
		MessageBox("请选择扇区！");
		return;
	}

	unsigned char blockNum = ((CComboBox*)GetDlgItem(IDC_BLOCK_WALLET))->GetCurSel();
	if(CB_ERR == blockNum){
		MessageBox("请选择块！");
		return;
	}
	((CEdit*)GetDlgItem(IDC_REMAIN))->SetWindowText("");

	CString accountStr;
	((CEdit*)GetDlgItem(IDC_DEBIT))->GetWindowText(accountStr);
	if(accountStr.IsEmpty()){
		isWalletOpeSuccess = false;
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("请输入消费金额！");
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
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText("扣款成功");
	}
	else{
		isWalletOpeSuccess = false;
		CString info = PrintErrInfos(status);
		((CEdit*)GetDlgItem(IDC_WALLET_STATUS))->SetWindowText(info);
	}
}
