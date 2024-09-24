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

#include "common/stack.h"
#include "ags/engine/ac/dialog.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/shared/ac/character_info.h"
#include "ags/shared/ac/dialog_topic.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_dialog.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_gui.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/shared/ac/keycode.h"
#include "ags/engine/ac/overlay.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/parser.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/dynobj/script_dialog_options_rendering.h"
#include "ags/engine/ac/dynobj/script_drawing_surface.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/script/cc_instance.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/gui/gui_textbox.h"
#include "ags/engine/main/game_run.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/script/script.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/media/audio/audio_system.h"

#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/cc_dialog.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

void Dialog_Start(ScriptDialog *sd) {
	RunDialog(sd->id);
}

#define CHOSE_TEXTPARSER -3053
#define SAYCHOSEN_USEFLAG 1
#define SAYCHOSEN_YES 2
#define SAYCHOSEN_NO  3

int Dialog_DisplayOptions(ScriptDialog *sd, int sayChosenOption) {
	if ((sayChosenOption < 1) || (sayChosenOption > 3))
		quit("!Dialog.DisplayOptions: invalid parameter passed");

	int chose = show_dialog_options(sd->id, sayChosenOption, (_GP(game).options[OPT_RUNGAMEDLGOPTS] != 0));
	if (SHOULD_QUIT)
		return -1;

	if (chose != CHOSE_TEXTPARSER) {
		chose++;
	}
	return chose;
}

void Dialog_SetOptionState(ScriptDialog *sd, int option, int newState) {
	SetDialogOption(sd->id, option, newState);
}

int Dialog_GetOptionState(ScriptDialog *sd, int option) {
	return GetDialogOption(sd->id, option);
}

int Dialog_HasOptionBeenChosen(ScriptDialog *sd, int option) {
	if ((option < 1) || (option > _G(dialog)[sd->id].numoptions))
		quit("!Dialog.HasOptionBeenChosen: Invalid option number specified");
	option--;

	if (_G(dialog)[sd->id].optionflags[option] & DFLG_HASBEENCHOSEN)
		return 1;
	return 0;
}

void Dialog_SetHasOptionBeenChosen(ScriptDialog *sd, int option, bool chosen) {
	if (option < 1 || option > _G(dialog)[sd->id].numoptions) {
		quit("!Dialog.HasOptionBeenChosen: Invalid option number specified");
	}
	option--;
	if (chosen) {
		_G(dialog)[sd->id].optionflags[option] |= DFLG_HASBEENCHOSEN;
	} else {
		_G(dialog)[sd->id].optionflags[option] &= ~DFLG_HASBEENCHOSEN;
	}
}

int Dialog_GetOptionCount(ScriptDialog *sd) {
	return _G(dialog)[sd->id].numoptions;
}

int Dialog_GetShowTextParser(ScriptDialog *sd) {
	return (_G(dialog)[sd->id].topicFlags & DTFLG_SHOWPARSER) ? 1 : 0;
}

const char *Dialog_GetOptionText(ScriptDialog *sd, int option) {
	if ((option < 1) || (option > _G(dialog)[sd->id].numoptions))
		quit("!Dialog.GetOptionText: Invalid option number specified");

	option--;

	return CreateNewScriptString(get_translation(_G(dialog)[sd->id].optionnames[option]));
}

int Dialog_GetID(ScriptDialog *sd) {
	return sd->id;
}

const char *Dialog_GetScriptName(ScriptDialog *sd) {
	return CreateNewScriptString(_GP(game).dialogScriptNames[sd->id]);
}

//=============================================================================

#define RUN_DIALOG_STAY          -1
#define RUN_DIALOG_STOP_DIALOG   -2
#define RUN_DIALOG_GOTO_PREVIOUS -4
// dialog manager stuff

void get_dialog_script_parameters(unsigned char *&script, unsigned short *param1, unsigned short *param2) {
	script++;
	*param1 = *script;
	script++;
	*param1 += *script * 256;
	script++;

	if (param2) {
		*param2 = *script;
		script++;
		*param2 += *script * 256;
		script++;
	}
}

int run_dialog_script(int dialogID, int offse, int optionIndex) {
	_G(said_speech_line) = 0;
	int result = RUN_DIALOG_STAY;

	if (_G(dialogScriptsInst)) {
		char func_name[100];
		snprintf(func_name, sizeof(func_name), "_run_dialog%d", dialogID);
		RuntimeScriptValue params[]{ optionIndex };
		RunScriptFunction(_G(dialogScriptsInst).get(), func_name, 1, params);
		result = _G(dialogScriptsInst)->returnValue;
	} else {
		// old dialog format
		if (offse == -1)
			return result;

		unsigned char *script = _G(old_dialog_scripts)[dialogID].data() + offse;

		unsigned short param1 = 0;
		unsigned short param2 = 0;
		bool script_running = true;

		while (script_running) {
			switch (*script) {
			case DCMD_SAY:
				get_dialog_script_parameters(script, &param1, &param2);

				if (param1 == DCHAR_PLAYER)
					param1 = _GP(game).playercharacter;

				if (param1 == DCHAR_NARRATOR)
					Display(get_translation(_G(old_speech_lines)[param2].GetCStr()));
				else
					DisplaySpeech(get_translation(_G(old_speech_lines)[param2].GetCStr()), param1);

				_G(said_speech_line) = 1;
				break;

			case DCMD_OPTOFF:
				get_dialog_script_parameters(script, &param1, nullptr);
				SetDialogOption(dialogID, param1 + 1, 0, true);
				break;

			case DCMD_OPTON:
				get_dialog_script_parameters(script, &param1, nullptr);
				SetDialogOption(dialogID, param1 + 1, DFLG_ON, true);
				break;

			case DCMD_RETURN:
				script_running = false;
				break;

			case DCMD_STOPDIALOG:
				result = RUN_DIALOG_STOP_DIALOG;
				script_running = false;
				break;

			case DCMD_OPTOFFFOREVER:
				get_dialog_script_parameters(script, &param1, nullptr);
				SetDialogOption(dialogID, param1 + 1, DFLG_OFFPERM, true);
				break;

			case DCMD_RUNTEXTSCRIPT:
				get_dialog_script_parameters(script, &param1, nullptr);
				result = run_dialog_request(param1);
				script_running = (result == RUN_DIALOG_STAY);
				break;

			case DCMD_GOTODIALOG:
				get_dialog_script_parameters(script, &param1, nullptr);
				result = param1;
				script_running = false;
				break;

			case DCMD_PLAYSOUND:
				get_dialog_script_parameters(script, &param1, nullptr);
				play_sound(param1);
				break;

			case DCMD_ADDINV:
				get_dialog_script_parameters(script, &param1, nullptr);
				add_inventory(param1);
				break;

			case DCMD_SETSPCHVIEW:
				get_dialog_script_parameters(script, &param1, &param2);
				SetCharacterSpeechView(param1, param2);
				break;

			case DCMD_NEWROOM:
				get_dialog_script_parameters(script, &param1, nullptr);
				NewRoom(param1);
				if (_G(in_new_room) <= 0)
					_G(in_new_room) = 1; // set only in case NewRoom was scheduled
				result = RUN_DIALOG_STOP_DIALOG;
				script_running = false;
				break;

			case DCMD_SETGLOBALINT:
				get_dialog_script_parameters(script, &param1, &param2);
				SetGlobalInt(param1, param2);
				break;

			case DCMD_GIVESCORE:
				get_dialog_script_parameters(script, &param1, nullptr);
				GiveScore(param1);
				break;

			case DCMD_GOTOPREVIOUS:
				result = RUN_DIALOG_GOTO_PREVIOUS;
				script_running = false;
				break;

			case DCMD_LOSEINV:
				get_dialog_script_parameters(script, &param1, nullptr);
				lose_inventory(param1);
				break;

			case DCMD_ENDSCRIPT:
				result = RUN_DIALOG_STOP_DIALOG;
				script_running = false;
				break;
			}
		}
	}

	if (_G(in_new_room) > 0 || _G(abort_engine))
		return RUN_DIALOG_STOP_DIALOG;

	if (_G(said_speech_line) > 0) {
		// the line below fixes the problem with the close-up face remaining on the
		// screen after they finish talking; however, it makes the dialog options
		// area flicker when going between topics.
		DisableInterface();
		UpdateGameOnce(); // redraw the screen to make sure it looks right
		EnableInterface();
		// if we're not about to abort the dialog, switch back to arrow
		if (result != RUN_DIALOG_STOP_DIALOG)
			set_mouse_cursor(CURS_ARROW);
	}

	return result;
}

int write_dialog_options(Bitmap *ds, bool ds_has_alpha, int dlgxp, int curyp, int numdisp, int mouseison, int areawid,
		int bullet_wid, int usingfont, DialogTopic *dtop, int *disporder, short *dispyp,
		int linespacing, int utextcol, int padding) {
	int ww;

	color_t text_color;
	for (ww = 0; ww < numdisp; ww++) {

		if ((dtop->optionflags[(int)disporder[ww]] & DFLG_HASBEENCHOSEN) &&
		        (_GP(play).read_dialog_option_colour >= 0)) {
			// 'read' colour
			text_color = ds->GetCompatibleColor(_GP(play).read_dialog_option_colour);
		} else {
			// 'unread' colour
			text_color = ds->GetCompatibleColor(_G(playerchar)->talkcolor);
		}

		if (mouseison == ww) {
			if (text_color == ds->GetCompatibleColor(utextcol))
				text_color = ds->GetCompatibleColor(13); // the normal colour is the same as highlight col
			else text_color = ds->GetCompatibleColor(utextcol);
		}

		break_up_text_into_lines(get_translation(dtop->optionnames[(int)disporder[ww]]), _GP(Lines), areawid - (2 * padding + 2 + bullet_wid), usingfont);
		dispyp[ww] = curyp;
		if (_GP(game).dialog_bullet > 0) {
			draw_gui_sprite_v330(ds, _GP(game).dialog_bullet, dlgxp, curyp, ds_has_alpha);
		}
		if (_GP(game).options[OPT_DIALOGNUMBERED] == kDlgOptNumbering) {
			char tempbfr[20];
			int actualpicwid = 0;
			if (_GP(game).dialog_bullet > 0)
				actualpicwid = _GP(game).SpriteInfos[_GP(game).dialog_bullet].Width + 3;

			snprintf(tempbfr, sizeof(tempbfr), "%d.", ww + 1);
			wouttext_outline(ds, dlgxp + actualpicwid, curyp, usingfont, text_color, tempbfr);
		}
		for (size_t cc = 0; cc < _GP(Lines).Count(); cc++) {
			wouttext_outline(ds, dlgxp + ((cc == 0) ? 0 : 9) + bullet_wid, curyp, usingfont, text_color, _GP(Lines)[cc].GetCStr());
			curyp += linespacing;
		}
		if (ww < numdisp - 1)
			curyp += data_to_game_coord(_GP(game).options[OPT_DIALOGGAP]);
	}
	return curyp;
}



#define GET_OPTIONS_HEIGHT {\
		needheight = 0;\
		for (int i = 0; i < numdisp; ++i) {\
			break_up_text_into_lines(get_translation(dtop->optionnames[(int)disporder[i]]), _GP(Lines), areawid-(2*padding+2+bullet_wid), usingfont);\
			needheight += get_text_lines_surf_height(usingfont, _GP(Lines).Count()) + data_to_game_coord(_GP(game).options[OPT_DIALOGGAP]);\
		}\
		if (parserInput) needheight += parserInput->GetHeight() + data_to_game_coord(_GP(game).options[OPT_DIALOGGAP]);\
	}


void draw_gui_for_dialog_options(Bitmap *ds, GUIMain *guib, int dlgxp, int dlgyp) {
	if (guib->BgColor != 0) {
		color_t draw_color = ds->GetCompatibleColor(guib->BgColor);
		ds->FillRect(Rect(dlgxp, dlgyp, dlgxp + guib->Width, dlgyp + guib->Height), draw_color);
	}
	if (guib->BgImage > 0)
		GfxUtil::DrawSpriteWithTransparency(ds, _GP(spriteset)[guib->BgImage], dlgxp, dlgyp);
}

bool get_custom_dialog_options_dimensions(int dlgnum) {
	_GP(ccDialogOptionsRendering).Reset();
	_GP(ccDialogOptionsRendering).dialogID = dlgnum;

	_GP(getDialogOptionsDimensionsFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
	run_function_on_non_blocking_thread(&_GP(getDialogOptionsDimensionsFunc));

	if ((_GP(ccDialogOptionsRendering).width > 0) &&
	        (_GP(ccDialogOptionsRendering).height > 0)) {
		return true;
	}
	return false;
}

#define DLG_OPTION_PARSER 99

struct DialogOptions {
	int dlgnum;
	bool runGameLoopsInBackground;

	int dlgxp;
	int dlgyp;
	int dialog_abs_x; // absolute dialog position on screen
	int padding;
	int usingfont;
	int lineheight;
	int linespacing;
	int curswas;
	int bullet_wid;
	int needheight;
	IDriverDependantBitmap *ddb;
	Bitmap *subBitmap;
	GUITextBox *parserInput;
	DialogTopic *dtop;

	// display order of options
	int disporder[MAXTOPICOPTIONS];
	// display Y coordinate of options
	short dispyp[MAXTOPICOPTIONS];
	// number of displayed options
	int numdisp;
	// currently chosen option
	int chose;

	Bitmap *tempScrn;
	int parserActivated;

	int curyp;
	bool needRedraw;
	bool wantRefresh;
	bool usingCustomRendering;
	int orixp;
	int oriyp;
	int areawid;
	int is_textwindow;
	int dirtyx;
	int dirtyy;
	int dirtywidth;
	int dirtyheight;

	int mouseison;
	int mousewason;

	int forecol;

	void Prepare(int _dlgnum, bool _runGameLoopsInBackground);
	void Show();
	void Redraw();
	// Runs the dialog options update;
	// returns whether should continue to run options loop, or stop
	bool Run();
	// Process all the buffered key events;
	// returns whether should continue to run options loop, or stop
	bool RunKeyControls();
	// Process single key event
	// returns whether should continue to run options loop, or stop
	bool RunKey(const KeyInput &ki);
	void Close();
};

void DialogOptions::Prepare(int _dlgnum, bool _runGameLoopsInBackground) {
	dlgnum = _dlgnum;
	runGameLoopsInBackground = _runGameLoopsInBackground;

	dlgyp = get_fixed_pixel_size(160);
	usingfont = FONT_NORMAL;
	lineheight = get_font_height_outlined(usingfont);
	linespacing = get_font_linespacing(usingfont);
	curswas = _G(cur_cursor);
	bullet_wid = 0;
	ddb = nullptr;
	subBitmap = nullptr;
	parserInput = nullptr;
	dtop = nullptr;

	if ((dlgnum < 0) || (dlgnum >= _GP(game).numdialog))
		quit("!RunDialog: invalid dialog number specified");

	can_run_delayed_command();

	_GP(play).in_conversation ++;

	if (_GP(game).dialog_bullet > 0)
		bullet_wid = _GP(game).SpriteInfos[_GP(game).dialog_bullet].Width + 3;

	// numbered options, leave space for the numbers
	if (_GP(game).options[OPT_DIALOGNUMBERED] == kDlgOptNumbering)
		bullet_wid += get_text_width_outlined("9. ", usingfont);

	_G(said_text) = 0;

	const Rect &ui_view = _GP(play).GetUIViewport();
	tempScrn = BitmapHelper::CreateBitmap(ui_view.GetWidth(), ui_view.GetHeight(), _GP(game).GetColorDepth());

	set_mouse_cursor(CURS_ARROW);

	dtop = &_G(dialog)[dlgnum];

	chose = -1;
	numdisp = 0;

	parserActivated = 0;
	if ((dtop->topicFlags & DTFLG_SHOWPARSER) && (_GP(play).disable_dialog_parser == 0)) {
		parserInput = new GUITextBox();
		parserInput->SetHeight(lineheight + get_fixed_pixel_size(4));
		parserInput->SetShowBorder(true);
		parserInput->Font = usingfont;
	}

	numdisp = 0;
	for (int i = 0; i < dtop->numoptions; ++i) {
		if ((dtop->optionflags[i] & DFLG_ON) == 0) continue;
		ensure_text_valid_for_font(dtop->optionnames[i], usingfont);
		disporder[numdisp] = i;
		numdisp++;
	}
}

void DialogOptions::Show() {
	if (numdisp < 1) {
		debug_script_warn("Dialog: all options have been turned off, stopping dialog.");
		return;
	}
	// Don't display the options if there is only one and the parser
	// is not enabled.
	if (!((numdisp > 1) || (parserInput != nullptr) || (_GP(play).show_single_dialog_option))) {
		chose = disporder[0];  // only one choice, so select it
		return;
	}

	is_textwindow = 0;
	forecol = _GP(play).dialog_options_highlight_color;

	mouseison = -1;
	mousewason = -10;
	const Rect &ui_view = _GP(play).GetUIViewport();
	dirtyx = 0;
	dirtyy = 0;
	dirtywidth = ui_view.GetWidth();
	dirtyheight = ui_view.GetHeight();
	usingCustomRendering = false;


	dlgxp = 1;
	if (get_custom_dialog_options_dimensions(dlgnum)) {
		usingCustomRendering = true;
		dirtyx = data_to_game_coord(_GP(ccDialogOptionsRendering).x);
		dirtyy = data_to_game_coord(_GP(ccDialogOptionsRendering).y);
		dirtywidth = data_to_game_coord(_GP(ccDialogOptionsRendering).width);
		dirtyheight = data_to_game_coord(_GP(ccDialogOptionsRendering).height);
		dialog_abs_x = dirtyx;
	} else if (_GP(game).options[OPT_DIALOGIFACE] > 0) {
		GUIMain *guib = &_GP(guis)[_GP(game).options[OPT_DIALOGIFACE]];
		if (guib->IsTextWindow()) {
			// text-window, so do the QFG4-style speech options
			is_textwindow = 1;
			forecol = guib->FgColor;
		} else {
			dlgxp = guib->X;
			dlgyp = guib->Y;

			dirtyx = dlgxp;
			dirtyy = dlgyp;
			dirtywidth = guib->Width;
			dirtyheight = guib->Height;
			dialog_abs_x = guib->X;

			areawid = guib->Width - 5;
			padding = TEXTWINDOW_PADDING_DEFAULT;

			GET_OPTIONS_HEIGHT

			if (_GP(game).options[OPT_DIALOGUPWARDS]) {
				// They want the options upwards from the bottom
				dlgyp = (guib->Y + guib->Height) - needheight;
			}

		}
	} else {
		//dlgyp=(_GP(play).viewport.GetHeight()-numdisp*txthit)-1;
		areawid = ui_view.GetWidth() - 5;
		padding = TEXTWINDOW_PADDING_DEFAULT;
		GET_OPTIONS_HEIGHT
		dlgyp = ui_view.GetHeight() - needheight;

		dirtyx = 0;
		dirtyy = dlgyp - 1;
		dirtywidth = ui_view.GetWidth();
		dirtyheight = ui_view.GetHeight() - dirtyy;
		dialog_abs_x = 0;
	}
	if (!is_textwindow)
		areawid -= data_to_game_coord(_GP(play).dialog_options_x) * 2;

	orixp = dlgxp;
	oriyp = dlgyp;
	needRedraw = false;
	wantRefresh = false;
	mouseison = -10;

	Redraw();
	while (Run() && !SHOULD_QUIT) {}

	// Close custom dialog options
	if (usingCustomRendering) {
		_GP(runDialogOptionCloseFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
		run_function_on_non_blocking_thread(&_GP(runDialogOptionCloseFunc));
	}
}

void DialogOptions::Redraw() {
	wantRefresh = true;

	if (usingCustomRendering) {
		tempScrn = recycle_bitmap(tempScrn, _GP(game).GetColorDepth(),
		                          data_to_game_coord(_GP(ccDialogOptionsRendering).width),
		                          data_to_game_coord(_GP(ccDialogOptionsRendering).height));
	}

	tempScrn->ClearTransparent();
	Bitmap *ds = tempScrn;

	dlgxp = orixp;
	dlgyp = oriyp;
	const Rect &ui_view = _GP(play).GetUIViewport();

	bool options_surface_has_alpha = false;

	if (usingCustomRendering) {
		_GP(ccDialogOptionsRendering).surfaceToRenderTo = _G(dialogOptionsRenderingSurface);
		_GP(ccDialogOptionsRendering).surfaceAccessed = false;
		_G(dialogOptionsRenderingSurface)->linkedBitmapOnly = tempScrn;
		_G(dialogOptionsRenderingSurface)->hasAlphaChannel = _GP(ccDialogOptionsRendering).hasAlphaChannel;
		options_surface_has_alpha = _G(dialogOptionsRenderingSurface)->hasAlphaChannel != 0;

		_GP(renderDialogOptionsFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
		run_function_on_non_blocking_thread(&_GP(renderDialogOptionsFunc));

		if (!_GP(ccDialogOptionsRendering).surfaceAccessed)
			debug_script_warn("dialog_options_get_dimensions was implemented, but no dialog_options_render function drew anything to the surface");

		if (parserInput) {
			parserInput->X = data_to_game_coord(_GP(ccDialogOptionsRendering).parserTextboxX);
			curyp = data_to_game_coord(_GP(ccDialogOptionsRendering).parserTextboxY);
			areawid = data_to_game_coord(_GP(ccDialogOptionsRendering).parserTextboxWidth);
			if (areawid == 0)
				areawid = tempScrn->GetWidth();
		}
		_GP(ccDialogOptionsRendering).needRepaint = false;
	} else if (is_textwindow) {
		// text window behind the options
		areawid = data_to_game_coord(_GP(play).max_dialogoption_width);
		int biggest = 0;
		padding = _GP(guis)[_GP(game).options[OPT_DIALOGIFACE]].Padding;
		for (int i = 0; i < numdisp; ++i) {
			break_up_text_into_lines(get_translation(dtop->optionnames[(int)disporder[i]]), _GP(Lines), areawid - ((2 * padding + 2) + bullet_wid), usingfont);
			if (_G(longestline) > biggest)
				biggest = _G(longestline);
		}
		if (biggest < areawid - ((2 * padding + 6) + bullet_wid))
			areawid = biggest + ((2 * padding + 6) + bullet_wid);

		if (areawid < data_to_game_coord(_GP(play).min_dialogoption_width)) {
			areawid = data_to_game_coord(_GP(play).min_dialogoption_width);
			if (_GP(play).min_dialogoption_width > _GP(play).max_dialogoption_width)
				quit("!_GP(game).min_dialogoption_width is larger than _GP(game).max_dialogoption_width");
		}

		GET_OPTIONS_HEIGHT

		int savedwid = areawid;
		int txoffs = 0, tyoffs = 0, yspos = ui_view.GetHeight() / 2 - (2 * padding + needheight) / 2;
		int xspos = ui_view.GetWidth() / 2 - areawid / 2;
		// shift window to the right if QG4-style full-screen pic
		if ((_GP(game).options[OPT_SPEECHTYPE] == 3) && (_G(said_text) > 0))
			xspos = (ui_view.GetWidth() - areawid) - get_fixed_pixel_size(10);

		// needs to draw the right text window, not the default
		Bitmap *text_window_ds = nullptr;
		draw_text_window(&text_window_ds, false, &txoffs, &tyoffs, &xspos, &yspos, &areawid, nullptr, needheight, _GP(game).options[OPT_DIALOGIFACE]);
		options_surface_has_alpha = _GP(guis)[_GP(game).options[OPT_DIALOGIFACE]].HasAlphaChannel();
		// since draw_text_window incrases the width, restore it
		areawid = savedwid;

		dirtyx = xspos;
		dirtyy = yspos;
		dirtywidth = text_window_ds->GetWidth();
		dirtyheight = text_window_ds->GetHeight();
		dialog_abs_x = txoffs + xspos;

		GfxUtil::DrawSpriteWithTransparency(ds, text_window_ds, xspos, yspos);
		// TODO: here we rely on draw_text_window always assigning new bitmap to text_window_ds;
		// should make this more explicit
		delete text_window_ds;

		// Ignore the dialog_options_x/y offsets when using a text window
		txoffs += xspos;
		tyoffs += yspos;
		dlgyp = tyoffs;
		curyp = write_dialog_options(ds, options_surface_has_alpha, txoffs, tyoffs, numdisp, mouseison, areawid, bullet_wid, usingfont, dtop, disporder, dispyp, linespacing, forecol, padding);
		if (parserInput)
			parserInput->X = txoffs;
	} else {

		if (wantRefresh) {
			// redraw the black background so that anti-alias
			// fonts don't re-alias themselves
			if (_GP(game).options[OPT_DIALOGIFACE] == 0) {
				color_t draw_color = ds->GetCompatibleColor(16);
				ds->FillRect(Rect(0, dlgyp - 1, ui_view.GetWidth() - 1, ui_view.GetHeight() - 1), draw_color);
			} else {
				GUIMain *guib = &_GP(guis)[_GP(game).options[OPT_DIALOGIFACE]];
				if (!guib->IsTextWindow())
					draw_gui_for_dialog_options(ds, guib, dlgxp, dlgyp);
			}
		}

		dirtyx = 0;
		dirtywidth = ui_view.GetWidth();

		if (_GP(game).options[OPT_DIALOGIFACE] > 0) {
			// the whole GUI area should be marked dirty in order
			// to ensure it gets drawn
			GUIMain *guib = &_GP(guis)[_GP(game).options[OPT_DIALOGIFACE]];
			dirtyheight = guib->Height;
			dirtyy = dlgyp;
			options_surface_has_alpha = guib->HasAlphaChannel();
		} else {
			dirtyy = dlgyp - 1;
			dirtyheight = needheight + 1;
			options_surface_has_alpha = false;
		}

		dlgxp += data_to_game_coord(_GP(play).dialog_options_x);
		dlgyp += data_to_game_coord(_GP(play).dialog_options_y);

		// if they use a negative dialog_options_y, make sure the
		// area gets marked as dirty
		if (dlgyp < dirtyy)
			dirtyy = dlgyp;

		curyp = dlgyp;
		curyp = write_dialog_options(ds, options_surface_has_alpha, dlgxp, curyp, numdisp, mouseison, areawid, bullet_wid, usingfont, dtop, disporder, dispyp, linespacing, forecol, padding);

		if (parserInput)
			parserInput->X = dlgxp;
	}

	if (parserInput) {
		// Set up the text box, if present
		parserInput->Y = curyp + data_to_game_coord(_GP(game).options[OPT_DIALOGGAP]);
		parserInput->SetWidth(areawid - get_fixed_pixel_size(10));
		parserInput->TextColor = _G(playerchar)->talkcolor;
		if (mouseison == DLG_OPTION_PARSER)
			parserInput->TextColor = forecol;

		if (_GP(game).dialog_bullet) { // the parser X will get moved in a second
			draw_gui_sprite_v330(ds, _GP(game).dialog_bullet, parserInput->X, parserInput->Y, options_surface_has_alpha);
		}

		parserInput->SetWidth(parserInput->GetWidth() - bullet_wid);
		parserInput->X += bullet_wid;

		parserInput->Draw(ds, parserInput->X, parserInput->Y);
		parserInput->IsActivated = false;
	}

	wantRefresh = false;

	subBitmap = recycle_bitmap(subBitmap,
		_G(gfxDriver)->GetCompatibleBitmapFormat(tempScrn->GetColorDepth()), dirtywidth, dirtyheight);

	if (usingCustomRendering) {
		subBitmap->Blit(tempScrn, 0, 0, 0, 0, tempScrn->GetWidth(), tempScrn->GetHeight());
		invalidate_rect(dirtyx, dirtyy, dirtyx + subBitmap->GetWidth(), dirtyy + subBitmap->GetHeight(), false);
	} else {
		subBitmap->Blit(tempScrn, dirtyx, dirtyy, 0, 0, dirtywidth, dirtyheight);
	}

	if ((ddb != nullptr) &&
	        ((ddb->GetWidth() != dirtywidth) ||
	         (ddb->GetHeight() != dirtyheight))) {
		_G(gfxDriver)->DestroyDDB(ddb);
		ddb = nullptr;
	}

	if (ddb == nullptr)
		ddb = _G(gfxDriver)->CreateDDBFromBitmap(subBitmap, options_surface_has_alpha, false);
	else
		_G(gfxDriver)->UpdateDDBFromBitmap(ddb, subBitmap, options_surface_has_alpha);

	if (runGameLoopsInBackground) {
		render_graphics(ddb, dirtyx, dirtyy);
	}
}

bool DialogOptions::Run() {
	// Run() can be called in a loop, so keep events going.
	sys_evt_process_pending();

	const bool new_custom_render = usingCustomRendering && _GP(game).options[OPT_DIALOGOPTIONSAPI] >= 0;

	if (runGameLoopsInBackground) {
		_GP(play).disabled_user_interface++;
		UpdateGameOnce(false, ddb, dirtyx, dirtyy);
		_GP(play).disabled_user_interface--;
	} else {
		update_audio_system_on_game_loop();
		UpdateCursorAndDrawables();
		render_graphics(ddb, dirtyx, dirtyy);
	}

	if (new_custom_render) {
		_GP(runDialogOptionRepExecFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
		run_function_on_non_blocking_thread(&_GP(runDialogOptionRepExecFunc));
	}

	needRedraw = false;

	// Handle keyboard
	if (!RunKeyControls())
		return false; // end loop

	if (needRedraw)
		Redraw();

	// Handle mouse
	mousewason = mouseison;
	mouseison = -1;
	if (new_custom_render); // do not automatically detect option under mouse
	else if (usingCustomRendering) {
		if ((_G(mousex) >= dirtyx) && (_G(mousey) >= dirtyy) &&
			(_G(mousex) < dirtyx + tempScrn->GetWidth()) &&
			(_G(mousey) < dirtyy + tempScrn->GetHeight())) {
			_GP(getDialogOptionUnderCursorFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
			run_function_on_non_blocking_thread(&_GP(getDialogOptionUnderCursorFunc));

			if (!_GP(getDialogOptionUnderCursorFunc).atLeastOneImplementationExists)
				quit("!The script function dialog_options_get_active is not implemented. It must be present to use a custom dialogue system.");

			mouseison = _GP(ccDialogOptionsRendering).activeOptionID;
		} else {
			_GP(ccDialogOptionsRendering).activeOptionID = -1;
		}
	} else if (_G(mousex) >= dialog_abs_x && _G(mousex) < (dialog_abs_x + areawid) &&
		_G(mousey) >= dlgyp && _G(mousey) < curyp) {
		mouseison = numdisp - 1;
		for (int i = 0; i < numdisp; ++i) {
			if (_G(mousey) < dispyp[i]) {
				mouseison = i - 1; break;
			}
		}
		if ((mouseison < 0) | (mouseison >= numdisp)) mouseison = -1;
	}

	if (parserInput != nullptr) {
		int relative_mousey = _G(mousey);
		if (usingCustomRendering)
			relative_mousey -= dirtyy;

		if ((relative_mousey > parserInput->Y) &&
			(relative_mousey < parserInput->Y + parserInput->GetHeight()))
			mouseison = DLG_OPTION_PARSER;

		if (parserInput->IsActivated)
			parserActivated = 1;
	}

	eAGSMouseButton mbut;
	int mwheelz;
	if (run_service_mb_controls(mbut, mwheelz) && mbut > kMouseNone &&
		!_GP(play).IsIgnoringInput()) {
		if (mouseison < 0 && !new_custom_render) {
			if (usingCustomRendering) {
				_GP(runDialogOptionMouseClickHandlerFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
				_GP(runDialogOptionMouseClickHandlerFunc).params[1].SetInt32(mbut);
				run_function_on_non_blocking_thread(&_GP(runDialogOptionMouseClickHandlerFunc));

				if (_GP(runDialogOptionMouseClickHandlerFunc).atLeastOneImplementationExists) {
					Redraw();
					return true; // continue running loop
				}
			}
			return true; // continue running loop
		}
		if (mouseison == DLG_OPTION_PARSER) {
			// they clicked the text box
			parserActivated = 1;
		} else if (new_custom_render) {
			_GP(runDialogOptionMouseClickHandlerFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
			_GP(runDialogOptionMouseClickHandlerFunc).params[1].SetInt32(mbut);
			run_function_on_non_blocking_thread(&_GP(runDialogOptionMouseClickHandlerFunc));
		} else if (usingCustomRendering) {
			chose = mouseison;
			return false; // end dialog options running loop
		} else {
			chose = disporder[mouseison];
			return false; // end dialog options running loop
		}
	}

	if (usingCustomRendering) {
		if (mwheelz != 0) {
			_GP(runDialogOptionMouseClickHandlerFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
			_GP(runDialogOptionMouseClickHandlerFunc).params[1].SetInt32((mwheelz < 0) ? 9 : 8);
			run_function_on_non_blocking_thread(&_GP(runDialogOptionMouseClickHandlerFunc));

			if (!new_custom_render) {
				if (_GP(runDialogOptionMouseClickHandlerFunc).atLeastOneImplementationExists)
					Redraw();
				return true; // continue running loop
			}
		}
	}

	if (parserActivated) {
		// They have selected a custom parser-based option
		if (!parserInput->Text.IsEmpty() != 0) {
			chose = DLG_OPTION_PARSER;
			return false; // end dialog options running loop
		} else {
			parserActivated = 0;
			parserInput->IsActivated = 0;
		}
	}
	if (mousewason != mouseison) {
		Redraw();
		return true; // continue running loop
	}
	if (new_custom_render) {
		if (_GP(ccDialogOptionsRendering).chosenOptionID >= 0) {
			chose = _GP(ccDialogOptionsRendering).chosenOptionID;
			_GP(ccDialogOptionsRendering).chosenOptionID = -1;
			return false; // end dialog options running loop
		}
		if (_GP(ccDialogOptionsRendering).needRepaint) {
			Redraw();
			return true; // continue running loop
		}
	}

	update_polled_stuff();

	if (!runGameLoopsInBackground && (_GP(play).fast_forward == 0)) { // note if runGameLoopsInBackground then it's called inside UpdateGameOnce
		WaitForNextFrame();
	}

	return true; // continue running loop
}

bool DialogOptions::RunKeyControls() {
	// Handle all the buffered key events
	bool do_break = false; // continue the loop or end dialog options
	while (ags_keyevent_ready()) {
		KeyInput ki;
		if (run_service_key_controls(ki) && !_GP(play).IsIgnoringInput()) {
			if (!do_break && !RunKey(ki)) {
				ags_clear_input_buffer();
				do_break = true; // end dialog options
			}
		}
	}
	return !do_break;
}

bool DialogOptions::RunKey(const KeyInput &ki) {
	const bool new_custom_render = usingCustomRendering && _GP(game).options[OPT_DIALOGOPTIONSAPI] >= 0;
	const bool old_keyhandle = _GP(game).options[OPT_KEYHANDLEAPI] == 0;

	const eAGSKeyCode agskey = ki.Key;
	if (parserInput) {
		wantRefresh = true;
		// type into the parser
		// TODO: find out what are these key commands, and are these documented?
		if ((agskey == eAGSKeyCodeF3) || ((agskey == eAGSKeyCodeSpace) && (parserInput->Text.GetLength() == 0))) {
			// write previous contents into textbox (F3 or Space when box is empty)
			size_t last_len = ustrlen(_GP(play).lastParserEntry);
			size_t cur_len = ustrlen(parserInput->Text.GetCStr());
			// [ikm] CHECKME: tbh I don't quite get the logic here (it was like this in original code);
			// but what we do is copying only the last part of the previous string
			if (cur_len < last_len) {
				const char *entry = _GP(play).lastParserEntry;
				// TODO: utility function for advancing N utf-8 chars
				for (size_t i = 0; i < cur_len; ++i) ugetxc(&entry);
				parserInput->Text.Append(entry);
			}
			needRedraw = true;
			return true; // continue running loop
		} else if ((ki.UChar > 0) || (agskey == eAGSKeyCodeReturn) || (agskey == eAGSKeyCodeBackspace)) {
			parserInput->OnKeyPress(ki);
			if (!parserInput->IsActivated) {
				needRedraw = true;
				return true; // continue running loop
			}
		}
	} else if (new_custom_render) {
		if (old_keyhandle || (ki.UChar == 0)) {
			// "dialog_options_key_press"
			_GP(runDialogOptionKeyPressHandlerFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
			_GP(runDialogOptionKeyPressHandlerFunc).params[1].SetInt32(AGSKeyToScriptKey(ki.Key));
			_GP(runDialogOptionKeyPressHandlerFunc).params[2].SetInt32(ki.Mod);
			run_function_on_non_blocking_thread(&_GP(runDialogOptionKeyPressHandlerFunc));
		}
		if (!old_keyhandle && (ki.UChar > 0)) {
			// "dialog_options_text_input"
			_GP(runDialogOptionTextInputHandlerFunc).params[0].SetScriptObject(&_GP(ccDialogOptionsRendering), &_GP(ccDialogOptionsRendering));
			_GP(runDialogOptionTextInputHandlerFunc).params[1].SetInt32(ki.UChar);
			run_function_on_non_blocking_thread(&_GP(runDialogOptionKeyPressHandlerFunc));
		}
	}
	// Allow selection of options by keyboard shortcuts
	else if (_GP(game).options[OPT_DIALOGNUMBERED] >= kDlgOptKeysOnly &&
		agskey >= '1' && agskey <= '9') {
		int numkey = agskey - '1';
		if (numkey < numdisp) {
			chose = disporder[numkey];
			return false; // end dialog options running loop
		}
	}
	return true; // continue running loop
}

void DialogOptions::Close() {
	ags_clear_input_buffer();
	invalidate_screen();

	if (parserActivated) {
		assert(parserInput);
		snprintf(_GP(play).lastParserEntry, MAX_MAXSTRLEN, "%s", parserInput->Text.GetCStr());
		ParseText(parserInput->Text.GetCStr());
		chose = CHOSE_TEXTPARSER;
	}

	if (parserInput) {
		delete parserInput;
		parserInput = nullptr;
	}

	if (ddb != nullptr)
		_G(gfxDriver)->DestroyDDB(ddb);
	delete subBitmap;

	set_mouse_cursor(curswas);
	// In case it's the QFG4 style dialog, remove the black screen
	_GP(play).in_conversation--;
	remove_screen_overlay(OVER_COMPLETE);

	delete tempScrn;
}

int show_dialog_options(int _dlgnum, int sayChosenOption, bool _runGameLoopsInBackground) {
	DialogOptions dlgopt;
	dlgopt.Prepare(_dlgnum, _runGameLoopsInBackground);
	dlgopt.Show();
	dlgopt.Close();

	int dialog_choice = dlgopt.chose;
	if (dialog_choice >= 0) { // NOTE: this condition also excludes CHOSE_TEXTPARSER
		assert(dialog_choice >= 0 && dialog_choice < MAXTOPICOPTIONS);
		DialogTopic *dialog_topic = dlgopt.dtop;
		int32_t &option_flags = dialog_topic->optionflags[dialog_choice];
		const char *option_name = dlgopt.dtop->optionnames[dialog_choice];

		option_flags |= DFLG_HASBEENCHOSEN;
		bool sayTheOption = false;
		if (sayChosenOption == SAYCHOSEN_YES) {
			sayTheOption = true;
		} else if (sayChosenOption == SAYCHOSEN_USEFLAG) {
			sayTheOption = ((option_flags & DFLG_NOREPEAT) == 0);
		}

		if (sayTheOption)
			DisplaySpeech(get_translation(option_name), _GP(game).playercharacter);
	}

	return dialog_choice;
}

// Dialog execution state
struct DialogExec {
	int DlgNum = -1;
	int DlgWas = -1;
	// CHECKME: this may be unnecessary, investigate later
	bool IsFirstEntry = true;
	// nested dialogs "stack"
	Common::Stack<int> TopicHist;

	DialogExec(int start_dlgnum) : DlgNum(start_dlgnum) {}
	int HandleDialogResult(int res);
	void Run();
};

int DialogExec::HandleDialogResult(int res) {
	// Handle goto-previous, see if there's any previous dialog in history
	if (res == RUN_DIALOG_GOTO_PREVIOUS) {
		if (TopicHist.size() == 0)
			return RUN_DIALOG_STOP_DIALOG;
		res = TopicHist.top();
		TopicHist.pop();
	}
	// Continue to the next dialog
	if (res >= 0) {
		// save the old topic number in the history, and switch to the new one
		TopicHist.push(DlgNum);
		DlgNum = res;
		return DlgNum;
	}
	return res;
 }

void DialogExec::Run() {
	while (DlgNum >= 0) {
		if (DlgNum < 0 || DlgNum >= _GP(game).numdialog)
			quitprintf("!RunDialog: invalid dialog number specified: %d", DlgNum);

		// current dialog object
		DialogTopic *dtop = &_G(dialog)[DlgNum];
		int res = 0; // dialog execution result
		// If a new dialog topic: run dialog entry point
		if (DlgNum != DlgWas) {
			res = run_dialog_script(DlgNum, dtop->startupentrypoint, 0);
			DlgWas = DlgNum;

			// Handle the dialog entry's result
			res = HandleDialogResult(res);
			if (res == RUN_DIALOG_STOP_DIALOG)
				return; // stop the dialog
			IsFirstEntry = false;
			if (res != RUN_DIALOG_STAY)
				continue; // skip to the next dialog
		}

		// Show current dialog's options
		int chose = show_dialog_options(DlgNum, SAYCHOSEN_USEFLAG, (_GP(game).options[OPT_RUNGAMEDLGOPTS] != 0));
		if (chose == CHOSE_TEXTPARSER) {
			_G(said_speech_line) = 0;
			res = run_dialog_request(DlgNum);
			if (_G(said_speech_line) > 0) {
				// fix the problem with the close-up face remaining on screen
				DisableInterface();
				UpdateGameOnce(); // redraw the screen to make sure it looks right
				EnableInterface();
				set_mouse_cursor(CURS_ARROW);
			}
		} else if (chose >= 0) {
			// chose some option - run its script
			res = run_dialog_script(DlgNum, dtop->entrypoints[chose], chose + 1);
		} else {
			return; // no option chosen? - stop the dialog
		}

		// Handle the dialog option's result
		res = HandleDialogResult(res);
		if (res == RUN_DIALOG_STOP_DIALOG)
			return; // stop the dialog
		// continue to the next dialog or show same dialog's options again
	}
}

void do_conversation(int dlgnum) {
	EndSkippingUntilCharStops();

	// AGS 2.x always makes the mouse cursor visible when displaying a dialog.
	if (_G(loaded_game_file_version) <= kGameVersion_272)
		_GP(play).mouse_cursor_hidden = 0;

	DialogExec dlgexec(dlgnum);
	dlgexec.Run();
	// CHECKME: find out if this is safe to do always, regardless of number of iterations
	if (dlgexec.IsFirstEntry) {
		// bail out from first startup script
		remove_screen_overlay(OVER_COMPLETE);
		_GP(play).in_conversation--;
	}
}

// end dialog manager

//=============================================================================
//
// Script API Functions
//
//=============================================================================

ScriptDialog *Dialog_GetByName(const char *name) {
	return static_cast<ScriptDialog *>(ccGetScriptObjectAddress(name, _GP(ccDynamicDialog).GetType()));
}

RuntimeScriptValue Sc_Dialog_GetByName(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_POBJ(ScriptDialog, _GP(ccDynamicDialog), Dialog_GetByName, const char);
}

// int (ScriptDialog *sd)
RuntimeScriptValue Sc_Dialog_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDialog, Dialog_GetID);
}

RuntimeScriptValue Sc_Dialog_GetScriptName(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptDialog, const char, _GP(myScriptStringImpl), Dialog_GetScriptName);
}

// int (ScriptDialog *sd)
RuntimeScriptValue Sc_Dialog_GetOptionCount(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDialog, Dialog_GetOptionCount);
}

// int (ScriptDialog *sd)
RuntimeScriptValue Sc_Dialog_GetShowTextParser(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptDialog, Dialog_GetShowTextParser);
}

// int (ScriptDialog *sd, int sayChosenOption)
RuntimeScriptValue Sc_Dialog_DisplayOptions(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT(ScriptDialog, Dialog_DisplayOptions);
}

// int (ScriptDialog *sd, int option)
RuntimeScriptValue Sc_Dialog_GetOptionState(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT(ScriptDialog, Dialog_GetOptionState);
}

// const char* (ScriptDialog *sd, int option)
RuntimeScriptValue Sc_Dialog_GetOptionText(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT(ScriptDialog, const char, _GP(myScriptStringImpl), Dialog_GetOptionText);
}

// int (ScriptDialog *sd, int option)
RuntimeScriptValue Sc_Dialog_HasOptionBeenChosen(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT(ScriptDialog, Dialog_HasOptionBeenChosen);
}

RuntimeScriptValue Sc_Dialog_SetHasOptionBeenChosen(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT_PBOOL(ScriptDialog, Dialog_SetHasOptionBeenChosen);
}

// void (ScriptDialog *sd, int option, int newState)
RuntimeScriptValue Sc_Dialog_SetOptionState(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(ScriptDialog, Dialog_SetOptionState);
}

// void (ScriptDialog *sd)
RuntimeScriptValue Sc_Dialog_Start(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptDialog, Dialog_Start);
}

void RegisterDialogAPI() {
	ScFnRegister dialog_api[] = {
		{"Dialog::GetByName", API_FN_PAIR(Dialog_GetByName)},
		{"Dialog::get_ID", API_FN_PAIR(Dialog_GetID)},
		{"Dialog::get_OptionCount", API_FN_PAIR(Dialog_GetOptionCount)},
		{"Dialog::get_ScriptName", API_FN_PAIR(Dialog_GetScriptName)},
		{"Dialog::get_ShowTextParser", API_FN_PAIR(Dialog_GetShowTextParser)},
		{"Dialog::DisplayOptions^1", API_FN_PAIR(Dialog_DisplayOptions)},
		{"Dialog::GetOptionState^1", API_FN_PAIR(Dialog_GetOptionState)},
		{"Dialog::GetOptionText^1", API_FN_PAIR(Dialog_GetOptionText)},
		{"Dialog::HasOptionBeenChosen^1", API_FN_PAIR(Dialog_HasOptionBeenChosen)},
		{"Dialog::SetHasOptionBeenChosen^2", API_FN_PAIR(Dialog_SetHasOptionBeenChosen)},
		{"Dialog::SetOptionState^2", API_FN_PAIR(Dialog_SetOptionState)},
		{"Dialog::Start^0", API_FN_PAIR(Dialog_Start)},
	};

	ccAddExternalFunctions361(dialog_api);
}

} // namespace AGS3
