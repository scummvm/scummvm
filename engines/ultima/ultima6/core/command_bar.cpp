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

#include <cassert>
#include "SDL.h"

#include "Configuration.h"

#include "nuvieDefs.h"
#include "U6misc.h"
#include "Game.h"
#include "Screen.h"
#include "Event.h"
#include "TileManager.h"
#include "Font.h"
#include "FontManager.h"
#include "GameClock.h"
#include "GamePalette.h"
#include "CommandBar.h"
#include "Weather.h"
#include "Party.h"
#include "Player.h"
#include "Objlist.h"
#include "NuvieIO.h"
#include "SaveManager.h"
#include "U6Shape.h"
#include "MapWindow.h"
#include "GUI.h"

namespace Ultima {
namespace Ultima6 {

using std::string;

static Tile placeholder_tile = {
	0,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	0,
	//uint8 qty;
	//uint8 flags;

	0,
	0,
	0,

	{
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
	}
};

CommandBar::CommandBar() : GUI_Widget(NULL) {
	selected_action = -1;
}

CommandBar::CommandBar(Game *g) : GUI_Widget(NULL) {
	game = g;
	background = NULL;
	Weather *weather;
	uint16 x_off = game->get_game_x_offset();
	uint16 y_off =  game->get_game_y_offset();
	bool right_pos_cb;
	Configuration *cfg;

	if (!game->is_orig_style()) {
		cfg = game->get_config();
		std::string pos_str;
		cfg->value(config_get_game_key(cfg) + "/cb_position", pos_str, "default");
		if (pos_str == "default")
			right_pos_cb = !game->is_new_style();
		else
			right_pos_cb = pos_str != "left";
	}

	if (game->get_game_type() == NUVIE_GAME_U6) {
		offset = OBJLIST_OFFSET_U6_COMMAND_BAR;
		if (!game->is_orig_style()) {

			int value;
			cfg->value(config_get_game_key(cfg) + "/cb_text_color", value, 115); // light blue so that it stands out most of the time and isn't too bold
			font_color = value;
			y_off += game->get_game_height() - 29;
			if (right_pos_cb && (game->get_game_height() > 228 || game->is_new_style())) // bottom right
				Init(NULL, x_off + 159 + game->get_game_width() - 320, y_off, 0, 0);
			else // bottom left
				Init(NULL, x_off, y_off, 0, 0);
		} else {
			font_color = FONT_COLOR_U6_NORMAL;
			Init(NULL, 8 + x_off, 168 + y_off, 0, 0);
		}
		area.w = 16 * 10; // space for 10 icons
		area.h = 24 + 1; // extra space for the underlined default action
	} else if (game->get_game_type() == NUVIE_GAME_MD) {
		if (!game->is_orig_style()) {
			background = new U6Shape();
			background->load_WoU_background(game->get_config(), game->get_game_type());
			y_off += game->get_game_height() - 34;
			if (right_pos_cb && (game->get_game_height() > 233 || game->is_new_style()))
				Init(NULL, x_off + game->get_game_width() - 320 + 174, y_off, 146, 34);
			else
				Init(NULL, 16 + x_off, y_off - 3, 146, 34);
		} else
			Init(NULL, 16 + x_off, 163 + y_off, 146, 34);
		offset = OBJLIST_OFFSET_MD_COMMAND_BAR;
	} else { // SE
		if (!game->is_orig_style()) {
			background = new U6Shape();
			background->load_WoU_background(game->get_config(), game->get_game_type());
			y_off += game->get_game_height() - 22;
			if (right_pos_cb && (game->get_game_height() > 221 || game->is_new_style())) // bottom right
				Init(NULL, x_off + 156 + game->get_game_width() - 320, y_off, 163, 19);
			else
				Init(NULL, 8 + x_off, y_off, 1643, 19);
		} else
			Init(NULL, 8 + x_off, 178 + y_off, 163, 19);
		offset = OBJLIST_OFFSET_SE_COMMAND_BAR;
	}

	event = NULL; // it's not set yet
	font = game->get_font_manager()->get_font(0);

	weather = game->get_weather();

	selected_action = -1;
	combat_mode = false;
	wind = "?";

	bg_color = game->get_palette()->get_bg_color();
	if (game->get_game_type() == NUVIE_GAME_U6)
		init_buttons();

	weather->add_wind_change_notification_callback((CallBack *)this); //we want to know when the wind direction changes.
}

CommandBar::~CommandBar() {
	if (background)
		delete background;
}

bool CommandBar::init_buttons() {
	if (game->get_game_type() == NUVIE_GAME_U6) {
		TileManager *tile_man = game->get_tile_manager();
		for (uint32 i = 0; i < 9; i++)
			icon[i] = tile_man->get_tile(i + 400);
		// NOTE: combat button has two states
		icon[9] = tile_man->get_tile(combat_mode ? 415 : 414);
		icon[10] = tile_man->get_tile(409); //save icon used by CommandBarNewUI
		icon[11] = tile_man->get_tile(409); // quick save
		icon[12] = tile_man->get_tile(409); // quick load
	} else if (game->get_game_type() == NUVIE_GAME_MD) {
		icon[0] = &placeholder_tile; // attack
		icon[1] = &placeholder_tile; // talk
		icon[2] = &placeholder_tile; // look
		icon[3] = &placeholder_tile; // get
		icon[4] = &placeholder_tile; // drop
		icon[5] = &placeholder_tile; // move
		icon[6] = &placeholder_tile; // use
		icon[7] = &placeholder_tile; // combat mode
		icon[8] = &placeholder_tile; // load/save
		icon[9] = &placeholder_tile; // quick save
		icon[10] = &placeholder_tile; // quick load
	} else { // SE
		icon[0] = &placeholder_tile; // move
		icon[1] = &placeholder_tile; // get
		icon[2] = &placeholder_tile; // drop
		icon[3] = &placeholder_tile; // use
		icon[4] = &placeholder_tile; // talk
		icon[5] = &placeholder_tile; // look
		icon[6] = &placeholder_tile; // attack
		icon[7] = &placeholder_tile; // rest
		icon[8] = &placeholder_tile; // combat mode
		icon[9] = &placeholder_tile; // load/save
		icon[10] = &placeholder_tile; // quick save
		icon[11] = &placeholder_tile; // quick load
	}
	return (true);
}

bool CommandBar::load(NuvieIO *objlist) {
	objlist->seek(offset);
	uint8 action = objlist->read1();
	selected_action = (action == 0xff ? -1 : action - 0x81);

	sint8 max_action;
	if (game->get_game_type() == NUVIE_GAME_U6)
		max_action = 9;
	else if (game->get_game_type() == NUVIE_GAME_SE)
		max_action = 8;
	else // MD
		max_action = 7;

	if (selected_action > max_action || selected_action < 0)
		selected_action = -1;
	return true;
}

bool CommandBar::save(NuvieIO *objlist) {
	objlist->seek(offset);
	objlist->write1(selected_action > 0 ? selected_action + 0x81 : 0xff);

	return true;
}

void CommandBar::fill_square(uint8 pal_index) {
	screen->fill(pal_index, area.x + selected_action * 18, area.y, 19, 1); // top row
	screen->fill(pal_index, area.x + selected_action * 18, area.y + 18, 19, 1); // bottom row
	screen->fill(pal_index, area.x + selected_action * 18, area.y + 1, 1, 17); // left side
	screen->fill(pal_index, area.x + selected_action * 18 + 18, area.y + 1, 1, 17); // right side
}

void CommandBar::select_action(sint8 activate) {
	if (!game->is_new_style() && game->get_game_type() == NUVIE_GAME_SE) // black out previous setting
		fill_square(0);
	if (selected_action == activate) // clear if already selected
		set_selected_action(-1);
	else
		set_selected_action(activate);
}

GUI_status CommandBar::MouseDown(int x, int y, int button) {
	x -= area.x;
	y -= area.y;

	if (game->get_game_type() != NUVIE_GAME_U6 ||
	        (y >= 8 && y <= 24)) {
		uint8 activate = x / 16; // icon selected
		if (game->get_game_type() == NUVIE_GAME_SE)
			activate = x / 18;
		else if (game->get_game_type() == NUVIE_GAME_MD) {
			activate = (x) / 18;
			if (activate > 7)
				activate = 7;
		}
		if (button == COMMANDBAR_USE_BUTTON)
			return (hit(activate));
		else if (button == COMMANDBAR_ACTION_BUTTON) {
			select_action(activate);
		}
	} else if (!game->is_orig_style())
		return GUI_PASS;
	return (GUI_YUM);
}

GUI_status CommandBar::hit(uint8 num) {
	if (!event) event = game->get_event();

	if (event->get_mode() != MOVE_MODE && event->get_mode() != EQUIP_MODE)
		return GUI_PASS;

	try_selected_action(num);

	return (GUI_YUM);
}

static const EventMode U6_mode_tbl[] = { ATTACK_MODE, CAST_MODE, TALK_MODE, LOOK_MODE, GET_MODE,
                                         DROP_MODE, PUSH_MODE, USE_MODE, REST_MODE, COMBAT_MODE
                                       };
static const EventMode MD_mode_tbl[] = { ATTACK_MODE, TALK_MODE, LOOK_MODE, GET_MODE,
                                         DROP_MODE, PUSH_MODE, USE_MODE, COMBAT_MODE
                                       };
static const EventMode SE_mode_tbl[] = { PUSH_MODE, GET_MODE, DROP_MODE, USE_MODE, TALK_MODE,
                                         LOOK_MODE, ATTACK_MODE, REST_MODE, COMBAT_MODE
                                       };

/*
 * return true if target is needed (only used for original CommandBar commands)
 */
bool CommandBar::try_selected_action(sint8 command_num) {
	if (!event) event = game->get_event();

	if (command_num == -1)
		command_num = selected_action;

	if (command_num == -1) // might happen if changing selected action when in EQUIP_MODE
		return false;

	EventMode mode;

	sint8 save_num, quick_save_num, quick_load_num;
	if (game->get_game_type() == NUVIE_GAME_U6) {
		save_num = 10;
		quick_save_num = 11;
		quick_load_num = 12;
	} else if (game->get_game_type() == NUVIE_GAME_MD) {
		save_num = 8;
		quick_save_num = 9;
		quick_load_num = 10;
	} else { // SE
		save_num = 9;
		quick_save_num = 10;
		quick_load_num = 11;
	}

// CommandBarNewUI only commands
	if (command_num == save_num) {
		event->saveDialog();
		return false;
	} else if (command_num == quick_save_num)
		return game->get_save_manager()->quick_save(0, QUICK_SAVE);
	else if (command_num == quick_load_num)
		return game->get_save_manager()->quick_save(0, QUICK_LOAD);
	else if (command_num >= save_num)
		return false;

// original CommandBar commands (also used in CommandBarNewUI)
	if (game->get_game_type() == NUVIE_GAME_U6)
		mode = U6_mode_tbl[command_num];
	else if (game->get_game_type() == NUVIE_GAME_MD)
		mode = MD_mode_tbl[command_num];
	else // SE
		mode = SE_mode_tbl[command_num];

	switch (mode) {
	case CAST_MODE:
	case GET_MODE:
	case DROP_MODE:
	case PUSH_MODE:
		if (game->get_player()->is_in_vehicle()) {
			event->display_not_aboard_vehicle();
			return false;
		}
		break;
	default:
		break;
	}

	event->newAction(mode);

	if (mode < REST_MODE) // needs target
		return true;
	else
		return false;
}

void CommandBar::set_combat_mode(bool mode) {
	TileManager *tile_man = game->get_tile_manager();
	if (combat_mode != mode) {
		combat_mode = mode;
		if (game->get_game_type() == NUVIE_GAME_U6) {
			icon[9] = tile_man->get_tile(combat_mode ? 415 : 414);
			update_display = true;
		}
	}

	//FIXME this probably should be moved else where. I think this was added to get party to fight first. It may not be needed now.
	if (combat_mode) { // make sure party attacks
		Player *player = game->get_player();
		Party *party = player->get_party();
		party->follow(0, 0);
	}
}

void CommandBar::Display(bool full_redraw) {
	Screen *screen = game->get_screen();

	if (full_redraw || update_display || !game->is_orig_style()) {
		update_display = false;
		if (game->get_game_type() == NUVIE_GAME_U6) {
			if (game->is_orig_style())
				screen->fill(bg_color, area.x, area.y, area.w, area.h);
			else if (game->is_original_plus_cutoff_map() && area.x != game->get_game_x_offset()) // over null background so clear area where text is displayed
				screen->clear(area.x + 2, area.y, area.w - 2, area.h - 16, NULL);

			display_information();
			for (uint32 i = 0; i < 10; i++)
				screen->blit(area.x + i * 16, area.y + 8, icon[i]->data, 8, 16, 16, 16);

			if (selected_action >= 0 && selected_action <= 9)
				screen->fill(9, area.x + selected_action * 16, area.y + 24, 16, 1);
		} else if (game->get_game_type() == NUVIE_GAME_SE) {
			if (!game->is_orig_style()) {
				unsigned char *se_ptr = background->get_data();
				se_ptr += ((320 * 178) + 8); // ((bg_w * image_y_off)  + image_x_off)
				screen->blit(area.x, area.y, se_ptr, 8, 163, 19, 320, true); // drawing command bar icons from background
			}
			if (selected_action >= 0 && selected_action <= 8)
				fill_square(6);
		} else { // MD
			if (!game->is_orig_style()) {
				unsigned char *md_bg_ptr = background->get_data();
				md_bg_ptr += ((320 * 163) + 15); // ((bg_w * image_y_off)  + image_x_off)
				screen->fill(0, area.x, area.y, area.w, area.h); // lever slots, text, top, and bottom have transparency so we need to fill in black first
				screen->blit(area.x, area.y, md_bg_ptr, 8, area.w, area.h, 320, true); // drawing command bar icons from background
				screen->fill(0, area.x, area.y, 1, area.h); // make left black so it looks better
				screen->fill(0, area.x + area.w - 1, area.y, 1, area.h); // make right black so it looks better
			}
			// FIXME code to display the switched levers goes here (the selected action and the current action will be have the lever down)
		}

		screen->update(area.x, area.y, area.w, area.h);
	}
}

void CommandBar::display_information() {
	string infostring(game->get_clock()->get_date_string());
	infostring += " Wind:";
	infostring += wind;
	font->drawString(screen, infostring.c_str(), area.x + 8, area.y, font_color, font_color);
}

uint16 CommandBar::callback(uint16 msg, CallBack *caller, void *data) {
	Weather *weather = game->get_weather();

	if (caller == (CallBack *)weather && msg == WEATHER_CB_CHANGE_WIND_DIR) {
		wind = weather->get_wind_dir_str();
		update_display = true;
	}

	return 1;
}

bool CommandBar::drag_accept_drop(int x, int y, int message, void *data) {
	GUI::get_gui()->force_full_redraw();
	DEBUG(0, LEVEL_DEBUGGING, "CommandBar::drag_accept_drop()\n");
	if (game->get_game_type() == NUVIE_GAME_U6 && !Game::get_game()->is_orig_style()
	        && message == GUI_DRAG_OBJ) {
		if (y < area.y + 8) // over text
			return Game::get_game()->get_map_window()->drag_accept_drop(x, y, message, data);
	}
	return false;
}

void CommandBar::drag_perform_drop(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "CommandBar::drag_perform_drop()\n");
	if (message == GUI_DRAG_OBJ) // should only happen with !orig_style in U6
		Game::get_game()->get_map_window()->drag_perform_drop(x, y, message, data);
}

} // End of namespace Ultima6
} // End of namespace Ultima
