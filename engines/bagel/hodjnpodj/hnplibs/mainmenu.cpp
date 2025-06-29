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
#include "bagel/hodjnpodj/hnplibs/dibapi.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/audiocfg.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {

BEGIN_MESSAGE_MAP(CMainMenu, CBmpDialog)
	//{{AFX_MSG_MAP(CMainMenu)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_OPTIONS_RULES, CMainMenu::OnClickedRules)
	ON_BN_CLICKED(IDC_OPTIONS_NEWGAME, CMainMenu::OnClickedNewgame)
	ON_BN_CLICKED(IDC_OPTIONS_OPTIONS, CMainMenu::OnClickedOptions)
	ON_BN_CLICKED(IDC_OPTIONS_AUDIO, CMainMenu::OnClickedAudio)
	ON_BN_CLICKED(IDC_OPTIONS_RETURN, CMainMenu::OnClickedReturn)
	ON_BN_CLICKED(IDC_OPTIONS_QUIT, CMainMenu::OnClickedQuit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMainMenu::CMainMenu(CWnd *pParent, CPalette *pPalette,
                     UINT nFlags, FPFUNC pOptionsFunc, const char *pRulesFileName,
                     const char *pWavFileName, LPGAMESTRUCT pGameParams)
	: CBmpDialog(pParent, pPalette, IDD_OPTIONS_DIALOG, ".\\ART\\OSCROLL.BMP") {
	// Can't access null pointers
	assert(pParent != nullptr);
	assert(pPalette != nullptr);
	assert(pRulesFileName != nullptr);

	// Inits
	_rulesFilename = pRulesFileName;
	_wavFilename = pWavFileName;
	_flags = nFlags;
	_gameParams = pGameParams;

	if (!(_flags & NO_OPTIONS)) {
		assert(pOptionsFunc != nullptr);
	}
	_optionsFunction = pOptionsFunc;

	//{{AFX_DATA_INIT(CMainMenu)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMainMenu::DoDataExchange(CDataExchange* pDX) {
	CBmpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainMenu)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


void CMainMenu::ClearDialogImage(void) {
	if (_rulesButton != nullptr) {                          // release the button
		delete _rulesButton;
		_rulesButton = nullptr;
	}

	if (_newGameButton != nullptr) {                          // release the button
		delete _newGameButton;
		_newGameButton = nullptr;
	}

	if (_optionsButton != nullptr) {                          // release the button
		delete _optionsButton;
		_optionsButton = nullptr;
	}

	if (_audioButton != nullptr) {                          // release the button
		delete _audioButton;
		_audioButton = nullptr;
	}

	if (_returnButton != nullptr) {                          // release the button
		delete _returnButton;
		_returnButton = nullptr;
	}

	if (_quitButton != nullptr) {                          // release the button
		delete _quitButton;
		_quitButton = nullptr;
	}

	if (m_pDlgBackground != nullptr)
		InvalidateRect(nullptr, FALSE);
}

void CMainMenu::OnDestroy() {
	CBmpDialog::OnDestroy();

	if (_rulesButton != nullptr) {                          // release the button
		delete _rulesButton;
		_rulesButton = nullptr;
	}

	if (_newGameButton != nullptr) {                          // release the button
		delete _newGameButton;
		_newGameButton = nullptr;
	}

	if (_optionsButton != nullptr) {                          // release the button
		delete _optionsButton;
		_optionsButton = nullptr;
	}

	if (_audioButton != nullptr) {                          // release the button
		delete _audioButton;
		_audioButton = nullptr;
	}

	if (_returnButton != nullptr) {                          // release the button
		delete _returnButton;
		_returnButton = nullptr;
	}

	if (_quitButton != nullptr) {                          // release the button
		delete _quitButton;
		_quitButton = nullptr;
	}

}

BOOL CMainMenu::OnInitDialog() {
	CWnd *pWndTemp;

	CBmpDialog::OnInitDialog();            // do basic dialog initialization

	if ((_rulesButton = new CColorButton) != nullptr) {                    // build a color QUIT button to let us exit
		(*_rulesButton).SetPalette(m_pPalette);                     // set the palette to use
		(*_rulesButton).SetControl(IDC_OPTIONS_RULES, this);            // tie to the dialog control
	}

	if ((_newGameButton = new CColorButton) != nullptr) {                  // build a color QUIT button to let us exit
		(*_newGameButton).SetPalette(m_pPalette);                       // set the palette to use
		(*_newGameButton).SetControl(IDC_OPTIONS_NEWGAME, this);            // tie to the dialog control
	}

	if ((_optionsButton = new CColorButton) != nullptr) {                  // build a color QUIT button to let us exit
		(*_optionsButton).SetPalette(m_pPalette);                       // set the palette to use
		(*_optionsButton).SetControl(IDC_OPTIONS_OPTIONS, this);            // tie to the dialog control
	}

	if ((_audioButton = new CColorButton) != nullptr) {                    // build a color QUIT button to let us exit
		(*_audioButton).SetPalette(m_pPalette);                     // set the palette to use
		(*_audioButton).SetControl(IDC_OPTIONS_AUDIO, this);            // tie to the dialog control
	}

	if ((_returnButton = new CColorButton) != nullptr) {                   // build a color QUIT button to let us exit
		(*_returnButton).SetPalette(m_pPalette);                        // set the palette to use
		(*_returnButton).SetControl(IDC_OPTIONS_RETURN, this);              // tie to the dialog control
	}

	if ((_quitButton = new CColorButton) != nullptr) {                 // build a color QUIT button to let us exit
		(*_quitButton).SetPalette(m_pPalette);                      // set the palette to use
		(*_quitButton).SetControl(IDC_OPTIONS_QUIT, this);              // tie to the dialog control
	}
	// Disable the Rules button if told to do so
	//
	if (_flags & NO_RULES) {

		pWndTemp = GetDlgItem(IDC_OPTIONS_RULES);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(FALSE);
	}

	// Disable the NewGame button if told to do so
	//
	if (_flags & NO_NEWGAME) {

		pWndTemp = GetDlgItem(IDC_OPTIONS_NEWGAME);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(FALSE);
	}

	// Disable the Options button if told to do so
	//
	if (_flags & NO_OPTIONS) {

		pWndTemp = GetDlgItem(IDC_OPTIONS_OPTIONS);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(FALSE);
	}

	// Disable the Audio button if told to do so
	//
	if (_flags & NO_AUDIO) {

		pWndTemp = GetDlgItem(IDC_OPTIONS_AUDIO);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(FALSE);
	}

	// Disable the Return button if told to do so
	//
	if (_flags & NO_RETURN) {

		pWndTemp = GetDlgItem(IDC_OPTIONS_RETURN);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(FALSE);
	}

	// Disable the Return button if told to do so
	//
	if (_flags & NO_QUIT) {

		pWndTemp = GetDlgItem(IDC_OPTIONS_QUIT);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(FALSE);
	}

	return (TRUE);  // return TRUE  unless focused on a control
}


BOOL CMainMenu::OnEraseBkgnd(CDC *pDC) {
	return (TRUE);
}


void CMainMenu::OnPaint() {
	CBmpDialog::OnPaint();
}


void CMainMenu::OnClickedRules() {
	CWnd *pControl;

	// Load the rules
	CRules  RulesDlg(this, _rulesFilename, m_pPalette, _wavFilename);

	// display the rules
	RulesDlg.DoModal();

	SetDefID(IDC_OPTIONS_OPTIONS);
	if (_flags & NO_RETURN) {
		SetDefID(IDC_OPTIONS_NEWGAME);
		pControl = GetDlgItem(IDC_OPTIONS_NEWGAME);
	} else {
		SetDefID(IDC_OPTIONS_RETURN);
		pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	}
	GotoDlgCtrl(pControl);
}

void CMainMenu::OnClickedNewgame() {
	// user has chosen to start a new game
	//
	ClearDialogImage();
	EndDialog(IDC_OPTIONS_NEWGAME);
}

void CMainMenu::OnClickedOptions() {
	CWnd *pControl;

	SetDefID(IDC_OPTIONS_RULES);
	if (_flags & NO_RETURN) {
		SetDefID(IDC_OPTIONS_NEWGAME);
		pControl = GetDlgItem(IDC_OPTIONS_NEWGAME);
	} else {
		SetDefID(IDC_OPTIONS_RETURN);
		pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	}
	GotoDlgCtrl(pControl);

	// call the user defined sub-options (we are the parent)
	//
	if (_optionsFunction != nullptr)
		(_optionsFunction)(this);
}

void CMainMenu::OnClickedAudio() {
	CWnd *pControl;

	SetDefID(IDC_OPTIONS_RULES);
	if (_flags & NO_RETURN) {
		SetDefID(IDC_OPTIONS_NEWGAME);
		pControl = GetDlgItem(IDC_OPTIONS_NEWGAME);
	} else {
		SetDefID(IDC_OPTIONS_RETURN);
		pControl = GetDlgItem(IDC_OPTIONS_RETURN);
	}
	GotoDlgCtrl(pControl);

	CAudioCfgDlg dlgAudioCfg(this, m_pPalette, IDD_AUDIOCFG);

	if (_gameParams != nullptr) {
		_gameParams->bMusicEnabled = GetPrivateProfileInt("Meta", "Music", TRUE, "HODJPODJ.INI");
		_gameParams->bSoundEffectsEnabled = GetPrivateProfileInt("Meta", "SoundEffects", TRUE, "HODJPODJ.INI");
	}
}

void CMainMenu::OnOK() {
	//
	// Don't do anything
	//
}

void CMainMenu::OnCancel() {
	// user is returning to Mini-game (only if Continue is not disabled)
	//
	if (!(_flags & NO_RETURN)) {
		ClearDialogImage();
		EndDialog(IDC_OPTIONS_RETURN);
	}
}

void CMainMenu::OnClickedReturn() {
	// user is returning to Mini-game
	//
	ClearDialogImage();
	EndDialog(IDC_OPTIONS_RETURN);
}

void CMainMenu::OnClickedQuit() {
	// user hit the Quit Button
	//
	ClearDialogImage();
	EndDialog(IDC_OPTIONS_QUIT);
}

} // namespace HodjNPodj
} // namespace Bagel
