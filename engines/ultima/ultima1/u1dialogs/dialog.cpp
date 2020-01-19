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

#include "ultima/ultima1/u1dialogs/dialog.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/shared/gfx/visual_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

Dialog::Dialog(Shared::GameBase *game) : Popup(game) {
	_bounds = Rect(31, 23, 287, 127);
}

Ultima1Game *Dialog::getGame() {
	return static_cast<Ultima1Game *>(TreeItem::getGame());
}

Maps::Ultima1Map *Dialog::getMap() {
	return static_cast<Maps::Ultima1Map *>(getGame()->getMap());
}

void Dialog::draw() {
	Shared::Gfx::VisualSurface s = getSurface();
	s.clear();

	// Draw the frame
	s.frameRect(Rect(3, 3, _bounds.width() - 3, _bounds.height() - 3), getGame()->_borderColor);
}

/*-------------------------------------------------------------------*/

void BuySellDialog::draw() {
	Dialog::draw();
	Shared::Gfx::VisualSurface s = getSurface();
	
	// Draw the title
	s.writeString(_title, Point((_bounds.width() - _title.size() * 8) / 2, 9), getGame()->_textColor);
}

} // End of namespace U1Dialogs
} // End of namespace Gfx
} // End of namespace Ultima
