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
#include "bagel/hodjnpodj/hodjnpodj.h"

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
	ON_BN_CLICKED(IDC_OPTIONS_HYPE, CMainMenu::OnClickedHype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMainMenu::CMainMenu(CWnd *pParent, CPalette *pPalette,
        unsigned int nFlags, FPFUNC pOptionsFunc, const char *pRulesFileName,
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


void CMainMenu::clearButtons() {
	delete _rulesButton;
	_rulesButton = nullptr;

	delete _newGameButton;
	_newGameButton = nullptr;

	delete _optionsButton;
	_optionsButton = nullptr;

	delete _audioButton;
	_audioButton = nullptr;

	delete _returnButton;
	_returnButton = nullptr;

	delete _quitButton;
	_quitButton = nullptr;

	delete _hypeButton;
	_hypeButton = nullptr;
}

void CMainMenu::ClearDialogImage() {
	clearButtons();

	if (m_pDlgBackground != nullptr)
		InvalidateRect(nullptr, false);
}

void CMainMenu::OnDestroy() {
	CBmpDialog::OnDestroy();

	clearButtons();
}

bool CMainMenu::OnInitDialog() {
	CWnd *pWndTemp;

	CBmpDialog::OnInitDialog();            // do basic dialog initialization

	// Set up replacement color buttons
	_rulesButton = new CColorButton();
	_rulesButton->SetPalette(m_pPalette);
	_rulesButton->SetControl(IDC_OPTIONS_RULES, this);

	_newGameButton = new CColorButton();
	_newGameButton->SetPalette(m_pPalette);
	_newGameButton->SetControl(IDC_OPTIONS_NEWGAME, this);

	_optionsButton = new CColorButton();
	_optionsButton->SetPalette(m_pPalette);
	_optionsButton->SetControl(IDC_OPTIONS_OPTIONS, this);

	_audioButton = new CColorButton();
	_audioButton->SetPalette(m_pPalette);
	_audioButton->SetControl(IDC_OPTIONS_AUDIO, this);

	_returnButton = new CColorButton();
	_returnButton->SetPalette(m_pPalette);
	_returnButton->SetControl(IDC_OPTIONS_RETURN, this);

	_quitButton = new CColorButton();
	_quitButton->SetPalette(m_pPalette);
	_quitButton->SetControl(IDC_OPTIONS_QUIT, this);

	// Add the Hype Button if running the demo
	if (g_engine->isDemo()) {
		CRect optionsRect, audioRect;
		_optionsButton->GetWindowRect(&optionsRect);
		_audioButton->GetWindowRect(&audioRect);
		CRect hypeRect(optionsRect.right + 5, optionsRect.top,
			optionsRect.right + 50, audioRect.bottom);

		_hypeButton = new CColorButton();
		_hypeButton->Create("?", BS_OWNERDRAW | WS_CHILD | WS_VISIBLE,
			hypeRect, this, IDC_OPTIONS_HYPE);
		_hypeButton->SetPalette(m_pPalette);

		_hypeFont.CreateFont(24, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
		_hypeButton->SetFont(&_hypeFont);
	}

	// Disable the Rules button if told to do so
	if (_flags & NO_RULES) {
		pWndTemp = GetDlgItem(IDC_OPTIONS_RULES);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(false);
	}

	// Disable the NewGame button if told to do so
	if (_flags & NO_NEWGAME) {
		pWndTemp = GetDlgItem(IDC_OPTIONS_NEWGAME);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(false);
	}

	// Disable the Options button if told to do so
	if (_flags & NO_OPTIONS) {
		pWndTemp = GetDlgItem(IDC_OPTIONS_OPTIONS);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(false);
	}

	// Disable the Audio button if told to do so
	if (_flags & NO_AUDIO) {
		pWndTemp = GetDlgItem(IDC_OPTIONS_AUDIO);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(false);
	}

	// Disable the Return button if told to do so
	if (_flags & NO_RETURN) {
		pWndTemp = GetDlgItem(IDC_OPTIONS_RETURN);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(false);
	}

	// Disable the Return button if told to do so
	if (_flags & NO_QUIT) {
		pWndTemp = GetDlgItem(IDC_OPTIONS_QUIT);
		assert(pWndTemp != nullptr);
		if (pWndTemp != nullptr)
			pWndTemp->EnableWindow(false);
	}

	return true;
}


bool CMainMenu::OnEraseBkgnd(CDC *pDC) {
	return true;
}


void CMainMenu::OnPaint() {
	CBmpDialog::OnPaint();
}


void CMainMenu::OnClickedRules() {
	CWnd *pControl;

	// Load the rules
	CRules  RulesDlg(this, _rulesFilename, m_pPalette, _wavFilename);

	// Display the rules
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

void CMainMenu::OnClickedHype() {
	CWnd *pControl;

	// Load the hype
	CRules  hypeDlg(this, "hype.txt", m_pPalette, nullptr);

	// Display the dialog
	hypeDlg.DoModal();

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
	// User has chosen to start a new game
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

	// Call the user defined sub-options (we are the parent)
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
		_gameParams->bMusicEnabled = GetPrivateProfileInt("Meta", "Music", true, "HODJPODJ.INI");
		_gameParams->bSoundEffectsEnabled = GetPrivateProfileInt("Meta", "SoundEffects", true, "HODJPODJ.INI");
	}
}

void CMainMenu::OnOK() {
	// Don't do anything
}

void CMainMenu::OnCancel() {
	// User is returning to Mini-game (only if Continue is not disabled)
	if (!(_flags & NO_RETURN)) {
		ClearDialogImage();
		EndDialog(IDC_OPTIONS_RETURN);
	}
}

void CMainMenu::OnClickedReturn() {
	// User is returning to Mini-game
	ClearDialogImage();
	EndDialog(IDC_OPTIONS_RETURN);
}

void CMainMenu::OnClickedQuit() {
	// User hit the Quit Button
	ClearDialogImage();
	EndDialog(IDC_OPTIONS_QUIT);
}

} // namespace HodjNPodj
} // namespace Bagel
