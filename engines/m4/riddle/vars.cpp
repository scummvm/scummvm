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

#include "common/debug.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/inventory.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/m4.h"

namespace M4 {
namespace Riddle {

Vars *g_vars;

/**
 * Structure for accented character replacement
 */
struct ConverterEntry {
	const char *_find;
	byte _replace;
};
static const ConverterEntry SPANISH_ASCII_CONVERTERS[] = {
	{ "\xc7", 1 },
	{ "\xfc", 2 },
	{ "\xe9", 3 },
	{ "\xe2", 4 },
	{ "\xe4", 5 },
	{ "\xe0", 6 },
	{ "\xe7", 0x0C },
	{ "\xea", 0x0D },
	{ "\xeb", 0x0E },
	{ "\xe8", 0x0F },
	{ "\xef", 0x69 },
	{ "\xee", 0x25 },
	{ "\xec", 0x12 },
	{ "\xc1", 0x23 },
	{ "\xc9", 0x14 },
	{ "\xf4", 0x15 },
	{ "\xf6", 0x16 },
	{ "\xf2", 0x17 },
	{ "\xfb", 0x18 },
	{ "\xf9", 0x19 },
	{ "\xcd", 0x3C },
	{ "\xd3", 0x3E },
	{ "\xda", 0x1C },
	{ "\xe1", 0x1D },
	{ "\xed", 0x1E },
	{ "\xf3", 0x1F },
	{ "\xfa", 0x40 },
	{ "\xf1", 0x5F },
	{ "\xd1", 0x7E },
	{ "\xbf", 0x7C },
	{ "\xa1", 0x7F },
	{ nullptr, 0 }
};

Vars::Vars() {
	g_vars = this;

	Inventory *inv = new Inventory();
	_inventory = inv;
}

Vars::~Vars() {
	g_vars = nullptr;
	delete _inventory;
}

void Vars::main_cold_data_init() {
	_player.walker_loads_first = false;
	_player.walker_visible = true;
	_global_sound_room = 997;
	_kernel.letter_box_y = LETTERBOX_Y;
	_kernel.first_fade = 0;
	_kernel.first_non_walker_cel_hash = 32;
	_i_just_hyperwalked = false;

	conv_set_default_hv(6, 4);
	_game.previous_room = 0;
	_game.room_id = 0;

	if (_game.new_room == 0)
		_game.setRoom(901);

	initMouseSeries("cursor", nullptr);
	conv_set_default_text_colour(7, 8);

	if (g_engine->getLanguage() == Common::ES_ESP) {
		_custom_ascii_converter = spanish_ascii_converter_proc;
	}
}

void Vars::global_menu_system_init() {
	AddSystemHotkey(KEY_ESCAPE, Hotkeys::escape_key_pressed);
	AddSystemHotkey(KEY_F2, Hotkeys::saveGame);
	AddSystemHotkey(KEY_F3, Hotkeys::loadGame);

	if (_interface.init(0, 5, 6, 8, 9))
		static_cast<Inventory *>(_inventory)->init();
}

void Vars::initialize_game() {
	// Put all the inventory items back in their original scenes
	for (const auto &item : _inventory->_objects)
		inv_put_thing_in(item->name, item->scene);

	_G(flags)[V071] = 2;
	_G(flags)[V088] = 1;
	_G(flags)[V086] = 2;
	_G(flags)[V014] = 1;
	_G(flags)[V270] = 824;
	_G(flags)[V282] = 1;
	_G(flags)[V292] = 1;
}

void Vars::spanish_ascii_converter_proc(char *string) {
	char *str;

	for (const auto *entry = SPANISH_ASCII_CONVERTERS; entry->_find; entry++) {
		while ((str = strstr(string, entry->_find)) != nullptr)
			*str = entry->_replace;
	}
}

} // namespace Riddle
} // namespace M4
