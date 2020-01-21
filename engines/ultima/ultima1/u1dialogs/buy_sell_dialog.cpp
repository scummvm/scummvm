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

#include "ultima/games/ultima1/u1dialogs/buy_sell_dialog.h"
#include "ultima/gfx/visual_surface.h"
#include "ultima/games/ultima1/game.h"
#include "ultima/games/ultima1/core/resources.h"
#include "ultima/games/ultima1/maps/map.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BuySellDialog::BuySellDialog(Ultima1Game *game, BuySell buySell, const Common::String &title) : Dialog(game),
	_buySell(buySell), _title(title) {
	assert(buySell == BUY || buySell == SOLD);
	_bounds = Rect(31, 23, 287, 127);
}

void BuySellDialog::draw() {
	Dialog::draw();
	Gfx::VisualSurface s = getSurface();
	
	// Draw the background and frame
	s.clear();
	s.frameRect(Rect(3, 3, _bounds.width() - 3, _bounds.height() - 3), getGame()->_borderColor);

	// Draw the title
	byte color = getGame()->_textColor;
	s.writeString(_title, Point((_bounds.width() - _title.size() * 8) / 2, 9), color);

	switch (_buySell) {
	case SOLD:
		s.writeString(getGame()->_res->SOLD, TextPoint(14, 5), color);
		break;

	case CANT_AFFORD:
		s.writeString(getGame()->_res->CANT_AFFORD, TextPoint(4, 6), color);
		break;

	default:
		break;
	}
}

void BuySellDialog::showSold() {
	_buySell = SOLD;
	setDirty();
}

void BuySellDialog::cantAfford() {
	_buySell = CANT_AFFORD;
	setDirty();
}

} // End of namespace U1Dialogs
} // End of namespace Gfx
} // End of namespace Ultima
