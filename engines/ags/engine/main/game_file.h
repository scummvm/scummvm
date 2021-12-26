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

#ifndef AGS_ENGINE_MAIN_GAME_FILE_H
#define AGS_ENGINE_MAIN_GAME_FILE_H

#include "ags/shared/util/error.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

using AGS::Shared::HError;

// Preload particular game-describing parameters from the game data header (title, save game dir name, etc)
HError preload_game_data();
// Loads game data and reinitializes the game state; assigns error message in case of failure
HError load_game_file();
void display_game_file_error(HError err);

} // namespace AGS3

#endif
