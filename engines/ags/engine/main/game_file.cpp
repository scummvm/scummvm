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

//
// Game data file management
//

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/shared/ac/dialog_topic.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/game/game_init.h"
#include "ags/shared/game/main_game_file.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/blender.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/engine/main/main.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/script/cc_common.h"
#include "ags/engine/script/script.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/text_stream_reader.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

// Test if engine supports extended capabilities required to run the game
bool test_game_caps(const std::set<String> &caps, std::set<String> &failed_caps) {
	// Currently we support nothing special
	failed_caps = caps;
	return caps.size() == 0;
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
	HGameFileError err = OpenMainGameFileFromDefaultAsset(src, _G(AssetMgr)->get());
	if (err ||
	        err->Code() == kMGFErr_SignatureFailed ||
	        err->Code() == kMGFErr_FormatVersionTooOld ||
	        err->Code() == kMGFErr_FormatVersionNotSupported) {
		// Log data description for debugging
		Debug::Printf(kDbgMsg_Info, "Opened game data file: %s", src.Filename.GetCStr());
		Debug::Printf(kDbgMsg_Info, "Game data version: %d", src.DataVersion);
		Debug::Printf(kDbgMsg_Info, "Compiled with: %s", src.CompiledWith.GetCStr());
		if (src.Caps.size() > 0) {
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

HError preload_game_data() {
	MainGameSource src;
	HGameFileError err = game_file_first_open(src);
	if (!err)
		return (HError)err;
	// Read only the particular data we need for preliminary game analysis
	PreReadGameData(_GP(game), src.InputStream.get(), src.DataVersion);
	_GP(game).compiled_with = src.CompiledWith;
	FixupSaveDirectory(_GP(game));
	return HError::None();
}

static inline HError MakeScriptLoadError(const char *name) {
	return new Error(String::FromFormat(
		"Failed to load a script module: %s", name),
		cc_get_error().ErrorString);
}

// Looks up for the game scripts available as separate assets.
// These are optional, so no error is raised if some of these are not found.
// For those that do exist, reads them and replaces any scripts of same kind
// in the already loaded game data.
HError LoadGameScripts(LoadedGameEntities &ents) {
	// Global script
	std::unique_ptr<Stream> in(_GP(AssetMgr)->OpenAsset("GlobalScript.o"));
	if (in) {
		PScript script(ccScript::CreateFromStream(in.get()));
		if (!script)
			return MakeScriptLoadError("GlobalScript.o");
		ents.GlobalScript = script;
	}
	// Dialog script
	in.reset(_GP(AssetMgr)->OpenAsset("DialogScript.o"));
	if (in) {
		PScript script(ccScript::CreateFromStream(in.get()));
		if (!script)
			return MakeScriptLoadError("DialogScript.o");
		ents.DialogScript = script;
	}
	// Script modules
	// First load a modules list
	std::vector<String> modules;
	in.reset(_GP(AssetMgr)->OpenAsset("ScriptModules.lst"));
	if (in) {
		TextStreamReader reader(in.get());
		in.release(); // TextStreamReader got it
		while (!reader.EOS())
			modules.push_back(reader.ReadLine());
	}
	if (modules.size() > ents.ScriptModules.size())
		ents.ScriptModules.resize(modules.size());
	// Now run by the list and try loading everything
	for (size_t i = 0; i < modules.size(); ++i) {
		in.reset(_GP(AssetMgr)->OpenAsset(modules[i]));
		if (in) {
			PScript script(ccScript::CreateFromStream(in.get()));
			if (!script)
				return MakeScriptLoadError(modules[i].GetCStr());
			ents.ScriptModules[i] = script;
		}
	}
	return HError::None();
}

HError load_game_file() {
	MainGameSource src;
	LoadedGameEntities ents(_GP(game));
	HError err = (HError)OpenMainGameFileFromDefaultAsset(src, _GP(AssetMgr).get());
	if (!err)
		return err;

	err = (HError)ReadGameData(ents, src.InputStream.get(), src.DataVersion);
	if (!err)
		return err;
	src.InputStream.reset();

	//-------------------------------------------------------------------------
	// Data overrides: for compatibility mode and custom engine support
	// NOTE: this must be done before UpdateGameData, or certain adjustments
	// won't be applied correctly.

	// Custom engine detection (ugly hack, depends on the known game GUIDs)
	if (strcmp(_GP(game).guid, "{d6795d1c-3cfe-49ec-90a1-85c313bfccaf}" /* Kathy Rain */ ) == 0 ||
		strcmp(_GP(game).guid, "{5833654f-6f0d-40d9-99e2-65c101c8544a}" /* Whispers of a Machine */ ) == 0)
	{
		_GP(game).options[OPT_CUSTOMENGINETAG] = CUSTOMENG_CLIFFTOP;
	}
	// Upscale mode -- for old games that supported it.
	if ((_G(loaded_game_file_version) < kGameVersion_310) && _GP(usetup).override_upscale) {
		if (_GP(game).GetResolutionType() == kGameResolution_320x200 || _GP(game).GetResolutionType() == kGameResolution_Default)
			_GP(game).SetGameResolution(kGameResolution_640x400);
		else if (_GP(game).GetResolutionType() == kGameResolution_320x240)
			_GP(game).SetGameResolution(kGameResolution_640x480);
	}
	if (_GP(game).options[OPT_CUSTOMENGINETAG] == CUSTOMENG_CLIFFTOP) {
		if (_GP(game).GetResolutionType() == kGameResolution_640x400)
			_GP(game).SetGameResolution(Size(640, 360));
	}

	err = (HError)UpdateGameData(ents, src.DataVersion);
	if (!err)
		return err;
	err = LoadGameScripts(ents);
	if (!err)
		return err;
	err = (HError)InitGameState(ents, src.DataVersion);
	if (!err)
		return err;
	return HError::None();
}

void display_game_file_error(HError err) {
	_G(platform)->DisplayAlert("Loading game failed with error:\n%s.\n\nThe game files may be incomplete, corrupt or from unsupported version of AGS.",
	                           err->FullMessage().GetCStr());
}

} // namespace AGS3
