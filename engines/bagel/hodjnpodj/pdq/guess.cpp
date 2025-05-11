
#include <afxwin.h>
#include <afxext.h>
#include <bitmaps.h>
#include <assert.h>
#include <cbofdlg.h>
#include "main.h"
#include "guess.h"
#include "game.h"

#define IDC_EDIT 101

/////////////////////////////////////////////////////////////////////////////
// CGuessDlg dialog

CGuessDlg::CGuessDlg(CWnd *pParent, CPalette *pPalette)
         : CBmpDialog(pParent, pPalette, IDD_GUESS_DLG, ".\\ART\\GUESS.BMP", 36, 48)
{
}

void CGuessDlg::DoDataExchange(CDataExchange* pDX)
{
    CBmpDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT, text);
}

BOOL CGuessDlg::OnInitDialog(void)
{
    CEdit *pEdit;

    CBmpDialog::OnInitDialog();

    pEdit =  (CEdit *)GetDlgItem(IDC_EDIT);
    pEdit->LimitText(MAX_PLENGTH_S + 1);

    SetTimer(10, 50, NULL);

    return(TRUE);
}

void CGuessDlg::OnTimer(UINT nEventID)
{
    char buf[MAX_PLENGTH_S + 2];
    CEdit *pEdit;

    KillTimer(nEventID);

    pEdit = (CEdit *)GetDlgItem(IDC_EDIT);
    pEdit->SetFocus();
    pEdit->GetWindowText(buf, MAX_PLENGTH + 1);
    pEdit->SetSel(-1, strlen(buf), TRUE);
}

void CGuessDlg::OnPaint()
{
    CBmpDialog::OnPaint();
}

void CGuessDlg::OnSetFocus(CWnd *)
{
    SetTimer(10, 50, NULL);
}

void CGuessDlg::OnActivate(UINT nState, CWnd *, BOOL bMinimized)
{
    CEdit *pEdit;

    pEdit = (CEdit *)GetDlgItem(IDC_EDIT);

    if (!bMinimized) {
        switch (nState) {

            case WA_ACTIVE:
            case WA_CLICKACTIVE:
                pEdit->SetSel(-1, 0, TRUE);
                InvalidateRect(NULL, FALSE);
                break;
        }
    }
}

BOOL CGuessDlg::OnEraseBkgnd(CDC *)
{
    return(TRUE);
}

BEGIN_MESSAGE_MAP(CGuessDlg, CBmpDialog)
    ON_WM_TIMER()
    ON_WM_PAINT()
    ON_WM_ACTIVATE()
    ON_WM_SETFOCUS()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()
