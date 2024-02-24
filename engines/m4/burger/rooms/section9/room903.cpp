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

#include "common/config-manager.h"
#include "m4/burger/rooms/section9/room903.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_trigger.h"
#include "m4/graphics/gr_series.h"
#include "m4/burger/vars.h"
#include "m4/burger/other.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const MenuButtonDef BUTTONS[6] = {
	{ 295, 125, 615, 155, 0, 1, 2, 3, BTNSTATE_ENABLED, 5 },
	{ 295, 165, 615, 195, 12, 13, 14, 15, BTNSTATE_ENABLED, 6 },
	{ 295, 205, 615, 235, 4, 5, 6, 7, BTNSTATE_ENABLED, 7 },
	{ 295, 245, 615, 275, 16, 17, 18, 19, BTNSTATE_ENABLED, 8 },
	{ 295, 285, 615, 315, 8, 9, 10, 11, BTNSTATE_ENABLED, 9 },
	{ 295, 325, 615, 355, 20, 21, 22, 23, BTNSTATE_ENABLED, 10 },
};

void Room903::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room903::init() {
	MenuRoom::init();

	if (!ConfMan.getBool("seen_intro")) {
		ConfMan.setBool("seen_intro", true);
		ConfMan.flushToDisk();
	}

	player_set_commands_allowed(false);
	_buttonsDrawn = false;

	setButtons(BUTTONS, 6);
	series_show("903logo", 0, 0, -1, -1, 0, 100, 386, 20);

	if (_G(game).previous_room <= 0 || _G(game).previous_room == 951) {
		kernel_trigger_dispatch_now(11);

	} else {
		if (!digi_play_state(1))
			kernel_trigger_dispatch_now(1);

		kernel_trigger_dispatch_now(2);
		kernel_trigger_dispatch_now(14);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 100, 60, -1);
	}
}

void Room903::daemon() {
	if (_G(menu).menuSystemInitialized)
		return;

	switch (_G(kernel).trigger) {
	case 1:
		digi_preload("903music");
		digi_play("903music", 3, 100, -1);
		break;

	case 2:
		if (!_buttonsDrawn) {
			digi_preload("901click");
			drawButtons();

			if (!g_engine->savesExist()) {
				_buttons[2]._state = BTNSTATE_DISABLED;
				drawButton(2);
			}

			if (!g_engine->autosaveExists()) {
				_buttons[3]._state = BTNSTATE_DISABLED;
				drawButton(3);
			}

			_buttonsDrawn = true;
			kernel_timing_trigger(30, 3);
		}
		break;

	case 3:
	case 4:
		if (_G(kernel).trigger == 3)
			_G(kernel).call_daemon_every_loop = true;

		resetSelectedButton();
		player_set_commands_allowed(true);

		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._state != BTNSTATE_DISABLED)
				setButtonState(i, BTNSTATE_ENABLED);
		}
		break;

	case 5:
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 9005);
		break;

	case 6:
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 15);
		break;

	case 7:
		g_engine->showLoadScreen(M4Engine::kLoadFromMainMenu);
		break;

	case 8:
		other_resurrect_player();
		break;

	case 9:
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 9004);
		break;

	case 10:
		player_set_commands_allowed(false);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 16);
		break;

	case 11:
		if (!digi_play_state(1))
			digi_play("903_001", 2, 155, -1);

		pal_fade_set_start(_G(master_palette), 0);
		series_play("903d", 1792, 16, 12, 8, 0, 100, -5, 50, 0, -1);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, -1);
		break;

	case 12:
		kernel_timing_trigger(0, 13);
		break;

	case 13:
		kernel_trigger_dispatch_now(2);
		break;

	case 14:
		series_show("903d", 0, 0, -1, -1, 23);
		break;

	case 15:
		g_vars->initialize_game();
		_G(game).new_room = 801;
		_G(game).new_section = 8;
		break;

	case 16:
		// Quit game
		_G(kernel).going = false;
		break;

	default:
		MenuRoom::daemon();
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
