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

#include "common/debug.h"

#include "qdengine/qdengine.h"
#include "qdengine/minigames/adv/common.h"
#include "qdengine/minigames/adv/RunTime.h"

namespace QDEngine {

qdMiniGameInterface *create_adv_minigame(const char *name, MinigameConsCallback callback) {
	debugC(3, kDebugMinigames, "open_game_interface: %s, runtime%s", name, g_runtime ? "!=0" : "==0");

	if (!g_runtime)
		return g_runtime = new MinigameManager(callback);

	return new MinigameManager(callback);
}

bool close_adv_minigame(qdMiniGameInterface *game) {
	debugC(3, kDebugMinigames, "close_game_interface, runtime%s%s", g_runtime == game ? "==game" : "!=game", g_runtime ? "!=0" : "==0");

	delete game;
	if (game == g_runtime)
		g_runtime = 0;

	return true;
}

} // namespace QDEngine
