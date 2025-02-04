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
#include "bagel/boflib/sound.h"
#include "bagel/boflib/std_keys.h"
#include "bagel/metaengine.h"
#include "bagel/bagel.h"

namespace Bagel {

#define DOUBLE_CLICK_TIME 250

// Static members defined here
CBofWindow *CBofWindow::_pWindowList = nullptr;
CBofWindow *CBofWindow::_pActiveWindow = nullptr;
CBofTimerPacket *CBofWindow::_pTimerList = nullptr;
int CBofWindow::_mouseX = 0;
int CBofWindow::_mouseY = 0;

CBofWindow::CBofWindow() {
	if (_pActiveWindow == nullptr)
		_pActiveWindow = this;

	if (_pWindowList == nullptr) {
		_pWindowList = this;
	} else {
		_pWindowList->Insert(this);
	}
}

CBofWindow::CBofWindow(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent) {
	if (_pWindowList == nullptr) {
		_pWindowList = this;
	} else {
		_pWindowList->Insert(this);
	}

	create(pszName, x, y, nWidth, nHeight, pParent);
}

CBofWindow::~CBofWindow() {
	assert(isValidObject(this));

	delete _surface;
	_surface = nullptr;

	killMyTimers();

	// Remove it from any parent
	if (_parent != nullptr)
		setParent(nullptr);

	// Remove this window from the list
	if (_pWindowList == this) {
		_pWindowList = (CBofWindow *)getNext();
	}

	killBackdrop();

	CBofWindow::destroy();
}

ErrorCode CBofWindow::initialize() {
	_pWindowList = nullptr;
	_pActiveWindow = nullptr;
	_pTimerList = nullptr;

	return ERR_NONE;
}

ErrorCode CBofWindow::shutdown() {
	return ERR_NONE;
}

Common::Point CBofWindow::getMousePos() {
	return Common::Point(_mouseX, _mouseY);
}

void CBofWindow::destroy() {
	releaseCapture();

	delete _surface;
	_surface = nullptr;

	// When gui elements are destroyed, remove them
	// from the _children array of their parent
	setParent(nullptr);
}

void CBofWindow::validateAnscestors(CBofRect *pRect) {
	assert(isValidObject(this));

	// Validate all anscestors
	CBofWindow *pParent = _parent;
	while (pParent != nullptr) {
		pParent->validateRect(pRect);
		pParent = pParent->getParent();
	}
}

ErrorCode CBofWindow::create(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID) {
	assert(isValidObject(this));
	assert(pszName != nullptr);
	assert(pParent != this);

	// Remember who our parent is
	if (pParent != nullptr)
		setParent(pParent);

	_nID = nControlID;

	// Remember the name of this window
	Common::strlcpy(_szTitle, pszName, MAX_TITLE);

	// Retain screen coordinates for this window
	_cWindowRect.setRect(x, y, x + nWidth - 1, y + nHeight - 1);

	// Calculate effective bounds
	Common::Rect stRect(x, y, x + nWidth, y + nHeight);
	if (pParent != nullptr)
		stRect.translate(pParent->getWindowRect().left,
			pParent->getWindowRect().top);

	delete _surface;
	_surface = new Graphics::ManagedSurface(*g_engine->_screen, stRect);

	if (!errorOccurred()) {
		CBofPalette *pPalette = CBofApp::getApp()->getPalette();
		if (pPalette != nullptr) {
			selectPalette(pPalette);
		}

		// Retain local coordinates (based on own window)
		_cRect.setRect(0, 0, _cWindowRect.width() - 1, _cWindowRect.height() - 1);
	}

	return _errCode;
}

void CBofWindow::updateWindow() {
	if (_visible) {
		if (isVisible())
			onPaint(&_cRect);

		for (uint i = 0; i < _children.size(); ++i)
			_children[i]->updateWindow();
	}
}

void CBofWindow::setParent(CBofWindow *parent) {
	if (_parent != nullptr)
		_parent->_children.remove(this);

	_parent = parent;
	if (parent)
		parent->_children.push_back(this);
}


ErrorCode CBofWindow::create(const char *pszName, CBofRect *pRect, CBofWindow *pParent, uint32 nControlID) {
	assert(isValidObject(this));
	assert(pszName != nullptr);

	int x = 0;
	int y = 0;

	int nWidth = USE_DEFAULT;
	int nHeight = USE_DEFAULT;

	if (pRect != nullptr) {
		x = pRect->left;
		y = pRect->top;
		nWidth = pRect->width();
		nHeight = pRect->height();
	}

	return create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
}

void CBofWindow::releaseCapture() {
	_bCaptured = false;
	if (hasCapture())
		CBofApp::getApp()->setCaptureControl(nullptr);
}

void CBofWindow::setCapture() {
	_bCaptured = true;
	CBofApp::getApp()->setCaptureControl(this);
}

bool CBofWindow::hasCapture() const {
	return CBofApp::getApp()->getCaptureControl() == this;
}

void CBofWindow::releaseFocus() {
	CBofApp::getApp()->setFocusControl(nullptr);
}

void CBofWindow::setFocus() {
	CBofApp::getApp()->setFocusControl(this);
}

bool CBofWindow::hasFocus() const {
	return CBofApp::getApp()->getFocusControl() == this;
}

void CBofWindow::center() {
	assert(isValidObject(this));

	CBofWindow *pParent = _parent;
	int x, y;

	if (pParent != nullptr) {
		CBofRect cWindowRect = pParent->getWindowRect();
		x = cWindowRect.left + (pParent->width() - width()) / 2;
		y = cWindowRect.top + (pParent->height() - height()) / 2;

	} else {
		x = (CBofApp::getApp()->screenWidth() - width()) / 2;
		y = (CBofApp::getApp()->screenHeight() - height()) / 2;
	}

	move(x, y);
}

void CBofWindow::move(const int x, const int y, bool bRepaint) {
	assert(isValidObject(this));
	assert(isCreated());

	// We now have a new position (in screen coordinates)
	_cWindowRect.setRect(x, y, x + _cRect.width() - 1, y + _cRect.height() - 1);

	// Recreate the surface at the new screen position
	delete _surface;
	_surface = new Graphics::ManagedSurface(*g_engine->_screen, _cWindowRect);
}

void CBofWindow::reSize(CBofRect *pRect, bool bRepaint) {
	if (_lockedScreen)
		return;
	assert(isValidObject(this));
	assert(isCreated());
	assert(pRect != nullptr);

	// We now have a new position (in screen coordinates)
	_cWindowRect = *pRect;
	_cRect.setRect(0, 0, _cWindowRect.width() - 1, _cWindowRect.height() - 1);

	// Recreate the surface at the new screen position
	delete _surface;
	_surface = new Graphics::ManagedSurface(*g_engine->_screen, _cWindowRect);
}

void CBofWindow::select() {
	// No implementation in ScummVM
}

void CBofWindow::show() {
	assert(isValidObject(this));

	if (!errorOccurred()) {
		assert(isCreated());

		if (isCreated()) {
			_visible = true;
			invalidateRect(&_cRect);
		}
	}
}

void CBofWindow::hide() {
	assert(isValidObject(this));

	if (!errorOccurred()) {
		assert(isCreated());

		_visible = false;
	}
}

void CBofWindow::postMessage(uint32 nMessage, uint32 lParam1, uint32 lParam2) {
	assert(isValidObject(this));
	assert(isCreated());
}

void CBofWindow::setTimer(uint32 nID, uint32 nInterval, BofCallback pCallBack) {
	assert(isValidObject(this));
	assert(isCreated());

	// Don't add it if there's already a timer there with the same id.
	CBofTimerPacket *pPacket = _pTimerList;

	while (pPacket != nullptr) {
		if (pPacket->_nID == nID)
			return;

		pPacket = (CBofTimerPacket *)pPacket->getNext();
	}

	pPacket = new CBofTimerPacket;
	pPacket->_nID = nID;
	pPacket->_nInterval = nInterval;
	pPacket->_pCallBack = pCallBack;
	pPacket->_pOwnerWindow = this;

	// Add this timer to the list of current timers
	if (_pTimerList != nullptr) {
		_pTimerList->addToHead(pPacket);
	}

	_pTimerList = pPacket;

	// Add the timer to the window
	_timers.push_back(WindowTimer(nInterval, nID, pCallBack));
}

void CBofWindow::killTimer(uint32 nID) {
	assert(isValidObject(this));

	// Remove the timer from the window timer list
	for (Common::List<WindowTimer>::iterator it = _timers.begin(); it != _timers.end(); ++it) {
		if (it->_id == nID) {
			_timers.erase(it);
			break;
		}
	}

	// Find and remove the timer packet for this timer
	CBofTimerPacket *pPacket = _pTimerList;

	while (pPacket != nullptr) {
		if (pPacket->_nID == nID) {
			if (pPacket == _pTimerList) {
				_pTimerList = (CBofTimerPacket *)_pTimerList->getNext();
			}

			delete pPacket;
			break;
		}

		pPacket = (CBofTimerPacket *)pPacket->getNext();
	}
}

void CBofWindow::killMyTimers() {
	assert(isValidObject(this));

	CBofTimerPacket *pTimer = _pTimerList;
	while (pTimer != nullptr) {
		CBofTimerPacket *pNextTimer = (CBofTimerPacket *)pTimer->getNext();

		if (pTimer->_pOwnerWindow == this) {
			killTimer(pTimer->_nID);
		}

		pTimer = pNextTimer;
	}
}

void CBofWindow::checkTimers() {
	for (uint i = 0; i < _children.size(); ++i)
		_children[i]->checkTimers();

	for (bool timersChanged = true; timersChanged;) {
		timersChanged = false;
		uint32 currTime = g_system->getMillis();

		// Iterate over the timers looking for any that have expired
		for (Common::List<WindowTimer>::iterator it = _timers.begin(); it != _timers.end(); ++it) {
			WindowTimer &timer = *it;

			if (currTime >= (timer._lastExpiryTime + timer._interval)) {
				// Timer has expired
				timer._lastExpiryTime = currTime;

				if (timer._callback) {
					(timer._callback)(timer._id, this);
				} else {
					onTimer(timer._id);
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

void CBofWindow::screenToClient(CBofPoint *pPoint) {
	// Not needed in ScummVM
}

CBofRect CBofWindow::getClientRect() {
	assert(isValidObject(this));

	CBofRect cRect(0, 0, _cRect.width() - 1, _cRect.height() - 1);
	return cRect;
}

void CBofWindow::postUserMessage(uint32 lMessage, uint32 lExtraInfo) {
	Common::Event e;
	e.type = (Common::EventType)EVENT_USER;
	e.mouse.x = lMessage;
	e.mouse.y = lExtraInfo;

	g_system->getEventManager()->pushEvent(e);
}

void CBofWindow::flushAllMessages() {
	// Make sure this is a valid window
	assert(isValidObject(this));
	assert(isCreated());
}

void CBofWindow::validateRect(const CBofRect *pRect) {
	// No implementation in ScummVM
}

void CBofWindow::invalidateRect(const CBofRect *pRect) {
}

ErrorCode CBofWindow::setBackdrop(CBofBitmap *pNewBitmap, bool bRefresh) {
	assert(isValidObject(this));
	assert(pNewBitmap != nullptr);

	// Destroy old backdrop (if any)
	killBackdrop();

	// We take ownership of this bitmap!
	_pBackdrop = pNewBitmap;

	if (bRefresh) {
		_pBackdrop->paint(this, 0, 0);
	}

	return _errCode;
}

ErrorCode CBofWindow::setBackdrop(const char *pszFileName, bool bRefresh) {
	assert(isValidObject(this));
	assert(pszFileName != nullptr);

	// Use Application's palette if none supplied
	CBofPalette *pPalette = CBofApp::getApp()->getPalette();
	CBofBitmap *pBmp = new CBofBitmap(pszFileName, pPalette);

	return setBackdrop(pBmp, bRefresh);
}

void CBofWindow::killBackdrop() {
	assert(isValidObject(this));

	delete _pBackdrop;
	_pBackdrop = nullptr;
}

ErrorCode CBofWindow::paintBackdrop(CBofRect *pRect, int nTransparentColor) {
	assert(isValidObject(this));

	if (_pBackdrop != nullptr) {
		if (pRect == nullptr) {
			_errCode = _pBackdrop->paint(this, &_cRect, nullptr, nTransparentColor);
		} else {
			_errCode = _pBackdrop->paint(this, pRect, pRect, nTransparentColor);
		}
	}

	return _errCode;
}

void CBofWindow::selectPalette(CBofPalette *pPal) {
	assert(isValidObject(this));
	assert(isCreated());
}

Graphics::ManagedSurface *CBofWindow::getSurface() {
	return _surface;
}


// Default version of these virtual functions don't do anything
//
void CBofWindow::onMouseMove(uint32, CBofPoint *, void *) {}

void CBofWindow::onLButtonDown(uint32, CBofPoint *, void *) {}
void CBofWindow::onLButtonUp(uint32, CBofPoint *, void *) {}
void CBofWindow::onLButtonDblClk(uint32, CBofPoint *) {}

void CBofWindow::onRButtonDown(uint32, CBofPoint *) {}
void CBofWindow::onRButtonUp(uint32, CBofPoint *) {}
void CBofWindow::onRButtonDblClk(uint32, CBofPoint *) {}

void CBofWindow::onKeyHit(uint32, uint32) {}

void CBofWindow::onReSize(CBofSize *) {}
void CBofWindow::onPaint(CBofRect *) {}
void CBofWindow::onTimer(uint32) {}

void CBofWindow::onClose() {}

void CBofWindow::onBofButton(CBofObject *, int) {}
void CBofWindow::onBofScrollBar(CBofObject *, int) {}
void CBofWindow::onBofListBox(CBofObject *, int) {}
void CBofWindow::onUserMessage(uint32, uint32) {}
void CBofWindow::onMainLoop() {}

void CBofWindow::onSoundNotify(CBofObject *, uint32) {}
void CBofWindow::onMovieNotify(uint32, uint32) {}

void CBofWindow::onActivate() {}
void CBofWindow::onDeActivate() {}

void CBofWindow::onMCINotify(uint32 wParam, uint32 lParam) {
	assert(isValidObject(this));
}

void CBofWindow::handleEvents() {
	Common::Event e;
	CBofWindow *capture = CBofApp::getApp()->getCaptureControl();
	CBofWindow *focus = CBofApp::getApp()->getFocusControl();
	bool eventsPresent = false;

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

		if (e.type != Common::EVENT_MOUSEMOVE) {
			eventsPresent = true;
			break;
		}
	}

	// Only do timer checks when not processing other pending events.
	// This simulates Windows behaviour, where the WM_TIMER events
	// would be added at the end of the event queue
	if (!eventsPresent)
		// Check for expired timers
		checkTimers();


}

void CBofWindow::handleEvent(const Common::Event &event) {
	assert(isValidObject(this));

	if (!_enabled || !_visible)
		// Window is disabled or hidden
		return;

	CPoint mousePos(event.mouse.x - _cWindowRect.left,
		event.mouse.y - _cWindowRect.top);
	for (auto parent = _parent; parent; parent = parent->_parent) {
		mousePos.x -= parent->_cWindowRect.left;
		mousePos.y -= parent->_cWindowRect.top;
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
			if (child.isVisible() && child.isEnabled() &&
					child.getWindowRect().ptInRect(mousePos)) {
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
		onMouseMove(0, &mousePos);
		break;

	case Common::EVENT_LBUTTONDOWN:
		if ((currTime - _lastLButtonTime) <= DOUBLE_CLICK_TIME) {
			_lastLButtonTime = 0;
			onLButtonDblClk(1, &mousePos);
		} else {
			onLButtonDown(1, &mousePos);
			_lastLButtonTime = currTime;
		}
		break;

	case Common::EVENT_LBUTTONUP:
		onLButtonUp(0, &mousePos);
		break;

	case Common::EVENT_RBUTTONDOWN:
		if ((currTime - _lastRButtonTime) <= DOUBLE_CLICK_TIME) {
			_lastRButtonTime = 0;
			onRButtonDblClk(2, &mousePos);
		} else {
			onRButtonDown(2, &mousePos);
			_lastRButtonTime = currTime;
		}
		break;

	case Common::EVENT_RBUTTONUP:
		onRButtonUp(0, &mousePos);
		break;

	case Common::EVENT_KEYDOWN:
		uint32 lNewKey;

		if ((lNewKey = translateKey(event)) != BKEY_UNKNOWN) {
			onKeyHit(lNewKey, event.kbdRepeat ? 1 : 0);
		}
		break;

	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		if (event.customType != KEYBIND_NONE)
			onKeyHit((event.customType == KEYBIND_WAIT)
				? BKEY_SPACE : BKEY_SCRL_LOCK, 0);
		break;

	case EVENT_USER:
		// Message type and param are stored in mouse x/y
		onUserMessage(event.mouse.x, event.mouse.y);
		break;

	case Common::EVENT_QUIT:
		onClose();
		break;

	default:
		break;
	}
}


uint32 CBofWindow::translateKey(const Common::Event &event) const {
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

void CBofWindow::fillWindow(byte iColor) {
	fillRect(nullptr, iColor);
}

void CBofWindow::fillRect(CBofRect *pRect, byte iColor) {
	CBofBitmap cBmp(width(), height(), CBofApp::getApp()->getPalette());
	cBmp.fillRect(pRect, iColor);
	cBmp.paint(this, 0, 0);
}

ErrorCode CBofWindow::paintBeveledText(CBofRect *rect, const CBofString &cString, const int size, const int weight, const RGBCOLOR color, int justify, uint32 format) {
	assert(rect != nullptr);

	CBofBitmap bmp(rect->width(), rect->height(), nullptr, false);

	// Assume no error
	ErrorCode errorCode = ERR_NONE;

	CBofRect r = bmp.getRect();
	CBofPalette *palette = nullptr;
	CBofApp *app = CBofApp::getApp();
	if (app != nullptr) {
		palette = app->getPalette();
	}

	if (palette != nullptr) {
		bmp.fillRect(nullptr, palette->getNearestIndex(RGB(92, 92, 92)));

		bmp.drawRect(&r, palette->getNearestIndex(RGB(0, 0, 0)));
	} else {
		bmp.fillRect(nullptr, COLOR_BLACK);
	}

	byte c1 = 3;
	byte c2 = 9;
	CBofRect cBevel = r;

	int left = cBevel.left;
	int top = cBevel.top;
	int right = cBevel.right;
	int bottom = cBevel.bottom;

	r.left += 6;
	r.top += 3;
	r.right -= 5;
	r.bottom -= 5;

	for (int i = 1; i <= 3; i++) {
		bmp.line(left + i, bottom - i, right - i, bottom - i, c1);
		bmp.line(right - i, bottom - i, right - i, top + i - 1, c1);
	}

	for (int i = 1; i <= 3; i++) {
		bmp.line(left + i, bottom - i, left + i, top + i - 1, c2);
		bmp.line(left + i, top + i - 1, right - i, top + i - 1, c2);
	}

	paintText(&bmp, &r, cString, size, weight, color, justify, format, FONT_DEFAULT);

	bmp.paint(this, rect);

	return errorCode;
}

} // namespace Bagel
