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

#include "mm/xeen/dialogs/credits_screen.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/xeen.h"

namespace MM {
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
	Common::String ttsMessage;
	windows[0].writeString(content, false, &ttsMessage);
	doScroll(false, false);

#ifdef USE_TTS
	speakText(ttsMessage, (_vm->getGameID() != GType_Swords || content == Res.SWORDS_CREDITS1));
#endif

	events.setCursor(0);
	windows[0].update();
	clearButtons();

	// Wait for keypress
	while (!_vm->shouldExit() && !events.isKeyMousePressed())
		events.pollEventsAndWait();

#ifdef USE_TTS
	_vm->stopTextToSpeech();
#endif
	doScroll(true, false);
}

#ifdef USE_TTS

void CreditsScreen::speakText(const Common::String &text, bool firstCreditsScreen) const {
	if (_vm->getGameID() == GType_Swords && firstCreditsScreen) {
		uint index = 0;
		// Developed/published by
		_vm->sayText(getNextTextSection(text, index, 2));

		// Next four headers are separate from their corresponding credits. First get the headers, then voice the person
		// for the first header, and then voice the second header and second person
		for (uint8 i = 0; i < 2; ++i) {
			_vm->sayText(getNextTextSection(text, index));
			Common::String nextHeader = getNextTextSection(text, index);
			_vm->sayText(getNextTextSection(text, index));
			_vm->sayText(nextHeader);
			_vm->sayText(getNextTextSection(text, index));
		}

		// Same as first four headers, but with two people at a time instead of one
		for (uint8 i = 0; i < 2; ++i) {
			// First two headers
			_vm->sayText(getNextTextSection(text, index));
			Common::String nextHeader = getNextTextSection(text, index);

			// First people listed
			_vm->sayText(getNextTextSection(text, index));
			Common::String nextHeaderPerson = getNextTextSection(text, index);

			// Second person listed under first header
			_vm->sayText(getNextTextSection(text, index));

			// Next header
			_vm->sayText(nextHeader);
			_vm->sayText(nextHeaderPerson);

			// Second person for second header
			if (i == 0) {
				_vm->sayText(getNextTextSection(text, index));
			} else {	// Last header has more than two people
				_vm->sayText(text.substr(index));
			}
		}
	} else {
		_vm->sayText(text);
	}
}

#endif

} // End of namespace Xeen
} // End of namespace MM
