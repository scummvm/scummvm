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

#include "bagel/spacebar/dialogs/restart_dialog.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/button_object.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/boflib/std_keys.h"
#include "bagel/spacebar/dialogs/opt_window.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {
namespace SpaceBar {

#define LOADING_BMP          "$SBARDIR\\GENERAL\\SYSTEM\\LOADING.BMP"

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
	assert(isValidObject(this));

	CBofDialog::onInitDialog();

	if (_pBackdrop == nullptr)
		fatalError(ERR_UNKNOWN, "Unexpected null value found in _paBackdrop");

	// Save off the current game's palette
	_pSavePalette = CBofApp::getApp()->getPalette();

	// Insert ours
	CBofPalette *pPal = _pBackdrop->getPalette();
	CBofApp::getApp()->setPalette(pPal);

	// Paint the SaveList Box onto the background
	pPal = _pBackdrop->getPalette();
	CBofBitmap cBmp(buildSysDir("RESTDBOX.BMP"), pPal);
	cBmp.paint(_pBackdrop, 181, 182);

	// Build all our buttons
	for (int i = 0; i < NUM_RESTART_BTNS; i++) {
		assert(_pButtons[i] == nullptr);

		_pButtons[i] = new CBofBmpButton;

		CBofBitmap *pUp = loadBitmap(buildSysDir(g_stRestartButtons[i]._pszUp), pPal);
		CBofBitmap *pDown = loadBitmap(buildSysDir(g_stRestartButtons[i]._pszDown), pPal);
		CBofBitmap *pFocus = loadBitmap(buildSysDir(g_stRestartButtons[i]._pszFocus), pPal);
		CBofBitmap *pDis = loadBitmap(buildSysDir(g_stRestartButtons[i]._pszDisabled), pPal);

		_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
		_pButtons[i]->create(g_stRestartButtons[i]._pszName, g_stRestartButtons[i]._nLeft, g_stRestartButtons[i]._nTop, g_stRestartButtons[i]._nWidth, g_stRestartButtons[i]._nHeight, this, g_stRestartButtons[i]._nID);
		_pButtons[i]->show();
	}

	// Show System cursor
	CBagCursor::showSystemCursor();
}


void CBagRestartDialog::onClose() {
	assert(isValidObject(this));

	CBagCursor::hideSystemCursor();

	// Destroy all buttons
	for (int i = 0; i < NUM_RESTART_BTNS; i++) {
		delete _pButtons[i];
		_pButtons[i] = nullptr;
	}

	CBofDialog::onClose();

	if (_nReturnValue != RESTART_BTN) {
		CBofApp::getApp()->setPalette(_pSavePalette);
	} else {
		CBofApp::getApp()->setPalette(nullptr);
	}
}


void CBagRestartDialog::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));

	paintBackdrop(pRect);
	validateAnscestors();
}


void CBagRestartDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
	assert(isValidObject(this));

	switch (lKey) {

	// Start a new game
	case BKEY_ENTER: {
		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();
			if (pWin != nullptr) {

				char szBuf[256];
				Common::strcpy_s(szBuf, LOADING_BMP);
				CBofString cStr(szBuf, 256);
				fixPathName(cStr);

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
	assert(isValidObject(this));
	assert(pObject != nullptr);

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
				Common::strcpy_s(szBuf, LOADING_BMP);
				CBofString cStr(szBuf, 256);
				fixPathName(cStr);

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
		logWarning(buildString("Restart/Restore: Unknown button: %d", pButton->getControlID()));
		break;
	}
}

} // namespace SpaceBar
} // namespace Bagel
