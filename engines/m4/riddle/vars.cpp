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
#include "common/events.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/inventory.h"
#include "m4/gui/gui_vmng.h"

namespace M4 {
namespace Riddle {

Vars *g_vars;

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
	_kernel.letter_box_y = 30;
	_kernel.first_fade = 0;
	_kernel.first_non_walker_cel_hash = 32;
	_i_just_hyperwalked = false;
	// val1 = 15;

	conv_set_default_hv(6, 4);
	// val2 = 1
	_game.previous_room = 0;
	_game.room_id = 0;

	if (_game.new_room == 0)
		_game.setRoom(901);

	initMouseSeries("cursor", nullptr);
	conv_set_default_text_colour(7, 8);
	//exit_button_code
}

void Vars::global_menu_system_init() {
/*
	AddSystemHotkey(KEY_ESCAPE, escape_key_pressed);
	AddSystemHotkey(KEY_F2, cb_F2);
	AddSystemHotkey(KEY_F3, cb_F3);

	if (_interface.init(0, 5, 6, 8, 9))
		static_cast<Inventory *>(_inventory)->init();
		*/
}

void Vars::initialize_game() {
/*
	// Put all the inventory items back in their original scenes
	for (const auto &item : _inventory->_objects)
		inv_put_thing_in(item->name, item->scene);
	inv_give_to_player("MONEY");

	// Reset the global variables
	_flags.reset();
	_flags.reset1();
	_flags.reset2();
	_flags.reset3();
	_flags.reset4();
	_flags.reset5();
	_flags.conv_reset_all();

	Rooms::Room::setWilburHotspot();
	*/
}

void Vars::custom_ascii_converter_proc(char *string) {
	/*
	char *str;

	for (const auto *entry = ASCII_CONVERTERS; entry->_find; entry++) {
		while ((str = strstr(string, entry->_find)) != nullptr)
			*str = entry->_replace;
	}
	*/
}

void Vars::escape_key_pressed(void *, void *) {

}

void Vars::cb_F2(void *, void *) {

}

void Vars::cb_F3(void *, void *) {

}

} // namespace Riddle
} // namespace M4
