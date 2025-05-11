/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  usercfg.h -
*
*  HISTORY
*
*       1.00        05/09/94    BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*       Header for user configuration dialog
*
*  RELEVANT DOCUMENTATION:
*
*
*
****************************************************************/
#ifndef USERCFG_H
#define USERCFG_H

#include <afxwin.h>
#include <afxext.h>
#include <button.h>
#include <cbofdlg.h>
#include <text.h>
#include "main.h"

#define RGBCOLOR_DARKRED	RGB(0, 128, 0)
#define RGBCOLOR_BLACK		RGB(0, 0, 0)
#define RGBCOLOR_WHITE		RGB(255, 255, 255)

#define FONT_SIZE		18				// CText object font size

// CText message
#define TEXT_LEFT			20 
#define TEXT_TOP			25
#define TEXT_RIG			220
#define TEXT_BOT			45

// card back rect 1
#define CBCK_RECT1_LEFT		40
#define CBCK_RECT1_TOP		55
#define CBCK_RECT1_RIG		110
#define CBCK_RECT1_BOT		125

// card back rect 2
#define CBCK_RECT2_LEFT		120
#define CBCK_RECT2_TOP		55
#define CBCK_RECT2_RIG		190
#define CBCK_RECT2_BOT		125

#define DELTA				0

#define IDD_USERCFG			100

class CUserCfgDlg : public CBmpDialog
{
    public:
        CUserCfgDlg(CWnd *pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_USERCFG);

    protected:

        virtual void DoDataExchange(CDataExchange *);
        virtual BOOL OnInitDialog(void);

        void OnClose();
        void OnPaint();
		void OnLButtonUp(UINT, CPoint);
		void OnCancel();
		void OnOK();
        DECLARE_MESSAGE_MAP()
		void MyFocusRect(CDC*, CRect, int nDrawMode, COLORREF);

private:
		UINT        	m_nTimeLimit;		//
        BOOL        	m_bSave;			// True if should save theses values
		CRect			m_cRectCardBack1;
		CRect			m_cRectCardBack2;
		UINT			m_nCardBack;
		CColorButton 	*m_pOKButton;		// OKAY button on scroll
		CText			*m_ctextBox;
};

#endif // USERCFG_H
