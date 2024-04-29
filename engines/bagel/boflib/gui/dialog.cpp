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
	Assert(pszFileName != nullptr);
	Assert(pParent != nullptr);

	// Inits
	_pDlgBackground = nullptr;
	_bFirstTime = true;
	_bTempBitmap = false;
	_lFlags = lFlags;
	_bEndDialog = false;
	_bHavePainted = false;

	CBofBitmap *pBmp = LoadBitmap(pszFileName);

	if (pBmp != nullptr) {
		// Use specified bitmap as this dialog's image
		SetBackdrop(pBmp);
	}

	Assert(m_pBackdrop != nullptr);
	CBofRect cRect = m_pBackdrop->GetRect();

	// Create the dialog box
	create("DialogBox", cRect.left, cRect.top, cRect.Width(), cRect.Height(), pParent, nID);
}

CBofDialog::~CBofDialog() {
	Assert(IsValidObject(this));

	delete _pDlgBackground;
	_pDlgBackground = nullptr;
}


ErrorCode CBofDialog::create(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	// Dialog boxes must have parent windows
	Assert(pParent != nullptr);

	// Inits
	_parent = pParent;
	m_nID = nControlID;

	// Remember the name of this window
	strncpy(m_szTitle, pszName, MAX_TITLE);

	// Calculate effective bounds
	Common::Rect stRect(x, y, x + nWidth, y + nHeight);
	if (pParent != nullptr)
		stRect.translate(pParent->GetWindowRect().left,
						 pParent->GetWindowRect().top);

	m_cRect = stRect;
	delete _surface;
	_surface = new Graphics::ManagedSurface(*g_engine->_screen, stRect);

	return m_errCode;
}


ErrorCode CBofDialog::create(const char *pszName, CBofRect *pRect, CBofWindow *pParent, uint32 nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	CBofRect cRect;
	int x = 0;
	int y = 0;
	int nWidth = USE_DEFAULT;
	int nHeight = USE_DEFAULT;

	if ((pRect == nullptr) && (m_pBackdrop != nullptr)) {
		cRect = m_pBackdrop->GetRect();
		pRect = &cRect;
	}

	if (pRect != nullptr) {
		x = pRect->left;
		y = pRect->top;
		nWidth = pRect->Width();
		nHeight = pRect->Height();
	}

	return create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
}


void CBofDialog::onClose() {
	Assert(IsValidObject(this));

	// Release any capture/focus that was active
	CBofApp *app = CBofApp::GetApp();
	app->setCaptureControl(nullptr);
	app->setFocusControl(nullptr);

	if (_parent != nullptr) {
		CBofWindow *pParent = _parent;
		pParent->Enable();

		// The parent window MUST now be enabled
		Assert(pParent->IsEnabled());
	}

	// If we saved the background, then paint it
	if (_lFlags & BOFDLG_SAVEBACKGND) {
		paintBackground();

	} else if (_parent != nullptr) {
		// Need to validate the portion of the parent window that we obscured
		// (but that we also have already repainted)
		// Otherwise, we need to cause the parent to repaint itself
		_parent->InvalidateRect(nullptr);
	}

	CBofWindow::onClose();

	// Stop our personal message loop
	_bEndDialog = true;
}


ErrorCode CBofDialog::Paint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	// Repaint the background behind the dialog
	if (!_bFirstTime) {
		paintBackground();
	}
	_bFirstTime = false;

	// Paint the dialog (uses bitmap instead of standard windows dialog)
	if (HasBackdrop()) {
		PaintBackdrop(pRect, COLOR_WHITE);
	}

	return m_errCode;
}


ErrorCode CBofDialog::paintBackground() {
	Assert(IsValidObject(this));

	// Paint back the background
	if (_pDlgBackground != nullptr) {
		m_errCode = _pDlgBackground->Paint(this, 0, 0);
	}

	return m_errCode;
}


ErrorCode CBofDialog::saveBackground() {
	Assert(IsValidObject(this));

	if (_lFlags & BOFDLG_SAVEBACKGND) {
		CBofPalette *pPalette = CBofApp::GetApp()->GetPalette();

		// Remove any previous background
		delete _pDlgBackground;
		// Save a copy of the background
		_pDlgBackground = new CBofBitmap(Width(), Height(), pPalette);
		if (_pDlgBackground != nullptr) {
			_pDlgBackground->CaptureScreen(this, &m_cRect);
			_pDlgBackground->SetReadOnly(true);

		} else {
			ReportError(ERR_MEMORY, "Unable to allocate a new CBofBitmap(%d x %d)", Width(), Height());
		}
	}

	_bFirstTime = false;

	return m_errCode;
}


ErrorCode CBofDialog::killBackground() {
	delete _pDlgBackground;
	_pDlgBackground = nullptr;

	return m_errCode;
}


void CBofDialog::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	if (_bFirstTime) {
		saveBackground();
	}

	Paint(pRect);

	_bHavePainted = true;
}


int CBofDialog::doModal() {
	Assert(IsValidObject(this));

	// The dialog box must have been successfully created first
	Assert(IsCreated());

	CBofWindow *pLastActive = GetActiveWindow();
	SetActive();
	onInitDialog();

	// Display the window
	Show();

	UpdateWindow();

	// Start our own message loop (simulate Modal)
	_bEndDialog = false;

	// Acquire and dispatch messages until quit message is received,
	// or until there are too many errors.
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_bEndDialog && !g_engine->shouldQuit() && (CBofError::GetErrorCount() < MAX_ERRORS)) {
		CBofSound::AudioTask();
		CBofTimer::HandleTimers();

		if (IsCreated()) {
			onMainLoop();
		}

		handleEvents();

		limiter.delayBeforeSwap();
		g_engine->_screen->update();
		limiter.startFrame();
	}

	if (pLastActive != nullptr) {
		pLastActive->SetActive();
	} else {
		m_pActiveWindow = nullptr;
	}

	return _nReturnValue;
}

///////////////////////////////////////////////////////////////////
// Virtual functions that the user can override if they want to
///////////////////////////////////////////////////////////////////

void CBofDialog::onInitDialog() {}

} // namespace Bagel
