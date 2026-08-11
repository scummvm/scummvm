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

#ifndef ULTIMA0_VIEWS_H
#define ULTIMA0_VIEWS_H

#include "ultima/ultima0/views/acknowledgements.h"
#include "ultima/ultima0/views/attack.h"
#include "ultima/ultima0/views/castle.h"
#include "ultima/ultima0/views/create_character.h"
#include "ultima/ultima0/views/dead.h"
#include "ultima/ultima0/views/dungeon.h"
#include "ultima/ultima0/views/info.h"
#include "ultima/ultima0/views/intro.h"
#include "ultima/ultima0/views/startup.h"
#include "ultima/ultima0/views/title.h"
#include "ultima/ultima0/views/town.h"
#include "ultima/ultima0/views/world_map.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

struct Views {
	Acknowledgements _acknowledgements;
	Attack _attack;
	Castle _castle;
	CreateCharacter _createCharacter;
	Dead _dead;
	Dungeon _dungeon;
	Info _info;
	Intro _intro;
	Startup _startup;
	Title _title;
	Town _town;
	WorldMap _worldMap;
};

} // namespace Views
} // namespace Ultima0
} // namespace Ultima

#endif
