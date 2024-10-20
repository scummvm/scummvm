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

#include "m4/riddle/gui/interface.h"
#include "m4/riddle/gui/game_menu.h"
#include "m4/riddle/gui/inventory.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/other.h"
#include "m4/core/cstring.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/gui_vmng.h"

namespace M4 {
namespace Riddle {
namespace GUI {

Interface::Interface() : M4::Interface() {
	_x1 = 0;
	_y1 = 374;
	_x2 = SCREEN_WIDTH;
	_y2 = SCREEN_HEIGHT;
}

bool Interface::init(int arrow, int wait, int look, int grab, int use) {
	M4::Interface::init(arrow, wait, look, grab, use);

	_sprite = AddWSAssetCELS("INTERFACE STUFF", 22, _G(master_palette));
	gr_pal_interface(_G(master_palette));

	if (_sprite != 22)
		error_show(FL, 'SLF!');

	mouse_set_sprite(arrow);

	if (!_G(gameInterfaceBuff)) {
		_G(gameInterfaceBuff) = new GrBuff(_x2 - _x1, _y2 - _y1);
		setup();
		return true;
	} else {
		return false;
	}
}

Interface::~Interface() {
	delete _interfaceBox;
	delete _inventory;
	delete _textField;
	delete _btnTake;
	delete _btnManipulate;
	delete _btnHandle;
	delete _btnBackpack;
	delete _btnBinky;
}

void Interface::show() {
	M4::Interface::show();
	_interfaceBox->_must_redraw_all = true;
	vmng_screen_show(_G(gameInterfaceBuff));
	_visible = true;
	track_hotspots_refresh();
}

void Interface::setup() {
	_interfaceBox = new InterfaceBox(RectClass(10, 10, SCREEN_WIDTH - 11, 101));
	_inventory = new GUI::Inventory(RectClass(207, 2, 557, 74),
		_sprite, 4, 2, 35, 35, 3);
	_textField = new TextField(13, 2, 189, 20);

	_btnTake = new ButtonClass(RectClass(2, 10, 40, 50), "take", 4,3, 5, 4, INTERFACE_SPRITES);
	_btnManipulate = new ButtonClass(RectClass(47, 10, 86, 50), "manipulate", 7, 6, 8, 7, INTERFACE_SPRITES);
	_btnHandle = new ButtonClass(RectClass(92, 10, 130, 50), "handle", 5, 0, 2, 4, INTERFACE_SPRITES);
	_interfaceBox->add(_btnTake);
	_interfaceBox->add(_btnManipulate);
	_interfaceBox->add(_btnHandle);

	_btnBackpack = new BackpackClass(RectClass(135, 10, 176, 50), "backpack", 6, 9, 9, 10, INTERFACE_SPRITES);
	_btnBinky = new ButtonClass(RectClass(582, 10, 629, 50), "binky", 8, 11, 13, 12, INTERFACE_SPRITES);
	_interfaceBox->add(_btnBackpack);
	_interfaceBox->add(_btnBinky);

	_inventory->addToInterfaceBox(_interfaceBox);
}

void Interface::cancel_sentence() {
	_textField->set_string(" ");
	_G(player).need_to_walk = false;
	_G(player).ready_to_walk = false;
	_G(player).command_ready = false;

	_prepText[0] = '\0';
	_nounText[0] = '\0';
	_verbText[0] = '\0';
	_iconSelected = false;

	track_hotspots_refresh();
}

void Interface::freshen_sentence() {
	_textField->set_string(" ");
	_G(player).need_to_walk = false;
	_G(player).ready_to_walk = _G(player).need_to_walk;
	_G(player).command_ready = _G(player).ready_to_walk;
	_prepText[0] = '\0';
	_nounText[0] = '\0';

	track_hotspots_refresh();
}

bool Interface::set_interface_palette(RGB8 *myPalette) {
	gr_pal_set_RGB8(&myPalette[0], 0, 0, 0);
	gr_pal_set_RGB8(&myPalette[1], 0, 68, 0);
	gr_pal_set_RGB8(&myPalette[2], 0, 0, 39);
	gr_pal_set_RGB8(&myPalette[3], 0, 75, 71);
	gr_pal_set_RGB8(&myPalette[4], 0, 107, 103);
	gr_pal_set_RGB8(&myPalette[5], 0, 135, 131);
	gr_pal_set_RGB8(&myPalette[6], 0, 171, 163);
	gr_pal_set_RGB8(&myPalette[7], 199, 215, 207);
	gr_pal_set_RGB8(&myPalette[8], 235, 247, 231);
	gr_pal_set_RGB8(&myPalette[9], 131, 103, 63);
	gr_pal_set_RGB8(&myPalette[10], 143, 115, 75);
	gr_pal_set_RGB8(&myPalette[11], 155, 127, 91);
	gr_pal_set_RGB8(&myPalette[12], 167, 143, 107);
	gr_pal_set_RGB8(&myPalette[13], 175, 131, 115);
	gr_pal_set_RGB8(&myPalette[14], 199, 155, 131);
	gr_pal_set_RGB8(&myPalette[15], 227, 183, 143);
	gr_pal_set_RGB8(&myPalette[16], 255, 215, 159);
	gr_pal_set_RGB8(&myPalette[17], 99, 27, 39);
	gr_pal_set_RGB8(&myPalette[18], 83, 19, 27);
	gr_pal_set_RGB8(&myPalette[19], 71, 11, 19);
	gr_pal_set_RGB8(&myPalette[20], 59, 7, 15);

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

	if (_G(kernel).fading_to_grey && event == _ME_L_release) {
		kernel_unexamine_inventory_object(_G(master_palette), 5, 1);
		return true;
	}

	if (player_commands_allowed()) {
		if (x == _savedX && y == _savedY && event != _ME_L_click &&
				event != _ME_L_release && event != _ME_L_hold && event != _ME_L_drag)
			return true;

		_savedX = x;
		_savedY = y;

		ControlStatus status = _interfaceBox->track(event, x - _x1, y - _y1);

		switch (status) {
		case NOTHING:
			_state = NOTHING;
			break;
		case SELECTED:
			trackIcons();
			break;
		default:
			_state = IN_CONTROL;
			break;
		}

		if (_state == NOTHING || _state == OVER_CONTROL) {
			status = _inventory->track(event, x - _x1, y - _y1);
			handleState(status);
			_state = status ? OVER_CONTROL : NOTHING;
		}

		if (_state == NOTHING) {
			int32 scrStatus;
			ScreenContext *screen = vmng_screen_find(_G(gameDrawBuff), &scrStatus);
			if (!screen)
				return false;

			if (y >= _y1) {
				if (!_iconSelected)
					mouse_set_sprite(kArrowCursor);

				_textField->set_string(" ");
			}

			if (trackHotspots(event, x - screen->x1, y - screen->y1) == SELECTED)
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

void Interface::trackIcons() {
	switch (_interfaceBox->_highlight_index) {
	case 4:
		// Take
		t_cb();
		break;

	case 5:
		// Look
		l_cb();
		break;

	case 6:
		// Backpack
		mouse_set_sprite(_arrow);
		_iconSelected = false;
		_inventory->toggleHidden();
		_inventory->refresh_scrollbars();
		break;

	case 7:
		// Use
		u_cb();
		break;

	case 8:
		// Game menu
		GUI::CreateGameMenu(_G(master_palette));
		break;

	case 9:
		// Scroll inventory left
		_inventory->check_left();
		break;

	case 10:
		// Scroll inventory right
		_inventory->check_right();
		break;

	default:
		break;
	}
}

ControlStatus Interface::trackHotspots(int event, int x, int y) {
	const HotSpotRec *hotspot = g_engine->_activeRoom->custom_hotspot_which(x, y);
	if (!hotspot)
		hotspot = hotspot_which(_G(currentSceneDef).hotspots, x, y);

	if (hotspot != _hotspot) {
		if (!hotspot) {
			_textField->set_string(" ");
			_hotspot = nullptr;
			return NOTHING;
		}

		if (!_iconSelected) {
			if (!mouse_set_sprite(hotspot->cursor_number))
				mouse_set_sprite(kArrowCursor);

			Common::strlcpy(_verbText, hotspot->verb, 40);
		}

		Common::String tmp = (g_engine->getLanguage() == Common::EN_ANY) ?
			hotspot->vocab : hotspot->prep;
		tmp.toUppercase();
		_textField->set_string(tmp.c_str());

		tmp = hotspot->vocab;
		tmp.toUppercase();
		Common::strlcpy(_nounText, tmp.c_str(), 40);

		_hotspot = hotspot;
	}

	if (event == 5 && hotspot) {
		_G(player).walk_x = x;
		_G(player).walk_y = y;
		_G(player).click_x = x;
		_G(player).click_y = y;

		if (hotspot) {
			if (hotspot->feet_x != 0x7fff)
				_G(player).walk_x = hotspot->feet_x;
			if (hotspot->feet_y != 0x7fff)
				_G(player).walk_y = hotspot->feet_y;
		}

		_G(player).walk_facing = hotspot->facing;
		_hotspot = nullptr;

		return SELECTED;
	} else {
		return IN_CONTROL;
	}

	return ControlStatus::NOTHING;
}

void Interface::dispatch_command() {
	--_savedX;
	cstrncpy(_G(player).verb, _verbText, 40);
	cstrncpy(_G(player).noun, _nounText, 40);
	cstrncpy(_G(player).prep, _prepText, 40);

	_G(player).waiting_for_walk = true;
	_G(player).ready_to_walk = true;
	_G(player).need_to_walk = true;
	_G(player).command_ready = true;
	_G(kernel).trigger = -1;
	_G(kernel).trigger_mode = KT_PREPARSE;
	_iconSelected = false;

	mouse_set_sprite(_arrow);
	_G(player).walker_trigger = -1;

	g_engine->_activeRoom->pre_parser();
	g_engine->_activeSection->pre_parser();
	g_engine->global_pre_parser();
}

void Interface::handleState(ControlStatus status) {
	int highlight = _inventory->_highlight;
	int index = _inventory->_scroll + highlight;

	switch (status) {
	case NOTHING:
		_hotspot = nullptr;
		cstrncpy(_nounText, " ", 40);
		_textField->set_string(" ");
		break;

	case OVER_CONTROL:
		if (highlight < -1 || (highlight != -1 && (
				highlight < 128 || highlight > 129))) {
			_hotspot = nullptr;
			cstrncpy(_nounText, _inventory->_items[index]._name.c_str(), 40);

			if (g_engine->getLanguage() == Common::EN_ANY) {
				_textField->set_string(_inventory->_items[index]._name.c_str());
			} else {
				_textField->set_string(_inventory->_items[index]._verb.c_str());
			}
		}
		break;

	case SELECTED:
		if (highlight != -1 && _inventory->_items[index]._cell != -1) {
			if (_iconSelected) {
				_hotspot = nullptr;
				cstrncpy(_nounText, _inventory->_items[index]._name.c_str(), 40);

				if (g_engine->getLanguage() == Common::EN_ANY) {
					_textField->set_string(_inventory->_items[index]._name.c_str());
				} else {
					_textField->set_string(_inventory->_items[index]._verb.c_str());
				}

				term_message("got %d", index);
				dispatch_command();

				_G(player).ready_to_walk = true;
				_G(player).need_to_walk = false;
				_G(player).waiting_for_walk = false;
			} else {
				_hotspot = nullptr;
				cstrncpy(_verbText, _inventory->_items[index]._name.c_str(), 40);

				if (g_engine->getLanguage() == Common::EN_ANY) {
					_textField->set_string(_inventory->_items[index]._name.c_str());
				} else {
					_textField->set_string(_inventory->_items[index]._verb.c_str());
				}

				mouse_set_sprite(_inventory->_items[index]._cursor);
				_iconSelected = true;
			}
		}
		break;

	default:
		break;
	}
}

void Interface::l_cb() {
	if (player_commands_allowed() && INTERFACE_VISIBLE) {
		Common::strcpy_s(_verbText, "look at");
		mouse_set_sprite(_look);
		_iconSelected = true;
		_G(cursor_state) = kLOOK;
	}
}

void Interface::u_cb() {
	if (player_commands_allowed() && INTERFACE_VISIBLE) {
		Common::strcpy_s(_verbText, "gear");
		mouse_set_sprite(_use);
		_iconSelected = true;
		_G(cursor_state) = kUSE;
	}
}

void Interface::t_cb() {
	if (player_commands_allowed() && INTERFACE_VISIBLE) {
		Common::strcpy_s(_verbText, "take");
		mouse_set_sprite(_grab);
		_iconSelected = true;
		_G(cursor_state) = kTAKE;
	}
}

void Interface::a_cb() {
	if (player_commands_allowed() && INTERFACE_VISIBLE) {
		Common::strcpy_s(_verbText, "<><><><><><><><>");
		mouse_set_sprite(_arrow);
		_iconSelected = true;
		_G(cursor_state) = kARROW;
	}
}

} // namespace GUI
} // namespace Riddle
} // namespace M4
