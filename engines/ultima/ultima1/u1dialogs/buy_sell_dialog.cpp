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
	ON_MESSAGE(ShowMsg)
	ON_MESSAGE(CharacterInputMsg)
END_MESSAGE_MAP()

BuySellDialog::BuySellDialog(Ultima1Game *game, const Common::String &title) :
	Dialog(game), _mode(SELECT), _title(title), _charInput(game) {
	_bounds = Rect(31, 23, 287, 127);
}

bool BuySellDialog::ShowMsg(CShowMsg &msg) {
	addInfoMsg(_game->_res->BUY_SELL, false);
	getKeypress();
	return true;
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

	case CANT_AFFORD:
		addInfoMsg("", true, true);
		break;

	default:
		break;
	}

	return true;
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

	switch (_mode) {
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

	switch (_mode) {
	case BUY:
		addInfoMsg(Common::String::format("%s%s", _game->_res->ACTION_NAMES[19], _game->_res->BUY), false, true);
		getKeypress();
		break;

	case SELL:
		addInfoMsg(Common::String::format("%s%s", _game->_res->ACTION_NAMES[19], _game->_res->SELL), false, true);
		getKeypress();
		break;

	case CANT_AFFORD:
		addInfoMsg(_game->_res->NOTHING);
		addInfoMsg(_game->_res->PRESS_SPACE_TO_CONTINUE, false);
		_game->playFX(1);
		getKeypress();
		break;

	default:
		break;
	}
}

void BuySellDialog::nothing() {
	addInfoMsg(_game->_res->NOTHING);
	_game->endOfTurn();
	hide();
}

} // End of namespace U1Dialogs
} // End of namespace Gfx
} // End of namespace Ultima
