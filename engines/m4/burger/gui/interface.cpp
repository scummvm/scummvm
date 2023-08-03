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

#include "m4/burger/gui/interface.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/gui_vmng.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace GUI {

#define INTERFACE_SPRITES 22

Interface::Interface() : M4::Interface() {
	_x1 = 0;
	_y1 = 374;
	_x2 = SCREEN_WIDTH;
	_y2 = SCREEN_HEIGHT;
}

void Interface::init(int arrow, int wait, int look, int grab, int use) {
	M4::Interface::init(arrow, wait, look, grab, use);

	_sprite = series_load("999intr", 22, nullptr);
	if (_sprite != 22)
		error_show(FL, 'SLF!');

	mouse_set_sprite(wait);

	_G(gameInterfaceBuff) = new GrBuff(_x2 - _x1, _y2 - _y1);
	setup();

	mouse_set_sprite(arrow);
}

Interface::~Interface() {
	delete _interfaceBox;
	delete _inventory;
	delete _textField;
	delete _btnTake;
	delete _btnManipulate;
	delete _btnHandle;
	delete _btnAbductFail;
	delete _btnMenu;
	delete _btnScrollLeft;
	delete _btnScrollRight;
}

void Interface::show() {
	M4::Interface::show();
	_interfaceBox->_must_redraw_all = true;
	vmng_screen_show(_G(gameInterfaceBuff));
	_visible = true;
	track_hotspots_refresh();
}

void Interface::setup() {
	_interfaceBox = new InterfaceBox(RectClass(0, 0, SCREEN_WIDTH - 1, 105));
	_inventory = new GUI::Inventory(RectClass(188, 22, 539, 97), _sprite, 9, 1, 39, 75, 3);
	_textField = new TextField(200, 1, 450, 21);
	_btnTake = new ButtonClass(RectClass(60, 35, 92, 66), "take", 4, 3, 3, 4, 5, INTERFACE_SPRITES);
	_btnManipulate = new ButtonClass(RectClass(105, 35, 137, 66), "manipulate", 7, 6, 6, 7, 8, INTERFACE_SPRITES);
	_btnHandle = new ButtonClass(RectClass(15, 35, 47, 66), "handle", 5, 0, 0, 1, 2, INTERFACE_SPRITES);

	_interfaceBox->add(_btnTake);
	_interfaceBox->add(_btnManipulate);
	_interfaceBox->add(_btnHandle);


	if (_G(executing) == WHOLE_GAME) {
		_btnAbductFail = new ButtonClass(RectClass(580, 10, 620, 69), "abductfail", 10, 69, 69, 70, 71, INTERFACE_SPRITES);
		_btnMenu = new ButtonClass(RectClass(582, 70, 619, 105), "menu", 11, 76, 76, 77, 78, INTERFACE_SPRITES);
		_interfaceBox->add(_btnAbductFail);
		_interfaceBox->add(_btnMenu);

	} else {
		_btnAbductFail = new ButtonClass(RectClass(580, 22, 620, 75), "abductfail", 10, 69, 69, 70, 71, INTERFACE_SPRITES);
		_interfaceBox->add(_btnAbductFail);
	}

	_btnScrollLeft = new ButtonClass(RectClass(168, 22, 188, 97), "scroll left", 8, 59, 60, 61, 62, INTERFACE_SPRITES);
	_btnScrollRight = new ButtonClass(RectClass(539, 22, 559, 97), "scroll right", 9, 63, 64, 65, 66, INTERFACE_SPRITES);
	_interfaceBox->add(_btnScrollLeft);
	_interfaceBox->add(_btnScrollRight);
}

void Interface::cancel_sentence() {
	_textField->set_string(" ");
	_G(player).need_to_walk = false;
	_G(player).ready_to_walk = false;
	_G(player).command_ready = false;

	_pointer1 = nullptr;
	_pointer2 = nullptr;
	_pointer3 = nullptr;
	_flag1 = false;

	track_hotspots_refresh();
}

void Interface::freshen_sentence() {
	_textField->set_string(" ");
	_G(player).need_to_walk = false;
	_G(player).ready_to_walk = _G(player).need_to_walk;
	_G(player).command_ready = _G(player).ready_to_walk;
	_pointer1 = nullptr;
	_pointer2 = nullptr;

	track_hotspots_refresh();
}

bool Interface::set_interface_palette(RGB8 *myPalette) {
	gr_pal_set_RGB8(&myPalette[1], 0, 68, 0);
	gr_pal_set_RGB8(&myPalette[2], 0, 134, 0);
	gr_pal_set_RGB8(&myPalette[3], 0, 204, 0);
	gr_pal_set_RGB8(&myPalette[4], 28, 8, 90);
	gr_pal_set_RGB8(&myPalette[5], 204, 204, 250);
	gr_pal_set_RGB8(&myPalette[6], 204, 204, 102);
	gr_pal_set_RGB8(&myPalette[7], 5, 1, 0);
	gr_pal_set_RGB8(&myPalette[8], 102, 51, 222);
	gr_pal_set_RGB8(&myPalette[9], 85, 117, 255);
	gr_pal_set_RGB8(&myPalette[10], 68, 68, 68);
	gr_pal_set_RGB8(&myPalette[11], 51, 255, 0);
	gr_pal_set_RGB8(&myPalette[12], 51, 51, 115);
	gr_pal_set_RGB8(&myPalette[13], 119, 119, 119);
	gr_pal_set_RGB8(&myPalette[14], 151, 153, 150);
	gr_pal_set_RGB8(&myPalette[15], 153, 0, 0);
	gr_pal_set_RGB8(&myPalette[16], 153, 53, 9);
	gr_pal_set_RGB8(&myPalette[17], 117, 246, 255);
	gr_pal_set_RGB8(&myPalette[18], 88, 0, 0);
	gr_pal_set_RGB8(&myPalette[19], 195, 0, 83);
	gr_pal_set_RGB8(&myPalette[20], 204, 102, 61);
	gr_pal_set_RGB8(&myPalette[21], 204, 153, 118);
	gr_pal_set_RGB8(&myPalette[22], 204, 255, 204);
	gr_pal_set_RGB8(&myPalette[23], 207, 158, 73);
	gr_pal_set_RGB8(&myPalette[24], 238, 0, 0);
	gr_pal_set_RGB8(&myPalette[25], 248, 51, 31);
	gr_pal_set_RGB8(&myPalette[26], 255, 131, 0);
	gr_pal_set_RGB8(&myPalette[27], 255, 153, 102);
	gr_pal_set_RGB8(&myPalette[28], 255, 204, 153);
	gr_pal_set_RGB8(&myPalette[29], 255, 252, 144);
	gr_pal_set_RGB8(&myPalette[30], 255, 255, 0);
	gr_pal_set_RGB8(&myPalette[31], 255, 255, 255);
	gr_pal_set_range(myPalette, 1, 31);

	return true;
}

void Interface::track_hotspots_refresh() {
	_hotspot = nullptr;
	--_savedX;

	bool z = false;
	eventHandler(_G(gameInterfaceBuff), EVENT_MOUSE, 1,
		_G(MouseState).CursorColumn, _G(MouseState).CursorRow, &z);
}

bool Interface::eventHandler(void *bufferPtr, int32 eventType, int32 event, int32 x, int32 y, bool *z) {
	if (eventType != EVENT_MOUSE)
		return false;

	if (_G(kernel).fading_to_grey && event == 5) {
		kernel_unexamine_inventory_object(_G(master_palette), 5, 1);
		return true;
	}

	if (player_commands_allowed()) {
		if (x == _savedX && y == _savedY && event != 2 && event != 5 &&
				event != 3 && event != 4)
			return true;

		_savedX = x;
		_savedY = y;

		ControlStatus status = _interfaceBox->track(event, _x1, _y1);

		switch (status) {
		case NOTHING:
			_state = 0;
			break;
		case SELECTED:
			sub1();
			break;
		default:
			_state = 1;
			break;
		}

		if (_state == 0 || _state == 2) {
			status = _inventory->track(event, _x1, _y1);
			_state = (status == NOTHING) ? 0 : 2;
		}

		if (!_state) {
			int32 scrStatus;
			ScreenContext *screen = vmng_screen_find(_G(gameDrawBuff), &scrStatus);

			if (y >= _y1) {
				if (!_flag1)
					mouse_set_sprite(kArrowCursor);

				_textField->set_string(" ");
			}

			if (track(event, x - screen->x1, y - screen->y1) == SELECTED)
				dispatch_command();
		}
	}

	if (_interfaceBox->_must_redraw_all) {
		_textField->_must_redraw = true;
		_inventory->_must_redraw_all = true;
	}

	_interfaceBox->draw(_G(gameInterfaceBuff));
	_textField->draw(_G(gameInterfaceBuff));
	_inventory->draw(_G(gameInterfaceBuff));

	return true;
}

void Interface::sub1() {
	warning("TODO: Interface::sub1");
}

void Interface::sub2() {
	warning("TODO: Interface::sub2");
}

ControlStatus Interface::track(int event, int x, int y) {
	warning("TODO: Interface::track");
	return NOTHING;
}

void Interface::dispatch_command() {
	warning("TODO: Interface::dispatch_command");
}

} // namespace GUI
} // namespace Burger
} // namespace M4
