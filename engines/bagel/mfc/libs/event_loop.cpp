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

void EventLoop::runEventLoop(CWnd *modalDialog) {
	_modalDialog = modalDialog;

	MSG msg;

	while (!g_engine->shouldQuit() && _mainWindow &&
			(!_modalDialog || _modalDialog->m_nModalResult == -1)) {
		if (!GetMessage(msg))
			break;

		if (msg.message != WM_NULL && !PreTranslateMessage(&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	_modalDialog = nullptr;
}

bool EventLoop::GetMessage(MSG &msg) {
	Libs::Event ev;

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
				// For mouse messages, if the highlighted control
				// changes, generate a WM_SETCURSOR event
				if (isMouseMsg(ev) && CWnd::FromHandle(hWnd) != _highlightedWin) {
					_highlightedWin = CWnd::FromHandle(hWnd);
					PostMessage(
						_modalDialog ? _modalDialog : _mainWindow,
						WM_SETCURSOR, (WPARAM)hWnd,
						MAKELPARAM(HTCLIENT, msg.message)
					);
				}
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
		if (_joystickWin) {
			switch (ev.type) {
			case Common::EVENT_JOYAXIS_MOTION:
				if (ev.joystick.axis == 0)
					_joystickPos.x = ev.joystick.position;
				else
					_joystickPos.y = ev.joystick.position;

				_joystickWin->SendMessage(MM_JOY1MOVE, JOYSTICKID1,
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

	// Fallback, send message to any active dialog,
	// or worst case to the main application window
	if (_modalDialog)
		hWnd = _modalDialog->m_hWnd;
	else
		hWnd = _mainWindow->m_hWnd;
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
	if (_modalDialog && ev.type == Common::EVENT_MOUSEMOVE &&
		!_modalDialog->_windowRect.contains(ev.mouse)) {
		hWnd = _mainWindow->m_hWnd;
		return;
	}

	CWnd *wnd = _modalDialog ? _modalDialog : _mainWindow;
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

	if (!g_system->getEventManager()->pollEvent(event)) {
		// Brief pauses and screen updates
		g_system->delayMillis(10);

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
	_messages.push(MSG(hWnd, Msg, wParam, lParam));
	return true;
}

void EventLoop::TranslateMessage(LPMSG lpMsg) {
	// No implementation
}

void EventLoop::DispatchMessage(LPMSG lpMsg) {
	CWnd *wnd = CWnd::FromHandle(lpMsg->hwnd);
	assert(wnd);

	// FIXME: This goes in hand with our currently
	// calling the runEventLoop for non-modal windows.
	// It ensures closing the window breaks the modal event loop
	if (wnd == _modalDialog && lpMsg->message == WM_CLOSE)
		_modalDialog = nullptr;

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
	if (wnd != _focusedWin) {
		CWnd *oldFocus = _focusedWin;

		if (_focusedWin) {
			_focusedWin->_hasFocus = false;
			_focusedWin->SendMessage(WM_KILLFOCUS,
				wnd ? (WPARAM)wnd->m_hWnd : (WPARAM)nullptr);
		}

		_focusedWin = wnd;
		if (_focusedWin) {
			_focusedWin->_hasFocus = true;
			_focusedWin->SendMessage(WM_SETFOCUS,
				oldFocus ? (WPARAM)oldFocus->m_hWnd : (WPARAM)nullptr);
		}
	}
}

bool EventLoop::validateDestroyedWnd(HWND hWnd) {
	MSG msg;
	return !_messages.peekMessage(&msg, hWnd, 0, 0, false);
}

MMRESULT EventLoop::joySetCapture(HWND hwnd, UINT uJoyID,
		UINT uPeriod, BOOL fChanged) {
	assert(uJoyID == JOYSTICKID1);
	_joystickWin = CWnd::FromHandle(hwnd);
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

} // namespace Libs
} // namespace MFC
} // namespace Bagel
