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

#ifndef ULTIMA1_VIEWS_H
#define ULTIMA1_VIEWS_H

#include "ultima/shared/engine/events.h"
#include "ultima/ultima1/views/interactions/dead.h"
#include "ultima/ultima1/views/interactions/drop.h"
#include "ultima/ultima1/views/interactions/king.h"
#include "ultima/ultima1/views/interactions/merchant.h"
#include "ultima/ultima1/views/interactions/princess_saved.h"
#include "ultima/ultima1/views/interactions/ready.h"
#include "ultima/ultima1/views/create_character.h"
#include "ultima/ultima1/views/game.h"
#include "ultima/ultima1/views/location_map.h"
#include "ultima/ultima1/views/main_menu.h"
#include "ultima/ultima1/views/overworld_map.h"
#include "ultima/ultima1/views/startup.h"
#include "ultima/ultima1/views/title.h"
#include "ultima/ultima1/views/zstats.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

struct Views : public Shared::Views {
	Interactions::Dead _dead;
	Interactions::Drop _drop;
	Interactions::King _king;
	Interactions::Merchant _merchant;
	Interactions::PrincessSaved _princessSaved;
	Interactions::Ready _ready;
	CreateCharacter _createCharacter;
	Game _game;
	LocationMap _locationMap;
	MainMenu _mainMenu;
	OverworldMap _overworldMap;
	Startup _startup;
	Title _title;
	ZStats _zStats;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
