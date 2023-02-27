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
// This unit provides functions for reading main game file into appropriate
// data structures. Main game file contains general game data, such as global
// options, lists of static game entities and compiled scripts modules.
//
//=============================================================================

#ifndef AGS_SHARED_GAME_MAIN_GAME_FILE_H
#define AGS_SHARED_GAME_MAIN_GAME_FILE_H

#include "ags/lib/std/functional.h"
#include "ags/lib/std/memory.h"
#include "ags/lib/std/set.h"
#include "ags/lib/std/vector.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/ac/game_version.h"
#include "ags/shared/ac/view.h"
#include "ags/shared/game/plugin_info.h"
#include "ags/shared/script/cc_script.h"
#include "ags/shared/util/error.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/version.h"

namespace AGS3 {

struct GameSetupStruct;
struct DialogTopic;

namespace AGS {
namespace Shared {

// Error codes for main game file reading
enum MainGameFileErrorType {
	kMGFErr_NoError,
	kMGFErr_FileOpenFailed,
	kMGFErr_SignatureFailed,
	// separate error given for "too old" format to provide clarifying message
	kMGFErr_FormatVersionTooOld,
	kMGFErr_FormatVersionNotSupported,
	kMGFErr_CapsNotSupported,
	kMGFErr_InvalidNativeResolution,
	kMGFErr_TooManySprites,
	kMGFErr_InvalidPropertySchema,
	kMGFErr_InvalidPropertyValues,
	kMGFErr_CreateGlobalScriptFailed,
	kMGFErr_CreateDialogScriptFailed,
	kMGFErr_CreateScriptModuleFailed,
	kMGFErr_GameEntityFailed,
	kMGFErr_PluginDataFmtNotSupported,
	kMGFErr_PluginDataSizeTooLarge,
	kMGFErr_ExtListFailed,
	kMGFErr_ExtUnknown
};

String GetMainGameFileErrorText(MainGameFileErrorType err);

typedef TypedCodeError<MainGameFileErrorType, GetMainGameFileErrorText> MainGameFileError;
typedef ErrorHandle<MainGameFileError> HGameFileError;
typedef std::unique_ptr<Stream> UStream;

// MainGameSource defines a successfully opened main game file
struct MainGameSource {
	// Standart main game file names for 3.* and 2.* games respectively
	static const char *DefaultFilename_v3;
	static const char *DefaultFilename_v2;
	// Signature of the current game format
	static const char *Signature;

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
	UStream             InputStream;

	MainGameSource();
};

// LoadedGameEntities is meant for keeping objects loaded from the game file.
// Because copying/assignment methods are not properly implemented for some
// of these objects yet, they have to be attached using references to be read
// directly. This is temporary solution that has to be resolved by the future
// code refactoring.
struct LoadedGameEntities {
	GameSetupStruct &Game;
	std::vector<DialogTopic> Dialogs;
	std::vector<ViewStruct> Views;
	PScript                 GlobalScript;
	PScript                 DialogScript;
	std::vector<PScript>    ScriptModules;
	std::vector<PluginInfo> PluginInfos;

	// Original sprite data (when it was read into const-sized arrays)
	size_t                  SpriteCount;
	std::vector<uint8_t>	SpriteFlags; // SPF_* flags

	// Old dialog support
	// legacy compiled dialog script of its own format,
	// requires separate interpreting
	std::vector<std::vector<uint8_t>> OldDialogScripts;
	// probably, actual dialog script sources kept within some older games
	std::vector<String>     OldDialogSources;
	// speech texts displayed during dialog
	std::vector<String>     OldSpeechLines;

	LoadedGameEntities(GameSetupStruct &game);
	~LoadedGameEntities();
};

class AssetManager;

// Tells if the given path (library filename) contains main game file
bool               IsMainGameLibrary(const String &filename);
// Scans given directory path for a package containing main game data, returns first found or none.
String             FindGameData(const String &path);
String             FindGameData(const String &path, bool(*fn_testfile)(const String &));
// Opens main game file for reading from an arbitrary file
HGameFileError     OpenMainGameFile(const String &filename, MainGameSource &src);
// Opens main game file for reading using the current Asset Manager (uses default asset name)
HGameFileError     OpenMainGameFileFromDefaultAsset(MainGameSource &src, AssetManager *mgr);
// Reads game data, applies necessary conversions to match current format version
HGameFileError     ReadGameData(LoadedGameEntities &ents, Stream *in, GameDataVersion data_ver);
// Pre-reads the heading game data, just enough to identify the game and its special file locations
void               PreReadGameData(GameSetupStruct &game, Stream *in, GameDataVersion data_ver);
// Applies necessary updates, conversions and fixups to the loaded data
// making it compatible with current engine
HGameFileError     UpdateGameData(LoadedGameEntities &ents, GameDataVersion data_ver);
// Ensures that the game saves directory path is valid
void               FixupSaveDirectory(GameSetupStruct &game);
// Maps legacy sound numbers to real audio clips
void               RemapLegacySoundNums(GameSetupStruct &game, std::vector<ViewStruct> &views, GameDataVersion data_ver);

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
