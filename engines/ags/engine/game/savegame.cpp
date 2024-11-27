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

#include "ags/engine/ac/button.h"
#include "ags/engine/ac/character.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/region.h"
#include "ags/engine/ac/rich_game_media.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_status.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/game/savegame.h"
#include "ags/engine/game/savegame_components.h"
#include "ags/engine/game/savegame_internal.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/main.h"
#include "ags/engine/main/update.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/plugins/ags_plugin_evts.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/engine/script/script.h"
#include "ags/shared/script/cc_common.h"
#include "ags/shared/util/data_stream.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"
#include "ags/shared/util/math.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace Shared;
using namespace Engine;

// function is currently implemented in savegame_v321.cpp
HSaveError restore_save_data_v321(Stream *in, GameDataVersion data_ver, const PreservedParams &pp, RestoredData &r_data);

namespace AGS {
namespace Engine {

const char *SavegameSource::LegacySignature = "Adventure Game Studio saved game";
const char *SavegameSource::Signature = "Adventure Game Studio saved game v2";

SavegameSource::SavegameSource()
	: Version(kSvgVersion_Undefined) {
}

SavegameDescription::SavegameDescription()
	: MainDataVersion(kGameVersion_Undefined)
	, ColorDepth(0)
	, LegacyID(0) {
}

PreservedParams::PreservedParams()
	: SpeechVOX(0)
	, MusicVOX(0)
	, GlScDataSize(0) {
}

RestoredData::ScriptData::ScriptData()
	: Len(0) {
}

RestoredData::RestoredData()
	: FPS(0)
	, DoOnceCount(0u)
	, RoomVolume(kRoomVolumeNormal)
	, CursorID(0)
	, CursorMode(0) {
	memset(RoomLightLevels, 0, sizeof(RoomLightLevels));
	memset(RoomTintLevels, 0, sizeof(RoomTintLevels));
	memset(RoomZoomLevels1, 0, sizeof(RoomZoomLevels1));
	memset(RoomZoomLevels2, 0, sizeof(RoomZoomLevels2));
	memset(DoAmbient, 0, sizeof(DoAmbient));
}

String GetSavegameErrorText(SavegameErrorType err) {
	switch (err) {
	case kSvgErr_NoError:
		return "No error.";
	case kSvgErr_FileOpenFailed:
		return "File not found or could not be opened.";
	case kSvgErr_SignatureFailed:
		return "Not an AGS saved game or unsupported format.";
	case kSvgErr_FormatVersionNotSupported:
		return "Save format version not supported.";
	case kSvgErr_IncompatibleEngine:
		return "Save was written by incompatible engine, or file is corrupted.";
	case kSvgErr_GameGuidMismatch:
		return "Game GUID does not match, saved by a different game.";
	case kSvgErr_ComponentListOpeningTagFormat:
		return "Failed to parse opening tag of the components list.";
	case kSvgErr_ComponentListClosingTagMissing:
		return "Closing tag of the components list was not met.";
	case kSvgErr_ComponentOpeningTagFormat:
		return "Failed to parse opening component tag.";
	case kSvgErr_ComponentClosingTagFormat:
		return "Failed to parse closing component tag.";
	case kSvgErr_ComponentSizeMismatch:
		return "Component data size mismatch.";
	case kSvgErr_UnsupportedComponent:
		return "Unknown and/or unsupported component.";
	case kSvgErr_ComponentSerialization:
		return "Failed to write the savegame component.";
	case kSvgErr_ComponentUnserialization:
		return "Failed to restore the savegame component.";
	case kSvgErr_InconsistentFormat:
		return "Inconsistent format, or file is corrupted.";
	case kSvgErr_UnsupportedComponentVersion:
		return "Component data version not supported.";
	case kSvgErr_GameContentAssertion:
		return "Saved content does not match current game.";
	case kSvgErr_InconsistentData:
		return "Inconsistent save data, or file is corrupted.";
	case kSvgErr_InconsistentPlugin:
		return "One of the game plugins did not restore its game data correctly.";
	case kSvgErr_DifferentColorDepth:
		return "Saved with the engine running at a different colour depth.";
	case kSvgErr_GameObjectInitFailed:
		return "Game object initialization failed after save restoration.";
	default:
		return "Unknown error.";
	}
}

Bitmap *RestoreSaveImage(Stream *in) {
	if (in->ReadInt32())
		return read_serialized_bitmap(in);
	return nullptr;
}

void SkipSaveImage(Stream *in) {
	if (in->ReadInt32())
		skip_serialized_bitmap(in);
}

HSaveError ReadDescription(Stream *in, SavegameVersion &svg_ver, SavegameDescription &desc, SavegameDescElem elems) {
	svg_ver = (SavegameVersion)in->ReadInt32();
	if (svg_ver < kSvgVersion_LowestSupported || svg_ver > kSvgVersion_Current)
		return new SavegameError(kSvgErr_FormatVersionNotSupported,
		                         String::FromFormat("Required: %d, supported: %d - %d.", svg_ver, kSvgVersion_LowestSupported, kSvgVersion_Current));

	// Environment information
	if (svg_ver >= kSvgVersion_351)
		in->ReadInt32(); // environment info size
	if (elems & kSvgDesc_EnvInfo) {
		desc.EngineName = StrUtil::ReadString(in);
		desc.EngineVersion.SetFromString(StrUtil::ReadString(in));
		desc.GameGuid = StrUtil::ReadString(in);
		desc.GameTitle = StrUtil::ReadString(in);
		desc.MainDataFilename = StrUtil::ReadString(in);
		if (svg_ver >= kSvgVersion_Cmp_64bit)
			desc.MainDataVersion = (GameDataVersion)in->ReadInt32();
		desc.ColorDepth = in->ReadInt32();
		if (svg_ver >= kSvgVersion_351)
			desc.LegacyID = in->ReadInt32();
	} else {
		StrUtil::SkipString(in); // engine name
		StrUtil::SkipString(in); // engine version
		StrUtil::SkipString(in); // game guid
		StrUtil::SkipString(in); // game title
		StrUtil::SkipString(in); // main data filename
		if (svg_ver >= kSvgVersion_Cmp_64bit)
			in->ReadInt32(); // game data version
		in->ReadInt32(); // color depth
		if (svg_ver >= kSvgVersion_351)
			in->ReadInt32(); // game legacy id
	}
	// User description
	if (elems & kSvgDesc_UserText)
		desc.UserText = StrUtil::ReadString(in);
	else
		StrUtil::SkipString(in);
	if (elems & kSvgDesc_UserImage)
		desc.UserImage.reset(RestoreSaveImage(in));
	else
		SkipSaveImage(in);

	return HSaveError::None();
}

HSaveError ReadDescription_v321(Stream *in, SavegameVersion &svg_ver, SavegameDescription &desc, SavegameDescElem elems) {
	// Legacy savegame header
	if (elems & kSvgDesc_UserText)
		desc.UserText.Read(in);
	else
		StrUtil::SkipCStr(in);
	svg_ver = (SavegameVersion)in->ReadInt32();

	// Check saved game format version
	if (svg_ver < kSvgVersion_LowestSupported ||
	        svg_ver > kSvgVersion_Current) {
		return new SavegameError(kSvgErr_FormatVersionNotSupported,
		                         String::FromFormat("Required: %d, supported: %d - %d.", svg_ver, kSvgVersion_LowestSupported, kSvgVersion_Current));
	}

	if (elems & kSvgDesc_UserImage)
		desc.UserImage.reset(RestoreSaveImage(in));
	else
		SkipSaveImage(in);

	// This is the lowest legacy save format we support,
	// judging by the engine code received from CJ.
	const Version low_compat_version(3, 2, 0, 1103);
	String version_str = String::FromStream(in);
	Version eng_version(version_str);
	if (eng_version > _G(EngineVersion) || eng_version < low_compat_version) {
		// Engine version is either non-forward or non-backward compatible
		return new SavegameError(kSvgErr_IncompatibleEngine,
		                         String::FromFormat("Required: %s, supported: %s - %s.", eng_version.LongString.GetCStr(), low_compat_version.LongString.GetCStr(), _G(EngineVersion).LongString.GetCStr()));
	}
	if (elems & kSvgDesc_EnvInfo) {
		desc.MainDataFilename.Read(in);
		in->ReadInt32(); // unscaled game height with borders, now obsolete
		desc.ColorDepth = in->ReadInt32();
	} else {
		StrUtil::SkipCStr(in);
		in->ReadInt32(); // unscaled game height with borders, now obsolete
		in->ReadInt32(); // color depth
	}

	return HSaveError::None();
}

HSaveError OpenSavegameBase(const String &filename, SavegameSource *src, SavegameDescription *desc, SavegameDescElem elems) {
	UStream in(File::OpenFileRead(filename));
	if (!in.get())
		return new SavegameError(kSvgErr_FileOpenFailed, String::FromFormat("Requested filename: %s.", filename.GetCStr()));

	// Skip MS Windows Vista rich media header
	RICH_GAME_MEDIA_HEADER rich_media_header;
	rich_media_header.ReadFromFile(in.get());

	// Check saved game signature
	bool is_new_save = false;
	size_t pre_sig_pos = in->GetPosition();
	String svg_sig = String::FromStreamCount(in.get(), strlen(SavegameSource::Signature));
	if (svg_sig.Compare(SavegameSource::Signature) == 0) {
		is_new_save = true;
	} else {
		in->Seek(pre_sig_pos, kSeekBegin);
		svg_sig = String::FromStreamCount(in.get(), strlen(SavegameSource::LegacySignature));
		if (svg_sig.Compare(SavegameSource::LegacySignature) != 0)
			return new SavegameError(kSvgErr_SignatureFailed);
	}

	SavegameVersion svg_ver;
	SavegameDescription temp_desc;
	HSaveError err;
	if (is_new_save)
		err = ReadDescription(in.get(), svg_ver, temp_desc, desc ? elems : kSvgDesc_None);
	else
		err = ReadDescription_v321(in.get(), svg_ver, temp_desc, desc ? elems : kSvgDesc_None);
	if (!err)
		return err;

	if (src) {
		src->Filename = filename;
		src->Version = svg_ver;
		src->InputStream.reset(in.release()); // give the stream away to the caller
	}
	if (desc) {
		if (elems & kSvgDesc_EnvInfo) {
			desc->EngineName = temp_desc.EngineName;
			desc->EngineVersion = temp_desc.EngineVersion;
			desc->GameGuid = temp_desc.GameGuid;
			desc->LegacyID = temp_desc.LegacyID;
			desc->GameTitle = temp_desc.GameTitle;
			desc->MainDataFilename = temp_desc.MainDataFilename;
			desc->MainDataVersion = temp_desc.MainDataVersion;
			desc->ColorDepth = temp_desc.ColorDepth;
		}
		if (elems & kSvgDesc_UserText)
			desc->UserText = temp_desc.UserText;
		if (elems & kSvgDesc_UserImage)
			desc->UserImage.reset(temp_desc.UserImage.release());
	}
	return err;
}

HSaveError OpenSavegame(const String &filename, SavegameSource &src, SavegameDescription &desc, SavegameDescElem elems) {
	return OpenSavegameBase(filename, &src, &desc, elems);
}

HSaveError OpenSavegame(const String &filename, SavegameDescription &desc, SavegameDescElem elems) {
	return OpenSavegameBase(filename, nullptr, &desc, elems);
}

// Prepares engine for actual save restore (stops processes, cleans up memory)
void DoBeforeRestore(PreservedParams &pp) {
	pp.SpeechVOX = _GP(play).voice_avail;
	pp.MusicVOX = _GP(play).separate_music_lib;
	memcpy(pp.GameOptions, _GP(game).options, GameSetupStruct::MAX_OPTIONS * sizeof(int));

	unload_old_room();
	_G(raw_saved_screen).reset();
	remove_all_overlays();
	_GP(play).complete_overlay_on = 0;
	_GP(play).text_overlay_on = 0;

	// cleanup dynamic sprites
	// NOTE: sprite 0 is a special constant sprite that cannot be dynamic
	for (int i = 1; i < (int)_GP(spriteset).GetSpriteSlotCount(); ++i) {
		if (_GP(game).SpriteInfos[i].Flags & SPF_DYNAMICALLOC) {
			free_dynamic_sprite(i);
		}
	}

    // Cleanup drawn caches
    clear_drawobj_cache();

	// preserve script data sizes and cleanup scripts
	pp.GlScDataSize = _G(gameinst)->globaldatasize;
	pp.ScMdDataSize.resize(_G(numScriptModules));
	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		pp.ScMdDataSize[i] = _GP(moduleInst)[i]->globaldatasize;
	}

	FreeAllScriptInstances();

	// reset saved room states
	resetRoomStatuses();
	// reset temp room state
	_GP(troom) = RoomStatus();
	// reset (some of the?) GameState data
	// FIXME: investigate and refactor to be able to just reset whole object
	_GP(play).FreeProperties();
	_GP(play).FreeViewportsAndCameras();
	free_do_once_tokens();

	RemoveAllButtonAnimations();
	// unregister gui controls from API exports
	// CHECKME: find out why are we doing this here? why only to gui controls?
	for (int i = 0; i < _GP(game).numgui; ++i) {
		unexport_gui_controls(i);
	}
	// Clear the managed object pool
	ccUnregisterAllObjects();

	// NOTE: channels are array of MAX_SOUND_CHANNELS+1 size
	for (int i = 0; i < TOTAL_AUDIO_CHANNELS; ++i) {
		stop_and_destroy_channel_ex(i, false);
	}

	clear_music_cache();
}

void RestoreViewportsAndCameras(const RestoredData &r_data) {
	// If restored from older saves, we have to adjust
	// cam and view sizes to a main viewport, which is init later
	const auto &main_view = _GP(play).GetMainViewport();

	for (size_t i = 0; i < r_data.Cameras.size(); ++i) {
		const auto &cam_dat = r_data.Cameras[i];
		auto cam = _GP(play).GetRoomCamera(i);
		cam->SetID(cam_dat.ID);
		if ((cam_dat.Flags & kSvgCamPosLocked) != 0)
			cam->Lock();
		else
			cam->Release();
		// Set size first, or offset position may clamp to the room
		if (r_data.LegacyViewCamera)
			cam->SetSize(main_view.GetSize());
		else
			cam->SetSize(Size(cam_dat.Width, cam_dat.Height));
		cam->SetAt(cam_dat.Left, cam_dat.Top);
	}
	for (size_t i = 0; i < r_data.Viewports.size(); ++i) {
		const auto &view_dat = r_data.Viewports[i];
		auto view = _GP(play).GetRoomViewport(i);
		view->SetID(view_dat.ID);
		view->SetVisible((view_dat.Flags & kSvgViewportVisible) != 0);
		if (r_data.LegacyViewCamera)
			view->SetRect(RectWH(view_dat.Left, view_dat.Top, main_view.GetWidth(), main_view.GetHeight()));
		else
			view->SetRect(RectWH(view_dat.Left, view_dat.Top, view_dat.Width, view_dat.Height));
		view->SetZOrder(view_dat.ZOrder);
		// Restore camera link
		int cam_index = view_dat.CamID;
		if (cam_index < 0) continue;
		auto cam = _GP(play).GetRoomCamera(cam_index);
		view->LinkCamera(cam);
		cam->LinkToViewport(view);
	}
	_GP(play).InvalidateViewportZOrder();
}

// Resets a number of options that are not supposed to be changed at runtime
static void CopyPreservedGameOptions(GameSetupStructBase &gs, const PreservedParams &pp) {
	const auto restricted_opts = GameSetupStructBase::GetRestrictedOptions();
	for (auto opt : restricted_opts)
		gs.options[opt] = pp.GameOptions[opt];
}

// Final processing after successfully restoring from save
HSaveError DoAfterRestore(const PreservedParams &pp, RestoredData &r_data) {
	// Use a yellow dialog highlight for older game versions
	// CHECKME: it is dubious that this should be right here
	if (_G(loaded_game_file_version) < kGameVersion_331)
		_GP(play).dialog_options_highlight_color = DIALOG_OPTIONS_HIGHLIGHT_COLOR_DEFAULT;

	// Preserve whether the music vox is available
	_GP(play).voice_avail = pp.SpeechVOX;
	_GP(play).separate_music_lib = pp.MusicVOX;

	// Restore particular game options that must not change at runtime
	CopyPreservedGameOptions(_GP(game), pp);

	// Restore debug flags
	if (_G(debug_flags) & DBG_DEBUGMODE)
		_GP(play).debug_mode = 1;

	// recache queued clips
	for (int i = 0; i < _GP(play).new_music_queue_size; ++i) {
		_GP(play).new_music_queue[i].cachedClip = nullptr;
	}

	// Remap old sound nums in case we restored a save having a different list of audio clips
	RemapLegacySoundNums(_GP(game), _GP(views), _G(loaded_game_file_version));

	// Restore Overlay bitmaps (older save format, which stored them along with overlays)
	auto &overs = get_overlays();
	for (auto &over_im : r_data.OverlayImages) {
		auto &over = overs[over_im._key];
		over.SetImage(std::move(over_im._value), over.HasAlphaChannel(), over.offsetX, over.offsetY);
	}

	// Restore dynamic surfaces
	const size_t dynsurf_num = MIN((uint)MAX_DYNAMIC_SURFACES, r_data.DynamicSurfaces.size());
	for (size_t i = 0; i < dynsurf_num; ++i) {
		_G(dynamicallyCreatedSurfaces)[i] = std::move(r_data.DynamicSurfaces[i]);
	}

	// Re-export any missing audio channel script objects, e.g. if restoring old save
	export_missing_audiochans();

	// CHECKME: find out why are we doing this here? why only to gui controls?
	for (int i = 0; i < _GP(game).numgui; ++i)
		export_gui_controls(i);

	update_gui_zorder();

	AllocScriptModules();
	if (create_global_script()) {
		return new SavegameError(kSvgErr_GameObjectInitFailed,
		                         String::FromFormat("Unable to recreate global script: %s", cc_get_error().ErrorString.GetCStr()));
	}

	// read the global data into the newly created script
	if (!r_data.GlobalScript.Data.empty())
		memcpy(_G(gameinst)->globaldata, &r_data.GlobalScript.Data.front(),
		       MIN((size_t)_G(gameinst)->globaldatasize, r_data.GlobalScript.Len));

	// restore the script module data
	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		if (!r_data.ScriptModules[i].Data.empty())
			memcpy(_GP(moduleInst)[i]->globaldata, &r_data.ScriptModules[i].Data.front(),
			       MIN((size_t)_GP(moduleInst)[i]->globaldatasize, r_data.ScriptModules[i].Len));
	}

	setup_player_character(_GP(game).playercharacter);

	// Save some parameters to restore them after room load
	int gstimer = _GP(play).gscript_timer;
	int oldx1 = _GP(play).mboundx1, oldx2 = _GP(play).mboundx2;
	int oldy1 = _GP(play).mboundy1, oldy2 = _GP(play).mboundy2;

	// disable the queue momentarily
	int queuedMusicSize = _GP(play).music_queue_size;
	_GP(play).music_queue_size = 0;

	// load the room the game was saved in
	if (_G(displayed_room) >= 0)
		load_new_room(_G(displayed_room), nullptr);
	else
		set_room_placeholder();

	_GP(play).gscript_timer = gstimer;
	// restore the correct room volume (they might have modified
	// it with SetMusicVolume)
	_GP(thisroom).Options.MusicVolume = r_data.RoomVolume;

	_GP(mouse).SetMoveLimit(Rect(oldx1, oldy1, oldx2, oldy2));

	set_cursor_mode(r_data.CursorMode);
	set_mouse_cursor(r_data.CursorID, true);
	if (r_data.CursorMode == MODE_USE)
		SetActiveInventory(_G(playerchar)->activeinv);
	// ensure that the current cursor is locked
	_GP(spriteset).PrecacheSprite(_GP(game).mcurs[r_data.CursorID].pic);

	sys_window_set_title(_GP(play).game_name.GetCStr());

	if (_G(displayed_room) >= 0) {
		// Fixup the frame index, in case the restored room does not have enough background frames
		if (_GP(play).bg_frame < 0 || static_cast<size_t>(_GP(play).bg_frame) >= _GP(thisroom).BgFrameCount)
			_GP(play).bg_frame = 0;

		for (int i = 0; i < MAX_ROOM_BGFRAMES; ++i) {
			if (r_data.RoomBkgScene[i]) {
				_GP(thisroom).BgFrames[i].Graphic = r_data.RoomBkgScene[i];
			}
		}

		_G(in_new_room) = 3;  // don't run "enters screen" events
		// now that room has loaded, copy saved light levels in
		for (size_t i = 0; i < MAX_ROOM_REGIONS; ++i) {
			_GP(thisroom).Regions[i].Light = r_data.RoomLightLevels[i];
			_GP(thisroom).Regions[i].Tint = r_data.RoomTintLevels[i];
		}
		generate_light_table();

		for (size_t i = 0; i < MAX_WALK_AREAS; ++i) {
			_GP(thisroom).WalkAreas[i].ScalingFar = r_data.RoomZoomLevels1[i];
			_GP(thisroom).WalkAreas[i].ScalingNear = r_data.RoomZoomLevels2[i];
		}

		on_background_frame_change();
	}

	GUI::Options.DisabledStyle = static_cast<GuiDisableStyle>(_GP(game).options[OPT_DISABLEOFF]);

	// restore the queue now that the music is playing
	_GP(play).music_queue_size = queuedMusicSize;

	if (_GP(play).digital_master_volume >= 0) {
		int temp_vol = _GP(play).digital_master_volume;
		_GP(play).digital_master_volume = -1; // reset to invalid state before re-applying
		System_SetVolume(temp_vol);
	}

	// Run audio clips on channels
	// these two crossfading parameters have to be temporarily reset
	const int cf_in_chan = _GP(play).crossfading_in_channel;
	const int cf_out_chan = _GP(play).crossfading_out_channel;
	_GP(play).crossfading_in_channel = 0;
	_GP(play).crossfading_out_channel = 0;

	// NOTE: channels are array of MAX_SOUND_CHANNELS+1 size
	for (int i = 0; i < TOTAL_AUDIO_CHANNELS; ++i) {
		const RestoredData::ChannelInfo &chan_info = r_data.AudioChans[i];
		if (chan_info.ClipID < 0)
			continue;
		if ((size_t)chan_info.ClipID >= _GP(game).audioClips.size()) {
			return new SavegameError(kSvgErr_GameObjectInitFailed,
				String::FromFormat("Invalid audio clip index: %d (clip count: %zu).", chan_info.ClipID, _GP(game).audioClips.size()));
		}
		play_audio_clip_on_channel(i, &_GP(game).audioClips[chan_info.ClipID],
			                        chan_info.Priority, chan_info.Repeat, chan_info.Pos);

		auto *ch = AudioChans::GetChannel(i);
		if (ch != nullptr) {
			ch->set_volume_direct(chan_info.VolAsPercent, chan_info.Vol);
			ch->set_speed(chan_info.Speed);
			ch->set_panning(chan_info.Pan);
			ch->_xSource = chan_info.XSource;
			ch->_ySource = chan_info.YSource;
			ch->_maximumPossibleDistanceAway = chan_info.MaxDist;

			if ((chan_info.Flags & kSvgAudioPaused) != 0)
				ch->pause();
		}
	}
	if ((cf_in_chan > 0) && (AudioChans::GetChannel(cf_in_chan) != nullptr))
		_GP(play).crossfading_in_channel = cf_in_chan;
	if ((cf_out_chan > 0) && (AudioChans::GetChannel(cf_out_chan) != nullptr))
		_GP(play).crossfading_out_channel = cf_out_chan;

	// If there were synced audio tracks, the time taken to load in the
	// different channels will have thrown them out of sync, so re-time it
	// NOTE: channels are array of MAX_SOUND_CHANNELS+1 size
	for (int i = 0; i < TOTAL_AUDIO_CHANNELS; ++i) {
		auto *ch = AudioChans::GetChannelIfPlaying(i);
		int pos = r_data.AudioChans[i].Pos;
		if ((pos > 0) && (ch != nullptr)) {
			ch->seek(pos);
		}
	}

	for (int i = NUM_SPEECH_CHANS; i < _GP(game).numGameChannels; ++i) {
		if (r_data.DoAmbient[i])
			PlayAmbientSound(i, r_data.DoAmbient[i], _GP(ambient)[i].vol, _GP(ambient)[i].x, _GP(ambient)[i].y);
	}
	update_directional_sound_vol();

	adjust_fonts_for_render_mode(_GP(game).options[OPT_ANTIALIASFONTS] != 0);

	restore_characters();
	restore_overlays();
	restore_movelists();

	GUI::MarkAllGUIForUpdate(true, true);

	RestoreViewportsAndCameras(r_data);

	_GP(play).ClearIgnoreInput(); // don't keep ignored input after save restore
	update_polled_stuff();

	pl_run_plugin_hooks(AGSE_POSTRESTOREGAME, 0);

	if (_G(displayed_room) < 0) {
		// the restart point, no room was loaded
		load_new_room(_G(playerchar)->room, _G(playerchar));

		first_room_initialization();
	}

	if ((_GP(play).music_queue_size > 0) && (_G(cachedQueuedMusic) == nullptr)) {
		_G(cachedQueuedMusic) = load_music_from_disk(_GP(play).music_queue[0], 0);
	}

	// Test if the old-style audio had playing music and it was properly loaded
	if (_G(current_music_type) > 0) {
		if ((_G(crossFading) > 0 && !AudioChans::GetChannelIfPlaying(_G(crossFading))) ||
				(_G(crossFading) <= 0 && !AudioChans::GetChannelIfPlaying(SCHAN_MUSIC))) {
			_G(current_music_type) = 0; // playback failed, reset flag
		}
	}

	set_game_speed(r_data.FPS);

	return HSaveError::None();
}

HSaveError RestoreGameState(Stream *in, SavegameVersion svg_version) {
	PreservedParams pp;
	RestoredData r_data;
	DoBeforeRestore(pp);
	HSaveError err;
	if (svg_version >= kSvgVersion_Components) {
		err = SavegameComponents::ReadAll(in, svg_version, pp, r_data);
	} else {
		GameDataVersion use_dataver = _GP(usetup).legacysave_assume_dataver != kGameVersion_Undefined ? _GP(usetup).legacysave_assume_dataver
																									  : _G(loaded_game_file_version);
		err = restore_save_data_v321(in, use_dataver, pp, r_data);
	}
	if (!err)
		return err;
	return DoAfterRestore(pp, r_data);
}


void WriteSaveImage(Stream *out, const Bitmap *screenshot) {
	// store the screenshot at the start to make it easily accessible
	out->WriteInt32((screenshot == nullptr) ? 0 : 1);

	if (screenshot)
		serialize_bitmap(screenshot, out);
}

void WriteDescription(Stream *out, const String &user_text, const Bitmap *user_image) {
	// Data format version
	out->WriteInt32(kSvgVersion_Current);
	soff_t env_pos = out->GetPosition();
	out->WriteInt32(0);
	// Environment information
	StrUtil::WriteString(get_engine_name(), out);
	StrUtil::WriteString(_G(EngineVersion).LongString, out);
	StrUtil::WriteString(_GP(game).guid, out);
	StrUtil::WriteString(_GP(game).gamename, out);
	StrUtil::WriteString(_GP(ResPaths).GamePak.Name, out);
	out->WriteInt32(_G(loaded_game_file_version));
	out->WriteInt32(_GP(game).GetColorDepth());
	out->WriteInt32(_GP(game).uniqueid);
	soff_t env_end_pos = out->GetPosition();
	out->Seek(env_pos, kSeekBegin);
	out->WriteInt32(env_end_pos - env_pos);
	out->Seek(env_end_pos, kSeekBegin);
	// User description
	StrUtil::WriteString(user_text, out);
	WriteSaveImage(out, user_image);
}

Stream *StartSavegame(const String &filename, const String &user_text, const Bitmap *user_image) {
	Stream *out = Shared::File::CreateFile(filename);
	if (!out)
		return nullptr;

	// Initialize and write Vista header
	RICH_GAME_MEDIA_HEADER vistaHeader;
	memset(&vistaHeader, 0, sizeof(RICH_GAME_MEDIA_HEADER));
	vistaHeader.dwMagicNumber = RM_MAGICNUMBER;
	vistaHeader.dwHeaderVersion = 1;
	vistaHeader.dwHeaderSize = sizeof(RICH_GAME_MEDIA_HEADER);
	vistaHeader.dwThumbnailOffsetHigherDword = 0;
	vistaHeader.dwThumbnailOffsetLowerDword = 0;
	vistaHeader.dwThumbnailSize = 0;
	convert_guid_from_text_to_binary(_GP(game).guid, &vistaHeader.guidGameId[0]);

	vistaHeader.setSaveName(user_text);

	vistaHeader.szLevelName[0] = 0;
	vistaHeader.szComments[0] = 0;
	// MS Windows Vista rich media header
	vistaHeader.WriteToFile(out);

	// Savegame signature
	out->Write(SavegameSource::Signature, strlen(SavegameSource::Signature));

	// CHECKME: what is this plugin hook suppose to mean, and if it is called here correctly
	pl_run_plugin_hooks(AGSE_PRESAVEGAME, 0);

	// Write descrition block
	WriteDescription(out, user_text, user_image);
	return out;
}

void DoBeforeSave() {
	if (_GP(play).cur_music_number >= 0) {
		if (IsMusicPlaying() == 0)
			_GP(play).cur_music_number = -1;
	}

	if (_G(displayed_room) >= 0) {
		// update the current room script's data segment copy
		if (_G(roominst))
			save_room_data_segment();

		// Update the saved interaction variable values
		for (size_t i = 0; i < _GP(thisroom).LocalVariables.size() && i < (size_t)MAX_GLOBAL_VARIABLES; ++i)
			_G(croom)->interactionVariableValues[i] = _GP(thisroom).LocalVariables[i].Value;
	}
}

void SaveGameState(Stream *out) {
	DoBeforeSave();
	SavegameComponents::WriteAllCommon(out);
}

void ReadPluginSaveData(Stream *in, PluginSvgVersion svg_ver, soff_t max_size) {
	const soff_t start_pos = in->GetPosition();
	const soff_t end_pos = start_pos + max_size;

	if (svg_ver >= kPluginSvgVersion_36115) {
		int num_plugins_read = in->ReadInt32();
		soff_t cur_pos = start_pos;
		while ((num_plugins_read--) > 0 && (cur_pos < end_pos)) {
			String pl_name = StrUtil::ReadString(in);
			size_t data_size = in->ReadInt32();
			soff_t data_start = in->GetPosition();

			auto pl_handle = AGSE_RESTOREGAME;
			pl_set_file_handle(pl_handle, in);
			pl_run_plugin_hook_by_name(pl_name, AGSE_RESTOREGAME, pl_handle);
			pl_clear_file_handle();

			// Seek to the end of plugin data, in case it ended up reading not in the end
			cur_pos = data_start + data_size;
			in->Seek(cur_pos, kSeekBegin);
		}
	} else {
		String pl_name;
		for (int pl_index = 0; pl_query_next_plugin_for_event(AGSE_RESTOREGAME, pl_index, pl_name); ++pl_index) {
			auto pl_handle = AGSE_RESTOREGAME;
			pl_set_file_handle(pl_handle, in);
			pl_run_plugin_hook_by_index(pl_index, AGSE_RESTOREGAME, pl_handle);
			pl_clear_file_handle();
		}
	}
}

void WritePluginSaveData(Stream *out) {
	soff_t pluginnum_pos = out->GetPosition();
	out->WriteInt32(0); // number of plugins which wrote data

	int num_plugins_wrote = 0;
	String pl_name;
	for (int pl_index = 0; pl_query_next_plugin_for_event(AGSE_SAVEGAME, pl_index, pl_name); ++pl_index) {
		// NOTE: we don't care if they really write anything,
		// but count them so long as they subscribed to AGSE_SAVEGAME
		num_plugins_wrote++;

		// Write a header for plugin data
		StrUtil::WriteString(pl_name, out);
		soff_t data_size_pos = out->GetPosition();
		out->WriteInt32(0); // data size

		// Create a stream section and write plugin data
		soff_t data_start_pos = out->GetPosition();
		auto pl_handle = AGSE_SAVEGAME;
		pl_set_file_handle(pl_handle, out);
		pl_run_plugin_hook_by_index(pl_index, AGSE_SAVEGAME, pl_handle);
		pl_clear_file_handle();

		// Finalize header
		soff_t data_end_pos = out->GetPosition();
		out->Seek(data_size_pos, kSeekBegin);
		out->WriteInt32(data_end_pos - data_start_pos);
		out->Seek(0, kSeekEnd);
	}

	// Write number of plugins
	out->Seek(pluginnum_pos, kSeekBegin);
	out->WriteInt32(num_plugins_wrote);
	out->Seek(0, kSeekEnd);
}

} // namespace Engine
} // namespace AGS
} // namespace AGS3
