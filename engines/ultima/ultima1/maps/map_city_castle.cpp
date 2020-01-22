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

#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/spells/spell.h"
#include "ultima/ultima1/widgets/urban_player.h"
#include "ultima/ultima1/widgets/person.h"
#include "ultima/ultima1/widgets/bard.h"
#include "ultima/ultima1/widgets/guard.h"
#include "ultima/ultima1/widgets/king.h"
#include "ultima/ultima1/widgets/princess.h"
#include "ultima/ultima1/widgets/wench.h"
#include "ultima/ultima1/widgets/merchant_armour.h"
#include "ultima/ultima1/widgets/merchant_grocer.h"
#include "ultima/ultima1/widgets/merchant_magic.h"
#include "ultima/ultima1/widgets/merchant_tavern.h"
#include "ultima/ultima1/widgets/merchant_transport.h"
#include "ultima/ultima1/widgets/merchant_weapons.h"
#include "ultima/ultima1/u1dialogs/drop.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

void MapCityCastle::load(Shared::Maps::MapId mapId) {
	clear();
	Shared::Maps::MapBase::load(mapId);

	setDimensions(Point(38, 18));
	_tilesPerOrigTile = Point(1, 1);
}

void MapCityCastle::clear() {
	Shared::Maps::MapBase::clear();
	_guardsHostile = false;
}

void MapCityCastle::loadWidgets() {
	// Set up widget for the player
	_playerWidget = new Widgets::UrbanPlayer(_game, this);
	addWidget(_playerWidget);

	for (int idx = 0; idx < 15; ++idx) {
		const int *lp = _game->_res->LOCATION_PEOPLE[_mapStyle * 15 + idx];
		if (lp[0] == -1)
			break;

		Widgets::Person *person;
		switch (lp[0]) {
		case 17:
			person = new Widgets::Guard(_game, this, lp[3]);
			break;
		case 19:
			person = new Widgets::Bard(_game, this, lp[3]);
			break;
		case 20:
			person = new Widgets::King(_game, this, lp[3]);
			break;
		case 21: {
			U1MapTile tile;
			getTileAt(Point(lp[1], lp[2]), &tile);

			switch (tile._tileId) {
			case 55:
				person = new Widgets::MerchantArmour(_game, this, lp[3]);
				break;
			case 57:
				person = new Widgets::MerchantGrocer(_game, this, lp[3]);
				break;
			case 59:
				person = new Widgets::MerchantWeapons(_game, this, lp[3]);
				break;
			case 60:
				person = new Widgets::MerchantMagic(_game, this, lp[3]);
				break;
			case 61:
				person = new Widgets::MerchantTavern(_game, this, lp[3]);
				break;
			case 62:
				person = new Widgets::MerchantTransport(_game, this, lp[3]);
				break;
			default:
				error("Invalid merchant");
			}
			break;
		}
		case 22:
			person = new Widgets::Princess(_game, this, lp[3]);
			break;
		case 50:
			person = new Widgets::Wench(_game, this, lp[3]);
			break;
		default:
			error("Unknown NPC type %d", lp[0]);
		}
		
		person->_position = Point(lp[1], lp[2]);
		addWidget(person);
	}
}

void MapCityCastle::getTileAt(const Point &pt, Shared::Maps::MapTile *tile, bool includePlayer) {
	MapBase::getTileAt(pt, tile, includePlayer);

	// Special handling for the cells indicating various merchant talk/steal positions
	if (tile->_tileDisplayNum >= 51)
		tile->_tileDisplayNum = 1;
}

Point MapCityCastle::getViewportPosition(const Point &viewportSize) {
	Point &topLeft = _viewportPos._topLeft;

	if (!_viewportPos.isValid() || _viewportPos._size != viewportSize) {
		// Calculate the new position
		topLeft.x = _playerWidget->_position.x - (viewportSize.x - 1) / 2;
		topLeft.y = _playerWidget->_position.y - (viewportSize.y - 1) / 2;

		// Fixed maps, so constrain top left corner so the map fills the viewport. This will accomodate
		// future renderings with more tiles, or greater tile size
		topLeft.x = CLIP((int)topLeft.x, 0, (int)(width() - viewportSize.x));
		topLeft.y = CLIP((int)topLeft.y, 0, (int)(height() - viewportSize.y));

		_viewportPos._mapId = _mapId;
		_viewportPos._size = viewportSize;
	}

	return topLeft;
}

void MapCityCastle::loadTownCastleData() {
	// Load the contents of the map
	Shared::File f("tcd.bin");
	f.seek(_mapStyle * 684);
	for (int x = 0; x < _size.x; ++x) {
		for (int y = 0; y < _size.y; ++y)
			_data[y][x] = f.readByte();
	}
}

Widgets::Merchant *MapCityCastle::getStealMerchant() {
	U1MapTile tile;
	getTileAt(getPosition(), &tile);

	// Scan for the correct merchant depending on the tile player is on
	switch (tile._tileId) {
	case 55:
		return dynamic_cast<Widgets::MerchantArmour *>(_widgets.findByClass(Widgets::MerchantArmour::type()));
		break;
	case 57:
		return dynamic_cast<Widgets::MerchantGrocer *>(_widgets.findByClass(Widgets::MerchantGrocer::type()));
		break;
	case 59:
		return dynamic_cast<Widgets::MerchantWeapons *>(_widgets.findByClass(Widgets::MerchantWeapons::type()));
		break;
	default:
		return nullptr;
	}
}

Widgets::Person *MapCityCastle::getTalkPerson() {
	U1MapTile tile;
	getTileAt(getPosition(), &tile);

	switch (tile._tileId) {
	case 54:
	case 55:
		return dynamic_cast<Widgets::Person *>(_widgets.findByClass(Widgets::MerchantArmour::type()));

	case 56:
	case 57:
		return dynamic_cast<Widgets::Person *>(_widgets.findByClass(Widgets::MerchantGrocer::type()));

	case 58:
	case 59:
		return dynamic_cast<Widgets::Person *>(_widgets.findByClass(Widgets::MerchantWeapons::type()));

	case 60:
		return dynamic_cast<Widgets::Person *>(_widgets.findByClass(Widgets::MerchantMagic::type()));

	case 61:
		return dynamic_cast<Widgets::Person *>(_widgets.findByClass(Widgets::MerchantTavern::type()));

	case 62:
		return dynamic_cast<Widgets::Person *>(_widgets.findByClass(
			dynamic_cast<MapCity *>(this) ? Widgets::MerchantTransport::type() : Widgets::King::type() ));

	default:
		return nullptr;
	}

}

void MapCityCastle::cast() {
	addInfoMsg(Common::String::format(" -- %s", _game->_res->NO_EFFECT));
	_game->playFX(6);
}

void MapCityCastle::drop() {
	U1Dialogs::Drop *drop = new U1Dialogs::Drop(_game);
	drop->show();
}

void MapCityCastle::inform() {
	addInfoMsg("");
	addInfoMsg(_name);
}

void MapCityCastle::steal() {
	Widgets::Merchant *merchant = getStealMerchant();

	if (merchant) {
		// Found a merchant, so call their steal handler
		merchant->steal();
	} else {
		addInfoMsg(_game->_res->NOTHING_HERE);
		_game->playFX(1);
	}
}

void MapCityCastle::attack(int direction, int effectId, uint maxDistance, uint amount, uint agility, const Common::String &hitWidget) {
	_game->playFX(effectId);
	Point delta = getDirectionDelta();
	U1MapTile tile;
	Widgets::Person *person;
	//int currTile;

	// Scan in the given direction for a person to attack
	uint distance = 1;
	do {
		Point pt = getPosition() + Point(delta.x * distance, delta.y * distance);
		getTileAt(pt, &tile);
		//currTile = tile._tileId == CTILE_63 ? -1 : tile._tileId;
		person = dynamic_cast<Widgets::Person *>(tile._widget);

	} while (++distance <= maxDistance && !person && (tile._tileId == CTILE_GROUND || tile._tileId >= CTILE_POND_EDGE1));

	if (person && _game->getRandomNumber(1, 100) <= agility) {
		addInfoMsg(Common::String::format(_game->_res->HIT_CREATURE, person->_name.c_str()), false);

		// Damage the person
		if (person->subtractHitPoints(amount)) {
			// Killed them
			addInfoMsg(_game->_res->KILLED);
		} else {
			// Still alive
			addInfoMsg(Common::String::format("%u %s!", amount, _game->_res->DAMAGE));
		}
	} else {
		addInfoMsg(_game->_res->MISSED);
	}

	_game->endOfTurn();
}

bool MapCityCastle::isWenchNearby() const {
	Shared::Maps::MapWidget *widget = _widgets.findByClass(Widgets::Wench::type());
	if (!widget)
		return false;

	const Point &playerPos = _playerWidget->_position;
	const Point &wenchPos = widget->_position;
	int distance = MAX(ABS(playerPos.x - wenchPos.x), ABS(playerPos.y - wenchPos.y));
	return distance == 1;
}

/*-------------------------------------------------------------------*/

void MapCity::load(Shared::Maps::MapId mapId) {
	MapCityCastle::load(mapId);

	_mapStyle = ((_mapId - 1) % 8) + 2;
	_mapIndex = _mapId;
	_name = Common::String::format("%s %s", _game->_res->THE_CITY_OF, _game->_res->LOCATION_NAMES[_mapId - 1]);

	loadTownCastleData();

	// Load up the widgets for the given map
	loadWidgets();
	setPosition(Common::Point(width() / 2, height() - 1));		// Start at bottom center edge of map
}

void MapCity::dropCoins(uint coins) {
	Shared::Character &c = *_game->_party;
	U1MapTile tile;
	getTileAt(getPosition(), &tile);

	if (tile._tileId == CTILE_POND_EDGE1 || tile._tileId == CTILE_POND_EDGE2 || tile._tileId == CTILE_POND_EDGE3) {
		addInfoMsg(_game->_res->SHAZAM);
		_game->playFX(5);

		switch (tile._tileId) {
		case CTILE_POND_EDGE1: {
			// Increase one of the attributes randomly
			uint *attrList[6] = { &c._strength, &c._agility, &c._stamina, &c._charisma, &c._wisdom, &c._intelligence };
			uint &attr = *attrList[_game->getRandomNumber(0, 5)];
			
			attr = MIN(attr + coins / 10, 99U);
			break;
		}

		case CTILE_POND_EDGE2: {
			// Increase the quantity of a random weapon
			uint weaponNum = _game->getRandomNumber(1, 15);
			Shared::Weapon &weapon = *c._weapons[weaponNum];
			weapon._quantity = MIN(weapon._quantity + 1, 255U);
			break;
		}

		case CTILE_POND_EDGE3:
			// Increase food
			c._food += coins;
			break;

		default:
			break;
		}
	} else {
		addInfoMsg(_game->_res->OK);
	}
}

void MapCity::get() {
	addInfoMsg(_game->_res->WHAT);
	_game->playFX(1);
}

void MapCity::talk() {
	if (_guardsHostile) {
		addInfoMsg(_game->_res->NONE_WILL_TALK);
	} else {
		Widgets::Person *person = getTalkPerson();

		if (person) {
			person->talk();
		} else {
			addInfoMsg("");
			addInfoMsg(_game->_res->NOT_BY_COUNTER);
			_game->endOfTurn();
		}
	}
}

void MapCity::unlock() {
	addInfoMsg(_game->_res->WHAT);
	_game->playFX(1);
}

/*-------------------------------------------------------------------*/

void MapCastle::load(Shared::Maps::MapId mapId) {
	MapCityCastle::load(mapId);

	_mapIndex = _mapId - 33;
	_mapStyle = _mapIndex % 2;
	_name = _game->_res->LOCATION_NAMES[_mapId - 1];
	_castleKey = _game->getRandomNumber(255) & 1 ? 61 : 60;
	_getCounter = 0;

	loadTownCastleData();

	// Set up door locks
	_data[_mapStyle ? 4 : 14][35] = CTILE_GATE;
	_data[_mapStyle ? 4 : 14][31] = CTILE_GATE;

	// Load up the widgets for the given map
	loadWidgets();
	setPosition(Common::Point(0, height() / 2));		// Start at center left edge of map
}

void MapCastle::synchronize(Common::Serializer &s) {
	MapCityCastle::synchronize(s);
	s.syncAsByte(_castleKey);
	s.syncAsByte(_getCounter);
	s.syncAsByte(_freeingPrincess);
}

void MapCastle::dropCoins(uint coins) {
	Shared::Character &c = *_game->_party;
	U1MapTile tile;
	getTileAt(getPosition(), &tile);

	if (tile._tileId == CTILE_POND_EDGE1) {
		uint hp = coins * 3 / 2;
		c._hitPoints = MIN(c._hitPoints + hp, 9999U);
		
		if (_game->getRandomNumber(1, 255) > 16) {
			addInfoMsg(_game->_res->SHAZAM);
		} else {
			uint spellNum = _game->getRandomNumber(1, 7);
			if (spellNum == Spells::SPELL_MAGIC_MISSILE)
				spellNum = Spells::SPELL_STEAL;
		
			c._spells[spellNum]->incrQuantity();
			addInfoMsg(_game->_res->ALAKAZOT);
		}
	} else {
		addInfoMsg(_game->_res->OK);
	}
}

void MapCastle::get() {
	Widgets::Merchant *merchant = getStealMerchant();

	if (merchant) {
		// Found a merchant, so call their get handler
		merchant->get();
	} else {
		addInfoMsg(_game->_res->NOTHING_HERE);
		_game->playFX(1);
	}
}

void MapCastle::talk() {
	addInfoMsg(_game->_res->WITH_KING);
	Widgets::Person *person = getTalkPerson();

	if (person) {
		person->talk();
	} else {
		addInfoMsg(_game->_res->HE_IS_NOT_HERE);
		_game->endOfTurn();
	}
}

void MapCastle::unlock() {
	U1MapTile tile;
	Point pt = getPosition();
	getTileAt(pt, &tile);

	if (tile._tileId != CTILE_LOCK1 && tile._tileId != CTILE_LOCK2) {
		addInfoMsg(_game->_res->WHAT);
		_game->playFX(1);
	} else if (!_castleKey) {
		addInfoMsg(_game->_res->NO_KEY);
	} else if (tile._tileId != (int)_castleKey) {
		addInfoMsg(_game->_res->INCORRECT_KEY);
	} else {
		addInfoMsg(_game->_res->DOOR_IS_OPEN);
		_data[pt.y][pt.x] = CTILE_GROUND;
		_freeingPrincess = true;
	}
}

bool MapCastle::isLordBritishCastle() const {
	return getMapIndex() == 0;
}

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima
