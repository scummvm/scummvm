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

#include "graphics/cursorman.h"
#include "bagel/spacebar/dialogs/credits_dialog.h"
#include "bagel/spacebar/dialogs/opt_window.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/string_functions.h"

namespace Bagel {
namespace SpaceBar {

struct ST_CREDITS {
	const char *_pszBackground;
	const char *_pszTextFile;

	int _nLeft;
	int _nTop;
	int _nRight;
	int _nBottom;

	int _nScrollRate;
	int _nPointSize;

};

#define CREDIT_TIMER_ID 567

#define NUM_SCREENS 12

#define MY_MASK_COLOR 1
#define LINE_HEIGHT 24
#define PIX_SCROLL_DY 1

static const ST_CREDITS g_cScreen[NUM_SCREENS] = {
	// Background        Credit text    topLeft   BotRight  Delay   PointSize
	{ "BARAREA.BMP",    "CREDITS1.TXT", 370,   6, 636, 150, 30,     20 },
	{ "CILIA.BMP",      "CREDITS2.TXT",   6, 120, 310, 360, 10,     24 },
	{ "AUDITON.BMP",    "CREDITS3.TXT", 320, 120, 636, 360, 10,     24 },
	{ "CAST1.BMP",      "CAST1.TXT",      6, 120, 310, 360, 10,     24 },
	{ "CAST2.BMP",      "CAST2.TXT",    376, 120, 636, 360, 10,     24 },
	{ "CAST3.BMP",      "CAST3.TXT",      6, 120, 310, 360, 10,     24 },
	{ "CAST4.BMP",      "CAST4.TXT",      6, 120, 290, 360, 10,     24 },
	{ "VILDROID.BMP",   "CREDITS5.TXT", 394, 120, 636, 360, 10,     22 },
	{ "ZZAZZL.BMP",     "CREDITS6.TXT",   6, 220, 284, 474, 10,     24 },
	{ "SRAFFAN.BMP",    "CREDITS7.TXT", 352, 310, 636, 474, 10,     24 },
	{ "FLEEBIX.BMP",    "CREDITS8.TXT", 466, 230, 636, 474, 10,     18 },
	{ "TRISECKS.BMP",   "CREDITS9.TXT",   6, 374, 636, 474, 10,     24 }
};

static bool g_b1 = false;

CBagCreditsDialog::CBagCreditsDialog() {
	_iScreen = 0;
	_pszNextLine = _pszEnd = nullptr;
	_pszText = nullptr;
	_nLines = 0;
	_nNumPixels = 0;
	_pCreditsBmp = nullptr;
	_pSaveBmp = nullptr;

	_bDisplay = false;
}

void CBagCreditsDialog::onInitDialog() {
	assert(isValidObject(this));

	// Hide cursor for credit screens
	CursorMan.showMouse(false);

	CBofDialog::onInitDialog();

	setReturnValue(-1);

	// Start at 1st credit screen
	_iScreen = 0;

	assert(_pBackdrop != nullptr);
	CBofPalette *pPal = _pBackdrop->getPalette();
	selectPalette(pPal);
	g_b1 = true;

	// Load 1st credit text file
	loadNextTextFile();
}

ErrorCode CBagCreditsDialog::loadNextTextFile() {
	assert(isValidObject(this));

	// Our credits text must exist
	assert(fileExists(buildSysDir(g_cScreen[_iScreen]._pszTextFile)));

	CBofRect cRect;
	cRect.left = g_cScreen[_iScreen]._nLeft;
	cRect.top = g_cScreen[_iScreen]._nTop;
	cRect.right = g_cScreen[_iScreen]._nRight;
	cRect.bottom = g_cScreen[_iScreen]._nBottom;

	// Get rid of any previous work area
	delete _pCreditsBmp;
	_pCreditsBmp = nullptr;

	// Create a new work area
	_pCreditsBmp = new CBofBitmap(cRect.width(), cRect.height() + LINE_HEIGHT + 2, _pBackdrop->getPalette());
	_pCreditsBmp->fillRect(nullptr, MY_MASK_COLOR);

	// Kill any previous work area
	delete _pSaveBmp;
	_pSaveBmp = new CBofBitmap(_pCreditsBmp->width(), _pCreditsBmp->height(), _pBackdrop->getPalette());

	CBofRect tmpRect = _pSaveBmp->getRect();
	_pBackdrop->paint(_pSaveBmp, &tmpRect, &cRect);

	// Get rid of any previous credits screen
	if (_pszText != nullptr) {
		bofFree(_pszText);
		_pszText = nullptr;
	}

	CBofFile cFile(buildSysDir(g_cScreen[_iScreen]._pszTextFile), CBF_BINARY | CBF_READONLY);

	if (!cFile.errorOccurred()) {
		// Read in text file
		uint32 lSize = cFile.getLength();
		_pszText = (char *)bofCleanAlloc(lSize + 1);

		cFile.read(_pszText, lSize);

		_pszNextLine = _pszText;
		_pszEnd = _pszText + lSize;
		_nNumPixels = 0;

		// Determine the number of lines of text in credits
		_nLines = strCharCount(_pszText, '\n');
		strreplaceChar(_pszText, '\r', ' ');
		strreplaceChar(_pszText, '\n', '\0');

		paintLine(linesPerPage() - 1, _pszNextLine);
		nextLine();
		paintLine(linesPerPage(), _pszNextLine);
		nextLine();

		_bDisplay = true;
	}

	return _errCode;
}

int CBagCreditsDialog::linesPerPage() {
	assert(isValidObject(this));

	int retVal = (g_cScreen[_iScreen]._nBottom - g_cScreen[_iScreen]._nTop) / (LINE_HEIGHT + 2) + 1;

	return retVal;
}

void CBagCreditsDialog::onClose() {
	assert(isValidObject(this));

	delete _pCreditsBmp;
	_pCreditsBmp = nullptr;

	delete _pSaveBmp;
	_pSaveBmp = nullptr;

	if (_pszText != nullptr) {
		bofFree(_pszText);
		_pszText = nullptr;
	}

	_pszNextLine = _pszEnd = nullptr;
	_nLines = 0;
	_nNumPixels = 0;

	_bDisplay = false;

	killBackdrop();

	CBofDialog::onClose();

	// Can have cursor back now
	CursorMan.showMouse(true);
}

void CBagCreditsDialog::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));

	paintBackdrop(pRect);

	validateAnscestors();
}

void CBagCreditsDialog::onLButtonDown(uint32 /*nFlags*/, CBofPoint */*pPoint*/, void *) {
	assert(isValidObject(this));

	nextScreen();
}

void CBagCreditsDialog::onKeyHit(uint32 /*lKey*/, uint32 /*nRepCount*/) {
	assert(isValidObject(this));

	nextScreen();
}

void CBagCreditsDialog::onMainLoop() {
	assert(isValidObject(this));

	// If it's OK to show the credits
	if (_bDisplay) {
		displayCredits();
	}

	// Check again...could have changed in displayCredits()
	if (_bDisplay) {
		// Control the scroll rate
		assert(_iScreen >= 0 && _iScreen < NUM_SCREENS);
		bofSleep(g_cScreen[_iScreen]._nScrollRate);
	}
}

ErrorCode CBagCreditsDialog::displayCredits() {
	assert(isValidObject(this));

	if (_nNumPixels < (_nLines + linesPerPage() + 1) * LINE_HEIGHT) {
		assert(_pCreditsBmp != nullptr);

		if (_pCreditsBmp != nullptr) {
			assert(_pBackdrop != nullptr);
			assert(_pSaveBmp != nullptr);

			_pSaveBmp->paint(_pBackdrop, g_cScreen[_iScreen]._nLeft, g_cScreen[_iScreen]._nTop);
			CBofRect cRect;
			cRect.setRect(0, 0, _pCreditsBmp->width() - 1, _pCreditsBmp->height() - 1 - (LINE_HEIGHT + 2));
			_pCreditsBmp->paint(_pBackdrop, g_cScreen[_iScreen]._nLeft, g_cScreen[_iScreen]._nTop, &cRect, MY_MASK_COLOR);

			if (g_b1) {
				_pBackdrop->paint(this, 0, 0);
				g_b1 = false;

			} else {
				cRect.left = g_cScreen[_iScreen]._nLeft;
				cRect.top = g_cScreen[_iScreen]._nTop;
				cRect.right = cRect.left + _pCreditsBmp->width() - 1;
				cRect.bottom = cRect.top + _pCreditsBmp->height() - 1 - (LINE_HEIGHT + 2);
				_pBackdrop->paint(this, &cRect, &cRect);
			}


			// Strip off top layer so it won't wrap around
			for (int i = 0; i < PIX_SCROLL_DY; i++) {
				_pCreditsBmp->line(0, i, _pCreditsBmp->width() - 1, i, MY_MASK_COLOR);
			}

			// Scroll text up 1 pixel
			_pCreditsBmp->scrollUp(PIX_SCROLL_DY);

			_nNumPixels += PIX_SCROLL_DY;
			if ((_nNumPixels % LINE_HEIGHT) == 0) {
				paintLine(linesPerPage(), _pszNextLine);
				nextLine();
			}
		}

	} else {
		nextScreen();
	}

	return _errCode;
}

ErrorCode CBagCreditsDialog::nextScreen() {
	assert(isValidObject(this));

	if (++_iScreen < NUM_SCREENS) {
		// Load next screen (flushes previous backdrop)
		CBofBitmap *pBmp = SpaceBar::loadBitmap(buildSysDir(g_cScreen[_iScreen]._pszBackground));
		if (pBmp != nullptr) {
			setBackdrop(pBmp);
			g_b1 = true;
		}

		// Load credit text for this screen
		loadNextTextFile();

	} else {
		// Since there are no more screens to show, then we are outta here
		_bDisplay = false;
		close();
	}

	return _errCode;
}

ErrorCode CBagCreditsDialog::paintLine(int nLine, char *pszText) {
	assert(isValidObject(this));
	assert(pszText != nullptr);
	assert(nLine >= 0 && nLine <= linesPerPage());
	assert(_pCreditsBmp != nullptr);

	CBofRect cRect;
	cRect.setRect(0, nLine * LINE_HEIGHT, _pCreditsBmp->width() - 1, (nLine + 1) * LINE_HEIGHT - 1);

	_pCreditsBmp->fillRect(&cRect, MY_MASK_COLOR);

	if (*pszText != '\0') {
		paintShadowedText(_pCreditsBmp, &cRect, pszText, g_cScreen[_iScreen]._nPointSize, TEXT_NORMAL, CTEXT_WHITE, JUSTIFY_CENTER);
	}

	return _errCode;
}

void CBagCreditsDialog::nextLine() {
	assert(isValidObject(this));
	assert(_pszNextLine != nullptr);

	if ((_pszNextLine != nullptr) && (_pszNextLine < _pszEnd)) {
		while (*_pszNextLine != '\0') {
			_pszNextLine++;
		}

		if (_pszNextLine < _pszEnd) {
			_pszNextLine++;
		}

		assert(_pszNextLine <= _pszEnd);
	}
}

} // namespace SpaceBar
} // namespace Bagel
