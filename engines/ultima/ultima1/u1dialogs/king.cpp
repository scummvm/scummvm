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

#include "ultima/ultima1/u1dialogs/king.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/shared/gfx/visual_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(King, Dialog)
	ON_MESSAGE(ShowMsg)
	ON_MESSAGE(CharacterInputMsg)
END_MESSAGE_MAP()

King::King(Ultima1Game *game) : Dialog(game), _mode(SELECT) {
	_bounds = Rect(31, 23, 287, 127);
}

bool King::ShowMsg(CShowMsg &msg) {
	addInfoMsg(_game->_res->KING_TEXT[0], false);
	return true;
}

void King::draw() {
	Dialog::draw();
	Shared::Gfx::VisualSurface s = getSurface();

	if (_mode != SELECT) {
		// Draw the background and frame
		s.clear();
		s.frameRect(Rect(3, 3, _bounds.width() - 3, _bounds.height() - 3), getGame()->_borderColor);

		// Draw the title
	//	s.writeString(, Point((_bounds.width() - _title.size() * 8) / 2, 9));
	}

}

void King::setMode(KingMode mode) {
	_mode = mode;
	setDirty();
}

bool King::CharacterInputMsg(CCharacterInputMsg &msg) {
	switch (_mode) {
	case SELECT:
		if (msg._keyState.keycode == Common::KEYCODE_s)
			setMode(SERVICE);
		else if (msg._keyState.keycode == Common::KEYCODE_s)
			setMode(PENCE);
		else
			nothing();
		break;

	default:
		break;
	}

	return true;
}

void King::nothing() {
	addInfoMsg(Common::String::format(" %s", _game->_res->NOTHING));
	hide();
}

} // End of namespace U1Dialogs
} // End of namespace Gfx
} // End of namespace Ultima
