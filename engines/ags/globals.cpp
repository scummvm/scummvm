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

#include "ags/globals.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/engine/ac/gamestate.h"

namespace AGS3 {

Globals *g_globals;

Globals::Globals() {
	g_globals = this;

	Common::fill(&_mousecurs[0], &_mousecurs[MAXCURSORS], nullptr);

	_play = new GameState();
	_game = new GameSetupStruct();
	_spriteset = new SpriteCache(_game->SpriteInfos);
}

Globals::~Globals() {
	g_globals = nullptr;
	delete _game;
	delete _play;
	delete _spriteset;
}

} // namespace AGS3
