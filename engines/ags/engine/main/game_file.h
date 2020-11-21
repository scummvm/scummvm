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

#ifndef AGS_ENGINE_MAIN_GAMEFILE_H
#define AGS_ENGINE_MAIN_GAMEFILE_H

#include "util/error.h"
#include "util/string.h"

namespace AGS3 {

using AGS::Common::HError;

// Preload particular game-describing parameters from the game data header (title, save game dir name, etc)
HError preload_game_data();
// Loads game data and reinitializes the game state; assigns error message in case of failure
HError load_game_file();
void display_game_file_error(HError err);

} // namespace AGS3

#endif
