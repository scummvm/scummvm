/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  usercfg.cpp   -
*
*  HISTORY
*
*       1.00        05/09/94    BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*
*
*  LOCALS:
*
*
*
*  GLOBALS:
*
*
*
*  RELEVANT DOCUMENTATION:
*
*
****************************************************************/
#include <afxwin.h>
#include <afxext.h>
#include <assert.h>
#include <globals.h>
#include <text.h>
#include <cbofdlg.h>
#include <button.h>
#include "usercfg.h"
#include "resource.h"

#define ID_RESET    104
#define ID_LIMIT    105

#define ID_PLAYER    106
#define ID_COMPUTER  107

#define PAGE_SIZE   1

CHAR *pszDiffLevel[DIFF_MAX+1] = {
    "Wimpy",
    "Average",
    "Hefty"
};

extern const char *INI_SECTION;


CUserCfgDlg::CUserCfgDlg(CWnd *pParent, CPalette *pPalette, UINT nID)
        : CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP")
{
    m_pOKButton = NULL;
    m_pCancelButton = NULL;
    m_pDefaultsButton = NULL;
    m_pUserButton = NULL;
    m_pCompButton = NULL;
    DoModal();
}

BOOL CUserCfgDlg::OnInitDialog(void)
{
    CRect tmpRect;
    CDC *pDC;

    CBmpDialog::OnInitDialog();

    m_pTxtLevel = NULL;
    m_pTxtDifficulty = NULL;

    tmpRect.SetRect(22, 135, 122, 155);
    if ((m_pScrollBar = new CScrollBar) != NULL) {
        m_pScrollBar->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_LIMIT);
        m_pScrollBar->SetScrollRange(DIFF_MIN, DIFF_MAX, TRUE);
    }

    if ((pDC = GetDC()) != NULL) {

        tmpRect.SetRect(25, 111, 80, 131);
        if ((m_pTxtLevel = new CText) != NULL) {
            m_pTxtLevel->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
        }

        tmpRect.SetRect(81, 111, 146, 131);
        if ((m_pTxtDifficulty = new CText) != NULL) {
            m_pTxtDifficulty->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
        }

        tmpRect.SetRect(25, 20, 150, 40);
        if ((m_pTxtOrder = new CText) != NULL) {
            m_pTxtOrder->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
        }

        ReleaseDC(pDC);
    }

    if ((m_pOKButton = new CColorButton) != NULL) {       // build a color QUIT button to let us exit
        (*m_pOKButton).SetPalette(m_pPalette);            // set the palette to use
        (*m_pOKButton).SetControl(IDOK, this);            // tie to the dialog control
	}
	
    if ((m_pCancelButton = new CColorButton) != NULL) {   // build a color QUIT button to let us exit
        (*m_pCancelButton).SetPalette(m_pPalette);        // set the palette to use
        (*m_pCancelButton).SetControl(IDCANCEL, this);    // tie to the dialog control
	}
	
    if ((m_pDefaultsButton = new CColorButton) != NULL) { // build a color QUIT button to let us exit
        (*m_pDefaultsButton).SetPalette(m_pPalette);      // set the palette to use
        (*m_pDefaultsButton).SetControl(ID_RESET, this);  // tie to the dialog control
	}

    if ((m_pUserButton = new CRadioButton) != NULL) {
        tmpRect.SetRect(21, 32, 75, 45);
        //m_pUserButton->Create("Human", BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP | BS_OWNERDRAW, tmpRect, this, ID_PLAYER);
        m_pUserButton->SetPalette(m_pPalette);
        m_pUserButton->SetControl(ID_PLAYER, this);
    }

    if ((m_pCompButton = new CRadioButton) != NULL) {
        tmpRect.SetRect(21, 45, 75, 58);
        //m_pUserButton->Create("Computer", BS_AUTORADIOBUTTON | WS_TABSTOP | BS_OWNERDRAW, tmpRect, this, ID_COMPUTER);
        m_pCompButton->SetPalette(m_pPalette);
        m_pCompButton->SetControl(ID_COMPUTER, this);
    }

    m_bSave = FALSE;

    LoadIniSettings();

    PutDlgData();
    return(TRUE);
}


void CUserCfgDlg::PutDlgData()
{
    m_pScrollBar->SetScrollPos(m_nDifficultyLevel);

    m_pUserButton->SetCheck(m_bUserGoesFirst);
    m_pCompButton->SetCheck(!m_bUserGoesFirst);
}

void CUserCfgDlg::GetDlgData()
{
    m_nDifficultyLevel = m_pScrollBar->GetScrollPos();

    m_bUserGoesFirst = FALSE;
    if (m_pUserButton->GetCheck() == 1)
        m_bUserGoesFirst = TRUE;
}


BOOL CUserCfgDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    //
    // respond to user
    //
    if (HIWORD(lParam) == BN_CLICKED) {

        switch (wParam) {

            case IDOK:
                m_bSave = TRUE;
                ClearDialogImage();
                EndDialog(IDOK);
                return(FALSE);

            case IDCANCEL:
                ClearDialogImage();
                EndDialog(IDCANCEL);
                return(FALSE);

            case ID_PLAYER:
                m_bUserGoesFirst = TRUE;
                PutDlgData();
                break;

            case ID_COMPUTER:
                m_bUserGoesFirst = FALSE;
                PutDlgData();
                break;

            /*
            * reset params to default
            */
            case ID_RESET:

                m_nDifficultyLevel = DIFF_DEF;
                m_bUserGoesFirst = TURN_DEF;

                PutDlgData();
                DispLimit();
                break;

            default:
                break;
        }
    }

    return(CBmpDialog::OnCommand(wParam, lParam));
}

VOID CUserCfgDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScroll)
{
    switch (nSBCode) {

        case SB_LEFT:
            m_nDifficultyLevel = DIFF_MIN;
            break;

        case SB_PAGELEFT:
            m_nDifficultyLevel -= PAGE_SIZE;
            break;

        case SB_LINELEFT:
            if (m_nDifficultyLevel > DIFF_MIN)
                m_nDifficultyLevel--;
            break;

        case SB_RIGHT:
            m_nDifficultyLevel = DIFF_MAX;
            break;

        case SB_PAGERIGHT:
            m_nDifficultyLevel += PAGE_SIZE;
            break;

        case SB_LINERIGHT:
            if (m_nDifficultyLevel < DIFF_MAX)
                m_nDifficultyLevel++;
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            m_nDifficultyLevel = nPos;
            break;

        default:
            break;
    }

    if (m_nDifficultyLevel <= DIFF_MIN)
        m_nDifficultyLevel = DIFF_MIN;
    if (m_nDifficultyLevel > DIFF_MAX)
        m_nDifficultyLevel = DIFF_MAX;

    // can't access a null pointer
    assert(pScroll != NULL);

    pScroll->SetScrollPos(m_nDifficultyLevel);

    DispLimit();
}


/*****************************************************************
*
*  LoadIniSettings
*
*  FUNCTIONAL DESCRIPTION:
*
*       Loads this game's parameters from .INI file
*
*  FORMAL PARAMETERS:
*
*       None
*
*  RETURN VALUE:
*
*       None
*
****************************************************************/
VOID CUserCfgDlg::LoadIniSettings(VOID)
{
    INT nVal;

    // Get the Difficulty level  (0..2)
    //
    nVal = GetPrivateProfileInt(INI_SECTION, "DifficultyLevel", DIFF_DEF, INI_FILENAME);
    m_nDifficultyLevel = nVal;
    if (nVal < DIFF_MIN || nVal > DIFF_MAX)
        m_nDifficultyLevel = DIFF_DEF;

    // Get the UserGoesFirst option setting
    //
    nVal = GetPrivateProfileInt(INI_SECTION, "UserGoesFirst", TURN_DEF, INI_FILENAME);
    m_bUserGoesFirst = (nVal == 0 ? FALSE : TRUE);
}

VOID CUserCfgDlg::SaveIniSettings(VOID)
{
    CHAR tmpBuf[10];

    itoa(m_nDifficultyLevel, tmpBuf, 10);
    WritePrivateProfileString(INI_SECTION, "DifficultyLevel", tmpBuf, INI_FILENAME);

    itoa(m_bUserGoesFirst, tmpBuf, 10);
    WritePrivateProfileString(INI_SECTION, "UserGoesFirst", tmpBuf, INI_FILENAME);
}

void CUserCfgDlg::OnPaint(void)
{
    CDC *pDC;

    CBmpDialog::OnPaint();

    if ((pDC = GetDC()) != NULL) {
        m_pTxtLevel->DisplayString(pDC, "Difficulty:", 14, TEXT_BOLD, RGB( 0, 0, 0));
        m_pTxtOrder->DisplayString(pDC, "Who goes first?", 14, TEXT_BOLD, RGB( 0, 0, 0));
        ReleaseDC(pDC);
    }

    DispLimit();
}

void CUserCfgDlg::OnDestroy()
{
    if (m_pOKButton != NULL) {                          // release the button
        delete m_pOKButton;
        m_pOKButton = NULL;
	}

    if (m_pCancelButton != NULL) {                        // release the button
        delete m_pCancelButton;
        m_pCancelButton = NULL;
	}

    if (m_pDefaultsButton != NULL) {                    // release the button
        delete m_pDefaultsButton;
        m_pDefaultsButton = NULL;
	}

    assert(m_pTxtDifficulty != NULL);
    if (m_pTxtDifficulty != NULL) {
        delete m_pTxtDifficulty;
        m_pTxtDifficulty = NULL;
    }

    assert(m_pTxtLevel != NULL);
    if (m_pTxtLevel != NULL) {
        delete m_pTxtLevel;
        m_pTxtLevel = NULL;
    }

    if (m_pTxtOrder != NULL) {
        delete m_pTxtOrder;
        m_pTxtOrder = NULL;
    }

    //
    // de-allocate the scroll bar
    //
    assert(m_pScrollBar != NULL);
    if (m_pScrollBar != NULL) {
        delete m_pScrollBar;
        m_pScrollBar = NULL;
    }

    CBmpDialog::OnDestroy();
}


void CUserCfgDlg::ClearDialogImage(void)
{
    if (m_bSave) {
        GetDlgData();
        SaveIniSettings();
    }

    if (m_pCompButton != NULL) {
        delete m_pCompButton;
        m_pCompButton = NULL;
    }

    if (m_pUserButton != NULL) {
        delete m_pUserButton;
        m_pUserButton = NULL;
    }

    if (m_pOKButton != NULL) {                          // release the button
        delete m_pOKButton;
        m_pOKButton = NULL;
	}

    if (m_pCancelButton != NULL) {                        // release the button
        delete m_pCancelButton;
        m_pCancelButton = NULL;
	}

    if (m_pDefaultsButton != NULL) {                    // release the button
        delete m_pDefaultsButton;
        m_pDefaultsButton = NULL;
	}

	ValidateRect(NULL);
}

void CUserCfgDlg::DispLimit(void)
{
    CDC *pDC;

    if ((pDC = GetDC()) != NULL) {

        m_pTxtDifficulty->DisplayString(pDC, pszDiffLevel[m_nDifficultyLevel], 14, TEXT_BOLD, RGB( 0, 0, 0));
        ReleaseDC(pDC);
    }
}

BEGIN_MESSAGE_MAP(CUserCfgDlg, CBmpDialog)
	ON_WM_DESTROY()
    ON_WM_HSCROLL()
    ON_WM_PAINT()
END_MESSAGE_MAP()
