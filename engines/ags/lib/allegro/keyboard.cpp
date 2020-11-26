/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/lib/allegro/keyboard.h"
#include "common/algorithm.h"
#include "common/textconsole.h"
#include "common/system.h"
#include "common/events.h"
#include "common/textconsole.h"

namespace AGS3 {

bool key[Common::KEYCODE_LAST];

int install_keyboard() {
	Common::fill(&key[0], &key[Common::KEYCODE_LAST], false);
	return 0;
}

void remove_keyboard() {
}

bool keyboard_needs_poll() {
	// TODO: Check if it's okay to be hardcoded for events, since I'm not sure
	// how to check with ScummVM event manager for pending events
	return true;
}

int poll_keyboard() {
	warning("TODO: poll_keyboard");
	return false;
}

void simulate_keypress(int keycode) {
	simulate_ukeypress(keycode, keycode);
}

void simulate_ukeypress(int keycode, int scancode) {
	Common::Event event;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.ascii = scancode;
	event.kbd.keycode = (Common::KeyCode)keycode;
	g_system->getEventManager()->pushEvent(event);

	event.type = Common::EVENT_KEYUP;
	g_system->getEventManager()->pushEvent(event);
}

bool keypressed() {
	warning("TODO: keypressed");
	return true;
}

int readkey(void) {
	warning("TODO: readkey");
	return 0;
}

} // namespace AGS3
