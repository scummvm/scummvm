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
 * but WITHOUT ANY WARRANTY {} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/adv_r/adv_interface.h"
#include "m4/gui/gui_vmng.h"
#include "m4/vars.h"

namespace M4 {

void Interface::init(int arrow, int wait, int look, int grab, int use) {
	_arrow = arrow;
	_wait = wait;
	_look = look;
	_grab = grab;
	_use = use;
}

void Interface::showWaitCursor() {
	mouse_set_sprite(_wait);
}

void Interface::show() {
	warning("TODO: Interface::show");
}

void Interface::hide() {
	if (_shown) {
		vmng_screen_hide(_G(gameInterfaceBuff));
		_visible = false;
	}
}

void Interface::track_hotspots_refresh() {
	warning("TODO: track_hotspots_refresh");
}


void interface_hide() {
	_GI().hide();
}

void interface_show() {
	_GI().show();
}

void track_hotspots_refresh() {
	_GI().track_hotspots_refresh();
}

bool intr_EventHandler(void *bufferPtr, int32 eventType, int32 event, int32 x, int32 y, bool *z) {
	warning("TODO: intr_EventHandler");
	return true;
}

void intr_cancel_sentence() {
	_GI().cancel_sentence();
}

} // End of namespace M4
