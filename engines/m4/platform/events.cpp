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
#include "m4/platform/events.h"
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {

/*
 *  Define call mask bit fields
 */

enum {
	CursorPositionChanged = 0,
	LeftButtonPressed, LeftButtonReleased, RightButtonPressed,
	RightButtonReleased
};

/*
 *  Define call mask values
 */
#define CPC				((uint16)(1 << CursorPositionChanged))
#define LBD				((uint16)(1 << LeftButtonPressed))
#define LBU				((uint16)(1 << LeftButtonReleased))
#define LBH				((uint16)(1 << LeftButtonHold))
#define RBD				((uint16)(1 << RightButtonPressed))
#define RBU				((uint16)(1 << RightButtonReleased))
#define RBH				((uint16)(1 << RightButtonHold))

#define LBC				(LBD + LBU)
#define RBC				(RBD + RBU)
#define MBC				(MBD + MBU)
#define MSA				(LBC + RBC + CPC)
#define OEMA			(LBC + RBC + MBC + CPC)

#define _MLD (_mouseStateEvent & LBD)
#define _ClearMLD _mouseStateEvent &= ~LBD
#define _MLU (_mouseStateEvent & LBU)
#define _ClearMLU _mouseStateEvent &= ~LBU

#define _MRD (_mouseStateEvent & RBD)
#define _ClearMRD _mouseStateEvent &= ~RBD
#define _MRU (_mouseStateEvent & RBU)
#define _ClearMRU _mouseStateEvent &= ~RBU

#define _MMOVE ((_mouseX != _oldX) || (_mouseY != _oldY))
#define _MSAVE _oldX = _mouseX; _oldY = _mouseY

Events *g_events;

Events::Events() {
	g_events = this;
}

Events::~Events() {
	g_events = nullptr;
}

void Events::process() {
	pollEvents();
}

void Events::pollEvents() {
	Common::Event ev;
	while (g_system->getEventManager()->pollEvent(ev)) {
		if (ev.type == Common::EVENT_QUIT || ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
			_G(kernel).going = false;
		} else if (ev.type >= Common::EVENT_MOUSEMOVE && ev.type <= Common::EVENT_MBUTTONUP) {
			handleMouseEvent(ev);
		} else if (ev.type == Common::EVENT_KEYDOWN || ev.type == Common::EVENT_KEYUP) {
			handleKeyboardEvent(ev);
		}
	}
}

void Events::handleMouseEvent(const Common::Event &ev) {
	_mouseX = ev.mouse.x;
	_mouseY = ev.mouse.y;

	switch (ev.type) {
	case Common::EVENT_MOUSEMOVE:
		_mouseStateEvent |= CPC;
		break;
	case Common::EVENT_LBUTTONDOWN:
		_mouseStateEvent |= LBD;
		ButtonState = 1;
		break;
	case Common::EVENT_LBUTTONUP:
		_mouseStateEvent |= LBU;
		ButtonState = 0;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_mouseStateEvent |= RBD;
		ButtonState = 2;
		break;
	case Common::EVENT_RBUTTONUP:
		_mouseStateEvent |= RBU;
		ButtonState = 0;
		break;

	case Common::EVENT_WHEELDOWN:
		_G(toggle_cursor) = CURSCHANGE_NEXT;
		break;
	case Common::EVENT_WHEELUP:
		_G(toggle_cursor) = CURSCHANGE_PREVIOUS;
		break;
	case Common::EVENT_MBUTTONDOWN:
		_G(toggle_cursor) = CURSCHANGE_TOGGLE;
		break;

	default:
		break;
	}
}

void Events::handleKeyboardEvent(const Common::Event &ev) {
	if (ev.type == Common::EVENT_KEYDOWN && _pendingKeys.size() < 16)
		_pendingKeys.push(ev.kbd);
}

MouseEvent Events::mouse_get_event() {
	process();

	switch (_mouse_state) {
	case _MS_no_event:
		if (_MLD) {
			_ClearMLD;
			if (_dclickTime && (timer_read_60() < _dclickTime)) {
				_mouse_state = _MS_doubleclick_Down;
				_dclickTime = 0;
				return _ME_doubleclick;
			}
			_dclickTime = 0;
			_mouse_state = _MS_L_clickDown;
			return _ME_L_click;
		}
		if (_MRD) {
			_ClearMRD;
			_mouse_state = _MS_R_clickDown;
			return _ME_R_click;
		}
		if (_MMOVE) {
			_MSAVE;
			return _ME_move;
		}
		return _ME_no_event;

	case _MS_L_clickDown:
		if (_MLU || !ButtonState) {
			_dclickTime = timer_read_60() + 15;
			_ClearMLU;
			_mouse_state = _MS_no_event;
			return _ME_L_release;
		}
		if (_MMOVE) {
			_MSAVE;
			return _ME_L_drag;
		}
		return _ME_L_hold;

	case _MS_R_clickDown:
		if (_MRU) {
			_ClearMRU;
			_mouse_state = _MS_no_event;
			_G(toggle_cursor) = CURSCHANGE_NEXT;
			return _ME_R_release;
		}
		if (_MMOVE) {
			_MSAVE;
			return _ME_R_drag;
		}
		return _ME_R_hold;

	case _MS_doubleclick_Down:
		if (_MLU) {
			_ClearMLU; _ClearMLD;
			_mouse_state = _MS_no_event;
			return _ME_doubleclick_release;
		}
		if (_MMOVE) {
			_MSAVE;
			return _ME_doubleclick_drag;
		}
		return _ME_doubleclick_hold;

	default:
		return _ME_no_event;
	}

	return _ME_no_event;
}

bool Events::util_kbd_check(int32 *parm1) {
	process();

	if (!parm1 || _pendingKeys.empty())
		return false;

	Common::KeyState ks = _pendingKeys.pop();
	if (is_mod_key(ks))
		return false;

	*parm1 = ks.keycode | ((ks.flags & (Common::KBD_CTRL | Common::KBD_ALT)) << 16);
	return true;
}

void Events::delay(uint amount) {
	uint32 beginTime = g_system->getMillis(), newTime;

	do {
		krn_pal_game_task();
		g_system->delayMillis(5);
		process();

		newTime = g_system->getMillis();
	} while (!g_engine->shouldQuit() && newTime < (beginTime + amount));
}

MouseEvent mouse_get_event() {
	return g_events->mouse_get_event();
}

bool util_kbd_check(int32 *parm1) {
	return g_events->util_kbd_check(parm1);
}

} // namespace M4
