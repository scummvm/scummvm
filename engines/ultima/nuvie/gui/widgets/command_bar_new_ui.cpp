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

#include "ultima/nuvie/conf/configuration.h"

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/core/tile_manager.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/fonts/font_manager.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/screen/game_palette.h"
#include "ultima/nuvie/gui/widgets/command_bar_new_ui.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/save/obj_list.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/gui/widgets/background.h"
#include "ultima/nuvie/keybinding/keys.h"
#include "common/events.h"

namespace Ultima {
namespace Nuvie {

using Std::string;

#define btn_size 17
#define SELECTED_COLOR 248
#define table_size_U6 13
#define table_size_SE 12
#define table_size_MD 11

static const char *U6_mode_name_tbl[table_size_U6] = {"Attack", "Cast", "Talk", "Look", "Get", "Drop", "Move", "Use", "Rest", "Combat mode", "Load/Save", "Quick save", "Quick load"};
static const char *SE_mode_name_tbl[table_size_SE] = {"Move", "Get", "Drop", "Use", "Talk", "Look", "Attack", "Rest", "Combat mode", "Load/Save", "Quick save", "Quick load"};
static const char *MD_mode_name_tbl[table_size_MD] = {"Attack", "Talk", "Look", "Get", "Drop", "Move", "Use", "Combat mode", "Load/Save", "Quick save", "Quick load"};
static const char *mode_name_tbl[table_size_U6];

CommandBarNewUI::CommandBarNewUI(Game *g) : CommandBar() {
	game = g;
	background = NULL;
	Weather *weather;
	uint16 x_off = game->get_game_x_offset();
	uint16 y_off = game->get_game_y_offset();

	icon_w = 5;
	icon_h = 3;
	uint8 text_height;
	uint16 map_width;
	uint16 map_height;

	offset = OBJLIST_OFFSET_U6_COMMAND_BAR;

	if (game->get_game_type() == NUVIE_GAME_U6) {
		num_icons = table_size_U6;
		for (uint8 i = 0; i < table_size_U6; i++)
			mode_name_tbl[i] = U6_mode_name_tbl[i];
	} else if (game->get_game_type() == NUVIE_GAME_SE) {
		num_icons = table_size_SE;
		for (uint8 i = 0; i < table_size_SE; i++)
			mode_name_tbl[i] = SE_mode_name_tbl[i];
	} else { // MD
		num_icons = table_size_MD;
		for (uint8 i = 0; i < table_size_MD; i++)
			mode_name_tbl[i] = MD_mode_name_tbl[i];
	}

	if (game->is_orig_style()) {
		text_height = 8;
		icon_y_offset = 0;
		map_width = 176;
		map_height = 176;
	} else {
		if (game->get_game_type() == NUVIE_GAME_U6) {
			text_height = 17;
			icon_y_offset = 9;
		} else {
			text_height = 8;
			icon_y_offset = 0;
		}
		if (game->is_original_plus())
			map_width = game->get_game_width() - game->get_background()->get_border_width();
		else
			map_width = game->get_game_width();
		map_height = game->get_game_height();
	}
	uint8 command_width = btn_size * icon_w;
	uint8 command_height = btn_size * icon_h + text_height;

	Init(NULL, (map_width - command_width) / 2 + x_off, (map_height - command_height) / 2 + y_off, 0, 0);
	area.setWidth(command_width); // space for 5x3 icons
	area.setHeight(command_height);

	event = NULL; // it's not set yet

	weather = game->get_weather();

	selected_action = -1;
	combat_mode = false;
	wind = weather->get_wind_dir_str();

	bg_color = game->get_palette()->get_bg_color();

	init_buttons();
	if (game->get_game_type() == NUVIE_GAME_U6 && !game->is_orig_style())
		weather->add_wind_change_notification_callback((CallBack *)this); //we want to know when the wind direction changes.

	cur_pos = 0;

	font = game->get_font_manager()->get_conv_font();
}

CommandBarNewUI::~CommandBarNewUI() {
}



GUI_status CommandBarNewUI::MouseDown(int x, int y, Shared::MouseButton button) {
	if (HitRect(x, y)) {
		x -= area.left;
		y -= area.right;

		if (y >= icon_y_offset) {
			uint8 pos = ((y - icon_y_offset) / btn_size) * icon_w;
			pos += x / btn_size;

			if (pos < num_icons) {
				cur_pos = pos;
				hit((sint8)cur_pos);
				Game::get_game()->get_keybinder()->set_enable_joy_repeat(true);
				Hide();
			}
		}
	}

	return (GUI_YUM);
}

GUI_status CommandBarNewUI::MouseUp(int x, int y, Shared::MouseButton button) {
	/*
	    if(HitRect(x, y))
	    {
	        x -= area.left;
	        y -= area.top;

	        if(y >= icon_y_offset && y < icon_y_offset + icon_h * btn_size)
	        {
	            hit((sint8)cur_pos);
	            Hide();
	        }
	    }
	*/
	return (GUI_YUM);
}

GUI_status CommandBarNewUI::KeyDown(const Common::KeyState &key) {
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case NORTH_KEY:
		do {
			if (cur_pos - icon_w < 0)
				cur_pos = icon_w * icon_h - (icon_w - cur_pos % icon_w);
			else
				cur_pos -= icon_w;
		} while (cur_pos >= num_icons);
		break;
	case SOUTH_KEY:
		do {
			cur_pos = (cur_pos + icon_w) % (icon_w * icon_h);
		} while (cur_pos >= num_icons);
		break;
	case WEST_KEY:
		do {
			if (cur_pos % icon_w == 0)
				cur_pos = (cur_pos / icon_w) * icon_w + icon_w - 1;
			else
				cur_pos--;
		} while (cur_pos >= num_icons);
		break;
	case EAST_KEY:
		do {
			cur_pos = (cur_pos / icon_w) * icon_w + (cur_pos + 1) % icon_w;
		} while (cur_pos >= num_icons);
		break;
	case DO_ACTION_KEY:
		if (cur_pos < num_icons) {
			hit((sint8)cur_pos);
			keybinder->set_enable_joy_repeat(true);
			Hide();
		}
		break;
	case CANCEL_ACTION_KEY:
	case NEW_COMMAND_BAR_KEY:
		keybinder->set_enable_joy_repeat(true);
		Hide();
		break;

	default :
		keybinder->handle_always_available_keys(a);
		break;
	}

	return GUI_YUM;
}

void CommandBarNewUI::Display(bool full_redraw) {
	Screen *scr = game->get_screen();

	//if(full_redraw || update_display)
	// {
	update_display = false;
	if (game->get_game_type() == NUVIE_GAME_U6) {
		//scr->fill(bg_color, area.left, area.top, area.width(), area.height());
		if (!game->is_orig_style()) {
			//display_information();
			string infostring(game->get_clock()->get_date_string());
			infostring += " Wind:";
			infostring += wind;
			font->drawString(scr, infostring.c_str(), area.left - 13, area.top); // sort of center
		}
	}
	uint8 i = 0;
	for (uint8 y = 0; y < icon_h; y++) {
		for (uint8 x = 0; x < icon_w && i < num_icons; x++, i++) {
			scr->blit(area.left + x * btn_size, icon_y_offset + area.top + y * btn_size, icon[i]->data, 8, 16, 16, 16);
			if (i == cur_pos) {
				scr->stipple_8bit(SELECTED_COLOR, area.left + x * btn_size, icon_y_offset + area.top + y * btn_size, 16, 16);
			}
		}
	}
	if (game->get_game_type() == NUVIE_GAME_U6) { // FIXME use new icon instead
		font->drawString(scr, "QS", area.left + 2 + btn_size, icon_y_offset + area.top + 2 * btn_size + 4);
		font->drawString(scr, "QL", area.left + 2 + 2 * btn_size, icon_y_offset + area.top + 2 * btn_size + 4);
	}
	font->drawString(scr, get_command_name(cur_pos), area.left, area.top + icon_y_offset + icon_h * btn_size);
	if (game->get_game_type() == NUVIE_GAME_U6 && !game->is_orig_style())
		scr->update(area.left - 13, area.top, area.width() + 26, area.height()); // need to have edges of text update
	else
		scr->update(area.left, area.top, area.width(), area.height());
	//  }
}

const char *CommandBarNewUI::get_command_name(sint8 command_num) {
	if (command_num < 0 || command_num >= num_icons)
		return "";

	return mode_name_tbl[command_num];
}

/*
void CommandBarNewUI::display_information()
{
    string infostring(game->get_clock()->get_date_string());
    infostring += " Wind:";
    infostring += wind;
    text->drawString(screen, infostring.c_str(), area.left + 8, area.top, 0);
}
*/

} // End of namespace Nuvie
} // End of namespace Ultima
