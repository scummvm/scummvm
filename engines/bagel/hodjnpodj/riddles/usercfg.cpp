/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_RIDDLES_USERCFG_H
#define HODJNPODJ_RIDDLES_USERCFG_H

#include <afxwin.h>
#include <afxext.h>
#include <assert.h>
#include <globals.h>
#include <text.h>
#include <cbofdlg.h>
#include <menures.h>
#include <stdinc.h>
#include "bagel/hodjnpodj/riddles/usercfg.h"
#include "bagel/hodjnpodj/riddles/riddles.h"

namespace Bagel {
namespace HodjNPodj {
namespace Riddles {

#define ID_RESET     104
#define ID_LIMIT     105
#define ID_LEVEL     106

#define PAGE_SIZE    2

#define SCROLL_MIN  0
#define SCROLL_DEF  4
#define SCROLL_MAX  MAX_TIME_LIMITS - 1


extern const char *INI_SECTION;

#define MAX_TIME_LIMITS 15
#define MAX_LEVELS 4

STATIC CHAR *pszLevels[MAX_LEVELS] = {
    "Easy",
    "Medium",
    "Hard",
    "Random"
};

static CHAR *pTimeLimit[MAX_TIME_LIMITS] = {
    "10 seconds",
    "15 seconds",
    "20 seconds",
    "25 seconds",
    "30 seconds",
    "40 seconds",
    "50 seconds",
    "60 seconds",
    "75 seconds",
    "90 seconds",
    "2 minutes",
    "3 minutes",
    "4 minutes",
    "5 minutes",
    "None"
};

static INT nTimeLimit[MAX_TIME_LIMITS] = {
    10, 15, 20, 25, 30, 40, 50, 60, 75, 90, 120, 180, 240, 300, 301
};

static	CColorButton *pOKButton = NULL;						// OKAY button on scroll
static	CColorButton *pCancelButton = NULL;					// Cancel button on scroll
static	CColorButton *pDefaultsButton = NULL;				// Defaults button on scroll

CUserCfgDlg::CUserCfgDlg(CWnd *pParent, CPalette *pPalette, UINT nID)
            : CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP")
{
    DoModal();
}

void CUserCfgDlg::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
}


void CUserCfgDlg::PutDlgData()
{
    m_pScrollBar->SetScrollPos(GetIndex(m_nTimeLimit));
    m_pLevelScroll->SetScrollPos(m_nDifficultyLevel);
}

INT CUserCfgDlg::GetIndex(INT nVal)
{
    INT i, nIndex;

    nIndex = SCROLL_DEF;
    for (i = 0; i < MAX_TIME_LIMITS; i++) {
        if (nVal == nTimeLimit[i]) {
            nIndex = i;
            break;
        }
    }

    return(nIndex);
}

void CUserCfgDlg::GetDlgData()
{
    m_nTimeLimit = nTimeLimit[m_pScrollBar->GetScrollPos()];
    m_nDifficultyLevel = m_pLevelScroll->GetScrollPos();
}


BOOL CUserCfgDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    /*
    * respond to user
    */
    if (HIWORD(lParam) == BN_CLICKED) {

        switch (wParam) {

            case IDOK:
                m_bSave = TRUE;
                PostMessage(WM_CLOSE, 0, 0);
                return(FALSE);

            case IDCANCEL:
                PostMessage(WM_CLOSE, 0, 0);
                return(FALSE);

            /*
            * reset params to default
            */
            case ID_RESET:

                m_nTimeLimit = LIMIT_DEF;
                m_nDifficultyLevel = LEVEL_DEF;

                PutDlgData();
                DispLimit();
                DispLevel();
                break;

            default:
                break;
        }
    }

    return(CDialog::OnCommand(wParam, lParam));
}

VOID CUserCfgDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScroll)
{
    UINT nCurPos;

    // can't access a null pointer
    assert(pScroll != NULL);

    nCurPos = pScroll->GetScrollPos();

    if (pScroll == m_pLevelScroll) {

        switch (nSBCode) {

            case SB_LEFT:
                nCurPos = LEVEL_MIN;
                break;

            case SB_PAGELEFT:
            case SB_LINELEFT:
                if (nCurPos > LEVEL_MIN)
                    nCurPos--;
                break;

            case SB_RIGHT:
                nCurPos = LEVEL_MAX;
                break;

            case SB_PAGERIGHT:
            case SB_LINERIGHT:
                if (nCurPos < LEVEL_MAX)
                    nCurPos++;
                break;

            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
                nCurPos = nPos;
                break;

            default:
                break;
        }

        if (nCurPos <= LEVEL_MIN)
            nCurPos = LEVEL_MIN;
        if (nCurPos > LEVEL_MAX)
            nCurPos = LEVEL_MAX;

        pScroll->SetScrollPos(nCurPos);

        m_nDifficultyLevel = nCurPos;

        DispLevel();

    } else {

        switch (nSBCode) {

            case SB_LEFT:
                nCurPos = SCROLL_MIN;
                break;

            case SB_PAGELEFT:
                nCurPos -= PAGE_SIZE;
                break;

            case SB_LINELEFT:
                if (nCurPos > SCROLL_MIN)
                    nCurPos--;
                break;

            case SB_RIGHT:
                nCurPos = SCROLL_MAX;
                break;

            case SB_PAGERIGHT:
                nCurPos += PAGE_SIZE;
                break;

            case SB_LINERIGHT:
                if (nCurPos < SCROLL_MAX)
                    nCurPos++;
                break;

            case SB_THUMBPOSITION:
            case SB_THUMBTRACK:
                nCurPos = nPos;
                break;

            default:
                break;
        }

        if (nCurPos <= SCROLL_MIN)
            nCurPos = SCROLL_MIN;
        if (nCurPos > SCROLL_MAX)
            nCurPos = SCROLL_MAX;

        pScroll->SetScrollPos(nCurPos);

        m_nTimeLimit = nTimeLimit[nCurPos];

        DispLimit();
    }
}


BOOL CUserCfgDlg::OnInitDialog(void)
{
    CRect tmpRect;
    CDC *pDC;

    CBmpDialog::OnInitDialog();

    // Inits
    tmpRect.SetRect(22, 135, 122, 155);
    m_pScrollBar = new CScrollBar;
    m_pScrollBar->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_LIMIT);
    m_pScrollBar->SetScrollRange(SCROLL_MIN, SCROLL_MAX, TRUE);

    tmpRect.SetRect(22, 56, 90, 76);
    m_pLevelScroll = new CScrollBar;
    m_pLevelScroll->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_LEVEL);
    m_pLevelScroll->SetScrollRange(LEVEL_MIN, LEVEL_MAX, TRUE);

    pDC = GetDC();

    tmpRect.SetRect(23, 116, 88, 136);
    if ((m_pTxtLimit = new CText) != NULL) {
        m_pTxtLimit->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
    }

    tmpRect.SetRect(88, 116, 153, 136);
    if ((m_pTxtTimeLimit = new CText) != NULL) {
        m_pTxtTimeLimit->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
    }

    tmpRect.SetRect(23, 40, 150, 55);
    if ((m_pTxtLevel = new CText) != NULL) {
        m_pTxtLevel->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
    }

    ReleaseDC(pDC);

	if ((pOKButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pOKButton).SetPalette(m_pPalette);						// set the palette to use
		(*pOKButton).SetControl(IDOK,this);				// tie to the dialog control
	}
	
	if ((pCancelButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pCancelButton).SetPalette(m_pPalette);						// set the palette to use
		(*pCancelButton).SetControl(IDCANCEL,this);				// tie to the dialog control
	}
	
	if ((pDefaultsButton = new CColorButton) != NULL) {					// build a color QUIT button to let us exit
		(*pDefaultsButton).SetPalette(m_pPalette);						// set the palette to use
		(*pDefaultsButton).SetControl(ID_RESET,this);				// tie to the dialog control
	}
	
    m_bSave = FALSE;

    m_nTimeLimit = GetPrivateProfileInt(INI_SECTION, "TimeLimit", LIMIT_DEF, INI_FILENAME);
    if (m_nTimeLimit == 0)
        m_nTimeLimit = LIMIT_MAX;
    else if ((m_nTimeLimit < LIMIT_MIN) || (m_nTimeLimit > LIMIT_MAX))
        m_nTimeLimit = LIMIT_DEF;

    m_nDifficultyLevel = GetPrivateProfileInt(INI_SECTION, "DifficultyLevel", LEVEL_DEF, INI_FILENAME);
    if ((m_nDifficultyLevel < LEVEL_MIN) || (m_nDifficultyLevel > LEVEL_MAX))
        m_nDifficultyLevel = LEVEL_DEF;

    PutDlgData();
    return(TRUE);
}


void CUserCfgDlg::OnPaint(void)
{
    CDC *pDC;

    CBmpDialog::OnPaint();

    if ((pDC = GetDC()) != NULL) {
        m_pTxtLimit->DisplayString(pDC, "Time Limit:", 14, FW_BOLD, RGB( 0, 0, 0));
        ReleaseDC(pDC);
    }

    DispLimit();
    DispLevel();
}

VOID CUserCfgDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CBmpDialog::OnLButtonDown(nFlags, point);
}


void CUserCfgDlg::OnClose()
{
    char tmpBuf[8];
    CDC *pDC;

    if (m_bSave) {
        GetDlgData();

        wsprintf(tmpBuf, "0");
        if (m_nTimeLimit != LIMIT_MAX)
            itoa(m_nTimeLimit, tmpBuf, 10);
        WritePrivateProfileString(INI_SECTION, "TimeLimit", tmpBuf, INI_FILENAME);

        itoa(m_nDifficultyLevel, tmpBuf, 10);
        WritePrivateProfileString(INI_SECTION, "DifficultyLevel", tmpBuf, INI_FILENAME);
    }

    pDC = GetDC();

	if (pOKButton != NULL) {                          // release the button
		delete pOKButton;
		pOKButton = NULL;
	}

	if (pCancelButton != NULL) {                     	// release the button
		delete pCancelButton;
		pCancelButton = NULL;
	}

	if (pDefaultsButton != NULL) {                    // release the button
		delete pDefaultsButton;
		pDefaultsButton = NULL;
	}

    assert(m_pTxtLevel != NULL);
    if (m_pTxtLevel != NULL) {
        m_pTxtLevel->RestoreBackground(pDC);
        delete m_pTxtLevel;
        m_pTxtLevel = NULL;
    }

    assert(m_pTxtTimeLimit != NULL);
    if (m_pTxtTimeLimit != NULL) {
        m_pTxtTimeLimit->RestoreBackground(pDC);
        delete m_pTxtTimeLimit;
        m_pTxtTimeLimit = NULL;
    }

    assert(m_pTxtLimit != NULL);
    if (m_pTxtLimit != NULL) {
        m_pTxtLimit->RestoreBackground(pDC);
        delete m_pTxtLimit;
        m_pTxtLimit = NULL;
    }

    ReleaseDC(pDC);

    //
    // de-allocate the scrol bars
    //
    assert(m_pScrollBar != NULL);
    if (m_pScrollBar != NULL) {
        delete m_pScrollBar;
        m_pScrollBar = NULL;
    }

    assert(m_pLevelScroll != NULL);
    if (m_pLevelScroll != NULL) {
        delete m_pLevelScroll;
        m_pLevelScroll = NULL;
    }

	ClearDialogImage();
    EndDialog(0);
}

void CUserCfgDlg::ClearDialogImage(void)
{
	if (pOKButton != NULL) {                          // release the button
		delete pOKButton;
		pOKButton = NULL;
	}

	if (pCancelButton != NULL) {                     	// release the button
		delete pCancelButton;
		pCancelButton = NULL;
	}

	if (pDefaultsButton != NULL) {                    // release the button
		delete pDefaultsButton;
		pDefaultsButton = NULL;
	}


	ValidateRect(NULL);
}

VOID CUserCfgDlg::DispLimit(VOID)
{
    CDC *pDC;

    if ((pDC = GetDC()) != NULL) {

        m_pTxtTimeLimit->DisplayString(pDC, pTimeLimit[GetIndex(m_nTimeLimit)], 14, FW_BOLD, RGB( 0, 0, 0));
        ReleaseDC(pDC);
    }
}

VOID CUserCfgDlg::DispLevel(VOID)
{
    CHAR szBuf[40];
    CDC *pDC;

    if ((pDC = GetDC()) != NULL) {

        wsprintf(szBuf, "Difficulty: %s", pszLevels[m_nDifficultyLevel]);
        m_pTxtLevel->DisplayString(pDC, szBuf, 14, FW_BOLD, RGB( 0, 0, 0));
        ReleaseDC(pDC);
    }
}

BEGIN_MESSAGE_MAP(CUserCfgDlg, CBmpDialog)
    ON_WM_CLOSE()
    ON_WM_HSCROLL()
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

} // namespace Riddles
} // namespace HodjNPodj
} // namespace Bagel
