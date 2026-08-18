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

using namespace Data;

static const byte POS[][2] = {
	{ 26, 4 }, { 26, 5 }, { 26, 6 }, { 26, 7 }, { 26, 8 }, { 26, 9 },
	{ 20, 11 }, { 20, 12 }, { 20, 13 }, { 20, 14 }, { 29, 16 }
};

bool CreateCharacter::msgFocus(const FocusMessage &msg) {
	reset();
	return true;
}

void CreateCharacter::reset() {
	_cursorIndex = 0;
	_pointsRemaining = POINTS_REMAINING;
	_cursor.show(Point(POS[0][0], POS[0][1]), 2, true);
}

void CreateCharacter::done() {
	// Unlike the original that took you back to the title screen after
	// creating a character, since in ScummVM we allow multiple savegames,
	// take the player directly in-game
	g_engine->_game.startGame();
}

void CreateCharacter::draw() {
	const Player &p = g_engine->_player;
	auto s = getSurface();
	s.clear();

	s.writeString(Point(0, 0), "          PLAYER GENERATION :");
	s.writeString(Point(0, 2), "     POINTS LEFT TO DISTRIBUTE : ");

	// Display points remaining in reverse text
	s.reverseColor();
	s.writeString(Common::String::format("%.2d", _pointsRemaining));
	s.reverseColor();

	s.writeString(Point(0, 4), "           STRENGTH.......\n"
		"           AGILITY........\n"
		"           STAMINA........\n"
		"           CHARISMA.......\n"
		"           WISDOM.........\n"
		"           INTELLIGENCE...\n\n"
		"                M/F-\n"
		"               RACE-\n"
		"               TYPE-\n"
		"               NAME-\n\n"
		"          SATISFACTORY (Y/N)-\n\n"
		"        RACES:         TYPES:\n"
		"         1-HUMAN        1-FIGHTER\n"
		"         2-ELF          2-CLERIC\n"
		"         3-DWARF        3-WIZARD\n"
		"         4-HOBBIT       4-THIEF");

	// Attributes
	if (_cursorIndex > 0)
		s.writeString(Point(POS[0][0], POS[0][1]), Common::String::format("%.2d", p._strength));
	if (_cursorIndex > 1)
		s.writeString(Point(POS[1][0], POS[1][1]), Common::String::format("%.2d", p._agility));
	if (_cursorIndex > 2)
		s.writeString(Point(POS[2][0], POS[2][1]), Common::String::format("%.2d", p._stamina));
	if (_cursorIndex > 3)
		s.writeString(Point(POS[3][0], POS[3][1]), Common::String::format("%.2d", p._charisma));
	if (_cursorIndex > 4)
		s.writeString(Point(POS[4][0], POS[4][1]), Common::String::format("%.2d", p._wisdom));
	if (_cursorIndex > 5)
		s.writeString(Point(POS[5][0], POS[5][1]), Common::String::format("%.2d", p._intelligence));

	// Other fields
	if (_cursorIndex > 6)
		s.writeString(Point(POS[6][0], POS[6][1]), SEX[p._sex]);
	if (_cursorIndex > 7)
		s.writeString(Point(POS[7][0], POS[7][1]), RACE[p._race]);
	if (_cursorIndex > 8)
		s.writeString(Point(POS[8][0], POS[8][1]), CLASS[p._class]);
	if (_cursorIndex > 9)
		s.writeString(Point(POS[9][0], POS[9][1]), p._name);
}

bool CreateCharacter::msgGame(const GameMessage &msg) {
	if (msg._name != "INPUT")
		return false;

	Player &p = g_engine->_player;
	char key = msg._stringValue.empty() ? 0 : toupper(msg._stringValue.firstChar());
	int num = atoi(msg._stringValue.c_str());

	switch (_cursorIndex) {
	case 6:
		if (key == 'M') {
			p._sex = SEX_MALE;
			p._strength += 5;		// Men are stronger
			++_cursorIndex;
		} else if (key == 'F') {
			p._sex = SEX_FEMALE;
			p._charisma += 10;		// Women are prettier
			++_cursorIndex;
		}
		break;
	case 7:
		if (num >= 1 && num <= 4) {
			p._race = (Race)num;
			++_cursorIndex;

			switch (p._race) {
			case RACE_HUMAN:
				p._intelligence += 5;
				break;
			case RACE_ELF:
				p._agility += 5;
				break;
			case RACE_DWARF:
				p._strength += 5;
				break;
			case RACE_HOBBIT:
				p._wisdom += 10;
				break;
			}
		}
		break;
	case 8:
		if (num >= 1 && num <= 4) {
			p._class = (Class)num;
			++_cursorIndex;

			switch (p._class) {
			case CLASS_FIGHTER:
				p._strength += 15;
				break;
			case CLASS_CLERIC:
				p._wisdom += 10;
				break;
			case CLASS_WIZARD:
				p._intelligence += 10;
				break;
			case CLASS_THIEF:
				p._agility += 10;
				break;
			}
		}
		break;
	case 9:
		strcpy_s(p._name, msg._stringValue.c_str());
		++_cursorIndex;
		break;
	case 10:
		if (key == 'Y') {
			done();
		} else {
			reset();
		}
		return true;
	default:
		// Attributes
		switch (_cursorIndex) {
		case 0: p._strength = num; break;
		case 1: p._agility = num; break;
		case 2: p._stamina = num; break;
		case 3: p._charisma = num; break;
		case 4: p._wisdom = num; break;
		default: p._intelligence = num; break;
		}

		_pointsRemaining -= num;
		if (_pointsRemaining < 0) {
			reset();
		} else {
			++_cursorIndex;
		}
		break;
	}

	// Set the new cursor position
	if (_cursorIndex < 6)
		_cursor.show(Point(POS[_cursorIndex][0], POS[_cursorIndex][1]), 2, true);
	else if (_cursorIndex == 9)
		_cursor.show(Point(POS[_cursorIndex][0], POS[_cursorIndex][1]), MAX_NAME_LENGTH);
	else
		_cursor.show(Point(POS[_cursorIndex][0], POS[_cursorIndex][1]), 1);

	redraw();
	return true;
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
