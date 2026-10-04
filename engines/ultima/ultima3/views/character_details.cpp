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

#include "ultima/ultima3/views/character_details.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

CharacterDetails::CharacterDetails() : WindowView("CharacterDetails") {
	_entry.setup(22, 18);
}

bool CharacterDetails::msgFocus(const FocusMessage &msg) {
	_state = ENTRY;
	_entry.reset();
	return View::msgFocus(msg);
}

void CharacterDetails::drawDetails(GfxSurface &s) {
	const Data::RosterEntry &e = _G(savegame).entry(_entry.decimal());

	s.writeString(Common::Point(15, 11), Common::String::format("Entry#%02d", _entry.decimal()));
	s.writeString(Common::Point(3, 13), Common::String("Name:") + e._name);
	s.writeString(Common::Point(3, 14), Common::String(" Sex:") +
		Data::SEX_NAMES[Data::lookupIndex(e._sex, Data::SEX_KEYS, Data::SEX_COUNT)]);
	s.writeString(Common::Point(3, 15), Common::String("Race:") +
		Data::RACE_NAMES[Data::lookupIndex(e._race, Data::RACE_KEYS, Data::RACE_COUNT)]);
	s.writeString(Common::Point(3, 16), Common::String("Type:") +
		Data::CLASS_NAMES[Data::lookupIndex(e._class, Data::CLASS_KEYS, Data::CLASS_COUNT)]);
	s.writeString(Common::Point(1, 17), Common::String("Status:") +
		Data::STATUS_NAMES[Data::lookupIndex(e._status, Data::STATUS_KEYS, Data::STATUS_COUNT)]);
	s.writeString(Common::Point(1, 19), Common::String("Weapon:") +
		Data::WEAPON_NAMES[MIN<int>(e._weaponIndex, Data::WEAPON_COUNT - 1)]);
	s.writeString(Common::Point(1, 20), Common::String("Armour:") +
		Data::ARMOUR_NAMES[MIN<int>(e._armourIndex, Data::ARMOUR_COUNT - 1)]);

	s.writeString(Common::Point(27, 13), Common::String::format("Strength:%02X", e._strength));
	s.writeString(Common::Point(26, 14), Common::String::format("Dexterity:%02X", e._dexterity));
	s.writeString(Common::Point(23, 15), Common::String::format("Intelligence:%02X", e._intelligence));
	s.writeString(Common::Point(29, 16), Common::String::format("Wisdom:%02X", e._wisdom));
	s.writeString(Common::Point(23, 18), Common::String::format("Hit points:%04X", e._hitPoints));
	s.writeString(Common::Point(23, 19), Common::String::format("Experience:%04X", e._experience));
	s.writeString(Common::Point(29, 20), Common::String::format("Food:%04X", e._food));
	s.writeString(Common::Point(29, 21), Common::String::format("Gold:%04X", e._gold));
}

void CharacterDetails::draw() {
	auto s = getSurface();
	clearWindow(s);

	if (_state == DETAILS) {
		drawDetails(s);
	} else {
		s.writeString(Common::Point(10, 15), "Look at a Character");
		s.writeString(Common::Point(16, 18), "Entry#");
		_entry.draw(s, _state == ENTRY);

		if (_state == MESSAGE)
			s.writeString(Common::Point(_messageCol, 21), _message);
	}

	if (_state != ENTRY)
		drawSpacePrompt(s);
}

bool CharacterDetails::msgKeypress(const KeypressMessage &msg) {
	if (_state == ENTRY) {
		if (_entry.handleKey(msg)) {
			int number = _entry.decimal();

			if (number < 1 || number > Data::ROSTER_COUNT) {
				_state = MESSAGE;
				_message = "(1-20 Only!)";
				_messageCol = 14;
			} else if (_G(savegame).entry(number).isEmpty()) {
				_state = MESSAGE;
				_message = "(No one there)";
				_messageCol = 13;
			} else {
				_state = DETAILS;
			}
		}

		redraw();
	} else if (isSpaceKey(msg)) {
		close();
	}

	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
