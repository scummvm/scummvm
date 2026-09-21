/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA2_VIEWS_H
#define ULTIMA2_VIEWS_H

#include "ultima/shared/engine/events.h"
#include "ultima/ultima2/views/create_character.h"
#include "ultima/ultima2/views/dungeon_map.h"
#include "ultima/ultima2/views/game.h"
#include "ultima/ultima2/views/location_map.h"
#include "ultima/ultima2/views/overworld_map.h"
#include "ultima/ultima2/views/space_map.h"
#include "ultima/ultima2/views/startup.h"
#include "ultima/ultima2/views/title.h"
#include "ultima/ultima2/views/world_map_overview.h"
#include "ultima/ultima2/views/zstats.h"
#include "ultima/ultima2/views/interactions/clue_seller.h"
#include "ultima/ultima2/views/interactions/dead.h"
#include "ultima/ultima2/views/interactions/direction.h"
#include "ultima/ultima2/views/interactions/food_shop.h"
#include "ultima/ultima2/views/interactions/hyperwarp_target.h"
#include "ultima/ultima2/views/interactions/offer_gold.h"
#include "ultima/ultima2/views/interactions/plane_flight.h"
#include "ultima/ultima2/views/interactions/pub.h"
#include "ultima/ultima2/views/interactions/ready_spell.h"
#include "ultima/ultima2/views/interactions/ready_weapon.h"
#include "ultima/ultima2/views/interactions/rocket_landing.h"
#include "ultima/ultima2/views/interactions/shop.h"
#include "ultima/ultima2/views/interactions/transport.h"
#include "ultima/ultima2/views/interactions/wear_armor.h"
#include "ultima/ultima2/views/interactions/yell.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

struct Views : public Shared::Views {
	Interactions::ClueSeller _clueSeller;
	Interactions::Dead _dead;
	Interactions::Direction _direction;
	Interactions::FoodShop _foodShop;
	Interactions::HyperwarpTarget _hyperwarpTarget;
	Interactions::OfferGold _offerGold;
	Interactions::PlaneFlight _planeFlight;
	Interactions::Pub _pub;
	Interactions::ReadySpell _readySpell;
	Interactions::ReadyWeapon _readyWeapon;
	Interactions::RocketLanding _rocketLanding;
	Interactions::Transport _transport;
	Interactions::WearArmor _wearArmor;
	Interactions::Yell _yell;
	Interactions::Shop _weaponShop{Interactions::Shop::WEAPON, "WeaponShop"};
	Interactions::Shop _armorShop{Interactions::Shop::ARMOR, "ArmorShop"};
	Interactions::Shop _clericSpellShop{Interactions::Shop::SPELL_CLERIC, "ClericSpellShop"};
	Interactions::Shop _wizardSpellShop{Interactions::Shop::SPELL_WIZARD, "WizardSpellShop"};

	CreateCharacter _createCharacter;
	DungeonMap _dungeonMap;
	Game _game;
	LocationMap _locationMap;
	OverworldMap _overworldMap;
	SpaceMap _spaceMap;
	Startup _startup;
	Title _title;
	WorldMapOverview _worldMapOverview;
	ZStats _zstats;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
