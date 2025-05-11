/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  usercfg.cpp   - User options for Fuge
*
*  HISTORY
*
*       1.00        06/21/94    BCW     Created this file
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
#include <menures.h> 
#include <button.h>
#include "usercfg.h"

#define ID_RESET     104

#define ID_SCROLL1   105
#define ID_SCROLL2   106
#define ID_SCROLL3   107
#define ID_SCROLL4   108

#define ID_WALLS     109

#define PAGE_SIZE    2

extern const char *INI_SECTION;

static	CColorButton *pOKButton = NULL;						// OKAY button on scroll
static	CColorButton *pCancelButton = NULL;					// Cancel button on scroll
static	CColorButton *pDefaultsButton = NULL;				// Defaults button on scroll


CUserCfgDlg::CUserCfgDlg(CWnd *pParent, CPalette *pPalette, UINT nID)
        : CBmpDialog(pParent, pPalette, nID, ".\\ART\\SSCROLL.BMP")
{
    // Inits
    //
    m_pScrollBar1 = NULL;
    m_pScrollBar2 = NULL;
    m_pScrollBar3 = NULL;
    m_pScrollBar4 = NULL;
    m_pTxtNumBalls = NULL;
    m_pTxtStartLevel = NULL;
    m_pTxtBallSpeed = NULL;
    m_pTxtPaddleSize = NULL;

    DoModal();
}

void CUserCfgDlg::DoDataExchange(CDataExchange *pDX)
{
    CBmpDialog::DoDataExchange(pDX);
}


void CUserCfgDlg::PutDlgData()
{
    m_pScrollBar1->SetScrollPos(m_nNumBalls);
    m_pScrollBar2->SetScrollPos(m_nStartLevel);
    m_pScrollBar3->SetScrollPos(m_nBallSpeed);
    m_pScrollBar4->SetScrollPos(m_nPaddleSize);

    m_pWallButton->SetCheck(m_bOutterWall);
}


void CUserCfgDlg::GetDlgData()
{
    m_nNumBalls   = m_pScrollBar1->GetScrollPos();
    m_nStartLevel = m_pScrollBar2->GetScrollPos();
    m_nBallSpeed  = m_pScrollBar3->GetScrollPos();
    m_nPaddleSize = m_pScrollBar4->GetScrollPos();

    m_bOutterWall = FALSE;
    if (m_pWallButton->GetCheck() == 1) {
        m_bOutterWall = TRUE;
    }
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

                m_nNumBalls   = BALLS_DEF;
                m_nStartLevel = LEVEL_DEF;
                m_nBallSpeed  = SPEED_DEF;
                m_nPaddleSize = SIZE_DEF;
                m_bOutterWall = FALSE;

                PutDlgData();

                UpdateOptions();
                break;

            case ID_WALLS:
                m_bOutterWall = !m_bOutterWall;
                PutDlgData();
                break;

            default:
                break;
        }
    }

    return(CBmpDialog::OnCommand(wParam, lParam));
}

VOID CUserCfgDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScroll)
{
    CHAR buf[40];
    INT nMin, nMax, nVal;
    CDC *pDC;

    // can't access a null pointer
    assert(pScroll != NULL);
    
    if (pScroll == m_pScrollBar1) {

        nMin = BALLS_MIN;
        nMax = BALLS_MAX;
        nVal = m_nNumBalls;

    } else if (pScroll == m_pScrollBar2) {

        nMin = LEVEL_MIN;
        nMax = LEVEL_MAX;
        nVal = m_nStartLevel;

    } else if (pScroll == m_pScrollBar3) {

        nMin = SPEED_MIN;
        nMax = SPEED_MAX;
        nVal = m_nBallSpeed;

    } else {
        assert(pScroll == m_pScrollBar4);

        nMin = SIZE_MIN;
        nMax = SIZE_MAX;
        nVal = m_nPaddleSize;
    }

    switch (nSBCode) {

        case SB_LEFT:
            nVal = nMin;
            break;

        case SB_PAGELEFT:
            nVal -= PAGE_SIZE;
            break;

        case SB_LINELEFT:
            if (nVal > nMin)
                nVal--;
            break;

        case SB_RIGHT:
            nVal = nMax;
            break;

        case SB_PAGERIGHT:
            nVal += PAGE_SIZE;
            break;

        case SB_LINERIGHT:
            if (nVal < nMax)
                nVal++;
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            nVal = nPos;
            break;

        default:
            break;
    }

    if (nVal < nMin)
        nVal = nMin;
    if (nVal > nMax)
        nVal = nMax;

    pScroll->SetScrollPos(nVal);

    if ((pDC = GetDC()) != NULL) {

        if (pScroll == m_pScrollBar1) {

            m_nNumBalls = nVal;
            if (m_pTxtNumBalls != NULL) {
                wsprintf(buf, "Number of Balls: %d", m_nNumBalls);
                m_pTxtNumBalls->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB( 0, 0, 0));
            }

        } else if (pScroll == m_pScrollBar2) {

            m_nStartLevel = nVal;
            if (m_pTxtStartLevel != NULL) {
                wsprintf(buf, "Starting Level: %d", m_nStartLevel);
                m_pTxtStartLevel->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB( 0, 0, 0));
            }

        } else if (pScroll == m_pScrollBar3) {

            m_nBallSpeed = nVal;
            if (m_pTxtBallSpeed != NULL) {
                wsprintf(buf, "Ball Speed: %d", m_nBallSpeed);
                m_pTxtBallSpeed->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB( 0, 0, 0));
            }

        } else if (pScroll == m_pScrollBar4) {

            m_nPaddleSize = nVal;
            if (m_pTxtPaddleSize != NULL) {
                wsprintf(buf, "Paddle Size: %d", m_nPaddleSize);
                m_pTxtPaddleSize->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB( 0, 0, 0));
            }

        } else {
            assert(0);
        }
        ReleaseDC(pDC);
    }
}


BOOL CUserCfgDlg::OnInitDialog(void)
{
    CRect tmpRect;
    CDC *pDC;

    CBmpDialog::OnInitDialog();

    if ((pDC = GetDC()) != NULL) {

        tmpRect.SetRect(22, 22, 135, 35);
        if ((m_pTxtNumBalls = new CText) != NULL) {
            m_pTxtNumBalls->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
        }

        tmpRect.SetRect(22, 35, 92, 53);
        if ((m_pScrollBar1 = new CScrollBar) != NULL) {
            m_pScrollBar1->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL1);
            m_pScrollBar1->SetScrollRange(BALLS_MIN, BALLS_MAX, TRUE);
        }

        tmpRect.SetRect(22, 57, 135, 70);
        if ((m_pTxtStartLevel = new CText) != NULL) {
            m_pTxtStartLevel->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
        }

        tmpRect.SetRect(22, 70, 92, 88);
        if ((m_pScrollBar2 = new CScrollBar) != NULL) {
            m_pScrollBar2->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL2);
            m_pScrollBar2->SetScrollRange(LEVEL_MIN, LEVEL_MAX, TRUE);
        }

        tmpRect.SetRect(22, 92, 135, 105);
        if ((m_pTxtBallSpeed = new CText) != NULL) {
            m_pTxtBallSpeed->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
        }

        tmpRect.SetRect(22, 105, 92, 123);
        if ((m_pScrollBar3 = new CScrollBar) != NULL) {
            m_pScrollBar3->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL3);
            m_pScrollBar3->SetScrollRange(SPEED_MIN, SPEED_MAX, TRUE);
        }

        tmpRect.SetRect(22, 127, 110, 140);
        if ((m_pTxtPaddleSize = new CText) != NULL) {
            m_pTxtPaddleSize->SetupText(pDC, m_pPalette, &tmpRect, JUSTIFY_LEFT);
        }

        tmpRect.SetRect(22, 140, 92, 158);
        if ((m_pScrollBar4 = new CScrollBar) != NULL) {
            m_pScrollBar4->Create(WS_VISIBLE | WS_CHILD | SBS_HORZ | SBS_BOTTOMALIGN, tmpRect, this, ID_SCROLL4);
            m_pScrollBar4->SetScrollRange(SIZE_MIN, SIZE_MAX, TRUE);
        }

        ReleaseDC(pDC);
    }

    if ((pOKButton = new CColorButton) != NULL) {           // build a color QUIT button to let us exit
        pOKButton->SetPalette(m_pPalette);                  // set the palette to use
        pOKButton->SetControl(IDOK, this);                  // tie to the dialog control
	}
	
    if ((pCancelButton = new CColorButton) != NULL) {       // build a color QUIT button to let us exit
        pCancelButton->SetPalette(m_pPalette);              // set the palette to use
        pCancelButton->SetControl(IDCANCEL, this);          // tie to the dialog control
    }
	
    if ((pDefaultsButton = new CColorButton) != NULL) {     // build a color QUIT button to let us exit
        pDefaultsButton->SetPalette(m_pPalette);            // set the palette to use
        pDefaultsButton->SetControl(ID_RESET, this);        // tie to the dialog control
	}

    if ((m_pWallButton = new CCheckButton) != NULL) {
        m_pWallButton->SetPalette(m_pPalette);
        m_pWallButton->SetControl(ID_WALLS, this);
    }

    m_bSave = FALSE;

    LoadIniSettings();

    PutDlgData();

    return(TRUE);
}

void CUserCfgDlg::OnPaint(void)
{
    CBmpDialog::OnPaint();

    UpdateOptions();
}

VOID CUserCfgDlg::UpdateOptions(VOID)
{
    CHAR buf[40];
    CDC *pDC;

    if ((pDC = GetDC()) != NULL) {

        if (m_pTxtNumBalls != NULL) {
            wsprintf(buf, "Number of Balls: %d", m_nNumBalls);
            m_pTxtNumBalls->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB( 0, 0, 0));
        }

        if (m_pTxtStartLevel != NULL) {
            wsprintf(buf, "Starting Level: %d", m_nStartLevel);
            m_pTxtStartLevel->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB( 0, 0, 0));
        }

        if (m_pTxtBallSpeed != NULL) {
            wsprintf(buf, "Ball Speed: %d", m_nBallSpeed);
            m_pTxtBallSpeed->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB( 0, 0, 0));
        }

        if (m_pTxtPaddleSize != NULL) {
            wsprintf(buf, "Paddle Size: %d", m_nPaddleSize);
            m_pTxtPaddleSize->DisplayString(pDC, buf, 14, TEXT_BOLD, RGB( 0, 0, 0));
        }

        ReleaseDC(pDC);
    }
}
     
BOOL CUserCfgDlg::OnEraseBkgnd(CDC *)
{
    return(TRUE);
}     


void CUserCfgDlg::OnClose()
{
    if (m_bSave) {

        SaveIniSettings();
    }

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

    assert(m_pTxtPaddleSize != NULL);
    if (m_pTxtPaddleSize != NULL) {
        delete m_pTxtPaddleSize;
        m_pTxtPaddleSize = NULL;
    }

    assert(m_pTxtBallSpeed != NULL);
    if (m_pTxtBallSpeed != NULL) {
        delete m_pTxtBallSpeed;
        m_pTxtBallSpeed = NULL;
    }

    assert(m_pTxtStartLevel != NULL);
    if (m_pTxtStartLevel != NULL) {
        delete m_pTxtStartLevel;
        m_pTxtStartLevel = NULL;
    }

    assert(m_pTxtNumBalls != NULL);
    if (m_pTxtNumBalls != NULL) {
        delete m_pTxtNumBalls;
        m_pTxtNumBalls = NULL;
    }

    //
    // de-allocate the scroll bars
    //
    assert(m_pScrollBar4 != NULL);
    if (m_pScrollBar4 != NULL) {
        delete m_pScrollBar4;
        m_pScrollBar4 = NULL;
    }
    assert(m_pScrollBar3 != NULL);
    if (m_pScrollBar3 != NULL) {
        delete m_pScrollBar3;
        m_pScrollBar3 = NULL;
    }
    assert(m_pScrollBar2 != NULL);
    if (m_pScrollBar2 != NULL) {
        delete m_pScrollBar2;
        m_pScrollBar2 = NULL;
    }
    assert(m_pScrollBar1 != NULL);
    if (m_pScrollBar1 != NULL) {
        delete m_pScrollBar1;
        m_pScrollBar1 = NULL;
    }

    if (m_pWallButton != NULL) {
        delete m_pWallButton;
        m_pWallButton = NULL;
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


VOID CUserCfgDlg::LoadIniSettings(VOID)
{
    m_nNumBalls = GetPrivateProfileInt(INI_SECTION, "NumberOfBalls", BALLS_DEF, INI_FILENAME);
    if ((m_nNumBalls < BALLS_MIN) || (m_nNumBalls > BALLS_MAX))
        m_nNumBalls = BALLS_DEF;

    m_nStartLevel = GetPrivateProfileInt(INI_SECTION, "StartingLevel", LEVEL_DEF, INI_FILENAME);
    if ((m_nStartLevel < LEVEL_MIN) || (m_nStartLevel > LEVEL_MAX))
        m_nStartLevel = LEVEL_DEF;

    m_nBallSpeed = GetPrivateProfileInt(INI_SECTION, "BallSpeed", SPEED_DEF, INI_FILENAME);
    if ((m_nBallSpeed < SPEED_MIN) || (m_nBallSpeed > SPEED_MAX))
        m_nBallSpeed = SPEED_DEF;

    m_nPaddleSize = GetPrivateProfileInt(INI_SECTION, "PaddleSize", SIZE_DEF, INI_FILENAME);
    if ((m_nPaddleSize < SIZE_MIN) || (m_nPaddleSize > SIZE_MAX))
        m_nPaddleSize = SIZE_DEF;

    m_bOutterWall = GetPrivateProfileInt(INI_SECTION, "OutterWall", 0, INI_FILENAME);
    if (m_bOutterWall != 0)
        m_bOutterWall = TRUE;
}

VOID CUserCfgDlg::SaveIniSettings(VOID)
{
    CHAR buf[40];

    WritePrivateProfileString(INI_SECTION, "NumberOfBalls", itoa(m_nNumBalls, buf, 10), INI_FILENAME);
    WritePrivateProfileString(INI_SECTION, "StartingLevel", itoa(m_nStartLevel, buf, 10), INI_FILENAME);
    WritePrivateProfileString(INI_SECTION, "BallSpeed", itoa(m_nBallSpeed, buf, 10), INI_FILENAME);
    WritePrivateProfileString(INI_SECTION, "PaddleSize", itoa(m_nPaddleSize, buf, 10), INI_FILENAME);
    WritePrivateProfileString(INI_SECTION, "OutterWall", itoa(m_bOutterWall ? 1 : 0, buf, 10), INI_FILENAME);
}

BEGIN_MESSAGE_MAP(CUserCfgDlg, CBmpDialog)
    ON_WM_CLOSE()    
    ON_WM_ERASEBKGND()
    ON_WM_HSCROLL()
    ON_WM_PAINT()
END_MESSAGE_MAP()
