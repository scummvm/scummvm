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

#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/RunTime.h"

namespace QDEngine {

qdMiniGameInterface *open_game_interface(const char* name) {
	dprintf("open_game_interface: %s, runtime%s\n", name, runtime ? "!=0" : "==0");

	if (!runtime)
		return runtime = new MinigameManager;

	return new MinigameManager;
}

bool close_game_interface(qdMiniGameInterface* game) {
	dprintf("close_game_interface, runtime%s%s\n", runtime == game ? "==game" : "!=game", runtime ? "!=0" : "==0");

	delete game;
	if (game == runtime)
		runtime = 0;

	return true;
}

} // namespace QDEngine
