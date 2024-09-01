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

#include "m4/riddle/hotkeys.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_sys.h"
#include "m4/adv_r/other.h"

namespace M4 {
namespace Riddle {

void Hotkeys::add_hot_keys() {
	M4::Hotkeys::add_hot_keys();

	AddSystemHotkey('t', t_cb);
	AddSystemHotkey('u', u_cb);
	AddSystemHotkey('l', l_cb);
	AddSystemHotkey('T', t_cb);
	AddSystemHotkey('U', u_cb);
	AddSystemHotkey('L', l_cb);

	AddSystemHotkey('A', l_cb);
	AddSystemHotkey('S', t_cb);
	AddSystemHotkey('D', u_cb);
	AddSystemHotkey('a', l_cb);
	AddSystemHotkey('s', t_cb);
	AddSystemHotkey('d', u_cb);

	AddSystemHotkey('F', a_cb);
	AddSystemHotkey('f', a_cb);
}

void Hotkeys::toggle_through_cursors(CursorChange cursChange) {
	cursor_states newCursor = kARROW;

	switch (cursChange) {
	case CURSCHANGE_NEXT:
		newCursor = _G(cursor_state) == kUSE ? kARROW :
			static_cast<cursor_states>((int)_G(cursor_state) + 1);
		break;

	case CURSCHANGE_PREVIOUS:
		newCursor = _G(cursor_state) == kARROW ? kUSE :
			static_cast<cursor_states>((int)_G(cursor_state) - 1);
		break;

	case CURSCHANGE_TOGGLE:
		newCursor = (_G(cursor_state) == kARROW) ? kUSE : kARROW;
		break;

	default:
		break;
	}

	switch (newCursor) {
	case kARROW:
		Hotkeys::a_cb(nullptr, nullptr);
		break;
	case kLOOK:
		Hotkeys::l_cb(nullptr, nullptr);
		break;
	case kTAKE:
		Hotkeys::t_cb(nullptr, nullptr);
		break;
	case kUSE:
		Hotkeys::u_cb(nullptr, nullptr);
		break;
	default:
		break;
	}
}

void Hotkeys::escape_key_pressed(void *, void *) {
}

void Hotkeys::show_version(void *, void *) {
	warning("TODO: show_version");
}

void Hotkeys::t_cb(void *, void *) {
//	g_vars->_interface.t_cb();
}

void Hotkeys::u_cb(void *, void *) {
//	g_vars->_interface.u_cb();
}

void Hotkeys::l_cb(void *, void *) {
//	g_vars->_interface.l_cb();
}

void Hotkeys::a_cb(void *, void *) {
//	g_vars->_interface.a_cb();
}

} // namespace Riddle
} // namespace M4
