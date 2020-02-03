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

#include "ultima/ultima1/u1gfx/view_char_gen.h"
#include "ultima/ultima1/u1gfx/drawing_support.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/u1gfx/text_cursor.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/gfx/text_input.h"
#include "ultima/shared/early/font_resources.h"
#include "ultima/shared/early/ultima_early.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

BEGIN_MESSAGE_MAP(ViewCharacterGeneration, Shared::Gfx::VisualItem)
	ON_MESSAGE(KeypressMsg)
	ON_MESSAGE(ShowMsg)
	ON_MESSAGE(TextInputMsg)
END_MESSAGE_MAP()

ViewCharacterGeneration::ViewCharacterGeneration(Shared::TreeItem *parent) :
		Shared::Gfx::VisualItem("CharGen", Rect(0, 0, 320, 200), parent) {
	_textInput = new Shared::Gfx::TextInput(getGame());
}

ViewCharacterGeneration::~ViewCharacterGeneration() {
	delete _textInput;
}

void ViewCharacterGeneration::setMode(uint flags) {
	_flags = flags;
	setDirty();

	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	Shared::Gfx::TextCursor *textCursor = game->_textCursor;
	textCursor->setVisible(false);

	if (flags & FLAG_FRAME) {
		// Set up character and attributes pointers
		_character = *game->_party;
		_attributes[0] = &_character->_strength;
		_attributes[1] = &_character->_agility;
		_attributes[2] = &_character->_stamina;
		_attributes[3] = &_character->_charisma;
		_attributes[4] = &_character->_wisdom;
		_attributes[5] = &_character->_intelligence;

		// Set character to default
		_pointsRemaining = 30;
		_selectedAttribute = 0;
		_character->_strength = 10;
		_character->_agility = 10;
		_character->_stamina = 10;
		_character->_charisma = 10;
		_character->_wisdom = 10;
		_character->_intelligence = 10;
	} else if (_flags & FLAG_RACE) {
		textCursor->setPosition(TextPoint(20, 17));
		textCursor->setVisible(true);
	} else if (_flags & FLAG_SEX) {
		textCursor->setPosition(TextPoint(19, 17));
		textCursor->setVisible(true);
	} else if (_flags & FLAG_CLASS) {
		textCursor->setPosition(TextPoint(21, 17));
		textCursor->setVisible(true);
	} else if (_flags & FLAG_NAME) {
		_textInput->show(TextPoint(19, 17), false, 14, game->_textColor);
	} else if (_flags & FLAG_SAVE) {
		textCursor->setPosition(TextPoint(30, 22));
		textCursor->setVisible(true);
	}
}

void ViewCharacterGeneration::draw() {
	Shared::Gfx::VisualItem::draw();
	Shared::Gfx::VisualSurface s = getSurface();

	if (_flags & FLAG_FRAME)
		drawFrame(s);
	if (_flags & FLAG_POINTS)
		drawPointsRemaining(s);
	if (_flags & FLAG_ATTRIBUTES)
		drawAttributes(s);
	if (_flags & FLAG_ATTR_POINTERS)
		drawAttributePointers(s);
	if (_flags & FLAG_HELP)
		drawHelp(s);
	if (_flags & FLAG_RACE)
		drawRace(s);
	if (_flags & FLAG_SEX)
		drawSex(s);
	if (_flags & FLAG_CLASS)
		drawClass(s);
	if (_flags & FLAG_NAME)
		drawName(s);
	if (_flags & FLAG_SAVE)
		drawSave(s);
}

void ViewCharacterGeneration::drawFrame(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());

	// Clear the view and draw the outer frame
	s.clear();
	DrawingSupport ds(s);
	ds.drawFrame();

	// Draw the header on the top
	ds.drawRightArrow(TextPoint(8, 0));
	s.writeString(game->_res->CHAR_GEN_TEXT[0], TextPoint(9, 0), game->_edgeColor);
	ds.drawLeftArrow(TextPoint(31, 0));
}

void ViewCharacterGeneration::drawPointsRemaining(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());

	s.writeString(Common::String::format(game->_res->CHAR_GEN_TEXT[1], _pointsRemaining), TextPoint(6, 4));
}

void ViewCharacterGeneration::drawAttributes(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	s.writeString(Common::String::format(game->_res->CHAR_GEN_TEXT[2],
		_character->_strength, _character->_agility, _character->_stamina,
		_character->_charisma, _character->_wisdom, _character->_intelligence),
		TextPoint(12, 6));
}

void ViewCharacterGeneration::drawAttributePointers(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	DrawingSupport ds(s);

	s.fillRect(Common::Rect(11 * 8, 6 * 8, 12 * 8, 12 * 8), game->_bgColor);
	s.fillRect(Common::Rect(30 * 8, 6 * 8, 31 * 8, 12 * 8), game->_bgColor);
	if (_selectedAttribute != -1) {
		ds.drawRightArrow(TextPoint(11, _selectedAttribute + 6));
		ds.drawLeftArrow(TextPoint(30, _selectedAttribute + 6));
	}
}

void ViewCharacterGeneration::drawHelp(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	s.writeString(game->_res->CHAR_GEN_TEXT[3], TextPoint(2, 16));
}

void ViewCharacterGeneration::drawRace(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	s.fillRect(TextRect(6, 2, 37, 4), game->_bgColor);
	s.fillRect(TextRect(2, 16, 37, 21), game->_bgColor);

	s.writeString(game->_res->CHAR_GEN_TEXT[6], TextPoint(3, 17));
	s.writeString(Common::String::format(game->_res->CHAR_GEN_TEXT[4], 
		game->_res->RACE_NAMES[0], game->_res->RACE_NAMES[1],
		game->_res->RACE_NAMES[2], game->_res->RACE_NAMES[3]),
		TextPoint(12, 19));
}

void ViewCharacterGeneration::drawSex(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());

	s.writeString(game->_res->CHAR_GEN_TEXT[9], TextPoint(14, 13));
	s.writeString(game->_res->RACE_NAMES[_character->_race]);

	s.fillRect(Rect(14, 128, 302, 184), game->_bgColor);
	s.writeString(game->_res->CHAR_GEN_TEXT[7], TextPoint(3, 17));
	s.writeString(Common::String::format(game->_res->CHAR_GEN_TEXT[5],
		game->_res->SEX_NAMES[0], game->_res->SEX_NAMES[1]), TextPoint(12, 19));
}

void ViewCharacterGeneration::drawClass(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());

	s.writeString(game->_res->CHAR_GEN_TEXT[10], TextPoint(15, 14));
	s.writeString(game->_res->SEX_NAMES[_character->_sex]);

	s.fillRect(Rect(14, 128, 302, 184), game->_bgColor);
	s.writeString(game->_res->CHAR_GEN_TEXT[8], TextPoint(3, 17));
	s.writeString(Common::String::format(game->_res->CHAR_GEN_TEXT[4],
		game->_res->CLASS_NAMES[0], game->_res->CLASS_NAMES[1],
		game->_res->CLASS_NAMES[2], game->_res->CLASS_NAMES[3]),
		TextPoint(12, 19));
}

void ViewCharacterGeneration::drawName(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());

	s.writeString(game->_res->CHAR_GEN_TEXT[11], TextPoint(13, 15));
	s.writeString(game->_res->CLASS_NAMES[_character->_class]);

	s.fillRect(Rect(14, 128, 302, 184), game->_bgColor);
	s.writeString(game->_res->CHAR_GEN_TEXT[12], TextPoint(3, 17));
}

void ViewCharacterGeneration::drawSave(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	s.fillRect(Rect(14, 128, 302, 184), game->_bgColor);
	s.writeString(_character->_name, TextPoint(12, 4));
	s.writeString(game->_res->CHAR_GEN_TEXT[13], TextPoint(3, 22));
}

void ViewCharacterGeneration::setRace(int raceNum) {
	_character->_race = raceNum;

	switch (raceNum) {
	case 0:
		_character->_intelligence += 5;
		break;
	case 1:
		_character->_agility += 5;
		break;
	case 2:
		_character->_strength += 5;
		break;
	case 3:
		_character->_wisdom += 10;
		_character->_strength -= 5;
		break;
	default:
		break;
	}

	setMode(FLAG_SEX | FLAG_ATTRIBUTES);
}

void ViewCharacterGeneration::setSex(int sexNum) {
	_character->_sex = (Shared::Sex)sexNum;
	setMode(FLAG_CLASS);
}

void ViewCharacterGeneration::setClass(int classNum) {
	_character->_class = classNum;

	switch (classNum) {
	case 0:
		_character->_strength += 10;
		_character->_agility += 10;
		break;
	case 1:
		_character->_wisdom += 10;
		break;
	case 2:
		_character->_intelligence += 10;
		break;
	case 3:
		_character->_agility += 10;
		break;
	default:
		break;
	}

	setMode(FLAG_NAME | FLAG_ATTRIBUTES);
}

bool ViewCharacterGeneration::ShowMsg(CShowMsg &msg) {
	Shared::Gfx::VisualItem::ShowMsg(msg);
	setMode(FLAG_INITIAL);
	return true;
}

bool ViewCharacterGeneration::HideMsg(CHideMsg &msg) {
	Shared::Gfx::VisualItem::HideMsg(msg);
	getGame()->_textCursor->setVisible(false);
	return true;
}

bool ViewCharacterGeneration::KeypressMsg(CKeypressMsg &msg) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());

	if (_flags & FLAG_RACE) {
		if (msg._keyState.keycode >= Common::KEYCODE_a && msg._keyState.keycode <= Common::KEYCODE_d)
			setRace(msg._keyState.keycode - Common::KEYCODE_a);
	} else if (_flags & FLAG_SEX) {
		if (msg._keyState.keycode >= Common::KEYCODE_a && msg._keyState.keycode <= Common::KEYCODE_b)
			setSex(msg._keyState.keycode - Common::KEYCODE_a);
	} else if (_flags & FLAG_CLASS) {
		if (msg._keyState.keycode >= Common::KEYCODE_a && msg._keyState.keycode <= Common::KEYCODE_d)
			setClass(msg._keyState.keycode - Common::KEYCODE_a);
	} else if (_flags & FLAG_NAME) {
		// Shouldn't reach here, since during name entry, keypresses go to text input
	} else if (_flags & FLAG_SAVE) {
		if (msg._keyState.keycode == Common::KEYCODE_y) {
			// Save the game
			if (save())
				setView("Game");
			else
				setView("Title");
		} else if (msg._keyState.keycode == Common::KEYCODE_n) {
			// Start at the beginning again
			setMode(FLAG_INITIAL);
		}
	} else {
		// Initial attributes allocation
		switch (msg._keyState.keycode) {
		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			_selectedAttribute = (_selectedAttribute == 0) ? ATTRIBUTE_COUNT - 1 : _selectedAttribute - 1;
			setMode(FLAG_ATTR_POINTERS);
			break;

		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2:
			_selectedAttribute = (_selectedAttribute == ATTRIBUTE_COUNT - 1) ? 0 : _selectedAttribute + 1;
			setMode(FLAG_ATTR_POINTERS);
			break;

		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_KP4:
			if (*_attributes[_selectedAttribute] > 10) {
				++_pointsRemaining;
				--*_attributes[_selectedAttribute];
			}
			setMode(FLAG_ATTRIBUTES | FLAG_POINTS);
			break;

		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_KP6:
			if (_pointsRemaining > 0 && *_attributes[_selectedAttribute] < 25) {
				--_pointsRemaining;
				++*_attributes[_selectedAttribute];
			}
			setMode(FLAG_ATTRIBUTES | FLAG_POINTS);
			break;

		case Common::KEYCODE_SPACE:
			if (_pointsRemaining == 0) {
				// Switch over to selecting race
				game->playFX(1);
				_selectedAttribute = -1;
				setMode(FLAG_RACE | FLAG_ATTR_POINTERS);
			} else {
				game->playFX(0);
			}
			break;

		case Common::KEYCODE_ESCAPE:
			// Switch back to main menu
			setView("Title");
			break;

		default:
			break;
		}
	}

	return true;
}

bool ViewCharacterGeneration::TextInputMsg(CTextInputMsg &msg) {
	if (!msg._escaped && !msg._text.empty()) {
		// Name provided
		_character->_name = msg._text;
		
		_textInput->hide();
		setMode(FLAG_SAVE);
	}

	return true;
}

bool ViewCharacterGeneration::save() {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	game->_randomSeed = game->getRandomNumber(0xfffffff);

	// Set the default position
	Shared::Maps::Map *map = game->_map;
	map->load(Ultima1::Maps::MAPID_OVERWORLD);
	map->setPosition(Point(49, 40));

	// Set other character properties
	_character->_hitPoints = 150;
	_character->_coins = 100;
	_character->_food = 200;
	_character->_experience = 0;
	_character->_equippedWeapon = 1;
	_character->_weapons[1]->_quantity = 1;			// Dagger
	_character->_equippedArmour = 1;
	_character->_armour[1]->_quantity = 1;			// Leather armour
	_character->_equippedSpell = 0;

	return g_vm->saveGameDialog();
}

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima
