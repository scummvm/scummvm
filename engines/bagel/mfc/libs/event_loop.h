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

#ifndef BAGEL_MFC_LIBS_EVENT_LOOP_H
#define BAGEL_MFC_LIBS_EVENT_LOOP_H

#include "common/events.h"
#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/libs/events.h"

namespace Bagel {
namespace MFC {
namespace Libs {

class EventLoop {
private:
	CWnd *&_mainWindow;
	CWnd *_modalDialog = nullptr;
	CWnd *_highlightedWin = nullptr;
	CWnd *_captureWin = nullptr;
	Libs::EventQueue _messages;
	uint32 _nextFrameTime = 0;
	bool _quitFlag = false;

	/**
	 * Get any pending event
	 * @return      Returns false if app should quit
	 */
	bool GetMessage(MSG &msg);

	/**
	 * Gets the destination window for a message
	*/
	void setMessageWnd(Common::Event &ev, HWND &hWnd);

	/**
	 * Determine the window to send a mouse message to,
	 * and translates the event mouse position to be
	 * relative to the control's client rect
	 */
	void setMouseMessageWnd(Common::Event &ev, HWND &hWnd);
	HWND getMouseMessageWnd(Common::Event &ev, CWnd *parent);

	/**
	 * Returns true if the event is mouse related
	 */
	bool isMouseMsg(const Common::Event &ev) const;

	/**
	 * Converts a position to be relative to a given window
	 */
	bool mousePosToClient(CWnd *wnd, POINT &pt);

public:
	EventLoop(CWnd *&mainWin) : _mainWindow(mainWin) {
	}

	void runEventLoop(CWnd *modalDialog = nullptr);

	/**
	 * Polls the ScummVM backend for any pending events.
	 * Should be mostly only called internally by the main
	 * event loop, but there are rare cases, like in the
	 * Hodj n Podj movie window, where it was convenient to
	 * to call in a simplified event loop.
	 */
	bool pollEvents(Common::Event &event);

	/**
	 * Checks the pending event queue for a message
	 * with the specified criteria.
	 */
	BOOL PeekMessage(LPMSG lpMsg, HWND hWnd,
		UINT wMsgFilterMin, UINT wMsgFilterMax,
		UINT wRemoveMsg);

	BOOL PostMessage(HWND hWnd, UINT Msg,
		WPARAM wParam, LPARAM lParam);

	/**
	 * Allow for MFC-specific message preprocessing
	 */
	bool PreTranslateMessage(MSG *pMsg);

	void TranslateMessage(LPMSG lpMsg);

	void DispatchMessage(LPMSG lpMsg);

	bool shouldQuit() const {
		return _quitFlag;
	}
	void quit() {
		_quitFlag = true;
	}

	void SetCapture(HWND hWnd);
	void ReleaseCapture();
	HWND GetCapture() const;

	void SetActiveWindow(CWnd *wnd) {
		// FIXME: modals are blocking, this shouldn't.
		// Which may become an issue if anything opened
		// multiple independent windows
		runEventLoop(wnd);
	}

	/**
	 * Checked that the passed HWND doesn't
	 * have any future messages pending for it
	 */
	bool validateDestroyedWnd(HWND hWnd);
};

} // namespace Libs
} // namespace MFC
} // namespace Bagel

#endif
