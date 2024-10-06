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
#include "m4/core/cstring.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/gui_vmng.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"
#include "m4/adv_r/other.h"

namespace M4 {
namespace Burger {
namespace GUI {

Interface::Interface() : M4::Interface() {
	_x1 = 0;
	_y1 = 374;
	_x2 = SCREEN_WIDTH;
	_y2 = SCREEN_HEIGHT;
}

bool Interface::init(int arrow, int wait, int look, int grab, int use) {
	M4::Interface::init(arrow, wait, look, grab, use);

	_sprite = series_load("999intr", 22, nullptr);
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
	_textField = new TextField(200, 1, 470, 21);
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

void Interface::refresh_right_arrow() {
	if (_inventory->need_right() || _inventory->need_left()) {
		_btnScrollRight->set_sprite_unknown(63);
		_btnScrollRight->set_sprite_relaxed(64);
		_btnScrollRight->set_sprite_picked(66);
		_btnScrollRight->set_sprite_over(65);
		_btnScrollRight->unhide();
	} else {
//		_btnScrollRight->hide();
//		_btnScrollLeft->hide();
	}
}

void Interface::refresh_left_arrow() {
	if (_inventory->need_right() || _inventory->need_left()) {
		_btnScrollLeft->set_sprite_unknown(59);
		_btnScrollLeft->set_sprite_relaxed(60);
		_btnScrollLeft->set_sprite_picked(62);
		_btnScrollLeft->set_sprite_over(61);
		_btnScrollLeft->unhide();
	} else {
//		_btnScrollRight->hide();
//		_btnScrollLeft->hide();
	}
}

void Interface::trackIcons() {
	KernelTriggerType oldMode = _G(kernel).trigger_mode;
	_G(kernel).trigger_mode = KT_DAEMON;

	switch (_interfaceBox->_highlight_index) {
	case 4:
		t_cb();
		break;

	case 5:
		l_cb();
		break;

	case 6:
		mouse_set_sprite(_arrow);
		_iconSelected = false;

		if (_btnScrollRight->is_hidden())
			refresh_right_arrow();
		else
			_btnScrollRight->hide();

		if (_btnScrollLeft->is_hidden())
			refresh_left_arrow();
		else
			_btnScrollLeft->hide();
		break;

	case 7:
		u_cb();
		break;

	case 8:
		if (!_btnScrollLeft->is_hidden()) {
			if (_inventory->need_left()) {
				_inventory->_scroll = (_inventory->_scroll <= 0) ? 0 :
					_inventory->_scroll - _inventory->_cells_v;
			}

			refresh_right_arrow();
			refresh_left_arrow();
			_inventory->_must_redraw_all = true;
		} else {
			return;
		}
		break;

	case 9:
		if (!_btnScrollRight->is_hidden()) {
			if (_inventory->need_right())
				_inventory->_scroll += _inventory->_cells_v;

			refresh_right_arrow();
			refresh_left_arrow();
			_inventory->_must_redraw_all = true;
		} else {
			return;
		}
		break;

	case 10:
		term_message("Abduct/Fail Button Pressed");

		if (_G(game).section_id == 1) {
			term_message("Abduct me now!");
			_G(wilbur_should) = 10017;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else if (_G(game).section_id == 7) {
			_G(walker).wilbur_speech("999w023");
		} else {
			term_message("Fail me now!");
			_G(wilbur_should) = 10015;
			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 11:
		// Game menu
		other_save_game_for_resurrection();

		if (g_engine->useOriginalSaveLoad()) {
			CreateGameMenu(_G(master_palette));
		} else {
			g_engine->openMainMenuDialog();
		}
		break;

	}

	_G(kernel).trigger_mode = oldMode;
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
		_G(click_x) = x;
		_G(click_y) = y;

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
		_iconSelected = false;
		_G(cursor_state) = kARROW;

		if (_hotspot && !mouse_set_sprite(_hotspot->cursor_number))
			mouse_set_sprite(kArrowCursor);
	}
}

} // namespace GUI
} // namespace Burger
} // namespace M4
