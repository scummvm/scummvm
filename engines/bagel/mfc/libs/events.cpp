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
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/libs/events.h"

namespace Bagel {
namespace MFC {
namespace Libs {

int Event::_flags;
int Event::_mouseX;
int Event::_mouseY;

void Event::init() {
	_flags = 0;
	_mouseX = _mouseY = 0;
}

Event::operator MSG() const {
	MSG msg;

	// Reset fields
	msg.hwnd = (HWND)0;
	msg.message = 0;
	msg.wParam = msg.lParam = 0;
	msg.time = g_system->getMillis();

	// Handle quit messages
	if (type == Common::EVENT_QUIT ||
		type == Common::EVENT_RETURN_TO_LAUNCHER) {
		msg.message = WM_QUIT;
		return msg;
	}

	// For mouse events, set the position
	if (type >= Common::EVENT_MOUSEMOVE &&
		type <= Common::EVENT_MBUTTONUP) {
		_mouseX = mouse.x;
		_mouseY = mouse.y;

		msg.lParam = MAKELPARAM(_mouseX, _mouseY);

		switch (type) {
		case Common::EVENT_LBUTTONDOWN:
			_flags |= MK_LBUTTON;
			break;
		case Common::EVENT_LBUTTONUP:
			_flags &= ~MK_LBUTTON;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_flags |= MK_RBUTTON;
			break;
		case Common::EVENT_RBUTTONUP:
			_flags &= ~MK_RBUTTON;
			break;
		case Common::EVENT_MBUTTONDOWN:
			_flags |= MK_MBUTTON;
			break;
		case Common::EVENT_MBUTTONUP:
			_flags &= ~MK_MBUTTON;
			break;
		default:
			break;
		}

		msg.wParam = _flags;
	}

	if (type == Common::EVENT_KEYDOWN ||
		type == Common::EVENT_KEYUP) {
		// Update flags
		if (kbd.flags & Common::KBD_CTRL)
			_flags |= MK_CONTROL;
		else
			_flags &= ~MK_CONTROL;

		if (kbd.flags & Common::KBD_SHIFT)
			_flags |= MK_SHIFT;
		else
			_flags &= ~MK_SHIFT;
	}

	switch (type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
		if (kbd.flags & Common::KBD_ALT)
			msg.message = (type == Common::EVENT_KEYDOWN) ?
				WM_SYSKEYDOWN : WM_SYSKEYUP;
		else
			msg.message = (type == Common::EVENT_KEYDOWN) ?
				WM_KEYDOWN : WM_KEYUP;

		msg.wParam = kbd.keycode;
		msg.lParam = (kbdRepeat ? 1 : 0) |
		    ((uint)kbd.keycode << 16) |
		    ((kbd.keycode >= 256 ? 1 : 0) << 24) |
		    (((kbd.flags & Common::KBD_ALT) ? 1 : 0) << 29) |
			((type == Common::EVENT_KEYUP ? 1 : 0) << 30) |
			((type == Common::EVENT_KEYDOWN ? 1 : 0) << 31);
		msg._kbdFlags = kbd.flags;
		msg._ascii = kbd.ascii;
		break;

	case Common::EVENT_MOUSEMOVE:
		msg.message = WM_MOUSEMOVE;
		break;
	case Common::EVENT_LBUTTONDOWN:
		msg.message = WM_LBUTTONDOWN;
		break;
	case Common::EVENT_LBUTTONUP:
		msg.message = WM_LBUTTONUP;
		break;
	case Common::EVENT_RBUTTONDOWN:
		msg.message = WM_RBUTTONDOWN;
		break;
	case Common::EVENT_RBUTTONUP:
		msg.message = WM_RBUTTONUP;
		break;
	case Common::EVENT_MBUTTONDOWN:
		msg.message = WM_MBUTTONDOWN;
		break;
	case Common::EVENT_MBUTTONUP:
		msg.message = WM_MBUTTONUP;
		break;
	default:
		msg.message = WM_NULL;
		break;
	}

	return msg;
}

/*--------------------------------------------*/

MSG EventQueue::pop() {
	assert(!empty());
	MSG result = _queue.back();
	_queue.remove_at(_queue.size() - 1);

	return result;
}

bool EventQueue::peekMessage(MSG *lpMsg, HWND hWnd,
		unsigned int wMsgFilterMin, unsigned int wMsgFilterMax,
		unsigned int wRemoveMsg) {
	bool result = false;

	for (uint i = 0; i < _queue.size(); ++i) {
		const MSG &msg = _queue[i];
		if ((hWnd == nullptr || hWnd == msg.hwnd) &&
			((wMsgFilterMin == 0 && wMsgFilterMax == 0) ||
				(msg.message >= wMsgFilterMin && msg.message <= wMsgFilterMax))) {
			// Found a matching message
			*lpMsg = msg;

			if (wRemoveMsg & PM_REMOVE)
				// Remove the message
				_queue.remove_at(i);

			return true;
		}
	}

	return result;
}

} // namespace Libs
} // namespace MFC
} // namespace Bagel
