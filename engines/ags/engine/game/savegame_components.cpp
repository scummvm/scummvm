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

#include "common/std/map.h"
#include "ags/engine/game/savegame_components.h"
#include "ags/shared/ac/audio_clip_type.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/dialog.h"
#include "ags/engine/ac/button.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/dynobj/cc_serializer.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/game/savegame_internal.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gui/animating_gui_button.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_inv.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/shared/gui/gui_listbox.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_slider.h"
#include "ags/shared/gui/gui_textbox.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/shared/script/cc_common.h"
#include "ags/engine/script/script.h"
#include "ags/shared/util/file_stream.h" // TODO: needed only because plugins expect file handle
#include "ags/engine/media/audio/audio_system.h"

namespace AGS3 {

using namespace Shared;

namespace AGS {
namespace Engine {

namespace SavegameComponents {

void WriteFormatTag(Stream *out, const String &tag, bool open = true) {
	String full_tag = String::FromFormat(open ? "<%s>" : "</%s>", tag.GetCStr());
	out->Write(full_tag.GetCStr(), full_tag.GetLength());
}

bool ReadFormatTag(Stream *in, String &tag, bool open = true) {
	if (in->ReadByte() != '<')
		return false;
	if (!open && in->ReadByte() != '/')
		return false;
	tag.Empty();
	while (!in->EOS()) {
		char c = in->ReadByte();
		if (c == '>')
			return true;
		tag.AppendChar(c);
	}
	return false; // reached EOS before closing symbol
}

bool AssertFormatTag(Stream *in, const String &tag, bool open = true) {
	String read_tag;
	if (!ReadFormatTag(in, read_tag, open))
		return false;
	return read_tag.Compare(tag) == 0;
}

bool AssertFormatTagStrict(HSaveError &err, Stream *in, const String &tag, bool open = true) {

	String read_tag;
	if (!ReadFormatTag(in, read_tag, open) || read_tag.Compare(tag) != 0) {
		err = new SavegameError(kSvgErr_InconsistentFormat,
		                        String::FromFormat("Mismatching tag: %s.", tag.GetCStr()));
		return false;
	}
	return true;
}

inline bool AssertCompatLimit(HSaveError &err, int count, int max_count, const char *content_name) {
	if (count > max_count) {
		err = new SavegameError(kSvgErr_IncompatibleEngine,
		                        String::FromFormat("Incompatible number of %s (count: %d, max: %d).",
		                                content_name, count, max_count));
		return false;
	}
	return true;
}

inline bool AssertCompatRange(HSaveError &err, int value, int min_value, int max_value, const char *content_name) {
	if (value < min_value || value > max_value) {
		err = new SavegameError(kSvgErr_IncompatibleEngine,
		                        String::FromFormat("Restore game error: incompatible %s (id: %d, range: %d - %d).",
		                                content_name, value, min_value, max_value));
		return false;
	}
	return true;
}

inline bool AssertGameContent(HSaveError &err, int new_val, int original_val, const char *content_name) {
	if (new_val != original_val) {
		err = new SavegameError(kSvgErr_GameContentAssertion,
		                        String::FromFormat("Mismatching number of %s (game: %d, save: %d).",
		                                content_name, original_val, new_val));
		return false;
	}
	return true;
}

inline bool AssertGameObjectContent(HSaveError &err, int new_val, int original_val, const char *content_name,
                                    const char *obj_type, int obj_id) {
	if (new_val != original_val) {
		err = new SavegameError(kSvgErr_GameContentAssertion,
		                        String::FromFormat("Mismatching number of %s, %s #%d (game: %d, save: %d).",
		                                content_name, obj_type, obj_id, original_val, new_val));
		return false;
	}
	return true;
}

inline bool AssertGameObjectContent2(HSaveError &err, int new_val, int original_val, const char *content_name,
                                     const char *obj1_type, int obj1_id, const char *obj2_type, int obj2_id) {
	if (new_val != original_val) {
		err = new SavegameError(kSvgErr_GameContentAssertion,
		                        String::FromFormat("Mismatching number of %s, %s #%d, %s #%d (game: %d, save: %d).",
		                                content_name, obj1_type, obj1_id, obj2_type, obj2_id, original_val, new_val));
		return false;
	}
	return true;
}


void WriteCameraState(const Camera &cam, Stream *out) {
	int flags = 0;
	if (cam.IsLocked()) flags |= kSvgCamPosLocked;
	out->WriteInt32(flags);
	const Rect &rc = cam.GetRect();
	out->WriteInt32(rc.Left);
	out->WriteInt32(rc.Top);
	out->WriteInt32(rc.GetWidth());
	out->WriteInt32(rc.GetHeight());
}

void WriteViewportState(const Viewport &view, Stream *out) {
	int flags = 0;
	if (view.IsVisible()) flags |= kSvgViewportVisible;
	out->WriteInt32(flags);
	const Rect &rc = view.GetRect();
	out->WriteInt32(rc.Left);
	out->WriteInt32(rc.Top);
	out->WriteInt32(rc.GetWidth());
	out->WriteInt32(rc.GetHeight());
	out->WriteInt32(view.GetZOrder());
	auto cam = view.GetCamera();
	if (cam)
		out->WriteInt32(cam->GetID());
	else
		out->WriteInt32(-1);
}

HSaveError WriteGameState(Stream *out) {
	// Game base
	_GP(game).WriteForSavegame(out);
	// Game palette
	// TODO: probably no need to save this for hi/true-res game
	out->WriteArray(_G(palette), sizeof(RGB), 256);

	if (_G(loaded_game_file_version) <= kGameVersion_272) {
		// Global variables
		out->WriteInt32(_G(numGlobalVars));
		for (int i = 0; i < _G(numGlobalVars); ++i)
			_G(globalvars)[i].Write(out);
	}

	// Game state
	_GP(play).WriteForSavegame(out);
	// Other dynamic values
	out->WriteInt32(_G(frames_per_second));
	out->WriteInt32(_G(loopcounter));
	out->WriteInt32(_G(ifacepopped));
	out->WriteInt32(_G(game_paused));
	// Mouse cursor
	out->WriteInt32(_G(cur_mode));
	out->WriteInt32(_G(cur_cursor));
	out->WriteInt32(_G(mouse_on_iface));

	// Viewports and cameras
	int viewcam_flags = 0;
	if (_GP(play).IsAutoRoomViewport())
		viewcam_flags |= kSvgGameAutoRoomView;
	out->WriteInt32(viewcam_flags);
	out->WriteInt32(_GP(play).GetRoomCameraCount());
	for (int i = 0; i < _GP(play).GetRoomCameraCount(); ++i)
		WriteCameraState(*_GP(play).GetRoomCamera(i), out);
	out->WriteInt32(_GP(play).GetRoomViewportCount());
	for (int i = 0; i < _GP(play).GetRoomViewportCount(); ++i)
		WriteViewportState(*_GP(play).GetRoomViewport(i), out);

	return HSaveError::None();
}

void ReadLegacyCameraState(Stream *in, RestoredData & r_data) {
	// Precreate viewport and camera and save data in temp structs
	int camx = in->ReadInt32();
	int camy = in->ReadInt32();
	_GP(play).CreateRoomCamera();
	_GP(play).CreateRoomViewport();
	RestoredData::CameraData cam_dat;
	cam_dat.ID = 0;
	cam_dat.Left = camx;
	cam_dat.Top = camy;
	r_data.Cameras.push_back(cam_dat);
	RestoredData::ViewportData view_dat;
	view_dat.ID = 0;
	view_dat.Flags = kSvgViewportVisible;
	view_dat.CamID = 0;
	r_data.Viewports.push_back(view_dat);
	r_data.LegacyViewCamera = true;
}

void ReadCameraState(RestoredData &r_data, Stream *in) {
	RestoredData::CameraData cam;
	cam.ID = r_data.Cameras.size();
	cam.Flags = in->ReadInt32();
	cam.Left = in->ReadInt32();
	cam.Top = in->ReadInt32();
	cam.Width = in->ReadInt32();
	cam.Height = in->ReadInt32();
	r_data.Cameras.push_back(cam);
}

void ReadViewportState(RestoredData &r_data, Stream *in) {
	RestoredData::ViewportData view;
	view.ID = r_data.Viewports.size();
	view.Flags = in->ReadInt32();
	view.Left = in->ReadInt32();
	view.Top = in->ReadInt32();
	view.Width = in->ReadInt32();
	view.Height = in->ReadInt32();
	view.ZOrder = in->ReadInt32();
	view.CamID = in->ReadInt32();
	r_data.Viewports.push_back(view);
}

HSaveError ReadGameState(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData &r_data) {
	HSaveError err;
	GameStateSvgVersion svg_ver = (GameStateSvgVersion)cmp_ver;
	// Game base
	_GP(game).ReadFromSavegame(in);
	// Game palette
	in->ReadArray(_G(palette), sizeof(RGB), 256);

	if (_G(loaded_game_file_version) <= kGameVersion_272) {
		// Legacy interaction global variables
		if (!AssertGameContent(err, in->ReadInt32(), _G(numGlobalVars), "Global Variables"))
			return err;
		for (int i = 0; i < _G(numGlobalVars); ++i)
			_G(globalvars)[i].Read(in);
	}

	// Game state
	_GP(play).ReadFromSavegame(in, _G(loaded_game_file_version), svg_ver, r_data);

	// Other dynamic values
	r_data.FPS = in->ReadInt32();
	set_loop_counter(in->ReadInt32());
	_G(ifacepopped) = in->ReadInt32();
	_G(game_paused) = in->ReadInt32();
	// Mouse cursor state
	r_data.CursorMode = in->ReadInt32();
	r_data.CursorID = in->ReadInt32();
	_G(mouse_on_iface) = in->ReadInt32();

	// Viewports and cameras
	if (svg_ver < kGSSvgVersion_350_10) {
		ReadLegacyCameraState(in, r_data);
		r_data.Cameras[0].Flags = r_data.Camera0_Flags;
	} else {
		int viewcam_flags = in->ReadInt32();
		_GP(play).SetAutoRoomViewport((viewcam_flags & kSvgGameAutoRoomView) != 0);
		// TODO: we create viewport and camera objects here because they are
		// required for the managed pool deserialization, but read actual
		// data into temp structs because we need to apply it after active
		// room is loaded.
		// See comments to RestoredData struct for further details.
		int cam_count = in->ReadInt32();
		for (int i = 0; i < cam_count; ++i) {
			_GP(play).CreateRoomCamera();
			ReadCameraState(r_data, in);
		}
		int view_count = in->ReadInt32();
		for (int i = 0; i < view_count; ++i) {
			_GP(play).CreateRoomViewport();
			ReadViewportState(r_data, in);
		}
	}
	return err;
}

// Savegame data format for RoomStatus
enum AudioSvgVersion {
	kAudioSvgVersion_Initial = 0,
	kAudioSvgVersion_35026 = 1,       // source position settings
	kAudioSvgVersion_36009 = 2,       // up number of channels
	kAudioSvgVersion_36130 = 3060130, // playback state
};

HSaveError WriteAudio(Stream *out) {
	// Game content assertion
	out->WriteInt32(_GP(game).audioClipTypes.size());
	out->WriteInt8(TOTAL_AUDIO_CHANNELS);
	out->WriteInt8(_GP(game).numGameChannels);
	out->WriteInt16(0); // reserved 2 bytes (remains of int32)

	// Audio types
	for (size_t i = 0; i < _GP(game).audioClipTypes.size(); ++i) {
		_GP(game).audioClipTypes[i].WriteToSavegame(out);
		out->WriteInt32(_GP(play).default_audio_type_volumes[i]);
	}

	// Audio clips and crossfade
	for (int i = 0; i < TOTAL_AUDIO_CHANNELS; i++) {
		auto *ch = AudioChans::GetChannelIfPlaying(i);
		if ((ch != nullptr) && (ch->_sourceClipID >= 0)) {
			out->WriteInt32(ch->_sourceClipID);
			out->WriteInt32(ch->get_pos());
			out->WriteInt32(ch->_priority);
			out->WriteInt32(ch->_repeat ? 1 : 0);
			out->WriteInt32(ch->get_volume255());
			out->WriteInt32(0); // was redundant data
			out->WriteInt32(ch->get_volume100());
			out->WriteInt32(ch->get_panning());
			out->WriteInt32(ch->get_speed());
			// since version kAudioSvgVersion_35026
			out->WriteInt32(ch->_xSource);
			out->WriteInt32(ch->_ySource);
			out->WriteInt32(ch->_maximumPossibleDistanceAway);
			// since version kAudioSvgVersion_36130
			int playback_flags = 0;
			if (ch->is_paused())
				playback_flags |= kSvgAudioPaused;
			out->WriteInt32(playback_flags);
			out->WriteInt32(0); // reserved 3 ints
			out->WriteInt32(0);
			out->WriteInt32(0);
		} else {
			out->WriteInt32(-1);
		}
	}
	out->WriteInt32(_G(crossFading));
	out->WriteInt32(_G(crossFadeVolumePerStep));
	out->WriteInt32(_G(crossFadeStep));
	out->WriteInt32(_G(crossFadeVolumeAtStart));
	// CHECKME: why this needs to be saved?
	out->WriteInt32(_G(current_music_type));

	// Ambient sound
	for (int i = 0; i < _GP(game).numGameChannels; ++i)
		_GP(ambient)[i].WriteToFile(out);
	return HSaveError::None();
}

HSaveError ReadAudio(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData &r_data) {
	HSaveError err;
	// Game content assertion
	if (!AssertGameContent(err, in->ReadInt32(), _GP(game).audioClipTypes.size(), "Audio Clip Types"))
		return err;
	int total_channels, max_game_channels;
	if (cmp_ver >= kAudioSvgVersion_36009) {
		total_channels = in->ReadInt8();
		max_game_channels = in->ReadInt8();
		in->ReadInt16(); // reserved 2 bytes
		if (!AssertCompatLimit(err, total_channels, TOTAL_AUDIO_CHANNELS, "System Audio Channels") ||
			!AssertCompatLimit(err, max_game_channels, MAX_GAME_CHANNELS, "Game Audio Channels"))
			return err;
	} else {
		total_channels = TOTAL_AUDIO_CHANNELS_v320;
		max_game_channels = MAX_GAME_CHANNELS_v320;
		in->ReadInt32(); // unused in prev format ver
	}

	// Audio types
	for (size_t i = 0; i < _GP(game).audioClipTypes.size(); ++i) {
		_GP(game).audioClipTypes[i].ReadFromSavegame(in);
		_GP(play).default_audio_type_volumes[i] = in->ReadInt32();
	}

	// Audio clips and crossfade
	for (int i = 0; i < total_channels; ++i) {
		RestoredData::ChannelInfo &chan_info = r_data.AudioChans[i];
		chan_info.Pos = 0;
		chan_info.ClipID = in->ReadInt32();
		if (chan_info.ClipID >= 0) {
			chan_info.Pos = in->ReadInt32();
			if (chan_info.Pos < 0)
				chan_info.Pos = 0;
			chan_info.Priority = in->ReadInt32();
			chan_info.Repeat = in->ReadInt32();
			chan_info.Vol = in->ReadInt32();
			in->ReadInt32(); // was redundant data
			chan_info.VolAsPercent = in->ReadInt32();
			chan_info.Pan = in->ReadInt32();
			chan_info.Speed = 1000;
			chan_info.Speed = in->ReadInt32();
			if (cmp_ver >= kAudioSvgVersion_35026) {
				chan_info.XSource = in->ReadInt32();
				chan_info.YSource = in->ReadInt32();
				chan_info.MaxDist = in->ReadInt32();
			}
			if (cmp_ver >= kAudioSvgVersion_36130) {
				chan_info.Flags = in->ReadInt32();
				in->ReadInt32(); // reserved 3 ints
				in->ReadInt32();
				in->ReadInt32();
			}
		}
	}
	_G(crossFading) = in->ReadInt32();
	_G(crossFadeVolumePerStep) = in->ReadInt32();
	_G(crossFadeStep) = in->ReadInt32();
	_G(crossFadeVolumeAtStart) = in->ReadInt32();
	// preserve legacy music type setting
	_G(current_music_type) = in->ReadInt32();

	// Ambient sound
	for (int i = 0; i < max_game_channels; ++i)
		_GP(ambient)[i].ReadFromFile(in);
	for (int i = NUM_SPEECH_CHANS; i < max_game_channels; ++i) {
		if (_GP(ambient)[i].channel == 0) {
			r_data.DoAmbient[i] = 0;
		} else {
			r_data.DoAmbient[i] = _GP(ambient)[i].num;
			_GP(ambient)[i].channel = 0;
		}
	}
	return err;
}

void WriteTimesRun272(const Interaction &intr, Stream *out) {
	for (size_t i = 0; i < intr.Events.size(); ++i)
		out->WriteInt32(intr.Events[i].TimesRun);
}

void WriteInteraction272(const Interaction &intr, Stream *out) {
	const size_t evt_count = intr.Events.size();
	out->WriteInt32(evt_count);
	for (size_t i = 0; i < evt_count; ++i)
		out->WriteInt32(intr.Events[i].Type);
	WriteTimesRun272(intr, out);
}

void ReadTimesRun272(Interaction &intr, Stream *in) {
	for (size_t i = 0; i < intr.Events.size(); ++i)
		intr.Events[i].TimesRun = in->ReadInt32();
}

HSaveError ReadInteraction272(Interaction &intr, Stream *in) {
	HSaveError err;
	const size_t evt_count = in->ReadInt32();
	if (!AssertCompatLimit(err, evt_count, MAX_NEWINTERACTION_EVENTS, "interactions"))
		return err;
	intr.Events.resize(evt_count);
	for (size_t i = 0; i < evt_count; ++i)
		intr.Events[i].Type = in->ReadInt32();
	ReadTimesRun272(intr, in);
	return err;
}

HSaveError WriteCharacters(Stream *out) {
	out->WriteInt32(_GP(game).numcharacters);
	for (int i = 0; i < _GP(game).numcharacters; ++i) {
		_GP(game).chars[i].WriteToSavegame(out, _GP(game).chars2[i]);
		_GP(charextra)[i].WriteToSavegame(out);
		Properties::WriteValues(_GP(play).charProps[i], out);
		if (_G(loaded_game_file_version) <= kGameVersion_272)
			WriteTimesRun272(*_GP(game).intrChar[i], out);
	}
	return HSaveError::None();
}

HSaveError ReadCharacters(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	HSaveError err;
	if (!AssertGameContent(err, in->ReadInt32(), _GP(game).numcharacters, "Characters"))
		return err;
	for (int i = 0; i < _GP(game).numcharacters; ++i) {
		_GP(game).chars[i].ReadFromSavegame(in, _GP(game).chars2[i], static_cast<CharacterSvgVersion>(cmp_ver));
		_GP(charextra)[i].ReadFromSavegame(in, static_cast<CharacterSvgVersion>(cmp_ver));
		Properties::ReadValues(_GP(play).charProps[i], in);
		if (_G(loaded_game_file_version) <= kGameVersion_272)
			ReadTimesRun272(*_GP(game).intrChar[i], in);
		// character movement path (for old saves)
		if (cmp_ver < kCharSvgVersion_36109) {
			err = _GP(mls)[CHMLSOFFS + i].ReadFromSavegame(in, kMoveSvgVersion_350);
			if (!err)
				return err;
		}
	}
	return err;
}

HSaveError WriteDialogs(Stream *out) {
	out->WriteInt32(_GP(game).numdialog);
	for (int i = 0; i < _GP(game).numdialog; ++i) {
		_G(dialog)[i].WriteToSavegame(out);
	}
	return HSaveError::None();
}

HSaveError ReadDialogs(Stream *in, int32_t /*cmp_ver*/, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	HSaveError err;
	if (!AssertGameContent(err, in->ReadInt32(), _GP(game).numdialog, "Dialogs"))
		return err;
	for (int i = 0; i < _GP(game).numdialog; ++i) {
		_G(dialog)[i].ReadFromSavegame(in);
	}
	return err;
}

HSaveError WriteGUI(Stream *out) {
	// GUI state
	WriteFormatTag(out, "GUIs");
	out->WriteInt32(_GP(game).numgui);
	for (const auto &gui : _GP(guis))
		gui.WriteToSavegame(out);

	WriteFormatTag(out, "GUIButtons");
	out->WriteInt32(static_cast<int32_t>(_GP(guibuts).size()));
	for (const auto &but : _GP(guibuts))
		but.WriteToSavegame(out);

	WriteFormatTag(out, "GUILabels");
	out->WriteInt32(static_cast<int32_t>(_GP(guilabels).size()));
	for (const auto &label : _GP(guilabels))
		label.WriteToSavegame(out);

	WriteFormatTag(out, "GUIInvWindows");
	out->WriteInt32(static_cast<int32_t>(_GP(guiinv).size()));
	for (const auto &inv : _GP(guiinv))
		inv.WriteToSavegame(out);

	WriteFormatTag(out, "GUISliders");
	out->WriteInt32(static_cast<int32_t>(_GP(guislider).size()));
	for (const auto &slider : _GP(guislider))
		slider.WriteToSavegame(out);

	WriteFormatTag(out, "GUITextBoxes");
	out->WriteInt32(static_cast<int32_t>(_GP(guitext).size()));
	for (const auto &tb : _GP(guitext))
		tb.WriteToSavegame(out);

	WriteFormatTag(out, "GUIListBoxes");
	out->WriteInt32(static_cast<int32_t>(_GP(guilist).size()));
	for (const auto &list : _GP(guilist))
		list.WriteToSavegame(out);

	// Animated buttons
	WriteFormatTag(out, "AnimatedButtons");
	size_t num_abuts = GetAnimatingButtonCount();
	out->WriteInt32(num_abuts);
	for (size_t i = 0; i < num_abuts; ++i)
		GetAnimatingButtonByIndex(i)->WriteToSavegame(out);
	return HSaveError::None();
}

HSaveError ReadGUI(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	HSaveError err;
	const GuiSvgVersion svg_ver = (GuiSvgVersion)cmp_ver;
	// GUI state
	if (!AssertFormatTagStrict(err, in, "GUIs"))
		return err;
	if (!AssertGameContent(err, static_cast<size_t>(in->ReadInt32()), _GP(game).numgui, "GUIs"))
		return err;
	for (int i = 0; i < _GP(game).numgui; ++i)
		_GP(guis)[i].ReadFromSavegame(in, svg_ver);

	if (!AssertFormatTagStrict(err, in, "GUIButtons"))
		return err;
	if (!AssertGameContent(err, static_cast<size_t>(in->ReadInt32()), _GP(guibuts).size(), "GUI Buttons"))
		return err;
	for (auto &but : _GP(guibuts))
		but.ReadFromSavegame(in, svg_ver);

	if (!AssertFormatTagStrict(err, in, "GUILabels"))
		return err;
	if (!AssertGameContent(err, static_cast<size_t>(in->ReadInt32()), _GP(guilabels).size(), "GUI Labels"))
		return err;
	for (auto &label : _GP(guilabels))
		label.ReadFromSavegame(in, svg_ver);

	if (!AssertFormatTagStrict(err, in, "GUIInvWindows"))
		return err;
	if (!AssertGameContent(err, static_cast<size_t>(in->ReadInt32()), _GP(guiinv).size(), "GUI InvWindows"))
		return err;
	for (auto &inv : _GP(guiinv))
		inv.ReadFromSavegame(in, svg_ver);

	if (!AssertFormatTagStrict(err, in, "GUISliders"))
		return err;
	if (!AssertGameContent(err, static_cast<size_t>(in->ReadInt32()), _GP(guislider).size(), "GUI Sliders"))
		return err;
	for (auto &slider : _GP(guislider))
		slider.ReadFromSavegame(in, svg_ver);

	if (!AssertFormatTagStrict(err, in, "GUITextBoxes"))
		return err;
	if (!AssertGameContent(err, static_cast<size_t>(in->ReadInt32()), _GP(guitext).size(), "GUI TextBoxes"))
		return err;
	for (auto &tb : _GP(guitext))
		tb.ReadFromSavegame(in, svg_ver);

	if (!AssertFormatTagStrict(err, in, "GUIListBoxes"))
		return err;
	if (!AssertGameContent(err, static_cast<size_t>(in->ReadInt32()), _GP(guilist).size(), "GUI ListBoxes"))
		return err;
	for (auto &list : _GP(guilist))
		list.ReadFromSavegame(in, svg_ver);

	// Animated buttons
	if (!AssertFormatTagStrict(err, in, "AnimatedButtons"))
		return err;
	int anim_count = in->ReadInt32();
	for (int i = 0; i < anim_count; ++i) {
		AnimatingGUIButton abut;
		abut.ReadFromSavegame(in, cmp_ver);
		AddButtonAnimation(abut);
	}
	return err;
}

HSaveError WriteInventory(Stream *out) {
	out->WriteInt32(_GP(game).numinvitems);
	for (int i = 0; i < _GP(game).numinvitems; ++i) {
		_GP(game).invinfo[i].WriteToSavegame(out);
		Properties::WriteValues(_GP(play).invProps[i], out);
		if (_G(loaded_game_file_version) <= kGameVersion_272)
			WriteTimesRun272(*_GP(game).intrInv[i], out);
	}
	return HSaveError::None();
}

HSaveError ReadInventory(Stream *in, int32_t /*cmp_ver*/, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	HSaveError err;
	if (!AssertGameContent(err, in->ReadInt32(), _GP(game).numinvitems, "Inventory Items"))
		return err;
	for (int i = 0; i < _GP(game).numinvitems; ++i) {
		_GP(game).invinfo[i].ReadFromSavegame(in);
		Properties::ReadValues(_GP(play).invProps[i], in);
		if (_G(loaded_game_file_version) <= kGameVersion_272)
			ReadTimesRun272(*_GP(game).intrInv[i], in);
	}
	return err;
}

HSaveError WriteMouseCursors(Stream *out) {
	out->WriteInt32(_GP(game).numcursors);
	for (int i = 0; i < _GP(game).numcursors; ++i) {
		_GP(game).mcurs[i].WriteToSavegame(out);
	}
	return HSaveError::None();
}

HSaveError ReadMouseCursors(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	HSaveError err;
	if (!AssertGameContent(err, in->ReadInt32(), _GP(game).numcursors, "Mouse Cursors"))
		return err;
	for (int i = 0; i < _GP(game).numcursors; ++i) {
		_GP(game).mcurs[i].ReadFromSavegame(in, cmp_ver);
	}
	return err;
}

HSaveError WriteViews(Stream *out) {
	out->WriteInt32(_GP(game).numviews);
	for (int view = 0; view < _GP(game).numviews; ++view) {
		out->WriteInt32(_GP(views)[view].numLoops);
		for (int loop = 0; loop < _GP(views)[view].numLoops; ++loop) {
			out->WriteInt32(_GP(views)[view].loops[loop].numFrames);
			for (int frame = 0; frame < _GP(views)[view].loops[loop].numFrames; ++frame) {
				out->WriteInt32(_GP(views)[view].loops[loop].frames[frame].sound);
				out->WriteInt32(_GP(views)[view].loops[loop].frames[frame].pic);
			}
		}
	}
	return HSaveError::None();
}

HSaveError ReadViews(Stream *in, int32_t /*cmp_ver*/, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	HSaveError err;
	if (!AssertGameContent(err, in->ReadInt32(), _GP(game).numviews, "Views"))
		return err;
	for (int view = 0; view < _GP(game).numviews; ++view) {
		if (!AssertGameObjectContent(err, in->ReadInt32(), _GP(views)[view].numLoops,
		                             "Loops", "View", view))
			return err;
		for (int loop = 0; loop < _GP(views)[view].numLoops; ++loop) {
			if (!AssertGameObjectContent2(err, in->ReadInt32(), _GP(views)[view].loops[loop].numFrames,
			                              "Frame", "View", view, "Loop", loop))
				return err;
			for (int frame = 0; frame < _GP(views)[view].loops[loop].numFrames; ++frame) {
				_GP(views)[view].loops[loop].frames[frame].sound = in->ReadInt32();
				_GP(views)[view].loops[loop].frames[frame].pic = in->ReadInt32();
			}
		}
	}
	return err;
}

HSaveError WriteDynamicSprites(Stream *out) {
	const soff_t ref_pos = out->GetPosition();
	out->WriteInt32(0); // number of dynamic sprites
	out->WriteInt32(0); // top index
	int count = 0;
	int top_index = 1;
	for (size_t i = 1; i < _GP(spriteset).GetSpriteSlotCount(); ++i) {
		if (_GP(game).SpriteInfos[i].Flags & SPF_DYNAMICALLOC) {
			count++;
			top_index = i;
			out->WriteInt32(i);
			out->WriteInt32(_GP(game).SpriteInfos[i].Flags);
			serialize_bitmap(_GP(spriteset)[i], out);
		}
	}
	const soff_t end_pos = out->GetPosition();
	out->Seek(ref_pos, kSeekBegin);
	out->WriteInt32(count);
	out->WriteInt32(top_index);
	out->Seek(end_pos, kSeekBegin);
	return HSaveError::None();
}

HSaveError ReadDynamicSprites(Stream *in, int32_t /*cmp_ver*/, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	HSaveError err;
	const int spr_count = in->ReadInt32();
	// ensure the sprite set is at least large enough
	// to accommodate top dynamic sprite index
	const int top_index = in->ReadInt32();
	_GP(spriteset).EnlargeTo(top_index);
	for (int i = 0; i < spr_count; ++i) {
		int id = in->ReadInt32();
		int flags = in->ReadInt32();
		std::unique_ptr<Bitmap> image(read_serialized_bitmap(in));
		add_dynamic_sprite(id, std::move(image), (flags & SPF_ALPHACHANNEL) != 0, flags);
	}
	return err;
}

HSaveError WriteOverlays(Stream *out) {
	const auto &overs = get_overlays();
	// Calculate and save valid overlays only
	uint32_t valid_count = 0;
	soff_t count_off = out->GetPosition();
	out->WriteInt32(0);
	for (const auto &over : overs) {
		if (over.type < 0)
			continue;
		valid_count++;
		over.WriteToSavegame(out);
	}
	out->Seek(count_off, kSeekBegin);
	out->WriteInt32(valid_count);
	out->Seek(0, kSeekEnd);
	return HSaveError::None();
}

HSaveError ReadOverlays(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData &r_data) {
	// Remember that overlay indexes may be non-sequential
	// the vector may be resized during read
	size_t over_count = in->ReadInt32();
	auto &overs = get_overlays();
	overs.resize(over_count); // reserve minimal size
	for (size_t i = 0; i < over_count; ++i) {
		ScreenOverlay over;
		bool has_bitmap;
		over.ReadFromSavegame(in, has_bitmap, cmp_ver);
		if (over.type < 0)
			continue; // safety abort
		if (has_bitmap)
			r_data.OverlayImages[over.type].reset(read_serialized_bitmap(in));
		if (overs.size() <= static_cast<uint32_t>(over.type))
			overs.resize(over.type + 1);
		overs[over.type] = std::move(over);
	}
	return HSaveError::None();
}

HSaveError WriteDynamicSurfaces(Stream *out) {
	out->WriteInt32(MAX_DYNAMIC_SURFACES);
	for (int i = 0; i < MAX_DYNAMIC_SURFACES; ++i) {
		if (_G(dynamicallyCreatedSurfaces)[i] == nullptr) {
			out->WriteInt8(0);
		} else {
			out->WriteInt8(1);
			serialize_bitmap(_G(dynamicallyCreatedSurfaces)[i].get(), out);
		}
	}
	return HSaveError::None();
}

HSaveError ReadDynamicSurfaces(Stream *in, int32_t /*cmp_ver*/, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData &r_data) {
	HSaveError err;
	if (!AssertCompatLimit(err, in->ReadInt32(), MAX_DYNAMIC_SURFACES, "Dynamic Surfaces"))
		return err;
	// Load the surfaces into a temporary array since ccUnserialiseObjects will destroy them otherwise
	r_data.DynamicSurfaces.resize(MAX_DYNAMIC_SURFACES);
	for (int i = 0; i < MAX_DYNAMIC_SURFACES; ++i) {
		if (in->ReadInt8() == 0)
			r_data.DynamicSurfaces[i].reset();
		else
			r_data.DynamicSurfaces[i].reset(read_serialized_bitmap(in));
	}
	return err;
}

HSaveError WriteScriptModules(Stream *out) {
	// write the data segment of the global script
	int data_len = _G(gameinst)->globaldatasize;
	out->WriteInt32(data_len);
	if (data_len > 0)
		out->Write(_G(gameinst)->globaldata, data_len);
	// write the script modules data segments
	out->WriteInt32(_G(numScriptModules));
	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		data_len = _GP(moduleInst)[i]->globaldatasize;
		out->WriteInt32(data_len);
		if (data_len > 0)
			out->Write(_GP(moduleInst)[i]->globaldata, data_len);
	}
	return HSaveError::None();
}

HSaveError ReadScriptModules(Stream *in, int32_t /*cmp_ver*/, soff_t cmp_size, const PreservedParams &pp, RestoredData &r_data) {
	HSaveError err;
	// read the global script data segment
	int data_len = in->ReadInt32();
	if (!AssertGameContent(err, data_len, pp.GlScDataSize, "global script data"))
		return err;
	r_data.GlobalScript.Len = data_len;
	r_data.GlobalScript.Data.resize(data_len);
	if (data_len > 0)
		in->Read(&r_data.GlobalScript.Data.front(), data_len);

	if (!AssertGameContent(err, in->ReadInt32(), _G(numScriptModules), "Script Modules"))
		return err;
	r_data.ScriptModules.resize(_G(numScriptModules));
	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		data_len = in->ReadInt32();
		if (!AssertGameObjectContent(err, data_len, pp.ScMdDataSize[i], "script module data", "module", i))
			return err;
		r_data.ScriptModules[i].Len = data_len;
		r_data.ScriptModules[i].Data.resize(data_len);
		if (data_len > 0)
			in->Read(&r_data.ScriptModules[i].Data.front(), data_len);
	}
	return err;
}

HSaveError WriteRoomStates(Stream *out) {
	// write the room state for all the rooms the player has been in
	out->WriteInt32(MAX_ROOMS);
	for (int i = 0; i < MAX_ROOMS; ++i) {
		if (isRoomStatusValid(i)) {
			RoomStatus *roomstat = getRoomStatus(i);
			if (roomstat->beenhere) {
				out->WriteInt32(i);
				WriteFormatTag(out, "RoomState", true);
				roomstat->WriteToSavegame(out, _G(loaded_game_file_version));
				WriteFormatTag(out, "RoomState", false);
			} else
				out->WriteInt32(-1);
		} else
			out->WriteInt32(-1);
	}
	return HSaveError::None();
}

HSaveError ReadRoomStates(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	HSaveError err;
	int roomstat_count = in->ReadInt32();
	for (; roomstat_count > 0; --roomstat_count) {
		int id = in->ReadInt32();
		// If id == -1, then the player has not been there yet (or room state was reset)
		if (id != -1) {
			if (!AssertCompatRange(err, id, 0, MAX_ROOMS - 1, "room index"))
				return err;
			if (!AssertFormatTagStrict(err, in, "RoomState", true))
				return err;
			RoomStatus *roomstat = getRoomStatus(id);
			roomstat->ReadFromSavegame(in, _G(loaded_game_file_version), (RoomStatSvgVersion)cmp_ver);
			if (!AssertFormatTagStrict(err, in, "RoomState", false))
				return err;
		}
	}
	return HSaveError::None();
}

HSaveError WriteThisRoom(Stream *out) {
	out->WriteInt32(_G(displayed_room));
	if (_G(displayed_room) < 0)
		return HSaveError::None();

	// modified room backgrounds
	for (int i = 0; i < MAX_ROOM_BGFRAMES; ++i) {
		out->WriteBool(_GP(play).raw_modified[i] != 0);
		if (_GP(play).raw_modified[i])
			serialize_bitmap(_GP(thisroom).BgFrames[i].Graphic.get(), out);
	}
	out->WriteBool(_G(raw_saved_screen) != nullptr);
	if (_G(raw_saved_screen))
		serialize_bitmap(_G(raw_saved_screen).get(), out);

	// room region state
	for (int i = 0; i < MAX_ROOM_REGIONS; ++i) {
		out->WriteInt32(_GP(thisroom).Regions[i].Light);
		out->WriteInt32(_GP(thisroom).Regions[i].Tint);
	}
	for (int i = 0; i < MAX_WALK_AREAS; ++i) {
		out->WriteInt32(_GP(thisroom).WalkAreas[i].ScalingFar);
		out->WriteInt32(_GP(thisroom).WalkAreas[i].ScalingNear);
	}

	// room music volume
	out->WriteInt32(_GP(thisroom).Options.MusicVolume);

	// persistent room's indicator
	const bool persist = _G(displayed_room) < MAX_ROOMS;
	out->WriteBool(persist);
	// write the current troom state, in case they save in temporary room
	if (!persist)
		_GP(troom).WriteToSavegame(out, _G(loaded_game_file_version));
	return HSaveError::None();
}

HSaveError ReadThisRoom(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData &r_data) {
	HSaveError err;
	_G(displayed_room) = in->ReadInt32();
	if (_G(displayed_room) < 0)
		return err;

	// modified room backgrounds
	for (int i = 0; i < MAX_ROOM_BGFRAMES; ++i) {
		_GP(play).raw_modified[i] = in->ReadBool();
		if (_GP(play).raw_modified[i])
			r_data.RoomBkgScene[i].reset(read_serialized_bitmap(in));
		else
			r_data.RoomBkgScene[i] = nullptr;
	}
	if (in->ReadBool())
		_G(raw_saved_screen).reset(read_serialized_bitmap(in));

	// room region state
	for (int i = 0; i < MAX_ROOM_REGIONS; ++i) {
		r_data.RoomLightLevels[i] = in->ReadInt32();
		r_data.RoomTintLevels[i] = in->ReadInt32();
	}
	for (int i = 0; i < MAX_WALK_AREAS; ++i) {
		r_data.RoomZoomLevels1[i] = in->ReadInt32();
		r_data.RoomZoomLevels2[i] = in->ReadInt32();
	}

	// room object movement paths, for old saves
	if (cmp_ver < kRoomStatSvgVersion_36109) {
		int objmls_count = in->ReadInt32();
		if (!AssertCompatLimit(err, objmls_count, CHMLSOFFS, "room object move lists"))
			return err;
		for (int i = 0; i < objmls_count; ++i) {
			err = _GP(mls)[i].ReadFromSavegame(in, kMoveSvgVersion_350);
			if (!err)
				return err;
		}
	}

	// save the new room music vol for later use
	r_data.RoomVolume = (RoomVolumeMod)in->ReadInt32();

	// read the current troom state, in case they saved in temporary room
	if (!in->ReadBool())
		_GP(troom).ReadFromSavegame(in, _G(loaded_game_file_version), (RoomStatSvgVersion)cmp_ver);

	return HSaveError::None();
}

HSaveError WriteMoveLists(Stream *out) {
	out->WriteInt32(static_cast<int32_t>(_GP(mls).size()));
	for (const auto &movelist : _GP(mls)) {
		movelist.WriteToSavegame(out);
	}
	return HSaveError::None();
}

HSaveError ReadMoveLists(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	HSaveError err;
	size_t movelist_count = in->ReadInt32();
	// TODO: this assertion is needed only because mls size is fixed to the
	// number of characters + max number of objects, where each game object
	// has a fixed movelist index. It may be removed if movelists will be
	// allocated on demand with an arbitrary index instead.
	if (!AssertGameContent(err, movelist_count, _GP(mls).size(), "Move Lists"))
		return err;
	for (size_t i = 0; i < movelist_count; ++i) {
		err = _GP(mls)[i].ReadFromSavegame(in, cmp_ver);
		if (!err)
			return err;
	}
	return err;
}

HSaveError WriteManagedPool(Stream *out) {
	ccSerializeAllObjects(out);
	return HSaveError::None();
}

HSaveError ReadManagedPool(Stream *in, int32_t /*cmp_ver*/, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	if (ccUnserializeAllObjects(in, &_GP(ccUnserializer))) {
		return new SavegameError(kSvgErr_GameObjectInitFailed,
		                         String::FromFormat("Managed pool deserialization failed: %s", cc_get_error().ErrorString.GetCStr()));
	}
	return HSaveError::None();
}

HSaveError WritePluginData(Stream *out) {
	WritePluginSaveData(out);
	return HSaveError::None();
}

HSaveError ReadPluginData(Stream *in, int32_t cmp_ver, soff_t cmp_size, const PreservedParams & /*pp*/, RestoredData & /*r_data*/) {
	ReadPluginSaveData(in, static_cast<PluginSvgVersion>(cmp_ver), cmp_size);
	return HSaveError::None();
}


// Description of a supported game state serialization component
struct ComponentHandler {
	String             Name;    // internal component's ID
	int32_t            Version; // current version to write and the highest supported version
	int32_t            LowestVersion; // lowest supported version that the engine can read
	HSaveError(*Serialize)(Stream *);
	HSaveError(*Unserialize)(Stream *, int32_t cmp_ver, soff_t cmp_size, const PreservedParams &, RestoredData &);
};

// Array of supported components
struct ComponentHandlers {
	// NOTE: the new format values should now be defined as AGS version
	// at which a change was introduced, represented as NN,NN,NN,NN.
	const ComponentHandler _items[18] = {
		{
			"Game State",
			kGSSvgVersion_361_14,
			kGSSvgVersion_Initial,
			WriteGameState,
			ReadGameState
		},
		{
			"Audio",
			kAudioSvgVersion_36130,
			kAudioSvgVersion_Initial,
			WriteAudio,
			ReadAudio
		},
		{
			"Characters",
			kCharSvgVersion_36115,
			kCharSvgVersion_350, // skip pre-alpha 3.5.0 ver
			WriteCharacters,
			ReadCharacters
		},
		{
			"Dialogs",
			0,
			0,
			WriteDialogs,
			ReadDialogs
		},
		{
			"GUI",
			kGuiSvgVersion_36025,
			kGuiSvgVersion_Initial,
			WriteGUI,
			ReadGUI
		},
		{
			"Inventory Items",
			0,
			0,
			WriteInventory,
			ReadInventory
		},
		{
			"Mouse Cursors",
			kCursorSvgVersion_36016,
			kCursorSvgVersion_Initial,
			WriteMouseCursors,
			ReadMouseCursors
		},
		{
			"Views",
			0,
			0,
			WriteViews,
			ReadViews
		},
		{
			"Dynamic Sprites",
			0,
			0,
			WriteDynamicSprites,
			ReadDynamicSprites
		},
		{
			"Overlays",
			kOverSvgVersion_36108,
			kOverSvgVersion_Initial,
			WriteOverlays,
			ReadOverlays
		},
		{
			"Dynamic Surfaces",
			0,
			0,
			WriteDynamicSurfaces,
			ReadDynamicSurfaces
		},
		{
			"Script Modules",
			0,
			0,
			WriteScriptModules,
			ReadScriptModules
		},
		{
			"Room States",
			kRoomStatSvgVersion_36109,
			kRoomStatSvgVersion_350_Mismatch, // support mismatching 3.5.0 ver here
			WriteRoomStates,
			ReadRoomStates
		},
		{
			"Loaded Room State",
			kRoomStatSvgVersion_36109, // must correspond to "Room States"
			kRoomStatSvgVersion_350, // skip pre-alpha 3.5.0 ver
			WriteThisRoom,
			ReadThisRoom
		},
		{
			"Move Lists",
			kMoveSvgVersion_36109,
			kMoveSvgVersion_350, // skip pre-alpha 3.5.0 ver
			WriteMoveLists,
			ReadMoveLists
		},
		{
			"Managed Pool",
			0,
			0,
			WriteManagedPool,
			ReadManagedPool
		},
		{
			"Plugin Data",
			kPluginSvgVersion_36115,
			kPluginSvgVersion_Initial,
			WritePluginData,
			ReadPluginData
		},
		{ nullptr, 0, 0, nullptr, nullptr } // end of array
	};
	const ComponentHandler &operator[](uint idx) {
		return _items[idx];
	}
};
ComponentHandlers *g_componentHandlers;

void component_handlers_init() {
	g_componentHandlers = new ComponentHandlers();
}

void component_handlers_free() {
	delete g_componentHandlers;
}

typedef std::map<String, ComponentHandler> HandlersMap;
void GenerateHandlersMap(HandlersMap &map) {
	map.clear();
	for (int i = 0; !(*g_componentHandlers)[i].Name.IsEmpty(); ++i)
		map[(*g_componentHandlers)[i].Name] = (*g_componentHandlers)[i];
}

// A helper struct to pass to (de)serialization handlers
struct SvgCmpReadHelper {
	SavegameVersion       Version;  // general savegame version
	const PreservedParams &PP;      // previous game state kept for reference
	RestoredData          &RData;   // temporary storage for loaded data, that
	// will be applied after loading is done
	// The map of serialization handlers, one per supported component type ID
	HandlersMap            Handlers;

	SvgCmpReadHelper(SavegameVersion svg_version, const PreservedParams &pp, RestoredData &r_data)
		: Version(svg_version)
		, PP(pp)
		, RData(r_data) {
	}
};

// The basic information about deserialized component, used for debugging purposes
struct ComponentInfo {
	String  Name;       // internal component's ID
	int32_t Version;    // data format version
	soff_t  Offset;     // offset at which an opening tag is located
	soff_t  DataOffset; // offset at which component data begins
	soff_t  DataSize;   // expected size of component data

	ComponentInfo() : Version(-1), Offset(0), DataOffset(0), DataSize(0) {}
};

HSaveError ReadComponent(Stream *in, SvgCmpReadHelper &hlp, ComponentInfo &info) {
	info = ComponentInfo(); // reset in case of early error
	info.Offset = in->GetPosition();
	if (!ReadFormatTag(in, info.Name, true))
		return new SavegameError(kSvgErr_ComponentOpeningTagFormat);
	info.Version = in->ReadInt32();
	info.DataSize = hlp.Version >= kSvgVersion_Cmp_64bit ? in->ReadInt64() : in->ReadInt32();
	info.DataOffset = in->GetPosition();

	// WORKAROUND: For some period, the component "Dynamic Surfaces"
	// was mis-named as "Drawing Surfaces"
	String componentName = info.Name;
	if (componentName == "Drawing Surfaces")
		componentName = "Dynamic Surfaces";

	const ComponentHandler *handler = nullptr;
	std::map<String, ComponentHandler>::const_iterator it_hdr = hlp.Handlers.find(componentName);
	if (it_hdr != hlp.Handlers.end())
		handler = &it_hdr->_value;

	// WORKAROUND: Managed Pool was incorrectly set as version 1
	// in the codebase originally imported to ScummVM
	if (componentName == "Managed Pool" && info.Version == 1
		&& handler && handler->Version == 0)
		info.Version = 0;

	if (!handler || !handler->Unserialize)
		return new SavegameError(kSvgErr_UnsupportedComponent);
	if (info.Version > handler->Version || info.Version < handler->LowestVersion)
		return new SavegameError(kSvgErr_UnsupportedComponentVersion, String::FromFormat("Saved version: %d, supported: %d - %d", info.Version, handler->LowestVersion, handler->Version));
	HSaveError err = handler->Unserialize(in, info.Version, info.DataSize, hlp.PP, hlp.RData);
	if (!err)
		return err;
	if (in->GetPosition() - info.DataOffset != info.DataSize)
		return new SavegameError(kSvgErr_ComponentSizeMismatch, String::FromFormat("Expected: %llu, actual: %llu",
			static_cast<int64>(info.DataSize), static_cast<int64>(in->GetPosition() - info.DataOffset)));
	if (!AssertFormatTag(in, info.Name, false))
		return new SavegameError(kSvgErr_ComponentClosingTagFormat);
	return HSaveError::None();
}

HSaveError ReadAll(Stream *in, SavegameVersion svg_version, const PreservedParams &pp, RestoredData &r_data) {
	// Prepare a helper struct we will be passing to the block reading proc
	SvgCmpReadHelper hlp(svg_version, pp, r_data);
	GenerateHandlersMap(hlp.Handlers);

	size_t idx = 0;
	const String ComponentListTag = "Components";
	if (!AssertFormatTag(in, ComponentListTag, true))
		return new SavegameError(kSvgErr_ComponentListOpeningTagFormat);
	do {
		// Look out for the end of the component list:
		// this is the only way how this function ends with success
		soff_t off = in->GetPosition();
		if (AssertFormatTag(in, ComponentListTag, false))
			return HSaveError::None();
		// If the list's end was not detected, then seek back and continue reading
		in->Seek(off, kSeekBegin);

		ComponentInfo info;
		HSaveError err = ReadComponent(in, hlp, info);
		if (!err) {
			return new SavegameError(kSvgErr_ComponentUnserialization,
			    String::FromFormat("(#%d) %s, version %i, at offset %lld.",
					idx, info.Name.IsEmpty() ? "unknown" : info.Name.GetCStr(), info.Version, info.Offset),
				err);
		}
		idx++;
	} while (!in->EOS());
	return new SavegameError(kSvgErr_ComponentListClosingTagMissing);
}

HSaveError WriteComponent(Stream *out, const ComponentHandler &hdlr) {
	WriteFormatTag(out, hdlr.Name, true);
	out->WriteInt32(hdlr.Version);
	soff_t ref_pos = out->GetPosition();
	out->WriteInt64(0); // placeholder for the component size
	HSaveError err = hdlr.Serialize(out);
	soff_t end_pos = out->GetPosition();
	out->Seek(ref_pos, kSeekBegin);
	out->WriteInt64(end_pos - ref_pos - sizeof(int64_t)); // size of serialized component data
	out->Seek(end_pos, kSeekBegin);
	if (err)
		WriteFormatTag(out, hdlr.Name, false);
	return err;
}

HSaveError WriteAllCommon(Stream *out) {
	const String ComponentListTag = "Components";
	WriteFormatTag(out, ComponentListTag, true);
	for (int type = 0; !(*g_componentHandlers)[type].Name.IsEmpty(); ++type) {
		HSaveError err = WriteComponent(out, (*g_componentHandlers)[type]);
		if (!err) {
			return new SavegameError(kSvgErr_ComponentSerialization,
			                         String::FromFormat("Component: (#%d) %s", type, (*g_componentHandlers)[type].Name.GetCStr()),
			                         err);
		}
	}
	WriteFormatTag(out, ComponentListTag, false);
	return HSaveError::None();
}

} // namespace SavegameBlocks
} // namespace Engine
} // namespace AGS
} // namespace AGS3
