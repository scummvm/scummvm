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

#include "ultima/ultima1/maps/map_base.h"
#include "ultima/ultima1/maps/map_tile.h"
#include "ultima/ultima1/maps/map_dungeon.h"
#include "ultima/ultima1/maps/map_city_castle.h"
#include "ultima/ultima1/maps/map_overworld.h"
#include "ultima/ultima1/maps/map.h"
#include "ultima/ultima1/core/party.h"
#include "ultima/ultima1/core/resources.h"
#include "ultima/ultima1/game.h"
#include "ultima/ultima1/spells/spell.h"
#include "ultima/ultima1/widgets/dungeon_widget.h"
#include "ultima/ultima1/widgets/dungeon_item.h"
#include "ultima/ultima1/widgets/merchant_armour.h"
#include "ultima/ultima1/widgets/merchant_grocer.h"
#include "ultima/ultima1/widgets/merchant_magic.h"
#include "ultima/ultima1/widgets/merchant_tavern.h"
#include "ultima/ultima1/widgets/merchant_transport.h"
#include "ultima/ultima1/widgets/merchant_weapons.h"
#include "ultima/ultima1/widgets/overworld_monster.h"
#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/widgets/dungeon_monster.h"
#include "ultima/ultima1/widgets/dungeon_player.h"
#include "ultima/ultima1/widgets/dungeon_chest.h"
#include "ultima/ultima1/widgets/dungeon_coffin.h"
#include "ultima/ultima1/widgets/bard.h"
#include "ultima/ultima1/widgets/guard.h"
#include "ultima/ultima1/widgets/king.h"
#include "ultima/ultima1/widgets/princess.h"
#include "ultima/ultima1/widgets/transport.h"
#include "ultima/ultima1/widgets/urban_player.h"
#include "ultima/ultima1/widgets/wench.h"

namespace Ultima {
namespace Ultima1 {
namespace Maps {

MapBase::MapBase(Ultima1Game *game, Ultima1Map *map) : Shared::Maps::MapBase(game, map), _game(game) {
}

void MapBase::getTileAt(const Point &pt, Shared::Maps::MapTile *tile, bool includePlayer) {
	Shared::Maps::MapBase::getTileAt(pt, tile, includePlayer);

	// Extended properties to set if an Ultima 1 map tile structure was passed in
	U1MapTile *mapTile = dynamic_cast<U1MapTile *>(tile);
	if (mapTile) {
		GameResources *res = _game->_res;
		mapTile->setMap(this);

		// Check for a location at the given position
		mapTile->_locationNum = -1;
		if (dynamic_cast<MapOverworld *>(this)) {
			for (int idx = 0; idx < LOCATION_COUNT; ++idx) {
				if (pt.x == res->LOCATION_X[idx] && pt.y == res->LOCATION_Y[idx]) {
					mapTile->_locationNum = idx + 1;
					break;
				}
			}
		}

		// Check for a dungeon item
		for (uint idx = 0; idx < _widgets.size() && !mapTile->_item; ++idx) {
			mapTile->_item = dynamic_cast<Widgets::DungeonItem *>(_widgets[idx].get());
		}
	}
}

void MapBase::unknownAction() {
	addInfoMsg("?");
	_game->playFX(1);
}

void MapBase::attack(int direction, int effectId) {
	uint agility, damage, maxDistance;
	Widgets::Transport *transport = dynamic_cast<Widgets::Transport *>(_playerWidget);

	if (effectId == 7) {
		Character &c = *static_cast<Party *>(_game->_party);
		maxDistance = c.equippedWeapon()->_distance;
		agility = c._agility + 50;
		damage = _game->getRandomNumber(2, c._strength + c._equippedWeapon * 8);
	} else {
		maxDistance = 3;
		agility = 80;
		damage = _game->getRandomNumber(1, (transport ? transport->transportId() : 0) * 10);
	}

	attack(direction, effectId, maxDistance, damage, agility, "PhysicalAttack");
}

void MapBase::board() {
	unknownAction();
	_game->endOfTurn();
}

void MapBase::cast() {
	const Shared::Character &c = *_game->_party;
	Shared::Spell &spell = *c._spells[c._equippedSpell];
	addInfoMsg(Common::String::format(" %s", spell._name.c_str()), false);

	if (c._equippedSpell == Spells::SPELL_PRAYER) {
		castSpell(c._equippedSpell);
	} else if (spell._quantity == 0) {
		addInfoMsg("");
		addInfoMsg(_game->_res->USED_UP_SPELL);
		_game->playFX(6);
	} else {
		spell.decrQuantity();
		castSpell(c._equippedSpell);
	}
}

void MapBase::castSpell(uint spellId) {
	const Shared::Character &c = *_game->_party;
	static_cast<Spells::Spell *>(c._spells[spellId])->cast(this);
}

Shared::Maps::MapWidget *MapBase::createWidget(const Common::String &name) {
	REGISTER_WIDGET(Bard);
	REGISTER_WIDGET(DungeonMonster);
	REGISTER_WIDGET(DungeonPlayer);
	REGISTER_WIDGET(DungeonChest);
	REGISTER_WIDGET(DungeonCoffin);
	REGISTER_WIDGET(Guard);
	REGISTER_WIDGET(King);
	REGISTER_WIDGET(MerchantArmour);
	REGISTER_WIDGET(MerchantGrocer);
	REGISTER_WIDGET(MerchantMagic);
	REGISTER_WIDGET(MerchantTavern);
	REGISTER_WIDGET(MerchantTransport);
	REGISTER_WIDGET(MerchantWeapons);
	REGISTER_WIDGET(OverworldMonster);
	REGISTER_WIDGET(Princess);
	REGISTER_WIDGET(TransportOnFoot);
	REGISTER_WIDGET(UrbanPlayer);
	REGISTER_WIDGET(Wench);
	REGISTER_WIDGET(Horse);
	REGISTER_WIDGET(Cart);
	REGISTER_WIDGET(Raft);
	REGISTER_WIDGET(Frigate);
	REGISTER_WIDGET(Aircar);
	REGISTER_WIDGET(Shuttle);

	error("Unknown widget type '%s'", name.c_str());
}

} // End of namespace Maps
} // End of namespace Ultima1
} // End of namespace Ultima
