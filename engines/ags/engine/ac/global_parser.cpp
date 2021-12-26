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

#include "ags/engine/ac/global_parser.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/string.h"
#include "ags/globals.h"

namespace AGS3 {

int SaidUnknownWord(char *buffer) {
	VALIDATE_STRING(buffer);
	strcpy(buffer, _GP(play).bad_parsed_word);
	if (_GP(play).bad_parsed_word[0] == 0)
		return 0;
	return 1;
}

} // namespace AGS3
