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

#include "ultima/ultima2/views/create_character.h"
#include "ultima/ultima2/ultima2.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

constexpr int COLOR_TEXT = 15; // white

constexpr int STARTING_POINTS = 90;
constexpr int MIN_ATTRIBUTE = 10;

static const char *raceName(Data::Race race) {
	switch (race) {
	case Data::RACE_HUMAN: return "HUMAN";
	case Data::RACE_ELF: return "ELF";
	case Data::RACE_DWARF: return "DWARF";
	case Data::RACE_HOBBIT: return "HOBBIT";
	default: return "";
	}
}

static const char *className(Data::CharClass charClass) {
	switch (charClass) {
	case Data::CLASS_FIGHTER: return "FIGHTER";
	case Data::CLASS_CLERIC: return "CLERIC";
	case Data::CLASS_WIZARD: return "WIZARD";
	case Data::CLASS_THIEF: return "THIEF";
	default: return "";
	}
}

CreateCharacter::CreateCharacter() : View("CreateCharacter"), _cursor("Cursor", this) {
}

bool CreateCharacter::msgFocus(const FocusMessage &msg) {
	reset();
	return View::msgFocus(msg);
}

void CreateCharacter::reset() {
	_state = STRENGTH;
	_pointsRemaining = STARTING_POINTS;
	_digitCount = 0;
	_name.clear();

	_G(savegame) = Data::Savegame();
	positionCursor();
	redraw();
}

void CreateCharacter::rejectEntry() {
	// TODO: play the original's triple-beep failure sound once sound
	// effects are implemented
	reset();
}

void CreateCharacter::enterStat(int value) {
	if (value < MIN_ATTRIBUTE || value > _pointsRemaining) {
		rejectEntry();
		return;
	}

	_pointsRemaining -= value;
	Data::Savegame &sg = _G(savegame);

	switch (_state) {
	case STRENGTH:
		sg._strength = value;
		_state = AGILITY;
		break;
	case AGILITY:
		sg._agility = value;
		_state = STAMINA;
		break;
	case STAMINA:
		sg._stamina = value;
		_state = CHARISMA;
		break;
	case CHARISMA:
		sg._charisma = value;
		_state = WISDOM;
		break;
	case WISDOM:
		sg._wisdom = value;
		_state = INTELLIGENCE;
		break;
	case INTELLIGENCE:
		sg._intelligence = value;
		_state = SEX;
		break;
	default:
		break;
	}

	_digitCount = 0;
	positionCursor();
	redraw();
}

void CreateCharacter::positionCursor() {
	switch (_state) {
	case STRENGTH: case AGILITY: case STAMINA:
	case CHARISMA: case WISDOM: case INTELLIGENCE:
		_cursor.setPosition(Common::Point(26 + _digitCount, 4 + (int)_state));
		break;
	case SEX:
		_cursor.setPosition(Common::Point(20, 11));
		break;
	case RACE:
		_cursor.setPosition(Common::Point(20, 12));
		break;
	case CLASS:
		_cursor.setPosition(Common::Point(20, 13));
		break;
	case NAME:
		_cursor.setPosition(Common::Point((int16)(20 + _name.size()), 14));
		break;
	case CONFIRM:
		_cursor.setPosition(Common::Point(29, 16));
		break;
	case DONE:
		_cursor.hide();
		return;
	}

	_cursor.show();
}

void CreateCharacter::draw() {
	auto s = getSurface();
	s.clear();
	s.setColor(COLOR_TEXT);

	if (_state == DONE) {
		drawDone(s);
		return;
	}

	Data::Savegame &sg = _G(savegame);

	s.writeString(Common::Point(10, 1), "PLAYER GENERATION :");

	s.writeString(Common::Point(5, 2), "POINTS LEFT TO DISTRIBUTE : ");
	s.reverseColor();
	s.writeString(Common::Point(33, 2), "%d", _pointsRemaining);
	s.reverseColor();

	s.writeString(Common::Point(11, 4), "STRENGTH.......");
	s.writeString(Common::Point(11, 5), "AGILITY........");
	s.writeString(Common::Point(11, 6), "STAMINA........");
	s.writeString(Common::Point(11, 7), "CHARISMA.......");
	s.writeString(Common::Point(11, 8), "WISDOM.........");
	s.writeString(Common::Point(11, 9), "INTELLIGENCE...");

	if (_state > STRENGTH) s.writeString(Common::Point(26, 4), "%d", sg._strength);
	if (_state > AGILITY) s.writeString(Common::Point(26, 5), "%d", sg._agility);
	if (_state > STAMINA) s.writeString(Common::Point(26, 6), "%d", sg._stamina);
	if (_state > CHARISMA) s.writeString(Common::Point(26, 7), "%d", sg._charisma);
	if (_state > WISDOM) s.writeString(Common::Point(26, 8), "%d", sg._wisdom);
	if (_state > INTELLIGENCE) s.writeString(Common::Point(26, 9), "%d", sg._intelligence);

	// The digits typed so far for whichever attribute is currently active
	if (_state <= INTELLIGENCE && _digitCount > 0) {
		Common::String digits;
		for (int i = 0; i < _digitCount; ++i)
			digits += (char)('0' + _digits[i]);
		s.writeString(Common::Point(26, 4 + (int)_state), digits);
	}

	s.writeString(Common::Point(16, 11), "M/F-");
	s.writeString(Common::Point(15, 12), "RACE-");
	s.writeString(Common::Point(15, 13), "TYPE-");
	s.writeString(Common::Point(15, 14), "NAME-");

	if (_state > SEX)
		s.writeString(Common::Point(20, 11), sg._sex == Data::SEX_MALE ? "MALE" : "FEMALE");
	if (_state > RACE)
		s.writeString(Common::Point(20, 12), raceName(sg._race));
	if (_state > CLASS)
		s.writeString(Common::Point(20, 13), className(sg._class));
	if (_state >= NAME)
		s.writeString(Common::Point(20, 14), _name);

	s.writeString(Common::Point(10, 16), "SATISFACTORY (Y/N)-");

	s.writeString(Common::Point(8, 18), "RACES:         TYPES:");
	s.writeString(Common::Point(9, 19), "1-HUMAN        1-FIGHTER");
	s.writeString(Common::Point(9, 20), "2-ELF          2-CLERIC");
	s.writeString(Common::Point(9, 21), "3-DWARF        3-WIZARD");
	s.writeString(Common::Point(9, 22), "4-HOBBIT       4-THIEF");
}

void CreateCharacter::drawDone(Shared::Gfx::GfxSurface &s) {
	Data::Savegame &sg = _G(savegame);

	s.writeString(Common::Point(10, 2), "CHARACTER CREATED");
	s.writeString(Common::Point(4, 4), "NAME: %s", sg._name);
	s.writeString(Common::Point(4, 5), "%s %s %s",
		sg._sex == Data::SEX_MALE ? "MALE" : "FEMALE", raceName(sg._race), className(sg._class));
	s.writeString(Common::Point(4, 7), "STR:%d AGL:%d STA:%d", sg._strength, sg._agility, sg._stamina);
	s.writeString(Common::Point(4, 8), "CHR:%d WIS:%d INT:%d", sg._charisma, sg._wisdom, sg._intelligence);
	s.writeString(Common::Point(4, 10), "HP:%d FOOD:%d GOLD:%d", sg._hp, sg._food, sg._gold);

	s.writeString(Common::Point(4, 13), "PRESS ANY KEY TO ENTER THE WORLD");
}

bool CreateCharacter::msgKeypress(const KeypressMessage &msg) {
	Data::Savegame &sg = _G(savegame);

	switch (_state) {
	case STRENGTH:
	case AGILITY:
	case STAMINA:
	case CHARISMA:
	case WISDOM:
	case INTELLIGENCE:
		if (msg.ascii < '0' || msg.ascii > '9')
			break;
		_digits[_digitCount++] = msg.ascii - '0';
		if (_digitCount == 2) {
			enterStat(_digits[0] * 10 + _digits[1]);
		} else {
			positionCursor();
			redraw();
		}
		break;

	case SEX:
		if (toupper(msg.ascii) == 'M') {
			sg._sex = Data::SEX_MALE;
			sg._strength += 5;
			_state = RACE;
			positionCursor();
			redraw();
		} else if (toupper(msg.ascii) == 'F') {
			sg._sex = Data::SEX_FEMALE;
			sg._charisma += 16;
			_state = RACE;
			positionCursor();
			redraw();
		}
		break;

	case RACE:
		if (msg.ascii >= '1' && msg.ascii <= '4') {
			sg._race = (Data::Race)(msg.ascii - '1');
			switch (sg._race) {
			case Data::RACE_HUMAN: sg._intelligence += 5; break;
			case Data::RACE_ELF: sg._agility += 5; break;
			case Data::RACE_DWARF: sg._strength += 5; break;
			case Data::RACE_HOBBIT: sg._wisdom += 16; break;
			default: break;
			}
			_state = CLASS;
			positionCursor();
			redraw();
		}
		break;

	case CLASS:
		if (msg.ascii >= '1' && msg.ascii <= '4') {
			sg._class = (Data::CharClass)(msg.ascii - '1');
			switch (sg._class) {
			case Data::CLASS_FIGHTER: sg._strength += 21; break;
			case Data::CLASS_CLERIC: sg._wisdom += 16; break;
			case Data::CLASS_WIZARD: sg._intelligence += 16; break;
			case Data::CLASS_THIEF: sg._agility += 16; break;
			default: break;
			}
			_state = NAME;
			positionCursor();
			redraw();
		}
		break;

	case NAME:
		if (msg.ascii == 8) { // backspace
			if (!_name.empty())
				_name.deleteLastChar();
			positionCursor();
			redraw();
		} else if (msg.ascii == 13) { // enter
			_state = CONFIRM;
			positionCursor();
			redraw();
		} else if (msg.ascii >= 32 && msg.ascii < 127 && (int)_name.size() < Data::MAX_NAME_LENGTH) {
			_name += (char)msg.ascii;
			positionCursor();
			redraw();
		}
		break;

	case CONFIRM:
		if (toupper(msg.ascii) == 'N') {
			reset();
		} else if (toupper(msg.ascii) == 'Y') {
			Common::strlcpy(sg._name, _name.c_str(), sizeof(sg._name));
			sg._hp = 400;
			sg._food = 400;
			sg._gold = 400;
			sg._mapX = 20;
			sg._mapY = 20;
			sg._mapEra = 2;
			sg._mapType = 0;
			_state = DONE;
			positionCursor();
			redraw();
		}
		break;

	case DONE:
		_G(map).load(sg._mapEra, sg._mapType);
		break;
	}

	return true;
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
