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

#include "bagel/spacebar/baglib/help.h"
#include "bagel/spacebar/boflib/file.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"
#include "bagel/spacebar/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

#define RULES_DIR "$SBARDIR\\GENERAL\\RULES\\"

#define HELP_OK_X  257
#define HELP_OK_Y  377
#define HELP_OK_CX 125
#define HELP_OK_CY 30
#define HELP_OK_UP         "DONE1.BMP"
#define HELP_OK_DOWN       "DONE2.BMP"
#define HELP_OK_FOCUS      "DONE3.BMP"
#define HELP_OK_DISABLED   "DONE4.BMP"
#define HELP_OK_ID 1

#define HELP_PAGE_UP_X  81
#define HELP_PAGE_UP_Y  377
#define HELP_PAGE_UP_CX 125
#define HELP_PAGE_UP_CY 30
#define HELP_PAGE_UP_UP         "PAGEUP1.BMP"
#define HELP_PAGE_UP_DOWN       "PAGEUP2.BMP"
#define HELP_PAGE_UP_FOCUS      "PAGEUP3.BMP"
#define HELP_PAGE_UP_DISABLED   "PAGEUP4.BMP"
#define HELP_PAGE_UP_ID 2

#define HELP_PAGE_DOWN_X  435
#define HELP_PAGE_DOWN_Y  377
#define HELP_PAGE_DOWN_CX 125
#define HELP_PAGE_DOWN_CY 30
#define HELP_PAGE_DOWN_UP         "PAGEDN1.BMP"
#define HELP_PAGE_DOWN_DOWN       "PAGEDN2.BMP"
#define HELP_PAGE_DOWN_FOCUS      "PAGEDN3.BMP"
#define HELP_PAGE_DOWN_DISABLED   "PAGEDN4.BMP"
#define HELP_PAGE_DOWN_ID 3

// Local functions
const char *buildHelpDir(const char *fileName);


CBagHelp::CBagHelp() {
	_textBox = nullptr;
	_okButton = nullptr;
	_pageUp = nullptr;
	_pageDown = nullptr;
	_savePalette = nullptr;
	setFlags(0);
}

ErrorCode CBagHelp::attach() {
	assert(isValidObject(this));

	// Save off the current game's palette
	_savePalette = CBofApp::getApp()->getPalette();

	// Insert ours
	CBofPalette *backPal = _pBackdrop->getPalette();
	CBofApp::getApp()->setPalette(backPal);

	_okButton = new CBofBmpButton;

	CBofBitmap *upBmp = loadBitmap(buildHelpDir(HELP_OK_UP), backPal);
	CBofBitmap *downBmp = loadBitmap(buildHelpDir(HELP_OK_DOWN), backPal);
	CBofBitmap *focusBmp = loadBitmap(buildHelpDir(HELP_OK_FOCUS), backPal);
	CBofBitmap *disableBmp = loadBitmap(buildHelpDir(HELP_OK_DISABLED), backPal);

	_okButton->loadBitmaps(upBmp, downBmp, focusBmp, disableBmp);
	_okButton->create("OK", HELP_OK_X, HELP_OK_Y, HELP_OK_CX, HELP_OK_CY, this, HELP_OK_ID);
	_okButton->show();

	_pageUp = new CBofBmpButton;

	upBmp = loadBitmap(buildHelpDir(HELP_PAGE_UP_UP), backPal);
	downBmp = loadBitmap(buildHelpDir(HELP_PAGE_UP_DOWN), backPal);
	focusBmp = loadBitmap(buildHelpDir(HELP_PAGE_UP_FOCUS), backPal);
	disableBmp = loadBitmap(buildHelpDir(HELP_PAGE_UP_DISABLED), backPal);

	_pageUp->loadBitmaps(upBmp, downBmp, focusBmp, disableBmp);
	_pageUp->create("PageUp", HELP_PAGE_UP_X, HELP_PAGE_UP_Y, HELP_PAGE_UP_CX, HELP_PAGE_UP_CY, this, HELP_PAGE_UP_ID);
	_pageUp->show();

	_pageDown = new CBofBmpButton;

	upBmp = loadBitmap(buildHelpDir(HELP_PAGE_DOWN_UP), backPal);
	downBmp = loadBitmap(buildHelpDir(HELP_PAGE_DOWN_DOWN), backPal);
	focusBmp = loadBitmap(buildHelpDir(HELP_PAGE_DOWN_FOCUS), backPal);
	disableBmp = loadBitmap(buildHelpDir(HELP_PAGE_DOWN_DISABLED), backPal);

	_pageDown->loadBitmaps(upBmp, downBmp, focusBmp, disableBmp);

	_pageDown->create("PageDown", HELP_PAGE_DOWN_X, HELP_PAGE_DOWN_Y, HELP_PAGE_DOWN_CX, HELP_PAGE_DOWN_CY, this, HELP_PAGE_DOWN_ID);
	_pageDown->show();

	CBofFile file(_textFile, CBF_BINARY | CBF_READONLY);

	const uint32 size = file.getLength();
	char *buffer = (char *)bofCleanAlloc(size + 1);

	file.read(buffer, size);

	CBofRect cRect;
	cRect.setRect(120, 100, 550, 348);

	_textBox = new CBofTextBox(this, &cRect, buffer);
	_textBox->setPageLength(18);

	bofFree(buffer);

	CBagCursor::showSystemCursor();

	return _errCode;
}


ErrorCode CBagHelp::detach() {
	assert(isValidObject(this));

	CBagCursor::hideSystemCursor();

	delete _textBox;
	_textBox = nullptr;

	delete _pageDown;
	_pageDown = nullptr;

	delete _pageUp;
	_pageUp = nullptr;

	delete _okButton;
	_okButton = nullptr;

	CBofApp::getApp()->setPalette(_savePalette);

	return _errCode;
}

ErrorCode CBagHelp::SetHelpFile(const char *textFile) {
	assert(isValidObject(this));
	assert(textFile != nullptr);

	_textFile = textFile;

	return _errCode;
}


void CBagHelp::onPaint(CBofRect *rect) {
	assert(isValidObject(this));

	paintBackdrop(rect);

	if (_textBox != nullptr) {
		_textBox->display();
	}

	validateAnscestors();
}


void CBagHelp::onKeyHit(uint32 keyCode, uint32 repCount) {
	assert(isValidObject(this));

	switch (keyCode) {
	case BKEY_UP:
		if (_textBox != nullptr) {
			_textBox->lineUp();
		}
		break;

	case BKEY_DOWN:
		if (_textBox != nullptr) {
			_textBox->lineDown();
		}
		break;

	case BKEY_PAGEUP:
		if (_textBox != nullptr) {
			_textBox->pageUp();
		}
		break;

	case BKEY_PAGEDOWN:
		if (_textBox != nullptr) {
			_textBox->pageDown();
		}
		break;

	case BKEY_ESC:
	case BKEY_ENTER:
		close();
	// FIXME: missing break?
	// fallthrough

	default:
		CBofDialog::onKeyHit(keyCode, repCount);
		break;
	}
}


void CBagHelp::onBofButton(CBofObject *object, int flags) {
	assert(isValidObject(this));
	assert(object != nullptr);

	if (flags != BUTTON_CLICKED)
		return;

	CBofBmpButton *pButton = (CBofBmpButton *)object;

	switch (pButton->getControlID()) {
	case HELP_OK_ID:
		close();
		break;

	case HELP_PAGE_UP_ID:
		if (_textBox != nullptr) {
			_textBox->pageUp();
		}
		break;

	case HELP_PAGE_DOWN_ID:
		if (_textBox != nullptr) {
			_textBox->pageDown();
		}
		break;

	default:
		logWarning("Unknown button");
		break;
	}
}

const char *buildHelpDir(const char *fileName) {
	return formPath(RULES_DIR, fileName);
}

void CBagHelp::onInitDialog() {
	assert(isValidObject(this));

	CBofDialog::onInitDialog();

	attach();
}

} // namespace SpaceBar
} // namespace Bagel
