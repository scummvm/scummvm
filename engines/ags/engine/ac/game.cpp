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

#include "common/memstream.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/audio_channel.h"
#include "ags/engine/ac/button.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/dialog.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_object.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/hotspot.h"
#include "ags/shared/ac/keycode.h"
#include "ags/engine/ac/lip_sync.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/rich_game_media.h"
#include "ags/engine/ac/room_status.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/translation.h"
#include "ags/engine/ac/dynobj/all_dynamic_classes.h"
#include "ags/engine/ac/dynobj/all_script_classes.h"
#include "ags/engine/ac/dynobj/script_camera.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/game/savegame.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_slider.h"
#include "ags/engine/gui/gui_dialog.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/media/video/video.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/path.h"

#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

//=============================================================================
// Audio
//=============================================================================

void Game_StopAudio(int audioType) {
	if (((audioType < 0) || ((size_t)audioType >= _GP(game).audioClipTypes.size())) && (audioType != SCR_NO_VALUE))
		quitprintf("!Game.StopAudio: invalid audio type %d", audioType);

	for (int aa = 0; aa < _GP(game).numGameChannels; aa++) {
		if (audioType == SCR_NO_VALUE) {
			stop_or_fade_out_channel(aa);
		} else {
			ScriptAudioClip *clip = AudioChannel_GetPlayingClip(&_G(scrAudioChannel)[aa]);
			if ((clip != nullptr) && (clip->type == audioType))
				stop_or_fade_out_channel(aa);
		}
	}

	remove_clips_of_type_from_queue(audioType);
}

int Game_IsAudioPlaying(int audioType) {
	if (((audioType < 0) || ((size_t)audioType >= _GP(game).audioClipTypes.size())) && (audioType != SCR_NO_VALUE))
		quitprintf("!Game.IsAudioPlaying: invalid audio type %d", audioType);

	if (_GP(play).fast_forward)
		return 0;

	for (int aa = 0; aa < _GP(game).numGameChannels; aa++) {
		ScriptAudioClip *clip = AudioChannel_GetPlayingClip(&_G(scrAudioChannel)[aa]);
		if (clip != nullptr) {
			if ((clip->type == audioType) || (audioType == SCR_NO_VALUE)) {
				return 1;
			}
		}
	}
	return 0;
}

void Game_SetAudioTypeSpeechVolumeDrop(int audioType, int volumeDrop) {
	if ((audioType < 0) || ((size_t)audioType >= _GP(game).audioClipTypes.size()))
		quitprintf("!Game.SetAudioTypeVolume: invalid audio type: %d", audioType);

	Debug::Printf("Game.SetAudioTypeSpeechVolumeDrop: type: %d, drop: %d", audioType, volumeDrop);
	_GP(game).audioClipTypes[audioType].volume_reduction_while_speech_playing = volumeDrop;
	update_volume_drop_if_voiceover();
}

void Game_SetAudioTypeVolume(int audioType, int volume, int changeType) {
	if ((volume < 0) || (volume > 100))
		quitprintf("!Game.SetAudioTypeVolume: volume %d is not between 0..100", volume);
	if ((audioType < 0) || ((size_t)audioType >= _GP(game).audioClipTypes.size()))
		quitprintf("!Game.SetAudioTypeVolume: invalid audio type: %d", audioType);

	const char *change_str[3]{"existing", "future", "all"};
	Debug::Printf("Game.SetAudioTypeVolume: type: %d, volume: %d, change: %s", audioType, volume,
				  change_str[changeType - VOL_CHANGEEXISTING]);
	if ((changeType == VOL_CHANGEEXISTING) || (changeType == VOL_BOTH)) {
		for (int aa = 0; aa < _GP(game).numGameChannels; aa++) {
			ScriptAudioClip *clip = AudioChannel_GetPlayingClip(&_G(scrAudioChannel)[aa]);
			if ((clip != nullptr) && (clip->type == audioType)) {
				auto *ch = AudioChans::GetChannel(aa);
				if (ch)
					ch->set_volume100(volume);
			}
		}
	}

	if ((changeType == VOL_SETFUTUREDEFAULT) ||
	        (changeType == VOL_BOTH)) {
		_GP(play).default_audio_type_volumes[audioType] = volume;

		// update queued clip volumes
		update_queued_clips_volume(audioType, volume);
	}
}

int Game_GetMODPattern() {
	if (_G(current_music_type) != MUS_MOD)
		return -1;
	auto *music_ch = AudioChans::GetChannelIfPlaying(SCHAN_MUSIC);
	return music_ch ? music_ch->get_pos() : -1;
}

//=============================================================================
// ---
//=============================================================================

int Game_GetDialogCount() {
	return _GP(game).numdialog;
}

void set_debug_mode(bool on) {
	_GP(play).debug_mode = on ? 1 : 0;
	debug_set_console(on);
}

void set_game_speed(int new_fps) {
	_G(frames_per_second) = new_fps;
	if (!isTimerFpsMaxed()) // if in maxed mode, don't update timer for now
		setTimerFps(new_fps);
}

void setup_for_dialog() {
	_G(cbuttfont) = _GP(play).normal_font;
	_G(acdialog_font) = _GP(play).normal_font;
	_G(oldmouse) = _G(cur_cursor);
	set_mouse_cursor(CURS_ARROW);
}
void restore_after_dialog() {
	set_mouse_cursor(_G(oldmouse));
	invalidate_screen();
}

String get_save_game_directory() {
	return _G(saveGameDirectory);
}

String get_save_game_suffix() {
	return _G(saveGameSuffix);
}

void set_save_game_suffix(const String &suffix) {
	_G(saveGameSuffix) = suffix;
}

String get_save_game_filename(int slotNum) {
	return String(g_engine->getSaveStateName(slotNum).c_str());
}

String get_save_game_path(int slotNum) {
#if AGS_PLATFORM_SCUMMVM
	return Common::String::format("%s%s", SAVE_FOLDER_PREFIX,
		::AGS::g_vm->getSaveStateName(slotNum).c_str());
#else
	return Path::ConcatPaths(_G(saveGameDirectory), get_save_game_filename(slotNum));
#endif
}

#if !AGS_PLATFORM_SCUMMVM
// Convert a path possibly containing path tags into acceptable save path
// NOTE that the game script may issue an order to change the save directory to
// a dir of a new name. While we let this work, we also try to keep these
// inside same parent location, would that be a common system directory,
// or a custom one set by a player in config.
static bool MakeSaveGameDir(const String &new_dir, FSLocation &fsdir) {
	fsdir = FSLocation();
	// don't allow absolute paths
	if (!is_relative_filename(new_dir))
		return false;

	String fixed_newdir = FixSlashAfterToken(new_dir);
	if (fixed_newdir.CompareLeft(UserSavedgamesRootToken, UserSavedgamesRootToken.GetLength()) == 0) {
		fixed_newdir.ClipLeft(UserSavedgamesRootToken.GetLength());
	} else if (game.options[OPT_SAFEFILEPATHS] > 0) { // For games made in the safe-path-aware versions of AGS, report a warning
		debug_script_warn("Attempt to explicitly set savegame location relative to the game installation directory ('%s') denied;\nPath will be remapped to the user documents directory: '%s'",
			fixed_newdir.GetCStr(), fsdir.FullDir.GetCStr());
	}

	// Resolve the new dir relative to the user data parent dir
	fsdir = GetGameUserDataDir().Concat(fixed_newdir);
	return true;
}
#endif

// Tries to assign a new save directory, and copies the restart point if available
static bool SetSaveGameDirectory(const FSLocation &fsdir) {
	if (!Directory::CreateAllDirectories(fsdir.BaseDir, fsdir.FullDir)) {
		debug_script_warn("SetSaveGameDirectory: failed to create all subdirectories: %s", fsdir.FullDir.GetCStr());
		return false;
	}
	String newSaveGameDir = fsdir.FullDir;

	if (!File::TestCreateFile(Path::ConcatPaths(newSaveGameDir, "agstmp.tmp")))
		return false;

	// copy the Restart Game file, if applicable
	String restartGamePath = Path::ConcatPaths(_G(saveGameDirectory), get_save_game_filename(RESTART_POINT_SAVE_GAME_NUMBER));
	Stream *restartGameFile = File::OpenFileRead(restartGamePath);
	if (restartGameFile != nullptr) {
		long fileSize = restartGameFile->GetLength();
		char *mbuffer = (char *)malloc(fileSize);
		restartGameFile->Read(mbuffer, fileSize);
		delete restartGameFile;

		restartGamePath = Path::ConcatPaths(newSaveGameDir, get_save_game_filename(RESTART_POINT_SAVE_GAME_NUMBER));
		restartGameFile = File::CreateFile(restartGamePath);
		restartGameFile->Write(mbuffer, fileSize);
		delete restartGameFile;
		free(mbuffer);
	}

	_G(saveGameDirectory) = newSaveGameDir;
	return true;
}

void SetDefaultSaveDirectory() {
	// Request a default save location, and assign it as a save dir
	FSLocation fsdir = GetGameUserDataDir();
	SetSaveGameDirectory(fsdir);
}

int Game_SetSaveGameDirectory(const char *newFolder) {
#if AGS_PLATFORM_SCUMMVM
	return 1;
#else
	// First resolve the script path (it may contain tokens)
	FSLocation fsdir;
	if (!MakeSaveGameDir(newFolder, fsdir))
		return 0;
	// If resolved successfully, try to assign the new dir
	return SetSaveGameDirectory(fsdir) ? 1 : 0;
#endif
}


const char *Game_GetSaveSlotDescription(int slnum) {
	String description;
	if (read_savedgame_description(get_save_game_path(slnum), description)) {
		return CreateNewScriptString(description.GetCStr());
	}
	return nullptr;
}


void restore_game_dialog() {
	can_run_delayed_command();
	if (_GP(thisroom).Options.SaveLoadDisabled == 1) {
		DisplayMessage(983);
		return;
	}
	if (_G(inside_script)) {
		_G(curscript)->queue_action(ePSARestoreGameDialog, 0, "RestoreGameDialog");
		return;
	}
	do_restore_game_dialog();
}

bool do_restore_game_dialog() {
	setup_for_dialog();
	int toload = loadgamedialog();
	restore_after_dialog();
	if (toload >= 0)
		try_restore_save(toload);
	return toload >= 0;
}

void save_game_dialog() {
	if (_GP(thisroom).Options.SaveLoadDisabled == 1) {
		DisplayMessage(983);
		return;
	}
	if (_G(inside_script)) {
		_G(curscript)->queue_action(ePSASaveGameDialog, 0, "SaveGameDialog");
		return;
	}
	do_save_game_dialog();
}

bool do_save_game_dialog() {
	setup_for_dialog();
	int toload = savegamedialog();
	restore_after_dialog();
	if (toload >= 0)
		save_game(toload, get_gui_dialog_buffer());
	return toload >= 0;
}

void free_do_once_tokens() {
	_GP(play).do_once_tokens.resize(0);
}


// Free all the memory associated with the game
void unload_game_file() {
	close_translation();

	_GP(play).FreeViewportsAndCameras();

	_GP(charextra).clear();
	_GP(mls).clear();

	dispose_game_drawdata();

	delete _G(gameinstFork);
	delete _G(gameinst);
	_G(gameinstFork) = nullptr;
	_G(gameinst) = nullptr;
	_GP(gamescript).reset();

	delete _G(dialogScriptsInst);
	_G(dialogScriptsInst) = nullptr;
	_GP(dialogScriptsScript).reset();

	for (size_t i = 0; i < _G(numScriptModules); ++i) {
		delete _GP(moduleInstFork)[i];
		delete _GP(moduleInst)[i];
		_GP(scriptModules)[i].reset();
	}

	_GP(moduleInstFork).resize(0);
	_GP(moduleInst).resize(0);
	_GP(scriptModules).resize(0);
	_GP(repExecAlways).moduleHasFunction.resize(0);
	_GP(lateRepExecAlways).moduleHasFunction.resize(0);
	_GP(getDialogOptionsDimensionsFunc).moduleHasFunction.resize(0);
	_GP(renderDialogOptionsFunc).moduleHasFunction.resize(0);
	_GP(getDialogOptionUnderCursorFunc).moduleHasFunction.resize(0);
	_GP(runDialogOptionMouseClickHandlerFunc).moduleHasFunction.resize(0);
	_GP(runDialogOptionKeyPressHandlerFunc).moduleHasFunction.resize(0);
	_GP(runDialogOptionTextInputHandlerFunc).moduleHasFunction.resize(0);
	_GP(runDialogOptionRepExecFunc).moduleHasFunction.resize(0);
	_GP(runDialogOptionCloseFunc).moduleHasFunction.resize(0);
	_G(numScriptModules) = 0;

	_GP(views).clear();

	if (_G(splipsync) != nullptr) {
		for (int i = 0; i < _G(numLipLines); ++i) {
			free(_G(splipsync)[i].endtimeoffs);
			free(_G(splipsync)[i].frame);
		}
		free(_G(splipsync));
		_G(splipsync) = nullptr;
		_G(numLipLines) = 0;
		_G(curLipLine) = -1;
	}

	for (auto &dlg : _G(dialog)) {
		if (dlg.optionscripts != nullptr)
			free(dlg.optionscripts);
	}
	_G(dialog).clear();
	delete[] _G(scrDialog);
	_G(scrDialog) = nullptr;

	_GP(guis).clear();
	free(_G(scrGui));

	free_all_fonts();

	ccRemoveAllSymbols();
	ccUnregisterAllObjects();
	pl_stop_plugins();

	free_do_once_tokens();
	_GP(play).gui_draw_order.clear();

	resetRoomStatuses();

	// free game struct last because it contains object counts
	_GP(game).Free();
}






const char *Game_GetGlobalStrings(int index) {
	if ((index < 0) || (index >= MAXGLOBALSTRINGS))
		quit("!Game.GlobalStrings: invalid index");

	return CreateNewScriptString(_GP(play).globalstrings[index]);
}


// ** GetGameParameter replacement functions

int Game_GetInventoryItemCount() {
	// because of the dummy item 0, this is always one higher than it should be
	return _GP(game).numinvitems - 1;
}

int Game_GetFontCount() {
	return _GP(game).numfonts;
}

int Game_GetMouseCursorCount() {
	return _GP(game).numcursors;
}

int Game_GetCharacterCount() {
	return _GP(game).numcharacters;
}

int Game_GetGUICount() {
	return _GP(game).numgui;
}

int Game_GetViewCount() {
	return _GP(game).numviews;
}

int Game_GetUseNativeCoordinates() {
	return _GP(game).IsDataInNativeCoordinates() ? 1 : 0;
}

int Game_GetSpriteWidth(int spriteNum) {
	if (spriteNum < 0)
		return 0;

	if (!_GP(spriteset).DoesSpriteExist(spriteNum))
		return 0;

	return game_to_data_coord(_GP(game).SpriteInfos[spriteNum].Width);
}

int Game_GetSpriteHeight(int spriteNum) {
	if (spriteNum < 0)
		return 0;

	if (!_GP(spriteset).DoesSpriteExist(spriteNum))
		return 0;

	return game_to_data_coord(_GP(game).SpriteInfos[spriteNum].Height);
}

int Game_GetLoopCountForView(int viewNumber) {
	if ((viewNumber < 1) || (viewNumber > _GP(game).numviews))
		quit("!GetGameParameter: invalid view specified");

	return _GP(views)[viewNumber - 1].numLoops;
}

int Game_GetRunNextSettingForLoop(int viewNumber, int loopNumber) {
	if ((viewNumber < 1) || (viewNumber > _GP(game).numviews))
		quit("!GetGameParameter: invalid view specified");
	if ((loopNumber < 0) || (loopNumber >= _GP(views)[viewNumber - 1].numLoops))
		quit("!GetGameParameter: invalid loop specified");

	return (_GP(views)[viewNumber - 1].loops[loopNumber].RunNextLoop()) ? 1 : 0;
}

int Game_GetFrameCountForLoop(int viewNumber, int loopNumber) {
	if ((viewNumber < 1) || (viewNumber > _GP(game).numviews))
		quit("!GetGameParameter: invalid view specified");
	if ((loopNumber < 0) || (loopNumber >= _GP(views)[viewNumber - 1].numLoops))
		quit("!GetGameParameter: invalid loop specified");

	return _GP(views)[viewNumber - 1].loops[loopNumber].numFrames;
}

ScriptViewFrame *Game_GetViewFrame(int viewNumber, int loopNumber, int frame) {
	if ((viewNumber < 1) || (viewNumber > _GP(game).numviews))
		quit("!GetGameParameter: invalid view specified");
	if ((loopNumber < 0) || (loopNumber >= _GP(views)[viewNumber - 1].numLoops))
		quit("!GetGameParameter: invalid loop specified");
	if ((frame < 0) || (frame >= _GP(views)[viewNumber - 1].loops[loopNumber].numFrames))
		quit("!GetGameParameter: invalid frame specified");

	ScriptViewFrame *sdt = new ScriptViewFrame(viewNumber - 1, loopNumber, frame);
	ccRegisterManagedObject(sdt, sdt);
	return sdt;
}

int Game_DoOnceOnly(const char *token) {
	for (int i = 0; i < (int)_GP(play).do_once_tokens.size(); i++) {
		if (_GP(play).do_once_tokens[i] == token) {
			return 0;
		}
	}
	_GP(play).do_once_tokens.push_back(token);
	return 1;
}

int Game_GetTextReadingSpeed() {
	return _GP(play).text_speed;
}

void Game_SetTextReadingSpeed(int newTextSpeed) {
	if (newTextSpeed < 1)
		quitprintf("!Game.TextReadingSpeed: %d is an invalid speed", newTextSpeed);

	_GP(play).text_speed = newTextSpeed;
}

int Game_GetMinimumTextDisplayTimeMs() {
	return _GP(play).text_min_display_time_ms;
}

void Game_SetMinimumTextDisplayTimeMs(int newTextMinTime) {
	_GP(play).text_min_display_time_ms = newTextMinTime;
}

int Game_GetIgnoreUserInputAfterTextTimeoutMs() {
	return _GP(play).ignore_user_input_after_text_timeout_ms;
}

void Game_SetIgnoreUserInputAfterTextTimeoutMs(int newValueMs) {
	_GP(play).ignore_user_input_after_text_timeout_ms = newValueMs;
}

const char *Game_GetFileName() {
	return CreateNewScriptString(_GP(ResPaths).GamePak.Name.GetCStr());
}

const char *Game_GetName() {
	return CreateNewScriptString(_GP(play).game_name);
}

void Game_SetName(const char *newName) {
	strncpy(_GP(play).game_name, newName, 99);
	_GP(play).game_name[99] = 0;
	sys_window_set_title(_GP(play).game_name);
}

int Game_GetSkippingCutscene() {
	if (_GP(play).fast_forward) {
		return 1;
	}
	return 0;
}

int Game_GetInSkippableCutscene() {
	if (_GP(play).in_cutscene) {
		return 1;
	}
	return 0;
}

int Game_GetColorFromRGB(int red, int grn, int blu) {
	if ((red < 0) || (red > 255) || (grn < 0) || (grn > 255) ||
	        (blu < 0) || (blu > 255))
		quit("!GetColorFromRGB: colour values must be 0-255");

	if (_GP(game).color_depth == 1) {
		return makecol8(red, grn, blu);
	}

	int agscolor = ((blu >> 3) & 0x1f);
	agscolor += ((grn >> 2) & 0x3f) << 5;
	agscolor += ((red >> 3) & 0x1f) << 11;
	return agscolor;
}

const char *Game_InputBox(const char *msg) {
	char buffer[STD_BUFFER_SIZE];
	sc_inputbox(msg, buffer);
	return CreateNewScriptString(buffer);
}

const char *Game_GetLocationName(int x, int y) {
	char buffer[STD_BUFFER_SIZE];
	GetLocationName(x, y, buffer);
	return CreateNewScriptString(buffer);
}

const char *Game_GetGlobalMessages(int index) {
	if ((index < 500) || (index >= MAXGLOBALMES + 500)) {
		return nullptr;
	}
	char buffer[STD_BUFFER_SIZE];
	buffer[0] = 0;
	replace_tokens(get_translation(get_global_message(index)), buffer, STD_BUFFER_SIZE);
	return CreateNewScriptString(buffer);
}

int Game_GetSpeechFont() {
	return _GP(play).speech_font;
}
int Game_GetNormalFont() {
	return _GP(play).normal_font;
}

const char *Game_GetTranslationFilename() {
	char buffer[STD_BUFFER_SIZE];
	GetTranslationName(buffer);
	return CreateNewScriptString(buffer);
}

int Game_ChangeTranslation(const char *newFilename) {
	if ((newFilename == nullptr) || (newFilename[0] == 0)) { // switch back to default translation
		close_translation();
		_GP(usetup).translation = "";
		GUI::MarkForTranslationUpdate();
		return 1;
	}

	String oldTransFileName = get_translation_name();
	if (!init_translation(newFilename, oldTransFileName))
		return 0; // failed, kept previous translation

	_GP(usetup).translation = newFilename;
	GUI::MarkForTranslationUpdate();
	return 1;
}

const char *Game_GetSpeechVoxFilename() {
	return CreateNewScriptString(get_voicepak_name().GetCStr());
}

bool Game_ChangeSpeechVox(const char *newFilename) {
	if (!init_voicepak(newFilename)) {
		// if failed (and was not default)- fallback to default
		if (strlen(newFilename) > 0)
			init_voicepak();
		return false;
	}
	return true;
}

ScriptAudioClip *Game_GetAudioClip(int index) {
	if (index < 0 || (size_t)index >= _GP(game).audioClips.size())
		return nullptr;
	return &_GP(game).audioClips[index];
}

ScriptCamera *Game_GetCamera() {
	return _GP(play).GetScriptCamera(0);
}

int Game_GetCameraCount() {
	return _GP(play).GetRoomCameraCount();
}

ScriptCamera *Game_GetAnyCamera(int index) {
	return _GP(play).GetScriptCamera(index);
}

void Game_SimulateKeyPress(int key) {
	ags_simulate_keypress(static_cast<eAGSKeyCode>(key));
}

int Game_BlockingWaitSkipped() {
	return _GP(play).GetWaitSkipResult();
}

//=============================================================================

// save game functions



void serialize_bitmap(const Shared::Bitmap *thispic, Stream *out) {
	if (thispic != nullptr) {
		out->WriteInt32(thispic->GetWidth());
		out->WriteInt32(thispic->GetHeight());
		out->WriteInt32(thispic->GetColorDepth());
		for (int cc = 0; cc < thispic->GetHeight(); cc++) {
			switch (thispic->GetColorDepth()) {
			case 8:
			// CHECKME: originally, AGS does not use real BPP here, but simply divides color depth by 8;
			// therefore 15-bit bitmaps are saved only partially? is this a bug? or?
			case 15:
				out->WriteArray(&thispic->GetScanLine(cc)[0], thispic->GetWidth(), 1);
				break;
			case 16:
				out->WriteArrayOfInt16((const int16_t *)&thispic->GetScanLine(cc)[0], thispic->GetWidth());
				break;
			case 32:
				out->WriteArrayOfInt32((const int32_t *)&thispic->GetScanLine(cc)[0], thispic->GetWidth());
				break;
			}
		}
	}
}

// On Windows we could just use IIDFromString but this is _G(platform)-independent
void convert_guid_from_text_to_binary(const char *guidText, unsigned char *buffer) {
	guidText++; // skip {
	for (int bytesDone = 0; bytesDone < 16; bytesDone++) {
		if (*guidText == '-')
			guidText++;

		char tempString[3];
		tempString[0] = guidText[0];
		tempString[1] = guidText[1];
		tempString[2] = 0;
		uint thisByte = 0;
		sscanf(tempString, "%X", &thisByte);

		buffer[bytesDone] = thisByte;
		guidText += 2;
	}

	// Swap bytes to give correct GUID order
	unsigned char temp;
	temp = buffer[0];
	buffer[0] = buffer[3];
	buffer[3] = temp;
	temp = buffer[1];
	buffer[1] = buffer[2];
	buffer[2] = temp;
	temp = buffer[4];
	buffer[4] = buffer[5];
	buffer[5] = temp;
	temp = buffer[6];
	buffer[6] = buffer[7];
	buffer[7] = temp;
}

Bitmap *read_serialized_bitmap(Stream *in) {
	Bitmap *thispic;
	int picwid = in->ReadInt32();
	int pichit = in->ReadInt32();
	int piccoldep = in->ReadInt32();
	thispic = BitmapHelper::CreateBitmap(picwid, pichit, piccoldep);
	if (thispic == nullptr)
		return nullptr;
	for (int vv = 0; vv < pichit; vv++) {
		switch (piccoldep) {
		case 8:
		// CHECKME: originally, AGS does not use real BPP here, but simply divides color depth by 8
		case 15:
			in->ReadArray(thispic->GetScanLineForWriting(vv), picwid, 1);
			break;
		case 16:
			in->ReadArrayOfInt16((int16_t *)thispic->GetScanLineForWriting(vv), picwid);
			break;
		case 32:
			in->ReadArrayOfInt32((int32_t *)thispic->GetScanLineForWriting(vv), picwid);
			break;
		}
	}

	return thispic;
}

void skip_serialized_bitmap(Stream *in) {
	int picwid = in->ReadInt32();
	int pichit = in->ReadInt32();
	int piccoldep = in->ReadInt32();
	// CHECKME: originally, AGS does not use real BPP here, but simply divides color depth by 8
	int bpp = piccoldep / 8;
	in->Seek(picwid * pichit * bpp);
}

long write_screen_shot_for_vista(Stream *out, Bitmap *screenshot) {
	// Save the screenshot to a memory stream so we can access the raw data
	Common::MemoryWriteStreamDynamic bitmap(DisposeAfterUse::YES);
	screenshot->SaveToFile(bitmap, _G(palette));

	update_polled_stuff();

	// Write the bitmap to the output stream
	out->Write(bitmap.getData(), bitmap.size());

	return bitmap.size();
}

Bitmap *create_savegame_screenshot() {
	// Render the view without any UI elements
	int old_flags = _G(debug_flags);
	_G(debug_flags) |= DBG_NOIFACE;
	construct_game_scene(true);
	render_to_screen();
	_G(debug_flags) = old_flags;

	int usewid = data_to_game_coord(_GP(play).screenshot_width);
	int usehit = data_to_game_coord(_GP(play).screenshot_height);
	const Rect &viewport = _GP(play).GetMainViewport();
	if (usewid > viewport.GetWidth())
		usewid = viewport.GetWidth();
	if (usehit > viewport.GetHeight())
		usehit = viewport.GetHeight();

	if ((_GP(play).screenshot_width < 16) || (_GP(play).screenshot_height < 16))
		quit("!Invalid game.screenshot_width/height, must be from 16x16 to screen res");

	Bitmap *screenshot = CopyScreenIntoBitmap(usewid, usehit);
	screenshot->GetAllegroBitmap()->makeOpaque();

	// Restore original screen
	construct_game_scene(true);
	render_to_screen();

	return screenshot;
}

void save_game(int slotn, const char *descript) {
	// dont allow save in rep_exec_always, because we dont save
	// the state of blocked scripts
	can_run_delayed_command();

	if (_G(inside_script)) {
		snprintf(_G(curscript)->postScriptSaveSlotDescription[_G(curscript)->queue_action(ePSASaveGame, slotn, "SaveGameSlot")],
					 MAX_QUEUED_ACTION_DESC, "%s", descript);
		return;
	}

	if (_G(platform)->GetDiskFreeSpaceMB() < 2) {
		Display("ERROR: There is not enough disk space free to save the game. Clear some disk space and try again.");
		return;
	}

	VALIDATE_STRING(descript);
	String nametouse = get_save_game_path(slotn);
	std::unique_ptr<Bitmap> screenShot;

	// WORKAROUND: AGS originally only creates savegames if the game flags
	// that it supports it. But we want it all the time for ScummVM GMM
	if (/*_GP(game).options[OPT_SAVESCREENSHOT] != 0*/ true)
		screenShot.reset(create_savegame_screenshot());

	std::unique_ptr<Stream> out(StartSavegame(nametouse, descript, screenShot.get()));
	if (out == nullptr) {
		Display("ERROR: Unable to open savegame file for writing!");
		return;
	}

	// Actual dynamic game data is saved here
	SaveGameState(out.get());

	if (screenShot != nullptr) {
		int screenShotOffset = out->GetPosition() - sizeof(RICH_GAME_MEDIA_HEADER);
		int screenShotSize = write_screen_shot_for_vista(out.get(), screenShot.get());

		update_polled_stuff();  // TODO: probably unneeded

		out->Seek(12, kSeekBegin);
		out->WriteInt32(screenShotOffset);
		out->Seek(4);
		out->WriteInt32(screenShotSize);
	}
}

bool read_savedgame_description(const String &savedgame, String &description) {
	SavegameDescription desc;
	HSaveError err = OpenSavegame(savedgame, desc, kSvgDesc_UserText);
	if (!err) {
		Debug::Printf(kDbgMsg_Error, "Unable to read save's description.\n%s", err->FullMessage().GetCStr());
		return false;
	}
	description = desc.UserText;
	return true;
}

bool read_savedgame_screenshot(const String &savedgame, int &want_shot) {
	want_shot = 0;

	SavegameDescription desc;
	HSaveError err = OpenSavegame(savedgame, desc, kSvgDesc_UserImage);
	if (!err) {
		Debug::Printf(kDbgMsg_Error, "Unable to read save's screenshot.\n%s", err->FullMessage().GetCStr());
		return false;
	}

	if (desc.UserImage.get()) {
		int slot = _GP(spriteset).GetFreeIndex();
		if (slot > 0) {
			// add it into the sprite set
			add_dynamic_sprite(slot, PrepareSpriteForUse(desc.UserImage.release(), false));
			want_shot = slot;
		}
	}
	return true;
}


// Test if the game file contains expected GUID / legacy id
bool test_game_guid(const String &filepath, const String &guid, int legacy_id) {
	std::unique_ptr<AssetManager> amgr(new AssetManager());
	if (amgr->AddLibrary(filepath) != kAssetNoError)
		return false;
	MainGameSource src;
	if (!OpenMainGameFileFromDefaultAsset(src, amgr.get()))
		return false;
	GameSetupStruct g;
	PreReadGameData(g, src.InputStream.get(), src.DataVersion);
	if (!guid.IsEmpty())
		return guid.CompareNoCase(g.guid) == 0;
	return legacy_id == g.uniqueid;
}

static const SavegameDescription *loadDesc;
static bool TestGame(const String &filepath) {
	return test_game_guid(filepath, loadDesc->GameGuid, loadDesc->LegacyID);
}

HSaveError load_game(const String &path, int slotNumber, bool &data_overwritten) {
	data_overwritten = false;
	_G(gameHasBeenRestored)++;

	_G(oldeip) = _G(our_eip);
	_G(our_eip) = 2050;

	HSaveError err;
	SavegameSource src;
	SavegameDescription desc;
	err = OpenSavegame(path, src, desc, kSvgDesc_EnvInfo);

	// saved in incompatible enviroment
	if (!err)
		return err;
	// CHECKME: is this color depth test still essential? if yes, is there possible workaround?
	else if (desc.ColorDepth != _GP(game).GetColorDepth())
		return new SavegameError(kSvgErr_DifferentColorDepth, String::FromFormat("Running: %d-bit, saved in: %d-bit.", _GP(game).GetColorDepth(), desc.ColorDepth));

	// saved with different game file
	// if savegame is modern enough then test game GUIDs
	if (!desc.GameGuid.IsEmpty() || desc.LegacyID != 0) {
		if (desc.GameGuid.Compare(_GP(game).guid) != 0 && desc.LegacyID != _GP(game).uniqueid) {
			// Try to find wanted game's data using game id
			loadDesc = &desc;
			String gamefile = FindGameData(_GP(ResPaths).DataDir, TestGame);

			if (Shared::File::TestReadFile(gamefile)) {
				RunAGSGame(gamefile.GetCStr(), 0, 0);
				_G(load_new_game_restore) = slotNumber;
				return HSaveError::None();
			}
			return new SavegameError(kSvgErr_GameGuidMismatch);
		}
	}
	// if it's old then do the stupid old-style filename test
	// TODO: remove filename test after deprecating old saves
	else if (desc.MainDataFilename.Compare(_GP(ResPaths).GamePak.Name)) {
		String gamefile = Path::ConcatPaths(_GP(ResPaths).DataDir, desc.MainDataFilename);
		if (Shared::File::TestReadFile(gamefile)) {
			RunAGSGame(desc.MainDataFilename, 0, 0);
			_G(load_new_game_restore) = slotNumber;
			return HSaveError::None();
		}
		// if it does not exist, continue loading savedgame in current game, and pray for the best
		Shared::Debug::Printf(kDbgMsg_Warn, "WARNING: the saved game '%s' references game file '%s' (title: '%s'), but it cannot be found in the current directory. Trying to restore in the running game instead.",
		                      path.GetCStr(), desc.MainDataFilename.GetCStr(), desc.GameTitle.GetCStr());
	}

	// do the actual restore
	err = RestoreGameState(src.InputStream.get(), src.Version);
	data_overwritten = true;
	if (!err)
		return err;
	src.InputStream.reset();
	_G(our_eip) = _G(oldeip);

	// ensure input state is reset
	ags_clear_input_state();
	// call "After Restore" event callback
	run_on_event(GE_RESTORE_GAME, RuntimeScriptValue().SetInt32(slotNumber));
	return HSaveError::None();
}

bool try_restore_save(int slot) {
	return try_restore_save(get_save_game_path(slot), slot);
}

bool try_restore_save(const Shared::String &path, int slot) {
	bool data_overwritten;
	HSaveError err = load_game(path, slot, data_overwritten);
	if (!err) {
		String error = String::FromFormat("Unable to restore the saved game.\n%s",
		                                  err->FullMessage().GetCStr());
		Debug::Printf(kDbgMsg_Error, "%s", error.GetCStr());
		// currently AGS cannot properly revert to stable state if some of the
		// game data was released or overwritten by the data from save file,
		// this is why we tell engine to shutdown if that happened.
		if (data_overwritten)
			quitprintf("%s", error.GetCStr());
		else
			Display(error.GetCStr());
		return false;
	}
	return true;
}

bool is_in_cutscene() {
	return _GP(play).in_cutscene > 0;
}

CutsceneSkipStyle get_cutscene_skipstyle() {
	return static_cast<CutsceneSkipStyle>(_GP(play).in_cutscene);
}

void start_skipping_cutscene() {
	_GP(play).fast_forward = 1;
	// if a drop-down icon bar is up, remove it as it will pause the game
	if (_G(ifacepopped) >= 0)
		remove_popup_interface(_G(ifacepopped));

	// if a text message is currently displayed, remove it
	if (_GP(play).text_overlay_on > 0) {
		remove_screen_overlay(_GP(play).text_overlay_on);
		_GP(play).SetWaitSkipResult(SKIP_AUTOTIMER);
	}
}

bool check_skip_cutscene_keypress(int kgn) {

	CutsceneSkipStyle skip = get_cutscene_skipstyle();
	if (skip == eSkipSceneAnyKey || skip == eSkipSceneKeyMouse ||
	        (kgn == eAGSKeyCodeEscape && (skip == eSkipSceneEscOnly || skip == eSkipSceneEscOrRMB))) {
		start_skipping_cutscene();
		return true;
	}
	return false;
}

bool check_skip_cutscene_mclick(int mbut) {
	CutsceneSkipStyle skip = get_cutscene_skipstyle();
	if (skip == eSkipSceneMouse || skip == eSkipSceneKeyMouse ||
	        (mbut == kMouseRight && skip == eSkipSceneEscOrRMB)) {
		start_skipping_cutscene();
		return true;
	}
	return false;
}

// Helper functions used by StartCutscene/EndCutscene, but also
// by SkipUntilCharacterStops
void initialize_skippable_cutscene() {
	_GP(play).end_cutscene_music = -1;
}

void stop_fast_forwarding() {
	// when the skipping of a cutscene comes to an end, update things
	_GP(play).fast_forward = 0;
	setpal();
	if (_GP(play).end_cutscene_music >= 0)
		newmusic(_GP(play).end_cutscene_music);

	// Restore actual volume of sounds
	for (int aa = 0; aa < TOTAL_AUDIO_CHANNELS; aa++) {
		auto *ch = AudioChans::GetChannelIfPlaying(aa);
		if (ch) {
			ch->set_mute(false);
		}
	}

	update_music_volume();
}

// allowHotspot0 defines whether Hotspot 0 returns LOCTYPE_HOTSPOT
// or whether it returns 0
int __GetLocationType(int xxx, int yyy, int allowHotspot0) {
	_G(getloctype_index) = 0;
	// If it's not in ProcessClick, then return 0 when over a GUI
	if ((GetGUIAt(xxx, yyy) >= 0) && (_G(getloctype_throughgui) == 0))
		return 0;

	_G(getloctype_throughgui) = 0;

	const int scrx = xxx;
	const int scry = yyy;
	VpPoint vpt = _GP(play).ScreenToRoomDivDown(xxx, yyy);
	if (vpt.second < 0)
		return 0;
	xxx = vpt.first.X;
	yyy = vpt.first.Y;
	if ((xxx >= _GP(thisroom).Width) | (xxx < 0) | (yyy < 0) | (yyy >= _GP(thisroom).Height))
		return 0;

	// check characters, objects and walkbehinds, work out which is
	// foremost visible to the player
	int charat = is_pos_on_character(xxx, yyy);
	int hsat = get_hotspot_at(xxx, yyy);
	int objat = GetObjectIDAtScreen(scrx, scry);

	data_to_game_coords(&xxx, &yyy);

	int wbat = _GP(thisroom).WalkBehindMask->GetPixel(xxx, yyy);

	if (wbat <= 0) wbat = 0;
	else wbat = _G(croom)->walkbehind_base[wbat];

	int winner = 0;
	// if it's an Ignore Walkbehinds object, then ignore the walkbehind
	if ((objat >= 0) && ((_G(objs)[objat].flags & OBJF_NOWALKBEHINDS) != 0))
		wbat = 0;
	if ((charat >= 0) && ((_GP(game).chars[charat].flags & CHF_NOWALKBEHINDS) != 0))
		wbat = 0;

	if ((charat >= 0) && (objat >= 0)) {
		if ((wbat > _G(obj_lowest_yp)) && (wbat > _G(char_lowest_yp)))
			winner = LOCTYPE_HOTSPOT;
		else if (_G(obj_lowest_yp) > _G(char_lowest_yp))
			winner = LOCTYPE_OBJ;
		else
			winner = LOCTYPE_CHAR;
	} else if (charat >= 0) {
		if (wbat > _G(char_lowest_yp))
			winner = LOCTYPE_HOTSPOT;
		else
			winner = LOCTYPE_CHAR;
	} else if (objat >= 0) {
		if (wbat > _G(obj_lowest_yp))
			winner = LOCTYPE_HOTSPOT;
		else
			winner = LOCTYPE_OBJ;
	}

	if (winner == 0) {
		if (hsat >= 0)
			winner = LOCTYPE_HOTSPOT;
	}

	if ((winner == LOCTYPE_HOTSPOT) && (!allowHotspot0) && (hsat == 0))
		winner = 0;

	if (winner == LOCTYPE_HOTSPOT)
		_G(getloctype_index) = hsat;
	else if (winner == LOCTYPE_CHAR)
		_G(getloctype_index) = charat;
	else if (winner == LOCTYPE_OBJ)
		_G(getloctype_index) = objat;

	return winner;
}

// Called whenever game looses input focus
void display_switch_out() {
	Debug::Printf("Switching out from the game");
	_G(switched_away) = true;
	ags_clear_input_state();
	// Always unlock mouse when switching out from the game
	_GP(mouse).UnlockFromWindow();
}

// Called when game looses input focus and must pause until focus is returned
void display_switch_out_suspend() {
	Debug::Printf("Suspending the game on switch out");
	_G(switching_away_from_game)++;
	_G(game_update_suspend)++;
	display_switch_out();

	_G(platform)->PauseApplication();

	// TODO: find out if anything has to be done here for SDL backend

	video_pause();

	// Pause all the sounds
	for (int i = 0; i < TOTAL_AUDIO_CHANNELS; i++) {
		auto *ch = AudioChans::GetChannelIfPlaying(i);
		if (ch) {
			ch->pause();
		}
	}

	_G(switching_away_from_game)--;
}

// Called whenever game gets input focus
void display_switch_in() {
	Debug::Printf("Switching back into the game");
	ags_clear_input_state();
	// If auto lock option is set, lock mouse to the game window
	if (_GP(usetup).mouse_auto_lock && _GP(scsystem).windowed)
		_GP(mouse).TryLockToWindow();
	_G(switched_away) = false;
}

// Called when game gets input focus and must resume after pause
void display_switch_in_resume() {
	Debug::Printf("Resuming the game on switch in");
	display_switch_in();

	// Resume all the sounds
	for (int i = 0; i < TOTAL_AUDIO_CHANNELS; i++) {
		auto *ch = AudioChans::GetChannelIfPlaying(i);
		if (ch) {
			ch->resume();
		}
	}
	video_resume();

	// clear the screen if necessary
	if (_G(gfxDriver) && _G(gfxDriver)->UsesMemoryBackBuffer())
		_G(gfxDriver)->ClearRectangle(0, 0, _GP(game).GetGameRes().Width - 1, _GP(game).GetGameRes().Height - 1, nullptr);

	// TODO: find out if anything has to be done here for SDL backend

	_G(platform)->ResumeApplication();
	_G(game_update_suspend)--;
}

void replace_tokens(const char *srcmes, char *destm, int maxlen) {
	int indxdest = 0, indxsrc = 0;
	const char *srcp;
	char *destp;
	while (srcmes[indxsrc] != 0) {
		srcp = &srcmes[indxsrc];
		destp = &destm[indxdest];
		if ((strncmp(srcp, "@IN", 3) == 0) || (strncmp(srcp, "@GI", 3) == 0)) {
			int tokentype = 0;
			if (srcp[1] == 'I') tokentype = 1;
			else tokentype = 2;
			int inx = atoi(&srcp[3]);
			srcp++;
			indxsrc += 2;
			while (srcp[0] != '@') {
				if (srcp[0] == 0) quit("!Display: special token not terminated");
				srcp++;
				indxsrc++;
			}
			char tval[10];
			if (tokentype == 1) {
				if ((inx < 1) | (inx >= _GP(game).numinvitems))
					quit("!Display: invalid inv item specified in @IN@");
				snprintf(tval, sizeof(tval), "%d", _G(playerchar)->inv[inx]);
			} else {
				if ((inx < 0) | (inx >= MAXGSVALUES))
					quit("!Display: invalid global int index speicifed in @GI@");
				snprintf(tval, sizeof(tval), "%d", GetGlobalInt(inx));
			}
			snprintf(destp, maxlen, "%s", tval);
			indxdest += strlen(tval);
		} else {
			destp[0] = srcp[0];
			indxdest++;
			indxsrc++;
		}
		if (indxdest >= maxlen - 3)
			break;
	}
	destm[indxdest] = 0;
}

const char *get_global_message(int msnum) {
	if (_GP(game).messages[msnum - 500].IsEmpty())
		return "";
	return get_translation(_GP(game).messages[msnum - 500].GetCStr());
}

void get_message_text(int msnum, char *buffer, char giveErr) {
	int maxlen = 9999;
	if (!giveErr)
		maxlen = MAX_MAXSTRLEN;

	if (msnum >= 500) {

		if ((msnum >= MAXGLOBALMES + 500) || (_GP(game).messages[msnum - 500].IsEmpty())) {
			if (giveErr)
				quit("!DisplayGlobalMessage: message does not exist");
			buffer[0] = 0;
			return;
		}
		buffer[0] = 0;
		replace_tokens(get_translation(_GP(game).messages[msnum - 500].GetCStr()), buffer, maxlen);
		return;
	} else if (msnum < 0 || (size_t)msnum >= _GP(thisroom).MessageCount) {
		if (giveErr)
			quit("!DisplayMessage: Invalid message number to display");
		buffer[0] = 0;
		return;
	}

	buffer[0] = 0;
	replace_tokens(get_translation(_GP(thisroom).Messages[msnum].GetCStr()), buffer, maxlen);
}

bool unserialize_audio_script_object(int index, const char *objectType, Stream *in, size_t data_sz) {
	if (strcmp(objectType, "AudioChannel") == 0) {
		_GP(ccDynamicAudio).Unserialize(index, in, data_sz);
	} else if (strcmp(objectType, "AudioClip") == 0) {
		_GP(ccDynamicAudioClip).Unserialize(index, in, data_sz);
	} else {
		return false;
	}
	return true;
}

void game_sprite_updated(int sprnum) {
	// update the shared texture (if exists)
	_G(gfxDriver)->UpdateSharedDDB(sprnum, _GP(spriteset)[sprnum], (_GP(game).SpriteInfos[sprnum].Flags & SPF_ALPHACHANNEL) != 0, false);

	// character and object draw caches
	reset_objcache_for_sprite(sprnum, false);

	// gui backgrounds
	for (auto &gui : _GP(guis)) {
		if (gui.BgImage == sprnum) {
			gui.MarkChanged();
		}
	}
	// gui buttons
	for (auto &but : _GP(guibuts)) {
		if (but.CurrentImage == sprnum) {
			but.MarkChanged();
		}
	}
	// gui sliders
	for (auto &slider : _GP(guislider)) {
		if ((slider.BgImage == sprnum) || (slider.HandleImage == sprnum)) {
			slider.MarkChanged();
		}
	}
	// overlays
	for (auto &over : _GP(screenover)) {
		if (over.GetSpriteNum() == sprnum)
			over.MarkChanged();
	}
}

void game_sprite_deleted(int sprnum) {
	// clear from texture cache
	_G(gfxDriver)->ClearSharedDDB(sprnum);
	// character and object draw caches
	reset_objcache_for_sprite(sprnum, true);

	// This is ugly, but apparently there are few games that may rely
	// (either with or without author's intent) on newly created sprite
	// being assigned same index as a recently deleted one, which results
	// in new sprite "secretly" taking place of an old one on the GUI, etc.
	// So for old games we keep only partial reset (full cleanup is 3.5.0+).
	const bool reset_sprindex_oldstyle =
		_G(loaded_game_file_version) < kGameVersion_350;

	// room object graphics
	if (_G(croom) != nullptr) {
		for (size_t i = 0; i < (size_t)_G(croom)->numobj; ++i) {
			if (_G(objs)[i].num == sprnum)
				_G(objs)[i].num = 0;
		}
	}
	// gui buttons
	for (auto &but : _GP(guibuts)) {
		if (but.Image == sprnum)
			but.Image = 0;
		if (but.MouseOverImage == sprnum)
			but.MouseOverImage = 0;
		if (but.PushedImage == sprnum)
			but.PushedImage = 0;

		if (but.CurrentImage == sprnum) {
			but.CurrentImage = 0;
			but.MarkChanged();
		}
	}

	if (reset_sprindex_oldstyle)
		return; // stop here for < 3.5.0 games

	// gui backgrounds
	for (size_t i = 0; i < (size_t)_GP(game).numgui; ++i) {
		if (_GP(guis)[i].BgImage == sprnum) {
			_GP(guis)[i].BgImage = 0;
			_GP(guis)[i].MarkChanged();
		}
	}
	// gui sliders
	for (auto &slider : _GP(guislider)) {
		if ((slider.BgImage == sprnum) || (slider.HandleImage == sprnum))
			slider.MarkChanged();
		if (slider.BgImage == sprnum)
			slider.BgImage = 0;
		if (slider.HandleImage == sprnum)
			slider.HandleImage = 0;
	}
	// views
	for (size_t v = 0; v < (size_t)_GP(game).numviews; ++v) {
		for (size_t l = 0; l < (size_t)_GP(views)[v].numLoops; ++l) {
			for (size_t f = 0; f < (size_t)_GP(views)[v].loops[l].numFrames; ++f) {
				if (_GP(views)[v].loops[l].frames[f].pic == sprnum)
					_GP(views)[v].loops[l].frames[f].pic = 0;
			}
		}
	}
	// overlays
	for (auto &over : _GP(screenover)) {
		if (over.GetSpriteNum() == sprnum)
			over.SetSpriteNum(0);
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// int  (int audioType);
RuntimeScriptValue Sc_Game_IsAudioPlaying(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(Game_IsAudioPlaying);
}

// void (int audioType, int volumeDrop)
RuntimeScriptValue Sc_Game_SetAudioTypeSpeechVolumeDrop(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(Game_SetAudioTypeSpeechVolumeDrop);
}

// void (int audioType, int volume, int changeType)
RuntimeScriptValue Sc_Game_SetAudioTypeVolume(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(Game_SetAudioTypeVolume);
}

// void (int audioType)
RuntimeScriptValue Sc_Game_StopAudio(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Game_StopAudio);
}

// int (const char *newFilename)
RuntimeScriptValue Sc_Game_ChangeTranslation(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(Game_ChangeTranslation, const char);
}

RuntimeScriptValue Sc_Game_ChangeSpeechVox(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL_POBJ(Game_ChangeSpeechVox, const char);
}

// int (const char *token)
RuntimeScriptValue Sc_Game_DoOnceOnly(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(Game_DoOnceOnly, const char);
}

// int (int red, int grn, int blu)
RuntimeScriptValue Sc_Game_GetColorFromRGB(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT3(Game_GetColorFromRGB);
}

// int (int viewNumber, int loopNumber)
RuntimeScriptValue Sc_Game_GetFrameCountForLoop(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(Game_GetFrameCountForLoop);
}

// const char* (int x, int y)
RuntimeScriptValue Sc_Game_GetLocationName(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ_PINT2(const char, _GP(myScriptStringImpl), Game_GetLocationName);
}

// int (int viewNumber)
RuntimeScriptValue Sc_Game_GetLoopCountForView(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(Game_GetLoopCountForView);
}

// int ()
RuntimeScriptValue Sc_Game_GetMODPattern(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetMODPattern);
}

// int (int viewNumber, int loopNumber)
RuntimeScriptValue Sc_Game_GetRunNextSettingForLoop(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT2(Game_GetRunNextSettingForLoop);
}

// const char* (int slnum)
RuntimeScriptValue Sc_Game_GetSaveSlotDescription(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ_PINT(const char, _GP(myScriptStringImpl), Game_GetSaveSlotDescription);
}

// ScriptViewFrame* (int viewNumber, int loopNumber, int frame)
RuntimeScriptValue Sc_Game_GetViewFrame(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT3(ScriptViewFrame, Game_GetViewFrame);
}

// const char* (const char *msg)
RuntimeScriptValue Sc_Game_InputBox(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ_POBJ(const char, _GP(myScriptStringImpl), Game_InputBox, const char);
}

// int (const char *newFolder)
RuntimeScriptValue Sc_Game_SetSaveGameDirectory(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(Game_SetSaveGameDirectory, const char);
}

// void (int evenAmbient);
RuntimeScriptValue Sc_StopAllSounds(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(StopAllSounds);
}

// int ()
RuntimeScriptValue Sc_Game_GetCharacterCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetCharacterCount);
}

// int ()
RuntimeScriptValue Sc_Game_GetDialogCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetDialogCount);
}

// const char *()
RuntimeScriptValue Sc_Game_GetFileName(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ(const char, _GP(myScriptStringImpl), Game_GetFileName);
}

// int ()
RuntimeScriptValue Sc_Game_GetFontCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetFontCount);
}

// const char* (int index)
RuntimeScriptValue Sc_Game_GetGlobalMessages(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ_PINT(const char, _GP(myScriptStringImpl), Game_GetGlobalMessages);
}

// const char* (int index)
RuntimeScriptValue Sc_Game_GetGlobalStrings(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ_PINT(const char, _GP(myScriptStringImpl), Game_GetGlobalStrings);
}

// void  (int index, char *newval);
RuntimeScriptValue Sc_SetGlobalString(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT_POBJ(SetGlobalString, const char);
}

// int ()
RuntimeScriptValue Sc_Game_GetGUICount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetGUICount);
}

// int ()
RuntimeScriptValue Sc_Game_GetIgnoreUserInputAfterTextTimeoutMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetIgnoreUserInputAfterTextTimeoutMs);
}

// void (int newValueMs)
RuntimeScriptValue Sc_Game_SetIgnoreUserInputAfterTextTimeoutMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Game_SetIgnoreUserInputAfterTextTimeoutMs);
}

// int ()
RuntimeScriptValue Sc_Game_GetInSkippableCutscene(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetInSkippableCutscene);
}

// int ()
RuntimeScriptValue Sc_Game_GetInventoryItemCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetInventoryItemCount);
}

// int ()
RuntimeScriptValue Sc_Game_GetMinimumTextDisplayTimeMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetMinimumTextDisplayTimeMs);
}

// void (int newTextMinTime)
RuntimeScriptValue Sc_Game_SetMinimumTextDisplayTimeMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Game_SetMinimumTextDisplayTimeMs);
}

// int ()
RuntimeScriptValue Sc_Game_GetMouseCursorCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetMouseCursorCount);
}

// const char *()
RuntimeScriptValue Sc_Game_GetName(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ(const char, _GP(myScriptStringImpl), Game_GetName);
}

// void (const char *newName)
RuntimeScriptValue Sc_Game_SetName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_POBJ(Game_SetName, const char);
}

// int ()
RuntimeScriptValue Sc_Game_GetNormalFont(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetNormalFont);
}

// void  (int fontnum);
RuntimeScriptValue Sc_SetNormalFont(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetNormalFont);
}

// int ()
RuntimeScriptValue Sc_Game_GetSkippingCutscene(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetSkippingCutscene);
}

// int ()
RuntimeScriptValue Sc_Game_GetSpeechFont(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetSpeechFont);
}

// void  (int fontnum);
RuntimeScriptValue Sc_SetSpeechFont(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SetSpeechFont);
}

// int (int spriteNum)
RuntimeScriptValue Sc_Game_GetSpriteWidth(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(Game_GetSpriteWidth);
}

// int (int spriteNum)
RuntimeScriptValue Sc_Game_GetSpriteHeight(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(Game_GetSpriteHeight);
}

// int ()
RuntimeScriptValue Sc_Game_GetTextReadingSpeed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetTextReadingSpeed);
}

// void (int newTextSpeed)
RuntimeScriptValue Sc_Game_SetTextReadingSpeed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Game_SetTextReadingSpeed);
}

// const char* ()
RuntimeScriptValue Sc_Game_GetTranslationFilename(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ(const char, _GP(myScriptStringImpl), Game_GetTranslationFilename);
}

RuntimeScriptValue Sc_Game_GetSpeechVoxFilename(const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_SCALL_OBJ(const char, _GP(myScriptStringImpl), Game_GetSpeechVoxFilename);
}

// int ()
RuntimeScriptValue Sc_Game_GetUseNativeCoordinates(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetUseNativeCoordinates);
}

// int ()
RuntimeScriptValue Sc_Game_GetViewCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetViewCount);
}

RuntimeScriptValue Sc_Game_GetAudioClipCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(game).audioClips.size());
}

RuntimeScriptValue Sc_Game_GetAudioClip(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT(ScriptAudioClip, _GP(ccDynamicAudioClip), Game_GetAudioClip);
}

RuntimeScriptValue Sc_Game_IsPluginLoaded(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL_POBJ(pl_is_plugin_loaded, const char);
}

RuntimeScriptValue Sc_Game_PlayVoiceClip(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_POBJ_PINT_PBOOL(ScriptAudioChannel, _GP(ccDynamicAudio), PlayVoiceClip, CharacterInfo);
}

RuntimeScriptValue Sc_Game_GetCamera(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptCamera, Game_GetCamera);
}

RuntimeScriptValue Sc_Game_GetCameraCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_GetCameraCount);
}

RuntimeScriptValue Sc_Game_GetAnyCamera(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT(ScriptCamera, Game_GetAnyCamera);
}

RuntimeScriptValue Sc_Game_SimulateKeyPress(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Game_SimulateKeyPress);
}

RuntimeScriptValue Sc_Game_BlockingWaitSkipped(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Game_BlockingWaitSkipped);
}

void RegisterGameAPI() {
	ccAddExternalStaticFunction("Game::IsAudioPlaying^1",                       Sc_Game_IsAudioPlaying);
	ccAddExternalStaticFunction("Game::SetAudioTypeSpeechVolumeDrop^2",         Sc_Game_SetAudioTypeSpeechVolumeDrop);
	ccAddExternalStaticFunction("Game::SetAudioTypeVolume^3",                   Sc_Game_SetAudioTypeVolume);
	ccAddExternalStaticFunction("Game::StopAudio^1",                            Sc_Game_StopAudio);
	ccAddExternalStaticFunction("Game::ChangeTranslation^1",                    Sc_Game_ChangeTranslation);
	ccAddExternalStaticFunction("Game::DoOnceOnly^1",                           Sc_Game_DoOnceOnly);
	ccAddExternalStaticFunction("Game::GetColorFromRGB^3",                      Sc_Game_GetColorFromRGB);
	ccAddExternalStaticFunction("Game::GetFrameCountForLoop^2",                 Sc_Game_GetFrameCountForLoop);
	ccAddExternalStaticFunction("Game::GetLocationName^2",                      Sc_Game_GetLocationName);
	ccAddExternalStaticFunction("Game::GetLoopCountForView^1",                  Sc_Game_GetLoopCountForView);
	ccAddExternalStaticFunction("Game::GetMODPattern^0",                        Sc_Game_GetMODPattern);
	ccAddExternalStaticFunction("Game::GetRunNextSettingForLoop^2",             Sc_Game_GetRunNextSettingForLoop);
	ccAddExternalStaticFunction("Game::GetSaveSlotDescription^1",               Sc_Game_GetSaveSlotDescription);
	ccAddExternalStaticFunction("Game::GetViewFrame^3",                         Sc_Game_GetViewFrame);
	ccAddExternalStaticFunction("Game::InputBox^1",                             Sc_Game_InputBox);
	ccAddExternalStaticFunction("Game::SetSaveGameDirectory^1",                 Sc_Game_SetSaveGameDirectory);
	ccAddExternalStaticFunction("Game::StopSound^1",                            Sc_StopAllSounds);
	ccAddExternalStaticFunction("Game::get_CharacterCount",                     Sc_Game_GetCharacterCount);
	ccAddExternalStaticFunction("Game::get_DialogCount",                        Sc_Game_GetDialogCount);
	ccAddExternalStaticFunction("Game::get_FileName",                           Sc_Game_GetFileName);
	ccAddExternalStaticFunction("Game::get_FontCount",                          Sc_Game_GetFontCount);
	ccAddExternalStaticFunction("Game::geti_GlobalMessages",                    Sc_Game_GetGlobalMessages);
	ccAddExternalStaticFunction("Game::geti_GlobalStrings",                     Sc_Game_GetGlobalStrings);
	ccAddExternalStaticFunction("Game::seti_GlobalStrings",                     Sc_SetGlobalString);
	ccAddExternalStaticFunction("Game::get_GUICount",                           Sc_Game_GetGUICount);
	ccAddExternalStaticFunction("Game::get_IgnoreUserInputAfterTextTimeoutMs",  Sc_Game_GetIgnoreUserInputAfterTextTimeoutMs);
	ccAddExternalStaticFunction("Game::set_IgnoreUserInputAfterTextTimeoutMs",  Sc_Game_SetIgnoreUserInputAfterTextTimeoutMs);
	ccAddExternalStaticFunction("Game::get_InSkippableCutscene",                Sc_Game_GetInSkippableCutscene);
	ccAddExternalStaticFunction("Game::get_InventoryItemCount",                 Sc_Game_GetInventoryItemCount);
	ccAddExternalStaticFunction("Game::get_MinimumTextDisplayTimeMs",           Sc_Game_GetMinimumTextDisplayTimeMs);
	ccAddExternalStaticFunction("Game::set_MinimumTextDisplayTimeMs",           Sc_Game_SetMinimumTextDisplayTimeMs);
	ccAddExternalStaticFunction("Game::get_MouseCursorCount",                   Sc_Game_GetMouseCursorCount);
	ccAddExternalStaticFunction("Game::get_Name",                               Sc_Game_GetName);
	ccAddExternalStaticFunction("Game::set_Name",                               Sc_Game_SetName);
	ccAddExternalStaticFunction("Game::get_NormalFont",                         Sc_Game_GetNormalFont);
	ccAddExternalStaticFunction("Game::set_NormalFont",                         Sc_SetNormalFont);
	ccAddExternalStaticFunction("Game::get_SkippingCutscene",                   Sc_Game_GetSkippingCutscene);
	ccAddExternalStaticFunction("Game::get_SpeechFont",                         Sc_Game_GetSpeechFont);
	ccAddExternalStaticFunction("Game::set_SpeechFont",                         Sc_SetSpeechFont);
	ccAddExternalStaticFunction("Game::geti_SpriteWidth",                       Sc_Game_GetSpriteWidth);
	ccAddExternalStaticFunction("Game::geti_SpriteHeight",                      Sc_Game_GetSpriteHeight);
	ccAddExternalStaticFunction("Game::get_TextReadingSpeed",                   Sc_Game_GetTextReadingSpeed);
	ccAddExternalStaticFunction("Game::set_TextReadingSpeed",                   Sc_Game_SetTextReadingSpeed);
	ccAddExternalStaticFunction("Game::get_TranslationFilename",                Sc_Game_GetTranslationFilename);
	ccAddExternalStaticFunction("Game::get_UseNativeCoordinates",               Sc_Game_GetUseNativeCoordinates);
	ccAddExternalStaticFunction("Game::get_ViewCount",                          Sc_Game_GetViewCount);
	ccAddExternalStaticFunction("Game::get_AudioClipCount",                     Sc_Game_GetAudioClipCount);
	ccAddExternalStaticFunction("Game::geti_AudioClips",                        Sc_Game_GetAudioClip);
	ccAddExternalStaticFunction("Game::IsPluginLoaded",                         Sc_Game_IsPluginLoaded);
	ccAddExternalStaticFunction("Game::ChangeSpeechVox", Sc_Game_ChangeSpeechVox);
	ccAddExternalStaticFunction("Game::PlayVoiceClip",                          Sc_Game_PlayVoiceClip);
	ccAddExternalStaticFunction("Game::SimulateKeyPress",                       Sc_Game_SimulateKeyPress);
	ccAddExternalStaticFunction("Game::get_BlockingWaitSkipped",                Sc_Game_BlockingWaitSkipped);
	ccAddExternalStaticFunction("Game::get_SpeechVoxFilename", Sc_Game_GetSpeechVoxFilename);

	ccAddExternalStaticFunction("Game::get_Camera",                             Sc_Game_GetCamera);
	ccAddExternalStaticFunction("Game::get_CameraCount",                        Sc_Game_GetCameraCount);
	ccAddExternalStaticFunction("Game::geti_Cameras",                           Sc_Game_GetAnyCamera);
}

void RegisterStaticObjects() {
	ccAddExternalStaticObject("game", &_GP(play), &_GP(GameStaticManager));
	ccAddExternalStaticObject("gs_globals", &_GP(play).globalvars[0], &_GP(GlobalStaticManager));
	ccAddExternalStaticObject("mouse", &_GP(scmouse), &_GP(GlobalStaticManager));
	ccAddExternalStaticObject("palette", &_G(palette)[0], &_GP(GlobalStaticManager));
	ccAddExternalStaticObject("system", &_GP(scsystem), &_GP(GlobalStaticManager));
	ccAddExternalStaticObject("savegameindex", &_GP(play).filenumbers[0], &_GP(GlobalStaticManager));
}

} // namespace AGS3
