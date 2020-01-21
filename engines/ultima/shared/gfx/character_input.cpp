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

#include "ultima/shared/gfx/character_input.h"
#include "ultima/shared/gfx/text_cursor.h"
#include "ultima/shared/early/game_base.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

BEGIN_MESSAGE_MAP(CharacterInput, Popup)
	ON_MESSAGE(KeypressMsg)
END_MESSAGE_MAP()

void CharacterInput::show(const Point &pt, byte color, TreeItem *respondTo) {
	Popup::show(respondTo);
	_color = color;
	_bounds = Rect(pt.x, pt.y, pt.x + 8, pt.y + 8);

	_game->_textCursor->setPosition(Point(_bounds.left, _bounds.top));
	_game->_textCursor->setVisible(true);
}

bool CharacterInput::KeypressMsg(CKeypressMsg &msg) {
	hide();

	CCharacterInputMsg inputMsg(msg._keyState);
	inputMsg.execute(_respondTo);

	return true;
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
