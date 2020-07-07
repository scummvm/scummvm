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

#include "xeen/dialogs/dialogs_awards.h"
#include "xeen/party.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

void Awards::show(XeenEngine *vm, const Character *ch) {
	Awards *dlg = new Awards(vm);
	dlg->execute(ch);
	delete dlg;
}

void Awards::execute(const Character *ch) {
	EventsManager &events = *g_vm->_events;
	Windows &windows = *g_vm->_windows;
	Common::StringArray awards;
	int numAwards;
	Mode oldMode = g_vm->_mode;
	int topIndex = 0;

	loadStrings("award.bin", 1);
	addButtons();

	// Open the window and draw contents
	bool win29Open = windows[29]._enabled;
	if (!win29Open) {
		windows[29].open();
		windows[30].open();
	}

	windows[29].writeString(Res.AWARDS_TEXT);
	drawButtons(&windows[0]);

	while (!_vm->shouldExit()) {
		// Build up a list of awards the character has
		awards.clear();
		awards.resize(AWARDS_TOTAL);
		numAwards = 0;

		for (int awardNum = 0; awardNum < AWARDS_TOTAL; ++awardNum) {
			if (ch->hasAward(awardNum)) {
				if (awardNum == WARZONE_AWARD) {
					// # Warzone Wins
					awards[numAwards] = Common::String::format(_textStrings[9].c_str(), ch->getAwardCount(WARZONE_AWARD));
				} else if (awardNum == 17) {
					// Legendary Race
					awards[numAwards] = Common::String::format(_textStrings[17].c_str(),
						Res.RACE_NAMES[ch->_race]);
				} else {
					awards[numAwards] = _textStrings[awardNum];
				}
				++numAwards;
			}
		}

		// If no awards, add in a message indicating so
		if (numAwards == 0) {
			awards[1] = Res.NO_AWARDS;
		}

		Common::String msg = Common::String::format(Res.AWARDS_FOR,
			ch->_name.c_str(), Res.CLASS_NAMES[ch->_class],
			awards[topIndex].c_str(),
			awards[topIndex + 1].c_str(),
			awards[topIndex + 2].c_str(),
			awards[topIndex + 3].c_str(),
			awards[topIndex + 4].c_str(),
			awards[topIndex + 5].c_str(),
			awards[topIndex + 6].c_str(),
			awards[topIndex + 7].c_str(),
			awards[topIndex + 8].c_str()
		);
		windows[30].writeString(msg);
		windows[24].update();

		// Wait for keypress
		do {
			events.pollEventsAndWait();
			checkEvents(_vm);
		} while (!g_vm->shouldExit() && !_buttonValue);

		if (_buttonValue == Common::KEYCODE_ESCAPE) {
			break;
		} else if (_buttonValue == Common::KEYCODE_u) {
			topIndex = MAX(topIndex - 1, 0);
		} else if (_buttonValue == Common::KEYCODE_d) {
			if ((++topIndex + 9) > numAwards)
				--topIndex;
		}
	}

	// Close the window
	if (!win29Open) {
		windows[30].close();
		windows[29].close();
	}

	g_vm->_mode = oldMode;
}

void Awards::addButtons() {
	_iconSprites.load("award.icn");
	addButton(Common::Rect(216, 109, 240, 129), Common::KEYCODE_u, &_iconSprites);
	addButton(Common::Rect(250, 109, 274, 129), Common::KEYCODE_d, &_iconSprites);
	addButton(Common::Rect(284, 109, 308, 129), Common::KEYCODE_ESCAPE, &_iconSprites);
}

} // End of namespace Xeen
