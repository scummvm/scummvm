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

#include "mm/mm1/views_enh/locations/inn.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

Inn::Inn() : ScrollView("Inn") {
	_bounds.setBorderSize(10);
}

bool Inn::msgFocus(const FocusMessage &msg) {
	if (g_maps->_currentMap)
		// Update the starting town
		g_globals->_startingTown = (Maps::TownId)g_maps->_currentMap->dataByte(Maps::MAP_ID);

	// Save the roster
	g_globals->_roster.update(_partyChars);
	g_globals->_roster.save();

	// Get a list of characters in the town
	_charNums.clear();
	for (uint i = 0; i < ROSTER_COUNT; ++i) {
		if (g_globals->_roster._towns[i] == g_globals->_startingTown)
			_charNums.push_back(i);
	}

	// Build up a list of characters in the party
	// (for if we're opening the inn from in-game)
	_partyChars.clear();
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		for (uint j = 0; j < ROSTER_COUNT; ++j) {
			if (g_globals->_roster[j] == g_globals->_party[i]) {
				_partyChars.push_back(j);
				break;
			}
		}
	}

	return true;
}

void Inn::draw() {
	ScrollView::draw();
	Graphics::ManagedSurface s = getSurface();

	// Write title
	Common::String title = Common::String::format(
		STRING["dialogs.inn.title"].c_str(),
		'0' + g_globals->_startingTown);
	title += STRING[Common::String::format(
		"stats.towns.%d", g_globals->_startingTown)];
	setReduced(false);
	writeString(0, 0, title, ALIGN_MIDDLE);

	if (_charNums.empty()) {
		writeString(0, 40, STRING["dialogs.misc.no_characters"], ALIGN_MIDDLE);

	} else {
		// Write out the roster
		setReduced(true);

		for (uint idx = 0; idx < _charNums.size(); ++idx) {
			uint charNum = _charNums[idx];
			Character &c = g_globals->_roster[charNum];

			Common::Point pt(_innerBounds.left + _innerBounds.width() / 3
				* (idx % 3), 20 + 30 * (idx / 3));
			c._faceSprites.draw(&s, 0, pt);
			writeString(pt.x + 30, pt.y + 10, c._name);
		}
	}
}

bool Inn::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode >= Common::KEYCODE_a &&
		msg.keycode < (Common::KeyCode)(Common::KEYCODE_a + _charNums.size())) {
		int charNum = _charNums[msg.keycode - Common::KEYCODE_a];

		if (msg.flags & Common::KBD_CTRL) {
			// Toggle in party
			if (_partyChars.contains(charNum))
				_partyChars.remove(charNum);
			else
				_partyChars.push_back(charNum);

			redraw();

		} else if (msg.flags == 0) {
			// View character
			g_globals->_currCharacter = &g_globals->_roster[charNum];
			//_characterView.addView();
		}
		return true;

	} else if (msg.keycode == Common::KEYCODE_x) {
		exitInn();
		return true;
	}

	return false;
}

bool Inn::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		replaceView("MainMenu");
		return true;
	case KEYBIND_SELECT:
		exitInn();
		return true;
	default:
		break;
	}

	return false;
}

void Inn::exitInn() {
	if (_partyChars.empty())
		return;

	// Load party from selected characters
	g_globals->_party.clear();
	for (uint i = 0; i < _partyChars.size(); ++i)
		g_globals->_party.push_back(
			g_globals->_roster[_partyChars[i]]);
	g_globals->_currCharacter = &g_globals->_party.front();

	// Load the given town
	g_globals->_maps.loadTown(g_globals->_startingTown);
}

} // namespace Location
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
