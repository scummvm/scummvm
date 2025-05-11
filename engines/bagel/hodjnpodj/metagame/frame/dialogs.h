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
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "hodjpodj.h"
#include "bfc.h"

#define	BUTTONSBMP		".\\ART\\NEWBTNS.BMP"

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Word Search
//////////////////////////////////////////////////////////////////////////////
class CMainGameDlg : public CBmpDialog
{
private:

public:
    CMainGameDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL );

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

    // data
#ifndef BMP_BUTTONS
    CColorButton	*m_pPlayMetaButton;
    CColorButton	*m_pPlayMiniButton;
    CColorButton	*m_pRestoreButton;
    CColorButton	*m_pGrandTourButton;
    CColorButton	*m_pRestartMovieButton;
    CColorButton	*m_pQuitButton;
#else
    CBmpButton	*m_pPlayMetaButton;
    CBmpButton	*m_pPlayMiniButton;
    CBmpButton	*m_pRestoreButton;
    CBmpButton	*m_pGrandTourButton;
    CBmpButton	*m_pRestartMovieButton;
    CBmpButton	*m_pQuitButton;
	CBitmap		*m_pButtonsBmp;
	CPalette	*m_pButtonPalette;
#endif
};

//////////////////////////////////////////////////////////////////////////////
////////    Options Specific to Word Search
//////////////////////////////////////////////////////////////////////////////
class CMetaSetupDlg : public CBmpDialog
{
private:
    CBfcMgr     *m_lpMetaGameStruct;
    int         m_nHodjSkillLevel;
    int         m_nPodjSkillLevel;
    int         m_nGameTime;
    BOOL        m_bPodjIsComputer;

public:
    CMetaSetupDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL );
    void SetInitialOptions(CBfcMgr *);  // Sets the private members

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

#endif // DIALOGS_H
