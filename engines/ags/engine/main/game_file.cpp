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

//
// Game data file management
//

#include "ags/engine/main/mainheader.h"
#include "ags/engine/main/game_file.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/charactercache.h"
#include "ags/shared/ac/dialogtopic.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/shared/ac/gamestructdefines.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/viewframe.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/gui/guilabel.h"
#include "ags/engine/main/main.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/blender.h"
#include "ags/shared/core/assetmanager.h"
#include "ags/shared/util/alignedstream.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/shared/game/main_game_file.h"
#include "ags/engine/game/game_init.h"
#include "ags/engine/plugin/agsplugin.h"
#include "ags/engine/script/script.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern int ifacepopped;

extern GameSetupStruct game;
extern ViewStruct *views;
extern DialogTopic *dialog;

extern AGSPlatformDriver *platform;
extern int numScriptModules;


// Test if engine supports extended capabilities required to run the game
bool test_game_caps(const std::set<String> &caps, std::set<String> &failed_caps) {
	// Currently we support nothing special
	failed_caps = caps;
	return caps.empty();
}

// Forms a simple list of capability names
String get_caps_list(const std::set<String> &caps) {
	String caps_list;
	for (std::set<String>::const_iterator it = caps.begin(); it != caps.end(); ++it) {
		caps_list.Append("\n\t");
		caps_list.Append(*it);
	}
	return caps_list;
}

// Called when the game file is opened for the first time (when preloading game data);
// it logs information on data version and reports first found errors, if any.
HGameFileError game_file_first_open(MainGameSource &src) {
	HGameFileError err = OpenMainGameFileFromDefaultAsset(src);
	if (err ||
		err->Code() == kMGFErr_SignatureFailed ||
		err->Code() == kMGFErr_FormatVersionTooOld ||
		err->Code() == kMGFErr_FormatVersionNotSupported) {
		// Log data description for debugging
		Debug::Printf(kDbgMsg_Info, "Opened game data file: %s", src.Filename.GetCStr());
		Debug::Printf(kDbgMsg_Info, "Game data version: %d", src.DataVersion);
		Debug::Printf(kDbgMsg_Info, "Compiled with: %s", src.CompiledWith.GetCStr());
		if (!src.Caps.empty()) {
			String caps_list = get_caps_list(src.Caps);
			Debug::Printf(kDbgMsg_Info, "Requested engine caps: %s", caps_list.GetCStr());
		}
	}
	// Quit in case of error
	if (!err)
		return err;

	// Test the extended caps
	std::set<String> failed_caps;
	if (!test_game_caps(src.Caps, failed_caps)) {
		String caps_list = get_caps_list(failed_caps);
		return new MainGameFileError(kMGFErr_CapsNotSupported, String::FromFormat("Missing engine caps: %s", caps_list.GetCStr()));
	}
	return HGameFileError::None();
}

void PreReadSaveFileInfo(Stream *in, GameDataVersion data_ver) {
	AlignedStream align_s(in, Shared::kAligned_Read);
	game.ReadFromFile(&align_s);
	// Discard game messages we do not need here
	delete[] game.load_messages;
	game.load_messages = nullptr;
	game.read_savegame_info(in, data_ver);
}

HError preload_game_data() {
	MainGameSource src;
	HGameFileError err = game_file_first_open(src);
	if (!err)
		return (HError)err;
	// Read only the particular data we need for preliminary game analysis
	PreReadSaveFileInfo(src.InputStream.get(), src.DataVersion);
	game.compiled_with = src.CompiledWith;
	FixupSaveDirectory(game);
	return HError::None();
}

HError load_game_file() {
	MainGameSource src;
	LoadedGameEntities ents(game, dialog, views);
	HGameFileError load_err = OpenMainGameFileFromDefaultAsset(src);
	if (load_err) {
		load_err = ReadGameData(ents, src.InputStream.get(), src.DataVersion);
		if (load_err)
			load_err = UpdateGameData(ents, src.DataVersion);
	}
	if (!load_err)
		return (HError)load_err;
	HGameInitError init_err = InitGameState(ents, src.DataVersion);
	if (!init_err)
		return (HError)init_err;
	return HError::None();
}

void display_game_file_error(HError err) {
	platform->DisplayAlert("Loading game failed with error:\n%s.\n\nThe game files may be incomplete, corrupt or from unsupported version of AGS.",
		err->FullMessage().GetCStr());
}

} // namespace AGS3
