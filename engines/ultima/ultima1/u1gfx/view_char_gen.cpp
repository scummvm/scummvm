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
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Gfx {

BEGIN_MESSAGE_MAP(ViewCharacterGeneration, Shared::Gfx::VisualContainer)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

ViewCharacterGeneration::ViewCharacterGeneration(Shared::TreeItem *parent) :
		Shared::Gfx::VisualContainer("CharGen", Rect(0, 0, 320, 200), parent), _flags(FLAG_INITIAL),
		_pointsRemaining(30), _selectedAttribute(0) {
	_character._strength = 10;
	_character._agility = 10;
	_character._stamina = 10;
	_character._charisma = 10;
	_character._wisdom = 10;
	_character._intelligence = 10;
	_attributes[0] = &_character._strength;
	_attributes[1] = &_character._agility;
	_attributes[2] = &_character._stamina;
	_attributes[3] = &_character._charisma;
	_attributes[4] = &_character._wisdom;
	_attributes[5] = &_character._intelligence;
}

void ViewCharacterGeneration::draw() {
	VisualContainer::draw();
	Shared::Gfx::VisualSurface s = getSurface();

	if (_flags & FLAG_FRAME)
		drawFrame(s);
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

void ViewCharacterGeneration::drawAttributes(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	s.writeString(Common::String::format(game->_res->CHAR_GEN_TEXT[1], _pointsRemaining),
		TextPoint(6, 4), game->_textColor);
	s.writeString(Common::String::format(game->_res->CHAR_GEN_TEXT[2],
		_character._strength, _character._agility, _character._stamina,
		_character._charisma, _character._wisdom, _character._intelligence),
		TextPoint(12, 6), game->_textColor);
}

void ViewCharacterGeneration::drawAttributePointers(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	DrawingSupport ds(s);

	s.fillRect(Common::Rect(11 * 8, 6 * 8, 12 * 8, 12 * 8), game->_bgColor);
	s.fillRect(Common::Rect(30 * 8, 6 * 8, 31 * 8, 12 * 8), game->_bgColor);
	ds.drawRightArrow(TextPoint(11, _selectedAttribute + 6));
	ds.drawLeftArrow(TextPoint(30, _selectedAttribute + 6));
}

void ViewCharacterGeneration::drawHelp(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
	s.writeString(game->_res->CHAR_GEN_TEXT[3], TextPoint(2, 16), game->_textColor);
}

void ViewCharacterGeneration::drawRace(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());

}

void ViewCharacterGeneration::drawSex(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());

}

void ViewCharacterGeneration::drawClass(Shared::Gfx::VisualSurface &s) {
	Ultima1Game *game = static_cast<Ultima1Game *>(getGame());
}

bool ViewCharacterGeneration::KeypressMsg(CKeypressMsg &msg) {

	return true;
}

} // End of namespace U1Gfx
} // End of namespace Shared
} // End of namespace Ultima
