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

#include "bagel/spacebar/dialogs/quit_dialog.h"
#include "bagel/spacebar/dialogs/opt_window.h"
#include "bagel/spacebar/baglib/cursor.h"
#include "bagel/spacebar/baglib/button_object.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

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

static const ST_BUTTONS g_stQuitButtons[NUM_QUIT_BUTTONS] = {
	{ "Save", "SAVEQTUP.BMP", "SAVEQTDN.BMP", "SAVEQTUP.BMP", "SAVEQTUP.BMP", 220, 190, 200, 38, SAVE_BTN },
	{ "Quit", "JUSTQTUP.BMP", "JUSTQTDN.BMP", "JUSTQTUP.BMP", "JUSTQTUP.BMP", 220, 237, 200, 38, QUIT_BTN },
	{ "Cancel", "PLAYUP.BMP", "PLAYDN.BMP", "PLAYUP.BMP", "PLAYUP.BMP", 220, 284, 200, 38, CANCEL_BTN }
};

CBagQuitDialog::CBagQuitDialog() {
	// Inits
	_nReturnValue = -1;
	for (int i = 0; i < NUM_QUIT_BUTTONS; i++) {
		_pButtons[i] = nullptr;
	}
}


void CBagQuitDialog::onInitDialog() {
	assert(isValidObject(this));

	CBofDialog::onInitDialog();
	setReturnValue(-1);

	if (_pBackdrop == nullptr)
		fatalError(ERR_UNKNOWN, "Unexpected null value found in _paBackdrop");

	CBofPalette *pPal = _pBackdrop->getPalette();
	selectPalette(pPal);

	// Paint the SaveList Box onto the background
	CBofBitmap cBmp(buildSysDir("QUITDBOX.BMP"), pPal);
	cBmp.paint(_pBackdrop, 205, 150);

	// Build all our buttons
	for (int i = 0; i < NUM_QUIT_BUTTONS; i++) {
		assert(_pButtons[i] == nullptr);

		_pButtons[i] = new CBofBmpButton;

		CBofBitmap *pUp = loadBitmap(buildSysDir(g_stQuitButtons[i]._pszUp), pPal);
		CBofBitmap *pDown = loadBitmap(buildSysDir(g_stQuitButtons[i]._pszDown), pPal);
		CBofBitmap *pFocus = loadBitmap(buildSysDir(g_stQuitButtons[i]._pszFocus), pPal);
		CBofBitmap *pDis = loadBitmap(buildSysDir(g_stQuitButtons[i]._pszDisabled), pPal);

		_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
		_pButtons[i]->create(g_stQuitButtons[i]._pszName, g_stQuitButtons[i]._nLeft, g_stQuitButtons[i]._nTop, g_stQuitButtons[i]._nWidth, g_stQuitButtons[i]._nHeight, this, g_stQuitButtons[i]._nID);
		_pButtons[i]->show();
	}

	// Show System cursor
	CBagCursor::showSystemCursor();
}


void CBagQuitDialog::onClose() {
	assert(isValidObject(this));

	CBagCursor::hideSystemCursor();

	// Destroy all buttons
	for (int i = 0; i < NUM_QUIT_BUTTONS; i++) {
		delete _pButtons[i];
		_pButtons[i] = nullptr;
	}

	if (_nReturnValue == QUIT_BTN || _nReturnValue == SAVE_BTN)
		killBackground();

	CBofDialog::onClose();
}


void CBagQuitDialog::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));

	paintBackdrop(pRect);
	validateAnscestors();
}


void CBagQuitDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
	assert(isValidObject(this));

	if (lKey == BKEY_ESC) {
		// Cancel
		//
		setReturnValue(CANCEL_BTN);
		close();
	} else
		CBofDialog::onKeyHit(lKey, nRepCount);
}


void CBagQuitDialog::onBofButton(CBofObject *pObject, int nFlags) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	if (nFlags != BUTTON_CLICKED)
		return;

	CBofBmpButton *pButton = (CBofBmpButton *)pObject;

	if (pButton != nullptr) {
		int nId = pButton->getControlID();

		bool bQuit = true;
		if (nId == SAVE_BTN) {
			CBagel *pApp = CBagel::getBagApp();
			if (pApp != nullptr) {
				CBagMasterWin *pWin = pApp->getMasterWnd();
				if (pWin != nullptr) {
					bQuit = pWin->showSaveDialog(this, false);
				}
			}
		}

		if (bQuit) {
			setReturnValue(nId);
			close();
		}
	}
}

} // namespace SpaceBar
} // namespace Bagel
