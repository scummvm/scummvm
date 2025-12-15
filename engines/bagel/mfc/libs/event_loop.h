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
#include "common/list.h"
#include "common/stack.h"
#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/libs/events.h"
#include "bagel/mfc/joystickapi.h"

namespace Bagel {
namespace MFC {
namespace Libs {

typedef void(*TimerProc)(
	HWND hwnd,       // handle of window associated with the timer (can be NULL)
	unsigned int uMsg,       // WM_TIMER message (always WM_TIMER)
	uintptr idEvent,// timer identifier (from SetTimer or returned value)
	uint32 dwTime     // system time when the callback was called (in milliseconds)
);

typedef Common::KeyCode(*KeybindProc)(int key);
typedef void (*FocusChangeProc)(CWnd *oldFocus, CWnd *newFocus);

#define JOYSTICK_REST_POS 32767

class WndList : public Common::List<CWnd *> {
public:
	bool contains(CWnd *wnd) const {
		for (auto it = begin(); it != end(); ++it) {
			if (*it == wnd)
				return true;
		}

		return false;
	}

	void add(CWnd *wnd) {
		if (!contains(wnd))
			push_back(wnd);
	}
	void remove(CWnd *wnd) {
		Common::List<CWnd *>::remove(wnd);
	}
	void clear() {
		Common::List<CWnd *>::clear();
	}
};


class EventLoop {
	struct TimerEntry {
		HWND _hWnd = nullptr;
		uintptr _idEvent = 0;
		uint32 _interval = 0;
		uint32 _nextTriggerTime = 0;
		TimerProc _callback = nullptr;

		TimerEntry() {}
		TimerEntry(HWND hWnd, uintptr idEvent,
			uint32 interval, TimerProc callback);
	};
	typedef Common::List<TimerEntry> TimerList;

private:
	CWnd *_mainWindow = nullptr;
	Common::Stack<CWnd *> _activeWindows;
	WndList _updateWnds;
	HWND _highlightedWin = nullptr;
	HWND _captureWin = nullptr;
	HWND _focusedWin = nullptr;
	HWND _joystickWin = nullptr;
	Libs::EventQueue _messages;
	TimerList _timers;
	int _timerIdCtr = 0;
	uint32 _nextFrameTime = 0;
	Common::Point _joystickPos = { JOYSTICK_REST_POS, JOYSTICK_REST_POS };
	Common::Point _mousePos;
	uint _joystickButtons = 0;
	Array<HOOKPROC> _kbdHookProc;
	int _idleCtr = 0;
	KeybindProc _keybindProc = nullptr;
	FocusChangeProc _focusChangeProc = nullptr;

private:
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
	 * Returns true if the event is joystick related
	 */
	bool isJoystickMsg(const Common::Event &ev) const;

	/**
	 * Converts a position to be relative to a given window
	 */
	bool mousePosToClient(CWnd *wnd, POINT &pt);

	/**
	 * Trigger any pending timers
	 */
	void triggerTimers();

	/**
	 * Returns true if a keycode is one that
	 * generates a WM_CHAR or WM_SYSCHAR event
	 */
	bool isChar(Common::KeyCode kc) const;

	/**
	 * Called when there are no pending messages.
	 * Handles screen frame updates and timers
	 */
	void checkForFrameUpdate();

public:
	EventLoop() {}
	virtual ~EventLoop() {}

	void runEventLoop();
	void SetActiveWindow(CWnd *wnd);
	void PopActiveWindow();
	CWnd *GetActiveWindow() const {
		return _activeWindows.empty() ? nullptr :
			_activeWindows.top();
	}

	/**
	 * Shows a modal dialog
	 */
	void doModal(CWnd *wnd);

	/**
	 * Polls for any pending messages and adds
	 * them to the messages queue
	 */
	void checkMessages();

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
	bool PeekMessage(LPMSG lpMsg, HWND hWnd,
		unsigned int wMsgFilterMin, unsigned int wMsgFilterMax,
		unsigned int wRemoveMsg);

	bool PostMessage(HWND hWnd, unsigned int Msg,
		WPARAM wParam, LPARAM lParam);

	void TranslateMessage(LPMSG lpMsg);

	void DispatchMessage(LPMSG lpMsg);

	/**
	 * Returns whether the app should quit.
	 */
	bool shouldQuit() const;

	/**
	 * Quit the game
	 */
	void quit();

	void SetCapture(HWND hWnd);
	void ReleaseCapture();
	HWND GetCapture() const;
	Common::Point getMousePos() const {
		return _mousePos;
	}
	void setMousePos(const Common::Point &pt);

	void SetFocus(CWnd *wnd);
	CWnd *GetFocus() const {
		return _focusedWin;
	}

	MMRESULT joySetCapture(HWND hwnd, unsigned int uJoyID,
		unsigned int uPeriod, bool fChanged);
	MMRESULT joySetThreshold(unsigned int uJoyID, unsigned int uThreshold);
	MMRESULT joyGetPos(unsigned int uJoyID, LPJOYINFO pji);
	MMRESULT joyReleaseCapture(unsigned int uJoyID);

	HHOOK HookKeyboard(HOOKPROC proc) {
		_kbdHookProc.push_back(proc);
		return (HHOOK)proc;
	}
	void UnhookKeyboard(HHOOK hook) {
		assert(_kbdHookProc.contains((HOOKPROC)hook));
		_kbdHookProc.remove((HOOKPROC)hook);
	}

	uintptr SetTimer(HWND hWnd, uintptr nIDEvent, unsigned int nElapse,
		void (CALLBACK *lpfnTimer)(HWND, unsigned int, uintptr, uint32));
	bool KillTimer(HWND hWnd, uintptr nIDEvent);

	void pause();

	WndList &afxUpdateWnds() {
		return _updateWnds;
	}

	virtual bool OnIdle(long lCount) {
		return false;
	}

	void setKeybinder(KeybindProc proc) {
		_keybindProc = proc;
	}

	void setFocusChangeProc(FocusChangeProc proc) {
		_focusChangeProc = proc;
	}
};

} // namespace Libs
} // namespace MFC
} // namespace Bagel

#endif
