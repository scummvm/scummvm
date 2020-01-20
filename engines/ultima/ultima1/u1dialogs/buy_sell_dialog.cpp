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
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/shared/core/str.h"
#include "ultima/shared/gfx/visual_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(BuySellDialog, Dialog)
	ON_MESSAGE(ShowMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(CharacterInputMsg)
END_MESSAGE_MAP()

BuySellDialog::BuySellDialog(Ultima1Game *game, const Common::String &title) :
	Dialog(game), _mode(SELECT), _title(title), _charInput(game), _closeCounter(0) {
	_bounds = Rect(31, 23, 287, 127);
}

bool BuySellDialog::ShowMsg(CShowMsg &msg) {
	addInfoMsg(_game->_res->BUY_SELL, false);
	getKeypress();
	return true;
}

bool BuySellDialog::FrameMsg(CFrameMsg &msg) {
	if (_closeCounter > 0 && --_closeCounter == 0) {
		_game->endOfTurn();
		hide();
	}

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
		centerText(String(_title).split('\n'), 1);
	}

	switch (_mode) {
	case SOLD:
		centerText(getGame()->_res->SOLD, 5);
		break;

	case CANT_AFFORD:
		centerText(getGame()->_res->CANT_AFFORD, 5);
		break;

	case DONE:
		centerText(getGame()->_res->DONE, 5);
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
		_game->playFX(1);
		break;

	default:
		break;
	}

	if (_mode == SOLD || _mode == CANT_AFFORD || _mode == DONE)
		// Start dialog close countdown
		closeShortly();
}

void BuySellDialog::nothing() {
	addInfoMsg(_game->_res->NOTHING);
	_game->endOfTurn();
	hide();
}

} // End of namespace U1Dialogs
} // End of namespace Gfx
} // End of namespace Ultima
