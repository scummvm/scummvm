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
	const char *_pszName;
	const char *_pszUp;
	const char *_pszDown;
	const char *_pszFocus;
	const char *_pszDisabled;
	int _nLeft;
	int _nTop;
	int _nWidth;
	int _nHeight;
	int _nID;
};

static const ST_BUTTONS g_stRestartButtons[NUM_RESTART_BTNS] = {
	{ "Yes", "yesup.bmp", "yesdn.bmp", "yesup.bmp", "yesup.bmp", 194, 249, 120, 40, RESTART_BTN },
	{ "Cancel", "cancelup.bmp", "canceldn.bmp", "cancelup.bmp", "cancelup.bmp", 324, 249, 120, 40, CANCEL_BTN }
};


CBagRestartDialog::CBagRestartDialog(const char *pszFileName, CBofWindow *pWin)
	: CBofDialog(pszFileName, pWin) {
	// Inits
	_pSavePalette = nullptr;
	_nReturnValue = -1;

	for (int i = 0; i < NUM_RESTART_BTNS; i++) {
		_pButtons[i] = nullptr;
	}
}


void CBagRestartDialog::onInitDialog() {
	Assert(IsValidObject(this));

	CBofDialog::onInitDialog();

	Assert(_pBackdrop != nullptr);

	// Save off the current game's palette
	_pSavePalette = CBofApp::getApp()->getPalette();

	// Insert ours
	CBofPalette *pPal = _pBackdrop->getPalette();
	CBofApp::getApp()->setPalette(pPal);

	// Paint the SaveList Box onto the background
	if (_pBackdrop != nullptr) {
		pPal = _pBackdrop->getPalette();
		CBofBitmap cBmp(buildSysDir("RESTDBOX.BMP"), pPal);
		cBmp.paint(_pBackdrop, 181, 182);
	}

	// Build all our buttons
	for (int i = 0; i < NUM_RESTART_BTNS; i++) {
		Assert(_pButtons[i] == nullptr);

		if ((_pButtons[i] = new CBofBmpButton) != nullptr) {

			CBofBitmap *pUp = loadBitmap(buildSysDir(g_stRestartButtons[i]._pszUp), pPal);
			CBofBitmap *pDown = loadBitmap(buildSysDir(g_stRestartButtons[i]._pszDown), pPal);
			CBofBitmap *pFocus = loadBitmap(buildSysDir(g_stRestartButtons[i]._pszFocus), pPal);
			CBofBitmap *pDis = loadBitmap(buildSysDir(g_stRestartButtons[i]._pszDisabled), pPal);

			_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);

			_pButtons[i]->create(g_stRestartButtons[i]._pszName, g_stRestartButtons[i]._nLeft, g_stRestartButtons[i]._nTop, g_stRestartButtons[i]._nWidth, g_stRestartButtons[i]._nHeight, this, g_stRestartButtons[i]._nID);
			_pButtons[i]->show();
		} else {
			reportError(ERR_MEMORY);
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
		if (_pButtons[i] != nullptr) {
			delete _pButtons[i];
			_pButtons[i] = nullptr;
		}
	}

	CBofDialog::onClose();

	if (_nReturnValue != RESTART_BTN) {
		CBofApp::getApp()->setPalette(_pSavePalette);
	} else {
		CBofApp::getApp()->setPalette(nullptr);
	}
}


void CBagRestartDialog::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	paintBackdrop(pRect);

	validateAnscestors();
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

				CBofCursor::hide();
				paintBitmap(this, cStr, &cRect);

				pWin->newGame();
				CBofCursor::show();

				killBackground();

				_nReturnValue = RESTART_BTN;
				onClose();
			}
		}
		break;
	}

	// Cancel
	case BKEY_ESC:
		close();
		break;

	default:
		CBofDialog::onKeyHit(lKey, nRepCount);
		break;
	}
}

void CBagRestartDialog::onBofButton(CBofObject *pObject, int nFlags) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	if (nFlags != BUTTON_CLICKED)
		return;

	CBofBmpButton *pButton = (CBofBmpButton *)pObject;

	switch (pButton->getControlID()) {

	// Cancel
	case CANCEL_BTN: {
		close();
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

				CBofCursor::hide();
				paintBitmap(this, cStr, &cRect);

				pWin->newGame();
				CBofCursor::show();

				killBackground();

				_nReturnValue = RESTART_BTN;
				onClose();
			}
		}
		break;
	}

	default:
		LogWarning(BuildString("Restart/Restore: Unknown button: %d", pButton->getControlID()));
		break;
	}
}

} // namespace Bagel
