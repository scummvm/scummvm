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

#include "common/std/algorithm.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/game_version.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/ac/dynobj/script_camera.h"
#include "ags/engine/ac/dynobj/script_system.h"
#include "ags/engine/ac/dynobj/script_viewport.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/shared/game/custom_properties.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/game/savegame_internal.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/shared/util/string_utils.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

GameState::GameState() {
}

bool GameState::IsAutoRoomViewport() const {
	return _isAutoRoomViewport;
}

void GameState::SetAutoRoomViewport(bool on) {
	_isAutoRoomViewport = on;
}

void GameState::SetMainViewport(const Rect &viewport) {
	_mainViewport = viewport;
	_GP(mouse).UpdateGraphicArea();
	_GP(scsystem).viewport_width = game_to_data_coord(_mainViewport.GetWidth());
	_GP(scsystem).viewport_height = game_to_data_coord(_mainViewport.GetHeight());
	_mainViewportHasChanged = true;
}

const Rect &GameState::GetMainViewport() const {
	return _mainViewport;
}

const Rect &GameState::GetUIViewport() const {
	return _uiViewport;
}

SpriteTransform GameState::GetGlobalTransform(bool full_frame_rend) const {
	// NOTE: shake_screen is not applied to the sprite batches,
	// but only as a final render factor (optimization)
	// TODO: also add global flip to the same transform, instead of passing separately?
	return SpriteTransform(_mainViewport.Left, _mainViewport.Top +
												   shake_screen_yoff * static_cast<int>(full_frame_rend));
}

PViewport GameState::GetRoomViewport(int index) const {
	return _roomViewports[index];
}

const std::vector<PViewport> &GameState::GetRoomViewportsZOrdered() const {
	return _roomViewportsSorted;
}

PViewport GameState::GetRoomViewportAt(int x, int y) const {
	// We iterate backwards, because in AGS low z-order means bottom
	for (auto it = _roomViewportsSorted.rbegin(); it != _roomViewportsSorted.rend(); ++it)
		if ((*it)->IsVisible() && (*it)->GetRect().IsInside(x, y))
			return *it;
	return nullptr;
}

Rect GameState::GetRoomViewportAbs(int index) const {
	return Rect::MoveBy(_roomViewports[index]->GetRect(), _mainViewport.Left, _mainViewport.Top);
}

void GameState::SetUIViewport(const Rect &viewport) {
	_uiViewport = viewport;
}

static bool ViewportZOrder(const PViewport e1, const PViewport e2) {
	return e1->GetZOrder() < e2->GetZOrder();
}

void GameState::UpdateViewports() {
	if (_mainViewportHasChanged) {
		on_mainviewport_changed();
		_mainViewportHasChanged = false;
	}
	if (_roomViewportZOrderChanged) {
		auto old_sort = _roomViewportsSorted;
		_roomViewportsSorted = _roomViewports;
		std::sort(_roomViewportsSorted.begin(), _roomViewportsSorted.end(), ViewportZOrder);
		for (size_t i = 0; i < _roomViewportsSorted.size(); ++i) {
			if (i >= old_sort.size() || _roomViewportsSorted[i] != old_sort[i])
				_roomViewportsSorted[i]->SetChangedVisible();
		}
		_roomViewportZOrderChanged = false;
	}
	size_t vp_changed = SIZE_MAX;
	for (size_t i = _roomViewportsSorted.size(); i-- > 0;) {
		auto vp = _roomViewportsSorted[i];
		if (vp->HasChangedSize() || vp->HasChangedPosition() || vp->HasChangedVisible()) {
			vp_changed = i;
			on_roomviewport_changed(vp.get());
			vp->ClearChangedFlags();
		}
	}
	if (vp_changed != SIZE_MAX)
		detect_roomviewport_overlaps(vp_changed);
	for (auto cam : _roomCameras) {
		if (cam->HasChangedSize() || cam->HasChangedPosition()) {
			on_roomcamera_changed(cam.get());
			cam->ClearChangedFlags();
		}
	}
}

void GameState::InvalidateViewportZOrder() {
	_roomViewportZOrderChanged = true;
}

PCamera GameState::GetRoomCamera(int index) const {
	return _roomCameras[index];
}

void GameState::UpdateRoomCameras() {
	for (size_t i = 0; i < _roomCameras.size(); ++i)
		UpdateRoomCamera(i);
}

void GameState::UpdateRoomCamera(int index) {
	auto cam = _roomCameras[index];
	const Rect &rc = cam->GetRect();
	const Size real_room_sz = Size(data_to_game_coord(_GP(thisroom).Width), data_to_game_coord(_GP(thisroom).Height));
	if ((real_room_sz.Width > rc.GetWidth()) || (real_room_sz.Height > rc.GetHeight())) {
		// TODO: split out into Camera Behavior
		if (!cam->IsLocked()) {
			int x = data_to_game_coord(_G(playerchar)->x) - rc.GetWidth() / 2;
			int y = data_to_game_coord(_G(playerchar)->y) - rc.GetHeight() / 2;
			cam->SetAt(x, y);
		}
	}
}

Point GameState::RoomToScreen(int roomx, int roomy) {
	return _roomViewports[0]->RoomToScreen(roomx, roomy, false).first;
}

int GameState::RoomToScreenX(int roomx) {
	return _roomViewports[0]->RoomToScreen(roomx, 0, false).first.X;
}

int GameState::RoomToScreenY(int roomy) {
	return _roomViewports[0]->RoomToScreen(0, roomy, false).first.Y;
}

VpPoint GameState::ScreenToRoomImpl(int scrx, int scry, int view_index, bool clip_viewport, bool convert_cam_to_data) {
	PViewport view;
	if (view_index < 0) {
		view = GetRoomViewportAt(scrx, scry);
		if (!view) {
			if (clip_viewport)
				return std::make_pair(Point(), -1);
			view = _roomViewports[0]; // use primary viewport
		}
	} else {
		view = _roomViewports[view_index];
	}
	return view->ScreenToRoom(scrx, scry, clip_viewport, convert_cam_to_data);
}

VpPoint GameState::ScreenToRoom(int scrx, int scry, bool restrict) {
	if (_GP(game).options[OPT_BASESCRIPTAPI] >= kScriptAPI_v3507)
		return ScreenToRoomImpl(scrx, scry, -1, restrict, false);
	return ScreenToRoomImpl(scrx, scry, 0, false, false);
}

VpPoint GameState::ScreenToRoomDivDown(int scrx, int scry) {
	if (_GP(game).options[OPT_BASESCRIPTAPI] >= kScriptAPI_v3507)
		return ScreenToRoomImpl(scrx, scry, -1, true, true);
	return ScreenToRoomImpl(scrx, scry, 0, false, true);
}

void GameState::CreatePrimaryViewportAndCamera() {
	if (_roomViewports.size() == 0) {
		_GP(play).CreateRoomViewport();
		_GP(play).RegisterRoomViewport(0);
	}
	if (_roomCameras.size() == 0) {
		_GP(play).CreateRoomCamera();
		_GP(play).RegisterRoomCamera(0);
	}
	_roomViewports[0]->LinkCamera(_roomCameras[0]);
	_roomCameras[0]->LinkToViewport(_roomViewports[0]);
}

PViewport GameState::CreateRoomViewport() {
	int index = (int)_roomViewports.size();
	PViewport viewport(new Viewport());
	viewport->SetID(index);
	viewport->SetRect(_mainViewport);
	_roomViewports.push_back(viewport);
	_scViewportHandles.push_back(0);
	_roomViewportsSorted.push_back(viewport);
	_roomViewportZOrderChanged = true;
	on_roomviewport_created(index);
	return viewport;
}

ScriptViewport *GameState::RegisterRoomViewport(int index, int32_t handle) {
	if (index < 0 || (size_t)index >= _roomViewports.size())
		return nullptr;
	auto scview = new ScriptViewport(index);
	if (handle == 0) {
		handle = ccRegisterManagedObject(scview, scview);
		ccAddObjectReference(handle); // one reference for the GameState
	} else {
		ccRegisterUnserializedObject(handle, scview, scview);
	}
	_scViewportHandles[index] = handle; // save handle for us
	return scview;
}

void GameState::DeleteRoomViewport(int index) {
	if (index < 0 || (size_t)index >= _roomViewports.size())
		return;
	auto handle = _scViewportHandles[index];
	auto scobj = const_cast<ScriptViewport*>((const ScriptViewport*)ccGetObjectAddressFromHandle(handle));
	if (scobj) {
		scobj->Invalidate();
		ccReleaseObjectReference(handle);
	}
	auto cam = _roomViewports[index]->GetCamera();
	if (cam)
		cam->UnlinkFromViewport(index);
	_roomViewports.erase(_roomViewports.begin() + index);
	_scViewportHandles.erase(_scViewportHandles.begin() + index);
	for (size_t i = index; i < _roomViewports.size(); ++i) {
		_roomViewports[i]->SetID(i);
		handle = _scViewportHandles[index];
		scobj = const_cast<ScriptViewport*>((const ScriptViewport*)ccGetObjectAddressFromHandle(handle));
		if (scobj)
			scobj->SetID(i);
	}
	for (size_t i = 0; i < _roomViewportsSorted.size(); ++i) {
		if (_roomViewportsSorted[i]->GetID() == index) {
			_roomViewportsSorted.erase(_roomViewportsSorted.begin() + i);
			break;
		}
	}
	on_roomviewport_deleted(index);
}

int GameState::GetRoomViewportCount() const {
	return (int)_roomViewports.size();
}

PCamera GameState::CreateRoomCamera() {
	int index = (int)_roomCameras.size();
	PCamera camera(new Camera());
	camera->SetID(index);
	camera->SetAt(0, 0);
	camera->SetSize(_mainViewport.GetSize());
	_scCameraHandles.push_back(0);
	_roomCameras.push_back(camera);
	return camera;
}

ScriptCamera *GameState::RegisterRoomCamera(int index, int32_t handle) {
	if (index < 0 || (size_t)index >= _roomCameras.size())
		return nullptr;
	auto sccamera = new ScriptCamera(index);
	if (handle == 0) {
		handle = ccRegisterManagedObject(sccamera, sccamera);
		ccAddObjectReference(handle); // one reference for the GameState
	} else {
		ccRegisterUnserializedObject(handle, sccamera, sccamera);
	}
	_scCameraHandles[index] = handle;
	return sccamera;
}

void GameState::DeleteRoomCamera(int index) {
	if (index < 0 || (size_t)index >= _roomCameras.size())
		return;
	auto handle = _scCameraHandles[index];
	auto scobj = const_cast<ScriptCamera*>((const ScriptCamera*)ccGetObjectAddressFromHandle(handle));
	if (scobj) {
		scobj->Invalidate();
		ccReleaseObjectReference(handle);
	}
	for (auto &viewref : _roomCameras[index]->GetLinkedViewports()) {
		auto view = viewref.lock();
		if (view)
			view->LinkCamera(nullptr);
	}
	_roomCameras.erase(_roomCameras.begin() + index);
	_scCameraHandles.erase(_scCameraHandles.begin() + index);
	for (size_t i = index; i < _roomCameras.size(); ++i) {
		_roomCameras[i]->SetID(i);
		handle = _scCameraHandles[index];
		scobj = const_cast<ScriptCamera*>((const ScriptCamera*)ccGetObjectAddressFromHandle(handle));
		if (scobj)
			scobj->SetID(i);
	}
}

int GameState::GetRoomCameraCount() const {
	return (int)_roomCameras.size();
}

ScriptViewport *GameState::GetScriptViewport(int index) {
	if (index < 0 || (size_t)index >= _roomViewports.size())
		return nullptr;
	return const_cast<ScriptViewport*>((const ScriptViewport*)ccGetObjectAddressFromHandle(_scViewportHandles[index]));
}

ScriptCamera *GameState::GetScriptCamera(int index) {
	if (index < 0 || (size_t)index >= _roomCameras.size())
		return nullptr;
	return const_cast<ScriptCamera*>((const ScriptCamera*)ccGetObjectAddressFromHandle(_scCameraHandles[index]));
}

bool GameState::IsIgnoringInput() const {
	return AGS_Clock::now() < _ignoreUserInputUntilTime;
}

void GameState::SetIgnoreInput(int timeout_ms) {
	if (AGS_Clock::now() + std::chrono::milliseconds(timeout_ms) > _ignoreUserInputUntilTime)
		_ignoreUserInputUntilTime = AGS_Clock::now() + std::chrono::milliseconds(timeout_ms);
}

void GameState::ClearIgnoreInput() {
	_ignoreUserInputUntilTime = AGS_Clock::now();
}

void GameState::SetWaitSkipResult(int how, int data) {
	wait_counter = 0;
	wait_skipped_by = how;
	wait_skipped_by_data = data;
}

int GameState::GetWaitSkipResult() const {
	// NOTE: we remove timer flag to make timeout reason = 0
	return ((wait_skipped_by & ~SKIP_AUTOTIMER) << SKIP_RESULT_TYPE_SHIFT)
		| (wait_skipped_by_data & SKIP_RESULT_DATA_MASK);
}

bool GameState::IsBlockingVoiceSpeech() const {
	return speech_has_voice && speech_voice_blocking;
}

bool GameState::IsNonBlockingVoiceSpeech() const {
	return speech_has_voice && !speech_voice_blocking;
}

bool GameState::ShouldPlayVoiceSpeech() const {
	return !_GP(play).fast_forward &&
		(_GP(play).speech_mode != kSpeech_TextOnly) && (_GP(play).voice_avail);
}

void GameState::ReadFromSavegame(Stream *in, GameDataVersion data_ver, GameStateSvgVersion svg_ver, RestoredData &r_data) {
	const bool old_save = svg_ver < kGSSvgVersion_Initial;
	const bool extended_old_save = old_save && (data_ver >= kGameVersion_340_4);
	const bool do_align_pad = old_save;

	score = in->ReadInt32();
	usedmode = in->ReadInt32();
	disabled_user_interface = in->ReadInt32();
	gscript_timer = in->ReadInt32();
	debug_mode = in->ReadInt32();
	in->ReadArrayOfInt32(globalvars, MAXGLOBALVARS);
	messagetime = in->ReadInt32();
	usedinv = in->ReadInt32();
	inv_top = in->ReadInt32();
	inv_numdisp = in->ReadInt32();
	inv_numorder = in->ReadInt32();
	inv_numinline = in->ReadInt32();
	text_speed = in->ReadInt32();
	sierra_inv_color = in->ReadInt32();
	talkanim_speed = in->ReadInt32();
	inv_item_wid = in->ReadInt32();
	inv_item_hit = in->ReadInt32();
	speech_text_shadow = in->ReadInt32();
	swap_portrait_side = in->ReadInt32();
	speech_textwindow_gui = in->ReadInt32();
	follow_change_room_timer = in->ReadInt32();
	totalscore = in->ReadInt32();
	skip_display = in->ReadInt32();
	no_multiloop_repeat = in->ReadInt32();
	roomscript_finished = in->ReadInt32();
	used_inv_on = in->ReadInt32();
	no_textbg_when_voice = in->ReadInt32();
	max_dialogoption_width = in->ReadInt32();
	no_hicolor_fadein = in->ReadInt32();
	bgspeech_game_speed = in->ReadInt32();
	bgspeech_stay_on_display = in->ReadInt32();
	unfactor_speech_from_textlength = in->ReadInt32();
	mp3_loop_before_end = in->ReadInt32();
	speech_music_drop = in->ReadInt32();
	in_cutscene = in->ReadInt32();
	fast_forward = in->ReadInt32();
	room_width = in->ReadInt32();
	room_height = in->ReadInt32();
	game_speed_modifier = in->ReadInt32();
	score_sound = in->ReadInt32();
	takeover_data = in->ReadInt32();
	replay_hotkey_unused = in->ReadInt32();
	dialog_options_x = in->ReadInt32();
	dialog_options_y = in->ReadInt32();
	narrator_speech = in->ReadInt32();
	ambient_sounds_persist = in->ReadInt32();
	lipsync_speed = in->ReadInt32();
	close_mouth_speech_time = in->ReadInt32();
	disable_antialiasing = in->ReadInt32();
	text_speed_modifier = in->ReadInt32();
	if (svg_ver < kGSSvgVersion_350)
		text_align = ConvertLegacyScriptAlignment((LegacyScriptAlignment)in->ReadInt32());
	else
		text_align = (HorAlignment)in->ReadInt32();
	speech_bubble_width = in->ReadInt32();
	min_dialogoption_width = in->ReadInt32();
	disable_dialog_parser = in->ReadInt32();
	anim_background_speed = in->ReadInt32();  // the setting for this room
	top_bar_backcolor = in->ReadInt32();
	top_bar_textcolor = in->ReadInt32();
	top_bar_bordercolor = in->ReadInt32();
	top_bar_borderwidth = in->ReadInt32();
	top_bar_ypos = in->ReadInt32();
	screenshot_width = in->ReadInt32();
	screenshot_height = in->ReadInt32();
	top_bar_font = in->ReadInt32();
	if (svg_ver < kGSSvgVersion_350)
		speech_text_align = ConvertLegacyScriptAlignment((LegacyScriptAlignment)in->ReadInt32());
	else
		speech_text_align = (HorAlignment)in->ReadInt32();
	auto_use_walkto_points = in->ReadInt32();
	inventory_greys_out = in->ReadInt32();
	skip_speech_specific_key = in->ReadInt32();
	abort_key = in->ReadInt32();
	fade_to_red = in->ReadInt32();
	fade_to_green = in->ReadInt32();
	fade_to_blue = in->ReadInt32();
	show_single_dialog_option = in->ReadInt32();
	keep_screen_during_instant_transition = in->ReadInt32();
	read_dialog_option_colour = in->ReadInt32();
	stop_dialog_at_end = in->ReadInt32();
	speech_portrait_placement = in->ReadInt32();
	speech_portrait_x = in->ReadInt32();
	speech_portrait_y = in->ReadInt32();
	speech_display_post_time_ms = in->ReadInt32();
	dialog_options_highlight_color = in->ReadInt32();
	if (old_save)
		in->ReadArrayOfInt32(reserved, GAME_STATE_RESERVED_INTS);
	// ** up to here is referenced in the script "game." object
	if (old_save) {
		in->ReadInt32(); // recording
		in->ReadInt32(); // playback
		in->ReadInt16(); // gamestep
		in->ReadInt16(); // alignment padding to int32
	}
	randseed = in->ReadInt32();    // random seed
	player_on_region = in->ReadInt32();    // player's current region
	if (old_save)
		in->ReadInt32(); // screen_is_faded_out
	check_interaction_only = in->ReadInt32();
	bg_frame = in->ReadInt32();
	bg_anim_delay = in->ReadInt32();  // for animating backgrounds
	music_vol_was = in->ReadInt32();  // before the volume drop
	wait_counter = in->ReadInt16();
	mboundx1 = in->ReadInt16();
	mboundx2 = in->ReadInt16();
	mboundy1 = in->ReadInt16();
	mboundy2 = in->ReadInt16();
	if (do_align_pad)
		in->ReadInt16(); // alignment padding to int32
	fade_effect = in->ReadInt32();
	bg_frame_locked = in->ReadInt32();
	in->ReadArrayOfInt32(globalscriptvars, MAXGSVALUES);
	cur_music_number = in->ReadInt32();
	music_repeat = in->ReadInt32();
	music_master_volume = in->ReadInt32();
	digital_master_volume = in->ReadInt32();
	in->Read(walkable_areas_on, MAX_WALK_AREAS);
	screen_flipped = in->ReadInt16();
	if (svg_ver < kGSSvgVersion_350_10) {
		short offsets_locked = in->ReadInt16();
		if (offsets_locked != 0)
			r_data.Camera0_Flags = kSvgCamPosLocked;
	}
	entered_at_x = in->ReadInt32();
	entered_at_y = in->ReadInt32();
	entered_edge = in->ReadInt32();
	speech_mode = (SpeechMode)in->ReadInt32();
	speech_skip_style = in->ReadInt32();
	in->ReadArrayOfInt32(script_timers, MAX_TIMERS);
	sound_volume = in->ReadInt32();
	speech_volume = in->ReadInt32();
	normal_font = in->ReadInt32();
	speech_font = in->ReadInt32();
	key_skip_wait = in->ReadInt8();
	if (do_align_pad)
		in->Seek(3); // alignment padding to int32
	swap_portrait_lastchar = in->ReadInt32();
	separate_music_lib = in->ReadInt32() != 0;
	in_conversation = in->ReadInt32();
	screen_tint = in->ReadInt32();
	num_parsed_words = in->ReadInt32();
	in->ReadArrayOfInt16(parsed_words, MAX_PARSED_WORDS);
	in->Read(bad_parsed_word, 100);
	if (do_align_pad)
		in->ReadInt16(); // alignment padding to int32 (15 int16 + 100 int8 = 65 int16 -> 66)
	raw_color = in->ReadInt32();
	if (old_save)
		in->ReadArrayOfInt32(raw_modified, MAX_ROOM_BGFRAMES);
	in->ReadArrayOfInt16(filenumbers, MAXSAVEGAMES);
	if (old_save)
		in->ReadInt32(); // room_changes
	mouse_cursor_hidden = in->ReadInt32();
	silent_midi = in->ReadInt32();
	silent_midi_channel = in->ReadInt32();
	current_music_repeating = in->ReadInt32();
	shakesc_delay = in->ReadInt32();
	shakesc_amount = in->ReadInt32();
	shakesc_length = in->ReadInt32();
	rtint_red = in->ReadInt32();
	rtint_green = in->ReadInt32();
	rtint_blue = in->ReadInt32();
	rtint_level = in->ReadInt32();
	rtint_light = in->ReadInt32();
	if (!old_save || extended_old_save) {
		rtint_enabled = in->ReadBool();
		if (do_align_pad)
			in->Seek(3); // alignment padding to int32
	} else {
		rtint_enabled = rtint_level > 0;
	}
	end_cutscene_music = in->ReadInt32();
	skip_until_char_stops = in->ReadInt32();
	get_loc_name_last_time = in->ReadInt32();
	get_loc_name_save_cursor = in->ReadInt32();
	restore_cursor_mode_to = in->ReadInt32();
	restore_cursor_image_to = in->ReadInt32();
	music_queue_size = in->ReadInt16();
	in->ReadArrayOfInt16(music_queue, MAX_QUEUED_MUSIC);
	new_music_queue_size = in->ReadInt16();
	if (!old_save) {
		for (int i = 0; i < MAX_QUEUED_MUSIC; ++i) {
			new_music_queue[i].ReadFromSavegame(in);
		}
	}

	crossfading_out_channel = in->ReadInt16();
	crossfade_step = in->ReadInt16();
	crossfade_out_volume_per_step = in->ReadInt16();
	crossfade_initial_volume_out = in->ReadInt16();
	crossfading_in_channel = in->ReadInt16();
	crossfade_in_volume_per_step = in->ReadInt16();
	crossfade_final_volume_in = in->ReadInt16();

	if (old_save) {
		in->ReadInt16(); // alignment padding to int32 (before array of structs)
		for (int i = 0; i < MAX_QUEUED_MUSIC; ++i) {
			new_music_queue[i].ReadFromSavegame_v321(in);
		}
	}

	in->Read(takeover_from, 50);
	playmp3file_name.ReadCount(in, PLAYMP3FILE_MAX_FILENAME_LEN);
	in->Read(globalstrings, MAXGLOBALSTRINGS * MAX_MAXSTRLEN);
	in->Read(lastParserEntry, MAX_MAXSTRLEN);
	if (svg_ver < kGSSvgVersion_361_14)
		game_name.ReadCount(in, LEGACY_GAMESTATE_GAMENAMELENGTH);
	else
		game_name = StrUtil::ReadString(in);
	ground_level_areas_disabled = in->ReadInt32();
	next_screen_transition = in->ReadInt32();
	in->ReadInt32(); // gamma_adjustment -- do not apply gamma level from savegame
	temporarily_turned_off_character = in->ReadInt16();
	inv_backwards_compatibility = in->ReadInt16();
	if (old_save) {
		in->ReadInt32(); // gui_draw_order
		in->ReadInt32(); // do_once_tokens;
	}
	r_data.DoOnceCount = static_cast<uint32_t>(in->ReadInt32());
	if (!old_save) {
		for (size_t i = 0; i < r_data.DoOnceCount; ++i) {
			do_once_tokens.insert(StrUtil::ReadString(in));
		}
	}
	text_min_display_time_ms = in->ReadInt32();
	ignore_user_input_after_text_timeout_ms = in->ReadInt32();
	if (svg_ver < kGSSvgVersion_350_9)
		in->ReadInt32(); // ignore_user_input_until_time -- do not apply from savegame
	if (old_save)
		in->ReadArrayOfInt32(default_audio_type_volumes, MAX_AUDIO_TYPES);
	if (svg_ver >= kGSSvgVersion_350_9) {
		int voice_speech_flags = in->ReadInt32();
		speech_has_voice = voice_speech_flags != 0;
		speech_voice_blocking = (voice_speech_flags & 0x02) != 0;
	}
}

void GameState::WriteForSavegame(Stream *out) const {
	// NOTE: following parameters are never saved:
	// recording, playback, gamestep, screen_is_faded_out, room_changes
	out->WriteInt32(score);
	out->WriteInt32(usedmode);
	out->WriteInt32(disabled_user_interface);
	out->WriteInt32(gscript_timer);
	out->WriteInt32(debug_mode);
	out->WriteArrayOfInt32(globalvars, MAXGLOBALVARS);
	out->WriteInt32(messagetime);
	out->WriteInt32(usedinv);
	out->WriteInt32(inv_top);
	out->WriteInt32(inv_numdisp);
	out->WriteInt32(inv_numorder);
	out->WriteInt32(inv_numinline);
	out->WriteInt32(text_speed);
	out->WriteInt32(sierra_inv_color);
	out->WriteInt32(talkanim_speed);
	out->WriteInt32(inv_item_wid);
	out->WriteInt32(inv_item_hit);
	out->WriteInt32(speech_text_shadow);
	out->WriteInt32(swap_portrait_side);
	out->WriteInt32(speech_textwindow_gui);
	out->WriteInt32(follow_change_room_timer);
	out->WriteInt32(totalscore);
	out->WriteInt32(skip_display);
	out->WriteInt32(no_multiloop_repeat);
	out->WriteInt32(roomscript_finished);
	out->WriteInt32(used_inv_on);
	out->WriteInt32(no_textbg_when_voice);
	out->WriteInt32(max_dialogoption_width);
	out->WriteInt32(no_hicolor_fadein);
	out->WriteInt32(bgspeech_game_speed);
	out->WriteInt32(bgspeech_stay_on_display);
	out->WriteInt32(unfactor_speech_from_textlength);
	out->WriteInt32(mp3_loop_before_end);
	out->WriteInt32(speech_music_drop);
	out->WriteInt32(in_cutscene);
	out->WriteInt32(fast_forward);
	out->WriteInt32(room_width);
	out->WriteInt32(room_height);
	out->WriteInt32(game_speed_modifier);
	out->WriteInt32(score_sound);
	out->WriteInt32(takeover_data);
	out->WriteInt32(replay_hotkey_unused);         // StartRecording: not supported
	out->WriteInt32(dialog_options_x);
	out->WriteInt32(dialog_options_y);
	out->WriteInt32(narrator_speech);
	out->WriteInt32(ambient_sounds_persist);
	out->WriteInt32(lipsync_speed);
	out->WriteInt32(close_mouth_speech_time);
	out->WriteInt32(disable_antialiasing);
	out->WriteInt32(text_speed_modifier);
	out->WriteInt32(text_align);
	out->WriteInt32(speech_bubble_width);
	out->WriteInt32(min_dialogoption_width);
	out->WriteInt32(disable_dialog_parser);
	out->WriteInt32(anim_background_speed);  // the setting for this room
	out->WriteInt32(top_bar_backcolor);
	out->WriteInt32(top_bar_textcolor);
	out->WriteInt32(top_bar_bordercolor);
	out->WriteInt32(top_bar_borderwidth);
	out->WriteInt32(top_bar_ypos);
	out->WriteInt32(screenshot_width);
	out->WriteInt32(screenshot_height);
	out->WriteInt32(top_bar_font);
	out->WriteInt32(speech_text_align);
	out->WriteInt32(auto_use_walkto_points);
	out->WriteInt32(inventory_greys_out);
	out->WriteInt32(skip_speech_specific_key);
	out->WriteInt32(abort_key);
	out->WriteInt32(fade_to_red);
	out->WriteInt32(fade_to_green);
	out->WriteInt32(fade_to_blue);
	out->WriteInt32(show_single_dialog_option);
	out->WriteInt32(keep_screen_during_instant_transition);
	out->WriteInt32(read_dialog_option_colour);
	out->WriteInt32(stop_dialog_at_end);
	out->WriteInt32(speech_portrait_placement);
	out->WriteInt32(speech_portrait_x);
	out->WriteInt32(speech_portrait_y);
	out->WriteInt32(speech_display_post_time_ms);
	out->WriteInt32(dialog_options_highlight_color);
	// ** up to here is referenced in the script "game." object
	out->WriteInt32(randseed);    // random seed
	out->WriteInt32(player_on_region);    // player's current region
	out->WriteInt32(check_interaction_only);
	out->WriteInt32(bg_frame);
	out->WriteInt32(bg_anim_delay);  // for animating backgrounds
	out->WriteInt32(music_vol_was);  // before the volume drop
	out->WriteInt16(wait_counter);
	out->WriteInt16(mboundx1);
	out->WriteInt16(mboundx2);
	out->WriteInt16(mboundy1);
	out->WriteInt16(mboundy2);
	out->WriteInt32(fade_effect);
	out->WriteInt32(bg_frame_locked);
	out->WriteArrayOfInt32(globalscriptvars, MAXGSVALUES);
	out->WriteInt32(cur_music_number);
	out->WriteInt32(music_repeat);
	out->WriteInt32(music_master_volume);
	out->WriteInt32(digital_master_volume);
	out->Write(walkable_areas_on, MAX_WALK_AREAS);
	out->WriteInt16(screen_flipped);
	out->WriteInt32(entered_at_x);
	out->WriteInt32(entered_at_y);
	out->WriteInt32(entered_edge);
	out->WriteInt32(speech_mode);
	out->WriteInt32(speech_skip_style);
	out->WriteArrayOfInt32(script_timers, MAX_TIMERS);
	out->WriteInt32(sound_volume);
	out->WriteInt32(speech_volume);
	out->WriteInt32(normal_font);
	out->WriteInt32(speech_font);
	out->WriteInt8(key_skip_wait);
	out->WriteInt32(swap_portrait_lastchar);
	out->WriteInt32(separate_music_lib ? 1 : 0);
	out->WriteInt32(in_conversation);
	out->WriteInt32(screen_tint);
	out->WriteInt32(num_parsed_words);
	out->WriteArrayOfInt16(parsed_words, MAX_PARSED_WORDS);
	out->Write(bad_parsed_word, 100);
	out->WriteInt32(raw_color);
	out->WriteArrayOfInt16(filenumbers, MAXSAVEGAMES);
	out->WriteInt32(mouse_cursor_hidden);
	out->WriteInt32(silent_midi);
	out->WriteInt32(silent_midi_channel);
	out->WriteInt32(current_music_repeating);
	out->WriteInt32(shakesc_delay);
	out->WriteInt32(shakesc_amount);
	out->WriteInt32(shakesc_length);
	out->WriteInt32(rtint_red);
	out->WriteInt32(rtint_green);
	out->WriteInt32(rtint_blue);
	out->WriteInt32(rtint_level);
	out->WriteInt32(rtint_light);
	out->WriteBool(rtint_enabled);
	out->WriteInt32(end_cutscene_music);
	out->WriteInt32(skip_until_char_stops);
	out->WriteInt32(get_loc_name_last_time);
	out->WriteInt32(get_loc_name_save_cursor);
	out->WriteInt32(restore_cursor_mode_to);
	out->WriteInt32(restore_cursor_image_to);
	out->WriteInt16(music_queue_size);
	out->WriteArrayOfInt16(music_queue, MAX_QUEUED_MUSIC);
	out->WriteInt16(new_music_queue_size);
	for (int i = 0; i < MAX_QUEUED_MUSIC; ++i) {
		new_music_queue[i].WriteToSavegame(out);
	}

	out->WriteInt16(crossfading_out_channel);
	out->WriteInt16(crossfade_step);
	out->WriteInt16(crossfade_out_volume_per_step);
	out->WriteInt16(crossfade_initial_volume_out);
	out->WriteInt16(crossfading_in_channel);
	out->WriteInt16(crossfade_in_volume_per_step);
	out->WriteInt16(crossfade_final_volume_in);

	out->Write(takeover_from, 50);
	playmp3file_name.WriteCount(out, PLAYMP3FILE_MAX_FILENAME_LEN);
	out->Write(globalstrings, MAXGLOBALSTRINGS * MAX_MAXSTRLEN);
	out->Write(lastParserEntry, MAX_MAXSTRLEN);
	StrUtil::WriteString(game_name, out);
	out->WriteInt32(ground_level_areas_disabled);
	out->WriteInt32(next_screen_transition);
	out->WriteInt32(gamma_adjustment);
	out->WriteInt16(temporarily_turned_off_character);
	out->WriteInt16(inv_backwards_compatibility);
	out->WriteInt32(static_cast<uint32_t>(do_once_tokens.size()));
	for (const auto &token : do_once_tokens) {
		StrUtil::WriteString(token, out);
	}
	out->WriteInt32(text_min_display_time_ms);
	out->WriteInt32(ignore_user_input_after_text_timeout_ms);

	int voice_speech_flags = speech_has_voice ? 0x01 : 0;
	if (speech_voice_blocking)
		voice_speech_flags |= 0x02;
	out->WriteInt32(voice_speech_flags);
}

void GameState::FreeProperties() {
	for (auto &p : charProps)
		p.clear();
	for (auto &p : invProps)
		p.clear();
}

void GameState::FreeViewportsAndCameras() {
	_roomViewports.clear();
	_roomViewportsSorted.clear();
	for (auto handle : _scViewportHandles) {
		auto scview = const_cast<ScriptViewport*>((const ScriptViewport*)ccGetObjectAddressFromHandle(handle));
		if (scview) {
			scview->Invalidate();
			ccReleaseObjectReference(handle);
		}
	}
	_scViewportHandles.clear();
	_roomCameras.clear();
	for (auto handle : _scCameraHandles) {
		auto sccam = const_cast<ScriptCamera*>((const ScriptCamera*)ccGetObjectAddressFromHandle(handle));
		if (sccam) {
			sccam->Invalidate();
			ccReleaseObjectReference(handle);
		}
	}
	_scCameraHandles.clear();
}

void GameState::ReadCustomProperties_v340(Stream *in, GameDataVersion data_ver) {
	if (data_ver >= kGameVersion_340_4) {
		// After runtime property values were read we also copy missing default,
		// because we do not keep defaults in the saved game, and also in case
		// this save is made by an older game version which had different
		// properties.
		for (int i = 0; i < _GP(game).numcharacters; ++i)
			Properties::ReadValues(charProps[i], in);
		for (int i = 0; i < _GP(game).numinvitems; ++i)
			Properties::ReadValues(invProps[i], in);
	}
}

void GameState::WriteCustomProperties_v340(Stream *out, GameDataVersion data_ver) const {
	if (data_ver >= kGameVersion_340_4) {
		// We temporarily remove properties that kept default values
		// just for the saving data time to avoid getting lots of
		// redundant data into saved games
		for (int i = 0; i < _GP(game).numcharacters; ++i)
			Properties::WriteValues(charProps[i], out);
		for (int i = 0; i < _GP(game).numinvitems; ++i)
			Properties::WriteValues(invProps[i], out);
	}
}

// Converts legacy alignment type used in script API
HorAlignment ConvertLegacyScriptAlignment(LegacyScriptAlignment align) {
	switch (align) {
	case kLegacyScAlignLeft:
		return kHAlignLeft;
	case kLegacyScAlignCentre:
		return kHAlignCenter;
	case kLegacyScAlignRight:
		return kHAlignRight;
	}
	return kHAlignNone;
}

// Reads legacy alignment type from the value set in script depending on the
// current Script API level. This is made to make it possible to change
// Alignment constants in the Script API and still support old version.
HorAlignment ReadScriptAlignment(int32_t align) {
	return _GP(game).options[OPT_BASESCRIPTAPI] < kScriptAPI_v350 ?
	       ConvertLegacyScriptAlignment((LegacyScriptAlignment)align) :
	       (HorAlignment)align;
}

} // namespace AGS3
