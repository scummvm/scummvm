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

//=============================================================================
//
// This unit provides game initialization routine, which takes place after
// main game file was successfully loaded.
//
//=============================================================================

#ifndef AGS_ENGINE_GAME_GAMEINIT_H
#define AGS_ENGINE_GAME_GAMEINIT_H

#include "game/main_game_file.h"
#include "util/string.h"

namespace AGS
{
namespace Engine
{

using namespace Common;

// Error codes for initializing the game
enum GameInitErrorType
{
    kGameInitErr_NoError,
    // currently AGS requires at least one font to be present in game
    kGameInitErr_NoFonts,
    kGameInitErr_TooManyAudioTypes,
    kGameInitErr_EntityInitFail,
    kGameInitErr_TooManyPlugins,
    kGameInitErr_PluginNameInvalid,
    kGameInitErr_ScriptLinkFailed
};

String GetGameInitErrorText(GameInitErrorType err);

typedef TypedCodeError<GameInitErrorType, GetGameInitErrorText> GameInitError;
typedef ErrorHandle<GameInitError> HGameInitError;

// Sets up game state for play using preloaded data
HGameInitError  InitGameState(const LoadedGameEntities &ents, GameDataVersion data_ver);

} // namespace Engine
} // namespace AGS

#endif
