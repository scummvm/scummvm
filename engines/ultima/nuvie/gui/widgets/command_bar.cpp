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
#include "ultima/nuvie/gui/widgets/command_bar.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/save/obj_list.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/files/u6_shape.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/nuvie.h"

namespace Ultima {
namespace Nuvie {

using Std::string;

static const Tile placeholder_tile = {
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

CommandBar::CommandBar() : GUI_Widget(nullptr), game(nullptr), event(nullptr),
		background(nullptr), font(nullptr), selected_action(-1), offset(0),
		combat_mode(false), bg_color(0), font_color(0), active_action_num(-1),
		lever_up(nullptr), lever_down(nullptr) {
	for (int i = 0; i < ARRAYSIZE(icon); i++)
		icon[i] = nullptr;
}

CommandBar::CommandBar(Game *g) : GUI_Widget(nullptr), game(g),
		background(nullptr), combat_mode(false), bg_color(0), active_action_num(-1),
		font_color(0), lever_up(nullptr), lever_down(nullptr) {
	Weather *weather;
	uint16 x_off = game->get_game_x_offset();
	uint16 y_off =  game->get_game_y_offset();
	bool right_pos_cb = false;
	const Configuration *cfg = nullptr;

	if (!game->is_orig_style()) {
		cfg = game->get_config();
		Std::string pos_str;
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
				Init(nullptr, x_off + 159 + game->get_game_width() - 320, y_off, 0, 0);
			else // bottom left
				Init(nullptr, x_off, y_off, 0, 0);
		} else {
			font_color = FONT_COLOR_U6_NORMAL;
			Init(nullptr, 8 + x_off, 168 + y_off, 0, 0);
		}
		area.setWidth(16 * 10); // space for 10 icons
		area.setHeight(24 + 1); // extra space for the underlined default action
	} else if (game->get_game_type() == NUVIE_GAME_MD) {
		if (!game->is_orig_style()) {
			background = new U6Shape();
			background->load_WoU_background(game->get_config(), game->get_game_type());
			y_off += game->get_game_height() - 34;
			if (right_pos_cb && (game->get_game_height() > 233 || game->is_new_style()))
				Init(nullptr, x_off + game->get_game_width() - 320 + 174, y_off, 146, 34);
			else
				Init(nullptr, 16 + x_off, y_off - 3, 146, 34);
		} else
			Init(nullptr, 16 + x_off, 163 + y_off, 146, 34);
		offset = OBJLIST_OFFSET_MD_COMMAND_BAR;
	} else { // SE
		if (!game->is_orig_style()) {
			background = new U6Shape();
			background->load_WoU_background(game->get_config(), game->get_game_type());
			y_off += game->get_game_height() - 22;
			if (right_pos_cb && (game->get_game_height() > 221 || game->is_new_style())) // bottom right
				Init(nullptr, x_off + 156 + game->get_game_width() - 320, y_off, 163, 19);
			else
				Init(nullptr, 8 + x_off, y_off, 1643, 19);
		} else
			Init(nullptr, 8 + x_off, 178 + y_off, 163, 19);
		offset = OBJLIST_OFFSET_SE_COMMAND_BAR;
	}

	event = nullptr; // it's not set yet
	font = game->get_font_manager()->get_font(0);

	weather = game->get_weather();

	selected_action = -1;
	combat_mode = false;
	wind = "?";

	bg_color = game->get_palette()->get_bg_color();
	init_buttons();

	weather->add_wind_change_notification_callback((CallBack *)this); //we want to know when the wind direction changes.
}

CommandBar::~CommandBar() {
	if (background)
		delete background;
	if (lever_up)
		delete lever_up;
	if (lever_down)
		delete lever_down;
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

		Common::Path filename;
		Configuration *config = Game::get_game()->get_config();
		config_get_path(config, "mdscreen.lzc", filename);
		lever_up = new U6Shape();
		lever_down = new U6Shape();
		lever_up->load_from_lzc(filename, 2, 1);
		lever_down->load_from_lzc(filename, 2, 0);
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
	return true;
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
	screen->fill(pal_index, area.left + selected_action * 18, area.top, 19, 1); // top row
	screen->fill(pal_index, area.left + selected_action * 18, area.top + 18, 19, 1); // bottom row
	screen->fill(pal_index, area.left + selected_action * 18, area.top + 1, 1, 17); // left side
	screen->fill(pal_index, area.left + selected_action * 18 + 18, area.top + 1, 1, 17); // right side
}

void CommandBar::select_action(sint8 activate) {
	if (!game->is_new_style() && game->get_game_type() == NUVIE_GAME_SE) // black out previous setting
		fill_square(0);
	if (selected_action == activate) // clear if already selected
		set_selected_action(-1);
	else
		set_selected_action(activate);
}

GUI_status CommandBar::MouseDown(int x, int y, Shared::MouseButton button) {
	x -= area.left;
	y -= area.top;

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
			return hit(activate);
		else if (button == COMMANDBAR_ACTION_BUTTON) {
			select_action(activate);
		}
	} else if (!game->is_orig_style())
		return GUI_PASS;
	return GUI_YUM;
}

GUI_status CommandBar::hit(uint8 num) {
	if (!event) event = game->get_event();

	if (event->get_mode() != MOVE_MODE && event->get_mode() != EQUIP_MODE)
		return GUI_PASS;

	try_selected_action(num);

	return GUI_YUM;
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
		g_engine->openMainMenuDialog();
		return false;
	} else if (command_num == quick_save_num)
		return g_engine->quickSave(0, true);
	else if (command_num == quick_load_num)
		return g_engine->quickSave(0, false);
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
	Screen *scr = game->get_screen();

	if (full_redraw || update_display || !game->is_orig_style()) {
		update_display = false;
		if (game->get_game_type() == NUVIE_GAME_U6) {
			if (game->is_orig_style())
				scr->fill(bg_color, area.left, area.top, area.width(), area.height());
			else if (game->is_original_plus_cutoff_map() && area.left != game->get_game_x_offset()) // over null background so clear area where text is displayed
				scr->clear(area.left + 2, area.top, area.width() - 2, area.height() - 16, nullptr);

			display_information();
			for (uint32 i = 0; i < 10; i++)
				scr->blit(area.left + i * 16, area.top + 8, icon[i]->data, 8, 16, 16, 16);

			if (selected_action >= 0 && selected_action <= 9)
				scr->fill(9, area.left + selected_action * 16, area.top + 24, 16, 1);
		} else if (game->get_game_type() == NUVIE_GAME_SE) {
			if (!game->is_orig_style()) {
				const unsigned char *se_ptr = background->get_data();
				se_ptr += ((320 * 178) + 8); // ((bg_w * image_y_off)  + image_x_off)
				scr->blit(area.left, area.top, se_ptr, 8, 163, 19, 320, true); // drawing command bar icons from background
			}
			if (selected_action >= 0 && selected_action <= 8)
				fill_square(6);
		} else { // MD
			if (!game->is_orig_style()) {
				const unsigned char *md_bg_ptr = background->get_data();
				md_bg_ptr += ((320 * 163) + 15); // ((bg_w * image_y_off)  + image_x_off)
				scr->fill(0, area.left, area.top, area.width(), area.height()); // lever slots, text, top, and bottom have transparency so we need to fill in black first
				scr->blit(area.left, area.top, md_bg_ptr, 8, area.width(), area.height(), 320, true); // drawing command bar icons from background
				scr->fill(0, area.left, area.top, 1, area.height()); // make left black so it looks better
				scr->fill(0, area.left + area.width() - 1, area.top, 1, area.height()); // make right black so it looks better
			}

			//
			// Display the switched levers. Original MD has lever down for:
			// * the right-click action (left click is move)
			// * a current action pressed via lever or keyboard (eg, drop)
			// * combat lever if in combat mode
			//
			// TODO: Switch the right-click action down to match original.
			//
			const U6Shape *lever;
			uint16 w, h;
			lever_up->get_size(&w, &h);
			for (int i = 0; i < 7; i++) {
				lever = (i == active_action_num) ? lever_down : lever_up;
				scr->blit(area.left + 18 * i + 6, area.top + 6, lever->get_data(), 8, w, h, w);
			}
			lever = (combat_mode ? lever_down : lever_up);
			scr->blit(area.left + 18 * 7 + 6, area.top + 6, lever->get_data(), 8, w, h, w);
		}

		scr->update(area.left, area.top, area.width(), area.height());
	}
}

void CommandBar::display_information() {
	string infostring(game->get_clock()->get_date_string());
	infostring += " Wind:";
	infostring += wind;
	font->drawString(screen, infostring.c_str(), area.left + 8, area.top, font_color, font_color);
}

void CommandBar::on_new_action(EventMode action) {
	// This is only really needed on MD, but keep track on others too.
	const EventMode *modetbl;
	int modetblsz;
	if (game->get_game_type() == NUVIE_GAME_U6) {
		modetbl = U6_mode_tbl;
		modetblsz = ARRAYSIZE(U6_mode_tbl);
	} else if (game->get_game_type() == NUVIE_GAME_MD) {
		modetbl = MD_mode_tbl;
		modetblsz = ARRAYSIZE(MD_mode_tbl);
	} else { // SE
		modetbl = SE_mode_tbl;
		modetblsz = ARRAYSIZE(SE_mode_tbl);
	}

	active_action_num = -1;
	for (int i = 0; i < modetblsz; i++) {
		if (action == modetbl[i])
			active_action_num = i;
	}
	update_display = true;
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
		if (y < area.top + 8) // over text
			return Game::get_game()->get_map_window()->drag_accept_drop(x, y, message, data);
	}
	return false;
}

void CommandBar::drag_perform_drop(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "CommandBar::drag_perform_drop()\n");
	if (message == GUI_DRAG_OBJ) // should only happen with !orig_style in U6
		Game::get_game()->get_map_window()->drag_perform_drop(x, y, message, data);
}

} // End of namespace Nuvie
} // End of namespace Ultima
