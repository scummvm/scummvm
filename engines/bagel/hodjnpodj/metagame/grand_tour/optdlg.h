/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * optdlg.h
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
#ifndef OPTDLG_H
#define OPTDLG_H

#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "gtlfrm.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Word Search
//////////////////////////////////////////////////////////////////////////////
class CMetaOptDlg : public CBmpDialog {
private:
	CWnd         *m_pParent;
	CPalette     *m_pPalette;
	CBfcMgr      *m_pBfcMgr;
	CColorButton *m_pRulesGameButton;
	CColorButton *m_pSaveGameButton;
	CColorButton *m_pLeaveGameButton;
	CColorButton *m_pContinueGameButton;
	CColorButton *m_pOptionsGameButton;

	BOOL         m_bMusic;
	BOOL         m_bSoundFX;
	BOOL         m_bScrolling;

public:
	CMetaOptDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL);
	void SetInitialOptions(CBfcMgr *);    // Sets the private members

private:
	BOOL OnInitDialog();
	void ClearDialogImage(void);

protected:
	//{{AFX_MSG(COptions)
	virtual void OnOK(void);
	virtual void OnCancel(void);
	virtual BOOL OnCommand(WPARAM, LPARAM);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif // OPTDLG_H

