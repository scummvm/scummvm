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

// this function, defined in CBOFWIN.CPP, is also used as our Dialog Box Procedure
//
#if BOF_WINDOWS
LRESULT CALLBACK BofWindowProcedure(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
#endif


CBofDialog::CBofDialog() {
	// Inits
	//
	_pDlgBackground = nullptr;
	_bFirstTime = TRUE;
	_bTempBitmap = FALSE;
	_lFlags = BOFDLG_DEFAULT;
	_bEndDialog = FALSE;
	_bHavePainted = FALSE;
}


CBofDialog::CBofDialog(const CHAR *pszFileName, CBofRect *pRect, CBofWindow *pParent, const UINT nID, const uint32 lFlags) {
	Assert(pszFileName != nullptr);
	Assert(pParent != nullptr);

	CBofRect cRect;

	// Inits
	//
	_pDlgBackground = nullptr;
	_bFirstTime = TRUE;
	_bTempBitmap = FALSE;
	_lFlags = lFlags;
	_bEndDialog = FALSE;
	_bHavePainted = FALSE;

	CBofBitmap *pBmp;

	if ((pBmp = LoadBitmap(pszFileName)) != nullptr) {
		// use specified bitmap as this dialog's image
		SetBackdrop(pBmp);
	}

	if (pRect == nullptr) {
		Assert(m_pBackdrop != nullptr);
		cRect = m_pBackdrop->GetRect();
		pRect = &cRect;
	}

	// create the dialog box
	//
	if (Create("DialogBox", pRect->left, pRect->top, pRect->Width(), pRect->Height(), pParent, nID) == ERR_NONE) {
	}
}


CBofDialog::CBofDialog(CBofBitmap *pImage, CBofRect *pRect, CBofWindow *pParent, const UINT nID, const uint32 lFlags) {
	Assert(pImage != nullptr);
	Assert(pParent != nullptr);

	CBofRect cRect;

	// Inits
	//
	_pDlgBackground = nullptr;
	_bFirstTime = TRUE;
	_bTempBitmap = FALSE;
	_lFlags = lFlags;
	_bEndDialog = FALSE;
	_bHavePainted = FALSE;

	SetBackdrop(pImage);

	if (pRect == nullptr) {
		Assert(m_pBackdrop != nullptr);
		cRect = m_pBackdrop->GetRect();
		pRect = &cRect;
	}

	if (Create("DialogBox", pRect->left, pRect->top, pRect->Width(), pRect->Width(), pParent, nID) == ERR_NONE) {
	}
}


CBofDialog::~CBofDialog() {
	Assert(IsValidObject(this));

	if (_pDlgBackground != nullptr) {
		delete _pDlgBackground;
		_pDlgBackground = nullptr;
	}

	//
	// might have to validate the part of the parent window that we obscured
	//
}


ERROR_CODE CBofDialog::Create(const CHAR *pszName, INT x, INT y, INT nWidth, INT nHeight, CBofWindow *pParent, UINT nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	// dialog boxes must have parent windows
	Assert(pParent != nullptr);

	// Inits
	//
	_parent = pParent;
	m_nID = nControlID;

	// remember the name of this window
	strncpy(m_szTitle, pszName, MAX_TITLE);

	// Calculate effective bounds
	Common::Rect stRect(x, y, x + nWidth, y + nHeight);
	if (pParent != nullptr)
		stRect.translate(pParent->GetWindowRect().left,
						 pParent->GetWindowRect().top);

	m_cRect = stRect;
	delete _surface;
	_surface = new Graphics::ManagedSurface(*g_engine->_screen, stRect);

#if BOF_WINDOWS

	static BOOL bInit = FALSE;
	WNDCLASS wc;
	DWORD dwStyle, dwExStyle;
	HWND hParent;

	hParent = nullptr;
	dwStyle = WS_POPUP;
	if (pParent != nullptr) {
		//pParent = pParent->GetAnscestor();
		pParent->Disable();
		hParent = pParent->GetHandle();
	}

	// Register the Dialog-Window Class
	//
	if (!bInit) {
		BofMemSet(&wc, 0, sizeof(WNDCLASS));
		wc.lpszClassName = "BofDialog";
		wc.lpfnWndProc = BofWindowProcedure;
		wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
		wc.hInstance = CBofApp::GetInstanceHandle();

		if ((wc.hCursor = CBofApp::GetApp()->GetDefaultCursor().GetWinCursor()) == nullptr) {
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		}
		wc.hbrBackground = nullptr; //(HBRUSH)(COLOR_WINDOW + 1);
		RegisterClass(&wc);
		bInit = TRUE;
	}

	dwExStyle =  WS_EX_NOPARENTNOTIFY | WS_EX_TRANSPARENT;
	dwStyle |= DS_SYSMODAL | DS_NOIDLEMSG;

#if !BOF_WIN16
	dwStyle |= DS_SETFOREGROUND;
#endif

	// There must not already be a window (would cause bad resource leak)
	Assert(m_hWnd == nullptr);

	if ((m_hWnd = ::CreateWindowEx(dwExStyle, "BofDialog", m_szTitle, dwStyle, x, y, nWidth, nHeight, hParent, nullptr, CBofApp::GetInstanceHandle(), nullptr)) != nullptr) {

		RECT rect;
		::GetWindowRect(m_hWnd, &rect);

		m_cWindowRect = rect;

		m_cRect.SetRect(0, 0, m_cWindowRect.Width() - 1, m_cWindowRect.Height() - 1);

		SelectPalette(CBofApp::GetApp()->GetPalette());

		// center this dialog box
		Center();

		// save what the background behind the dialog box looks like
		SaveBackground();

	} else {
		ReportError(ERR_UNKNOWN, "Unable to CreateWindowEx(%s)", m_szTitle);
	}

#elif BOF_MAC
	UCHAR szBuf[256];
	Rect stRect = {y, x, y + nHeight, x + nWidth};

	// If this is a child window then convert it's area to global coordinates
	//
	if (pParent != nullptr) {

		stRect.left += pParent->GetWindowRect().left;
		stRect.right += pParent->GetWindowRect().left;
		stRect.top += pParent->GetWindowRect().top;
		stRect.bottom += pParent->GetWindowRect().top;
	}

	strcpy((CHAR *)szBuf, m_szTitle);
	StrCToPascal((CHAR *)szBuf);

	INT     winType = plainDBox;
	if (IsCustomWindow() == true) {
		winType = (16 * 1000) + 0;
	}

	if ((m_pWindow = NewCWindow(nullptr, &stRect, szBuf, FALSE, winType, WindowPtr(-1), FALSE, 0)) != nullptr) {

		SetPort(m_pWindow);

		m_cWindowRect.top = stRect.top;
		m_cWindowRect.left = stRect.left;
		m_cWindowRect.bottom = stRect.bottom - 1;
		m_cWindowRect.right = stRect.right - 1;

		m_cRect.SetRect(0, 0, m_cWindowRect.Width() - 1, m_cWindowRect.Height() - 1);

		SelectPalette(CBofApp::GetApp()->GetPalette());

		// center this dialog box
		Center();

		// save what the background behind the dialog box looks like
		SaveBackground();

	} else {
		ReportError(ERR_UNKNOWN, "Unable to NewCWindow(%s)", pszName);
	}

#endif

	return m_errCode;
}


ERROR_CODE CBofDialog::Create(const CHAR *pszName, CBofRect *pRect, CBofWindow *pParent, UINT nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	CBofRect cRect;
	INT x, y, nWidth, nHeight;

	x = y = 0;
	nWidth = nHeight = USE_DEFAULT;

	if (pRect == nullptr) {
		if (m_pBackdrop != nullptr) {
			cRect = m_pBackdrop->GetRect();
			pRect = &cRect;
		}
	}

	if (pRect != nullptr) {
		x = pRect->left;
		y = pRect->top;
		nWidth = pRect->Width();
		nHeight = pRect->Height();
	}

	return Create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
}


VOID CBofDialog::OnClose() {
	Assert(IsValidObject(this));

	// Release any capture/focus that was active
	Bagel::CBofApp *app = CBofApp::GetApp();
	app->setCaptureControl(nullptr);
	app->setFocusControl(nullptr);

	if (_parent != nullptr) {
		CBofWindow *pParent;
		pParent = _parent;
		pParent->Enable();

		// the parent window MUST now be enabled
		Assert(pParent->IsEnabled());
	}

	// if we saved the background, then paint it
	//
	if (_lFlags & BOFDLG_SAVEBACKGND) {
		PaintBackground();

		// need to validate the portion of the parent window that we obscured (but that we also have already repainted)
		//

		// Otherwise, we need to cause the parent to repaint itself
		//
	} else {

		if (_parent != nullptr) {
			_parent->InvalidateRect(nullptr);
		}
	}

	CBofWindow::OnClose();

	// stop our personal message loop
	_bEndDialog = TRUE;
}


ERROR_CODE CBofDialog::Paint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	// repaint the background behind the dialog
	//
	if (!_bFirstTime) {
		PaintBackground();
	}
	_bFirstTime = FALSE;

	// paint the dialog (uses bitmap instead of standard windows dialog)
	//
	if (HasBackdrop()) {

		PaintBackdrop(pRect, COLOR_WHITE);
	}

	return m_errCode;
}


ERROR_CODE CBofDialog::PaintBackground() {
	Assert(IsValidObject(this));

	// paint back the background
	//
	if (_pDlgBackground != nullptr) {

		m_errCode = _pDlgBackground->Paint(this, 0, 0);
	}

	return m_errCode;
}


ERROR_CODE CBofDialog::SaveBackground() {
	Assert(IsValidObject(this));

	if (_lFlags & BOFDLG_SAVEBACKGND) {

		CBofPalette *pPalette;

		pPalette = CBofApp::GetApp()->GetPalette();

		// Remove any previous background
		//
		if (_pDlgBackground != nullptr) {
			delete _pDlgBackground;
		}

		// save a copy of the background
		//
		if ((_pDlgBackground = new CBofBitmap(Width(), Height(), pPalette)) != nullptr) {
			_pDlgBackground->CaptureScreen(this, &m_cRect);
			_pDlgBackground->SetReadOnly(TRUE);

		} else {
			ReportError(ERR_MEMORY, "Unable to allocate a new CBofBitmap(%d x %d)", Width(), Height());
		}
	}
	_bFirstTime = FALSE;

	return m_errCode;
}


ERROR_CODE CBofDialog::KillBackground() {
	if (_pDlgBackground != nullptr) {
		delete _pDlgBackground;
		_pDlgBackground = nullptr;
	}

	return m_errCode;
}


VOID CBofDialog::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	Assert(pRect != nullptr);

	if (_bFirstTime) {
		SaveBackground();
	}

	Paint(pRect);

	_bHavePainted = TRUE;
}


INT CBofDialog::DoModal() {
	Assert(IsValidObject(this));

	// The dialog box must have been successfully created first
	Assert(IsCreated());

	CBofWindow *pLastActive = GetActiveWindow();
	SetActive();
	OnInitDialog();

	// display the window
	Show();

	UpdateWindow();

#if BOF_WINDOWS
	MSG msg;
#elif BOF_MAC
	RgnHandle cursorRgn = ::NewRgn();       // Need a mouse region
	CBofRect cRect(0, 0, 0, 0);
	EventRecord event;

	RectRgn(cursorRgn, &(cRect.GetMacRect()));
#endif

	// Start our own message loop (simulate Modal)
	//
	_bEndDialog = FALSE;

	// Acquire and dispatch messages until quit message is received,
	// or until there are too many errors.
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_bEndDialog && !g_engine->shouldQuit() && (CBofError::GetErrorCount() < MAX_ERRORS)) {
#if BOF_WINDOWS

		// if there is a message for our window, then process it
		//
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

#elif BOF_MAC

		// if there is a message for a window, then process it
		//
		if (WaitNextEvent(everyEvent, &event, 0xFFFFFFFF, cursorRgn) != 0) {

			// Convert the Mac event into a message our CBofWindows can handle.
			// HandleMacEvent returns TRUE when QUIT message is received.
			//
			// call our own instance of HandleMacEvent
			if (HandleMacEvent(&event))
				break;
		}

		//  Make sure to check out our timers.
		CBofWindow::HandleMacTimers();
#endif
		// calls macqt for us, no need to do it explicitly
		// Give some CPU time to the sound library
		CBofSound::AudioTask();

		// Give time to timers
		CBofTimer::HandleTimers();

		if (IsCreated()) {
#if BOF_MAC && PALETTESHIFTFIX
			CBofWindow::CheckPaletteShiftList();
#endif
			OnMainLoop();
		}

		handleEvents();

		limiter.delayBeforeSwap();
		g_engine->_screen->update();
		limiter.startFrame();
	}

#if BOF_MAC || BOF_WINMAC
	DisposeRgn(cursorRgn);          // get rid of cursor rgn
#endif
	if (pLastActive != nullptr) {
		pLastActive->SetActive();
	} else {
		m_pActiveWindow = nullptr;
	}

	return _nReturnValue;
}

#if BOF_MAC
BOOL CBofDialog::HandleMacEvent(EventRecord *pEvent) {
	switch (pEvent->what) {

	case keyUp:
		break;

	case autoKey:
	case keyDown:
		break;

	case updateEvt:
		break;

	case mouseDown:
	case mouseUp:

		// If not in our window, then reject it.
		//

#ifdef SPACEBAR
#else
		Rect    modalRect = m_cWindowRect.GetMacRect();

		if (!PtInRect(pEvent->where, &modalRect)) {
			if (pEvent->what == mouseDown)
				SysBeep(10);
			return false;
		}
		modalRect.left = modalRect.left;
		break;
#endif

	case app3Evt:
		break;

	case activateEvt:
		break;

	case osEvt:
		break;

	default:
		break;
	}

	return CBofWindow::HandleMacEvent(pEvent);
}
#endif

///////////////////////////////////////////////////////////////////
// Virtual functions that the user can override if they want to
///////////////////////////////////////////////////////////////////

VOID CBofDialog::OnInitDialog() {}

} // namespace Bagel
