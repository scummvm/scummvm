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
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/game/savegame.h"
#include "ags/engine/game/savegame_components.h"
#include "ags/engine/game/savegame_internal.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/main.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/engine/script/script.h"
#include "ags/shared/script/cc_error.h"
#include "ags/shared/util/aligned_stream.h"
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
HSaveError restore_game_data(Stream *in, SavegameVersion svg_version, const PreservedParams &pp, RestoredData &r_data);

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
		return "Game GUID does not match, saved by a different _GP(game).";
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
		break;
	}
	return "Unknown error.";
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

	// Enviroment information
	if (svg_ver >= kSvgVersion_351)
		in->ReadInt32(); // enviroment info size
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

	String version_str = String::FromStream(in);
	Version eng_version(version_str);
	if (eng_version > _G(EngineVersion) ||
	        eng_version < _G(SavedgameLowestBackwardCompatVersion)) {
		// Engine version is either non-forward or non-backward compatible
		return new SavegameError(kSvgErr_IncompatibleEngine,
		                         String::FromFormat("Required: %s, supported: %s - %s.", eng_version.LongString.GetCStr(), _G(SavedgameLowestBackwardCompatVersion).LongString.GetCStr(), _G(EngineVersion).LongString.GetCStr()));
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
	pp.SpeechVOX = _GP(play).want_speech;
	pp.MusicVOX = _GP(play).separate_music_lib;

	unload_old_room();
	delete _G(raw_saved_screen);
	_G(raw_saved_screen) = nullptr;
	remove_screen_overlay(-1);
	_GP(play).complete_overlay_on = 0;
	_GP(play).text_overlay_on = 0;

	// cleanup dynamic sprites
	// NOTE: sprite 0 is a special constant sprite that cannot be dynamic
	for (int i = 1; i < (int)_GP(spriteset).GetSpriteSlotCount(); ++i) {
		if (_GP(game).SpriteInfos[i].Flags & SPF_DYNAMICALLOC) {
			// do this early, so that it changing _GP(guibuts) doesn't
			// affect the restored data
			free_dynamic_sprite(i);
		}
	}

	// cleanup GUI backgrounds
	for (int i = 0; i < _GP(game).numgui; ++i) {
		delete _G(guibg)[i];
		_G(guibg)[i] = nullptr;

		if (_G(guibgbmp)[i])
			_G(gfxDriver)->DestroyDDB(_G(guibgbmp)[i]);
		_G(guibgbmp)[i] = nullptr;
	}

	// preserve script data sizes and cleanup scripts
	pp.GlScDataSize = _G(gameinst)->globaldatasize;
	delete _G(gameinstFork);
	delete _G(gameinst);
	_G(gameinstFork) = nullptr;
	_G(gameinst) = nullptr;
	pp.ScMdDataSize.resize(_G(numScriptModules));
	for (int i = 0; i < _G(numScriptModules); ++i) {
		pp.ScMdDataSize[i] = _GP(moduleInst)[i]->globaldatasize;
		delete _GP(moduleInstFork)[i];
		delete _GP(moduleInst)[i];
		_GP(moduleInst)[i] = nullptr;
	}

	_GP(play).FreeProperties();
	_GP(play).FreeViewportsAndCameras();

	delete _G(roominstFork);
	delete _G(roominst);
	_G(roominstFork) = nullptr;
	_G(roominst) = nullptr;

	delete _G(dialogScriptsInst);
	_G(dialogScriptsInst) = nullptr;

	resetRoomStatuses();
	_GP(troom).FreeScriptData();
	_GP(troom).FreeProperties();
	free_do_once_tokens();

	// unregister gui controls from API exports
	// TODO: find out why are we doing this here? is this really necessary?
	for (int i = 0; i < _GP(game).numgui; ++i) {
		unexport_gui_controls(i);
	}
	// Clear the managed object pool
	ccUnregisterAllObjects();

	// NOTE: channels are array of MAX_SOUND_CHANNELS+1 size
	for (int i = 0; i <= MAX_SOUND_CHANNELS; ++i) {
		stop_and_destroy_channel_ex(i, false);
	}

	clear_music_cache();
}

void RestoreViewportsAndCameras(const RestoredData &r_data) {
	for (size_t i = 0; i < r_data.Cameras.size(); ++i) {
		const auto &cam_dat = r_data.Cameras[i];
		auto cam = _GP(play).GetRoomCamera(i);
		cam->SetID(cam_dat.ID);
		if ((cam_dat.Flags & kSvgCamPosLocked) != 0)
			cam->Lock();
		else
			cam->Release();
		cam->SetAt(cam_dat.Left, cam_dat.Top);
		cam->SetSize(Size(cam_dat.Width, cam_dat.Height));
	}
	for (size_t i = 0; i < r_data.Viewports.size(); ++i) {
		const auto &view_dat = r_data.Viewports[i];
		auto view = _GP(play).GetRoomViewport(i);
		view->SetID(view_dat.ID);
		view->SetVisible((view_dat.Flags & kSvgViewportVisible) != 0);
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

// Final processing after successfully restoring from save
HSaveError DoAfterRestore(const PreservedParams &pp, const RestoredData &r_data) {
	// Use a yellow dialog highlight for older game versions
	// CHECKME: it is dubious that this should be right here
	if (_G(loaded_game_file_version) < kGameVersion_331)
		_GP(play).dialog_options_highlight_color = DIALOG_OPTIONS_HIGHLIGHT_COLOR_DEFAULT;

	// Preserve whether the music vox is available
	_GP(play).separate_music_lib = pp.MusicVOX;
	// If they had the vox when they saved it, but they don't now
	if ((pp.SpeechVOX < 0) && (_GP(play).want_speech >= 0))
		_GP(play).want_speech = (-_GP(play).want_speech) - 1;
	// If they didn't have the vox before, but now they do
	else if ((pp.SpeechVOX >= 0) && (_GP(play).want_speech < 0))
		_GP(play).want_speech = (-_GP(play).want_speech) - 1;

	// Restore debug flags
	if (_G(debug_flags) & DBG_DEBUGMODE)
		_GP(play).debug_mode = 1;

	// recache queued clips
	for (int i = 0; i < _GP(play).new_music_queue_size; ++i) {
		_GP(play).new_music_queue[i].cachedClip = nullptr;
	}

	// Remap old sound nums in case we restored a save having a different list of audio clips
	RemapLegacySoundNums(_GP(game), _G(views), _G(loaded_game_file_version));

	// restore these to the ones retrieved from the save game
	const size_t dynsurf_num = Math::Min((uint)MAX_DYNAMIC_SURFACES, r_data.DynamicSurfaces.size());
	for (size_t i = 0; i < dynsurf_num; ++i) {
		_G(dynamicallyCreatedSurfaces)[i] = r_data.DynamicSurfaces[i];
	}

	for (int i = 0; i < _GP(game).numgui; ++i)
		export_gui_controls(i);
	update_gui_zorder();

	if (create_global_script()) {
		return new SavegameError(kSvgErr_GameObjectInitFailed,
		                         String::FromFormat("Unable to recreate global script: %s", _G(ccErrorString).GetCStr()));
	}

	// read the global data into the newly created script
	if (r_data.GlobalScript.Data.get())
		memcpy(_G(gameinst)->globaldata, r_data.GlobalScript.Data.get(),
		       Math::Min((size_t)_G(gameinst)->globaldatasize, r_data.GlobalScript.Len));

	// restore the script module data
	for (int i = 0; i < _G(numScriptModules); ++i) {
		if (r_data.ScriptModules[i].Data.get())
			memcpy(_GP(moduleInst)[i]->globaldata, r_data.ScriptModules[i].Data.get(),
			       Math::Min((size_t)_GP(moduleInst)[i]->globaldatasize, r_data.ScriptModules[i].Len));
	}

	setup_player_character(_GP(game).playercharacter);

	// Save some parameters to restore them after room load
	int gstimer = _GP(play).gscript_timer;
	int oldx1 = _GP(play).mboundx1, oldx2 = _GP(play).mboundx2;
	int oldy1 = _GP(play).mboundy1, oldy2 = _GP(play).mboundy2;

	// disable the queue momentarily
	int queuedMusicSize = _GP(play).music_queue_size;
	_GP(play).music_queue_size = 0;

	update_polled_stuff_if_runtime();

	// load the room the game was saved in
	if (_G(displayed_room) >= 0)
		load_new_room(_G(displayed_room), nullptr);

	update_polled_stuff_if_runtime();

	_GP(play).gscript_timer = gstimer;
	// restore the correct room volume (they might have modified
	// it with SetMusicVolume)
	_GP(thisroom).Options.MusicVolume = r_data.RoomVolume;

	_GP(mouse).SetMoveLimit(Rect(oldx1, oldy1, oldx2, oldy2));

	set_cursor_mode(r_data.CursorMode);
	set_mouse_cursor(r_data.CursorID);
	if (r_data.CursorMode == MODE_USE)
		SetActiveInventory(_G(playerchar)->activeinv);
	// ensure that the current cursor is locked
	_GP(spriteset).Precache(_GP(game).mcurs[r_data.CursorID].pic);

	sys_window_set_title(_GP(play).game_name);

	update_polled_stuff_if_runtime();

	if (_G(displayed_room) >= 0) {
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

		for (size_t i = 0; i < MAX_WALK_AREAS + 1; ++i) {
			_GP(thisroom).WalkAreas[i].ScalingFar = r_data.RoomZoomLevels1[i];
			_GP(thisroom).WalkAreas[i].ScalingNear = r_data.RoomZoomLevels2[i];
		}

		on_background_frame_change();
	}

	_G(gui_disabled_style) = convert_gui_disabled_style(_GP(game).options[OPT_DISABLEOFF]);

	// restore the queue now that the music is playing
	_GP(play).music_queue_size = queuedMusicSize;

	if (_GP(play).digital_master_volume >= 0)
		System_SetVolume(_GP(play).digital_master_volume);

	// Run audio clips on channels
	// these two crossfading parameters have to be temporarily reset
	const int cf_in_chan = _GP(play).crossfading_in_channel;
	const int cf_out_chan = _GP(play).crossfading_out_channel;
	_GP(play).crossfading_in_channel = 0;
	_GP(play).crossfading_out_channel = 0;

	{
		AudioChannelsLock lock;
		// NOTE: channels are array of MAX_SOUND_CHANNELS+1 size
		for (int i = 0; i <= MAX_SOUND_CHANNELS; ++i) {
			const RestoredData::ChannelInfo &chan_info = r_data.AudioChans[i];
			if (chan_info.ClipID < 0)
				continue;
			if ((size_t)chan_info.ClipID >= _GP(game).audioClips.size()) {
				return new SavegameError(kSvgErr_GameObjectInitFailed,
					String::FromFormat("Invalid audio clip index: %d (clip count: %zu).", chan_info.ClipID, _GP(game).audioClips.size()));
			}
			play_audio_clip_on_channel(i, &_GP(game).audioClips[chan_info.ClipID],
			                           chan_info.Priority, chan_info.Repeat, chan_info.Pos);

			auto *ch = lock.GetChannel(i);
			if (ch != nullptr) {
				ch->set_volume_direct(chan_info.VolAsPercent, chan_info.Vol);
				ch->set_speed(chan_info.Speed);
				ch->set_panning(chan_info.Pan);
				ch->_panningAsPercentage = chan_info.PanAsPercent;
				ch->_xSource = chan_info.XSource;
				ch->_ySource = chan_info.YSource;
				ch->_maximumPossibleDistanceAway = chan_info.MaxDist;
			}
		}
		if ((cf_in_chan > 0) && (lock.GetChannel(cf_in_chan) != nullptr))
			_GP(play).crossfading_in_channel = cf_in_chan;
		if ((cf_out_chan > 0) && (lock.GetChannel(cf_out_chan) != nullptr))
			_GP(play).crossfading_out_channel = cf_out_chan;

		// If there were synced audio tracks, the time taken to load in the
		// different channels will have thrown them out of sync, so re-time it
		// NOTE: channels are array of MAX_SOUND_CHANNELS+1 size
		for (int i = 0; i <= MAX_SOUND_CHANNELS; ++i) {
			auto *ch = lock.GetChannelIfPlaying(i);
			int pos = r_data.AudioChans[i].Pos;
			if ((pos > 0) && (ch != nullptr)) {
				ch->seek(pos);
			}
		}
	} // -- AudioChannelsLock

	// TODO: investigate loop range
	for (int i = 1; i < MAX_SOUND_CHANNELS; ++i) {
		if (r_data.DoAmbient[i])
			PlayAmbientSound(i, r_data.DoAmbient[i], _GP(ambient)[i].vol, _GP(ambient)[i].x, _GP(ambient)[i].y);
	}
	update_directional_sound_vol();

	for (int i = 0; i < _GP(game).numgui; ++i) {
		_G(guibg)[i] = BitmapHelper::CreateBitmap(_GP(guis)[i].Width, _GP(guis)[i].Height, _GP(game).GetColorDepth());
		_G(guibg)[i] = ReplaceBitmapWithSupportedFormat(_G(guibg)[i]);
	}

	recreate_overlay_ddbs();

	GUI::MarkAllGUIForUpdate();

	RestoreViewportsAndCameras(r_data);

	_GP(play).ClearIgnoreInput(); // don't keep ignored input after save restore
	update_polled_stuff_if_runtime();

	pl_run_plugin_hooks(AGSE_POSTRESTOREGAME, 0);

	if (_G(displayed_room) < 0) {
		// the restart point, no room was loaded
		load_new_room(_G(playerchar)->room, _G(playerchar));
		_G(playerchar)->prevroom = -1;

		first_room_initialization();
	}

	if ((_GP(play).music_queue_size > 0) && (_G(cachedQueuedMusic) == nullptr)) {
		_G(cachedQueuedMusic) = load_music_from_disk(_GP(play).music_queue[0], 0);
	}

	// Test if the old-style audio had playing music and it was properly loaded
	if (_G(current_music_type) > 0) {
		AudioChannelsLock lock;

		if ((_G(crossFading) > 0 && !lock.GetChannelIfPlaying(_G(crossFading))) ||
		        (_G(crossFading) <= 0 && !lock.GetChannelIfPlaying(SCHAN_MUSIC))) {
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
	if (svg_version >= kSvgVersion_Components)
		err = SavegameComponents::ReadAll(in, svg_version, pp, r_data);
	else
		err = restore_game_data(in, svg_version, pp, r_data);
	if (!err)
		return err;
	return DoAfterRestore(pp, r_data);
}


void WriteSaveImage(Stream *out, const Bitmap *screenshot) {
	// store the screenshot at the start to make it easily accesible
	out->WriteInt32((screenshot == nullptr) ? 0 : 1);

	if (screenshot)
		serialize_bitmap(screenshot, out);
}

void WriteDescription(Stream *out, const String &user_text, const Bitmap *user_image) {
	// Data format version
	out->WriteInt32(kSvgVersion_Current);
	soff_t env_pos = out->GetPosition();
	out->WriteInt32(0);
	// Enviroment information
	StrUtil::WriteString("Adventure Game Studio run-time engine", out);
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

} // namespace Engine
} // namespace AGS
} // namespace AGS3
