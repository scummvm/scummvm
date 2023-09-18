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
#include "m4/gui/gui_vmng.h"

namespace M4 {
namespace Riddle {

Vars *g_vars;

Vars::Vars() {
	g_vars = this;
}

Vars::~Vars() {
	g_vars = nullptr;
	delete _inventory;
}

void Vars::main_cold_data_init() {
	// TODO
	initMouseSeries("cursor", nullptr);

	_kernel.first_fade = 32;
	_G(custom_ascii_converter) = custom_ascii_converter_proc;

	_game.setRoom(101);		/*****DEBUG*****/

	font_set_colors(2, 1, 3);
}

void Vars::global_menu_system_init() {
/*
	AddSystemHotkey(Common::KEYCODE_ESCAPE, escape_key_pressed);
	AddSystemHotkey(Common::KEYCODE_F2, cb_F2);
	AddSystemHotkey(Common::KEYCODE_F3, cb_F3);

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

void Vars::initMouseSeries(const Common::String &assetName, RGB8 *myPalette) {
	int32 maxW, maxH;

	_mouseSeriesHandle = nullptr;
	_mouseSeriesOffset = 0;
	_mouseSeriesPalOffset = 0;

	if (_mouseSprite)
		_mouseSprite->data = nullptr;
	if (_mouseBuffer.data)
		mem_free(_mouseBuffer.data);

	if (LoadSpriteSeries(assetName.c_str(), &_mouseSeriesHandle, &_mouseSeriesOffset, &_mouseSeriesPalOffset, myPalette) > 0) {
		_mouseSeriesResource = assetName;

		if (ws_GetSSMaxWH(_mouseSeriesHandle, _mouseSeriesOffset, &maxW, &maxH)) {
			if (maxW && maxH) {
				_mouseBuffer.data = (byte *)mem_alloc(maxW * maxH, "mouse graphic");
				_mouseBuffer.w = maxW;
				_mouseBuffer.h = maxH;
				_mouseBuffer.stride = maxW;

				vmng_screen_show(_mouseScreenSource);
				ResizeScreen(_mouseScreenSource, maxW, maxH);

				_currMouseNum = -1;
				_mouseIsLocked = false;
				mouse_set_sprite(kArrowCursor);
			}
		}
	}
}

void Vars::custom_ascii_converter_proc(char *string) {
	char *str;
/*
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
