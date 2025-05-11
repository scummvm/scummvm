/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * dialogs.h
 *
 * HISTORY
 *
 *
 * MODULE DESCRIPTION:
 *
 *      header file for all dialogs used in poker
 *   
 * RELEVANT DOCUMENTATION:
 *
 *      [Specifications, documents, test plans, etc.]
 *   
 ****************************************************************/
#ifndef DIALOGS_H
#define DIALOGS_H

#define LEVELMAX       36
#define LIVESMAX       5
#define LEVELMIN       1
#define LIVESMIN       1

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "packrat.h"

//////////////////////////////////////////////////////////////////////////////
////////	Options Specific to Pack Rat
//////////////////////////////////////////////////////////////////////////////
class CPackRatOptDlg : public CBmpDialog
{
private:
	int 	m_nLives;
	int 	m_nLevel;

public:
	CPackRatOptDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_MINIOPTIONS_DIALOG );
	void SetInitialOptions( int =1, int =1 );	// Sets the private members
    void ClearDialogImage();
	BOOL OnInitDialog();
        
protected:
    //{{AFX_MSG(COptions)
    virtual void OnCancel(void);
    virtual void OnOK(void);
	virtual BOOL OnCommand(WPARAM, LPARAM);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
    //}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////
////////	Pack Rat Message Box
//////////////////////////////////////////////////////////////////////////////
class CMsgDlg : public CBmpDialog
{
private:
    int 	m_nWhichMsg;
	long	m_lCurrentScore;
	int 	m_nLevel;
	
public:
	CMsgDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_MESSAGEBOX );
	void SetInitialOptions( int, long, int =0 );	// Sets the private members
    void ClearDialogImage();
	BOOL OnInitDialog();
        
protected:
    //{{AFX_MSG(COptions)
    virtual void OnCancel(void);
    virtual void OnOK(void);
	virtual BOOL OnCommand(WPARAM, LPARAM);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
    //}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // DIALOGS_H
