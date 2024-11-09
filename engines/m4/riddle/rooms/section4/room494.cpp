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

#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/rooms/section4/room494.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/walker.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room494::preload() {
	_G(kernel).letter_box_y = LETTERBOX_Y;
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room494::init() {
	midi_stop();
	mouse_show();

	_hotkeyEscape = GetSystemHotkey(KEY_ESCAPE);
	_hotkeySave = GetSystemHotkey(KEY_F2);
	_hotkeyLoad = GetSystemHotkey(KEY_F3);

	RemoveSystemHotkey(KEY_F2);
	AddSystemHotkey(KEY_ESCAPE, escapeFn);
	AddSystemHotkey(KEY_F3, escapeFn);
	_machine1 = _machine2 = 0;
	_selectedBtn1 = _selectedBtn2 = -1;
	midi_stop();

	digi_preload("gallery", 950);
	digi_play_loop("gallery", 1, 255, -1, 950);
	interface_hide();
	_G(kernel).call_daemon_every_loop = true;

	_menuButtons = series_load("494 MENU BUTTONS");
	_buttons[0] = series_show_sprite("494 MENU BUTTONS", 0, 0xf00);
	_buttons[1] = series_show_sprite("494 MENU BUTTONS", 3, 0xf00);
	_buttons[2] = series_show_sprite("494 MENU BUTTONS", 6, 0xf00);
	_buttons[3] = series_show_sprite("494 MENU BUTTONS", 9, 0xf00);
	_buttons[4] = series_show_sprite("494 MENU BUTTONS", 12, 0xf00);
	_buttons[5] = series_show_sprite("494 MENU BUTTONS", 15, 0xf00);
}

void Room494::daemon() {
	int selectedBtn = getSelectedButton();
	bool btnClicked = false;

	switch (_G(kernel).trigger) {
	case 111:
		if (g_engine->autosaveExists()) {
			restoreHotkeys();
			restoreAutosave();

		} else {
			_G(flags)[V001] = 1;
			digi_stop(1);
			_G(game).setRoom(901);
		}
		break;

	case 304:
		restoreHotkeys();
		conv_reset_all();
		player_reset_been();

		_G(flags)[V001] = 1;
		interface_show();
		digi_stop(1);
		_G(game).setRoom(304);
		break;

	case 493:
		restoreHotkeys();
		adv_kill_digi_between_rooms(false);
		_G(game).setRoom(493);
		break;

	case 666:
		_G(kernel).going = false;
		break;

	case 901:
		restoreHotkeys();

		_G(flags)[V001] = 1;
		digi_stop(1);
		_G(game).setRoom(901);
		break;

	case 999:
		interface_hide();
		break;

	default:
		break;
	}

	if (_G(MouseState).ButtonState) {
		if (_selectState) {
			if (selectedBtn == _selectedBtn2 && _machine2) {
				terminateMachineAndNull(_machine2);
				_selectedBtn2 = -1;
				digi_play("950_s51", 2);
			}

			if (!_machine2) {
				_machine2 = series_show_sprite("494 menu buttons",
					selectedBtn * 3 + 2, 0x100);
				_selectedBtn2 = selectedBtn;
				digi_play("950_s51", 2);
			}
		} else {
			if (_machine1) {
				terminateMachineAndNull(_machine1);
				_selectedBtn1 = -1;
			}

			if (selectedBtn != -1) {
				_machine2 = series_show_sprite("494 menu buttons",
					selectedBtn * 3 + 2, 0x100);
				_selectedBtn2 = selectedBtn;
				digi_play("950_s51", 2);
			}
		}

		_selectState = true;

	} else if (_selectState) {
		if (_machine2) {
			terminateMachineAndNull(_machine2);
			_selectedBtn2 = -1;
		}

		_selectState = 0;
		btnClicked = true;
	}

	if (selectedBtn != -1) {
		if (_selectedBtn1 != -1 && selectedBtn != _selectedBtn1) {
			terminateMachineAndNull(_machine1);
			_selectedBtn1 = -1;
		}

		if (_selectedBtn1 == -1 && !_machine2) {
			_selectedBtn1 = selectedBtn;
			_machine1 = series_show_sprite("494 menu buttons",
				selectedBtn * 3 + 1, 0x100);
		}
	} else {
		if (selectedBtn != _selectedBtn1) {
			terminateMachineAndNull(_machine1);
			_selectedBtn1 = selectedBtn;
		}
	}

	if (btnClicked) {
		switch (selectedBtn) {
		case 0:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(901);
			break;
		case 1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(304);
			break;
		case 2:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(111);
			break;
		case 3:
			escapeFn(nullptr, nullptr);
			break;
		case 4:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(493);
			break;
		case 5:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(666);
			break;
		default:
			break;
		}
	}
}

void Room494::pre_parser() {
	_G(player).need_to_walk = false;
	_G(player).ready_to_walk = true;
	_G(player).waiting_for_walk = false;
}

void Room494::escapeFn(void *, void *) {
	warning("TODO: room 494 escapeFn");
}

int Room494::getSelectedButton() const {
	static const Common::Rect BUTTONS[6] = {
		{  91, 323, 150, 370 },
		{ 174, 323, 226, 370 },
		{ 247, 323, 312, 370 },
		{ 333, 323, 388, 370 },
		{ 423, 323, 458, 370 },
		{ 494, 323, 547, 370 }
	};

	int x = _G(MouseState).CursorColumn;
	int y = _G(MouseState).CursorRow;

	for (int i = 0; i < 6; ++i) {
		if (BUTTONS[i].contains(x, y))
			return i;
	}

	return -1;
}

void Room494::restoreHotkeys() {
	AddSystemHotkey(KEY_ESCAPE, _hotkeyEscape);
	AddSystemHotkey(KEY_F2, _hotkeySave);
	AddSystemHotkey(KEY_F3, _hotkeyLoad);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
