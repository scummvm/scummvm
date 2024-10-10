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

#include "common/std/vector.h"
#include "ags/shared/core/types.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/dialog.h"
#include "ags/engine/ac/button.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/overlay.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/room_status.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/dynobj/cc_serializer.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/game/savegame.h"
#include "ags/engine/game/savegame_components.h"
#include "ags/engine/game/savegame_internal.h"
#include "ags/engine/gui/animating_gui_button.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_inv.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/shared/gui/gui_listbox.h"
#include "ags/shared/gui/gui_slider.h"
#include "ags/shared/gui/gui_textbox.h"
#include "ags/engine/media/audio/audio.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/engine/script/script.h"
#include "ags/shared/script/cc_common.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

static const uint32_t MAGICNUMBER = 0xbeefcafe;

inline bool AssertGameContent(HSaveError &err, int game_val, int sav_val, const char *content_name, bool warn_only = false) {
	if (game_val != sav_val) {
		String msg = String::FromFormat("Mismatching number of %s (game: %d, save: %d).", content_name, game_val, sav_val);
		if (warn_only)
			Debug::Printf(kDbgMsg_Warn, "WARNING: restored save may be incompatible: %s", msg.GetCStr());
		else
			err = new SavegameError(kSvgErr_GameContentAssertion, msg);
	}
	return warn_only || (game_val == sav_val);
}

template<typename TObject>
inline bool AssertAndCopyGameContent(const std::vector<TObject> &old_list, std::vector<TObject> &new_list,
									 HSaveError &err, const char *content_name, bool warn_only = false) {
	if (!AssertGameContent(err, old_list.size(), new_list.size(), content_name, warn_only))
		return false;

	if (new_list.size() < old_list.size()) {
		size_t copy_at = new_list.size();
		new_list.resize(old_list.size());
		Common::copy(old_list.begin() + copy_at, old_list.end(), new_list.begin() + copy_at);
	}
	return true;
}

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
		unsigned char spriteflag = in->ReadInt8();
		std::unique_ptr<Bitmap> image(read_serialized_bitmap(in));
		add_dynamic_sprite(sprnum, std::move(image));
		_GP(game).SpriteInfos[sprnum].Flags = spriteflag;
		sprnum = in->ReadInt32();
	}
}

static HSaveError restore_game_scripts(Stream *in, const PreservedParams &pp, RestoredData &r_data) {
	HSaveError err;
	// read the global script data segment
	size_t gdatasize = (uint32_t)in->ReadInt32();
	if (!AssertGameContent(err, pp.GlScDataSize, gdatasize, "global script data"))
		return err;
	r_data.GlobalScript.Len = gdatasize;
	r_data.GlobalScript.Data.resize(gdatasize);
	if (gdatasize > 0)
		in->Read(&r_data.GlobalScript.Data.front(), gdatasize);

	uint32_t num_modules = (uint32_t)in->ReadInt32();
	if (!AssertGameContent(err, _G(numScriptModules), num_modules, "Script Modules"))
		return err;
	r_data.ScriptModules.resize(_G(numScriptModules));
	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		size_t module_size = (uint32_t)in->ReadInt32();
		if (pp.ScMdDataSize[i] != module_size) {
			return new SavegameError(kSvgErr_GameContentAssertion, String::FromFormat("Mismatching size of script module data, module %zu.", i));
		}
		r_data.ScriptModules[i].Len = module_size;
		r_data.ScriptModules[i].Data.resize(module_size);
		if (module_size > 0)
			in->Read(&r_data.ScriptModules[i].Data.front(), module_size);
	}
	return HSaveError::None();
}

static void restore_game_room_state(Stream *in, GameDataVersion data_ver) {
	_G(displayed_room) = in->ReadInt32();

	// read the room state for all the rooms the player has been in
	for (int vv = 0; vv < MAX_ROOMS; vv++) {
		int beenhere = in->ReadInt8();
		if (beenhere) {
			RoomStatus *roomstat = getRoomStatus(vv);
			roomstat->beenhere = beenhere;

			if (roomstat->beenhere) {
				roomstat->ReadFromSavegame_v321(in, data_ver);
				if (roomstat->tsdatasize > 0) {
					roomstat->tsdata.resize(roomstat->tsdatasize);
					in->Read(roomstat->tsdata.data(), roomstat->tsdatasize);
				}
			}
		}
	}
}

static void restore_game_play(Stream *in, GameDataVersion data_ver, RestoredData &r_data) {
	int screenfadedout_was = _GP(play).screen_is_faded_out;
	int roomchanges_was = _GP(play).room_changes;

	_GP(play).ReadFromSavegame(in, data_ver, kGSSvgVersion_OldFormat, r_data);
	r_data.Cameras[0].Flags = r_data.Camera0_Flags;

	_GP(play).screen_is_faded_out = screenfadedout_was;
	_GP(play).room_changes = roomchanges_was;

	char rbuffer[200]; // old doonceonly token length
	for (size_t i = 0; i < r_data.DoOnceCount; ++i) {
		StrUtil::ReadCStr(rbuffer, in, sizeof(rbuffer));
		_GP(play).do_once_tokens.insert(rbuffer);
	}

	// Skip gui_draw_order (no longer applied from saves)
	in->Seek(_GP(game).numgui * sizeof(int32_t));
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

static HSaveError restore_game_gui(Stream *in) {
	// Legacy saves allowed to resize gui lists, and stored full gui data
	// (could be unintentional side effect). Here we emulate this for
	// upgraded games by letting read **less** data from saves, and copying
	// missing elements from reserved game data.
	const std::vector<GUIMain> res_guis = std::move(_GP(guis));
	const std::vector<GUIButton> res_guibuts = std::move(_GP(guibuts));
	const std::vector<GUIInvWindow> res_guiinv = std::move(_GP(guiinv));
	const std::vector<GUILabel> res_guilabels = std::move(_GP(guilabels));
	const std::vector<GUIListBox> res_guilist = std::move(_GP(guilist));
	const std::vector<GUISlider> res_guislider = std::move(_GP(guislider));
	const std::vector<GUITextBox> res_guitext = std::move(_GP(guitext));

	HError guierr = GUI::ReadGUI(in, true);
	if (!guierr)
		return new SavegameError(kSvgErr_GameObjectInitFailed, guierr);

	HSaveError err;
	const bool warn_only = _GP(usetup).legacysave_let_gui_diff;
	if (!AssertAndCopyGameContent(res_guis, _GP(guis), err, "GUIs", warn_only) ||
		!AssertAndCopyGameContent(res_guibuts, _GP(guibuts), err, "GUI Buttons", warn_only) ||
		!AssertAndCopyGameContent(res_guiinv, _GP(guiinv), err, "GUI InvWindows", warn_only) ||
		!AssertAndCopyGameContent(res_guilabels, _GP(guilabels), err, "GUI Labels", warn_only) ||
		!AssertAndCopyGameContent(res_guilist, _GP(guilist), err, "GUI ListBoxes", warn_only) ||
		!AssertAndCopyGameContent(res_guislider, _GP(guislider), err, "GUI Sliders", warn_only) ||
		!AssertAndCopyGameContent(res_guitext, _GP(guitext), err, "GUI TextBoxes", warn_only))

		return err;
	GUI::RebuildGUI(); // rebuild guis in case they were copied from reserved game data
	_GP(game).numgui = _GP(guis).size();

	int anim_count = in->ReadInt32();
	for (int i = 0; i < anim_count; ++i) {
		AnimatingGUIButton abtn;
		abtn.ReadFromSavegame(in, 0);
		AddButtonAnimation(abtn);
	}
	return HSaveError::None();
}

static HSaveError restore_game_audiocliptypes(Stream *in) {
	HSaveError err;
	if (!AssertGameContent(err, _GP(game).audioClipTypes.size(), (uint32_t)in->ReadInt32(), "Audio Clip Types"))
		return err;

	for (size_t i = 0; i < _GP(game).audioClipTypes.size(); ++i) {
		_GP(game).audioClipTypes[i].ReadFromFile(in);
	}
	return HSaveError::None();
}

static void restore_game_thisroom(Stream *in, RestoredData &r_data) {
	in->ReadArrayOfInt16(r_data.RoomLightLevels, MAX_ROOM_REGIONS);
	in->ReadArrayOfInt32(r_data.RoomTintLevels, MAX_ROOM_REGIONS);
	in->ReadArrayOfInt16(r_data.RoomZoomLevels1, MAX_WALK_AREAS);
	in->ReadArrayOfInt16(r_data.RoomZoomLevels2, MAX_WALK_AREAS);
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

static void ReadOverlays_Aligned(Stream *in, std::vector<int> &has_bitmap, size_t num_overs) {
	// Remember that overlay indexes may be non-sequential
	auto &overs = get_overlays();
	for (size_t i = 0; i < num_overs; ++i) {
		bool has_bm;
		ScreenOverlay over;
		over.ReadFromSavegame(in, has_bm, -1);
		if (over.type < 0)
			continue; // safety abort
		if (overs.size() <= static_cast<uint32_t>(over.type))
			overs.resize(over.type + 1);
		overs[over.type] = std::move(over);
		if (has_bm)
			has_bitmap.push_back(over.type);
	}
}

static void restore_game_overlays(Stream *in, RestoredData &r_data) {
	size_t num_overs = in->ReadInt32();
	// Remember that overlay indexes may be not sequential,
	// the vector may be resized during read
	auto &overs = get_overlays();
	overs.resize(num_overs);
	std::vector<int> has_bitmap;
	ReadOverlays_Aligned(in, has_bitmap, num_overs);
	for (auto over_id : has_bitmap) {
		r_data.OverlayImages[over_id].reset(read_serialized_bitmap(in));
	}
}

static void restore_game_dynamic_surfaces(Stream *in, RestoredData &r_data) {
	// load into a temp array since ccUnserialiseObjects will destroy
	// it otherwise
	r_data.DynamicSurfaces.resize(MAX_DYNAMIC_SURFACES);
	for (int i = 0; i < MAX_DYNAMIC_SURFACES; ++i) {
		if (in->ReadInt8() == 0) {
			r_data.DynamicSurfaces[i].reset();
		} else {
			r_data.DynamicSurfaces[i].reset(read_serialized_bitmap(in));
		}
	}
}

static void restore_game_displayed_room_status(Stream *in, GameDataVersion data_ver, RestoredData &r_data) {
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
			_G(raw_saved_screen).reset(read_serialized_bitmap(in));

		// get the current troom, in case they save in room 600 or whatever
		_GP(troom).ReadFromSavegame_v321(in, data_ver);

		if (_GP(troom).tsdatasize > 0) {
			_GP(troom).tsdata.resize(_GP(troom).tsdatasize);
			in->Read(_GP(troom).tsdata.data(), _GP(troom).tsdatasize);
		} else
			_GP(troom).tsdata.clear();
	}
}

static HSaveError restore_game_globalvars(Stream *in) {
	HSaveError err;
	if (!AssertGameContent(err, _G(numGlobalVars), in->ReadInt32(), "Global Variables"))
		return err;

	for (int i = 0; i < _G(numGlobalVars); ++i) {
		_G(globalvars)[i].Read(in);
	}
	return HSaveError::None();
}

static HSaveError restore_game_views(Stream *in) {
	HSaveError err;
	if (!AssertGameContent(err, _GP(game).numviews, in->ReadInt32(), "Views"))
		return err;

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

static HSaveError restore_game_audio_and_crossfade(Stream *in, GameDataVersion data_ver, RestoredData &r_data) {
	in->ReadInt32(); // audio clips count, ignore

	for (int i = 0; i < TOTAL_AUDIO_CHANNELS_v320; ++i) {
		RestoredData::ChannelInfo &chan_info = r_data.AudioChans[i];
		chan_info.Pos = 0;
		chan_info.ClipID = in->ReadInt32();
		if (chan_info.ClipID >= 0) {
			if ((size_t)chan_info.ClipID >= _GP(game).audioClips.size()) {
				return new SavegameError(kSvgErr_GameObjectInitFailed,
										 String::FromFormat("Invalid audio clip index %zu (valid range is 0..%zu)",
															(size_t)chan_info.ClipID, _GP(game).audioClips.size() - 1));
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
			if (data_ver >= kGameVersion_340_2)
				chan_info.Speed = in->ReadInt32();
		}
	}
	_G(crossFading) = in->ReadInt32();
	_G(crossFadeVolumePerStep) = in->ReadInt32();
	_G(crossFadeStep) = in->ReadInt32();
	_G(crossFadeVolumeAtStart) = in->ReadInt32();
	return HSaveError::None();
}

HSaveError restore_save_data_v321(Stream *in, GameDataVersion data_ver, const PreservedParams &pp, RestoredData &r_data) {
	HSaveError err = restore_game_head_dynamic_values(in, r_data);
	if (!err)
		return err;
	restore_game_spriteset(in);

	err = restore_game_scripts(in, pp, r_data);
	if (!err)
		return err;
	restore_game_room_state(in, data_ver);
	restore_game_play(in, data_ver, r_data);
	// Global character movelists
	for (int i = 0; i < _GP(game).numcharacters + MAX_ROOM_OBJECTS_v300 + 1; ++i) {
		_GP(mls)[i].ReadFromSavegame_Legacy(in);
	}

	// List of game objects, used to compare with the save contents
	struct ObjectCounts {
		int CharacterCount = _GP(game).numcharacters;
		int DialogCount = _GP(game).numdialog;
		int InvItemCount = _GP(game).numinvitems;
		int ViewCount = _GP(game).numviews;
	} objwas;

	GameSetupStruct::SerializeInfo info;
	_GP(game).GameSetupStructBase::ReadFromFile(in, data_ver, info);

	if (!AssertGameContent(err, objwas.CharacterCount, _GP(game).numcharacters, "Characters") ||
		!AssertGameContent(err, objwas.DialogCount, _GP(game).numdialog, "Dialogs") ||
		!AssertGameContent(err, objwas.InvItemCount, _GP(game).numinvitems, "Inventory Items") ||
		!AssertGameContent(err, objwas.ViewCount, _GP(game).numviews, "Views"))
		return err;

	_GP(game).ReadFromSaveGame_v321(in);

	// Modified custom properties are read separately to keep existing save format
	_GP(play).ReadCustomProperties_v340(in, data_ver);

	// Character extras (runtime only data)
	for (int i = 0; i < _GP(game).numcharacters; ++i) {
		_GP(charextra)[i].ReadFromSavegame(in, kCharSvgVersion_Initial);
	}
	restore_game_palette(in);
	restore_game_dialogs(in);
	restore_game_more_dynamic_values(in);
	err = restore_game_gui(in);
	if (!err)
		return err;
	err = restore_game_audiocliptypes(in);
	if (!err)
		return err;
	restore_game_thisroom(in, r_data);
	restore_game_ambientsounds(in, r_data);
	restore_game_overlays(in, r_data);
	restore_game_dynamic_surfaces(in, r_data);
	restore_game_displayed_room_status(in, data_ver, r_data);
	err = restore_game_globalvars(in);
	if (!err)
		return err;
	err = restore_game_views(in);
	if (!err)
		return err;

	if (static_cast<uint32_t>(in->ReadInt32()) != (MAGICNUMBER + 1)) {
		return new SavegameError(kSvgErr_InconsistentFormat, "Audio section header expected but not found.");
	}

	err = restore_game_audio_and_crossfade(in, data_ver, r_data);
	if (!err)
		return err;

	ReadPluginSaveData(in, kPluginSvgVersion_Initial, SIZE_MAX);
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
