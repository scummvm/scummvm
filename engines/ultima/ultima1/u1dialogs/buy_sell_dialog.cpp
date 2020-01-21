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

#include "ultima/ultima1/u1dialogs/buy_sell_dialog.h"
#include "ultima/shared/gfx/visual_surface.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(BuySellDialog, Dialog)
	ON_MESSAGE(CharacterInputMsg)
END_MESSAGE_MAP()

BuySellDialog::BuySellDialog(Ultima1Game *game, const Common::String &title) :
	Dialog(game), _mode(SELECT), _title(title), _charInput(game) {
	_bounds = Rect(31, 23, 287, 127);
}

void BuySellDialog::draw() {
	Dialog::draw();
	Shared::Gfx::VisualSurface s = getSurface();

	if (_mode != SELECT) {
		// Draw the background and frame
		s.clear();
		s.frameRect(Rect(3, 3, _bounds.width() - 3, _bounds.height() - 3), getGame()->_borderColor);

		// Draw the title
		s.writeString(_title, Point((_bounds.width() - _title.size() * 8) / 2, 9));
	}

	// Clear part of the bottom info text line
	s.fillRect(TextRect(8, 24, 28, 24), _game->_bgColor);

	switch (_mode) {
	case SELECT: {
		// Display the buy/sell text and prompt for input
		s.writeString(_game->_res->BUY_SELL, TextPoint(8, 24));
		_charInput.show(TextPoint(20, 24), _game->_textColor, this);
		break;
	}
	case BUY: {
		// Display the buy/sell text and prompt for input
		s.writeString(_game->_res->BUY, TextPoint(8, 24));
		_charInput.show(TextPoint(15, 24), _game->_textColor, this);
		break;
	}
	case SELL: {
		// Display the buy/sell text and prompt for input
		s.writeString(_game->_res->SELL, TextPoint(8, 24));
		_charInput.show(TextPoint(15, 24), _game->_textColor, this);
		break;
	}
	case SOLD:
		s.writeString(getGame()->_res->SOLD, TextPoint(14, 5));
		break;

	case CANT_AFFORD:
		s.writeString(getGame()->_res->CANT_AFFORD, TextPoint(4, 6));
		break;

	default:
		break;
	}
}

void BuySellDialog::setMode(BuySell mode) {
	_mode = mode;
	setDirty();
}

bool BuySellDialog::CharacterInputMsg(CCharacterInputMsg &msg) {
	switch (_mode) {
	case SELECT:
		if (msg._keyState.keycode == Common::KEYCODE_b)
			setMode(BUY);
		else if (msg._keyState.keycode == Common::KEYCODE_s)
			setMode(SELL);
		else
			nothing();
		break;

	default:
		break;
	}

	return true;
}

void BuySellDialog::nothing() {
	addInfoMsg(Common::String::format(" %s", _game->_res->NOTHING));
	hide();
}

} // End of namespace U1Dialogs
} // End of namespace Gfx
} // End of namespace Ultima
