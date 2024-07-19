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

//=============================================================================
//
// This unit provides game initialization routine, which takes place after
// main game file was successfully loaded.
//
//=============================================================================

#ifndef AGS_ENGINE_GAME_GAME_INIT_H
#define AGS_ENGINE_GAME_GAME_INIT_H

#include "ags/shared/game/main_game_file.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

using namespace Shared;

// Error codes for initializing the game
enum GameInitErrorType {
	kGameInitErr_NoError,
	// currently AGS requires at least one font to be present in game
	kGameInitErr_NoFonts,
	kGameInitErr_TooManyAudioTypes,
	kGameInitErr_EntityInitFail,
	kGameInitErr_PluginNameInvalid,
	kGameInitErr_NoGlobalScript,
	kGameInitErr_ScriptLinkFailed
};

String GetGameInitErrorText(GameInitErrorType err);

typedef TypedCodeError<GameInitErrorType, GetGameInitErrorText> GameInitError;
typedef ErrorHandle<GameInitError> HGameInitError;

// Sets up game state for play using preloaded data
HGameInitError  InitGameState(const LoadedGameEntities &ents, GameDataVersion data_ver);

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
