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

#include "bagel/spacebar/dialogs/start_dialog.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/button_object.h"
#include "bagel/boflib/sound.h"
#include "bagel/spacebar/dialogs/opt_window.h"
#include "bagel/bagel.h"
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

static const ST_BUTTONS g_stStartButtons[NUM_START_BTNS] = {
	{ "Restore", "start1up.bmp", "start1dn.bmp", "start1gr.bmp", "start1gr.bmp", 110, 396, 120, 40, RESTORE_BTN },
	{ "Restart", "start2up.bmp", "start2dn.bmp", "start2up.bmp", "start2up.bmp", 238, 416, 120, 40, RESTART_BTN },
	{ "Quit", "start3up.bmp", "start3dn.bmp", "start3up.bmp", "start3up.bmp", 366, 436, 120, 40, QUIT_BTN }
};


CBagStartDialog::CBagStartDialog(const char *pszFileName, CBofWindow *pWin)
	: CBofDialog(pszFileName, pWin) {
	// Inits
	_lFlags &= ~BOFDLG_SAVEBACKGND;
	Common::fill(_buttons, _buttons + NUM_START_BTNS, (CBofBmpButton *)nullptr);
}

void CBagStartDialog::onInitDialog() {
	assert(isValidObject(this));

	CBofDialog::onInitDialog();
	setReturnValue(-1);

	// Halt all audio when user dies (and at start of game).
	CBofSound::stopSounds();

	// Save off the current game's palette
	_savePalette = CBofApp::getApp()->getPalette();

	// Insert ours
	CBofPalette *pPal = _pBackdrop->getPalette();
	CBofApp::getApp()->setPalette(pPal);

	// Build all our buttons
	for (int i = 0; i < NUM_START_BTNS; i++) {
		assert(_buttons[i] == nullptr);

		_buttons[i] = new CBofBmpButton;

		CBofBitmap *pUp = loadBitmap(buildSysDir(g_stStartButtons[i]._pszUp), pPal);
		CBofBitmap *pDown = loadBitmap(buildSysDir(g_stStartButtons[i]._pszDown), pPal);
		CBofBitmap *pFocus = loadBitmap(buildSysDir(g_stStartButtons[i]._pszFocus), pPal);
		CBofBitmap *pDis = loadBitmap(buildSysDir(g_stStartButtons[i]._pszDisabled), pPal);

		_buttons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
		_buttons[i]->create(g_stStartButtons[i]._pszName, g_stStartButtons[i]._nLeft, g_stStartButtons[i]._nTop, g_stStartButtons[i]._nWidth, g_stStartButtons[i]._nHeight, this, g_stStartButtons[i]._nID);
		_buttons[i]->show();
	}

	// Disable the restore button if there are no saved games
	CBagel *pApp = CBagel::getBagApp();
	if ((pApp != nullptr) && !g_engine->savesExist())
		_buttons[0]->setState(BUTTON_DISABLED);

	// Show System cursor
	CBagCursor::showSystemCursor();
}


void CBagStartDialog::onClose() {
	assert(isValidObject(this));

	CBagCursor::hideSystemCursor();

	// Destroy all buttons
	for (int i = 0; i < NUM_START_BTNS; i++) {
		delete _buttons[i];
		_buttons[i] = nullptr;
	}

	killBackground();

	CBofDialog::onClose();
}


void CBagStartDialog::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));

	paintBackdrop(pRect);
	validateAnscestors();
}


void CBagStartDialog::onKeyHit(uint32 lKey, uint32 nRepCount) {
	assert(isValidObject(this));

	if (lKey == BKEY_ESC) {
		setReturnValue(QUIT_BTN);
		close();
	} else {
		CBofDialog::onKeyHit(lKey, nRepCount);
	}
}


void CBagStartDialog::onBofButton(CBofObject *pObject, int nFlags) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	if (nFlags != BUTTON_CLICKED)
		return;

	CBofBmpButton *pButton = (CBofBmpButton *)pObject;
	int nId = pButton->getControlID();

	if (nId == RESTORE_BTN) {
		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWin = pApp->getMasterWnd();

			if ((pWin != nullptr) && pWin->showRestoreDialog(this)) {
				close();
			}
		}
	} else {
		setReturnValue(nId);
		close();
	}
}

} // namespace SpaceBar
} // namespace Bagel
