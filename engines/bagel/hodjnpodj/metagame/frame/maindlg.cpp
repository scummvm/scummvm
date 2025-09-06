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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/metagame/frame/resource.h"
#include "bagel/hodjnpodj/metagame/frame/dialogs.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/boflib/misc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Frame {

extern  bool    bMetaLoaded;

extern const char *gpszSaveGameFile;

/*****************************************************************
 *
 * CMainGameDlg
 *
 * FUNCTIONAL DESCRIPTION:
 *
 *      Constructor sends the input to the CBmpDialog constructor and
 *      the intializes the private members
 *
 * FORMAL PARAMETERS:
 *
 *      Those needed to contruct a CBmpDialog dialog: pParent,pPalette, nID
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      private member m_nCurrenLEVEL
 *  globals     rectDisplayAmount and pSeLEVELPalette
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
CMainGameDlg::CMainGameDlg(CWnd *pParent, CPalette *pPalette)
	: CBmpDialog(pParent, pPalette, IDD_MAIN_SCROLL, ".\\ART\\MLSCROLL.BMP", -1, -1, false) {
	// Inits
	m_pPlayMetaButton = nullptr;
	m_pPlayMiniButton = nullptr;
	m_pRestoreButton = nullptr;
	m_pGrandTourButton = nullptr;
	m_pRestartMovieButton = nullptr;
	m_pQuitButton = nullptr;
}

/*****************************************************************
 *
 * OnCommand
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Process the "Set" and "Cancel" buttons
 *
 * This function is called when a WM_COMMAND message is issued,
 * typically in order to process control related activities.
 *
 * FORMAL PARAMETERS:
 *
 *      wParam          identifier for the button to be processed
 *      lParam          type of message to be processed
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
bool CMainGameDlg::OnCommand(WPARAM wParam, LPARAM lParam) {

// What ever button is clicked, end the dialog and send the ID of the button
// clicked as the return from the dialog
	if (HIWORD(lParam) == BN_CLICKED) {
		switch (wParam) {
		case IDC_PLAY_META:
		case IDC_PLAY_MINI:
		case IDC_RESTORE_GAME:
		case IDC_GRAND_TOUR:
		case IDC_RESTART_MOVIE:
		case IDC_QUIT_GAME:
			ClearDialogImage();
			EndDialog((int)wParam);
			return true;
		}
	}
	return CBmpDialog::OnCommand(wParam, lParam);
}

void CMainGameDlg::OnCancel() {
	ClearDialogImage();
//  CBmpDialog::OnCancel();
	EndDialog(0);
	return;
}

void CMainGameDlg::OnOK() {
	if (m_pPlayMetaButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_PLAY_META, (LPARAM)BN_CLICKED);
		return;
	}
	if (m_pPlayMiniButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_PLAY_MINI, (LPARAM)BN_CLICKED);
		return;
	}
	if (m_pRestoreButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_RESTORE_GAME, (LPARAM)BN_CLICKED);
		return;
	}
	if (m_pGrandTourButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_GRAND_TOUR, (LPARAM)BN_CLICKED);
		return;
	}
	if (m_pRestartMovieButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_RESTART_MOVIE, (LPARAM)BN_CLICKED);
		return;
	}
	if (m_pQuitButton->GetState() & 0x0008) {
		SendMessage(WM_COMMAND, IDC_QUIT_GAME, (LPARAM)BN_CLICKED);
		return;
	}
	return;
}

void CMainGameDlg::ClearDialogImage() {
	/*
	    if ( m_pPlayMetaButton != nullptr ) {
	        delete m_pPlayMetaButton;
	        m_pPlayMetaButton = nullptr;
	    }

	    if ( m_pPlayMiniButton != nullptr ) {
	        delete m_pPlayMiniButton;
	        m_pPlayMiniButton = nullptr;
	    }

	    if ( m_pRestoreButton != nullptr ) {
	        delete m_pRestoreButton;
	        m_pRestoreButton = nullptr;
	    }

	    if ( m_pGrandTourButton != nullptr ) {
	        delete m_pGrandTourButton;
	        m_pGrandTourButton = nullptr;
	    }

	    if ( m_pRestartMovieButton != nullptr ) {
	        delete m_pRestartMovieButton;
	        m_pRestartMovieButton = nullptr;
	    }

	    if ( m_pQuitButton != nullptr ) {
	        delete m_pQuitButton;
	        m_pQuitButton = nullptr;
	    }

	    ValidateRect(nullptr);
	*/
}


bool CMainGameDlg::OnInitDialog() {
	bool    bSuccess;

	CBmpDialog::OnInitDialog();

	#ifndef BMP_BUTTONS

	m_pPlayMetaButton = new CColorButton();
	ASSERT(m_pPlayMetaButton != nullptr);
	m_pPlayMetaButton->SetPalette(m_pPalette);
	bSuccess = m_pPlayMetaButton->SetControl(IDC_PLAY_META, this);
	ASSERT(bSuccess);


	m_pPlayMiniButton = new CColorButton();
	ASSERT(m_pPlayMiniButton != nullptr);
	m_pPlayMiniButton->SetPalette(m_pPalette);
	bSuccess = m_pPlayMiniButton->SetControl(IDC_PLAY_MINI, this);
	ASSERT(bSuccess);

	m_pRestoreButton = new CColorButton();
	ASSERT(m_pRestoreButton != nullptr);
	m_pRestoreButton->SetPalette(m_pPalette);
	bSuccess = m_pRestoreButton->SetControl(IDC_RESTORE_GAME, this);
	if (g_engine->listSaves().empty())
		m_pRestoreButton->EnableWindow(false);
	ASSERT(bSuccess);

	m_pGrandTourButton = new CColorButton();
	ASSERT(m_pGrandTourButton != nullptr);
	m_pGrandTourButton->SetPalette(m_pPalette);
	bSuccess = m_pGrandTourButton->SetControl(IDC_GRAND_TOUR, this);
	ASSERT(bSuccess);

	m_pRestartMovieButton = new CColorButton();
	ASSERT(m_pRestartMovieButton != nullptr);
	m_pRestartMovieButton->SetPalette(m_pPalette);
	bSuccess = m_pRestartMovieButton->SetControl(IDC_RESTART_MOVIE, this);
	ASSERT(bSuccess);

	m_pQuitButton = new CColorButton();
	ASSERT(m_pQuitButton != nullptr);
	m_pQuitButton->SetPalette(m_pPalette);
	bSuccess = m_pQuitButton->SetControl(IDC_QUIT_GAME, this);
	ASSERT(bSuccess);

	#else
	pDC = GetDC();

	if (bDestroyBmp)
		m_pButtonsBmp = FetchBitmap(pDC, nullptr, BUTTONSBMP);

	pUp = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDn = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pFcs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;

	m_pPlayMetaButton = new CBmpButton();
	ASSERT(m_pPlayMetaButton != nullptr);
	bSuccess = m_pPlayMetaButton->LoadBitmaps(m_pButtonPalette, pUp, pDn, pFcs, pDs);
	ASSERT(bSuccess);
	bSuccess = m_pPlayMetaButton->SetControl(IDC_PLAY_META, this);
	ASSERT(bSuccess);

	pUp = pDn = pFcs = pDs = nullptr;
	nLeft += 270;
	nTop = 0;
	pUp = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDn = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pFcs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;

	m_pRestoreButton = new CBmpButton();
	ASSERT(m_pRestoreButton != nullptr);
	bSuccess = m_pRestoreButton->LoadBitmaps(m_pButtonPalette, pUp, pDn, pFcs, pDs);
	ASSERT(bSuccess);
	bSuccess = m_pRestoreButton->SetControl(IDC_RESTORE_GAME, this);
	if (!FileExists(gpszSaveGameFile)) {
		m_pRestoreButton->EnableWindow(false);
	}
	ASSERT(bSuccess);

	pUp = pDn = pFcs = pDs = nullptr;
	nLeft += 270;
	nTop = 0;
	pUp = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDn = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pFcs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;

	m_pPlayMiniButton = new CBmpButton();
	ASSERT(m_pPlayMiniButton != nullptr);
	bSuccess = m_pPlayMiniButton->LoadBitmaps(m_pButtonPalette, pUp, pDn, pFcs, pDs);
	ASSERT(bSuccess);
	bSuccess = m_pPlayMiniButton->SetControl(IDC_PLAY_MINI, this);
	ASSERT(bSuccess);

	pUp = pDn = pFcs = pDs = nullptr;
	nLeft += 270;
	nTop = 0;
	pUp = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDn = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pFcs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;

	m_pGrandTourButton = new CBmpButton();
	ASSERT(m_pGrandTourButton != nullptr);
	bSuccess = m_pGrandTourButton->LoadBitmaps(m_pButtonPalette, pUp, pDn, pFcs, pDs);
	ASSERT(bSuccess);
	bSuccess = m_pGrandTourButton->SetControl(IDC_GRAND_TOUR, this);
	ASSERT(bSuccess);

	pUp = pDn = pFcs = pDs = nullptr;
	nLeft += 270;
	nTop = 0;
	pUp = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDn = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pFcs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;

	m_pRestartMovieButton = new CBmpButton();
	ASSERT(m_pRestartMovieButton != nullptr);
	bSuccess = m_pRestartMovieButton->LoadBitmaps(m_pButtonPalette, pUp, pDn, pFcs, pDs);
	ASSERT(bSuccess);
	bSuccess = m_pRestartMovieButton->SetControl(IDC_RESTART_MOVIE, this);
	ASSERT(bSuccess);

	pUp = pDn = pFcs = pDs = nullptr;
	nLeft += 270;
	nTop = 0;
	pUp = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDn = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pFcs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;
	pDs = ExtractBitmap(pDC, m_pButtonsBmp, m_pButtonPalette, nLeft, nTop, nWidth, nHeight);
	nTop += 34;

	m_pQuitButton = new CBmpButton();
	ASSERT(m_pQuitButton != nullptr);
	bSuccess = m_pQuitButton->LoadBitmaps(m_pButtonPalette, pUp, pDn, pFcs, pDs);
	ASSERT(bSuccess);
	bSuccess = m_pQuitButton->SetControl(IDC_QUIT_GAME, this);
	ASSERT(bSuccess);

	ReleaseDC(pDC);

	#endif

	return true;
}

/*****************************************************************
 *
 * OnPaint
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * Repaint the screen whenever needed; e.g. when uncovered by an
 * overlapping window, when maximized from an icon, and when it the
 * window is initially created.
 *
 * This uses the CBmpDialog Paint as its base, and displays the current
 * amount chosen from the scrollbar
 *
 * This routine is called whenever Windows sends a WM_PAINT message.
 * Note that creating a CPaintDC automatically does a BeginPaint and
 * an EndPaint call is done when it is destroyed at the end of this
 * function.  CPaintDC's constructor needs the window (this).
 *
 * FORMAL PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT INPUT PARAMETERS:
 *
 *      n/a
 *
 * IMPLICIT OUTPUT PARAMETERS:
 *
 *      n/a
 *
 * RETURN VALUE:
 *
 *      n/a
 *
 ****************************************************************/
void CMainGameDlg::OnPaint() {

//      call CBmpDialog onpaint, to paint the background
	CBmpDialog::OnPaint();

	return;
}

void CMainGameDlg::OnDestroy() {
	if (m_pPlayMetaButton != nullptr) {
		delete m_pPlayMetaButton;
		m_pPlayMetaButton = nullptr;
	}

	if (m_pPlayMiniButton != nullptr) {
		delete m_pPlayMiniButton;
		m_pPlayMiniButton = nullptr;
	}

	if (m_pRestoreButton != nullptr) {
		delete m_pRestoreButton;
		m_pRestoreButton = nullptr;
	}

	if (m_pGrandTourButton != nullptr) {
		delete m_pGrandTourButton;
		m_pGrandTourButton = nullptr;
	}

	if (m_pRestartMovieButton != nullptr) {
		delete m_pRestartMovieButton;
		m_pRestartMovieButton = nullptr;
	}

	if (m_pQuitButton != nullptr) {
		delete m_pQuitButton;
		m_pQuitButton = nullptr;
	}

	CBmpDialog::OnDestroy();
}

// Message Map
BEGIN_MESSAGE_MAP(CMainGameDlg, CBmpDialog)
	//{{AFX_MSG_MAP( CMainPokerWindow )
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

} // namespace Frame
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
