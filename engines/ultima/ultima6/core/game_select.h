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

#ifndef ULTIMA6_CORE_GAME_SELECT_H
#define ULTIMA6_CORE_GAME_SELECT_H

namespace Ultima {
namespace Ultima6 {

class Configuration;
class Screen;
class Game;

class GameSelect {
protected:

	Configuration *config;
	Screen *screen;
	Game *game;

public:

	GameSelect(Configuration *cfg);
	~GameSelect();

	uint8 load(Screen *s, uint8 game_type = NUVIE_GAME_NONE);

};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
