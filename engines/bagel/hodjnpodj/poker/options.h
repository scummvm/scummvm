// options.h -- definition of COptions class
// Written by John J. Xenakis for Boffo Games Inc., 1994

// Note to programmers:

//	Please do not modify this file.  Modify your copy of
//	"options.inc" if desired to add fields to the COptions class.



/*****************************************************************
 *
 *  options.h
 *
 *  Copyright (c) 1994 by Ledge Multimedia, All Rights Reserved
 *
 *  HISTORY
 *
 *      1.0      04/19/94     EDS     refer to options.cpp for details
 *
 *  MODULE DESCRIPTION:
 *
 *      Class definitions for COptions.
 *   
 *  RELEVANT DOCUMENTATION:
 *
 *      n/a
 *   
 ****************************************************************/


#ifndef _INC_OPTIONS
#define _INC_OPTIONS

#include <time.h>
#include "dibdoc.h"
#include "globals.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "poker.h"

#include "optres.h"

class COptions : public CDialog
{
private:

int	m_iDlgId;
	
// Construction
public:
//	COptions(CWnd* pParent = NULL, CPalette *pPalette = NULL);	// standard constructor
	COptions::COptions(CWnd* pParent,CPalette *pPalette, int iDlgId) ;
// #include "options.inc"		// put in your own include file

private:


// Dialog Data
	//{{AFX_DATA(COptions)
	enum { IDD = IDD_OPTIONS_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation
protected:
	void RefreshBackground(); 
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	void OnCancel();
	// Generated message map functions
	//{{AFX_MSG(COptions)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	virtual void OnOK();
	afx_msg void OnPaint();
  afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnClickedRules() ;
	afx_msg void OnClickedNewgame() ;
	afx_msg void OnClickedOptions() ;
	afx_msg void OnClickedReturn();
	afx_msg void OnClickedQuit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CFont   *m_pFont;
    CBitmap *m_pDlgBackground;
};

#endif //!_INC_OPTIONS
