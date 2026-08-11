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

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/display.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/font/fonts.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/speech.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/top_bar_settings.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/main/game_run.h"

namespace AGS3 {

using namespace AGS::Shared;

void DisplayAtYImpl(int ypos, const char *texx, bool as_speech);

void Display(const char *texx, ...) {
	char displbuf[STD_BUFFER_SIZE];
	va_list ap;
	va_start(ap, texx);
	vsnprintf(displbuf, sizeof(displbuf), get_translation(texx), ap);
	va_end(ap);
	DisplayAtY(-1, displbuf);
}

void DisplaySimple(const char *text) {
	DisplayAtY(-1, text);
}

void DisplayMB(const char *text) {
	DisplayAtYImpl(-1, text, false);
}

void DisplayTopBar(int ypos, int ttexcol, int backcol, const char *title, const char *text) {
	// FIXME: refactor source_text_length and get rid of this ugly hack!
	const int real_text_sourcelen = _G(source_text_length);
	snprintf(_GP(topBar).text, sizeof(_GP(topBar).text), "%s", get_translation(title));
	_G(source_text_length) = real_text_sourcelen;

	if (ypos > 0)
		_GP(play).top_bar_ypos = ypos;
	if (ttexcol > 0)
		_GP(play).top_bar_textcolor = ttexcol;
	if (backcol > 0)
		_GP(play).top_bar_backcolor = backcol;

	_GP(topBar).wantIt = 1;
	_GP(topBar).font = FONT_NORMAL;
	_GP(topBar).height = get_font_height_outlined(_GP(topBar).font);
	_GP(topBar).height += data_to_game_coord(_GP(play).top_bar_borderwidth) * 2 + get_fixed_pixel_size(1);

	// they want to customize the font
	if (_GP(play).top_bar_font >= 0)
		_GP(topBar).font = _GP(play).top_bar_font;

	// DisplaySpeech normally sets this up, but since we're not going via it...
	if (_GP(play).speech_skip_style & SKIP_AUTOTIMER)
		_GP(play).messagetime = GetTextDisplayTime(text);

	DisplayAtY(_GP(play).top_bar_ypos, text);
}

// Display a room/global message in the bar
void DisplayMessageBar(int ypos, int ttexcol, int backcol, const char *title, int msgnum) {
	char msgbufr[3001];
	get_message_text(msgnum, msgbufr);
	DisplayTopBar(ypos, ttexcol, backcol, title, msgbufr);
}

void DisplayMessageAtY(int msnum, int ypos) {
	char msgbufr[3001];
	if (msnum >= 500) {
		get_message_text(msnum, msgbufr);
		if (_G(display_message_aschar) > 0)
			DisplaySpeech(msgbufr, _G(display_message_aschar));
		else
			DisplayAtY(ypos, msgbufr);
		_G(display_message_aschar) = 0;
		return;
	}

	if (_G(display_message_aschar) > 0) {
		_G(display_message_aschar) = 0;
		quit("!DisplayMessage: data column specified a character for local\n"
		     "message; use the message editor to select the character for room\n"
		     "messages.\n");
	}

	int repeatloop = 1;
	while (repeatloop) {
		get_message_text(msnum, msgbufr);

		if (_GP(thisroom).MessageInfos[msnum].DisplayAs > 0) {
			DisplaySpeech(msgbufr, _GP(thisroom).MessageInfos[msnum].DisplayAs - 1);
		} else {
			// time out automatically if they have set that
			int oldGameSkipDisp = _GP(play).skip_display;
			if (_GP(thisroom).MessageInfos[msnum].Flags & MSG_TIMELIMIT)
				_GP(play).skip_display = 0;

			DisplayAtY(ypos, msgbufr);

			_GP(play).skip_display = oldGameSkipDisp;
		}
		if (_GP(thisroom).MessageInfos[msnum].Flags & MSG_DISPLAYNEXT) {
			msnum++;
			repeatloop = 1;
		} else
			repeatloop = 0;
	}

}

void DisplayMessage(int msnum) {
	DisplayMessageAtY(msnum, -1);
}

void DisplayAt(int xxp, int yyp, int widd, const char *text) {
	if (_GP(play).screen_is_faded_out > 0)
		debug_script_warn("Warning: blocking Display call during fade-out.");

	data_to_game_coords(&xxp, &yyp);
	widd = data_to_game_coord(widd);

	if (widd < 1) widd = _GP(play).GetUIViewport().GetWidth() / 2;
	if (xxp < 0) xxp = _GP(play).GetUIViewport().GetWidth() / 2 - widd / 2;
	display_at(xxp, yyp, widd, text);
}

void DisplayAtYImpl(int ypos, const char *texx, bool as_speech) {
	const Rect &ui_view = _GP(play).GetUIViewport();
	if ((ypos < -1) || (ypos >= ui_view.GetHeight()))
		quitprintf("!DisplayAtY: invalid Y co-ordinate supplied (used: %d; valid: 0..%d)", ypos, ui_view.GetHeight());
	if (_GP(play).screen_is_faded_out > 0)
		debug_script_warn("Warning: blocking Display call during fade-out.");

	// Display("") ... a bit of a stupid thing to do, so ignore it
	if (texx[0] == 0)
		return;

	if (ypos > 0)
		ypos = data_to_game_coord(ypos);

	if (as_speech)
		DisplaySpeechAt(-1, (ypos > 0) ? game_to_data_coord(ypos) : ypos, -1, _GP(game).playercharacter, texx);
	else {
		// Normal "Display" in text box

		if (is_screen_dirty()) {
			// erase any previous DisplaySpeech
			_GP(play).disabled_user_interface++;
			UpdateGameOnce();
			_GP(play).disabled_user_interface--;
		}

		display_at(-1, ypos, ui_view.GetWidth() / 2 + ui_view.GetWidth() / 4, get_translation(texx));
	}
}

void DisplayAtY(int ypos, const char *texx) {
	DisplayAtYImpl(ypos, texx, _GP(game).options[OPT_ALWAYSSPCH] != 0);
}

void SetSpeechStyle(int newstyle) {
	if ((newstyle < 0) || (newstyle > 3))
		quit("!SetSpeechStyle: must use a SPEECH_* constant as parameter");
	_GP(game).options[OPT_SPEECHTYPE] = newstyle;
}

void SetSkipSpeech(SkipSpeechStyle newval) {
	if ((newval < kSkipSpeechFirst) || (newval > kSkipSpeechLast))
		quit("!SetSkipSpeech: invalid skip mode specified");

	debug_script_log("SkipSpeech style set to %d", newval);
	_GP(play).speech_skip_style = user_to_internal_skip_speech((SkipSpeechStyle)newval);
}

SkipSpeechStyle GetSkipSpeech() {
	return internal_skip_speech_to_user(_GP(play).speech_skip_style);
}

} // namespace AGS3
