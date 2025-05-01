/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  restdlg.cpp
*
*  HISTORY
*
*       1.00      09/15/94     BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*       [Describe the function of the module]
*
*  CONSTRUCTORS:
*
*       [list constructors with one line descriptions]
*
*  DESTRUCTORS:
*
*       [list destructors with one line descriptions]
*
*  PUBLIC:
*
*       [list public routines with one line descriptions]
*
*  PUBLIC GLOBAL:
*
*       [list global public routines with one line descriptions]
*
*  PROTECTED:
*
*       [list protected routines with one line descriptions]
*
*  PRIVATE:
*
*       [list private routines with one line descriptions]
*
*  MEMBERS:
*
*       [list members of the class with one line descriptions]
*
****************************************************************/
#include <afxwin.h>
#include <afxext.h>
#include <assert.h>
#include <stdinc.h>

#include "restdlg.h"
#include "resource.h"

STATIC const CHAR *gpszTitle = "Restore Game";

CRestoreDlg::CRestoreDlg(CHAR *pszDescriptions[], CWnd *pWnd, CPalette *pPalette)
            :CBmpDialog(pWnd, pPalette, IDD_RESTORE, ".\\ART\\MLSCROLL.BMP", -1, -1, TRUE)
{
    INT i;

    // Inits
    m_pTxtRestore = NULL;

    m_pQuitButton = NULL;
    for (i = 0; i < MAX_SAVEGAMES; i++) {
        m_pSlotButtons[i] = NULL;
        m_pszDescriptions[i] = pszDescriptions[i];
    }

    // the order if these IDs matter
    //
    assert(IDC_RSLOT1 + 1 == IDC_RSLOT2);
    assert(IDC_RSLOT1 + 2 == IDC_RSLOT3);
    assert(IDC_RSLOT1 + 3 == IDC_RSLOT4);
    assert(IDC_RSLOT1 + 4 == IDC_RSLOT5);
    assert(IDC_RSLOT1 + 5 == IDC_RSLOT6);
}


BOOL CRestoreDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (HIWORD(lParam) == BN_CLICKED) {

        switch (wParam) {

            case IDC_RSLOT1:
            case IDC_RSLOT2:
            case IDC_RSLOT3:
            case IDC_RSLOT4:
            case IDC_RSLOT5:
            case IDC_RSLOT6:
                ClearDialogImage();
                EndDialog((int)wParam - IDC_RSLOT1);
                return(TRUE);

            case IDCANCEL:
                ClearDialogImage();
                EndDialog(-1);
                return(TRUE);
        }
    }
    return(CBmpDialog::OnCommand(wParam, lParam));
}


void CRestoreDlg::OnCancel(void)
{
    ClearDialogImage();
    EndDialog(-1);
}


BOOL CRestoreDlg::OnInitDialog(void)
{
    CRect rect;
    CDC *pDC;
    INT i;
    BOOL bSuccess;

    CBmpDialog::OnInitDialog();            // do basic dialog initialization

    // create the buttons (each restore-game-slot is a button)
    //
    for (i = 0; i < MAX_SAVEGAMES; i++) {

        if ((m_pSlotButtons[i] = new CColorButton()) != NULL) {
            m_pSlotButtons[i]->SetPalette( m_pPalette );
            bSuccess = m_pSlotButtons[i]->SetControl(IDC_RSLOT1 + i, this);
            assert(bSuccess);
        }

        // if a slot is empty
        //
        if (m_pszDescriptions[i] == NULL) {

            m_pSlotButtons[i]->SetWindowText("empty");

            // then disable the button
            //
            m_pSlotButtons[i]->EnableWindow(FALSE);

        } else {
            m_pSlotButtons[i]->SetWindowText(m_pszDescriptions[i]);
        }
    }

    if ((m_pQuitButton = new CColorButton()) != NULL) {
        m_pQuitButton->SetPalette( m_pPalette );
        bSuccess = m_pQuitButton->SetControl(IDCANCEL, this);
        m_pQuitButton->SetFocus();
        assert(bSuccess);
    }

    if ((m_pTxtRestore = new CText) != NULL) {
        rect.SetRect(193, 40, 365, 80);
        pDC = GetDC();
        m_pTxtRestore->SetupText(pDC, m_pPalette, &rect, JUSTIFY_LEFT);
        ReleaseDC(pDC);
    }

    return(FALSE);
}


void CRestoreDlg::OnPaint(void)
{
    CDC *pDC;

    CBmpDialog::OnPaint();

    if (m_pTxtRestore != NULL) {
        pDC = GetDC();
        m_pTxtRestore->DisplayString(pDC, gpszTitle, 25, FW_BOLD, RGB( 0, 0, 0));
        ReleaseDC(pDC);
    }
}


void CRestoreDlg::ClearDialogImage(void)
{
    INT i;
    if (m_pTxtRestore != NULL) {
        delete m_pTxtRestore;
        m_pTxtRestore = NULL;
    }

    if (m_pQuitButton != NULL) {
        delete m_pQuitButton;
        m_pQuitButton = NULL;
    }

    for (i = 0; i < MAX_SAVEGAMES; i++) {
        if (m_pSlotButtons[i] != NULL ) {
            delete m_pSlotButtons[i];
            m_pSlotButtons[i] = NULL;
        }
    }
}


void CRestoreDlg::OnDestroy(void)
{
    INT i;

    if (m_pTxtRestore != NULL) {
        delete m_pTxtRestore;
        m_pTxtRestore = NULL;
    }

    if (m_pQuitButton != NULL) {
        delete m_pQuitButton;
        m_pQuitButton = NULL;
    }

    for (i = 0; i < MAX_SAVEGAMES; i++) {
        if (m_pSlotButtons[i] != NULL ) {
            delete m_pSlotButtons[i];
            m_pSlotButtons[i] = NULL;
        }
    }

    CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CRestoreDlg, CBmpDialog)
    //{{AFX_MSG_MAP(CRestoreDlg)
    ON_WM_PAINT()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
