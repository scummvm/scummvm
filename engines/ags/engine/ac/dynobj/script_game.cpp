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

#include "ags/engine/ac/dynobj/script_game.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_state.h"


namespace AGS3 {

void StaticGame::WriteInt32(const char *address, intptr_t offset, int32_t val) {
	if (offset == 4 * sizeof(int32_t)) { // game.debug_mode
		set_debug_mode(val != 0);
	} else if (offset == 99 * sizeof(int32_t) || offset == 112 * sizeof(int32_t)) { // game.text_align, game.speech_text_align
		*(int32_t *)(address + offset) = ReadScriptAlignment(val);
	} else {
		*(int32_t *)(address + offset) = val;
	}
}

} // namespace AGS3
