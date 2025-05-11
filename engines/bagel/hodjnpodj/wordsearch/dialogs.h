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

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "wordsrch.h"

//////////////////////////////////////////////////////////////////////////////
////////	Options Specific to Word Search
//////////////////////////////////////////////////////////////////////////////
class CWSOptDlg : public CBmpDialog
{
private:
	int 	m_nGameTime;
	BOOL	m_bShowWord;
	BOOL	m_bWordForwardOnly;

public:
	CWSOptDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_MINIOPTIONS_DIALOG );
	void SetInitialOptions( int =0, BOOL =TRUE, BOOL =FALSE );	// Sets the private members
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
	long	m_nWordsLeft;
	
public:
	CMsgDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_MESSAGEBOX );
	void SetInitialOptions( int, int );	// Sets the private members
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
