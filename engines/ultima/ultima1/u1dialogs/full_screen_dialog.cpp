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

#include "ultima/ultima1/u1dialogs/full_screen_dialog.h"
#include "ultima/ultima1/u1gfx/drawing_support.h"
#include "ultima/ultima1/game.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

FullScreenDialog::FullScreenDialog(Ultima1Game *game) : Dialog(game) {
	_bounds = Common::Rect(0, 0, 320, 200);
}
	
void FullScreenDialog::drawFrame(const Common::String &title) {
	Shared::Gfx::VisualSurface s = getSurface();
	U1Gfx::DrawingSupport ds(s);
	s.fillRect(TextRect(0, 0, 40, 20), _game->_bgColor);
	ds.drawGameFrame();

	size_t titleLen = title.size() + 2;
	size_t xStart = 20 - titleLen / 2;
	ds.drawRightArrow(TextPoint(xStart - 1, 0));
	s.fillRect(TextRect(xStart, 0, xStart + titleLen, 0), 0);
	s.writeString(title, TextPoint(xStart + 1, 0));
	ds.drawLeftArrow(TextPoint(xStart + titleLen, 0));
}

void FullScreenDialog::hide() {
	Ultima1Game *game = _game;
	Dialog::hide();

	game->endOfTurn();
}

} // End of namespace U1Dialogs
} // End of namespace Gfx
} // End of namespace Ultima
