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

#include "bagel/dialogs/quit_dialog.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/baglib/cursor.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/master_win.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

struct ST_BUTTONS {
	const char *m_pszName;
	const char *m_pszUp;
	const char *m_pszDown;
	const char *m_pszFocus;
	const char *m_pszDisabled;
	int m_nLeft;
	int m_nTop;
	int m_nWidth;
	int m_nHeight;
	int m_nID;
};

static const ST_BUTTONS g_stQuitButtons[NUM_QUIT_BUTTONS] = {
	{ "Save", "SAVEQTUP.BMP", "SAVEQTDN.BMP", "SAVEQTUP.BMP", "SAVEQTUP.BMP", 220, 190, 200, 38, SAVE_BTN },
	{ "Quit", "JUSTQTUP.BMP", "JUSTQTDN.BMP", "JUSTQTUP.BMP", "JUSTQTUP.BMP", 220, 237, 200, 38, QUIT_BTN },
	{ "Cancel", "PLAYUP.BMP", "PLAYDN.BMP", "PLAYUP.BMP", "PLAYUP.BMP", 220, 284, 200, 38, CANCEL_BTN }
};

CBagQuitDialog::CBagQuitDialog() {
	// Inits
	_nReturnValue = -1;
	for (int i = 0; i < NUM_QUIT_BUTTONS; i++) {
		m_pButtons[i] = nullptr;
	}
}


void CBagQuitDialog::onInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::onInitDialog();
	SetReturnValue(-1);

	Assert(m_pBackdrop != nullptr);
	CBofPalette *pPal = m_pBackdrop->GetPalette();
	SelectPalette(pPal);

	// Paint the SaveList Box onto the background
	if (m_pBackdrop != nullptr) {
		CBofBitmap cBmp(BuildSysDir("QUITDBOX.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 205, 150);
	}

	// Build all our buttons
	for (int i = 0; i < NUM_QUIT_BUTTONS; i++) {
		Assert(m_pButtons[i] == nullptr);

		if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {
			CBofBitmap *pUp = LoadBitmap(BuildSysDir(g_stQuitButtons[i].m_pszUp), pPal);
			CBofBitmap *pDown = LoadBitmap(BuildSysDir(g_stQuitButtons[i].m_pszDown), pPal);
			CBofBitmap *pFocus = LoadBitmap(BuildSysDir(g_stQuitButtons[i].m_pszFocus), pPal);
			CBofBitmap *pDis = LoadBitmap(BuildSysDir(g_stQuitButtons[i].m_pszDisabled), pPal);

			m_pButtons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);

			m_pButtons[i]->Create(g_stQuitButtons[i].m_pszName, g_stQuitButtons[i].m_nLeft, g_stQuitButtons[i].m_nTop, g_stQuitButtons[i].m_nWidth, g_stQuitButtons[i].m_nHeight, this, g_stQuitButtons[i].m_nID);
			m_pButtons[i]->Show();
		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	// Show System cursor
	CBagCursor::showSystemCursor();
}


void CBagQuitDialog::onClose() {
	Assert(IsValidObject(this));

	CBagCursor::hideSystemCursor();

	// Destroy all buttons
	for (int i = 0; i < NUM_QUIT_BUTTONS; i++) {
		if (m_pButtons[i] != nullptr) {
			delete m_pButtons[i];
			m_pButtons[i] = nullptr;
		}
	}

	if (_nReturnValue == QUIT_BTN || _nReturnValue == SAVE_BTN)
		killBackground();

	CBofDialog::onClose();
}


void CBagQuitDialog::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	PaintBackdrop(pRect);

	ValidateAnscestors();
}


void CBagQuitDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

	if (lKey == BKEY_ESC) {
		// Cancel
		//
		SetReturnValue(CANCEL_BTN);
		Close();
	} else 
		CBofDialog::onKeyHit(lKey, nRepCount);
}


void CBagQuitDialog::OnBofButton(CBofObject *pObject, int nFlags) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	if (nFlags != BUTTON_CLICKED)
		return;

	CBofBmpButton *pButton = (CBofBmpButton *)pObject;

	if (pButton != nullptr) {
		int nId = pButton->GetControlID();

		bool bQuit = true;
		if (nId == SAVE_BTN) {
			CBagel *pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				CBagMasterWin *pWin = pApp->getMasterWnd();
				if (pWin != nullptr) {

					bQuit = pWin->ShowSaveDialog(this, false);
				}
			}
		}

		if (bQuit) {
			SetReturnValue(nId);
			Close();
		}
	}
}

} // namespace Bagel
