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
#include <cbofdlg.h>
#include <button.h>
#include <misc.h>

#define IDD_USERCFG 100

#define DIFF_MIN    0
#define DIFF_MAX    2
#define DIFF_DEF    2

#define TURN_DEF    FALSE

class CUserCfgDlg : public CBmpDialog
{
    public:
        CUserCfgDlg(CWnd *pParent = NULL, CPalette *pPalette = NULL, UINT = IDD_USERCFG);

    protected:

        virtual BOOL OnCommand(WPARAM, LPARAM);
        virtual BOOL OnInitDialog(void);
        void PutDlgData(void);
        void GetDlgData(void);
        void DispLimit(void);
        VOID LoadIniSettings(VOID);
        VOID SaveIniSettings(VOID);

        void OnHScroll(UINT, UINT, CScrollBar *);
        void OnDestroy();
        void OnPaint();
		void ClearDialogImage(void);
        DECLARE_MESSAGE_MAP()

        CColorButton *m_pOKButton;              // OKAY button on scroll
        CColorButton *m_pCancelButton;          // Cancel button on scroll
        CColorButton *m_pDefaultsButton;        // Defaults button on scroll
        CRadioButton *m_pUserButton;
        CRadioButton *m_pCompButton;
        CText        *m_pTxtDifficulty;
        CText        *m_pTxtLevel;
        CText        *m_pTxtOrder;
        CScrollBar   *m_pScrollBar;
        UINT          m_nDifficultyLevel;       // Difficulty level for BattleFish
        BOOL          m_bSave;                  // True if should save theses settings
        BOOL          m_bUserGoesFirst;         // TRUE if Human player goes 1st
};

#endif // USERCFG_H
