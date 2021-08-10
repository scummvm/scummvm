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

#include "ags/engine/ac/character.h"
#include "ags/engine/ac/character_cache.h"
#include "ags/engine/ac/dialog.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/file.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/lip_sync.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/dynobj/all_dynamic_classes.h"
#include "ags/engine/ac/dynobj/all_script_classes.h"
#include "ags/engine/ac/statobj/ags_static_object.h"
#include "ags/engine/ac/statobj/static_array.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/font/ags_font_renderer.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/game/game_init.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/shared/script/cc_error.h"
#include "ags/engine/script/exports.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/util/string_utils.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace Shared;
using namespace Engine;

namespace AGS {
namespace Engine {

String GetGameInitErrorText(GameInitErrorType err) {
	switch (err) {
	case kGameInitErr_NoError:
		return "No error.";
	case kGameInitErr_NoFonts:
		return "No fonts specified to be used in this _GP(game).";
	case kGameInitErr_TooManyAudioTypes:
		return "Too many audio types for this engine to handle.";
	case kGameInitErr_EntityInitFail:
		return "Failed to initialize game entities.";
	case kGameInitErr_TooManyPlugins:
		return "Too many plugins for this engine to handle.";
	case kGameInitErr_PluginNameInvalid:
		return "Plugin name is invalid.";
	case kGameInitErr_NoGlobalScript:
		return "No global script in game.";
	case kGameInitErr_ScriptLinkFailed:
		return "Script link failed.";
	}
	return "Unknown error.";
}

// Initializes audio channels and clips and registers them in the script system
void InitAndRegisterAudioObjects() {
	for (int i = 0; i <= MAX_SOUND_CHANNELS; ++i) {
		_G(scrAudioChannel)[i].id = i;
		ccRegisterManagedObject(&_G(scrAudioChannel)[i], &_GP(ccDynamicAudio));
	}

	for (size_t i = 0; i < _GP(game).audioClips.size(); ++i) {
		// Note that as of 3.5.0 data format the clip IDs are still restricted
		// to actual item index in array, so we don't make any difference
		// between game versions, for now.
		_GP(game).audioClips[i].id = i;
		ccRegisterManagedObject(&_GP(game).audioClips[i], &_GP(ccDynamicAudioClip));
		ccAddExternalDynamicObject(_GP(game).audioClips[i].scriptName, &_GP(game).audioClips[i], &_GP(ccDynamicAudioClip));
	}
}

// Initializes characters and registers them in the script system
void InitAndRegisterCharacters() {
	_GP(characterScriptObjNames).resize(_GP(game).numcharacters);
	for (int i = 0; i < _GP(game).numcharacters; ++i) {
		_GP(game).chars[i].walking = 0;
		_GP(game).chars[i].animating = 0;
		_GP(game).chars[i].pic_xoffs = 0;
		_GP(game).chars[i].pic_yoffs = 0;
		_GP(game).chars[i].blinkinterval = 140;
		_GP(game).chars[i].blinktimer = _GP(game).chars[i].blinkinterval;
		_GP(game).chars[i].index_id = i;
		_GP(game).chars[i].blocking_width = 0;
		_GP(game).chars[i].blocking_height = 0;
		_GP(game).chars[i].prevroom = -1;
		_GP(game).chars[i].loop = 0;
		_GP(game).chars[i].frame = 0;
		_GP(game).chars[i].walkwait = -1;
		ccRegisterManagedObject(&_GP(game).chars[i], &_GP(ccDynamicCharacter));

		// export the character's script object
		_GP(characterScriptObjNames)[i] = _GP(game).chars[i].scrname;
		ccAddExternalDynamicObject(_GP(characterScriptObjNames)[i], &_GP(game).chars[i], &_GP(ccDynamicCharacter));
	}
}

// Initializes dialog and registers them in the script system
void InitAndRegisterDialogs() {
	_G(scrDialog) = new ScriptDialog[_GP(game).numdialog];
	for (int i = 0; i < _GP(game).numdialog; ++i) {
		_G(scrDialog)[i].id = i;
		_G(scrDialog)[i].reserved = 0;
		ccRegisterManagedObject(&_G(scrDialog)[i], &_GP(ccDynamicDialog));

		if (!_GP(game).dialogScriptNames[i].IsEmpty())
			ccAddExternalDynamicObject(_GP(game).dialogScriptNames[i], &_G(scrDialog)[i], &_GP(ccDynamicDialog));
	}
}

// Initializes dialog options rendering objects and registers them in the script system
void InitAndRegisterDialogOptions() {
	ccRegisterManagedObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));

	_G(dialogOptionsRenderingSurface) = new ScriptDrawingSurface();
	_G(dialogOptionsRenderingSurface)->isLinkedBitmapOnly = true;
	long dorsHandle = ccRegisterManagedObject(_G(dialogOptionsRenderingSurface), _G(dialogOptionsRenderingSurface));
	ccAddObjectReference(dorsHandle);
}

// Initializes gui and registers them in the script system
HError InitAndRegisterGUI() {
	_G(scrGui) = (ScriptGUI *)malloc(sizeof(ScriptGUI) * _GP(game).numgui);
	for (int i = 0; i < _GP(game).numgui; ++i) {
		_G(scrGui)[i].id = -1;
	}

	_GP(guiScriptObjNames).resize(_GP(game).numgui);
	for (int i = 0; i < _GP(game).numgui; ++i) {
		// link controls to their parent guis
		HError err = _GP(guis)[i].RebuildArray();
		if (!err)
			return err;
		// export all the GUI's controls
		export_gui_controls(i);
		// copy the script name to its own memory location
		// because ccAddExtSymbol only keeps a reference
		_GP(guiScriptObjNames)[i] = _GP(guis)[i].Name;
		_G(scrGui)[i].id = i;
		ccAddExternalDynamicObject(_GP(guiScriptObjNames)[i], &_G(scrGui)[i], &_GP(ccDynamicGUI));
		ccRegisterManagedObject(&_G(scrGui)[i], &_GP(ccDynamicGUI));
	}
	return HError::None();
}

// Initializes inventory items and registers them in the script system
void InitAndRegisterInvItems() {
	for (int i = 0; i < MAX_INV; ++i) {
		_G(scrInv)[i].id = i;
		_G(scrInv)[i].reserved = 0;
		ccRegisterManagedObject(&_G(scrInv)[i], &_GP(ccDynamicInv));

		if (!_GP(game).invScriptNames[i].IsEmpty())
			ccAddExternalDynamicObject(_GP(game).invScriptNames[i], &_G(scrInv)[i], &_GP(ccDynamicInv));
	}
}

// Initializes room hotspots and registers them in the script system
void InitAndRegisterHotspots() {
	for (int i = 0; i < MAX_ROOM_HOTSPOTS; ++i) {
		_G(scrHotspot)[i].id = i;
		_G(scrHotspot)[i].reserved = 0;
		ccRegisterManagedObject(&_G(scrHotspot)[i], &_GP(ccDynamicHotspot));
	}
}

// Initializes room objects and registers them in the script system
void InitAndRegisterRoomObjects() {
	for (int i = 0; i < MAX_ROOM_OBJECTS; ++i) {
		ccRegisterManagedObject(&_G(scrObj)[i], &_GP(ccDynamicObject));
	}
}

// Initializes room regions and registers them in the script system
void InitAndRegisterRegions() {
	for (int i = 0; i < MAX_ROOM_REGIONS; ++i) {
		_G(scrRegion)[i].id = i;
		_G(scrRegion)[i].reserved = 0;
		ccRegisterManagedObject(&_G(scrRegion)[i], &_GP(ccDynamicRegion));
	}
}

// Registers static entity arrays in the script system
void RegisterStaticArrays() {
	_GP(StaticCharacterArray).Create(&_GP(ccDynamicCharacter), sizeof(CharacterInfo), sizeof(CharacterInfo));
	_GP(StaticObjectArray).Create(&_GP(ccDynamicObject), sizeof(ScriptObject), sizeof(ScriptObject));
	_GP(StaticGUIArray).Create(&_GP(ccDynamicGUI), sizeof(ScriptGUI), sizeof(ScriptGUI));
	_GP(StaticHotspotArray).Create(&_GP(ccDynamicHotspot), sizeof(ScriptHotspot), sizeof(ScriptHotspot));
	_GP(StaticRegionArray).Create(&_GP(ccDynamicRegion), sizeof(ScriptRegion), sizeof(ScriptRegion));
	_GP(StaticInventoryArray).Create(&_GP(ccDynamicInv), sizeof(ScriptInvItem), sizeof(ScriptInvItem));
	_GP(StaticDialogArray).Create(&_GP(ccDynamicDialog), sizeof(ScriptDialog), sizeof(ScriptDialog));

	ccAddExternalStaticArray("character", &_GP(game).chars[0], &_GP(StaticCharacterArray));
	ccAddExternalStaticArray("object", &_G(scrObj)[0], &_GP(StaticObjectArray));
	ccAddExternalStaticArray("gui", &_G(scrGui)[0], &_GP(StaticGUIArray));
	ccAddExternalStaticArray("hotspot", &_G(scrHotspot)[0], &_GP(StaticHotspotArray));
	ccAddExternalStaticArray("region", &_G(scrRegion)[0], &_GP(StaticRegionArray));
	ccAddExternalStaticArray("inventory", &_G(scrInv)[0], &_GP(StaticInventoryArray));
	ccAddExternalStaticArray("dialog", &_G(scrDialog)[0], &_GP(StaticDialogArray));
}

// Initializes various game entities and registers them in the script system
HError InitAndRegisterGameEntities() {
	InitAndRegisterAudioObjects();
	InitAndRegisterCharacters();
	InitAndRegisterDialogs();
	InitAndRegisterDialogOptions();
	HError err = InitAndRegisterGUI();
	if (!err)
		return err;
	InitAndRegisterInvItems();

	InitAndRegisterHotspots();
	InitAndRegisterRegions();
	InitAndRegisterRoomObjects();
	_GP(play).CreatePrimaryViewportAndCamera();

	RegisterStaticArrays();

	setup_player_character(_GP(game).playercharacter);
	if (_G(loaded_game_file_version) >= kGameVersion_270)
		ccAddExternalStaticObject("player", &_G(sc_PlayerCharPtr), &_GP(GlobalStaticManager));
	return HError::None();
}

void LoadFonts(GameDataVersion data_ver) {
	for (int i = 0; i < _GP(game).numfonts; ++i) {
		if (!wloadfont_size(i, _GP(game).fonts[i]))
			quitprintf("Unable to load font %d, no renderer could load a matching file", i);
	}
}

void LoadLipsyncData() {
	std::unique_ptr<Stream> speechsync(_GP(AssetMgr)->OpenAsset("syncdata.dat", "voice"));
	if (!speechsync)
		return;
	// this game has voice lip sync
	int lipsync_fmt = speechsync->ReadInt32();
	if (lipsync_fmt != 4) {
		Debug::Printf(kDbgMsg_Info, "Unknown speech lip sync format (%d).\nLip sync disabled.", lipsync_fmt);
	} else {
		_G(numLipLines) = speechsync->ReadInt32();
		_G(splipsync) = (SpeechLipSyncLine *)malloc(sizeof(SpeechLipSyncLine) * _G(numLipLines));
		for (int ee = 0; ee < _G(numLipLines); ee++) {
			_G(splipsync)[ee].numPhonemes = speechsync->ReadInt16();
			speechsync->Read(_G(splipsync)[ee].filename, 14);
			_G(splipsync)[ee].endtimeoffs = (int32_t *)malloc(_G(splipsync)[ee].numPhonemes * sizeof(int));
			speechsync->ReadArrayOfInt32(_G(splipsync)[ee].endtimeoffs, _G(splipsync)[ee].numPhonemes);
			_G(splipsync)[ee].frame = (short *)malloc(_G(splipsync)[ee].numPhonemes * sizeof(short));
			speechsync->ReadArrayOfInt16(_G(splipsync)[ee].frame, _G(splipsync)[ee].numPhonemes);
		}
	}
	Debug::Printf(kDbgMsg_Info, "Lipsync data found and loaded");
}

void AllocScriptModules() {
	_GP(moduleInst).resize(_G(numScriptModules), nullptr);
	_GP(moduleInstFork).resize(_G(numScriptModules), nullptr);
	_GP(moduleRepExecAddr).resize(_G(numScriptModules));
	_GP(repExecAlways).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(lateRepExecAlways).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(getDialogOptionsDimensionsFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(renderDialogOptionsFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(getDialogOptionUnderCursorFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(runDialogOptionMouseClickHandlerFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(runDialogOptionKeyPressHandlerFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	_GP(runDialogOptionRepExecFunc).moduleHasFunction.resize(_G(numScriptModules), true);
	for (int i = 0; i < _G(numScriptModules); ++i) {
		_GP(moduleRepExecAddr)[i].Invalidate();
	}
}

HGameInitError InitGameState(const LoadedGameEntities &ents, GameDataVersion data_ver) {
	const ScriptAPIVersion base_api = (ScriptAPIVersion)_GP(game).options[OPT_BASESCRIPTAPI];
	const ScriptAPIVersion compat_api = (ScriptAPIVersion)_GP(game).options[OPT_SCRIPTCOMPATLEV];
	if (data_ver >= kGameVersion_341) {
		const char *base_api_name = GetScriptAPIName(base_api);
		const char *compat_api_name = GetScriptAPIName(compat_api);
		Debug::Printf(kDbgMsg_Info, "Requested script API: %s (%d), compat level: %s (%d)",
			base_api >= 0 && base_api <= kScriptAPI_Current ? base_api_name : "unknown", base_api,
			compat_api >= 0 && compat_api <= kScriptAPI_Current ? compat_api_name : "unknown", compat_api);
	}
	// If the game was compiled using unsupported version of the script API,
	// we warn about potential incompatibilities but proceed further.
	if (_GP(game).options[OPT_BASESCRIPTAPI] > kScriptAPI_Current)
		_G(platform)->DisplayAlert("Warning: this game requests a higher version of AGS script API, it may not run correctly or run at all.");

	//
	// 1. Check that the loaded data is valid and compatible with the current
	// engine capabilities.
	//
	if (_GP(game).numfonts == 0)
		return new GameInitError(kGameInitErr_NoFonts);
	if (_GP(game).audioClipTypes.size() > MAX_AUDIO_TYPES)
		return new GameInitError(kGameInitErr_TooManyAudioTypes,
			String::FromFormat("Required: %zu, max: %zu", _GP(game).audioClipTypes.size(), MAX_AUDIO_TYPES));

	//
	// 3. Allocate and init game objects
	//
	_G(charextra) = (CharacterExtras *)calloc(_GP(game).numcharacters, sizeof(CharacterExtras));
	_G(charcache) = (CharacterCache *)calloc(1, sizeof(CharacterCache) * _GP(game).numcharacters + 5);
	_G(mls) = (MoveList *)calloc(_GP(game).numcharacters + MAX_ROOM_OBJECTS + 1, sizeof(MoveList));
	_G(actSpsCount) = _GP(game).numcharacters + MAX_ROOM_OBJECTS + 2;
	_G(actsps) = (Bitmap **)calloc(_G(actSpsCount), sizeof(Bitmap *));
	_G(actspsbmp) = (IDriverDependantBitmap **)calloc(_G(actSpsCount), sizeof(IDriverDependantBitmap *));
	_G(actspswb) = (Bitmap **)calloc(_G(actSpsCount), sizeof(Bitmap *));
	_G(actspswbbmp) = (IDriverDependantBitmap **)calloc(_G(actSpsCount), sizeof(IDriverDependantBitmap *));
	_G(actspswbcache) = (CachedActSpsData *)calloc(_G(actSpsCount), sizeof(CachedActSpsData));
	_GP(play).charProps.resize(_GP(game).numcharacters);
	_G(old_dialog_scripts) = ents.OldDialogScripts;
	_G(old_speech_lines) = ents.OldSpeechLines;
	HError err = InitAndRegisterGameEntities();
	if (!err)
		return new GameInitError(kGameInitErr_EntityInitFail, err);
	LoadFonts(data_ver);

	//
	// 4. Initialize certain runtime variables
	//
	_G(game_paused) = 0;  // reset the game paused flag
	_G(ifacepopped) = -1;

	String svg_suffix;
	if (_GP(game).saveGameFileExtension[0] != 0)
		svg_suffix.Format(".%s", _GP(game).saveGameFileExtension);
	set_save_game_suffix(svg_suffix);

	_GP(play).score_sound = _GP(game).scoreClipID;
	_GP(play).fade_effect = _GP(game).options[OPT_FADETYPE];

	//
	// 5. Initialize runtime state of certain game objects
	//
	for (int i = 0; i < _G(numguilabels); ++i) {
		// labels are not clickable by default
		_GP(guilabels)[i].SetClickable(false);
	}
	_GP(play).gui_draw_order = (int32_t *)calloc(_GP(game).numgui * sizeof(int), 1);
	update_gui_zorder();
	calculate_reserved_channel_count();

	//
	// 6. Register engine API exports
	// NOTE: we must do this before plugin start, because some plugins may
	// require access to script API at initialization time.
	//
	ccSetScriptAliveTimer(150000);
	ccSetStringClassImpl(&_GP(myScriptStringImpl));
	setup_script_exports(base_api, compat_api);

	//
	// 7. Start up plugins
	//
	pl_register_plugins(ents.PluginInfos);
	pl_startup_plugins();

	//
	// 8. Create script modules
	// NOTE: we must do this after plugins, because some plugins may export
	// script symbols too.
	//
	if (!ents.GlobalScript)
		return new GameInitError(kGameInitErr_NoGlobalScript);
	_GP(gamescript) = ents.GlobalScript;
	_GP(dialogScriptsScript) = ents.DialogScript;
	_G(numScriptModules) = ents.ScriptModules.size();
	_GP(scriptModules) = ents.ScriptModules;
	AllocScriptModules();
	if (create_global_script())
		return new GameInitError(kGameInitErr_ScriptLinkFailed, _G(ccErrorString));

	return HGameInitError::None();
}

} // namespace Engine
} // namespace AGS
} // namespace AGS3
