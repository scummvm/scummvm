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

#include "mm/mm1/views_enh/create_characters.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define RIGHT_X 220

void CreateCharacters::NewCharacter::clear() {
	Common::fill(_attribs1, _attribs1 + 7, 0);
	Common::fill(_attribs2, _attribs2 + 7, 0);
	_class = KNIGHT;
	_race = HUMAN;
	_alignment = GOOD;
	_sex = MALE;
	_name = "";

	Common::fill(_classesAllowed, _classesAllowed + 7, 0);
}

void CreateCharacters::NewCharacter::reroll() {
	clear();

	// Generate attributes
	for (int attrib = INTELLECT; attrib <= LUCK; ++attrib)
		_attribs1[attrib] = g_engine->getRandomNumber(4, 17);
	Common::copy(_attribs1, _attribs1 + 7, _attribs2);

	// Select which classes are available
	_classesAllowed[KNIGHT] = _attribs1[MIGHT] >= 12;
	_classesAllowed[PALADIN] = _attribs1[MIGHT] >= 12 && _attribs1[PERSONALITY] >= 12 &&
		_attribs1[ENDURANCE] >= 12;
	_classesAllowed[ARCHER] = _attribs1[INTELLECT] >= 12 && _attribs1[ACCURACY] >= 12;
	_classesAllowed[CLERIC] = _attribs1[PERSONALITY] >= 12;
	_classesAllowed[SORCERER] = _attribs1[INTELLECT] >= 12;
	_classesAllowed[ROBBER] = true;
}

void CreateCharacters::NewCharacter::loadPortrait() {
	Common::String cname = Common::String::format("char%02d.fac",
		_portrait * 2 + (_sex == MALE ? 0 : 1) + 1);
	_portraits.load(cname);
}

void CreateCharacters::NewCharacter::save() {
	uint i = 0;
	while (i < ROSTER_COUNT && g_globals->_roster._towns[i])
		++i;

	g_globals->_roster._towns[i] = Maps::SORPIGAL;
	g_globals->_currCharacter = &g_globals->_roster[i];
	Character &re = *g_globals->_currCharacter;
	re.clear();

	Common::strcpy_s(re._name, _name.c_str());
	re._sex = _sex;
	re._alignment = re._alignmentInitial = _alignment;
	re._race = _race;
	re._class = _class;
	re._intelligence = _attribs1[INTELLECT];
	re._might = _attribs1[MIGHT];
	re._personality = _attribs1[PERSONALITY];
	re._endurance = _attribs1[ENDURANCE];
	re._speed = _attribs1[SPEED];
	re._accuracy = _attribs1[ACCURACY];
	re._luck = _attribs1[LUCK];

	switch (_class) {
	case KNIGHT:
		setHP(12);
		break;
	case PALADIN:
	case ARCHER:
		setHP(10);
		break;
	case CLERIC:
		setHP(8);
		setSP(_attribs1[PERSONALITY]);
		break;
	case SORCERER:
		setHP(6);
		setSP(_attribs1[INTELLECT]);
		break;
	case ROBBER:
		setHP(8);
		re._trapCtr = 50;
		break;
	default:
		break;
	}

	switch (_race) {
	case HUMAN:
		re._resistances._s._fear = 70;
		re._resistances._s._psychic = 25;
		break;
	case ELF:
		re._resistances._s._fear = 70;
		break;
	case DWARF:
		re._resistances._s._poison = 25;
		break;
	case GNOME:
		re._resistances._s._magic = 20;
		break;
	case HALF_ORC:
		re._resistances._s._psychic = 50;
		break;
	}

	re._food = 10;
	re._backpack[0]._id = 1;
	const int ALIGNMENT_VALS[3] = { 0, 0x10, 0x20 };
	re._alignmentCtr = ALIGNMENT_VALS[re._alignmentInitial];

	g_globals->_roster.save();
}

void CreateCharacters::NewCharacter::setHP(int hp) {
	Character &re = *g_globals->_currCharacter;

	if (_attribs1[ENDURANCE] >= 19)
		hp += 4;
	else if (_attribs1[ENDURANCE] >= 17)
		hp += 3;
	else if (_attribs1[ENDURANCE] >= 15)
		hp += 2;
	else if (_attribs1[ENDURANCE] >= 13)
		hp += 1;
	else if (_attribs1[ENDURANCE] < 5)
		hp -= 2;
	else if (_attribs1[ENDURANCE] < 8)
		hp -= 1;

	re._hpCurrent = re._hp = re._hpMax = hp;

	int ac = 0;
	if (_attribs1[SPEED] >= 19)
		ac = 4;
	else if (_attribs1[SPEED] >= 17)
		ac = 3;
	else if (_attribs1[SPEED] >= 15)
		ac = 2;
	if (_attribs1[SPEED] >= 13)
		ac = 1;

	re._ac = ac;
}

void CreateCharacters::NewCharacter::setSP(int amount) {
	Character &re = *g_globals->_currCharacter;

	int level = 0;
	if (amount >= 19)
		level = 4;
	else if (amount >= 17)
		level = 3;
	else if (amount >= 15)
		level = 2;
	else if (amount >= 13)
		level = 1;

	re._sp = level + 3;
	re._spellLevel = 1;
}

/*------------------------------------------------------------------------*/

CreateCharacters::CreateCharacters() : ScrollView("CreateCharacters") {
	_icons.load("create.icn");

	addButton(&_icons, Common::Point(120, 172), 4, KEYBIND_ESCAPE, true);
	addButton(&_icons, Common::Point(40, 120), 0, Common::KEYCODE_r);
	addButton(&_icons, Common::Point(190, 110), 6, Common::KEYCODE_UP);
	addButton(&_icons, Common::Point(190, 130), 8, Common::KEYCODE_DOWN);
	addButton(&_icons, Common::Point(220, 120), 2, KEYBIND_SELECT);

	setButtonEnabled(2, false);
	setButtonEnabled(3, false);
	setButtonEnabled(4, false);
}

void CreateCharacters::draw() {
	ScrollView::draw();
	printAttributes();

	if ((int)_state >= SELECT_NAME) {
		Graphics::ManagedSurface s = getSurface();
		_newChar._portraits.draw(&s, 0, Common::Point(10, 10));
	}

	writeString(135, 174, STRING["enhdialogs.misc.go_back"]);
	writeString(70, 125, STRING["enhdialogs.create_characters.roll"]);

	switch (_state) {
	case SELECT_CLASS:
		printClasses();
		if (g_globals->_roster.full())
			writeLine(9, STRING["dialogs.create_characters.full"], ALIGN_MIDDLE, 190);
		break;

	case SELECT_RACE:
		printRaces();
		break;

	case SELECT_ALIGNMENT:
		printAlignments();
		break;

	case SELECT_SEX:
		printSexes();
		break;

	case SELECT_PORTRAIT:
		printPortraits();
		break;

	case SELECT_NAME:
		printSelectName();
		break;

	case SAVE_PROMPT:
		printSummary();
		break;

	default:
		break;
	}
}

void CreateCharacters::printAttributes() {
	writeLine(0, STRING["dialogs.create_characters.title"], ALIGN_MIDDLE);

	writeLine(5, STRING["enhdialogs.create_characters.intellect"], ALIGN_RIGHT, 90);
	writeLine(6, STRING["enhdialogs.create_characters.might"], ALIGN_RIGHT, 90);
	writeLine(7, STRING["enhdialogs.create_characters.personality"], ALIGN_RIGHT, 90);
	writeLine(8, STRING["enhdialogs.create_characters.endurance"], ALIGN_RIGHT, 90);
	writeLine(9, STRING["enhdialogs.create_characters.speed"], ALIGN_RIGHT, 90);
	writeLine(10, STRING["enhdialogs.create_characters.accuracy"], ALIGN_RIGHT, 90);
	writeLine(11, STRING["enhdialogs.create_characters.luck"], ALIGN_RIGHT, 90);

	for (int i = 0; i < 7; ++i, _textPos.y += 2) {
		writeLine(5 + i,
			Common::String::format("%u", _newChar._attribs1[i]),
			ALIGN_RIGHT, 110);
	}
}

void CreateCharacters::addSelection(int yStart, int num) {
	Common::Rect r(170, 0, 320, 9);
	r.translate(0, (yStart + num) * 9);

	addButton(r, Common::KeyState((Common::KeyCode)(Common::KEYCODE_0 + num), '0' + num));
}

void CreateCharacters::printClasses() {
	for (int classNum = KNIGHT; classNum <= SORCERER; ++classNum) {
		setTextColor(_newChar._classesAllowed[classNum] ? 0 : 1);
		writeLine(4 + classNum, Common::String::format("%d) %s",
			classNum,
			STRING[Common::String::format("stats.classes.%d", classNum)].c_str()
		), ALIGN_LEFT, 170);

		if (_newChar._classesAllowed[classNum])
			addSelection(4, classNum);
	}

	setTextColor(0);
	writeLine(10, Common::String::format("6) %s", STRING["stats.classes.6"].c_str()),
		ALIGN_LEFT, 170);
	addSelection(4, ROBBER);

	writeLine(13, STRING["dialogs.create_characters.select_class"], ALIGN_MIDDLE, RIGHT_X);
	writeLine(14, "(1-6)", ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printRaces() {
	writeLine(5, STRING["enhdialogs.create_characters.class"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);

	for (int i = 1; i <= 5; ++i) {
		writeLine(6 + i, Common::String::format("%d) %s", i,
			STRING[Common::String::format("stats.races.%d", i)].c_str()),
			ALIGN_LEFT, 170);
		addSelection(6, i);
	}

	writeLine(13, STRING["dialogs.create_characters.select_race"], ALIGN_MIDDLE, RIGHT_X);
	writeLine(14, "(1-5)", ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printAlignments() {
	writeLine(5, STRING["enhdialogs.create_characters.class"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);
	writeLine(6, STRING["enhdialogs.create_characters.race"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.races.%d", _newChar._race)]);

	for (int i = 1; i <= 3; ++i) {
		writeLine(7 + i, Common::String::format("%d) %s", i,
			STRING[Common::String::format("stats.alignments.%d", i)].c_str()),
			ALIGN_LEFT, 170);
		addSelection(7, i);
	}

	writeLine(13, STRING["dialogs.create_characters.select_alignment"], ALIGN_MIDDLE, RIGHT_X);
	writeLine(14, "(1-3)", ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printSexes() {
	writeLine(5, STRING["enhdialogs.create_characters.class"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);
	writeLine(6, STRING["enhdialogs.create_characters.race"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.races.%d", _newChar._race)]);
	writeLine(7, STRING["enhdialogs.create_characters.alignment"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.alignments.%d", _newChar._alignment)]);

	writeLine(9, "1) ", ALIGN_LEFT, 170);
	writeString(STRING["stats.sex.1"]);
	addSelection(8, 1);
	writeLine(10, "2) ", ALIGN_LEFT, 170);
	writeString(STRING["stats.sex.2"]);
	addSelection(8, 2);

	writeLine(14, STRING["dialogs.create_characters.select_sex"], ALIGN_MIDDLE, RIGHT_X);
	writeLine(15, "(1-2)", ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printSelections() {
	writeLine(5, STRING["enhdialogs.create_characters.class"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);
	writeLine(6, STRING["enhdialogs.create_characters.race"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.races.%d", _newChar._race)]);
	writeLine(7, STRING["enhdialogs.create_characters.alignment"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.alignments.%d", _newChar._alignment)]);
	writeLine(8, STRING["enhdialogs.create_characters.sex"], ALIGN_RIGHT, RIGHT_X);
	writeString(STRING[Common::String::format("stats.sex.%d", _newChar._sex)]);
}

void CreateCharacters::printPortraits() {
	printSelections();
	writeLine(10, STRING["enhdialogs.create_characters.select_portrait"], ALIGN_MIDDLE, RIGHT_X);

	Graphics::ManagedSurface s = getSurface();
	_newChar._portraits.draw(&s, 0, Common::Point(160, 120));

	writeString(250, 126, STRING["enhdialogs.create_characters.select"]);
}

void CreateCharacters::printSelectName() {
	printSelections();
	writeLine(10, STRING["enhdialogs.create_characters.enter_name"], ALIGN_MIDDLE, RIGHT_X);
}

void CreateCharacters::printSummary() {
	printSelections();
	writeLine(9, STRING["enhdialogs.create_characters.name"], ALIGN_RIGHT, RIGHT_X);
	writeString(_newChar._name);

	writeLine(12, STRING["dialogs.create_characters.save_character"], ALIGN_MIDDLE, RIGHT_X);
	writeLine(14, "(Y/N)?", ALIGN_MIDDLE, RIGHT_X);
}

bool CreateCharacters::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_r && _state != SELECT_NAME) {
		setState(SELECT_CLASS);
		_newChar.reroll();
		redraw();
		return true;
	}

	switch (_state) {
	case SELECT_CLASS:
		if (msg.keycode >= Common::KEYCODE_1 &&
				msg.keycode <= Common::KEYCODE_6) {
			if (_newChar._classesAllowed[msg.keycode - Common::KEYCODE_0] &&
					!g_globals->_roster.full()) {
				// Selected a valid class
				_newChar._class = (CharacterClass)(msg.keycode - Common::KEYCODE_0);
				setState(SELECT_RACE);
				redraw();
			}
		}
		break;

	case SELECT_RACE:
		if (msg.keycode >= Common::KEYCODE_1 &&
				msg.keycode <= Common::KEYCODE_5) {
			// Selected a race
			_newChar._race = (Race)(msg.keycode - Common::KEYCODE_0);

			switch (_newChar._race) {
			case ELF:
				_newChar._attribs1[INTELLECT]++;
				_newChar._attribs1[ACCURACY]++;
				_newChar._attribs1[MIGHT]--;
				_newChar._attribs1[ENDURANCE]--;
				break;
			case DWARF:
				_newChar._attribs1[ENDURANCE]++;
				_newChar._attribs1[LUCK]++;
				_newChar._attribs1[INTELLECT]--;
				_newChar._attribs1[SPEED]--;
				break;
			case GNOME:
				_newChar._attribs1[LUCK] += 2;
				_newChar._attribs1[SPEED]--;
				_newChar._attribs1[ACCURACY]--;
				break;
			case HALF_ORC:
				_newChar._attribs1[MIGHT]++;
				_newChar._attribs1[ENDURANCE]++;
				_newChar._attribs1[INTELLECT]--;
				_newChar._attribs1[PERSONALITY]--;
				_newChar._attribs1[LUCK]--;
				break;
			default:
				break;
			}

			setState(SELECT_ALIGNMENT);
			redraw();
		}
		break;

	case SELECT_ALIGNMENT:
		if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_3) {
			// Selected a valid alignment
			_newChar._alignment = (Alignment)(msg.keycode - Common::KEYCODE_0);
			setState(SELECT_SEX);
			redraw();
		}
		break;

	case SELECT_SEX:
		if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_2) {
			// Selected a valid sex
			_newChar._sex = (Sex)(msg.keycode - Common::KEYCODE_0);
			_newChar.loadPortrait();
			setState(SELECT_PORTRAIT);
			redraw();
		}
		break;

	case SELECT_PORTRAIT:
		switch (msg.keycode) {
		case Common::KEYCODE_UP:
			_newChar._portrait = (_newChar._portrait == 0) ?
				NUM_PORTRAITS - 1 : _newChar._portrait - 1;
			_newChar.loadPortrait();
			redraw();
			break;
		case Common::KEYCODE_DOWN:
			_newChar._portrait = (_newChar._portrait + 1) % NUM_PORTRAITS;
			_newChar.loadPortrait();
			redraw();
			break;
		case Common::KEYCODE_s:
			msgAction(ActionMessage(KEYBIND_SELECT));
			break;
		default:
			break;
		}

		return true;

	case SAVE_PROMPT:
		if (msg.keycode == Common::KEYCODE_y)
			_newChar.save();

		setState(SELECT_CLASS);
		redraw();
		break;
	}

	return true;
}

bool CreateCharacters::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		if (_state == SELECT_CLASS) {
			close();
		} else {
			setState(SELECT_CLASS);
			_newChar.reroll();
			redraw();
		}
		return true;

	case KEYBIND_SELECT:
		switch (_state) {
		case SELECT_CLASS:
			// Re-roll attributes
			_newChar.reroll();
			redraw();
			break;
		case SELECT_PORTRAIT:
			setState(SELECT_NAME);
			break;
		case SAVE_PROMPT:
			_newChar.save();

			setState(SELECT_CLASS);
			_newChar.reroll();
			redraw();
			break;
		default:
			break;
		}
		return true;

	default:
		break;
	}

	return false;
}

void CreateCharacters::setState(State state) {
	_state = state;

	setButtonEnabled(2, _state == SELECT_PORTRAIT);
	setButtonEnabled(3, _state == SELECT_PORTRAIT);
	setButtonEnabled(4, _state == SELECT_PORTRAIT);
	removeButtons(5, -1);

	if (_state == SELECT_CLASS)
		_newChar.reroll();

	if (_state == SELECT_NAME) {
		draw();
		_textEntry.display(160, 110, 15, false,
			[]() {
				CreateCharacters *view = static_cast<CreateCharacters *>(
					g_events->focusedView());
				view->setState(SELECT_CLASS);
			},
			[](const Common::String &name) {
				CreateCharacters *view = static_cast<CreateCharacters *>(
					g_events->focusedView());

				view->_newChar._name = name;
				view->setState(SAVE_PROMPT);
			}
		);
	} else {
		redraw();
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
