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

#include "tetraedge/game/game_sound.h"
#include "tetraedge/game/syberia_game.h"
#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_lua_thread.h"

namespace Tetraedge {

GameSound::GameSound() {
}

bool GameSound::onSoundStopped() {
	SyberiaGame *game = dynamic_cast<SyberiaGame *>(g_engine->getGame());
	if (!game || !game->luaContext().isCreated())
		return false;

	Common::Array<SyberiaGame::YieldedCallback> &callbacks = game->yieldedCallbacks();
	for (uint i = 0; i < callbacks.size(); i++) {
		if (callbacks[i]._luaFnName == "OnFreeSoundFinished" && callbacks[i]._luaParam == _name) {
			TeLuaThread *thread = callbacks[i]._luaThread;
			callbacks.remove_at(i);
			if (thread) {
				thread->resume();
				return false;
			}
			break;
		}
	}
	game->luaScript().execute("OnFreeSoundFinished", _name);
	game->luaScript().execute("OnCellFreeSoundFinished", _name);
	return false;
}


} // end namespace Tetraedge
