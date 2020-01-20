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

#include "ultima/ultima1/u1dialogs/tavern.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/core/str.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

BEGIN_MESSAGE_MAP(Tavern, BuySellDialog)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

Tavern::Tavern(Ultima1Game *game, Maps::MapCityCastle *map, int tavernNum) :
		BuySellDialog(game, game->_res->TAVERN_NAMES[tavernNum]), _map(map),
		_tavernNum(tavernNum), _countdown(0), _tipNumber(0), _buyDisplay(INITIAL) {
}

void Tavern::setMode(BuySell mode) {
	switch (mode) {
	case BUY:
		_mode = BUY;
		addInfoMsg(Common::String::format("%s%s%s", _game->_res->ACTION_NAMES[19], _game->_res->BUY, _game->_res->TAVERN_TEXT[3]), false, true);
		setDirty();
		delay(DIALOG_CLOSE_DELAY * 2);
		break;

	case SELL:
		_mode = SELL;
		addInfoMsg(Common::String::format("%s%s", _game->_res->ACTION_NAMES[19], _game->_res->SELL), false, true);

		delay(DIALOG_CLOSE_DELAY * 2);
		setDirty();
		break;

	default:
		BuySellDialog::setMode(mode);
		break;
	}
}

bool Tavern::FrameMsg(CFrameMsg &msg) {
	const Shared::Character &c = *_game->_party;

	if (_countdown > 0 && --_countdown == 0) {
		switch (_mode) {
		case BUY:
			switch (_buyDisplay) {
			case INITIAL:
				if (c._coins != 0) {
					if (++_map->_tipCounter > (c._stamina / 4) && isWenchNearby()) {
						// TODO
					}
				}
				break;

			default:
				break;
			}

			addInfoMsg("");
			_game->endOfTurn();
			hide();
			break;

		case SELL:
			addInfoMsg(_game->_res->NOTHING);
			_game->endOfTurn();
			hide();
			break;
		default:
			break;
		}
	}

	return true;
}

bool Tavern::isWenchNearby() const {
	//Shared::Maps::Map *map = _game->_map;
	
//	dynamic_cast<Widgets::Person *>(_widgets.findByClass(Widgets::MerchantGrocer::type()))
	
	//Point playerPos = map->getPosition();
	return true;
}

void Tavern::draw() {
	BuySellDialog::draw();

	switch (_mode) {
	case BUY:
		drawBuy();
		break;

	case SELL:
		drawSell();
		break;

	default:
		break;
	}
}

void Tavern::drawBuy() {
	Shared::Gfx::VisualSurface s = getSurface();
	const Shared::Character &c = *_game->_party;
	int titleLines = String(_title).split("\r\n").size();

	switch (_buyDisplay) {
	case INITIAL:
		if (c._coins == 0) {
			// Broke
			centerText(String(_game->_res->TAVERN_TEXT[0]).split("\r\n"), titleLines + 2);
		} else {
			// Initial ale give
			centerText(String(_game->_res->TAVERN_TEXT[2]).split("\r\n"), titleLines + 2);
		}
		break;

	case TIP0:
	case TIP_PAGE1: {
		
	}

	default:
		break;
	}

}

void Tavern::drawSell() {
	int titleLines = String(_title).split("\r\n").size();

	centerText(String(_game->_res->TAVERN_TEXT[1]).split("\r\n"), titleLines + 2);
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
