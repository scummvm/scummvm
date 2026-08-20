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

#include "ultima/ultima1/views/create_character.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Data;

// Colors used
constexpr int COLOR_EDGE = 15; // white
constexpr int COLOR_TEXT = 11; // light cyan

// Pads a label out with dots to a fixed width, matching the original's
// dot-leader alignment for the attribute list
static Common::String padLabel(const Common::String &label, int width) {
	Common::String result = label;
	while ((int)result.size() < width)
		result += '.';
	return result;
}

CreateCharacter::CreateCharacter() : Dialog("CreateCharacter"),
		_cursor("Cursor", this), _nameInput("NameInput", this, _cursor) {
	// Hidden until race/sex/class/save selection needs it
	_cursor.hide();
	_nameInput.setColor(COLOR_TEXT);
}

bool CreateCharacter::msgFocus(const FocusMessage &msg) {
	reset();
	return View::msgFocus(msg);
}

void CreateCharacter::reset() {
	Player &p = g_engine->_player;
	p._strength = p._agility = p._stamina = p._charisma = p._wisdom = p._intelligence = 10;

	_pointsRemaining = STARTING_POINTS;
	_selectedAttribute = 0;
	_state = ATTRIBUTES;
	_cursor.hide();
	redraw();
}

int16 &CreateCharacter::attribute(int index) {
	Player &p = g_engine->_player;
	switch (index) {
	case 0: return p._strength;
	case 1: return p._agility;
	case 2: return p._stamina;
	case 3: return p._charisma;
	case 4: return p._wisdom;
	default: return p._intelligence;
	}
}

void CreateCharacter::draw() {
	auto s = getSurface();
	s.clear();
	drawFrame();

	drawRightArrow(Common::Point(8, 0));
	s.setColor(COLOR_EDGE);
	s.writeString(Common::Point(9, 0), " Character Generation ");
	drawLeftArrow(Common::Point(31, 0));

	drawAttributes(s);

	switch (_state) {
	case ATTRIBUTES:
		s.setColor(COLOR_TEXT);
		s.writeString(Common::Point(6, 4),
			Common::String::format("Points left to distribute: %d", _pointsRemaining));
		drawHelp(s);
		drawRightArrow(Common::Point(11, _selectedAttribute + 6));
		drawLeftArrow(Common::Point(30, _selectedAttribute + 6));
		break;

	case RACE:
		s.setColor(COLOR_TEXT);
		s.writeString(Common::Point(3, 17), "Select thy race: ");
		s.writeString(Common::Point(12, 19), "a) Human");
		s.writeString(Common::Point(12, 20), "b) Elf");
		s.writeString(Common::Point(12, 21), "c) Dwarf");
		s.writeString(Common::Point(12, 22), "d) Bobbit");
		break;

	case SEX:
		drawConfirmedRace(s);
		s.setColor(COLOR_TEXT);
		s.writeString(Common::Point(3, 17), "Select thy sex: ");
		s.writeString(Common::Point(12, 19), "a) Male");
		s.writeString(Common::Point(12, 20), "b) Female");
		break;

	case CLASS:
		drawConfirmedRace(s);
		drawConfirmedSex(s);
		s.setColor(COLOR_TEXT);
		s.writeString(Common::Point(3, 17), "Select thy class: ");
		s.writeString(Common::Point(12, 19), "a) Fighter");
		s.writeString(Common::Point(12, 20), "b) Cleric");
		s.writeString(Common::Point(12, 21), "c) Wizard");
		s.writeString(Common::Point(12, 22), "d) Thief");
		break;

	case NAME:
		drawConfirmedRace(s);
		drawConfirmedSex(s);
		drawConfirmedClass(s);
		s.setColor(COLOR_TEXT);
		s.writeString(Common::Point(3, 17), "Enter thy name: ");
		break;

	case SAVE:
		drawConfirmedRace(s);
		drawConfirmedSex(s);
		drawConfirmedClass(s);
		s.setColor(COLOR_TEXT);
		s.writeString(Common::Point(12, 4), g_engine->_player._name);
		s.writeString(Common::Point(3, 22), "Use this character? (Y-N) ");
		break;
	}
}

void CreateCharacter::drawAttributes(Shared::Gfx::GfxSurface &s) {
	const Player &p = g_engine->_player;
	s.setColor(COLOR_TEXT);

	s.writeString(Common::Point(12, 6), padLabel("Strength", 16) + Common::String::format("%2d", p._strength));
	s.writeString(Common::Point(12, 7), padLabel("Agility", 16) + Common::String::format("%2d", p._agility));
	s.writeString(Common::Point(12, 8), padLabel("Stamina", 16) + Common::String::format("%2d", p._stamina));
	s.writeString(Common::Point(12, 9), padLabel("Charisma", 16) + Common::String::format("%2d", p._charisma));
	s.writeString(Common::Point(12, 10), padLabel("Wisdom", 16) + Common::String::format("%2d", p._wisdom));
	s.writeString(Common::Point(12, 11), padLabel("Intelligence", 16) + Common::String::format("%2d", p._intelligence));
}

void CreateCharacter::drawHelp(Shared::Gfx::GfxSurface &s) {
	s.setColor(COLOR_TEXT);
	s.writeString(Common::Point(2, 16), "Move cursor with up and down arrows;");
	s.writeString(Common::Point(2, 17), "increase and decrease attributes");
	s.writeString(Common::Point(2, 18), "with left and right arrows.  Press");
	s.writeString(Common::Point(2, 19), "space bar when finished, or escape");
	s.writeString(Common::Point(2, 20), "to return to the main menu.");
}

void CreateCharacter::drawConfirmedRace(Shared::Gfx::GfxSurface &s) {
	s.setColor(COLOR_TEXT);
	s.writeString(Common::Point(14, 13),
		Common::String::format("Race: %s", RACE_NAMES[g_engine->_player._race]));
}

void CreateCharacter::drawConfirmedSex(Shared::Gfx::GfxSurface &s) {
	s.setColor(COLOR_TEXT);
	s.writeString(Common::Point(15, 14),
		Common::String::format("Sex: %s", SEX_NAMES[g_engine->_player._sex]));
}

void CreateCharacter::drawConfirmedClass(Shared::Gfx::GfxSurface &s) {
	s.setColor(COLOR_TEXT);
	s.writeString(Common::Point(13, 15),
		Common::String::format("Class: %s", CLASS_NAMES[g_engine->_player._class]));
}

bool CreateCharacter::msgKeypress(const KeypressMessage &msg) {
	switch (_state) {
	case ATTRIBUTES:
		return msgAttributesKey(msg);
	case RACE:
		return msgRaceKey(msg);
	case SEX:
		return msgSexKey(msg);
	case CLASS:
		return msgClassKey(msg);
	case NAME:
		// Let it fall through to the name text input child
		return View::msgKeypress(msg);
	case SAVE:
		return msgSaveKey(msg);
	}

	return false;
}

bool CreateCharacter::msgAttributesKey(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_UP:
		_selectedAttribute = (_selectedAttribute == 0) ? ATTRIBUTE_COUNT - 1 : _selectedAttribute - 1;
		redraw();
		return true;

	case Common::KEYCODE_DOWN:
		_selectedAttribute = (_selectedAttribute == ATTRIBUTE_COUNT - 1) ? 0 : _selectedAttribute + 1;
		redraw();
		return true;

	case Common::KEYCODE_LEFT:
		if (attribute(_selectedAttribute) > 10) {
			--attribute(_selectedAttribute);
			++_pointsRemaining;
			redraw();
		}
		return true;

	case Common::KEYCODE_RIGHT:
		if (_pointsRemaining > 0 && attribute(_selectedAttribute) < 25) {
			++attribute(_selectedAttribute);
			--_pointsRemaining;
			redraw();
		}
		return true;

	case Common::KEYCODE_SPACE:
		if (_pointsRemaining == 0) {
			_state = RACE;
			_cursor.setPosition(Common::Point(20, 17));
			_cursor.show();
			redraw();
		}
		return true;

	case Common::KEYCODE_ESCAPE:
		replaceView("MainMenu");
		return true;

	default:
		return false;
	}
}

bool CreateCharacter::msgRaceKey(const KeypressMessage &msg) {
	char key = toupper(msg.ascii);
	if (key < 'A' || key > 'D')
		return false;

	selectRace(key - 'A' + 1);
	return true;
}

bool CreateCharacter::msgSexKey(const KeypressMessage &msg) {
	char key = toupper(msg.ascii);
	if (key != 'A' && key != 'B')
		return false;

	selectSex(key == 'A' ? SEX_MALE : SEX_FEMALE);
	return true;
}

bool CreateCharacter::msgClassKey(const KeypressMessage &msg) {
	char key = toupper(msg.ascii);
	if (key < 'A' || key > 'D')
		return false;

	selectClass(key - 'A' + 1);
	return true;
}

bool CreateCharacter::msgSaveKey(const KeypressMessage &msg) {
	char key = toupper(msg.ascii);
	if (key == 'Y') {
		confirmAndSave();
		return true;
	} else if (key == 'N') {
		reset();
		return true;
	}

	return false;
}

void CreateCharacter::selectRace(int race) {
	Player &p = g_engine->_player;
	p._race = race;

	switch (race) {
	case RACE_HUMAN:
		p._intelligence += 5;
		break;
	case RACE_ELF:
		p._agility += 5;
		break;
	case RACE_DWARF:
		p._strength += 5;
		break;
	case RACE_BOBBIT:
		p._wisdom += 10;
		p._strength -= 5;
		break;
	default:
		break;
	}

	_state = SEX;
	_cursor.setPosition(Common::Point(19, 17));
	redraw();
}

void CreateCharacter::selectSex(int sex) {
	g_engine->_player._sex = sex;

	_state = CLASS;
	_cursor.setPosition(Common::Point(21, 17));
	redraw();
}

void CreateCharacter::selectClass(int charClass) {
	Player &p = g_engine->_player;
	p._class = charClass;

	switch (charClass) {
	case CLASS_FIGHTER:
		p._strength += 10;
		p._agility += 10;
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
	default:
		break;
	}

	_state = NAME;
	_nameInput.setPosition(Common::Point(19, 17), MAX_NAME_LENGTH);
	redraw();
}

bool CreateCharacter::msgGame(const GameMessage &msg) {
	if (_state == NAME && msg._name == "INPUT" && !msg._stringValue.empty()) {
		strcpy_s(g_engine->_player._name, msg._stringValue.c_str());

		_nameInput.hide();
		_state = SAVE;
		_cursor.setPosition(Common::Point(29, 22));
		_cursor.show();
		redraw();
		return true;
	}

	return false;
}

void CreateCharacter::confirmAndSave() {
	Player &p = g_engine->_player;

	p._randomSeed = getRandomNumber(0x7fff) + p._wisdom + p._class;
	p._hits = 150;
	p._coins = 100;
	p._experience = 0;
	p._food = 200;
	p._equippedWeapon = WEAPON_DAGGER;
	p._equippedSpell = SPELL_NONE;
	p._equippedArmor = ARMOR_LEATHER;
	p._transportType = TRANSPORT_FOOT;
	p._position = Common::Point(49, 40);
	p._soundOn = true;

	for (int i = 0; i < QUEST_COUNT; ++i)
		p._quests[i] = 0;
	p._redGems = p._greenGems = p._blueGem = p._whiteGem = 0;

	for (int i = 0; i < ARMOR_COUNT; ++i)
		p._armor[i] = 0;
	p._armor[0] = -1;
	p._armor[ARMOR_LEATHER] = 1;

	for (int i = 0; i < WEAPON_COUNT; ++i)
		p._weapons[i] = 0;
	p._weapons[0] = -1;
	p._weapons[WEAPON_DAGGER] = 2;

	for (int i = 0; i < SPELL_COUNT; ++i)
		p._spells[i] = 0;
	p._spells[0] = -1;

	for (int i = 0; i < TRANSPORT_COUNT; ++i)
		p._transports[i] = 0;
	p._transports[0] = -1;

	p._enemyVessels = 0;
	p._signMarker = -1;
	p._moveCount = 0;

	_cursor.hide();

	g_engine->_game.startGame();
}

} // namespace Views
} // namespace Ultima1
} // namespace Ultima
