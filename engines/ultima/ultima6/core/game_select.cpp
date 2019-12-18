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

#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/ultima6/conf/configuration.h"
#include "ultima/ultima6/misc/u6_misc.h"
#include "ultima/ultima6/core/game_select.h"
#include "ultima/ultima6/core/console.h"

namespace Ultima {
namespace Ultima6 {

GameSelect::GameSelect(Configuration *cfg) {
	config = cfg;
	screen = NULL;
	game = NULL;
}

GameSelect::~GameSelect() {

}

uint8 GameSelect::load(Screen *s, uint8 game_type) {
	std::string cfg_game_string;

	screen = s;

	if (game_type == NUVIE_GAME_NONE) { // if game_type not specified on cmdline
		config->value("config/loadgame", cfg_game_string, ""); // attempt to get game_type_string from config
		game_type = get_game_type(cfg_game_string.c_str());

		if (game_type == NUVIE_GAME_NONE)
			game_type = NUVIE_GAME_U6; // FIX we should select game from a menu here.
	}

	return game_type;
}

} // End of namespace Ultima6
} // End of namespace Ultima
