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
// These functions are restoring old savegames used by engines 3.2.1 - 3.5.0.
// The main point of keeping these today are to be able to compare game
// behavior when running with original/older binaries and latest engine.
// Perhaps the optimal solution would be to have a savegame converter instead.
//
//=============================================================================

#include "ags/lib/std/vector.h"
#include "ags/shared/core/types.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/dialog.h"
#include "ags/engine/ac/button.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/overlay.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/room_status.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/dynobj/cc_serializer.h"
#include "ags/engine/game/savegame.h"
#include "ags/engine/game/savegame_components.h"
#include "ags/engine/game/savegame_internal.h"
#include "ags/engine/gui/animating_gui_button.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/engine/media/audio/audio.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/engine/script/script.h"
#include "ags/shared/script/cc_common.h"
#include "ags/shared/util/aligned_stream.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

static const uint32_t MAGICNUMBER = 0xbeefcafe;

static HSaveError restore_game_head_dynamic_values(Stream *in, RestoredData &r_data) {
	r_data.FPS = in->ReadInt32();
	r_data.CursorMode = in->ReadInt32();
	r_data.CursorID = in->ReadInt32();
	SavegameComponents::ReadLegacyCameraState(in, r_data);
	set_loop_counter(in->ReadInt32());
	return HSaveError::None();
}

static void restore_game_spriteset(Stream *in) {
	// ensure the sprite set is at least as large as it was
	// when the game was saved
	_GP(spriteset).EnlargeTo(in->ReadInt32() - 1); // they saved top_index + 1
	// get serialized dynamic sprites
	int sprnum = in->ReadInt32();
	while (sprnum) {
		unsigned char spriteflag = in->ReadByte();
		add_dynamic_sprite(sprnum, read_serialized_bitmap(in));
		_GP(game).SpriteInfos[sprnum].Flags = spriteflag;
		sprnum = in->ReadInt32();
	}
}

static HSaveError restore_game_scripts(Stream *in, const PreservedParams &pp, RestoredData &r_data) {
	// read the global script data segment
	size_t gdatasize = (uint32_t)in->ReadInt32();
	if (pp.GlScDataSize != gdatasize) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching size of global script data.");
	}
	r_data.GlobalScript.Len = gdatasize;
	r_data.GlobalScript.Data.reset(new char[gdatasize]);
	in->Read(r_data.GlobalScript.Data.get(), gdatasize);

	if ((uint32_t)in->ReadInt32() != _G(numScriptModules)) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of script modules.");
	}
	r_data.ScriptModules.resize(_G(numScriptModules));
	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		size_t module_size = (uint32_t)in->ReadInt32();
		if (pp.ScMdDataSize[i] != module_size) {
			return new SavegameError(kSvgErr_GameContentAssertion, String::FromFormat("Mismatching size of script module data, module %d.", i));
		}
		r_data.ScriptModules[i].Len = module_size;
		r_data.ScriptModules[i].Data.reset(new char[module_size]);
		in->Read(r_data.ScriptModules[i].Data.get(), module_size);
	}
	return HSaveError::None();
}

static void ReadRoomStatus_Aligned(RoomStatus *roomstat, Stream *in) {
	AlignedStream align_s(in, Shared::kAligned_Read);
	roomstat->ReadFromFile_v321(&align_s);
}

static void restore_game_room_state(Stream *in) {
	int vv;

	_G(displayed_room) = in->ReadInt32();

	// read the room state for all the rooms the player has been in
	RoomStatus *roomstat;
	int beenhere;
	for (vv = 0; vv < MAX_ROOMS; vv++) {
		beenhere = in->ReadByte();
		if (beenhere) {
			roomstat = getRoomStatus(vv);
			roomstat->beenhere = beenhere;

			if (roomstat->beenhere) {
				ReadRoomStatus_Aligned(roomstat, in);
				if (roomstat->tsdatasize > 0) {
					roomstat->tsdata = new char[roomstat->tsdatasize + 8];  // JJS: Why allocate 8 additional bytes?
					in->Read(&roomstat->tsdata[0], roomstat->tsdatasize);
				}
			}
		}
	}
}

static void ReadGameState_Aligned(Stream *in, RestoredData &r_data) {
	AlignedStream align_s(in, Shared::kAligned_Read);
	_GP(play).ReadFromSavegame(&align_s, kGSSvgVersion_OldFormat, r_data);
}

static void restore_game_play_ex_data(Stream *in) {
	char rbuffer[200];
	for (size_t i = 0; i < _GP(play).do_once_tokens.size(); ++i) {
		StrUtil::ReadCStr(rbuffer, in, sizeof(rbuffer));
		_GP(play).do_once_tokens[i] = rbuffer;
	}

	in->Seek(_GP(game).numgui * sizeof(int32_t)); // gui_draw_order
}

static void restore_game_play(Stream *in, RestoredData &r_data) {
	int screenfadedout_was = _GP(play).screen_is_faded_out;
	int roomchanges_was = _GP(play).room_changes;

	ReadGameState_Aligned(in, r_data);
	r_data.Cameras[0].Flags = r_data.Camera0_Flags;

	_GP(play).screen_is_faded_out = screenfadedout_was;
	_GP(play).room_changes = roomchanges_was;

	restore_game_play_ex_data(in);
}

static void ReadMoveList_Aligned(Stream *in) {
	AlignedStream align_s(in, Shared::kAligned_Read);
	for (int i = 0; i < _GP(game).numcharacters + MAX_ROOM_OBJECTS_v300 + 1; ++i) {
		_GP(mls)[i].ReadFromFile_Legacy(&align_s);
		align_s.Reset();
	}
}

static void ReadGameSetupStructBase_Aligned(Stream *in) {
	AlignedStream align_s(in, Shared::kAligned_Read);
	_GP(game).GameSetupStructBase::ReadFromFile(&align_s);
}

static void ReadCharacterExtras_Aligned(Stream *in) {
	AlignedStream align_s(in, Shared::kAligned_Read);
	for (int i = 0; i < _GP(game).numcharacters; ++i) {
		_GP(charextra)[i].ReadFromSavegame(&align_s, 0);
		align_s.Reset();
	}
}

static void restore_game_palette(Stream *in) {
	in->ReadArray(&_G(palette)[0], sizeof(RGB), 256);
}

static void restore_game_dialogs(Stream *in) {
	for (int vv = 0; vv < _GP(game).numdialog; vv++)
		in->ReadArrayOfInt32(&_G(dialog)[vv].optionflags[0], MAXTOPICOPTIONS);
}

static void restore_game_more_dynamic_values(Stream *in) {
	_G(mouse_on_iface) = in->ReadInt32();
	in->ReadInt32(); // mouse_on_iface_button
	in->ReadInt32(); // mouse_pushed_iface
	_G(ifacepopped) = in->ReadInt32();
	_G(game_paused) = in->ReadInt32();
}

void ReadAnimatedButtons_Aligned(Stream *in, int num_abuts) {
	AlignedStream align_s(in, Shared::kAligned_Read);
	for (int i = 0; i < num_abuts; ++i) {
		AnimatingGUIButton abtn;
		abtn.ReadFromSavegame(&align_s, 0);
		AddButtonAnimation(abtn);
		align_s.Reset();
	}
}

static HSaveError restore_game_gui(Stream *in, int numGuisWas) {
	HError err = GUI::ReadGUI(in, true);
	if (!err)
		return new SavegameError(kSvgErr_GameObjectInitFailed, err);
	_GP(game).numgui = _GP(guis).size();

	if (numGuisWas != _GP(game).numgui) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of GUI.");
	}

	RemoveAllButtonAnimations();
	int anim_count = in->ReadInt32();
	ReadAnimatedButtons_Aligned(in, anim_count);
	return HSaveError::None();
}

static HSaveError restore_game_audiocliptypes(Stream *in) {
	if ((uint32_t)in->ReadInt32() != _GP(game).audioClipTypes.size()) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Audio Clip Types.");
	}

	for (size_t i = 0; i < _GP(game).audioClipTypes.size(); ++i) {
		_GP(game).audioClipTypes[i].ReadFromFile(in);
	}
	return HSaveError::None();
}

static void restore_game_thisroom(Stream *in, RestoredData &r_data) {
	in->ReadArrayOfInt16(r_data.RoomLightLevels, MAX_ROOM_REGIONS);
	in->ReadArrayOfInt32(r_data.RoomTintLevels, MAX_ROOM_REGIONS);
	in->ReadArrayOfInt16(r_data.RoomZoomLevels1, MAX_WALK_AREAS + 1);
	in->ReadArrayOfInt16(r_data.RoomZoomLevels2, MAX_WALK_AREAS + 1);
}

static void restore_game_ambientsounds(Stream *in, RestoredData &r_data) {
	for (int i = 0; i < MAX_GAME_CHANNELS_v320; ++i) {
		_GP(ambient)[i].ReadFromFile(in);
	}

	for (int bb = NUM_SPEECH_CHANS; bb < MAX_GAME_CHANNELS_v320; bb++) {
		if (_GP(ambient)[bb].channel == 0)
			r_data.DoAmbient[bb] = 0;
		else {
			r_data.DoAmbient[bb] = _GP(ambient)[bb].num;
			_GP(ambient)[bb].channel = 0;
		}
	}
}

static void ReadOverlays_Aligned(Stream *in, std::vector<bool> &has_bitmap, size_t num_overs) {
	AlignedStream align_s(in, Shared::kAligned_Read);
	has_bitmap.resize(num_overs);
	for (size_t i = 0; i < num_overs; ++i) {
		bool has_bm;
		_GP(screenover)[i].ReadFromFile(&align_s, has_bm, 0);
		has_bitmap[i] = has_bm;
		align_s.Reset();
	}
}

static void restore_game_overlays(Stream *in) {
	size_t num_overs = in->ReadInt32();
	_GP(screenover).resize(num_overs);
	std::vector<bool> has_bitmap;
	ReadOverlays_Aligned(in, has_bitmap, num_overs);
	for (size_t i = 0; i < num_overs; ++i) {
		if (has_bitmap[i])
			_GP(screenover)[i].SetImage(read_serialized_bitmap(in), _GP(screenover)[i].offsetX, _GP(screenover)[i].offsetY);
	}
}

static void restore_game_dynamic_surfaces(Stream *in, RestoredData &r_data) {
	// load into a temp array since ccUnserialiseObjects will destroy
	// it otherwise
	r_data.DynamicSurfaces.resize(MAX_DYNAMIC_SURFACES);
	for (int i = 0; i < MAX_DYNAMIC_SURFACES; ++i) {
		if (in->ReadInt8() == 0) {
			r_data.DynamicSurfaces[i] = nullptr;
		} else {
			r_data.DynamicSurfaces[i] = read_serialized_bitmap(in);
		}
	}
}

static void restore_game_displayed_room_status(Stream *in, RestoredData &r_data) {
	int bb;
	for (bb = 0; bb < MAX_ROOM_BGFRAMES; bb++)
		r_data.RoomBkgScene[bb].reset();

	if (_G(displayed_room) >= 0) {

		for (bb = 0; bb < MAX_ROOM_BGFRAMES; bb++) {
			r_data.RoomBkgScene[bb] = nullptr;
			if (_GP(play).raw_modified[bb]) {
				r_data.RoomBkgScene[bb].reset(read_serialized_bitmap(in));
			}
		}
		bb = in->ReadInt32();

		if (bb)
			_G(raw_saved_screen) = read_serialized_bitmap(in);

		// get the current troom, in case they save in room 600 or whatever
		ReadRoomStatus_Aligned(&_GP(troom), in);

		if (_GP(troom).tsdatasize > 0) {
			_GP(troom).tsdata = new char[_GP(troom).tsdatasize + 5];
			in->Read(&_GP(troom).tsdata[0], _GP(troom).tsdatasize);
		} else
			_GP(troom).tsdata = nullptr;
	}
}

static HSaveError restore_game_globalvars(Stream *in) {
	if (in->ReadInt32() != _G(numGlobalVars)) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Restore game error: mismatching number of Global Variables.");
	}

	for (int i = 0; i < _G(numGlobalVars); ++i) {
		_G(globalvars)[i].Read(in);
	}
	return HSaveError::None();
}

static HSaveError restore_game_views(Stream *in) {
	if (in->ReadInt32() != _GP(game).numviews) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Views.");
	}

	for (int bb = 0; bb < _GP(game).numviews; bb++) {
		for (int cc = 0; cc < _GP(views)[bb].numLoops; cc++) {
			for (int dd = 0; dd < _GP(views)[bb].loops[cc].numFrames; dd++) {
				_GP(views)[bb].loops[cc].frames[dd].sound = in->ReadInt32();
				_GP(views)[bb].loops[cc].frames[dd].pic = in->ReadInt32();
			}
		}
	}
	return HSaveError::None();
}

static HSaveError restore_game_audioclips_and_crossfade(Stream *in, RestoredData &r_data) {
	if ((uint32_t)in->ReadInt32() != _GP(game).audioClips.size()) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Audio Clips.");
	}

	for (int i = 0; i < TOTAL_AUDIO_CHANNELS_v320; ++i) {
		RestoredData::ChannelInfo &chan_info = r_data.AudioChans[i];
		chan_info.Pos = 0;
		chan_info.ClipID = in->ReadInt32();
		if (chan_info.ClipID >= 0) {
			if ((size_t)chan_info.ClipID >= _GP(game).audioClips.size()) {
				return new SavegameError(kSvgErr_GameObjectInitFailed, "Invalid audio clip index.");
			}

			chan_info.Pos = in->ReadInt32();
			if (chan_info.Pos < 0)
				chan_info.Pos = 0;
			chan_info.Priority = in->ReadInt32();
			chan_info.Repeat = in->ReadInt32();
			chan_info.Vol = in->ReadInt32();
			in->ReadInt32(); // unused
			chan_info.VolAsPercent = in->ReadInt32();
			chan_info.Pan = in->ReadInt32();
			chan_info.Speed = 1000;
			if (_G(loaded_game_file_version) >= kGameVersion_340_2)
				chan_info.Speed = in->ReadInt32();
		}
	}
	_G(crossFading) = in->ReadInt32();
	_G(crossFadeVolumePerStep) = in->ReadInt32();
	_G(crossFadeStep) = in->ReadInt32();
	_G(crossFadeVolumeAtStart) = in->ReadInt32();
	return HSaveError::None();
}

HSaveError restore_save_data_v321(Stream *in, const PreservedParams &pp, RestoredData &r_data) {
	HSaveError err = restore_game_head_dynamic_values(in, r_data);
	if (!err)
		return err;
	restore_game_spriteset(in);

	err = restore_game_scripts(in, pp, r_data);
	if (!err)
		return err;
	restore_game_room_state(in);
	restore_game_play(in, r_data);
	ReadMoveList_Aligned(in);

	// save pointer members before reading
	char *gswas = _GP(game).globalscript;
	ccScript *compsc = _GP(game).compiled_script;
	CharacterInfo *chwas = _GP(game).chars;
	WordsDictionary *olddict = _GP(game).dict;
	std::vector<String> mesbk(MAXGLOBALMES);
	int numchwas = _GP(game).numcharacters;
	for (size_t i = 0; i < MAXGLOBALMES; ++i)
		mesbk[i] = _GP(game).messages[i];
	int numdiwas = _GP(game).numdialog;
	int numinvwas = _GP(game).numinvitems;
	int numviewswas = _GP(game).numviews;
	int numGuisWas = _GP(game).numgui;

	ReadGameSetupStructBase_Aligned(in);

	// Delete unneeded data
	// TODO: reorganize this (may be solved by optimizing safe format too)
	delete[] _GP(game).load_messages;
	_GP(game).load_messages = nullptr;

	if (_GP(game).numdialog != numdiwas) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Dialogs.");
	}
	if (numchwas != _GP(game).numcharacters) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Characters.");
	}
	if (numinvwas != _GP(game).numinvitems) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Inventory Items.");
	}
	if (_GP(game).numviews != numviewswas) {
		return new SavegameError(kSvgErr_GameContentAssertion, "Mismatching number of Views.");
	}

	_GP(game).ReadFromSaveGame_v321(in, gswas, compsc, chwas, olddict, mesbk);

	// Modified custom properties are read separately to keep existing save format
	_GP(play).ReadCustomProperties_v340(in);

	ReadCharacterExtras_Aligned(in);
	restore_game_palette(in);
	restore_game_dialogs(in);
	restore_game_more_dynamic_values(in);
	err = restore_game_gui(in, numGuisWas);
	if (!err)
		return err;
	err = restore_game_audiocliptypes(in);
	if (!err)
		return err;
	restore_game_thisroom(in, r_data);
	restore_game_ambientsounds(in, r_data);
	restore_game_overlays(in);
	restore_game_dynamic_surfaces(in, r_data);
	restore_game_displayed_room_status(in, r_data);
	err = restore_game_globalvars(in);
	if (!err)
		return err;
	err = restore_game_views(in);
	if (!err)
		return err;

	if (static_cast<uint32_t>(in->ReadInt32()) != (MAGICNUMBER + 1)) {
		return new SavegameError(kSvgErr_InconsistentFormat, "MAGICNUMBER not found before Audio Clips.");
	}

	err = restore_game_audioclips_and_crossfade(in, r_data);
	if (!err)
		return err;

	auto pluginFileHandle = AGSE_RESTOREGAME;
	pl_set_file_handle(pluginFileHandle, in);
	pl_run_plugin_hooks(AGSE_RESTOREGAME, pluginFileHandle);
	pl_clear_file_handle();
	if (static_cast<uint32_t>(in->ReadInt32()) != MAGICNUMBER)
		return new SavegameError(kSvgErr_InconsistentPlugin);

	// save the new room music vol for later use
	r_data.RoomVolume = (RoomVolumeMod)in->ReadInt32();

	if (ccUnserializeAllObjects(in, &_GP(ccUnserializer))) {
		return new SavegameError(kSvgErr_GameObjectInitFailed,
		                         String::FromFormat("Managed pool deserialization failed: %s.", cc_get_error().ErrorString.GetCStr()));
	}

	// preserve legacy music type setting
	_G(current_music_type) = in->ReadInt32();

	return HSaveError::None();
}

} // namespace AGS3
