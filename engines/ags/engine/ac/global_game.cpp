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

#include "common/savefile.h"
#include "ags/lib/std/math.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/ac/audio_clip_type.h"
#include "ags/engine/ac/global_game.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_hotspot.h"
#include "ags/engine/ac/global_inventory_item.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/hotspot.h"
#include "ags/shared/ac/keycode.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/object.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/gui/gui_dialog.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/main/game_start.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/game_file.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string_utils.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

void GiveScore(int amnt) {
	GUI::MarkSpecialLabelsForUpdate(kLabelMacro_AllScore);
	_GP(play).score += amnt;

	if ((amnt > 0) && (_GP(play).score_sound >= 0))
		play_audio_clip_by_index(_GP(play).score_sound);

	run_on_event(GE_GOT_SCORE, RuntimeScriptValue().SetInt32(amnt));
}

void restart_game() {
	can_run_delayed_command();
	if (_G(inside_script)) {
		_G(curscript)->queue_action(ePSARestartGame, 0, "RestartGame");
		return;
	}
	try_restore_save(RESTART_POINT_SAVE_GAME_NUMBER);
}

void RestoreGameSlot(int slnum) {
	if (_G(displayed_room) < 0)
		quit("!RestoreGameSlot: a game cannot be restored from within game_start");

	can_run_delayed_command();
	if (_G(inside_script)) {
		_G(curscript)->queue_action(ePSARestoreGame, slnum, "RestoreGameSlot");
		return;
	}
	try_restore_save(slnum);
}

void DeleteSaveSlot(int slnum) {
	String nametouse;
	nametouse = get_save_game_path(slnum);
	Shared::File::DeleteFile(nametouse);
}

void PauseGame() {
	_G(game_paused)++;
	debug_script_log("Game paused");
}
void UnPauseGame() {
	if (_G(game_paused) > 0)
		_G(game_paused)--;
	debug_script_log("Game UnPaused, pause level now %d", _G(game_paused));
}


int IsGamePaused() {
	if (_G(game_paused) > 0) return 1;
	return 0;
}

bool GetSaveSlotDescription(int slnum, String &description) {
	if (read_savedgame_description(get_save_game_path(slnum), description))
		return true;
	description.Format("INVALID SLOT %d", slnum);
	return false;
}

int GetSaveSlotDescription(int slnum, char *desbuf) {
	VALIDATE_STRING(desbuf);
	String description;
	bool res = GetSaveSlotDescription(slnum, description);
	snprintf(desbuf, MAX_MAXSTRLEN, "%s", description.GetCStr());
	return res ? 1 : 0;
}

int LoadSaveSlotScreenshot(int slnum, int width, int height) {
	int gotSlot;
	data_to_game_coords(&width, &height);

	if (!read_savedgame_screenshot(get_save_game_path(slnum), gotSlot))
		return 0;

	if (gotSlot == 0)
		return 0;

	if ((_GP(game).SpriteInfos[gotSlot].Width == width) && (_GP(game).SpriteInfos[gotSlot].Height == height))
		return gotSlot;

	// resize the sprite to the requested size
	Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, _GP(spriteset)[gotSlot]->GetColorDepth());
	newPic->StretchBlt(_GP(spriteset)[gotSlot],
	                   RectWH(0, 0, _GP(game).SpriteInfos[gotSlot].Width, _GP(game).SpriteInfos[gotSlot].Height),
	                   RectWH(0, 0, width, height));

	update_polled_stuff_if_runtime();

	// replace the bitmap in the sprite set
	free_dynamic_sprite(gotSlot);
	add_dynamic_sprite(gotSlot, newPic);

	return gotSlot;
}

void FillSaveList(std::vector<SaveListItem> &saves, size_t max_count) {
	if (max_count == 0)
		return; // duh

	String svg_dir = get_save_game_directory();
	String svg_suff = get_save_game_suffix();
	String searchPath = Path::ConcatPaths(svg_dir, String::FromFormat("agssave.???%s", svg_suff.GetCStr()));
	time_t time = 0;

	SaveStateList saveList = ::AGS::g_vm->listSaves();
	for (uint idx = 0; idx < saveList.size(); ++idx) {
		int saveGameSlot = saveList[idx].getSaveSlot();

		// only list games .000 to .099 (to allow higher slots for other perposes)
		if (saveGameSlot > 99)
			continue;

		String description;
		GetSaveSlotDescription(saveGameSlot, description);
		saves.push_back(SaveListItem(saveGameSlot, description, time));
		if (saves.size() >= max_count)
			break;
	}
}

void SetGlobalInt(int index, int valu) {
	if ((index < 0) | (index >= MAXGSVALUES))
		quit("!SetGlobalInt: invalid index");

	if (_GP(play).globalscriptvars[index] != valu) {
		debug_script_log("GlobalInt %d set to %d", index, valu);
	}

	_GP(play).globalscriptvars[index] = valu;
}


int GetGlobalInt(int index) {
	if ((index < 0) | (index >= MAXGSVALUES))
		quit("!GetGlobalInt: invalid index");
	return _GP(play).globalscriptvars[index];
}

void SetGlobalString(int index, const char *newval) {
	if ((index < 0) | (index >= MAXGLOBALSTRINGS))
		quit("!SetGlobalString: invalid index");
	debug_script_log("GlobalString %d set to '%s'", index, newval);
	strncpy(_GP(play).globalstrings[index], newval, MAX_MAXSTRLEN);
	// truncate it to 200 chars, to be sure
	_GP(play).globalstrings[index][MAX_MAXSTRLEN - 1] = 0;
}

void GetGlobalString(int index, char *strval) {
	if ((index < 0) | (index >= MAXGLOBALSTRINGS))
		quit("!GetGlobalString: invalid index");
	strcpy(strval, _GP(play).globalstrings[index]);
}

// TODO: refactor this method, and use same shared procedure at both normal stop/startup and in RunAGSGame
int RunAGSGame(const String &newgame, unsigned int mode, int data) {

	can_run_delayed_command();

	int AllowedModes = RAGMODE_PRESERVEGLOBALINT | RAGMODE_LOADNOW;

	if ((mode & (~AllowedModes)) != 0)
		quit("!RunAGSGame: mode value unknown");

	if (_G(editor_debugging_enabled)) {
		quit("!RunAGSGame cannot be used while running the game from within the AGS Editor. You must build the game EXE and run it from there to use this function.");
	}

	if ((mode & RAGMODE_LOADNOW) == 0) {
		_GP(ResPaths).GamePak.Path = PathFromInstallDir(newgame);
		_GP(ResPaths).GamePak.Name = newgame;
		_GP(play).takeover_data = data;
		_G(load_new_game_restore) = -1;

		if (_G(inside_script)) {
			_G(curscript)->queue_action(ePSARunAGSGame, mode | RAGMODE_LOADNOW, "RunAGSGame");
			ccInstance::GetCurrentInstance()->Abort();
		} else
			_G(load_new_game) = mode | RAGMODE_LOADNOW;

		return 0;
	}

	int ee;

	unload_old_room();
	_G(displayed_room) = -10;

#if defined (AGS_AUTO_WRITE_USER_CONFIG)
	save_config_file(); // save current user config in case engine fails to run new game
#endif // AGS_AUTO_WRITE_USER_CONFIG
	unload_game_file();

	// Adjust config (NOTE: normally, RunAGSGame would need a redesign to allow separate config etc per each game)
	_GP(usetup).translation = ""; // reset to default, prevent from trying translation file of game A in game B

	_GP(AssetMgr)->RemoveAllLibraries();

	// TODO: refactor and share same code with the startup!
	if (_GP(AssetMgr)->AddLibrary(_GP(ResPaths).GamePak.Path) != Shared::kAssetNoError)
		quitprintf("!RunAGSGame: unable to load new game file '%s'", _GP(ResPaths).GamePak.Path.GetCStr());
	engine_assign_assetpaths();

	show_preload();

	HError err = load_game_file();
	if (!err)
		quitprintf("!RunAGSGame: error loading new game file:\n%s", err->FullMessage().GetCStr());

	_GP(spriteset).Reset();
	err = _GP(spriteset).InitFile(SpriteFile::DefaultSpriteFileName, SpriteFile::DefaultSpriteIndexName);
	if (!err)
		quitprintf("!RunAGSGame: error loading new sprites:\n%s", err->FullMessage().GetCStr());

	if ((mode & RAGMODE_PRESERVEGLOBALINT) == 0) {
		// reset GlobalInts
		for (ee = 0; ee < MAXGSVALUES; ee++)
			_GP(play).globalscriptvars[ee] = 0;
	}

	engine_init_game_settings();
	_GP(play).screen_is_faded_out = 1;

	if (_G(load_new_game_restore) >= 0) {
		try_restore_save(_G(load_new_game_restore));
		_G(load_new_game_restore) = -1;
	} else
		start_game();

	return 0;
}

int GetGameParameter(int parm, int data1, int data2, int data3) {
	switch (parm) {
	case GP_SPRITEWIDTH:
		return Game_GetSpriteWidth(data1);
	case GP_SPRITEHEIGHT:
		return Game_GetSpriteHeight(data1);
	case GP_NUMLOOPS:
		return Game_GetLoopCountForView(data1);
	case GP_NUMFRAMES:
		return Game_GetFrameCountForLoop(data1, data2);
	case GP_FRAMESPEED:
	case GP_FRAMEIMAGE:
	case GP_FRAMESOUND:
	case GP_ISFRAMEFLIPPED: {
		if ((data1 < 1) || (data1 > _GP(game).numviews)) {
			quitprintf("!GetGameParameter: invalid view specified (v: %d, l: %d, f: %d)", data1, data2, data3);
		}
		if ((data2 < 0) || (data2 >= _G(views)[data1 - 1].numLoops)) {
			quitprintf("!GetGameParameter: invalid loop specified (v: %d, l: %d, f: %d)", data1, data2, data3);
		}
		if ((data3 < 0) || (data3 >= _G(views)[data1 - 1].loops[data2].numFrames)) {
			quitprintf("!GetGameParameter: invalid frame specified (v: %d, l: %d, f: %d)", data1, data2, data3);
		}

		ViewFrame *pvf = &_G(views)[data1 - 1].loops[data2].frames[data3];

		if (parm == GP_FRAMESPEED)
			return pvf->speed;
		else if (parm == GP_FRAMEIMAGE)
			return pvf->pic;
		else if (parm == GP_FRAMESOUND)
			return get_old_style_number_for_sound(pvf->sound);
		else if (parm == GP_ISFRAMEFLIPPED)
			return (pvf->flags & VFLG_FLIPSPRITE) ? 1 : 0;
		else
			quit("GetGameParameter internal error");
		break;
	}
	case GP_ISRUNNEXTLOOP:
		return Game_GetRunNextSettingForLoop(data1, data2);
	case GP_NUMGUIS:
		return _GP(game).numgui;
	case GP_NUMOBJECTS:
		return _G(croom)->numobj;
	case GP_NUMCHARACTERS:
		return _GP(game).numcharacters;
	case GP_NUMINVITEMS:
		return _GP(game).numinvitems;
	default:
		quit("!GetGameParameter: unknown parameter specified");
	}
	return 0;
}

void QuitGame(int dialog) {
	if (dialog) {
		int rcode;
		setup_for_dialog();
		rcode = quitdialog();
		restore_after_dialog();
		if (rcode == 0) return;
	}
	quit("|You have exited.");
}




void SetRestartPoint() {
	save_game(RESTART_POINT_SAVE_GAME_NUMBER, "Restart Game Auto-Save");
}



void SetGameSpeed(int newspd) {
	newspd += _GP(play).game_speed_modifier;
	if (newspd > 1000) newspd = 1000;
	if (newspd < 10) newspd = 10;
	set_game_speed(newspd);
	debug_script_log("Game speed set to %d", newspd);
}

int GetGameSpeed() {
	return ::lround(get_current_fps()) - _GP(play).game_speed_modifier;
}

int SetGameOption(int opt, int setting) {
	if (((opt < 1) || (opt > OPT_HIGHESTOPTION)) && (opt != OPT_LIPSYNCTEXT))
		quit("!SetGameOption: invalid option specified");

	if (opt == OPT_ANTIGLIDE) {
		for (int i = 0; i < _GP(game).numcharacters; i++) {
			if (setting)
				_GP(game).chars[i].flags |= CHF_ANTIGLIDE;
			else
				_GP(game).chars[i].flags &= ~CHF_ANTIGLIDE;
		}
	}

	if ((opt == OPT_CROSSFADEMUSIC) && (_GP(game).audioClipTypes.size() > AUDIOTYPE_LEGACY_MUSIC)) {
		// legacy compatibility -- changing crossfade speed here also
		// updates the new audio clip type style
		_GP(game).audioClipTypes[AUDIOTYPE_LEGACY_MUSIC].crossfadeSpeed = setting;
	}

	int oldval = _GP(game).options[opt];
	_GP(game).options[opt] = setting;

	if (opt == OPT_DUPLICATEINV)
		update_invorder();
	else if (opt == OPT_DISABLEOFF) {
		_G(gui_disabled_style) = convert_gui_disabled_style(_GP(game).options[OPT_DISABLEOFF]);
		// If GUI was disabled at this time then also update it, as visual style could've changed
		if (_GP(play).disabled_user_interface > 0) {
			GUI::MarkAllGUIForUpdate();
		}
	} else if (opt == OPT_PORTRAITSIDE) {
		if (setting == 0)  // set back to Left
			_GP(play).swap_portrait_side = 0;
	}

	return oldval;
}

int GetGameOption(int opt) {
	if (((opt < 1) || (opt > OPT_HIGHESTOPTION)) && (opt != OPT_LIPSYNCTEXT))
		quit("!GetGameOption: invalid option specified");

	return _GP(game).options[opt];
}

void SkipUntilCharacterStops(int cc) {
	if (!is_valid_character(cc))
		quit("!SkipUntilCharacterStops: invalid character specified");
	if (_GP(game).chars[cc].room != _G(displayed_room))
		quit("!SkipUntilCharacterStops: specified character not in current room");

	// if they are not currently moving, do nothing
	if (!_GP(game).chars[cc].walking)
		return;

	if (is_in_cutscene())
		quit("!SkipUntilCharacterStops: cannot be used within a cutscene");

	initialize_skippable_cutscene();
	_GP(play).fast_forward = 2;
	_GP(play).skip_until_char_stops = cc;
}

void EndSkippingUntilCharStops() {
	// not currently skipping, so ignore
	if (_GP(play).skip_until_char_stops < 0)
		return;

	stop_fast_forwarding();
	_GP(play).skip_until_char_stops = -1;
}

void StartCutscene(int skipwith) {
	static ScriptPosition last_cutscene_script_pos;

	if (is_in_cutscene()) {
		quitprintf("!StartCutscene: already in a cutscene; previous started in \"%s\", line %d",
		           last_cutscene_script_pos.Section.GetCStr(), last_cutscene_script_pos.Line);
	}

	if ((skipwith < 1) || (skipwith > 6))
		quit("!StartCutscene: invalid argument, must be 1 to 5.");

	get_script_position(last_cutscene_script_pos);

	// make sure they can't be skipping and cutsceneing at the same time
	EndSkippingUntilCharStops();

	_GP(play).in_cutscene = skipwith;
	initialize_skippable_cutscene();
}

void SkipCutscene() {
	if (is_in_cutscene())
		start_skipping_cutscene();
}

int EndCutscene() {
	if (!is_in_cutscene())
		quit("!EndCutscene: not in a cutscene");

	int retval = _GP(play).fast_forward;
	_GP(play).in_cutscene = 0;
	// Stop it fast-forwarding
	stop_fast_forwarding();

	// make sure that the screen redraws
	invalidate_screen();

	// Return whether the player skipped it
	return retval;
}

void sc_inputbox(const char *msg, char *bufr) {
	VALIDATE_STRING(bufr);
	setup_for_dialog();
	enterstringwindow(get_translation(msg), bufr);
	restore_after_dialog();
}

// GetLocationType exported function - just call through
// to the main function with default 0
int GetLocationType(int xxx, int yyy) {
	return __GetLocationType(xxx, yyy, 0);
}

void SaveCursorForLocationChange() {
	// update the current location name
	char tempo[100];
	GetLocationName(game_to_data_coord(_G(mousex)), game_to_data_coord(_G(mousey)), tempo);

	if (_GP(play).get_loc_name_save_cursor != _GP(play).get_loc_name_last_time) {
		_GP(play).get_loc_name_save_cursor = _GP(play).get_loc_name_last_time;
		_GP(play).restore_cursor_mode_to = GetCursorMode();
		_GP(play).restore_cursor_image_to = GetMouseCursor();
		debug_script_log("Saving mouse: mode %d cursor %d", _GP(play).restore_cursor_mode_to, _GP(play).restore_cursor_image_to);
	}
}

void GetLocationName(int xxx, int yyy, char *tempo) {
	if (_G(displayed_room) < 0)
		quit("!GetLocationName: no room has been loaded");

	VALIDATE_STRING(tempo);

	tempo[0] = 0;

	if (GetGUIAt(xxx, yyy) >= 0) {
		int mover = GetInvAt(xxx, yyy);
		if (mover > 0) {
			if (_GP(play).get_loc_name_last_time != 1000 + mover)
				GUI::MarkSpecialLabelsForUpdate(kLabelMacro_Overhotspot);
			_GP(play).get_loc_name_last_time = 1000 + mover;
			strcpy(tempo, get_translation(_GP(game).invinfo[mover].name));
		} else if ((_GP(play).get_loc_name_last_time > 1000) && (_GP(play).get_loc_name_last_time < 1000 + MAX_INV)) {
			// no longer selecting an item
			GUI::MarkSpecialLabelsForUpdate(kLabelMacro_Overhotspot);
			_GP(play).get_loc_name_last_time = -1;
		}
		return;
	}

	int loctype = GetLocationType(xxx, yyy); // GetLocationType takes screen coords
	VpPoint vpt = _GP(play).ScreenToRoomDivDown(xxx, yyy);
	if (vpt.second < 0)
		return;
	xxx = vpt.first.X;
	yyy = vpt.first.Y;
	if ((xxx >= _GP(thisroom).Width) | (xxx < 0) | (yyy < 0) | (yyy >= _GP(thisroom).Height))
		return;

	int onhs, aa;
	if (loctype == 0) {
		if (_GP(play).get_loc_name_last_time != 0) {
			_GP(play).get_loc_name_last_time = 0;
			GUI::MarkSpecialLabelsForUpdate(kLabelMacro_Overhotspot);
		}
		return;
	}

	// on character
	if (loctype == LOCTYPE_CHAR) {
		onhs = _G(getloctype_index);
		strcpy(tempo, get_translation(_GP(game).chars[onhs].name));
		if (_GP(play).get_loc_name_last_time != 2000 + onhs)
			GUI::MarkSpecialLabelsForUpdate(kLabelMacro_Overhotspot);
		_GP(play).get_loc_name_last_time = 2000 + onhs;
		return;
	}
	// on object
	if (loctype == LOCTYPE_OBJ) {
		aa = _G(getloctype_index);
		strcpy(tempo, get_translation(_GP(thisroom).Objects[aa].Name.GetCStr()));
		// Compatibility: < 3.1.1 games returned space for nameless object
		// (presumably was a bug, but fixing it affected certain games behavior)
		if (_G(loaded_game_file_version) < kGameVersion_311 && tempo[0] == 0) {
			tempo[0] = ' ';
			tempo[1] = 0;
		}
		if (_GP(play).get_loc_name_last_time != 3000 + aa)
			GUI::MarkSpecialLabelsForUpdate(kLabelMacro_Overhotspot);
		_GP(play).get_loc_name_last_time = 3000 + aa;
		return;
	}
	onhs = _G(getloctype_index);
	if (onhs > 0) strcpy(tempo, get_translation(_GP(thisroom).Hotspots[onhs].Name.GetCStr()));
	if (_GP(play).get_loc_name_last_time != onhs)
		GUI::MarkSpecialLabelsForUpdate(kLabelMacro_Overhotspot);
	_GP(play).get_loc_name_last_time = onhs;
}

int IsKeyPressed(int keycode) {
	auto status = ags_iskeydown(static_cast<eAGSKeyCode>(keycode));
	if (status < 0) {
		debug_script_log("IsKeyPressed: unsupported keycode %d", keycode);
		return 0;
	}
	return status;
}

int SaveScreenShot(const char *namm) {
	String fileName;
	String svg_dir = get_save_game_directory();

	if (strchr(namm, '.') == nullptr)
		fileName = Path::MakePath(svg_dir, namm, "bmp");
	else
		fileName = Path::ConcatPaths(svg_dir, namm);

	Bitmap *buffer = CopyScreenIntoBitmap(_GP(play).GetMainViewport().GetWidth(), _GP(play).GetMainViewport().GetHeight());
	if (!buffer->SaveToFile(fileName, _G(palette)) != 0) {
		delete buffer;
		return 0;
	}
	delete buffer;
	return 1;  // successful
}

void SetMultitasking(int mode) {
	if ((mode < 0) | (mode > 1))
		quit("!SetMultitasking: invalid mode parameter");

	if (_GP(usetup).override_multitasking >= 0) {
		mode = _GP(usetup).override_multitasking;
	}

	// Don't allow background running if full screen
	if ((mode == 1) && (!_GP(scsystem).windowed))
		mode = 0;

	// Install engine callbacks for switching in and out the window
	if (mode == 0) {
		sys_set_background_mode(false);
		sys_evt_set_focus_callbacks(display_switch_in_resume, display_switch_out_suspend);
	} else {
		sys_set_background_mode(true);
		sys_evt_set_focus_callbacks(display_switch_in, display_switch_out);
	}
}

void RoomProcessClick(int xx, int yy, int mood) {
	_G(getloctype_throughgui) = 1;
	int loctype = GetLocationType(xx, yy);
	VpPoint vpt = _GP(play).ScreenToRoomDivDown(xx, yy);
	if (vpt.second < 0)
		return;
	xx = vpt.first.X;
	yy = vpt.first.Y;

	if ((mood == MODE_WALK) && (_GP(game).options[OPT_NOWALKMODE] == 0)) {
		int hsnum = get_hotspot_at(xx, yy);
		if (hsnum < 1);
		else if (_GP(thisroom).Hotspots[hsnum].WalkTo.X < 1);
		else if (_GP(play).auto_use_walkto_points == 0);
		else {
			xx = _GP(thisroom).Hotspots[hsnum].WalkTo.X;
			yy = _GP(thisroom).Hotspots[hsnum].WalkTo.Y;
			debug_script_log("Move to walk-to point hotspot %d", hsnum);
		}
		walk_character(_GP(game).playercharacter, xx, yy, 0, true);
		return;
	}
	_GP(play).usedmode = mood;

	if (loctype == 0) {
		// click on nothing -> hotspot 0
		_G(getloctype_index) = 0;
		loctype = LOCTYPE_HOTSPOT;
	}

	if (loctype == LOCTYPE_CHAR) {
		if (check_click_on_character(xx, yy, mood)) return;
	} else if (loctype == LOCTYPE_OBJ) {
		if (check_click_on_object(xx, yy, mood)) return;
	} else if (loctype == LOCTYPE_HOTSPOT)
		RunHotspotInteraction(_G(getloctype_index), mood);
}

int IsInteractionAvailable(int xx, int yy, int mood) {
	_G(getloctype_throughgui) = 1;
	int loctype = GetLocationType(xx, yy);
	VpPoint vpt = _GP(play).ScreenToRoomDivDown(xx, yy);
	if (vpt.second < 0)
		return 0;
	xx = vpt.first.X;
	yy = vpt.first.Y;

	// You can always walk places
	if ((mood == MODE_WALK) && (_GP(game).options[OPT_NOWALKMODE] == 0))
		return 1;

	_GP(play).check_interaction_only = 1;

	if (loctype == 0) {
		// click on nothing -> hotspot 0
		_G(getloctype_index) = 0;
		loctype = LOCTYPE_HOTSPOT;
	}

	if (loctype == LOCTYPE_CHAR) {
		check_click_on_character(xx, yy, mood);
	} else if (loctype == LOCTYPE_OBJ) {
		check_click_on_object(xx, yy, mood);
	} else if (loctype == LOCTYPE_HOTSPOT)
		RunHotspotInteraction(_G(getloctype_index), mood);

	int ciwas = _GP(play).check_interaction_only;
	_GP(play).check_interaction_only = 0;

	if (ciwas == 2)
		return 1;

	return 0;
}

void GetMessageText(int msg, char *buffer) {
	VALIDATE_STRING(buffer);
	get_message_text(msg, buffer, 0);
}

void SetSpeechFont(int fontnum) {
	if ((fontnum < 0) || (fontnum >= _GP(game).numfonts))
		quit("!SetSpeechFont: invalid font number.");
	_GP(play).speech_font = fontnum;
}

void SetNormalFont(int fontnum) {
	if ((fontnum < 0) || (fontnum >= _GP(game).numfonts))
		quit("!SetNormalFont: invalid font number.");
	_GP(play).normal_font = fontnum;
}

void _sc_AbortGame(const char *text) {
	char displbuf[STD_BUFFER_SIZE] = "!?";
	snprintf(&displbuf[2], STD_BUFFER_SIZE - 3, "%s", text);
	quit(displbuf);
}

int GetGraphicalVariable(const char *varName) {
	InteractionVariable *theVar = FindGraphicalVariable(varName);
	if (theVar == nullptr) {
		quitprintf("!GetGraphicalVariable: interaction variable '%s' not found", varName);
		return 0;
	}
	return theVar->Value;
}

void SetGraphicalVariable(const char *varName, int p_value) {
	InteractionVariable *theVar = FindGraphicalVariable(varName);
	if (theVar == nullptr) {
		quitprintf("!SetGraphicalVariable: interaction variable '%s' not found", varName);
	} else
		theVar->Value = p_value;
}

int WaitImpl(int skip_type, int nloops) {
	_GP(play).wait_counter = nloops;
	_GP(play).wait_skipped_by = SKIP_NONE;
	_GP(play).wait_skipped_by = SKIP_AUTOTIMER; // we set timer flag by default to simplify that case
	_GP(play).wait_skipped_by_data = 0;
	_GP(play).key_skip_wait = skip_type;

    GameLoopUntilValueIsZero(&_GP(play).wait_counter);

	if (_GP(game).options[OPT_BASESCRIPTAPI] < kScriptAPI_v360) {
		// < 3.6.0 return 1 is skipped by user input, otherwise 0
		return (_GP(play).wait_skipped_by & (SKIP_KEYPRESS | SKIP_MOUSECLICK)) != 0 ? 1 : 0;
	}
	// >= 3.6.0 return positive keycode, negative mouse button code, or 0 as time-out
	return _GP(play).GetWaitSkipResult();
}

void scrWait(int nloops) {
	WaitImpl(SKIP_AUTOTIMER, nloops);
}

int WaitKey(int nloops) {
	return WaitImpl(SKIP_KEYPRESS | SKIP_AUTOTIMER, nloops);
}

int WaitMouse(int nloops) {
	return WaitImpl(SKIP_MOUSECLICK | SKIP_AUTOTIMER, nloops);
}

int WaitMouseKey(int nloops) {
	return WaitImpl(SKIP_KEYPRESS | SKIP_MOUSECLICK | SKIP_AUTOTIMER, nloops);
}

void SkipWait() {
	_GP(play).wait_counter = 0;
}

} // namespace AGS3
