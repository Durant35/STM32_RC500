// MFRC500_DEMO.h : main header file for the MFRC500_DEMO application
//

#if !defined(AFX_MFRC500_DEMO_H__6551F43E_600B_4BE1_AFB8_658305CB96EA__INCLUDED_)
#define AFX_MFRC500_DEMO_H__6551F43E_600B_4BE1_AFB8_658305CB96EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMFRC500_DEMOApp:
// See MFRC500_DEMO.cpp for the implementation of this class
//

class CMFRC500_DEMOApp : public CWinApp
{
public:
	CMFRC500_DEMOApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFRC500_DEMOApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMFRC500_DEMOApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFRC500_DEMO_H__6551F43E_600B_4BE1_AFB8_658305CB96EA__INCLUDED_)
