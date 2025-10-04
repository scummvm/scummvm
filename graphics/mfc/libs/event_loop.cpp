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
#include "engines/engine.h"
#include "graphics/mfc/afxwin.h"
#include "graphics/mfc/libs/event_loop.h"
#include "graphics/mfc/winnt.h"

namespace Graphics {
namespace MFC {
namespace Libs {

#define FRAME_RATE 50

void EventLoop::runEventLoop(bool isModalDialog) {
	MSG msg;

	while (!shouldQuit() && !_activeWindows.empty()) {
		CWnd *activeWin = GetActiveWindow();
		if (activeWin->_modalResult != DEFAULT_MODAL_RESULT)
			break;

		if (!GetMessage(msg))
			break;

		CWnd *mainWnd = GetActiveWindow();
		if (msg.message != WM_NULL && mainWnd && !mainWnd->PreTranslateMessage(&msg) &&
				(!isModalDialog || !mainWnd->IsDialogMessage(&msg))) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void EventLoop::SetActiveWindow(CWnd *wnd) {
	assert(!shouldQuit());
	if (wnd == GetActiveWindow())
		// Already the active window
		return;

	// If it's the first window added, and we don't have
	// a main window defined, set it as the main window
	if (_activeWindows.empty())
		_mainWindow = wnd;

	// Add the window to the list
	// Note: Currently we don't supportly multiple
	// open windows at the same time. Each new window
	// is effectively a dialog on top of previous ones

	if (!_activeWindows.empty()) {
		auto *win = _activeWindows.top();
		win->SendMessage(WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), 0);
		win->SendMessage(WM_PALETTECHANGED, (WPARAM)wnd);
	}

	_activeWindows.push(wnd);
	wnd->SendMessage(WM_ACTIVATE, MAKEWPARAM(WA_ACTIVE, 0), 0);
	wnd->SendMessage(WM_QUERYNEWPALETTE, 0, 0);
}

void EventLoop::PopActiveWindow() {
	_activeWindows.top()->SendMessage(WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), 0);
	_activeWindows.pop();

	if (!_activeWindows.empty()) {
		CWnd *wnd = _activeWindows.top();
		wnd->RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN);
		wnd->SendMessage(WM_ACTIVATE, MAKEWPARAM(WA_ACTIVE, 0), 0);
	}
}

void EventLoop::doModal(CWnd *wnd) {
	SetActiveWindow(wnd);
	runEventLoop(true);
	if (GetActiveWindow() == wnd)
		wnd->DestroyWindow();
}

void EventLoop::checkMessages() {
	// Don't do any actual ScummVM event handling
	// until at least one window has been set up
	if (_activeWindows.empty())
		return;

	if (_messages.empty() && _idleCtr >= 0) {
		if (!OnIdle(_idleCtr))
			// OnIdle returning false means disabling permanently
			_idleCtr = -1;
	}

	// Poll for event in ScummVM event manager
	MSG priorMsg;
	Libs::Event ev;

	while (pollEvents(ev)) {
		// Handle custom keybinding actions mapping back to keys
		if (_keybindProc) {
			if (ev.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				ev.type = Common::EVENT_KEYDOWN;
				ev.kbd.keycode = _keybindProc(ev.customType);
			} else if (ev.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
				ev.type = Common::EVENT_KEYUP;
				ev.kbd.keycode = _keybindProc(ev.customType);
			}
		}

		HWND hWnd = nullptr;
		setMessageWnd(ev, hWnd);
		MSG msg = ev;
		msg.hwnd = hWnd;

		if (msg.message == WM_MOUSEMOVE &&
				priorMsg.message == WM_MOUSEMOVE) {
			// Preventing multiple sequential mouse move messages
			priorMsg = msg;
		} else if (msg.message != WM_NULL) {
			if (priorMsg.message != WM_NULL)
				_messages.push(priorMsg);
			priorMsg = msg;
		}

		if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
			// Update saved mouse position
			_mousePos.x = LOWORD(msg.lParam);
			_mousePos.y = HIWORD(msg.lParam);

			// For mouse messages, if the highlighted control
			// changes, generate a WM_SETCURSOR event
			if (msg.hwnd != _highlightedWin) {
				// Add mouse leave event if win is still alive
				CWnd *highlightedWin = CWnd::FromHandle(_highlightedWin);
				if (highlightedWin)
					highlightedWin->PostMessage(WM_MOUSELEAVE);

				// Switch to newly highlighted control
				_highlightedWin = msg.hwnd;
				if (_highlightedWin)
					PostMessage(_highlightedWin,
						WM_SETCURSOR, (WPARAM)msg.hwnd,
						MAKELPARAM(HTCLIENT, msg.message)
					);
			}
		} else if (msg.message == WM_QUIT) {
			// Add a window message close message as well
			MSG cmsg;
			cmsg.message = WM_CLOSE;
			cmsg.hwnd = hWnd;
			_messages.push(cmsg);
		}
	}

	if (priorMsg.message != WM_NULL)
		_messages.push(priorMsg);

	// If there are no pending messages,
	// do a brief pause and check for frame updates
	if (_messages.empty())
		checkForFrameUpdate();
}

bool EventLoop::GetMessage(MSG &msg) {
	checkMessages();

	// Queue window/control repaints if needed
	for (CWnd *wnd : _updateWnds) {
		if (wnd->IsWindowDirty())
			wnd->PostMessage(WM_PAINT);
	}

	_updateWnds.clear();

	// Check for any existing messages
	if (!_messages.empty()) {
		msg = _messages.pop();

		if (msg.hwnd) {
			if ((msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) &&
					!_kbdHookProc.empty()) {
				if (_kbdHookProc.front()(HC_ACTION, msg.wParam, msg.lParam))
					msg.message = WM_NULL;
			}
		} else if (msg.message != WM_QUIT) {
			msg.message = WM_NULL;
		} else {
			debug(1, "Got WM_QUIT message..");
		}
	} else {
		msg.message = WM_NULL;
	}

	return !((msg.message == WM_QUIT) || (shouldQuit() && _messages.empty()));
}

void EventLoop::setMessageWnd(Common::Event &ev, HWND &hWnd) {
	if (isMouseMsg(ev)) {
		setMouseMessageWnd(ev, hWnd);
		return;
	}

	if (_focusedWin && (ev.type == Common::EVENT_KEYDOWN ||
			ev.type == Common::EVENT_KEYUP)) {
		hWnd = _focusedWin->m_hWnd;
		return;
	}

	if (isJoystickMsg(ev)) {
		CWnd *joystickWin = CWnd::FromHandle(_joystickWin);

		if (joystickWin) {
			switch (ev.type) {
			case Common::EVENT_JOYAXIS_MOTION:
				if (ev.joystick.axis == 0)
					_joystickPos.x = ev.joystick.position + JOYSTICK_REST_POS;
				else
					_joystickPos.y = ev.joystick.position + JOYSTICK_REST_POS;

				joystickWin->SendMessage(MM_JOY1MOVE, JOYSTICKID1,
					MAKELPARAM(_joystickPos.x, _joystickPos.y));
				break;

			default:
				_joystickButtons = ev.joystick.button;
				_joystickWin->SendMessage(MM_JOY1MOVE, JOYSTICKID1,
					MAKELPARAM(_joystickPos.x, _joystickPos.y));
				break;
			}
		}

		hWnd = nullptr;
		return;
	}

	// Fallback, send message to active window
	CWnd *activeWin = _activeWindows.top();
	hWnd = activeWin->m_hWnd;
}

void EventLoop::setMouseMessageWnd(Common::Event &ev, HWND &hWnd) {
	// Handle mouse capture
	if (_captureWin) {
		hWnd = _captureWin->m_hWnd;

		POINT pt;
		pt.x = ev.mouse.x;
		pt.y = ev.mouse.y;
		mousePosToClient(_captureWin, pt);

		ev.mouse.x = pt.x;
		ev.mouse.y = pt.y;
		return;
	}

	// Special case for mouse moves: if there's an modal dialog,
	// mouse moves will still be routed to the main window
	// if the mouse is outside the dialog bounds
	CWnd *activeWin = _activeWindows.top();
	if (ev.type == Common::EVENT_MOUSEMOVE &&
		!activeWin->_windowRect.contains(ev.mouse)) {
		hWnd = _mainWindow->m_hWnd;
		return;
	}

	CWnd *wnd = _activeWindows.top();
	hWnd = getMouseMessageWnd(ev, wnd);
}

HWND EventLoop::getMouseMessageWnd(Common::Event &ev, CWnd *parent) {
	POINT pt;
	pt.x = ev.mouse.x;
	pt.y = ev.mouse.y;

	if (!mousePosToClient(parent, pt))
		return nullptr;

	// Iterate through any children
	for (const auto &node : parent->getChildren()) {
		HWND child = getMouseMessageWnd(ev, node._value);
		if (child)
			return child;
	}

	// Final control under mouse
	ev.mouse.x = pt.x;
	ev.mouse.y = pt.y;
	return parent;
}

bool EventLoop::mousePosToClient(CWnd *wnd, POINT &pt) {
	RECT clientRect;

	// Get the mouse position in passed window
	wnd->ScreenToClient(&pt);
	wnd->GetClientRect(&clientRect);

	Common::Rect r = clientRect;
	return r.contains(pt.x, pt.y);
}

bool EventLoop::pollEvents(Common::Event &event) {
	return g_system->getEventManager()->pollEvent(event);
}

void EventLoop::checkForFrameUpdate() {
	// Brief pauses and screen updates
	g_system->delayMillis(10);

	// Trigger any pending timers
	triggerTimers();

	// Handle screen updates
	uint32 time = g_system->getMillis();
	if (time >= _nextFrameTime) {
		_nextFrameTime = time + (1000 / FRAME_RATE);
		AfxGetApp()->getScreen()->update();
	}

	// Cleanup any temporary handle wrapper
	AfxGetApp()->AfxUnlockTempMaps();
}

bool EventLoop::PeekMessage(LPMSG lpMsg, HWND hWnd,
		unsigned int wMsgFilterMin, unsigned int wMsgFilterMax,
		unsigned int wRemoveMsg) {
	checkMessages();
	return _messages.peekMessage(lpMsg, hWnd,
		wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

bool EventLoop::PostMessage(HWND hWnd, unsigned int Msg,
		WPARAM wParam, LPARAM lParam) {
	if (shouldQuit())
		return false;
	if (!hWnd && Msg == WM_PARENTNOTIFY)
		// Hodj minigame launched directly without metagame,
		// so we can ignore the WM_PARENTNOTIFY on closure
		return false;

	assert(hWnd);
	_messages.push(MSG(hWnd, Msg, wParam, lParam));
	return true;
}

void EventLoop::TranslateMessage(LPMSG lpMsg) {
	if ((lpMsg->message == WM_KEYDOWN || lpMsg->message == WM_SYSKEYDOWN) &&
			(isChar((Common::KeyCode)lpMsg->wParam) || Common::isPrint(lpMsg->_ascii))) {
		uint message = (lpMsg->message == WM_SYSKEYDOWN) ?
			WM_SYSCHAR : WM_CHAR;
		WPARAM wParam = lpMsg->_ascii;
		LPARAM lParam = lpMsg->lParam;
		PostMessage(lpMsg->hwnd, message, wParam, lParam);
	}
}

void EventLoop::DispatchMessage(LPMSG lpMsg) {
	CWnd *wnd = CWnd::FromHandle(lpMsg->hwnd);

	if (wnd) {
		wnd->SendMessage(lpMsg->message,
			lpMsg->wParam, lpMsg->lParam);
	}
}

bool EventLoop::isMouseMsg(const Common::Event &ev) const {
	return ev.type == Common::EVENT_MOUSEMOVE ||
		ev.type == Common::EVENT_LBUTTONDOWN ||
		ev.type == Common::EVENT_LBUTTONUP ||
		ev.type == Common::EVENT_RBUTTONDOWN ||
		ev.type == Common::EVENT_RBUTTONUP ||
		ev.type == Common::EVENT_WHEELUP ||
		ev.type == Common::EVENT_WHEELDOWN ||
		ev.type == Common::EVENT_MBUTTONDOWN ||
		ev.type == Common::EVENT_MBUTTONUP;;
}

bool EventLoop::isJoystickMsg(const Common::Event &ev) const {
	return ev.type == Common::EVENT_JOYAXIS_MOTION ||
		ev.type == Common::EVENT_JOYBUTTON_DOWN ||
		ev.type == Common::EVENT_JOYBUTTON_UP;
}

bool EventLoop::shouldQuit() const {
	return g_engine->shouldQuit();
}

void EventLoop::quit() {
	g_engine->quitGame();
}

void EventLoop::SetCapture(HWND hWnd) {
	_captureWin = hWnd;
}

void EventLoop::ReleaseCapture() {
	_captureWin = nullptr;
}

HWND EventLoop::GetCapture() const {
	return _captureWin;
}

void EventLoop::SetFocus(CWnd *wnd) {
	HWND oldFocus = _focusedWin;
	HWND newFocus = wnd ? wnd->m_hWnd : nullptr;

	if (newFocus != _focusedWin) {
		CWnd *focusedWin = CWnd::FromHandle(_focusedWin);
		CWnd *newFocusedWin = CWnd::FromHandle(newFocus);

		if (_focusChangeProc)
			_focusChangeProc(focusedWin, newFocusedWin);

		if (focusedWin) {
			focusedWin->_hasFocus = false;
			focusedWin->SendMessage(WM_KILLFOCUS,
				wnd ? (WPARAM)newFocus : (WPARAM)nullptr);
		}

		_focusedWin = newFocus;
		if (wnd) {
			wnd->_hasFocus = true;
			wnd->SendMessage(WM_SETFOCUS, (WPARAM)oldFocus);
		}
	}
}

void EventLoop::setMousePos(const Common::Point &pt) {
	_mousePos = pt;
	g_system->warpMouse(pt.x, pt.y);
}

MMRESULT EventLoop::joySetCapture(HWND hwnd, unsigned int uJoyID,
		unsigned int uPeriod, bool fChanged) {
	assert(uJoyID == JOYSTICKID1);
	_joystickWin = hwnd;
	return JOYERR_NOERROR;
}

MMRESULT EventLoop::joySetThreshold(unsigned int uJoyID, unsigned int uThreshold) {
	// No implementation
	return JOYERR_NOERROR;
}

MMRESULT EventLoop::joyGetPos(unsigned int uJoyID, LPJOYINFO pji) {
	assert(uJoyID == JOYSTICKID1);

	pji->wXpos = _joystickPos.x;
	pji->wYpos = _joystickPos.y;
	pji->wZpos = 0;
	pji->wButtons = _joystickButtons;

	return JOYERR_NOERROR;
}

MMRESULT EventLoop::joyReleaseCapture(unsigned int uJoyID) {
	assert(uJoyID == JOYSTICKID1);
	return JOYERR_NOERROR;
}

uintptr EventLoop::SetTimer(HWND hWnd, uintptr nIDEvent, unsigned int nElapse,
		void (CALLBACK *lpfnTimer)(HWND, unsigned int, uintptr, uint32)) {
	if (!nIDEvent)
		nIDEvent = ++_timerIdCtr;

	_timers.push_back(TimerEntry(hWnd, nIDEvent, nElapse, lpfnTimer));

	return nIDEvent;
}

bool EventLoop::KillTimer(HWND hWnd, uintptr nIDEvent) {
	for (auto it = _timers.begin(); it != _timers.end(); ++it) {
		if (it->_hWnd == hWnd && it->_idEvent == nIDEvent) {
			_timers.erase(it);
			return true;
		}
	}

	return false;
}

void EventLoop::triggerTimers() {
	uint32 currTime = g_system->getMillis();

	for (auto it = _timers.begin(); it != _timers.end(); ) {
		if (currTime >= it->_nextTriggerTime) {
			// First update the timer for the next time
			it->_nextTriggerTime = currTime + it->_interval;

			if (it->_callback) {
				// Call the callback
				it->_callback(it->_hWnd, WM_TIMER, it->_idEvent, currTime);
			} else {
				// Otherwise, send timer event
				CWnd *wnd = CWnd::FromHandle(it->_hWnd);
				if (wnd)
					wnd->SendMessage(WM_TIMER, it->_idEvent, 0);
			}

			// Since it's conceivable that a timer callback might
			// remove timers, always restart the iterator afterwards
			it = _timers.begin();
		} else {
			++it;
		}
	}
}

EventLoop::TimerEntry::TimerEntry(HWND hWnd, uintptr idEvent,
		uint32 interval, TimerProc callback) :
		_hWnd(hWnd), _idEvent(idEvent),
		_interval(interval), _callback(callback) {
	_nextTriggerTime = g_system->getMillis() + interval;
}

void EventLoop::pause() {
	// Pause and update screen
	g_system->delayMillis(20);
	AfxGetApp()->getScreen()->update();
	AfxGetApp()->checkMessages();
}

bool EventLoop::isChar(Common::KeyCode kc) const {
	return kc == Common::KEYCODE_SPACE ||
		kc == Common::KEYCODE_TAB ||
		kc == Common::KEYCODE_RETURN ||
		kc == Common::KEYCODE_BACKSPACE ||
		kc == Common::KEYCODE_ESCAPE;
}

} // namespace Libs
} // namespace MFC
} // namespace Graphics
