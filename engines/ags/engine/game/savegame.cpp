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

#include "ac/character.h"
#include "ac/common.h"
#include "ac/draw.h"
#include "ac/dynamicsprite.h"
#include "ac/event.h"
#include "ac/game.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/gamesetup.h"
#include "ac/global_audio.h"
#include "ac/global_character.h"
#include "ac/gui.h"
#include "ac/mouse.h"
#include "ac/overlay.h"
#include "ac/region.h"
#include "ac/richgamemedia.h"
#include "ac/room.h"
#include "ac/roomstatus.h"
#include "ac/spritecache.h"
#include "ac/system.h"
#include "ac/timer.h"
#include "debug/out.h"
#include "device/mousew32.h"
#include "gfx/bitmap.h"
#include "gfx/ddb.h"
#include "gfx/graphicsdriver.h"
#include "game/savegame.h"
#include "game/savegame_components.h"
#include "game/savegame_internal.h"
#include "main/engine.h"
#include "main/main.h"
#include "platform/base/agsplatformdriver.h"
#include "plugin/agsplugin.h"
#include "plugin/plugin_engine.h"
#include "script/script.h"
#include "script/cc_error.h"
#include "util/alignedstream.h"
#include "util/file.h"
#include "util/stream.h"
#include "util/string_utils.h"
#include "media/audio/audio_system.h"

namespace AGS3 {

using namespace Shared;
using namespace Engine;

// function is currently implemented in game.cpp
HSaveError restore_game_data(Stream *in, SavegameVersion svg_version, const PreservedParams &pp, RestoredData &r_data);

extern GameSetupStruct game;
extern Bitmap **guibg;
extern AGS::Engine::IDriverDependantBitmap **guibgbmp;
extern AGS::Engine::IGraphicsDriver *gfxDriver;
extern Bitmap *dynamicallyCreatedSurfaces[MAX_DYNAMIC_SURFACES];
extern Bitmap *raw_saved_screen;
extern RoomStatus troom;
extern RoomStatus *croom;


namespace AGS {
namespace Engine {

const String SavegameSource::LegacySignature = "Adventure Game Studio saved game";
const String SavegameSource::Signature = "Adventure Game Studio saved game v2";

SavegameSource::SavegameSource()
	: Version(kSvgVersion_Undefined) {
}

SavegameDescription::SavegameDescription()
	: MainDataVersion(kGameVersion_Undefined)
	, ColorDepth(0) {
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
	if (elems & kSvgDesc_EnvInfo) {
		desc.EngineName = StrUtil::ReadString(in);
		desc.EngineVersion.SetFromString(StrUtil::ReadString(in));
		desc.GameGuid = StrUtil::ReadString(in);
		desc.GameTitle = StrUtil::ReadString(in);
		desc.MainDataFilename = StrUtil::ReadString(in);
		if (svg_ver >= kSvgVersion_Cmp_64bit)
			desc.MainDataVersion = (GameDataVersion)in->ReadInt32();
		desc.ColorDepth = in->ReadInt32();
	} else {
		StrUtil::SkipString(in);
		StrUtil::SkipString(in);
		StrUtil::SkipString(in);
		StrUtil::SkipString(in);
		StrUtil::SkipString(in);
		if (svg_ver >= kSvgVersion_Cmp_64bit)
			in->ReadInt32(); // game data version
		in->ReadInt32(); // color depth
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
	if (eng_version > EngineVersion ||
		eng_version < SavedgameLowestBackwardCompatVersion) {
		// Engine version is either non-forward or non-backward compatible
		return new SavegameError(kSvgErr_IncompatibleEngine,
			String::FromFormat("Required: %s, supported: %s - %s.", eng_version.LongString.GetCStr(), SavedgameLowestBackwardCompatVersion.LongString.GetCStr(), EngineVersion.LongString.GetCStr()));
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
	String svg_sig = String::FromStreamCount(in.get(), SavegameSource::Signature.GetLength());
	if (svg_sig.Compare(SavegameSource::Signature) == 0) {
		is_new_save = true;
	} else {
		in->Seek(pre_sig_pos, kSeekBegin);
		svg_sig = String::FromStreamCount(in.get(), SavegameSource::LegacySignature.GetLength());
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
	pp.SpeechVOX = play.want_speech;
	pp.MusicVOX = play.separate_music_lib;

	unload_old_room();
	delete raw_saved_screen;
	raw_saved_screen = nullptr;
	remove_screen_overlay(-1);
	is_complete_overlay = 0;
	is_text_overlay = 0;

	// cleanup dynamic sprites
	// NOTE: sprite 0 is a special constant sprite that cannot be dynamic
	for (int i = 1; i < spriteset.GetSpriteSlotCount(); ++i) {
		if (game.SpriteInfos[i].Flags & SPF_DYNAMICALLOC) {
			// do this early, so that it changing guibuts doesn't
			// affect the restored data
			free_dynamic_sprite(i);
		}
	}

	// cleanup GUI backgrounds
	for (int i = 0; i < game.numgui; ++i) {
		delete guibg[i];
		guibg[i] = nullptr;

		if (guibgbmp[i])
			gfxDriver->DestroyDDB(guibgbmp[i]);
		guibgbmp[i] = nullptr;
	}

	// preserve script data sizes and cleanup scripts
	pp.GlScDataSize = gameinst->globaldatasize;
	delete gameinstFork;
	delete gameinst;
	gameinstFork = nullptr;
	gameinst = nullptr;
	pp.ScMdDataSize.resize(numScriptModules);
	for (int i = 0; i < numScriptModules; ++i) {
		pp.ScMdDataSize[i] = moduleInst[i]->globaldatasize;
		delete moduleInstFork[i];
		delete moduleInst[i];
		moduleInst[i] = nullptr;
	}

	play.FreeProperties();
	play.FreeViewportsAndCameras();

	delete roominstFork;
	delete roominst;
	roominstFork = nullptr;
	roominst = nullptr;

	delete dialogScriptsInst;
	dialogScriptsInst = nullptr;

	resetRoomStatuses();
	troom.FreeScriptData();
	troom.FreeProperties();
	free_do_once_tokens();

	// unregister gui controls from API exports
	// TODO: find out why are we doing this here? is this really necessary?
	for (int i = 0; i < game.numgui; ++i) {
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
		auto cam = play.GetRoomCamera(i);
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
		auto view = play.GetRoomViewport(i);
		view->SetID(view_dat.ID);
		view->SetVisible((view_dat.Flags & kSvgViewportVisible) != 0);
		view->SetRect(RectWH(view_dat.Left, view_dat.Top, view_dat.Width, view_dat.Height));
		view->SetZOrder(view_dat.ZOrder);
		// Restore camera link
		int cam_index = view_dat.CamID;
		if (cam_index < 0) continue;
		auto cam = play.GetRoomCamera(cam_index);
		view->LinkCamera(cam);
		cam->LinkToViewport(view);
	}
	play.InvalidateViewportZOrder();
}

// Final processing after successfully restoring from save
HSaveError DoAfterRestore(const PreservedParams &pp, const RestoredData &r_data) {
	// Use a yellow dialog highlight for older game versions
	// CHECKME: it is dubious that this should be right here
	if (loaded_game_file_version < kGameVersion_331)
		play.dialog_options_highlight_color = DIALOG_OPTIONS_HIGHLIGHT_COLOR_DEFAULT;

	// Preserve whether the music vox is available
	play.separate_music_lib = pp.MusicVOX;
	// If they had the vox when they saved it, but they don't now
	if ((pp.SpeechVOX < 0) && (play.want_speech >= 0))
		play.want_speech = (-play.want_speech) - 1;
	// If they didn't have the vox before, but now they do
	else if ((pp.SpeechVOX >= 0) && (play.want_speech < 0))
		play.want_speech = (-play.want_speech) - 1;

	// recache queued clips
	for (int i = 0; i < play.new_music_queue_size; ++i) {
		play.new_music_queue[i].cachedClip = nullptr;
	}

	// restore these to the ones retrieved from the save game
	const size_t dynsurf_num = Math::Min((size_t)MAX_DYNAMIC_SURFACES, r_data.DynamicSurfaces.size());
	for (size_t i = 0; i < dynsurf_num; ++i) {
		dynamicallyCreatedSurfaces[i] = r_data.DynamicSurfaces[i];
	}

	for (int i = 0; i < game.numgui; ++i)
		export_gui_controls(i);
	update_gui_zorder();

	if (create_global_script()) {
		return new SavegameError(kSvgErr_GameObjectInitFailed,
			String::FromFormat("Unable to recreate global script: %s", ccErrorString.GetCStr()));
	}

	// read the global data into the newly created script
	if (r_data.GlobalScript.Data.get())
		memcpy(gameinst->globaldata, r_data.GlobalScript.Data.get(),
			Math::Min((size_t)gameinst->globaldatasize, r_data.GlobalScript.Len));

	// restore the script module data
	for (int i = 0; i < numScriptModules; ++i) {
		if (r_data.ScriptModules[i].Data.get())
			memcpy(moduleInst[i]->globaldata, r_data.ScriptModules[i].Data.get(),
				Math::Min((size_t)moduleInst[i]->globaldatasize, r_data.ScriptModules[i].Len));
	}

	setup_player_character(game.playercharacter);

	// Save some parameters to restore them after room load
	int gstimer = play.gscript_timer;
	int oldx1 = play.mboundx1, oldx2 = play.mboundx2;
	int oldy1 = play.mboundy1, oldy2 = play.mboundy2;

	// disable the queue momentarily
	int queuedMusicSize = play.music_queue_size;
	play.music_queue_size = 0;

	update_polled_stuff_if_runtime();

	// load the room the game was saved in
	if (displayed_room >= 0)
		load_new_room(displayed_room, nullptr);

	update_polled_stuff_if_runtime();

	play.gscript_timer = gstimer;
	// restore the correct room volume (they might have modified
	// it with SetMusicVolume)
	thisroom.Options.MusicVolume = r_data.RoomVolume;

	Mouse::SetMoveLimit(Rect(oldx1, oldy1, oldx2, oldy2));

	set_cursor_mode(r_data.CursorMode);
	set_mouse_cursor(r_data.CursorID);
	if (r_data.CursorMode == MODE_USE)
		SetActiveInventory(playerchar->activeinv);
	// ensure that the current cursor is locked
	spriteset.Precache(game.mcurs[r_data.CursorID].pic);

	set_window_title(play.game_name);

	update_polled_stuff_if_runtime();

	if (displayed_room >= 0) {
		for (int i = 0; i < MAX_ROOM_BGFRAMES; ++i) {
			if (r_data.RoomBkgScene[i]) {
				thisroom.BgFrames[i].Graphic = r_data.RoomBkgScene[i];
			}
		}

		in_new_room = 3; // don't run "enters screen" events
		// now that room has loaded, copy saved light levels in
		for (size_t i = 0; i < MAX_ROOM_REGIONS; ++i) {
			thisroom.Regions[i].Light = r_data.RoomLightLevels[i];
			thisroom.Regions[i].Tint = r_data.RoomTintLevels[i];
		}
		generate_light_table();

		for (size_t i = 0; i < MAX_WALK_AREAS + 1; ++i) {
			thisroom.WalkAreas[i].ScalingFar = r_data.RoomZoomLevels1[i];
			thisroom.WalkAreas[i].ScalingNear = r_data.RoomZoomLevels2[i];
		}

		on_background_frame_change();
	}

	gui_disabled_style = convert_gui_disabled_style(game.options[OPT_DISABLEOFF]);

	// restore the queue now that the music is playing
	play.music_queue_size = queuedMusicSize;

	if (play.digital_master_volume >= 0)
		System_SetVolume(play.digital_master_volume);

	// Run audio clips on channels
	// these two crossfading parameters have to be temporarily reset
	const int cf_in_chan = play.crossfading_in_channel;
	const int cf_out_chan = play.crossfading_out_channel;
	play.crossfading_in_channel = 0;
	play.crossfading_out_channel = 0;

	{
		AudioChannelsLock lock;
		// NOTE: channels are array of MAX_SOUND_CHANNELS+1 size
		for (int i = 0; i <= MAX_SOUND_CHANNELS; ++i) {
			const RestoredData::ChannelInfo &chan_info = r_data.AudioChans[i];
			if (chan_info.ClipID < 0)
				continue;
			if ((size_t)chan_info.ClipID >= game.audioClips.size()) {
				return new SavegameError(kSvgErr_GameObjectInitFailed,
					String::FromFormat("Invalid audio clip index: %d (clip count: %u).", chan_info.ClipID, game.audioClips.size()));
			}
			play_audio_clip_on_channel(i, &game.audioClips[chan_info.ClipID],
				chan_info.Priority, chan_info.Repeat, chan_info.Pos);

			auto *ch = lock.GetChannel(i);
			if (ch != nullptr) {
				ch->set_volume_direct(chan_info.VolAsPercent, chan_info.Vol);
				ch->set_speed(chan_info.Speed);
				ch->set_panning(chan_info.Pan);
				ch->panningAsPercentage = chan_info.PanAsPercent;
				ch->xSource = chan_info.XSource;
				ch->ySource = chan_info.YSource;
				ch->maximumPossibleDistanceAway = chan_info.MaxDist;
			}
		}
		if ((cf_in_chan > 0) && (lock.GetChannel(cf_in_chan) != nullptr))
			play.crossfading_in_channel = cf_in_chan;
		if ((cf_out_chan > 0) && (lock.GetChannel(cf_out_chan) != nullptr))
			play.crossfading_out_channel = cf_out_chan;

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
			PlayAmbientSound(i, r_data.DoAmbient[i], ambient[i].vol, ambient[i].x, ambient[i].y);
	}
	update_directional_sound_vol();

	for (int i = 0; i < game.numgui; ++i) {
		guibg[i] = BitmapHelper::CreateBitmap(guis[i].Width, guis[i].Height, game.GetColorDepth());
		guibg[i] = ReplaceBitmapWithSupportedFormat(guibg[i]);
	}

	recreate_overlay_ddbs();

	guis_need_update = 1;

	RestoreViewportsAndCameras(r_data);

	play.ClearIgnoreInput(); // don't keep ignored input after save restore
	update_polled_stuff_if_runtime();

	pl_run_plugin_hooks(AGSE_POSTRESTOREGAME, 0);

	if (displayed_room < 0) {
		// the restart point, no room was loaded
		load_new_room(playerchar->room, playerchar);
		playerchar->prevroom = -1;

		first_room_initialization();
	}

	if ((play.music_queue_size > 0) && (cachedQueuedMusic == nullptr)) {
		cachedQueuedMusic = load_music_from_disk(play.music_queue[0], 0);
	}

	// Test if the old-style audio had playing music and it was properly loaded
	if (current_music_type > 0) {
		AudioChannelsLock lock;

		if ((crossFading > 0 && !lock.GetChannelIfPlaying(crossFading)) ||
			(crossFading <= 0 && !lock.GetChannelIfPlaying(SCHAN_MUSIC))) {
			current_music_type = 0; // playback failed, reset flag
		}
	}

	set_game_speed(r_data.FPS);

	return HSaveError::None();
}

HSaveError RestoreGameState(PStream in, SavegameVersion svg_version) {
	PreservedParams pp;
	RestoredData r_data;
	DoBeforeRestore(pp);
	HSaveError err;
	if (svg_version >= kSvgVersion_Components)
		err = SavegameComponents::ReadAll(in, svg_version, pp, r_data);
	else
		err = restore_game_data(in.get(), svg_version, pp, r_data);
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
	// Enviroment information
	StrUtil::WriteString("Adventure Game Studio run-time engine", out);
	StrUtil::WriteString(EngineVersion.LongString, out);
	StrUtil::WriteString(game.guid, out);
	StrUtil::WriteString(game.gamename, out);
	StrUtil::WriteString(ResPaths.GamePak.Name, out);
	out->WriteInt32(loaded_game_file_version);
	out->WriteInt32(game.GetColorDepth());
	// User description
	StrUtil::WriteString(user_text, out);
	WriteSaveImage(out, user_image);
}

PStream StartSavegame(const String &filename, const String &user_text, const Bitmap *user_image) {
	Stream *out = Common::File::CreateFile(filename);
	if (!out)
		return PStream();

	// Initialize and write Vista header
	RICH_GAME_MEDIA_HEADER vistaHeader;
	memset(&vistaHeader, 0, sizeof(RICH_GAME_MEDIA_HEADER));
	memcpy(&vistaHeader.dwMagicNumber, RM_MAGICNUMBER, sizeof(int));
	vistaHeader.dwHeaderVersion = 1;
	vistaHeader.dwHeaderSize = sizeof(RICH_GAME_MEDIA_HEADER);
	vistaHeader.dwThumbnailOffsetHigherDword = 0;
	vistaHeader.dwThumbnailOffsetLowerDword = 0;
	vistaHeader.dwThumbnailSize = 0;
	convert_guid_from_text_to_binary(game.guid, &vistaHeader.guidGameId[0]);
	uconvert(game.gamename, U_ASCII, (char *)&vistaHeader.szGameName[0], U_UNICODE, RM_MAXLENGTH);
	uconvert(user_text, U_ASCII, (char *)&vistaHeader.szSaveName[0], U_UNICODE, RM_MAXLENGTH);
	vistaHeader.szLevelName[0] = 0;
	vistaHeader.szComments[0] = 0;
	// MS Windows Vista rich media header
	vistaHeader.WriteToFile(out);

	// Savegame signature
	out->Write(SavegameSource::Signature.GetCStr(), SavegameSource::Signature.GetLength());

	// CHECKME: what is this plugin hook suppose to mean, and if it is called here correctly
	pl_run_plugin_hooks(AGSE_PRESAVEGAME, 0);

	// Write descrition block
	WriteDescription(out, user_text, user_image);
	return PStream(out);
}

void DoBeforeSave() {
	if (play.cur_music_number >= 0) {
		if (IsMusicPlaying() == 0)
			play.cur_music_number = -1;
	}

	if (displayed_room >= 0) {
		// update the current room script's data segment copy
		if (roominst)
			save_room_data_segment();

		// Update the saved interaction variable values
		for (size_t i = 0; i < thisroom.LocalVariables.size() && i < (size_t)MAX_GLOBAL_VARIABLES; ++i)
			croom->interactionVariableValues[i] = thisroom.LocalVariables[i].Value;
	}
}

void SaveGameState(PStream out) {
	DoBeforeSave();
	SavegameComponents::WriteAllCommon(out);
}

} // namespace Engine
} // namespace AGS
} // namespace AGS3
