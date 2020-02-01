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

#include "ultima/ultima1/u1dialogs/transports.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/maps/map_overworld.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/game.h"
#include "ultima/shared/engine/messages.h"
#include "ultima/shared/core/str.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

EMPTY_MESSAGE_MAP(Transports, BuySellDialog);

Transports::Transports(Ultima1Game *game, int transportsNum) : BuySellDialog(game, game->_res->WEAPONRY_NAMES[transportsNum]) {
	loadOverworldFreeTiles();
}

void Transports::loadOverworldFreeTiles() {
	Maps::MapOverworld *map = static_cast<Maps::Ultima1Map *>(_game->_map)->getOverworldMap();
	Maps::Ultima1Map *currMap = static_cast<Maps::Ultima1Map *>(_game->_map);

	Point delta;

	Maps::U1MapTile mapTile;
	_water = _woods = _grass = 0;

	// Iterate through the tiles surrounding the city/castle
	for (delta.y = -1; delta.y <= 1; ++delta.y) {
		for (delta.x = -1; delta.x <= 1; ++delta.x) {
			if (delta.x != 0 || delta.y != 0) {
				map->getTileAt(map->getPosition() + delta, &mapTile);

				if (!mapTile._widget) {
					if (mapTile.isOriginalWater())
						++_water;
					else if (mapTile.isOriginalGrass())
						++_grass;
					else if (mapTile.isOriginalWoods())
						++_woods;
				}
			}
		}
	}

	// Count the number of transports
	_transportCount = 0;
	_hasShuttle = false;
	for (uint idx = 0; idx < map->_widgets.size(); ++idx) {
		if (dynamic_cast<Widgets::Transport *>(map->_widgets[idx].get()))
			++_transportCount;
		if (dynamic_cast<Widgets::Shuttle *>(map->_widgets[idx].get()))
			_hasShuttle = true;
	}

	_hasFreeTiles = _water != 0 || _woods != 0 || _grass != 0;
	_isClosed = !_hasFreeTiles || (_hasShuttle && _transportCount == 15)
		|| (!_grass && _transportCount == 15);

	bool flag = !_hasShuttle && _transportCount == 15;
	_transports[0] = _transports[1] = (_woods || _grass) && !flag;
	_transports[2] = _transports[3] = _water && !flag;
	_transports[4] = currMap->_moveCounter > 3000 && _grass && !flag;
	_transports[5] = currMap->_moveCounter > 3000 && _grass && !_hasShuttle;
}

void Transports::setMode(BuySell mode) {
	_mode = mode;
	setDirty();

	switch (mode) {
	case BUY: {
		addInfoMsg(Common::String::format("%s%s", _game->_res->ACTION_NAMES[19], _game->_res->BUY), false, true);

		if (_isClosed) {
			addInfoMsg(_game->_res->NOTHING, false);
			closeShortly();
		} else {
			getKeypress();
		}
		break;
	}

	case SELL:
		addInfoMsg(Common::String::format("%s%s%s", _game->_res->ACTION_NAMES[19], _game->_res->SELL, _game->_res->NOTHING), false, true);
		closeShortly();
		break;

	default:
		BuySellDialog::setMode(mode);
		break;
	}
}

uint Transports::getBuyCost(int transportIndex) const {
	const Shared::Character &c = *_game->_party;
	return (200 - c._intelligence) / 5 * transportIndex * transportIndex;
}


void Transports::draw() {
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

void Transports::drawBuy() {
	Shared::Gfx::VisualSurface s = getSurface();
	int titleLines = String(_title).split("\r\n").size();
	Common::String line;

	if (_hasFreeTiles) {
		for (int idx = 0, yp = titleLines + 2; idx < 6; ++idx) {
			if (_transports[idx]) {
				line = Common::String::format("%c) %s", 'a' + idx, _game->_res->TRANSPORT_NAMES[idx + 1]);
				s.writeString(line, TextPoint(8, yp));
				line = Common::String::format("- %u", getBuyCost(idx + 1));
				s.writeString(line, TextPoint(19, yp));
				++yp;
			}
		}
	} else {
		centerText(_game->_res->TRANSPORTS_TEXT[1], titleLines + 2);
	}
}

void Transports::drawSell() {
	int titleLines = String(_title).split("\r\n").size();
	centerText(String(_game->_res->TRANSPORTS_TEXT[0]).split("\r\n"), titleLines + 2);
}

bool Transports::CharacterInputMsg(CCharacterInputMsg &msg) {
	Shared::Character &c = *_game->_party;
	int transportIndex = msg._keyState.keycode - Common::KEYCODE_a;

	if (_mode == BUY) {
		if (msg._keyState.keycode >= Common::KEYCODE_a &&
				msg._keyState.keycode <= Common::KEYCODE_f &&
				_transports[transportIndex]) {
			uint cost = getBuyCost(transportIndex + 1);
			if (cost <= c._coins) {
				// Display the bought transport name in the info area
				addInfoMsg(_game->_res->TRANSPORT_NAMES[transportIndex + 1]);

				// Remove the cost, and add in the new transport
				c._coins -= cost;
				addTransport(transportIndex);

				// Show sold and close the dialog
				setMode(SOLD);
				return true;
			}
		}

		nothing();
		return true;
	} else {
		return BuySellDialog::CharacterInputMsg(msg);
	}
}

void Transports::addTransport(int transportIndex) {
	Maps::MapOverworld *map = static_cast<Maps::Ultima1Map *>(_game->_map)->getOverworldMap();
	Point delta;
	Maps::U1MapTile mapTile;
	const char *const WIDGET_NAMES[6] = {
		"Horse", "Cart", "Raft", "Frigate", "Aircar", "Shuttle"
	};

	// Iterate through the tiles surrounding the city/castle
	for (delta.y = -1; delta.y <= 1; ++delta.y) {
		for (delta.x = -1; delta.x <= 1; ++delta.x) {
			map->getTileAt(map->getPosition() + delta, &mapTile);

			if (!mapTile._widget && mapTile._locationNum == -1) {
				if ((transportIndex <= 1 && (mapTile.isOriginalWoods() || (!_woods && mapTile.isOriginalGrass())))
						|| (transportIndex >= 2 && transportIndex <= 3 && mapTile.isOriginalWater())
						|| (transportIndex >= 4 && mapTile.isOriginalGrass())) {
					// Add the transport onto the designated tile around the location
					Shared::Maps::MapWidget *widget = map->createWidget(WIDGET_NAMES[transportIndex]);
					assert(widget);
					widget->_position = map->getPosition() + delta;
					map->addWidget(widget);
					return;
				}
			}
		}
	}
}

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima
