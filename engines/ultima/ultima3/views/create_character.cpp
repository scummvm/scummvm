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

#include "common/util.h"
#include "ultima/ultima3/views/create_character.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {

constexpr int STARTING_POINTS = 50;
constexpr int ATTRIBUTE_MIN = 5;
constexpr int ATTRIBUTE_MAX = 25;
constexpr int FIELD_COL = 19;
constexpr uint16 STARTING_VALUE = 0x150;

static const char *const ATTRIBUTE_LABELS[4] = {
	"Strength........", "Dexterity.......", "Intelligence....", "Wisdom.........."
};
static const char *const YES_NO_WORDS[2] = { "Yes", "No" };

CreateCharacter::CreateCharacter() : WindowView("CreateCharacter") {
	_entry.setup(22, 18);
	_name.setup(Data::NAME_MAX, FIELD_COL, 13);
	_sex.setup(Data::SEX_KEYS, Data::SEX_NAMES, Data::SEX_COUNT, FIELD_COL, 14);
	_race.setup(Data::RACE_KEYS, Data::RACE_NAMES, Data::RACE_COUNT, FIELD_COL, 15);
	_class.setup(Data::CLASS_KEYS, Data::CLASS_NAMES, Data::CLASS_COUNT, FIELD_COL, 16);
	_confirm.setup("YN", YES_NO_WORDS, 2, FIELD_COL, 21);

	for (int i = 0; i < 4; ++i)
		_attributes[i].setup(27, 17 + i);
}

bool CreateCharacter::msgFocus(const FocusMessage &msg) {
	_state = ENTRY;
	_showForm = false;
	_entry.reset();
	_name.reset();
	_sex.reset();
	_race.reset();
	_class.reset();
	_confirm.reset();

	for (int i = 0; i < 4; ++i)
		_attributes[i].reset();

	return View::msgFocus(msg);
}

void CreateCharacter::drawForm(GfxSurface &s) {
	s.writeString(Common::Point(11, 11), Common::String::format("Entry#%02d", _number));
	s.writeString(Common::Point(20, 11), Common::String::format("Points:%02d", _points));
	s.writeString(Common::Point(14, 13), "Name:");
	s.writeString(Common::Point(14, 14), " Sex:");
	s.writeString(Common::Point(14, 15), "Race:");
	s.writeString(Common::Point(14, 16), "Type:");
	s.writeString(Common::Point(14, 21), "O.K.?");

	for (int i = 0; i < 4; ++i)
		s.writeString(Common::Point(11, 17 + i), ATTRIBUTE_LABELS[i]);

	_name.draw(s, _state == NAME);
	_sex.draw(s, _state == SEX);
	_race.draw(s, _state == RACE);
	_class.draw(s, _state == CLASS);
	_confirm.draw(s, _state == CONFIRM);

	for (int i = 0; i < 4; ++i)
		_attributes[i].draw(s, _state == ATTRIBUTE && _attribute == i);
}

void CreateCharacter::draw() {
	auto s = getSurface();
	clearWindow(s);

	if (_showForm) {
		drawForm(s);
	} else {
		s.writeString(Common::Point(17, 15), "Create");
		s.writeString(Common::Point(16, 18), "Entry#");
		_entry.draw(s, _state == ENTRY);

		if (_state == MESSAGE)
			s.writeString(Common::Point(14, 21), _message);
	}

	if (_state == MESSAGE || _state == FINISHED)
		drawSpacePrompt(s);
}

bool CreateCharacter::acceptAttribute(int value) {
	if (value < ATTRIBUTE_MIN || value > ATTRIBUTE_MAX)
		return false;

	// The points left have to cover the minimum for the attributes still to
	// come, with the last one only needing to fit within what's left
	int remaining = _points - value;
	switch (_attribute) {
	case 1:
		if (remaining < 10)
			return false;
		break;
	case 2:
		if (remaining < ATTRIBUTE_MIN)
			return false;
		break;
	case 3:
		if (remaining < 0)
			return false;
		break;
	default:
		break;
	}

	_values[_attribute] = value;
	_points = remaining;
	return true;
}

void CreateCharacter::createCharacter() {
	Data::RosterEntry &e = _G(savegame).entry(_number);
	e.clear();

	Common::strlcpy(e._name, _name.text().c_str(), sizeof(e._name));
	e._sex = _sex.key();
	e._race = _race.key();
	e._class = _class.key();
	e._strength = Data::toBcd(_values[0]);
	e._dexterity = Data::toBcd(_values[1]);
	e._intelligence = Data::toBcd(_values[2]);
	e._wisdom = Data::toBcd(_values[3]);
	e._status = Data::STATUS_GOOD;
	e._hitPoints = e._maxHitPoints = e._food = e._gold = STARTING_VALUE;
	e._weaponOwned[0] = 1;
	e._armourOwned[0] = 1;
}

bool CreateCharacter::msgKeypress(const KeypressMessage &msg) {
	switch (_state) {
	case ENTRY:
		if (_entry.handleKey(msg)) {
			int number = _entry.decimal();

			if (number < 1 || number > Data::ROSTER_COUNT) {
				_state = MESSAGE;
				_message = "(1-20 Only!)";
			} else if (!_G(savegame).entry(number).isEmpty()) {
				_state = MESSAGE;
				_message = "(Not Empty!)";
			} else {
				_number = number;
				_points = STARTING_POINTS;
				_showForm = true;
				_state = NAME;
			}
		}
		break;

	case NAME:
		if (_name.handleKey(msg))
			_state = _name.text().empty() ? FINISHED : SEX;
		break;

	case SEX:
		if (_sex.handleKey(msg))
			_state = RACE;
		break;

	case RACE:
		if (_race.handleKey(msg))
			_state = CLASS;
		break;

	case CLASS:
		if (_class.handleKey(msg)) {
			_attribute = 0;
			_state = ATTRIBUTE;
		}
		break;

	case ATTRIBUTE:
		if (_attributes[_attribute].handleKey(msg)) {
			if (acceptAttribute(_attributes[_attribute].decimal())) {
				if (++_attribute == 4)
					_state = CONFIRM;
			} else {
				_attributes[_attribute].reset();
			}
		}
		break;

	case CONFIRM:
		if (_confirm.handleKey(msg)) {
			if (_confirm.key() == 'Y')
				createCharacter();
			_state = FINISHED;
		}
		break;

	case MESSAGE:
	case FINISHED:
		if (isSpaceKey(msg))
			close();
		break;
	}

	redraw();
	return true;
}

} // namespace Views
} // namespace Ultima3
} // namespace Ultima
