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

#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/ac/dynobj/script_game.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/script/cc_common.h" // cc_error
#include "ags/globals.h"

namespace AGS3 {

int32_t CCScriptGame::ReadInt32(void *address, intptr_t offset) {
	const int index = offset / sizeof(int32_t);
	if (index >= 5 && index < 5 + MAXGLOBALVARS)
		return _GP(play).globalvars[index - 5];

	switch (index) {
	case 0:
		return _GP(play).score;
	case 1:
		return _GP(play).usedmode;
	case 2:
		return _GP(play).disabled_user_interface;
	case 3:
		return _GP(play).gscript_timer;
	case 4:
		return _GP(play).debug_mode;
	// 5 -> 54: _GP(play).globalvars
	case 55:
		return _GP(play).messagetime;
	case 56:
		return _GP(play).usedinv;
	case 57:
		return _GP(play).inv_top;
	case 58:
		return _GP(play).inv_numdisp;
	case 59:
		return _GP(play).inv_numorder;
	case 60:
		return _GP(play).inv_numinline;
	case 61:
		return _GP(play).text_speed;
	case 62:
		return _GP(play).sierra_inv_color;
	case 63:
		return _GP(play).talkanim_speed;
	case 64:
		return _GP(play).inv_item_wid;
	case 65:
		return _GP(play).inv_item_hit;
	case 66:
		return _GP(play).speech_text_shadow;
	case 67:
		return _GP(play).swap_portrait_side;
	case 68:
		return _GP(play).speech_textwindow_gui;
	case 69:
		return _GP(play).follow_change_room_timer;
	case 70:
		return _GP(play).totalscore;
	case 71:
		return _GP(play).skip_display;
	case 72:
		return _GP(play).no_multiloop_repeat;
	case 73:
		return _GP(play).roomscript_finished;
	case 74:
		return _GP(play).used_inv_on;
	case 75:
		return _GP(play).no_textbg_when_voice;
	case 76:
		return _GP(play).max_dialogoption_width;
	case 77:
		return _GP(play).no_hicolor_fadein;
	case 78:
		return _GP(play).bgspeech_game_speed;
	case 79:
		return _GP(play).bgspeech_stay_on_display;
	case 80:
		return _GP(play).unfactor_speech_from_textlength;
	case 81:
		return _GP(play).mp3_loop_before_end;
	case 82:
		return _GP(play).speech_music_drop;
	case 83:
		return _GP(play).in_cutscene;
	case 84:
		return _GP(play).fast_forward;
	case 85:
		return _GP(play).room_width;
	case 86:
		return _GP(play).room_height;
	case 87:
		return _GP(play).game_speed_modifier;
	case 88:
		return _GP(play).score_sound;
	case 89:
		return _GP(play).takeover_data;
	case 90:
		return 0; // _GP(play).replay_hotkey
	case 91:
		return _GP(play).dialog_options_x;
	case 92:
		return _GP(play).dialog_options_y;
	case 93:
		return _GP(play).narrator_speech;
	case 94:
		return _GP(play).ambient_sounds_persist;
	case 95:
		return _GP(play).lipsync_speed;
	case 96:
		return _GP(play).close_mouth_speech_time;
	case 97:
		return _GP(play).disable_antialiasing;
	case 98:
		return _GP(play).text_speed_modifier;
	case 99:
		return _GP(play).text_align;
	case 100:
		return _GP(play).speech_bubble_width;
	case 101:
		return _GP(play).min_dialogoption_width;
	case 102:
		return _GP(play).disable_dialog_parser;
	case 103:
		return _GP(play).anim_background_speed;
	case 104:
		return _GP(play).top_bar_backcolor;
	case 105:
		return _GP(play).top_bar_textcolor;
	case 106:
		return _GP(play).top_bar_bordercolor;
	case 107:
		return _GP(play).top_bar_borderwidth;
	case 108:
		return _GP(play).top_bar_ypos;
	case 109:
		return _GP(play).screenshot_width;
	case 110:
		return _GP(play).screenshot_height;
	case 111:
		return _GP(play).top_bar_font;
	case 112:
		return _GP(play).speech_text_align;
	case 113:
		return _GP(play).auto_use_walkto_points;
	case 114:
		return _GP(play).inventory_greys_out;
	case 115:
		return _GP(play).skip_speech_specific_key;
	case 116:
		return _GP(play).abort_key;
	case 117:
		return _GP(play).fade_to_red;
	case 118:
		return _GP(play).fade_to_green;
	case 119:
		return _GP(play).fade_to_blue;
	case 120:
		return _GP(play).show_single_dialog_option;
	case 121:
		return _GP(play).keep_screen_during_instant_transition;
	case 122:
		return _GP(play).read_dialog_option_colour;
	case 123:
		return _GP(play).stop_dialog_at_end;
	case 124:
		return _GP(play).speech_portrait_placement;
	case 125:
		return _GP(play).speech_portrait_x;
	case 126:
		return _GP(play).speech_portrait_y;
	case 127:
		return _GP(play).speech_display_post_time_ms;
	case 128:
		return _GP(play).dialog_options_highlight_color;
	default:
		cc_error("ScriptGame: unsupported variable offset %d", offset);
		return 0;
	}
}

void CCScriptGame::WriteInt32(void *address, intptr_t offset, int32_t val) {
	const int index = offset / sizeof(int32_t);
	if (index >= 5 && index < 5 + MAXGLOBALVARS) {
		_GP(play).globalvars[index - 5] = val;
		return;
	}

	switch (index) {
	case 0:
		_GP(play).score = val;
		break;
	case 1:
		_GP(play).usedmode = val;
		break;
	case 2:
		_GP(play).disabled_user_interface = val;
		break;
	case 3:
		_GP(play).gscript_timer = val;
		break;
	case 4:
		set_debug_mode(val != 0);
		break; // _GP(play).debug_mode
	// 5 -> 54: _GP(play).globalvars
	case 55:
		_GP(play).messagetime = val;
		break;
	case 56:
		_GP(play).usedinv = val;
		break;
	case 57:
		_GP(play).inv_top = val;
		GUI::MarkInventoryForUpdate(_GP(game).playercharacter, true);
		break;
	case 58: // play.inv_numdisp
	case 59: // play.inv_numorder
	case 60: // play.inv_numinline
		debug_script_warn("ScriptGame: attempt to write in readonly variable at offset %d, value %d", offset, val);
		break;
	case 61:
		_GP(play).text_speed = val;
		break;
	case 62:
		_GP(play).sierra_inv_color = val;
		break;
	case 63:
		_GP(play).talkanim_speed = val;
		break;
	case 64:
		_GP(play).inv_item_wid = val;
		break;
	case 65:
		_GP(play).inv_item_hit = val;
		break;
	case 66:
		_GP(play).speech_text_shadow = val;
		break;
	case 67:
		_GP(play).swap_portrait_side = val;
		break;
	case 68:
		_GP(play).speech_textwindow_gui = val;
		break;
	case 69:
		_GP(play).follow_change_room_timer = val;
		break;
	case 70:
		_GP(play).totalscore = val;
		break;
	case 71:
		_GP(play).skip_display = val;
		break;
	case 72:
		_GP(play).no_multiloop_repeat = val;
		break;
	case 73:
		_GP(play).roomscript_finished = val;
		break;
	case 74:
		_GP(play).used_inv_on = val;
		break;
	case 75:
		_GP(play).no_textbg_when_voice = val;
		break;
	case 76:
		_GP(play).max_dialogoption_width = val;
		break;
	case 77:
		_GP(play).no_hicolor_fadein = val;
		break;
	case 78:
		_GP(play).bgspeech_game_speed = val;
		break;
	case 79:
		_GP(play).bgspeech_stay_on_display = val;
		break;
	case 80:
		_GP(play).unfactor_speech_from_textlength = val;
		break;
	case 81:
		_GP(play).mp3_loop_before_end = val;
		break;
	case 82:
		_GP(play).speech_music_drop = val;
		break;
	case 83: // _GP(play).in_cutscene
	case 84: // _GP(play).fast_forward;
	case 85: // _GP(play).room_width;
	case 86: // _GP(play).room_height;
		debug_script_warn("ScriptGame: attempt to write in readonly variable at offset %d, value %d", offset, val);
		break;
	case 87:
		_GP(play).game_speed_modifier = val;
		break;
	case 88:
		_GP(play).score_sound = val;
		break;
	case 89:
		_GP(play).takeover_data = val;
		break;
	case 90:
		break; // _GP(play).replay_hotkey
	case 91:
		_GP(play).dialog_options_x = val;
		break;
	case 92:
		_GP(play).dialog_options_y = val;
		break;
	case 93:
		_GP(play).narrator_speech = val;
		break;
	case 94:
		_GP(play).ambient_sounds_persist = val;
		break;
	case 95:
		_GP(play).lipsync_speed = val;
		break;
	case 96:
		_GP(play).close_mouth_speech_time = val;
		break;
	case 97:
		_GP(play).disable_antialiasing = val;
		break;
	case 98:
		_GP(play).text_speed_modifier = val;
		break;
	case 99:
		_GP(play).text_align = ReadScriptAlignment(val);
		break;
	case 100:
		_GP(play).speech_bubble_width = val;
		break;
	case 101:
		_GP(play).min_dialogoption_width = val;
		break;
	case 102:
		_GP(play).disable_dialog_parser = val;
		break;
	case 103:
		_GP(play).anim_background_speed = val;
		break;
	case 104:
		_GP(play).top_bar_backcolor = val;
		break;
	case 105:
		_GP(play).top_bar_textcolor = val;
		break;
	case 106:
		_GP(play).top_bar_bordercolor = val;
		break;
	case 107:
		_GP(play).top_bar_borderwidth = val;
		break;
	case 108:
		_GP(play).top_bar_ypos = val;
		break;
	case 109:
		_GP(play).screenshot_width = val;
		break;
	case 110:
		_GP(play).screenshot_height = val;
		break;
	case 111:
		_GP(play).top_bar_font = val;
		break;
	case 112:
		_GP(play).speech_text_align = ReadScriptAlignment(val);
		break;
	case 113:
		_GP(play).auto_use_walkto_points = val;
		break;
	case 114:
		_GP(play).inventory_greys_out = val;
		break;
	case 115:
		_GP(play).skip_speech_specific_key = val;
		break;
	case 116:
		_GP(play).abort_key = val;
		break;
	case 117: // _GP(play).fade_to_red;
	case 118: // _GP(play).fade_to_green;
	case 119: // _GP(play).fade_to_blue;
		debug_script_warn("ScriptGame: attempt to write in readonly variable at offset %d, value %d", offset, val);
		break;
	case 120:
		_GP(play).show_single_dialog_option = val;
		break;
	case 121:
		_GP(play).keep_screen_during_instant_transition = val;
		break;
	case 122:
		_GP(play).read_dialog_option_colour = val;
		break;
	case 123:
		_GP(play).stop_dialog_at_end = val;
		break;
	case 124:
		_GP(play).speech_portrait_placement = val;
		break;
	case 125:
		_GP(play).speech_portrait_x = val;
		break;
	case 126:
		_GP(play).speech_portrait_y = val;
		break;
	case 127:
		_GP(play).speech_display_post_time_ms = val;
		break;
	case 128:
		_GP(play).dialog_options_highlight_color = val;
		break;
	default:
		cc_error("ScriptGame: unsupported variable offset %d", offset);
		break;
	}
}

} // namespace AGS3
