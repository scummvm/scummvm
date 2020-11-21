//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// This unit provides game initialization routine, which takes place after
// main game file was successfully loaded.
//
//=============================================================================

#ifndef __AGS_EE_GAME__GAMEINIT_H
#define __AGS_EE_GAME__GAMEINIT_H

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

#endif // __AGS_EE_GAME__GAMEINIT_H
