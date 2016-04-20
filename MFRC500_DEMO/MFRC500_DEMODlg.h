// MFRC500_DEMODlg.h : header file
//

#if !defined(AFX_MFRC500_DEMODLG_H__E77B6DBF_22D6_4B98_B811_0C3791C4129F__INCLUDED_)
#define AFX_MFRC500_DEMODLG_H__E77B6DBF_22D6_4B98_B811_0C3791C4129F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NOPARITY            0
#define ODDPARITY           1
#define EVENPARITY          2
#define MARKPARITY          3
#define SPACEPARITY         4

#define ONESTOPBIT          0
#define ONE5STOPBITS        1
#define TWOSTOPBITS         2

#define CBR_110             110
#define CBR_300             300
#define CBR_600             600
#define CBR_1200            1200
#define CBR_2400            2400
#define CBR_4800            4800
#define CBR_9600            9600
#define CBR_14400           14400
#define CBR_19200           19200
#define CBR_38400           38400
#define CBR_56000           56000
#define CBR_57600           57600
#define CBR_115200          115200
#define CBR_128000          128000
#define CBR_256000          256000

#define DATABITS_5        ((WORD)0x0001)
#define DATABITS_6        ((WORD)0x0002)
#define DATABITS_7        ((WORD)0x0004)
#define DATABITS_8        ((WORD)0x0008)

/////////////////////////////////////////////////////////////////////////////
// CMFRC500_DEMODlg dialog

class CMFRC500_DEMODlg : public CDialog
{
// Construction
public:
	CMFRC500_DEMODlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMFRC500_DEMODlg)
	enum { IDD = IDD_MFRC500_DEMO_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFRC500_DEMODlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMFRC500_DEMODlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnOpenPort();
	afx_msg void OnBtnGetUID();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBtnDefaultKey();
	afx_msg void OnRadioA();
	afx_msg void OnRadioB();
	afx_msg void OnBtnAuthen();
	afx_msg void OnBtnReadSector();
	afx_msg void OnBtnReadBlock();
	afx_msg void OnBtnWriteBlock();
	afx_msg void OnBtnWalletInit();
	afx_msg void OnBtnWalletCheck();
	afx_msg void OnBtnWalletRecharge();
	afx_msg void OnBtnWalletDebit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool isPortOpened;
	CFont m_font;
	bool isUIDGetSuccess;
	bool isWalletOpeSuccess;
	unsigned char keyType;

	void InitPortSettings();
	void InitSectorBlock();
	void PortSettingEnable(bool enable);
	void HexStr2CharArray(CString hexStr, unsigned char *asc, unsigned char *asc_len);
	void ClearRWEditors();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFRC500_DEMODLG_H__E77B6DBF_22D6_4B98_B811_0C3791C4129F__INCLUDED_)
