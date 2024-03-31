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

#include "bagel/dialogs/start_dialog.h"
#include "bagel/dialogs/save_dialog.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/buttons.h"
#include "bagel/boflib/sound.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/bagel.h"

namespace Bagel {

struct ST_BUTTONS {
	const CHAR *m_pszName;
	const CHAR *m_pszUp;
	const CHAR *m_pszDown;
	const CHAR *m_pszFocus;
	const CHAR *m_pszDisabled;
	INT m_nLeft;
	INT m_nTop;
	INT m_nWidth;
	INT m_nHeight;
	INT m_nID;
};

static const ST_BUTTONS g_stStartButtons[NUM_START_BTNS] = {
	{ "Restore", "start1up.bmp", "start1dn.bmp", "start1gr.bmp", "start1gr.bmp", 110, 396, 120, 40, RESTORE_BTN },
	{ "Restart", "start2up.bmp", "start2dn.bmp", "start2up.bmp", "start2up.bmp", 238, 416, 120, 40, RESTART_BTN },
	{ "Quit", "start3up.bmp", "start3dn.bmp", "start3up.bmp", "start3up.bmp", 366, 436, 120, 40, QUIT_BTN }
};


CBagStartDialog::CBagStartDialog(const CHAR *pszFileName, CBofRect *pRect, CBofWindow *pWin)
		: CBofDialog(pszFileName, pRect, pWin) {
	// Inits
	_lFlags &= ~BOFDLG_SAVEBACKGND;
	Common::fill(_buttons, _buttons + NUM_START_BTNS, (CBofBmpButton *)nullptr);
}

VOID CBagStartDialog::OnInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::OnInitDialog();

	SetReturnValue(-1);

	// Halt all audio when user dies (and at start of game).
	CBofSound::StopSounds();

	CBofPalette *pPal;
	INT i;

	// Save off the current game's palette
	_savePalette = CBofApp::GetApp()->GetPalette();

	// Insert ours
	pPal = m_pBackdrop->GetPalette();
	CBofApp::GetApp()->SetPalette(pPal);

	// Build all our buttons
	for (i = 0; i < NUM_START_BTNS; i++) {
		Assert(_buttons[i] == nullptr);

		if ((_buttons[i] = new CBofBmpButton) != nullptr) {
			CBofBitmap *pUp, *pDown, *pFocus, *pDis;

			pUp = LoadBitmap(BuildSysDir(g_stStartButtons[i].m_pszUp), pPal);
			pDown = LoadBitmap(BuildSysDir(g_stStartButtons[i].m_pszDown), pPal);
			pFocus = LoadBitmap(BuildSysDir(g_stStartButtons[i].m_pszFocus), pPal);
			pDis = LoadBitmap(BuildSysDir(g_stStartButtons[i].m_pszDisabled), pPal);

			_buttons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);
			_buttons[i]->Create(g_stStartButtons[i].m_pszName, g_stStartButtons[i].m_nLeft, g_stStartButtons[i].m_nTop, g_stStartButtons[i].m_nWidth, g_stStartButtons[i].m_nHeight, this, g_stStartButtons[i].m_nID);
			_buttons[i]->Show();

		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	// Disable the restore button if there are no saved games
	CBagel *pApp;
	if ((pApp = CBagel::GetBagApp()) != nullptr) {

		if (!g_engine->savesExist())
			_buttons[0]->SetState(BUTTON_DISABLED);
	}

	// Show System cursor
	CBagCursor::ShowSystemCursor();
}


VOID CBagStartDialog::OnClose() {
	Assert(IsValidObject(this));

	CBagCursor::HideSystemCursor();

	// Destroy all buttons
	for (INT i = 0; i < NUM_START_BTNS; i++) {
		if (_buttons[i] != nullptr) {
			delete _buttons[i];
			_buttons[i] = nullptr;
		}
	}

	KillBackground();

	CBofDialog::OnClose();
}


VOID CBagStartDialog::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	PaintBackdrop(pRect);

#if BOF_MAC
	for (INT i = 0; i < NUM_START_BTNS; i++) {
		if (m_pButtons[i] != nullptr) {
			m_pButtons[i]->Paint();
		}
	}
#endif

	ValidateAnscestors();
}


VOID CBagStartDialog::OnKeyHit(ULONG lKey, ULONG nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {
	case BKEY_ESC:
		SetReturnValue(QUIT_BTN);
		Close();
		break;

	default:
		CBofDialog::OnKeyHit(lKey, nRepCount);
		break;
	}
}


VOID CBagStartDialog::OnBofButton(CBofObject *pObject, INT nFlags) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	if (nFlags == BUTTON_CLICKED) {
		CBofBmpButton *pButton = (CBofBmpButton *)pObject;
		INT nId = pButton->GetControlID();

		if (nId == RESTORE_BTN) {
			CBagel *pApp;
			if ((pApp = CBagel::GetBagApp()) != nullptr) {
				CBagMasterWin *pWin;

				if ((pWin = pApp->GetMasterWnd()) != nullptr) {
					if (pWin->ShowRestoreDialog(this)) {
						Close();
					}
				}
			}
		} else {
			SetReturnValue(nId);

			Close();
		}
	}
}

} // namespace Bagel
