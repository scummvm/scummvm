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

#ifndef MM1_DATA_GAME_STATE_H
#define MM1_DATA_GAME_STATE_H

#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "mm/mm1/data/character.h"
#include "mm/mm1/data/party.h"
#include "mm/mm1/data/active_spells.h"
#include "mm/mm1/data/spells_state.h"

namespace MM {
namespace MM1 {

/**
 * This acts as a container for everything in the game
 * that is persisted to savegames
 */
struct GameState {
	Party _party;
	ActiveSpells _activeSpells;
	SpellsState _spellsState;

	GameState();
};

} // namespace MM1
} // namespace MM

#endif
