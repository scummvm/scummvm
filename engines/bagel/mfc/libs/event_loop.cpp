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

	while (!g_engine->shouldQuit() &&
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
		if (pollEvents(ev))
			// Convert other event types
			msg = ev;
		else
			msg.message = WM_NULL;
	}

	return !g_engine->shouldQuit();
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
	
}

void EventLoop::DispatchMessage(LPMSG lpMsg) {
	CWnd *wnd = CWnd::FromHandle(lpMsg->hwnd);
	assert(wnd);

	wnd->SendMessage(lpMsg->message,
		lpMsg->wParam, lpMsg->lParam);
}

} // namespace Libs
} // namespace MFC
} // namespace Bagel
