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
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/libs/event_loop.h"

namespace Bagel {
namespace MFC {
namespace Libs {

#define FRAME_RATE 50

void EventLoop::runEventLoop() {
	MSG msg;

	while (!g_engine->shouldQuit() && !_activeWindows.empty()) {
		CWnd *activeWin = GetActiveWindow();
		if (activeWin->m_nModalResult != -1)
			break;

		if (!GetMessage(msg))
			break;

		if (msg.message != WM_NULL && !PreTranslateMessage(&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		CWnd *activeWin2 = GetActiveWindow();
		if (activeWin2 != nullptr && activeWin2 != activeWin)
			// New top window, so signal to redraw it
			activeWin2->Invalidate();
	}
}

void EventLoop::SetActiveWindow(CWnd *wnd) {
	if (_activeWindows.empty())
		_mainWindow = wnd;
	_activeWindows.push(wnd);
}

void EventLoop::PopActiveWindow() {
	_activeWindows.pop();
}

bool EventLoop::GetMessage(MSG &msg) {
	Libs::Event ev;

	// Queue window repaints if needed and no messages pending
	if (_messages.empty() && _activeWindows.top()->IsWindowDirty())
		PostMessage(_activeWindows.top()->m_hWnd, WM_PAINT, 0, 0);

	// Check for any existing messages
	if (!_messages.empty()) {
		msg = _messages.pop();

	} else {
		// Poll for event in ScummVM event manager
		if (!pollEvents(ev)) {
			msg.message = WM_NULL;
		} else {
			HWND hWnd = nullptr;
			setMessageWnd(ev, hWnd);
			msg = ev;
			msg.hwnd = hWnd;

			if (hWnd) {
				if (isMouseMsg(ev)) {
					// Update saved mouse position
					_mousePos = ev.mouse;

					// For mouse messages, if the highlighted control
					// changes, generate a WM_SETCURSOR event
					if (hWnd != _highlightedWin) {
						// Add mouse leave event if win is still alive
						CWnd *highlightedWin = CWnd::FromHandle(_highlightedWin);
						if (highlightedWin)
							highlightedWin->PostMessage(WM_MOUSELEAVE);

						// Switch to newly highlighted control
						_highlightedWin = hWnd;
						if (_highlightedWin)
							PostMessage(_highlightedWin,
								WM_SETCURSOR, (WPARAM)hWnd,
								MAKELPARAM(HTCLIENT, msg.message)
							);
					}
				}

				if ((msg.message == WM_KEYDOWN || msg.message == WM_KEYUP) &&
						_kbdHookProc) {
					if (_kbdHookProc(HC_ACTION, msg.wParam, msg.lParam))
						msg.message = WM_NULL;
				}
			} else {
				msg.message = WM_NULL;
			}
		}
	}

	return !g_engine->shouldQuit() && msg.message != WM_QUIT;
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
					_joystickPos.x = ev.joystick.position;
				else
					_joystickPos.y = ev.joystick.position;

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
	if (_quitFlag)
		return false;

	if (!_events.empty()) {
		event = _events.pop();
	} else if (!g_system->getEventManager()->pollEvent(event)) {
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

		return false;
	}

	// Check for quit event
	if ((event.type == Common::EVENT_QUIT) ||
		(event.type == Common::EVENT_RETURN_TO_LAUNCHER)) {
		_quitFlag = true;
		return false;
	}

	return true;
}

bool EventLoop::PreTranslateMessage(MSG *pMsg) {
	// No implementation currently
	return false;
}

BOOL EventLoop::PeekMessage(LPMSG lpMsg, HWND hWnd,
		UINT wMsgFilterMin, UINT wMsgFilterMax,
		UINT wRemoveMsg) {
	return _messages.peekMessage(lpMsg, hWnd,
		wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

BOOL EventLoop::PostMessage(HWND hWnd, UINT Msg,
		WPARAM wParam, LPARAM lParam) {
	assert(hWnd && hWnd != (HWND)0xdddddddd);
	_messages.push(MSG(hWnd, Msg, wParam, lParam));
	return true;
}

void EventLoop::TranslateMessage(LPMSG lpMsg) {
	// No implementation
}

void EventLoop::DispatchMessage(LPMSG lpMsg) {
	CWnd *wnd = CWnd::FromHandle(lpMsg->hwnd);
	if (!wnd)
		// Recipient has been destroyed
		return;

	wnd->SendMessage(lpMsg->message,
		lpMsg->wParam, lpMsg->lParam);
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

MMRESULT EventLoop::joySetCapture(HWND hwnd, UINT uJoyID,
		UINT uPeriod, BOOL fChanged) {
	assert(uJoyID == JOYSTICKID1);
	_joystickWin = hwnd;
	return JOYERR_NOERROR;
}

MMRESULT EventLoop::joySetThreshold(UINT uJoyID, UINT uThreshold) {
	// No implementation
	return JOYERR_NOERROR;
}

MMRESULT EventLoop::joyGetPos(UINT uJoyID, LPJOYINFO pji) {
	assert(uJoyID == JOYSTICKID1);

	pji->wXpos = _joystickPos.x;
	pji->wYpos = _joystickPos.y;
	pji->wZpos = 0;
	pji->wButtons = _joystickButtons;

	return JOYERR_NOERROR;
}

MMRESULT EventLoop::joyReleaseCapture(UINT uJoyID) {
	assert(uJoyID == JOYSTICKID1);
	return JOYERR_NOERROR;
}

UINT_PTR EventLoop::SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT nElapse,
		void (CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD)) {
	if (!nIDEvent)
		nIDEvent = ++_timerIdCtr;

	_timers.push_back(TimerEntry(hWnd, nIDEvent, nElapse, lpfnTimer));

	return nIDEvent;
}

BOOL EventLoop::KillTimer(HWND hWnd, UINT_PTR nIDEvent) {
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

EventLoop::TimerEntry::TimerEntry(HWND hWnd, UINT_PTR idEvent,
		DWORD interval, TimerProc callback) :
		_hWnd(hWnd), _idEvent(idEvent),
		_interval(interval), _callback(callback) {
	_nextTriggerTime = g_system->getMillis() + interval;
}

void EventLoop::pause() {
	// Pause and update screen
	g_system->delayMillis(20);
	AfxGetApp()->getScreen()->update();

	// Do polling, and save the events for
	// when we can process them. This allows
	// the backend to keep the mouse cursor updated
	Common::Event ev;
	while (g_system->getEventManager()->pollEvent(ev))
		_events.push(ev);
}

} // namespace Libs
} // namespace MFC
} // namespace Bagel
