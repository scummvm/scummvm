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

#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/events.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/bagel.h"
#include "bagel/metaengine.h"
#include "bagel/bagel.h"

namespace Bagel {

#define DOUBLE_CLICK_TIME 250

// Static members defined here
CBofWindow *CBofWindow::m_pWindowList = nullptr;
CBofWindow *CBofWindow::m_pActiveWindow = nullptr;
CBofTimerPacket *CBofWindow::m_pTimerList = nullptr;
int CBofWindow::_mouseX = 0;
int CBofWindow::_mouseY = 0;
static const CBofRect viewPortRect(80, 10, 559, 369);

#if BOF_MAC
CBofWindow *CBofWindow::m_pCapturedWindow = nullptr;
#endif

CBofWindow::CBofWindow() {
	if (m_pActiveWindow == nullptr)
		m_pActiveWindow = this;

	if (m_pWindowList == nullptr) {
		m_pWindowList = this;
	} else {
		m_pWindowList->Insert(this);
	}
}

CBofWindow::CBofWindow(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent) {
	if (m_pWindowList == nullptr) {
		m_pWindowList = this;
	} else {
		m_pWindowList->Insert(this);
	}

	Create(pszName, x, y, nWidth, nHeight, pParent);
}

CBofWindow::~CBofWindow() {
	Assert(IsValidObject(this));

	delete _surface;
	_surface = nullptr;

	KillMyTimers();

	// Remove it from any parent
	if (_parent != nullptr)
		setParent(nullptr);

	// Remove this window from the list
	if (m_pWindowList == this) {
		m_pWindowList = (CBofWindow *)GetNext();
	}

	KillBackdrop();

	Destroy();
}

ErrorCode CBofWindow::initialize() {
	m_pWindowList = nullptr;
	m_pActiveWindow = nullptr;
	m_pTimerList = nullptr;

	return ERR_NONE;
}

ErrorCode CBofWindow::shutdown() {
	return ERR_NONE;
}

Common::Point CBofWindow::getMousePos() {
	return Common::Point(_mouseX, _mouseY);
}

void CBofWindow::Destroy() {
	ReleaseCapture();

	if (_surface != nullptr) {
		delete _surface;
		_surface = nullptr;
	}

	// When gui elements are destroyed, remove them
	// from the _children array of their parent
	setParent(nullptr);
}

void CBofWindow::ValidateAnscestors(CBofRect *pRect) {
	Assert(IsValidObject(this));

	CBofWindow *pParent;

	// Validate all anscestors
	pParent = _parent;
	while (pParent != nullptr) {
#if BOF_MAC || BOF_WINMAC
		//  On the mac, we have to make sure that
		//  the grafport is our current grafport (i.e. the window
		//  that we are validating).

		if (pParent->m_pWindow) {
#endif
			pParent->ValidateRect(pRect);

#if BOF_MAC || BOF_WINMAC
		}
#endif
		pParent = pParent->GetParent();
	}
}

ErrorCode CBofWindow::Create(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);
	Assert(pParent != this);

	// Remember who our parent is
	if (pParent != nullptr)
		setParent(pParent);

	m_nID = nControlID;

	// Remember the name of this window
	strncpy(m_szTitle, pszName, MAX_TITLE);

	// Retain screen coordinates for this window
	m_cWindowRect.SetRect(x, y, x + nWidth - 1, y + nHeight - 1);

	// Calculate effective bounds
	Common::Rect stRect(x, y, x + nWidth, y + nHeight);
	if (pParent != nullptr)
		stRect.translate(pParent->GetWindowRect().left,
			pParent->GetWindowRect().top);

	delete _surface;
	_surface = new Graphics::ManagedSurface(*g_engine->_screen, stRect);

#if BOF_MAC
	byte szBuf[256];
	Rect stRect = {y, x, y + nHeight, x + nWidth};

	// If this is a child window then convert it's area to global coordinates
	//
	if (pParent != nullptr) {

		stRect.left += pParent->GetWindowRect().left;
		stRect.right += pParent->GetWindowRect().left;
		stRect.top += pParent->GetWindowRect().top;
		stRect.bottom += pParent->GetWindowRect().top;
	}

	strcpy((char *)szBuf, m_szTitle);
	StrCToPascal((char *)szBuf);

	int winType = plainDBox;
	if (IsCustomWindow() == true) {
		winType = (16 * 1000) + 0;
	}

	if ((m_pWindow = NewCWindow(nullptr, &stRect, szBuf, false, winType, WindowPtr(-1), false, 0)) != nullptr) {

		// SetWRefCon(m_pWindow, this);

		SetPort(m_pWindow);

		// ForeColor(blackColor);
		// BackColor(whiteColor);
		// PenNormal();

		// set this window's global coordinates
		m_cWindowRect = stRect;

	} else {
		ReportError(ERR_UNKNOWN, "Unable to NewCWindow(%s)", pszName);
	}

#endif

	if (!ErrorOccurred()) {
		CBofPalette *pPalette;
		if ((pPalette = CBofApp::GetApp()->GetPalette()) != nullptr) {
			SelectPalette(pPalette);
		}

		// Retain local coordinates (based on own window)
		m_cRect.SetRect(0, 0, m_cWindowRect.Width() - 1, m_cWindowRect.Height() - 1);
	}

	return m_errCode;
}

void CBofWindow::UpdateWindow() {
	if (_visible) {
		if (IsVisible())
			OnPaint(&m_cRect);

		for (uint i = 0; i < _children.size(); ++i)
			_children[i]->UpdateWindow();
	}
}

void CBofWindow::setParent(CBofWindow *parent) {
	if (_parent != nullptr)
		_parent->_children.remove(this);

	_parent = parent;
	if (parent)
		parent->_children.push_back(this);
}


ErrorCode CBofWindow::Create(const char *pszName, const CBofRect *pRect, CBofWindow *pParent, uint32 nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	int x, y, nWidth, nHeight;

	x = y = 0;
	nWidth = nHeight = USE_DEFAULT;

	if (pRect != nullptr) {
		x = pRect->left;
		y = pRect->top;
		nWidth = pRect->Width();
		nHeight = pRect->Height();
	}

	return Create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
}

void CBofWindow::ReleaseCapture() {
	m_bCaptured = false;
	if (HasCapture())
		CBofApp::GetApp()->setCaptureControl(nullptr);
}

void CBofWindow::SetCapture() {
	m_bCaptured = true;
	CBofApp::GetApp()->setCaptureControl(this);
}

bool CBofWindow::HasCapture() const {
	return CBofApp::GetApp()->getCaptureControl() == this;
}

void CBofWindow::ReleaseFocus() {
	CBofApp::GetApp()->setFocusControl(nullptr);
}

void CBofWindow::SetFocus() {
	CBofApp::GetApp()->setFocusControl(this);
}

bool CBofWindow::HasFocus() const {
	return CBofApp::GetApp()->getFocusControl() == this;
}

void CBofWindow::Center() {
	Assert(IsValidObject(this));

	CBofWindow *pParent;
	int x, y;

	if ((pParent = _parent) != nullptr) {
		CBofRect cWindowRect;

		cWindowRect = pParent->GetWindowRect();
		x = cWindowRect.left + (pParent->Width() - Width()) / 2;
		y = cWindowRect.top + (pParent->Height() - Height()) / 2;

	} else {
		x = (CBofApp::GetApp()->ScreenWidth() - Width()) / 2;
		y = (CBofApp::GetApp()->ScreenHeight() - Height()) / 2;
	}

	Move(x, y);
}

void CBofWindow::Move(const int x, const int y, bool bRepaint) {
	Assert(IsValidObject(this));
	Assert(IsCreated());

	// We now have a new position (in screen coordinates)
	m_cWindowRect.SetRect(x, y, x + m_cRect.Width() - 1, y + m_cRect.Height() - 1);

	// Recreate the surface at the new screen position
	delete _surface;
	_surface = new Graphics::ManagedSurface(*g_engine->_screen, m_cWindowRect);
}

void CBofWindow::ReSize(CBofRect *pRect, bool bRepaint) {
	Assert(IsValidObject(this));
	Assert(IsCreated());
	Assert(pRect != nullptr);

	// We now have a new position (in screen coordinates)
	m_cWindowRect = *pRect;
	m_cRect.SetRect(0, 0, m_cWindowRect.Width() - 1, m_cWindowRect.Height() - 1);

	// Recreate the surface at the new screen position
	delete _surface;
	_surface = new Graphics::ManagedSurface(*g_engine->_screen, m_cWindowRect);
}

void CBofWindow::Select() {
#if BOF_MAC
	::SelectWindow(m_pWindow);
	::SetPort(m_pWindow);
#endif
}

void CBofWindow::Show() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		Assert(IsCreated());

		if (IsCreated()) {
			_visible = true;
			InvalidateRect(&m_cRect);
		}
	}
}

void CBofWindow::Hide() {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		Assert(IsCreated());

		_visible = false;
	}
}

void CBofWindow::PostMessage(uint32 nMessage, uint32 lParam1, uint32 lParam2) {
	Assert(IsValidObject(this));
	Assert(IsCreated());

#if BOF_MAC

	CBofMessage *pMessage;

	// Create a user defined message.
	// NOTE: This message will be deleted by HandleMacEvent()
	//
	if ((pMessage = new CBofMessage) != nullptr) {

		pMessage->m_pWindow = this;
		pMessage->m_nMessage = nMessage;
		pMessage->m_lParam1 = lParam1;
		pMessage->m_lParam2 = lParam2;

		PostEvent(app3Evt, (int32)pMessage);
	}
#endif
}

#if BOF_MAC
#define GETTIME() (uint32)(16.66 * TickCount())
#endif

void CBofWindow::SetTimer(uint32 nID, uint32 nInterval, BofCallback pCallBack) {
	Assert(IsValidObject(this));
	Assert(IsCreated());

	// Don't add it if there's already a timer there with the same id.
	CBofTimerPacket *pPacket = m_pTimerList;

	while (pPacket != nullptr) {
		if (pPacket->m_nID == nID)
			return;

		pPacket = (CBofTimerPacket *)pPacket->GetNext();
	}

	if ((pPacket = new CBofTimerPacket) != nullptr) {
		pPacket->m_nID = nID;
		pPacket->m_nInterval = nInterval;
		pPacket->m_pCallBack = pCallBack;
		pPacket->m_pOwnerWindow = this;

		// Add this timer to the list of current timers
		if (m_pTimerList != nullptr) {
			m_pTimerList->AddToHead(pPacket);
		}

		m_pTimerList = pPacket;
	}

	// Add the timer to the window
	_timers.push_back(WindowTimer(nInterval, nID, pCallBack));
}

void CBofWindow::KillTimer(uint32 nID) {
	Assert(IsValidObject(this));

	// Remove the timer from the window timer list
	for (Common::List<WindowTimer>::iterator it = _timers.begin(); it != _timers.end(); ++it) {
		if (it->_id == nID) {
			_timers.erase(it);
			break;
		}
	}

	// Find and remove the timer packet for this timer
	CBofTimerPacket *pPacket = m_pTimerList;

	while (pPacket != nullptr) {
		if (pPacket->m_nID == nID) {
			if (pPacket == m_pTimerList) {
				m_pTimerList = (CBofTimerPacket *)m_pTimerList->GetNext();
			}

			delete pPacket;
			break;
		}

		pPacket = (CBofTimerPacket *)pPacket->GetNext();
	}
}

void CBofWindow::KillMyTimers() {
	Assert(IsValidObject(this));

	CBofTimerPacket *pTimer, *pNextTimer;

	pTimer = m_pTimerList;
	while (pTimer != nullptr) {
		pNextTimer = (CBofTimerPacket *)pTimer->GetNext();

		if (pTimer->m_pOwnerWindow == this) {
			KillTimer(pTimer->m_nID);
		}

		pTimer = pNextTimer;
	}
}

void CBofWindow::CheckTimers() {
	uint32 currTime;

	for (uint i = 0; i < _children.size(); ++i)
		_children[i]->CheckTimers();

	for (bool timersChanged = true; timersChanged;) {
		timersChanged = false;
		currTime = g_system->getMillis();

		// Iterate over the timers looking for any that have expired
		for (Common::List<WindowTimer>::iterator it = _timers.begin(); it != _timers.end(); ++it) {
			WindowTimer &timer = *it;

			if (currTime >= (timer._lastExpiryTime + timer._interval)) {
				// Timer has expired
				timer._lastExpiryTime = currTime;

				if (timer._callback) {
					(timer._callback)(timer._id, this);
				} else {
					OnTimer(timer._id);
				}

				// Flag to restart scanning through the timer list
				// for any other expired timers, since the timer call
				// may have modified the existing list
				timersChanged = true;
				break;
			}
		}
	}
}

void CBofWindow::ScreenToClient(CBofPoint *pPoint) {
	Assert(IsValidObject(this));

	Assert(pPoint != nullptr);

#if BOF_MAC
	// make sure that our frontmost window is our current
	// grafport.

	Point stPoint;
	GrafPtr savePort;

	//::GetPort (&savePort);
	//::SetPort (m_pWindow);

	stPoint.v = pPoint->y;
	stPoint.h = pPoint->x;

	GlobalToLocal(&stPoint);

	//::SetPort (savePort);

	pPoint->y = stPoint.v;
	pPoint->x = stPoint.h;
#endif
}

void CBofWindow::ClientToScreen(CBofPoint *pPoint) {
	Assert(IsValidObject(this));

	Assert(pPoint != nullptr);
}

CBofRect CBofWindow::GetClientRect() {
	Assert(IsValidObject(this));

	CBofRect cRect(0, 0, m_cRect.Width() - 1, m_cRect.Height() - 1);

	return cRect;
}

void CBofWindow::PostUserMessage(uint32 lMessage, uint32 lExtraInfo) {
	Common::Event e;
	e.type = (Common::EventType)EVENT_USER;
	e.mouse.x = lMessage;
	e.mouse.y = lExtraInfo;

	g_system->getEventManager()->pushEvent(e);
}

CBofWindow *CBofWindow::GetAnscestor() {
	Assert(IsValidObject(this));

	CBofWindow *pCurWnd, *pLastWnd;

	pLastWnd = this;
	pCurWnd = _parent;

	while (pCurWnd != nullptr) {
		pLastWnd = pCurWnd;

		pCurWnd = pCurWnd->_parent;
	}

	return pLastWnd;
}

void CBofWindow::FlushAllMessages() {
	// Make sure this is a valid window
	Assert(IsValidObject(this));
	Assert(IsCreated());

#if BOF_MAC
	FlushEvents(0xFFFF, 0);
#endif
}

void CBofWindow::ValidateRect(const CBofRect *pRect) {
#if BOF_MAC
	{
		// set current port... don't require caller to do this.
		STBofPort stSavePort(GetMacWindow());
		if (pRect == nullptr)
			pRect = &m_cRect;

		Rect stRect = {pRect->left, pRect->top, pRect->right + 1, pRect->bottom + 1};

		::ValidRect(&stRect);
	}
#endif
}

void CBofWindow::InvalidateRect(const CBofRect *pRect) {
}

ErrorCode CBofWindow::SetBackdrop(CBofBitmap *pNewBitmap, bool bRefresh) {
	Assert(IsValidObject(this));
	Assert(pNewBitmap != nullptr);

	// Destroy old backdrop (if any)
	KillBackdrop();

	// We take ownership of this bitmap!
	m_pBackdrop = pNewBitmap;

	if (bRefresh) {
		m_pBackdrop->Paint(this, 0, 0);
	}

	return m_errCode;
}

ErrorCode CBofWindow::SetBackdrop(const char *pszFileName, bool bRefresh) {
	Assert(IsValidObject(this));
	Assert(pszFileName != nullptr);

	CBofBitmap *pBmp;
	CBofPalette *pPalette;

	// Use Application's palette if none supplied
	pPalette = CBofApp::GetApp()->GetPalette();

	if ((pBmp = new CBofBitmap(pszFileName, pPalette)) != nullptr) {
		return SetBackdrop(pBmp, bRefresh);

	} else {
		ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap");
	}

	return m_errCode;
}

void CBofWindow::KillBackdrop() {
	Assert(IsValidObject(this));

	if (m_pBackdrop != nullptr) {
		delete m_pBackdrop;
		m_pBackdrop = nullptr;
	}
}

ErrorCode CBofWindow::PaintBackdrop(CBofRect *pRect, int nTransparentColor) {
	Assert(IsValidObject(this));

	if (m_pBackdrop != nullptr) {
		if (pRect == nullptr) {
			m_errCode = m_pBackdrop->Paint(this, &m_cRect, nullptr, nTransparentColor);

		} else {
			m_errCode = m_pBackdrop->Paint(this, pRect, pRect, nTransparentColor);
		}
	}

	return m_errCode;
}

void CBofWindow::SelectPalette(CBofPalette *pPal) {
	Assert(IsValidObject(this));
	Assert(IsCreated());

	if (pPal != nullptr) {
		Assert(IsValidObject(pPal));

#if BOF_MAC
		Assert(pPal != nullptr);
		Assert(m_pWindow != nullptr);
		PaletteHandle newPH = pPal->GetPalette();

		// have seen newPH nullptr, if it is, the default 256 colors
		// of the game are used (see 'pltt' resource)
		if (newPH != nullptr) {
			Assert((*newPH)->pmEntries == 256);
			Assert(GetHandleSize((Handle)newPH) != 0);

			// palette shift fix... take all the calls that cause palette shifts
			// and move them as close to the onscreen rendering code as possible, this will
			// minimize the shift... but not eliminate it.

#if PALETTESHIFTFIX
			// simplified using AddToPaletteShiftList

			AddToPaletteShiftList(SETPALETTE, (int32)newPH, (int32)m_pWindow);
#else
			::SetPalette(m_pWindow, newPH, false);
#endif
		}
#endif
	}
}

#if BOF_WINMAC
bool CBofWindow::SetMacPalette(CBofPalette *pPalette) {
	Assert(IsValidObject(this));
	Assert(pPalette != nullptr);
	Assert(pPalette->GetPalette() != nullptr);

	WindowPtr myWindow;
	CGrafPtr pThisPort;
	GrafPtr pSavePort;
	PALETTEENTRY PalEntries[256];
	int i;
	OSErr err;
	static PaletteHandle hMainMacPal = nullptr;
	PaletteHandle hOldMainMacPal;
	HWND hMainWnd;
	HPALETTE hMainPal;
	CTabHandle hMainCLUT;

	hMainWnd = m_hWnd;
	hMainPal = (HPALETTE)pPalette->GetPalette();

	myWindow = GetWrapperWindow(hMainWnd);
	pThisPort = (CGrafPtr)myWindow;

	GetPort(&pSavePort);
	SetPort((GrafPort *)pThisPort);

	LockPixels(pThisPort->portPixMap);
	hMainCLUT = (**(*pThisPort).portPixMap).pmTable;
	UnlockPixels(pThisPort->portPixMap);
	err = HandToHand((char ***)&hMainCLUT);
	if (err != noErr) {
		SetPort(pSavePort);
		return false;
	}

	::GetPaletteEntries(hMainPal, 0, 256, PalEntries);
	for (i = 0; i < (*hMainCLUT)->ctSize; i++) {
		(*hMainCLUT)->ctTable[i].rgb.red = (uint16)((PalEntries[i].peRed * 0xFFFF) / 0xFF);
		(*hMainCLUT)->ctTable[i].rgb.green = (uint16)((PalEntries[i].peGreen * 0xFFFF) / 0xFF);
		(*hMainCLUT)->ctTable[i].rgb.blue = (uint16)((PalEntries[i].peBlue * 0xFFFF) / 0xFF);
		(*hMainCLUT)->ctTable[i].value = (int16)i;

		PalEntries[i].peFlags = (byte)(PC_EXPLICIT | i);
	}
	::SetPaletteEntries(hMainPal, 0, 256, PalEntries);

	(*hMainCLUT)->ctFlags = 0x4000;
	(*hMainCLUT)->ctSeed = GetCTSeed();

	hOldMainMacPal = hMainMacPal;
	hMainMacPal = NewPalette(256, hMainCLUT, pmExplicit | pmTolerant, 0x0000);
	if (hMainMacPal == nullptr) {
		DisposCTable(hMainCLUT);
		SetPort(pSavePort);
		return false;

	} else {

		if (hOldMainMacPal && (hOldMainMacPal = ::GetPalette(myWindow)) != nullptr)
			DisposePalette(hOldMainMacPal);
	}

	SetPalette((WindowPtr) - 1, hMainMacPal, true);

	SetPalette(myWindow, hMainMacPal, true);
	ActivatePalette(myWindow);

	SetPort(pSavePort);

	g_bRealizePalette = false;

	return true;
}
#endif

Graphics::ManagedSurface *CBofWindow::getSurface() {
	return _surface;
}


// Default version of these virtual functions don't do anything
//
void CBofWindow::OnMouseMove(uint32, CBofPoint *, void *) {}

void CBofWindow::OnLButtonDown(uint32, CBofPoint *, void *) {}
void CBofWindow::OnLButtonUp(uint32, CBofPoint *, void *) {}
void CBofWindow::OnLButtonDblClk(uint32, CBofPoint *) {}

void CBofWindow::OnRButtonDown(uint32, CBofPoint *) {}
void CBofWindow::OnRButtonUp(uint32, CBofPoint *) {}
void CBofWindow::OnRButtonDblClk(uint32, CBofPoint *) {}

void CBofWindow::OnKeyHit(uint32, uint32) {}

void CBofWindow::OnReSize(CBofSize *) {}
void CBofWindow::OnPaint(CBofRect *) {}
void CBofWindow::OnTimer(uint32) {}

void CBofWindow::OnClose() {}

void CBofWindow::OnBofButton(CBofObject *, int) {}
void CBofWindow::OnBofScrollBar(CBofObject *, int) {}
void CBofWindow::OnBofListBox(CBofObject *, int) {}
void CBofWindow::OnUserMessage(uint32, uint32) {}
void CBofWindow::OnMainLoop() {}

void CBofWindow::OnSoundNotify(CBofObject *, uint32) {}
void CBofWindow::OnMovieNotify(uint32, uint32) {}

void CBofWindow::OnActivate() {}
void CBofWindow::OnDeActivate() {}

void CBofWindow::OnMCINotify(uint32 wParam, uint32 lParam) {
	Assert(IsValidObject(this));
}

void CBofWindow::handleEvents() {
	Common::Event e;
	CBofWindow *capture = CBofApp::GetApp()->getCaptureControl();
	CBofWindow *focus = CBofApp::GetApp()->getFocusControl();

	// Check for expired timers before handling events
	CheckTimers();

	while (g_system->getEventManager()->pollEvent(e)) {
		if (capture)
			capture->handleEvent(e);
		else if (e.type == Common::EVENT_KEYDOWN && focus)
			focus->handleEvent(e);
		else
			handleEvent(e);

		if (e.type >= Common::EVENT_MOUSEMOVE && e.type <= Common::EVENT_MBUTTONUP) {
			_mouseX = e.mouse.x;
			_mouseY = e.mouse.y;
		}

		if (e.type != Common::EVENT_MOUSEMOVE)
			break;
	}
}

void CBofWindow::handleEvent(const Common::Event &event) {
	Assert(IsValidObject(this));

	if (!_enabled || !_visible)
		// Window is disabled or hidden
		return;

	CPoint mousePos(event.mouse.x - m_cWindowRect.left,
		event.mouse.y - m_cWindowRect.top);
	for (auto parent = _parent; parent; parent = parent->_parent) {
		mousePos.x -= parent->m_cWindowRect.left;
		mousePos.y -= parent->m_cWindowRect.top;
	}

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP: {
		// Check if the mouse is within the area of a child control
		for (uint i = 0; i < _children.size(); ++i) {
			auto &child = *_children[i];
			if (child.IsVisible() && child.IsEnabled() &&
					child.GetWindowRect().PtInRect(mousePos)) {
				child.handleEvent(event);
				return;
			}
		}
		break;
	}

	default:
		break;
	}

	uint32 currTime = g_system->getMillis();

	switch ((int)event.type) {
	case Common::EVENT_MOUSEMOVE:
		OnMouseMove(0, &mousePos);
		break;

	case Common::EVENT_LBUTTONDOWN:
		if ((currTime - _lastLButtonTime) <= DOUBLE_CLICK_TIME) {
			_lastLButtonTime = 0;
			OnLButtonDblClk(1, &mousePos);
		} else {
			OnLButtonDown(1, &mousePos);
			_lastLButtonTime = currTime;
		}
		break;

	case Common::EVENT_LBUTTONUP:
		OnLButtonUp(0, &mousePos);
		break;

	case Common::EVENT_RBUTTONDOWN:
		if ((currTime - _lastRButtonTime) <= DOUBLE_CLICK_TIME) {
			_lastRButtonTime = 0;
			OnRButtonDblClk(2, &mousePos);
		} else {
			OnRButtonDown(2, &mousePos);
			_lastRButtonTime = currTime;
		}
		break;

	case Common::EVENT_RBUTTONUP:
		OnRButtonUp(0, &mousePos);
		break;

	case Common::EVENT_KEYDOWN:
		uint32 lNewKey;

		if ((lNewKey = TranslateKey(event)) != BKEY_UNKNOWN) {
			OnKeyHit(lNewKey, event.kbdRepeat ? 1 : 0);
		}
		break;

	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		if (event.customType != KEYBIND_NONE)
			OnKeyHit((event.customType == KEYBIND_WAIT)
				? BKEY_SPACE : BKEY_SCRL_LOCK, 0);
		break;

	case EVENT_USER:
		// Message type and param are stored in mouse x/y
		OnUserMessage(event.mouse.x, event.mouse.y);
		break;

	case Common::EVENT_QUIT:
		OnClose();
		break;

	default:
		break;
	}
}


uint32 CBofWindow::TranslateKey(const Common::Event &event) const {
	uint32 nCode = BKEY_UNKNOWN;

	switch (event.kbd.keycode) {
	case Common::KEYCODE_F1: nCode = BKEY_F1; break;
	case Common::KEYCODE_F2: nCode = BKEY_SAVE; break;
	case Common::KEYCODE_F3: nCode = BKEY_RESTORE; break;
	case Common::KEYCODE_F4: nCode = BKEY_F4; break;
	case Common::KEYCODE_F5: nCode = BKEY_SAVE; break;
	case Common::KEYCODE_F6: nCode = BKEY_F6; break;
	case Common::KEYCODE_F7: nCode = BKEY_RESTORE; break;
	case Common::KEYCODE_F8: nCode = BKEY_F8; break;
	case Common::KEYCODE_F9: nCode = BKEY_F9; break;
	case Common::KEYCODE_F10: nCode = BKEY_F10; break;
	case Common::KEYCODE_F11: nCode = BKEY_F11; break;
	case Common::KEYCODE_F12: nCode = BKEY_F12; break;

	case Common::KEYCODE_END: nCode = BKEY_END; break;
	case Common::KEYCODE_HOME: nCode = BKEY_HOME; break;
	case Common::KEYCODE_LEFT: nCode = BKEY_LEFT; break;
	case Common::KEYCODE_RIGHT: nCode = BKEY_RIGHT; break;
	case Common::KEYCODE_UP: nCode = BKEY_UP; break;
	case Common::KEYCODE_DOWN: nCode = BKEY_DOWN; break;
	case Common::KEYCODE_RETURN: nCode = BKEY_ENTER; break;
	case Common::KEYCODE_INSERT: nCode = BKEY_INS; break;
	case Common::KEYCODE_BACKSPACE: nCode = BKEY_BACK; break;
	case Common::KEYCODE_DELETE: nCode = BKEY_DEL; break;
	case Common::KEYCODE_SCROLLOCK: nCode = BKEY_SCRL_LOCK; break;
	case Common::KEYCODE_PAGEUP: nCode = BKEY_PAGEUP; break;
	case Common::KEYCODE_PAGEDOWN: nCode = BKEY_PAGEDOWN; break;
	case Common::KEYCODE_ESCAPE: nCode = BKEY_ESC; break;

	default:
		// No translation for this key
		if (event.kbd.ascii >= 32 && event.kbd.ascii <= 127)
			nCode = event.kbd.ascii;
		break;
	}

	if (nCode != BKEY_UNKNOWN) {
		if (event.kbd.flags & Common::KBD_ALT) {
			nCode = tolower(nCode) | BKF_ALT;
		}
	}

	return nCode;
}

void CBofWindow::FillWindow(byte iColor) {
	FillRect(nullptr, iColor);
}

void CBofWindow::FillRect(CBofRect *pRect, byte iColor) {
#if BOF_MAC || BOF_WINMAC
	// On the mac, we'll just draw directly into the graphics port that
	// is being used by the window.
	WindowPtr pWindow = GetMacWindow();
	STBofPort stSavePort(pWindow);
	Pattern cPat;

	// make sure we don't get a white flash
	LMSetPaintWhite(false);

	for (int i = 0; i < 8; i++) {
		((char *)&cPat)[i] = iColor;
	}

	Rect theRect;

	if (pRect == nullptr)
		theRect = pWindow->portRect;
	else
		theRect = pRect->GetMacRect();

	RgnHandle theRgn = ::NewRgn();
	WindowPtr pFWindow = ::FrontWindow();
	::ShowWindow(pWindow);

	::GetClip(theRgn);
	::ClipRect(&theRect);

	::FillRect(&theRect, &cPat);

	::SetClip(theRgn);
	::DisposeRgn(theRgn);

	if (pFWindow != nullptr)
		::ShowWindow(pFWindow);
#else
	// Slow, but should work fine
	CBofBitmap cBmp(Width(), Height(), CBofApp::GetApp()->GetPalette());
	cBmp.FillRect(pRect, iColor);
	cBmp.Paint(this, 0, 0);
#endif
}

#if BOF_WINMAC
WindowPtr CBofWindow::GetMacWindow() {
	Assert(IsValidObject(this));

	WindowPtr pWindow;

	if ((pWindow = GetWrapperWindow(m_hWnd)) == nullptr) {
		pWindow = GetWrapperContainerWindow(m_hWnd);
	}
	return pWindow;
}
#endif

#if BOF_MAC

STBofScreen::STBofScreen(Rect *parentWindRect) {
	GDHandle screenGD;
	GWorldPtr curWorld;

	GetGWorld(&curWorld, &screenGD);
	Rect winRect = *parentWindRect;

	m_screenRect = (*(*screenGD)->gdPMap)->bounds;

	(*(*screenGD)->gdPMap)->bounds = winRect;
	(*screenGD)->gdRect = winRect;
}

STBofScreen::~STBofScreen() {
	GDHandle screenGD;
	GWorldPtr curWorld;

	GetGWorld(&curWorld, &screenGD);

	(*(*screenGD)->gdPMap)->bounds = m_screenRect;
	(*screenGD)->gdRect = m_screenRect;
}

// used to switch to the port that we really need to update
// and restore when we're done.
STBofPort::STBofPort(WindowPtr pMacWindow) {
	m_bNewPort = false;
	::GetPort(&m_pSavePort);

	bool bValid = true;

	if (m_pSavePort != pMacWindow && pMacWindow != nullptr && bValid) {
		m_nCheckCookie = m_pSavePort->device;
		m_bNewPort = true;
		::SetPort(pMacWindow);
	}
}

STBofPort::~STBofPort() {
	bool bValid = true;
	if (m_bNewPort) {

#if true
		CBofWindow *pWnd = CBofWindow::FromMacWindow(m_pSavePort);
		if (pWnd == nullptr || pWnd->GetMacWindow() == nullptr) {
			bValid = false;
		}
#else

		// check to make sure this is a valid window.
		if (m_pSavePort->portBits.rowBytes & 0x8000) {
			if (GetHandleSize((Handle)((CGrafPtr)m_pSavePort)->portPixMap) == 0) {
				bValid = false;
			}
		} else {
			if (GetPtrSize((Ptr)m_pSavePort->portBits.baseAddr) == 0) {
				bValid = false;
			}
		}
#endif

		// Don't bother with these debug messages as they are a fairly common
		// occurrence (it will get triggered whenever a mouse down/up causes
		// a window to go away, such as with popup menus).
#if DEVELOPMENT
#if DISPLAYINVGRAFPORT
		if (bValid == false) {
			DebugStr("\pSTBofPort::~STBofPort is restoring an invalid grafport...");
		}
#endif
#endif

		if (m_nCheckCookie != m_pSavePort->device) {
#if DEVELOPMENT
			DebugStr("\pSTBofPort::~STBofPort ran into a weird situation...");
#endif
		} else {
			if (bValid == true) {
				::SetPort(m_pSavePort);
			}
		}
	}
}

// need to have routines to have more than a single
// active object.

bool CBofWindow::IsInActiveList() {
	CBofWindow *pactive = m_pActiveWindow;

	while (pactive) {
		if (pactive == this) {
			return true;
		}
		pactive = pactive->m_pPrevActiveWindow;
	}

	return false;
}

void CBofWindow::SetActive() {

	if (m_pActiveWindow != this) {
		if (m_pActiveWindow == nullptr) {
			m_pActiveWindow = this;
		} else {
			if (!IsInActiveList()) {
				CBofWindow *pactive = m_pActiveWindow;
				m_pActiveWindow = this;
				m_pActiveWindow->m_pPrevActiveWindow = pactive;
			} else {
				// it's in the active list, but it's not at the front...
				// find it and bring it to the front.
				CBofWindow *prevActive = m_pActiveWindow;
				CBofWindow *pActive = m_pActiveWindow->m_pPrevActiveWindow;

				while (pActive) {
					if (pActive == this) {
						prevActive->m_pPrevActiveWindow = pActive->m_pPrevActiveWindow;
						pActive->m_pPrevActiveWindow = m_pActiveWindow;
						m_pActiveWindow = pActive;
						return;
					}
					prevActive = pActive;
					pActive = pActive->m_pPrevActiveWindow;
				}
			}
		}
	}
}

void CBofWindow::RemoveFromActiveList() {

	// Should ALWAYS be the first object in the list.

	CBofWindow *prevActive = m_pActiveWindow;
	CBofWindow *pActive = (m_pActiveWindow == nullptr ? nullptr : m_pActiveWindow->m_pPrevActiveWindow);

	if (m_pActiveWindow == this) {
		m_pActiveWindow = m_pActiveWindow->m_pPrevActiveWindow;
	} else {
		while (pActive) {
			if (pActive == this) {
				prevActive->m_pPrevActiveWindow = pActive->m_pPrevActiveWindow;
				break;
			} else {
				prevActive = pActive;
				pActive = pActive->m_pPrevActiveWindow;
			}
		}
	}
}

#endif

STBofFont::STBofFont(int nFont) {
#if BOF_MAC
	GrafPtr curPort;

	::GetPort(&curPort);
	m_nSaveFont = curPort->txFont;

	::TextFont(nFont);
#else
#endif
}

STBofFont::~STBofFont() {
#if BOF_MAC
	::TextFont(m_nSaveFont);
#else
#endif
}

} // namespace Bagel
