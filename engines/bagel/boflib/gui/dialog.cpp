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

#include "common/system.h"
#include "common/events.h"
#include "graphics/framelimiter.h"
#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/timer.h"
#include "bagel/boflib/sound.h"
#include "bagel/bagel.h"

namespace Bagel {

CBofDialog::CBofDialog() {
	// Inits
	_pDlgBackground = nullptr;
	_bFirstTime = true;
	_bTempBitmap = false;
	_lFlags = BOFDLG_DEFAULT;
	_bEndDialog = false;
	_bHavePainted = false;
}


CBofDialog::CBofDialog(const char *pszFileName, CBofWindow *pParent, const uint32 nID, const uint32 lFlags) {
	assert(pszFileName != nullptr);
	assert(pParent != nullptr);

	// Inits
	_pDlgBackground = nullptr;
	_bFirstTime = true;
	_bTempBitmap = false;
	_lFlags = lFlags;
	_bEndDialog = false;
	_bHavePainted = false;

	CBofBitmap *pBmp = loadBitmap(pszFileName);

	if (pBmp != nullptr) {
		// Use specified bitmap as this dialog's image
		setBackdrop(pBmp);
	}

	assert(_pBackdrop != nullptr);
	CBofRect cRect = _pBackdrop->getRect();

	// Create the dialog box
	create("DialogBox", cRect.left, cRect.top, cRect.width(), cRect.height(), pParent, nID);
}

CBofDialog::~CBofDialog() {
	assert(isValidObject(this));

	delete _pDlgBackground;
	_pDlgBackground = nullptr;
}


ErrorCode CBofDialog::create(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID) {
	assert(isValidObject(this));
	assert(pszName != nullptr);

	// Dialog boxes must have parent windows
	assert(pParent != nullptr);

	// Inits
	_parent = pParent;
	_nID = nControlID;

	// Remember the name of this window
	Common::strlcpy(_szTitle, pszName, MAX_TITLE);

	// Calculate effective bounds
	Common::Rect stRect(x, y, x + nWidth, y + nHeight);
	if (pParent != nullptr)
		stRect.translate(pParent->getWindowRect().left, pParent->getWindowRect().top);

	_cRect = stRect;

	delete _surface;
	_surface = new Graphics::ManagedSurface(*g_engine->_screen, stRect);

	return _errCode;
}


ErrorCode CBofDialog::create(const char *pszName, CBofRect *pRect, CBofWindow *pParent, uint32 nControlID) {
	assert(isValidObject(this));
	assert(pszName != nullptr);

	CBofRect cRect;
	int x = 0;
	int y = 0;
	int nWidth = USE_DEFAULT;
	int nHeight = USE_DEFAULT;

	if ((pRect == nullptr) && (_pBackdrop != nullptr)) {
		cRect = _pBackdrop->getRect();
		pRect = &cRect;
	}

	if (pRect != nullptr) {
		x = pRect->left;
		y = pRect->top;
		nWidth = pRect->width();
		nHeight = pRect->height();
	}

	return create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
}


void CBofDialog::onClose() {
	assert(isValidObject(this));

	// Release any capture/focus that was active
	CBofApp *app = CBofApp::getApp();
	app->setCaptureControl(nullptr);
	app->setFocusControl(nullptr);

	if (_parent != nullptr) {
		CBofWindow *pParent = _parent;
		pParent->enable();

		// The parent window MUST now be enabled
		assert(pParent->isEnabled());
	}

	// If we saved the background, then paint it
	if (_lFlags & BOFDLG_SAVEBACKGND) {
		paintBackground();

	} else if (_parent != nullptr) {
		// Need to validate the portion of the parent window that we obscured
		// (but that we also have already repainted)
		// Otherwise, we need to cause the parent to repaint itself
		_parent->invalidateRect(nullptr);
	}

	CBofWindow::onClose();

	// Stop our personal message loop
	_bEndDialog = true;
}


ErrorCode CBofDialog::paint(CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pRect != nullptr);

	// Repaint the background behind the dialog
	if (!_bFirstTime) {
		paintBackground();
	}
	_bFirstTime = false;

	// Paint the dialog (uses bitmap instead of standard windows dialog)
	if (hasBackdrop()) {
		paintBackdrop(pRect, COLOR_WHITE);
	}

	return _errCode;
}


ErrorCode CBofDialog::paintBackground() {
	assert(isValidObject(this));

	// Paint back the background
	if (_pDlgBackground != nullptr) {
		_errCode = _pDlgBackground->paint(this, 0, 0);
	}

	return _errCode;
}


ErrorCode CBofDialog::saveBackground() {
	assert(isValidObject(this));

	if (_lFlags & BOFDLG_SAVEBACKGND) {
		CBofPalette *pPalette = CBofApp::getApp()->getPalette();

		// Remove any previous background
		delete _pDlgBackground;
		// Save a copy of the background
		_pDlgBackground = new CBofBitmap(width(), height(), pPalette);

		_pDlgBackground->captureScreen(this, &_cRect);
		_pDlgBackground->setReadOnly(true);
	}

	_bFirstTime = false;

	return _errCode;
}


ErrorCode CBofDialog::killBackground() {
	delete _pDlgBackground;
	_pDlgBackground = nullptr;

	return _errCode;
}


void CBofDialog::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pRect != nullptr);

	if (_bFirstTime) {
		saveBackground();
	}

	paint(pRect);

	_bHavePainted = true;
}


int CBofDialog::doModal() {
	assert(isValidObject(this));

	// The dialog box must have been successfully created first
	assert(isCreated());

	CBofWindow *pLastActive = getActiveWindow();
	setActive();
	onInitDialog();

	// Display the window
	show();

	updateWindow();

	// Start our own message loop (simulate Modal)
	_bEndDialog = false;

	// Acquire and dispatch messages until quit message is received,
	// or until there are too many errors.
	Graphics::FrameLimiter limiter(g_system, 60, false);

	while (!_bEndDialog && !g_engine->shouldQuit() && (CBofError::getErrorCount() < MAX_ERRORS)) {
		CBofSound::audioTask();
		CBofTimer::handleTimers();

		if (isCreated()) {
			onMainLoop();
		}

		handleEvents();

		limiter.delayBeforeSwap();
		g_engine->_screen->update();
		limiter.startFrame();
	}

	if (pLastActive != nullptr) {
		pLastActive->setActive();
	} else {
		_pActiveWindow = nullptr;
	}

	return _nReturnValue;
}

///////////////////////////////////////////////////////////////////
// Virtual functions that the user can override if they want to
///////////////////////////////////////////////////////////////////

void CBofDialog::onInitDialog() {}

} // namespace Bagel
