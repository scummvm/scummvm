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

#include <afxwin.h>
#include <afxext.h>
#include <assert.h>
#include <cbofdlg.h>
#include <text.h>
#include <button.h>
#include "c2btndlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define TEXT_COLOR          RGB(0, 0, 0)    // displayed text color

#define MESSAGE_COL         16              // first message box positioning
#define MESSAGE_ROW         40

#define MESSAGE_WIDTH       186             // standard message box dims
#define MESSAGE_HEIGHT      20

#define MESSAGE2_ROW_OFFSET ( MESSAGE_HEIGHT + 4 )  // Row offset from first 
                                                    // message box

#define IDD_2BTNDLG 208


C2ButtonDialog::C2ButtonDialog(CWnd *pParent, CPalette *pPalette, char *pszButton1Text, char *pszButton2Text, char *pszText1, char *pszText2, char *pszText3)
         : CBmpDialog(pParent, pPalette, IDD_2BTNDLG, ".\\ART\\MSSCROLL.BMP")
{
    assert(pParent != NULL);
    assert(pPalette != NULL);
    assert(pszText1 != NULL);
    assert(pszButton1Text != NULL);
    assert(pszButton2Text != NULL);

    // Initialize all members
    //
    m_pPalette = pPalette;
    m_pszMessage1 = pszText1;
    m_pszMessage2 = pszText2;
    m_pszMessage3 = pszText3;
    m_pszButton1Text = pszButton1Text;
    m_pszButton2Text = pszButton2Text;

    m_cTextMessage1 = NULL;
    m_cTextMessage2 = NULL;
    m_cTextMessage3 = NULL;

    //{{AFX_DATA_INIT(C2ButtonDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void C2ButtonDialog::ClearDialogImage()
{
    if (m_pButton1 != NULL) {                          // release the 1st button
        delete m_pButton1;
        m_pButton1 = NULL;
    }
    if (m_pButton2 != NULL) {                          // release the 2nd button
        delete m_pButton2;
        m_pButton2 = NULL;
    }

    ValidateRect(NULL);
}

void C2ButtonDialog::OnDestroy()
{
    if (m_cTextMessage1 != NULL) {
        delete m_cTextMessage1;
        m_cTextMessage1 = NULL;
    }

    if (m_cTextMessage2 != NULL) {
        delete m_cTextMessage2;
        m_cTextMessage2 = NULL;
    }

    if (m_cTextMessage3 != NULL) {
        delete m_cTextMessage3;
        m_cTextMessage3 = NULL;
    }

    CBmpDialog::OnDestroy();
}

void C2ButtonDialog::DoDataExchange(CDataExchange* pDX)
{
    CBmpDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(C2ButtonDialog)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(C2ButtonDialog, CBmpDialog)
    //{{AFX_MSG_MAP(C2ButtonDialog)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// C2ButtonDialog message handlers

BOOL C2ButtonDialog::OnInitDialog()
{
    CRect   statsRect;                  // game stats displays
    int     nStat_col_offset;           // game stats placement
    int     nStat_row_offset;
    int     nStatWidth, nStatHeight;
    BOOL    bAssertCheck;
    CDC     *pDC;

    CBmpDialog::OnInitDialog();

    pDC = GetDC();

    nStat_col_offset    = MESSAGE_COL;
    nStat_row_offset    = MESSAGE_ROW;
    nStatWidth          = MESSAGE_WIDTH;
    nStatHeight         = MESSAGE_HEIGHT;
    statsRect.SetRect   (
                         nStat_col_offset,
                         nStat_row_offset,
                         nStat_col_offset + nStatWidth,
                         nStat_row_offset + nStatHeight
                        );

    if ((m_cTextMessage1 = new CText()) != NULL) {
        bAssertCheck = (*m_cTextMessage1).SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
        ASSERT(bAssertCheck);   // initialize the text objext
    }

    nStat_row_offset += MESSAGE2_ROW_OFFSET;
    statsRect.SetRect(   nStat_col_offset,
                         nStat_row_offset,
                         nStat_col_offset + nStatWidth,
                         nStat_row_offset + nStatHeight
                     );

    if ((m_cTextMessage2 = new CText()) != NULL) {
        bAssertCheck = (*m_cTextMessage2).SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
        ASSERT(bAssertCheck);   // initialize the text objext
    }

    nStat_row_offset += MESSAGE2_ROW_OFFSET;
    statsRect.SetRect(   nStat_col_offset,
                         nStat_row_offset,
                         nStat_col_offset + nStatWidth,
                         nStat_row_offset + nStatHeight
                     );

    if ((m_cTextMessage3 = new CText()) != NULL) {
        bAssertCheck = m_cTextMessage3->SetupText(pDC, m_pPalette, &statsRect, JUSTIFY_CENTER);
        ASSERT(bAssertCheck);   // initialize the text objext
    }

    ReleaseDC(pDC);

    if ((m_pButton1 = new CColorButton) != NULL) {            // build the first color button
        (*m_pButton1).SetPalette(m_pPalette);                 // set the palette to use
        (*m_pButton1).SetControl(IDOK, this);                 // tie to the dialog control
        (*m_pButton1).SetWindowText(m_pszButton1Text);
    }
    if ((m_pButton2 = new CColorButton) != NULL) {             // build the second color button
        (*m_pButton2).SetPalette(m_pPalette);                  // set the palette to use
        (*m_pButton2).SetControl(IDCANCEL, this);              // tie to the dialog control
        (*m_pButton2).SetWindowText(m_pszButton2Text);
    }
    
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void C2ButtonDialog::OnPaint()
{
    // TODO: Add your message handler code here
    CDC     *pDC;
    BOOL    bAssertCheck;

    CBmpDialog::OnPaint();
    // Do not call CDialog::OnPaint() for painting messages

    pDC = GetDC();

    if ( m_pszMessage1 != NULL ) {
        bAssertCheck = m_cTextMessage1->DisplayString(pDC, m_pszMessage1, 21, FW_BOLD, TEXT_COLOR);
        ASSERT(bAssertCheck);
    }

    if ( m_pszMessage2 != NULL ) {
        bAssertCheck = m_cTextMessage2->DisplayString(pDC, m_pszMessage2, 21, FW_BOLD, TEXT_COLOR);
        ASSERT(bAssertCheck);
    }

    if ( m_pszMessage3 != NULL ) {
        bAssertCheck = m_cTextMessage3->DisplayString(pDC, m_pszMessage3, 21, FW_BOLD, TEXT_COLOR);
        ASSERT(bAssertCheck);
    }

    ReleaseDC(pDC);
}

void C2ButtonDialog::OnOK()
{
    ClearDialogImage();
    EndDialog(IDOK);
}

void C2ButtonDialog::OnCancel()
{
    ClearDialogImage();
    EndDialog(IDCANCEL);
}

BOOL C2ButtonDialog::OnEraseBkgnd(CDC *)
{
    // Prevents refreshing of background
    return(TRUE);
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
