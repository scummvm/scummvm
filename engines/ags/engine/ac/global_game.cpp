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

#include "ags/shared/core/platform.h"
#include "ags/shared/ac/audiocliptype.h"
#include "ags/shared/util/path.h"
#include "ags/engine/ac/global_game.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/view.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynamicsprite.h"
#include "ags/engine/ac/event.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_hotspot.h"
#include "ags/engine/ac/global_inventoryitem.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/hotspot.h"
#include "ags/engine/ac/keycode.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/object.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/roomstatus.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/gui/guidialog.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/game_start.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/script/script.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/shared/core/assetmanager.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/game_file.h"
#include "ags/shared/util/string_utils.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

#define ALLEGRO_KEYBOARD_HANDLER

void GiveScore(int amnt) {
	_G(guis_need_update) = 1;
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
	// The code below renames the highest save game to fill in the gap
	// This does not work (because the system rename() function does not
	// handle our "/saves/" prefix). We could remove it here and use
	// g_system->getSavefileManager()->renameSavefile. But it might
	// actually be better to not fill the gap. The original AGS engine
	// sorts savegame by date, but as we don't have access to the date,
	// we save them by slot. So moving the highest slot to the gap may
	// not be a good idea. An alternative would be to shift by one all
	// the savegames after the removed one.
	// One aspect to keep in mind is that MAXSAVEGAMES is 50, but we
	// allow slot up to 099. So we have some margin.
#ifndef AGS_PLATFORM_SCUMMVM
	if ((slnum >= 1) && (slnum <= MAXSAVEGAMES)) {
		String thisname;
		for (int i = MAXSAVEGAMES; i > slnum; i--) {
			thisname = get_save_game_path(i);
			if (Shared::File::TestReadFile(thisname)) {
				// Rename the highest save game to fill in the gap
				rename(thisname, nametouse);
				break;
			}
		}

	}
#endif
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

int GetSaveSlotDescription(int slnum, char *desbuf) {
	VALIDATE_STRING(desbuf);
	String description;
	if (read_savedgame_description(get_save_game_path(slnum), description)) {
		strcpy(desbuf, description);
		return 1;
	}
	sprintf(desbuf, "INVALID SLOT %d", slnum);
	return 0;
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
int RunAGSGame(const char *newgame, unsigned int mode, int data) {

	can_run_delayed_command();

	int AllowedModes = RAGMODE_PRESERVEGLOBALINT | RAGMODE_LOADNOW;

	if ((mode & (~AllowedModes)) != 0)
		quit("!RunAGSGame: mode value unknown");

	if (_G(editor_debugging_enabled)) {
		quit("!RunAGSGame cannot be used while running the game from within the AGS Editor. You must build the game EXE and run it from there to use this function.");
	}

	if ((mode & RAGMODE_LOADNOW) == 0) {
		// need to copy, since the script gets destroyed
		get_install_dir_path(_G(gamefilenamebuf), newgame);
		_GP(ResPaths).GamePak.Path = _G(gamefilenamebuf);
		_GP(ResPaths).GamePak.Name = Shared::Path::get_filename(_G(gamefilenamebuf));
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

	save_config_file(); // save current user config in case engine fails to run new game
	unload_game_file();

	// Adjust config (NOTE: normally, RunAGSGame would need a redesign to allow separate config etc per each game)
	_GP(usetup).translation = ""; // reset to default, prevent from trying translation file of game A in game B

	if (Shared::AssetManager::SetDataFile(_GP(ResPaths).GamePak.Path) != Shared::kAssetNoError)
		quitprintf("!RunAGSGame: unable to load new game file '%s'", _GP(ResPaths).GamePak.Path.GetCStr());

	show_preload();

	HError err = load_game_file();
	if (!err)
		quitprintf("!RunAGSGame: error loading new game file:\n%s", err->FullMessage().GetCStr());

	_GP(spriteset).Reset();
	err = _GP(spriteset).InitFile(SpriteCache::DefaultSpriteFileName, SpriteCache::DefaultSpriteIndexName);
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
	case GP_ISFRAMEFLIPPED:
	{
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
		return 0;
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
	else if (opt == OPT_DISABLEOFF)
		_G(gui_disabled_style) = convert_gui_disabled_style(_GP(game).options[OPT_DISABLEOFF]);
	else if (opt == OPT_PORTRAITSIDE) {
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
				_G(guis_need_update) = 1;
			_GP(play).get_loc_name_last_time = 1000 + mover;
			strcpy(tempo, get_translation(_GP(game).invinfo[mover].name));
		} else if ((_GP(play).get_loc_name_last_time > 1000) && (_GP(play).get_loc_name_last_time < 1000 + MAX_INV)) {
			// no longer selecting an item
			_G(guis_need_update) = 1;
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
			_G(guis_need_update) = 1;
		}
		return;
	}

	// on character
	if (loctype == LOCTYPE_CHAR) {
		onhs = _G(getloctype_index);
		strcpy(tempo, get_translation(_GP(game).chars[onhs].name));
		if (_GP(play).get_loc_name_last_time != 2000 + onhs)
			_G(guis_need_update) = 1;
		_GP(play).get_loc_name_last_time = 2000 + onhs;
		return;
	}
	// on object
	if (loctype == LOCTYPE_OBJ) {
		aa = _G(getloctype_index);
		strcpy(tempo, get_translation(_GP(thisroom).Objects[aa].Name));
		// Compatibility: < 3.1.1 games returned space for nameless object
		// (presumably was a bug, but fixing it affected certain games behavior)
		if (_G(loaded_game_file_version) < kGameVersion_311 && tempo[0] == 0) {
			tempo[0] = ' ';
			tempo[1] = 0;
		}
		if (_GP(play).get_loc_name_last_time != 3000 + aa)
			_G(guis_need_update) = 1;
		_GP(play).get_loc_name_last_time = 3000 + aa;
		return;
	}
	onhs = _G(getloctype_index);
	if (onhs > 0)
		strcpy(tempo, get_translation(_GP(thisroom).Hotspots[onhs].Name));
	if (_GP(play).get_loc_name_last_time != onhs)
		_G(guis_need_update) = 1;
	_GP(play).get_loc_name_last_time = onhs;
}

int IsKeyPressed(int keycode) {
	if (keyboard_needs_poll())
		poll_keyboard();

	switch (keycode) {
	case eAGSKeyCodeBackspace:
		return ags_iskeypressed(__allegro_KEY_BACKSPACE);
	case eAGSKeyCodeTab:
		return ags_iskeypressed(__allegro_KEY_TAB);
	case eAGSKeyCodeReturn:
		return ags_iskeypressed(__allegro_KEY_ENTER) || ags_iskeypressed(__allegro_KEY_ENTER_PAD);
	case eAGSKeyCodeEscape:
		return ags_iskeypressed(__allegro_KEY_ESC);
	case eAGSKeyCodeSpace:
		return ags_iskeypressed(__allegro_KEY_SPACE);
	case eAGSKeyCodeSingleQuote:
		return ags_iskeypressed(__allegro_KEY_QUOTE);
	case eAGSKeyCodeComma:
		return ags_iskeypressed(__allegro_KEY_COMMA);
	case eAGSKeyCodePeriod:
		return ags_iskeypressed(__allegro_KEY_STOP);
	case eAGSKeyCodeForwardSlash:
		return ags_iskeypressed(__allegro_KEY_SLASH) || ags_iskeypressed(__allegro_KEY_SLASH_PAD);
	case eAGSKeyCodeBackSlash:
		return ags_iskeypressed(__allegro_KEY_BACKSLASH) || ags_iskeypressed(__allegro_KEY_BACKSLASH2);
	case eAGSKeyCodeSemiColon:
		return ags_iskeypressed(__allegro_KEY_SEMICOLON);
	case eAGSKeyCodeEquals:
		return ags_iskeypressed(__allegro_KEY_EQUALS) || ags_iskeypressed(__allegro_KEY_EQUALS_PAD);
	case eAGSKeyCodeOpenBracket:
		return ags_iskeypressed(__allegro_KEY_OPENBRACE);
	case eAGSKeyCodeCloseBracket:
		return ags_iskeypressed(__allegro_KEY_CLOSEBRACE);
	// NOTE: we're treating EQUALS like PLUS, even though it is only available shifted.
	case eAGSKeyCodePlus:
		return ags_iskeypressed(__allegro_KEY_EQUALS) || ags_iskeypressed(__allegro_KEY_PLUS_PAD);
	case eAGSKeyCodeHyphen:
		return ags_iskeypressed(__allegro_KEY_MINUS) || ags_iskeypressed(__allegro_KEY_MINUS_PAD);

	// non-shifted versions of keys
	case eAGSKeyCodeColon:
		return ags_iskeypressed(__allegro_KEY_COLON) || ags_iskeypressed(__allegro_KEY_COLON2);
	case eAGSKeyCodeAsterisk:
		return ags_iskeypressed(__allegro_KEY_ASTERISK);
	case eAGSKeyCodeAt:
		return ags_iskeypressed(__allegro_KEY_AT);

	case eAGSKeyCode0:
		return ags_iskeypressed(__allegro_KEY_0);
	case eAGSKeyCode1:
		return ags_iskeypressed(__allegro_KEY_1);
	case eAGSKeyCode2:
		return ags_iskeypressed(__allegro_KEY_2);
	case eAGSKeyCode3:
		return ags_iskeypressed(__allegro_KEY_3);
	case eAGSKeyCode4:
		return ags_iskeypressed(__allegro_KEY_4);
	case eAGSKeyCode5:
		return ags_iskeypressed(__allegro_KEY_5);
	case eAGSKeyCode6:
		return ags_iskeypressed(__allegro_KEY_6);
	case eAGSKeyCode7:
		return ags_iskeypressed(__allegro_KEY_7);
	case eAGSKeyCode8:
		return ags_iskeypressed(__allegro_KEY_8);
	case eAGSKeyCode9:
		return ags_iskeypressed(__allegro_KEY_9);

	case eAGSKeyCodeA:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('A'));
	case eAGSKeyCodeB:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('B'));
	case eAGSKeyCodeC:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('C'));
	case eAGSKeyCodeD:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('D'));
	case eAGSKeyCodeE:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('E'));
	case eAGSKeyCodeF:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('F'));
	case eAGSKeyCodeG:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('G'));
	case eAGSKeyCodeH:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('H'));
	case eAGSKeyCodeI:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('I'));
	case eAGSKeyCodeJ:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('J'));
	case eAGSKeyCodeK:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('K'));
	case eAGSKeyCodeL:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('L'));
	case eAGSKeyCodeM:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('M'));
	case eAGSKeyCodeN:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('N'));
	case eAGSKeyCodeO:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('O'));
	case eAGSKeyCodeP:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('P'));
	case eAGSKeyCodeQ:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('Q'));
	case eAGSKeyCodeR:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('R'));
	case eAGSKeyCodeS:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('S'));
	case eAGSKeyCodeT:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('T'));
	case eAGSKeyCodeU:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('U'));
	case eAGSKeyCodeV:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('V'));
	case eAGSKeyCodeW:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('W'));
	case eAGSKeyCodeX:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('X'));
	case eAGSKeyCodeY:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('Y'));
	case eAGSKeyCodeZ:
		return ags_iskeypressed(_G(platform)->ConvertKeycodeToScanCode('Z'));

	case eAGSKeyCodeF1:
		return ags_iskeypressed(__allegro_KEY_F1);
	case eAGSKeyCodeF2:
		return ags_iskeypressed(__allegro_KEY_F2);
	case eAGSKeyCodeF3:
		return ags_iskeypressed(__allegro_KEY_F3);
	case eAGSKeyCodeF4:
		return ags_iskeypressed(__allegro_KEY_F4);
	case eAGSKeyCodeF5:
		return ags_iskeypressed(__allegro_KEY_F5);
	case eAGSKeyCodeF6:
		return ags_iskeypressed(__allegro_KEY_F6);
	case eAGSKeyCodeF7:
		return ags_iskeypressed(__allegro_KEY_F7);
	case eAGSKeyCodeF8:
		return ags_iskeypressed(__allegro_KEY_F8);
	case eAGSKeyCodeF9:
		return ags_iskeypressed(__allegro_KEY_F9);
	case eAGSKeyCodeF10:
		return ags_iskeypressed(__allegro_KEY_F10);
	case eAGSKeyCodeF11:
		return ags_iskeypressed(__allegro_KEY_F11);
	case eAGSKeyCodeF12:
		return ags_iskeypressed(__allegro_KEY_F12);

	case eAGSKeyCodeHome:
		return ags_iskeypressed(__allegro_KEY_HOME) || ags_iskeypressed(__allegro_KEY_7_PAD);
	case eAGSKeyCodeUpArrow:
		return ags_iskeypressed(__allegro_KEY_UP) || ags_iskeypressed(__allegro_KEY_8_PAD);
	case eAGSKeyCodePageUp:
		return ags_iskeypressed(__allegro_KEY_PGUP) || ags_iskeypressed(__allegro_KEY_9_PAD);
	case eAGSKeyCodeLeftArrow:
		return ags_iskeypressed(__allegro_KEY_LEFT) || ags_iskeypressed(__allegro_KEY_4_PAD);
	case eAGSKeyCodeNumPad5:
		return ags_iskeypressed(__allegro_KEY_5_PAD);
	case eAGSKeyCodeRightArrow:
		return ags_iskeypressed(__allegro_KEY_RIGHT) || ags_iskeypressed(__allegro_KEY_6_PAD);
	case eAGSKeyCodeEnd:
		return ags_iskeypressed(__allegro_KEY_END) || ags_iskeypressed(__allegro_KEY_1_PAD);
	case eAGSKeyCodeDownArrow:
		return ags_iskeypressed(__allegro_KEY_DOWN) || ags_iskeypressed(__allegro_KEY_2_PAD);
	case eAGSKeyCodePageDown:
		return ags_iskeypressed(__allegro_KEY_PGDN) || ags_iskeypressed(__allegro_KEY_3_PAD);
	case eAGSKeyCodeInsert:
		return ags_iskeypressed(__allegro_KEY_INSERT) || ags_iskeypressed(__allegro_KEY_0_PAD);
	case eAGSKeyCodeDelete:
		return ags_iskeypressed(__allegro_KEY_DEL) || ags_iskeypressed(__allegro_KEY_DEL_PAD);

	// These keys are not defined in the eAGSKey enum but are in the manual
	// https://adventuregamestudio.github.io/ags-manual/ASCIIcodes.html

	case 403:
		return ags_iskeypressed(__allegro_KEY_LSHIFT);
	case 404:
		return ags_iskeypressed(__allegro_KEY_RSHIFT);
	case 405:
		return ags_iskeypressed(__allegro_KEY_LCONTROL);
	case 406:
		return ags_iskeypressed(__allegro_KEY_RCONTROL);
	case 407:
		return ags_iskeypressed(__allegro_KEY_ALT);

	// (noted here for interest)
	// The following are the AGS_EXT_KEY_SHIFT, derived from applying arithmetic to the original keycodes.
	// These do not have a corresponding ags key enum, do not appear in the manual and may not be accessible because of OS contraints.

	case 392:
		return ags_iskeypressed(__allegro_KEY_PRTSCR);
	case 393:
		return ags_iskeypressed(__allegro_KEY_PAUSE);
	case 394:
		return ags_iskeypressed(__allegro_KEY_ABNT_C1);
	case 395:
		return ags_iskeypressed(__allegro_KEY_YEN);
	case 396:
		return ags_iskeypressed(__allegro_KEY_KANA);
	case 397:
		return ags_iskeypressed(__allegro_KEY_CONVERT);
	case 398:
		return ags_iskeypressed(__allegro_KEY_NOCONVERT);
	case 400:
		return ags_iskeypressed(__allegro_KEY_CIRCUMFLEX);
	case 402:
		return ags_iskeypressed(__allegro_KEY_KANJI);
	case 420:
		return ags_iskeypressed(__allegro_KEY_ALTGR);
	case 421:
		return ags_iskeypressed(__allegro_KEY_LWIN);
	case 422:
		return ags_iskeypressed(__allegro_KEY_RWIN);
	case 423:
		return ags_iskeypressed(__allegro_KEY_MENU);
	case 424:
		return ags_iskeypressed(__allegro_KEY_SCRLOCK);
	case 425:
		return ags_iskeypressed(__allegro_KEY_NUMLOCK);
	case 426:
		return ags_iskeypressed(__allegro_KEY_CAPSLOCK);

	// Allegro4 keys that were never supported:
	// __allegro_KEY_COMMAND
	// __allegro_KEY_TILDE
	// __allegro_KEY_BACKQUOTE

	default:
		// Remaining Allegro4 keycodes are offset by AGS_EXT_KEY_SHIFT
		if (keycode >= AGS_EXT_KEY_SHIFT) {
			if (ags_iskeypressed(keycode - AGS_EXT_KEY_SHIFT)) {
				return 1;
			}
		}
		debug_script_log("IsKeyPressed: unsupported keycode %d", keycode);
		return 0;
	}
}

int SaveScreenShot(const char *namm) {
	String fileName;
	String svg_dir = get_save_game_directory();

	if (strchr(namm, '.') == nullptr)
		fileName.Format("%s%s.bmp", svg_dir.GetCStr(), namm);
	else
		fileName.Format("%s%s", svg_dir.GetCStr(), namm);

	Bitmap *buffer = CopyScreenIntoBitmap(_GP(play).GetMainViewport().GetWidth(), _GP(play).GetMainViewport().GetHeight());
	if (!buffer->SaveToFile(fileName, _G(palette))) {
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

	if (mode == 0) {
		if (set_display_switch_mode(SWITCH_PAUSE) == -1)
			set_display_switch_mode(SWITCH_AMNESIA);
		// install callbacks to stop the sound when switching away
		set_display_switch_callback(SWITCH_IN, display_switch_in_resume);
		set_display_switch_callback(SWITCH_OUT, display_switch_out_suspend);
	} else {
		if (set_display_switch_mode(SWITCH_BACKGROUND) == -1)
			set_display_switch_mode(SWITCH_BACKAMNESIA);
		set_display_switch_callback(SWITCH_IN, display_switch_in);
		set_display_switch_callback(SWITCH_OUT, display_switch_out);
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
	_GP(play).wait_skipped_by = SKIP_AUTOTIMER; // we set timer flag by default to simplify that case
	_GP(play).wait_skipped_by_data = 0;
	_GP(play).key_skip_wait = skip_type;

	GameLoopUntilValueIsZero(&_GP(play).wait_counter);

	if (_GP(game).options[OPT_BASESCRIPTAPI] < kScriptAPI_v351) {
		// < 3.5.1 return 1 is skipped by user input, otherwise 0
		return (_GP(play).wait_skipped_by & (SKIP_KEYPRESS | SKIP_MOUSECLICK)) != 0 ? 1 : 0;
	}
	// >= 3.5.1 return positive keycode, negative mouse button code, or 0 as time-out
	switch (_GP(play).wait_skipped_by) {
	case SKIP_KEYPRESS:
		return _GP(play).wait_skipped_by_data;
	case SKIP_MOUSECLICK:
		return -(_GP(play).wait_skipped_by_data + 1); // convert to 1-based code and negate
	default:
		return 0;
	}
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
