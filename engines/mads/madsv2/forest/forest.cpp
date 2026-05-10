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

#include "engines/util.h"
#include "mads/madsv2/console.h"
#include "mads/madsv2/forest/forest.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/screen.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

Common::Error ForestEngine::run() {
	initGraphics(320, 200);
	_screen = new Graphics::Screen();
	scr_live.data = (byte *)_screen->getPixels();

	// Create a debugger console
	setDebugger(new Console());

	// Set up sound manager
	_soundManager = nullptr; // new ForestSoundManager(_mixer, _soundFlag);
	//_soundManager->validate();

	// Run the game
	// TODO

	return Common::kNoError;
}

void ForestEngine::global_init_code() {

}

void ForestEngine::section_music(int section_num) {

}

void ForestEngine::global_object_sprite() {

}

void ForestEngine::stop_walker_basic() {
	int random;
	int count;
	int how_many;

	random = imath_random(1, 30000);

	switch (player.facing) {
	case FACING_SOUTH:
		if (random < 500) {
			how_many = imath_random(4, 10);
			for (count = 0; count < how_many; count++) {
				player_add_stop_walker((random < 250) ? 1 : 2, 0);
			}
		} else if (random < 750) {
			for (count = 0; count < 4; count++) {
				player_add_stop_walker(1, 0);
			}

			player_add_stop_walker(0, 0);

			for (count = 0; count < 4; count++) {
				player_add_stop_walker(2, 0);
			}

			player_add_stop_walker(0, 0);
		}
		break;

	case FACING_SOUTHEAST:
	case FACING_SOUTHWEST:
	case FACING_NORTHEAST:
	case FACING_NORTHWEST:
		if (random < 150) {
			player_add_stop_walker(-1, 0);
			player_add_stop_walker(1, 0);
			for (count = 0; count < 6; count++) {
				player_add_stop_walker(0, 0);
			}
		}
		break;

	case FACING_EAST:
	case FACING_WEST:
		if (random < 250) {
			player_add_stop_walker(-1, 0);
			how_many = imath_random(2, 6);
			for (count = 0; count < how_many; count++) {
				player_add_stop_walker(2, 0);
			}
			player_add_stop_walker(1, 0);
			player_add_stop_walker(0, 0);
			player_add_stop_walker(0, 0);
		} else if (random < 500) {
			WRITE_LE_UINT32(&global[walker_timing], kernel.clock);
		}
		break;

	case FACING_NORTH:
		if (random < 250) {
			player_add_stop_walker(-1, 0);
			how_many = imath_random(3, 7);
			for (count = 0; count < how_many; count++) {
				player_add_stop_walker(2, 0);
			}
			player_add_stop_walker(1, 0);
			player_add_stop_walker(0, 0);
		}
		break;

	}
}

void ForestEngine::stop_walker_tricks() {

}

void ForestEngine::global_section_constructor() {

}

void ForestEngine::syncRoom(Common::Serializer &s) {

}

void ForestEngine::global_daemon_code() {

}

void ForestEngine::global_pre_parser_code() {

}

void ForestEngine::global_parser_code() {

}

void ForestEngine::global_error_code() {

}

void ForestEngine::global_room_init() {

}

void ForestEngine::global_sound_driver() {

}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
