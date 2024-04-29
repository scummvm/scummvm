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

#include "bagel/dialogs/restart_dialog.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/master_win.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/std_keys.h"
#include "bagel/dialogs/opt_window.h"

namespace Bagel {

#define LOADINGBMP          "$SBARDIR\\GENERAL\\SYSTEM\\LOADING.BMP"

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

static const ST_BUTTONS g_stRestartButtons[NUM_RESTART_BTNS] = {
	{ "Yes", "yesup.bmp", "yesdn.bmp", "yesup.bmp", "yesup.bmp", 194, 249, 120, 40, RESTART_BTN },
	{ "Cancel", "cancelup.bmp", "canceldn.bmp", "cancelup.bmp", "cancelup.bmp", 324, 249, 120, 40, CANCEL_BTN }
};


CBagRestartDialog::CBagRestartDialog(const char *pszFileName, CBofWindow *pWin)
	: CBofDialog(pszFileName, pWin) {
	// Inits
	m_pSavePalette = nullptr;
	_nReturnValue = -1;

	for (int i = 0; i < NUM_RESTART_BTNS; i++) {
		m_pButtons[i] = nullptr;
	}
}


void CBagRestartDialog::onInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::onInitDialog();

	Assert(m_pBackdrop != nullptr);

	// Save off the current game's palette
	m_pSavePalette = CBofApp::GetApp()->GetPalette();

	// Insert ours
	CBofPalette *pPal = m_pBackdrop->GetPalette();
	CBofApp::GetApp()->SetPalette(pPal);

	// Paint the SaveList Box onto the background
	if (m_pBackdrop != nullptr) {
		pPal = m_pBackdrop->GetPalette();
		CBofBitmap cBmp(BuildSysDir("RESTDBOX.BMP"), pPal);
		cBmp.Paint(m_pBackdrop, 181, 182);
	}

	// Build all our buttons
	for (int i = 0; i < NUM_RESTART_BTNS; i++) {
		Assert(m_pButtons[i] == nullptr);

		if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {

			CBofBitmap *pUp = LoadBitmap(BuildSysDir(g_stRestartButtons[i].m_pszUp), pPal);
			CBofBitmap *pDown = LoadBitmap(BuildSysDir(g_stRestartButtons[i].m_pszDown), pPal);
			CBofBitmap *pFocus = LoadBitmap(BuildSysDir(g_stRestartButtons[i].m_pszFocus), pPal);
			CBofBitmap *pDis = LoadBitmap(BuildSysDir(g_stRestartButtons[i].m_pszDisabled), pPal);

			m_pButtons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);

			m_pButtons[i]->create(g_stRestartButtons[i].m_pszName, g_stRestartButtons[i].m_nLeft, g_stRestartButtons[i].m_nTop, g_stRestartButtons[i].m_nWidth, g_stRestartButtons[i].m_nHeight, this, g_stRestartButtons[i].m_nID);
			m_pButtons[i]->Show();
		} else {
			ReportError(ERR_MEMORY);
			break;
		}
	}

	// Show System cursor
	CBagCursor::showSystemCursor();
}


void CBagRestartDialog::onClose() {
	Assert(IsValidObject(this));

	CBagCursor::hideSystemCursor();

	// Destroy all buttons
	for (int i = 0; i < NUM_RESTART_BTNS; i++) {
		if (m_pButtons[i] != nullptr) {
			delete m_pButtons[i];
			m_pButtons[i] = nullptr;
		}
	}

	CBofDialog::onClose();

	if (_nReturnValue != RESTART_BTN) {
		CBofApp::GetApp()->SetPalette(m_pSavePalette);
	} else {
		CBofApp::GetApp()->SetPalette(nullptr);
	}
}


void CBagRestartDialog::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	PaintBackdrop(pRect);

	ValidateAnscestors();
}


void CBagRestartDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
	Assert(IsValidObject(this));

	switch (lKey) {

	// Start a new game
	case BKEY_ENTER: {
		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {

				char szBuf[256];
				Common::strcpy_s(szBuf, LOADINGBMP);
				CBofString cStr(szBuf, 256);
				MACROREPLACE(cStr);

				CBofRect cRect;
				cRect.left = (640 - 180) / 2;
				cRect.top = (480 - 50) / 2;
				cRect.right = cRect.left + 180 - 1;
				cRect.bottom = cRect.top + 50 - 1;

				CBofCursor::Hide();
				PaintBitmap(this, cStr, &cRect);

				pWin->NewGame();
				CBofCursor::Show();

				killBackground();

				_nReturnValue = RESTART_BTN;
				onClose();
			}
		}
		break;
	}

	// Cancel
	case BKEY_ESC:
		Close();
		break;

	default:
		CBofDialog::onKeyHit(lKey, nRepCount);
		break;
	}
}

void CBagRestartDialog::OnBofButton(CBofObject *pObject, int nFlags) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	if (nFlags != BUTTON_CLICKED)
		return;

	CBofBmpButton *pButton = (CBofBmpButton *)pObject;

	switch (pButton->GetControlID()) {

	// Cancel
	case CANCEL_BTN: {
		Close();
		break;
	}

	// Restart a new game
	case RESTART_BTN: {
		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {

				char szBuf[256];
				Common::strcpy_s(szBuf, LOADINGBMP);
				CBofString cStr(szBuf, 256);
				MACROREPLACE(cStr);

				CBofRect cRect;
				cRect.left = (640 - 180) / 2;
				cRect.top = (480 - 50) / 2;
				cRect.right = cRect.left + 180 - 1;
				cRect.bottom = cRect.top + 50 - 1;

				CBofCursor::Hide();
				PaintBitmap(this, cStr, &cRect);

				pWin->NewGame();
				CBofCursor::Show();

				killBackground();

				_nReturnValue = RESTART_BTN;
				onClose();
			}
		}
		break;
	}

	default:
		LogWarning(BuildString("Restart/Restore: Unknown button: %d", pButton->GetControlID()));
		break;
	}
}

} // namespace Bagel
