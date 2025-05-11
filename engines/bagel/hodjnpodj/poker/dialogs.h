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

//#include <options.h>
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "poker.h"

#define	AMOUNTMAX	10
#define	AMOUNTMIN	1

//////////////////////////////////////////////////////////////////////////////
////////	Options Specific to Poker
//////////////////////////////////////////////////////////////////////////////
class COptionsDlg : public CBmpDialog
{
private:
	BOOL	m_bSoundOn;					// bool for telling me should I play sounds or not
	BOOL	m_bDisableSets;   			// bool for telling me should I disable the 
													// "Set Amount" and "Set Payoffs" buttons

public:
	COptionsDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_OPTIONS );
	void SetInitialOptions( BOOL =FALSE, BOOL =TRUE);	// Sets the private members
    void ClearDialogImage();
        
protected:
	virtual BOOL OnCommand(WPARAM, LPARAM);
	virtual BOOL OnInitDialog(void);	
  afx_msg void OnPaint();
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////
////////	Set Current Number of Crowns
//////////////////////////////////////////////////////////////////////////////
class CSetAmountDlg : public CBmpDialog
{
private:
	int 	m_nCurrentAmount;	// contains the current amount set
	char	m_cAmount [6];

public:
	CSetAmountDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_SETAMOUNT );
	void SetInitialOptions( long =0 );	// Sets the private members
    void ClearDialogImage();
    BOOL OnInitDialog();
        
protected:
        //{{AFX_MSG(COptions)
	virtual BOOL OnCommand(WPARAM, LPARAM);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnDestroy();
        //}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//////////////////////////////////////////////////////////////////////////////
////////	Set the Payoffs
//////////////////////////////////////////////////////////////////////////////
class CSetPayoffsDlg : public CBmpDialog
{
private:
char m_cPair[6];
char m_cPairJacksPlus[6];
char m_cTwoPair[6];
char m_cThreeOAK[6];
char m_cStriaght[6];
char m_cFlush[6];
char m_cFullHouse[6];
char m_cFourOAK[6];
char m_cStraightFlush[6];
char m_cRoyalFlush[6];
int  m_nSetOfOdds;	
BOOL m_bJustDisplay;
public:
	CSetPayoffsDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_WINRATIO, int = IDC_POPO, BOOL = FALSE );
    void ClearDialogImage();
	void OnKlingon( BOOL = TRUE );
	void OnKuwaiti( BOOL = TRUE );
	void OnMartian( BOOL = TRUE );
	void OnPopo( BOOL = TRUE );
	void OnVegas( BOOL = TRUE );
        
protected:
	virtual void DoDataExchange(CDataExchange*);
	virtual BOOL OnInitDialog(void);
	afx_msg void OnSetpayoffs();
    virtual BOOL OnCommand(WPARAM, LPARAM);
	afx_msg void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg	BOOL OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////
////////	User won dialog
//////////////////////////////////////////////////////////////////////////////
class CUserWonDlg : public CBmpDialog
{
private:
	int	m_nWinPhrase;	

public:
	CUserWonDlg(CWnd* pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_USERWON );
	void SetInitialOptions( int = 0 );
    void ClearDialogImage();
    BOOL OnInitDialog();
        
protected:
    virtual void OnOK(void);
	virtual BOOL OnCommand(WPARAM, LPARAM);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};


#endif // DIALOGS_H
