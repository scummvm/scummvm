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

#include "xeen/dialogs/credits_screen.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

void CreditsScreen::show(XeenEngine *vm) {
	CreditsScreen *dlg = new CreditsScreen(vm);

	switch (vm->getGameID()) {
	case GType_Clouds:
		dlg->execute(Res.CLOUDS_CREDITS);
		break;
	case GType_Swords:
		dlg->execute(Res.SWORDS_CREDITS1);
		dlg->execute(Res.SWORDS_CREDITS2);
		break;
	default:
		dlg->execute(Res.DARK_SIDE_CREDITS);
		break;
	}

	delete dlg;
}

void CreditsScreen::execute(const char *content) {
	Screen &screen = *_vm->_screen;
	Windows &windows = *_vm->_windows;
	EventsManager &events = *_vm->_events;

	// Handle drawing the credits screen
	doScroll(true, false);
	windows[GAME_WINDOW].close();

	screen.loadBackground("marb.raw");
	windows[0].writeString(content);
	doScroll(false, false);

	events.setCursor(0);
	windows[0].update();
	clearButtons();

	// Wait for keypress
	while (!_vm->shouldExit() && !events.isKeyMousePressed())
		events.pollEventsAndWait();

	doScroll(true, false);
}

} // End of namespace Xeen
