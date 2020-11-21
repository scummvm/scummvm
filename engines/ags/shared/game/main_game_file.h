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
// This unit provides functions for reading main game file into appropriate
// data structures. Main game file contains general game data, such as global
// options, lists of static game entities and compiled scripts modules.
//
//=============================================================================

#ifndef __AGS_CN_GAME__MAINGAMEFILE_H
#define __AGS_CN_GAME__MAINGAMEFILE_H

#include <memory>
#include <set>
#include <vector>
#include "ac/game_version.h"
#include "game/plugininfo.h"
#include "script/cc_script.h"
#include "util/error.h"
#include "util/stream.h"
#include "util/string.h"
#include "util/version.h"

struct GameSetupStruct;
struct DialogTopic;
struct ViewStruct;

namespace AGS
{
namespace Common
{

// Error codes for main game file reading
enum MainGameFileErrorType
{
    kMGFErr_NoError,
    kMGFErr_FileOpenFailed,
    kMGFErr_SignatureFailed,
    // separate error given for "too old" format to provide clarifying message
    kMGFErr_FormatVersionTooOld,
    kMGFErr_FormatVersionNotSupported,
    kMGFErr_CapsNotSupported,
    kMGFErr_InvalidNativeResolution,
    kMGFErr_TooManySprites,
    kMGFErr_TooManyCursors,
    kMGFErr_InvalidPropertySchema,
    kMGFErr_InvalidPropertyValues,
    kMGFErr_NoGlobalScript,
    kMGFErr_CreateGlobalScriptFailed,
    kMGFErr_CreateDialogScriptFailed,
    kMGFErr_CreateScriptModuleFailed,
    kMGFErr_GameEntityFailed,
    kMGFErr_PluginDataFmtNotSupported,
    kMGFErr_PluginDataSizeTooLarge
};

String GetMainGameFileErrorText(MainGameFileErrorType err);

typedef TypedCodeError<MainGameFileErrorType, GetMainGameFileErrorText> MainGameFileError;
typedef ErrorHandle<MainGameFileError> HGameFileError;
typedef std::shared_ptr<Stream> PStream;

// MainGameSource defines a successfully opened main game file
struct MainGameSource
{
    // Standart main game file names for 3.* and 2.* games respectively
    static const String DefaultFilename_v3;
    static const String DefaultFilename_v2;
    // Signature of the current game format
    static const String Signature;

    // Name of the asset file
    String              Filename;
    // Game file format version
    GameDataVersion     DataVersion;
    // Tool identifier (like version) this game was compiled with
    String              CompiledWith;
    // Extended engine capabilities required by the game; their primary use
    // currently is to let "alternate" game formats indicate themselves
    std::set<String>    Caps;
    // A ponter to the opened stream
    PStream             InputStream;

    MainGameSource();
};

// LoadedGameEntities is meant for keeping objects loaded from the game file.
// Because copying/assignment methods are not properly implemented for some
// of these objects yet, they have to be attached using references to be read
// directly. This is temporary solution that has to be resolved by the future
// code refactoring.
struct LoadedGameEntities
{
    GameSetupStruct        &Game;
    DialogTopic           *&Dialogs;
    ViewStruct            *&Views;
    PScript                 GlobalScript;
    PScript                 DialogScript;
    std::vector<PScript>    ScriptModules;
    std::vector<PluginInfo> PluginInfos;

    // Original sprite data (when it was read into const-sized arrays)
    size_t                  SpriteCount;
    std::unique_ptr<char[]> SpriteFlags;

    // Old dialog support
    // legacy compiled dialog script of its own format,
    // requires separate interpreting
    std::vector< std::shared_ptr<unsigned char> > OldDialogScripts;
    // probably, actual dialog script sources kept within some older games
    std::vector<String>     OldDialogSources;
    // speech texts displayed during dialog
    std::vector<String>     OldSpeechLines;

    LoadedGameEntities(GameSetupStruct &game, DialogTopic *&dialogs, ViewStruct *&views);
    ~LoadedGameEntities();
};

// Tells if the given path (library filename) contains main game file
bool               IsMainGameLibrary(const String &filename);
// Opens main game file for reading from an arbitrary file
HGameFileError     OpenMainGameFile(const String &filename, MainGameSource &src);
// Opens main game file for reading from the asset library (uses default asset name)
HGameFileError     OpenMainGameFileFromDefaultAsset(MainGameSource &src);
// Reads game data, applies necessary conversions to match current format version
HGameFileError     ReadGameData(LoadedGameEntities &ents, Stream *in, GameDataVersion data_ver);
// Applies necessary updates, conversions and fixups to the loaded data
// making it compatible with current engine
HGameFileError     UpdateGameData(LoadedGameEntities &ents, GameDataVersion data_ver);
// Ensures that the game saves directory path is valid
void               FixupSaveDirectory(GameSetupStruct &game);

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_GAME__MAINGAMEFILE_H
