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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/burger/rooms/section9/room901.h"
#include "m4/graphics/gr_series.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum {
	MENU_SHOW = 1,
	MENU_RESTORE = 2,
	MENU_3 = 3,
	MENU_4 = 4,
	MENU_5 = 5,
	MENU_RESTARTING = 6,
	MENU_RESTART = 7,
	MENU_8 = 8
};

static const MenuButtonDef DEMO_BUTTONS[4] = {
	{ 337, 82, 622, 140, 0, 1, 2, 3, BTNSTATE_ENABLED, 3 },
	{ 337, 138, 622, 197, 4, 5, 6, 7, BTNSTATE_ENABLED, 6 },
	{ 337, 198, 622, 256, 8, 9, 10, 11, BTNSTATE_ENABLED, 5 },
	{ 337, 260, 622, 317, 12, 13, 14, 15, BTNSTATE_ENABLED, 9 }
};

static const MenuButtonDef GAME_BUTTONS[2] = {
	{ 337, 82, 622, 140, 4, 5, 6, 7, BTNSTATE_ENABLED, 6 },
	{ 337, 138, 622, 197, 12, 13, 14, 15, BTNSTATE_ENABLED, 9 }
};

void Room901::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room901::init() {
	MenuRoom::init();

	switch (_G(executing)) {
	case JUST_OVERVIEW:
	case INTERACTIVE_DEMO:
	case MAGAZINE_DEMO:
		setButtons(DEMO_BUTTONS, 4);
		series_play("901order", 0, 0, -1, 60, -1, 100, 165, 395, 0, -1);
		break;

	case WHOLE_GAME:
		setButtons(GAME_BUTTONS, 2);
		series_play("901order", 0, 0, -1, 60, -1, 100, 470, 245, 0, -1);
		break;
	}

	_G(kernel).suppress_fadeup = true;
	pal_fade_set_start(_G(master_palette), 0);
	pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 100, 60, MENU_SHOW);

	drawButtons();
}

void Room901::daemon() {
	switch (_G(kernel).trigger) {
	case MENU_SHOW:
	case MENU_RESTORE:
		if (_G(kernel).trigger == MENU_SHOW)
			_G(kernel).call_daemon_every_loop = true;

		resetSelectedButton();
		player_set_commands_allowed(true);

		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._state != BTNSTATE_DISABLED)
				setButtonState(i, BTNSTATE_ENABLED);
		}
		break;

	case MENU_3:
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 9005);
		break;

	case MENU_4:
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 9006);
		break;

	case MENU_5:
	case MENU_8:
		_G(room902Flag) = _G(kernel).trigger == 8;
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 9002);
		break;

	case MENU_RESTARTING:
		g_vars->initialize_game();
		conv_reset_all();
		_G(flags).reset2();

		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, MENU_RESTART);
		break;

	case MENU_RESTART:
		_G(game).setRoom(601);
		break;

	default:
		MenuRoom::daemon();
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
