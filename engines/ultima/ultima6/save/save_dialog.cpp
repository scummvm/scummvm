/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#include "ultima/ultima6/core/nuvie_defs.h"

#include "ultima/ultima6/gui/gui.h"
#include "ultima/ultima6/gui/gui_types.h"
#include "ultima/ultima6/gui/gui_button.h"
#include "ultima/ultima6/gui/gui_text.h"
#include "ultima/ultima6/gui/gui_Scroller.h"
#include "ultima/ultima6/gui/gui_callback.h"
#include "ultima/ultima6/gui/gui_area.h"

#include "ultima/ultima6/gui/gui_Dialog.h"
#include "ultima/ultima6/save/save_slot.h"
#include "ultima/ultima6/save/save_dialog.h"
#include "ultima/ultima6/files/nuvie_file_list.h"
#include "ultima/ultima6/keybinding/keys.h"
#include "ultima/ultima6/core/event.h"
#include "ultima/ultima6/core/console.h"

namespace Ultima {
namespace Ultima6 {

#define CURSOR_HIDDEN 0
#define CURSOR_AT_TOP 1
#define CURSOR_AT_SLOTS 2

#define NUVIE_SAVE_SCROLLER_ROWS   3
#define NUVIE_SAVE_SCROLLER_HEIGHT NUVIE_SAVE_SCROLLER_ROWS * NUVIE_SAVESLOT_HEIGHT
#define SD_WIDTH 320
#define SD_HEIGHT 200

SaveDialog::SaveDialog(GUI_CallBack *callback)
	: GUI_Dialog(Game::get_game()->get_game_x_offset() + (Game::get_game()->get_game_width() - SD_WIDTH) / 2,
	             Game::get_game()->get_game_y_offset() + (Game::get_game()->get_game_height() - SD_HEIGHT) / 2,
	             SD_WIDTH, SD_HEIGHT, 244, 216, 131, GUI_DIALOG_UNMOVABLE) {
	callback_object = callback;
	selected_slot = NULL;
	scroller = NULL;
	save_button = NULL;
	load_button = NULL;
	cancel_button = NULL;
	cursor_tile = NULL;
	show_cursor = false;
	index = 0;
	save_index = 3;
	set_cursor_pos(0);
	cursor_loc = CURSOR_HIDDEN;
}

bool SaveDialog::init(const char *save_directory, const char *search_prefix) {
	uint32 num_saves, i;
	NuvieFileList filelist;
	std::string *filename;
	GUI_Widget *widget;
	GUI *gui = GUI::get_gui();
	GUI_Color bgColor = GUI_Color(162, 144, 87);
	GUI_Color bgColor1 = GUI_Color(147, 131, 74);
	GUI_Color *color_ptr;

	if (filelist.open(save_directory, search_prefix, NUVIE_SORT_TIME_DESC) == false) {
		ConsoleAddError(std::string("Failed to open ") + save_directory);
		return false;
	}

	scroller = new GUI_Scroller(20, 26, 280, NUVIE_SAVE_SCROLLER_HEIGHT, 135, 119, 76, NUVIE_SAVESLOT_HEIGHT);
	widget = (GUI_Widget *) new GUI_Text(20, 12, 0, 0, 0, "Load/Save", gui->get_font());
	AddWidget(widget);

	num_saves = filelist.get_num_files();



// Add an empty slot at the top.
	widget = new SaveSlot(this, bgColor1);
	((SaveSlot *)widget)->init(NULL, NULL);

	scroller->AddWidget(widget);

	color_ptr = &bgColor;

	for (i = 0; i < num_saves + 1; i++) {
		if (i < num_saves)
			filename = filelist.next();
		else
			filename = NULL;
		widget = new SaveSlot(this, *color_ptr);
		if (((SaveSlot *)widget)->init(save_directory, filename) == true) {
			scroller->AddWidget(widget);

			if (color_ptr == &bgColor)
				color_ptr = &bgColor1;
			else
				color_ptr = &bgColor;
		} else
			delete(SaveSlot *)widget;
	}

// pad out empty slots if required
	/*
	 if(num_saves < NUVIE_SAVE_SCROLLER_ROWS-1)
	   {
	    for(i=0; i < NUVIE_SAVE_SCROLLER_ROWS - num_saves - 1; i++)
	      {
	       widget = new SaveSlot(this, *color_ptr);
	       ((SaveSlot *)widget)->init(NULL);

	       scroller->AddWidget(widget);

	       if(color_ptr == &bgColor)
	         color_ptr = &bgColor1;
	       else
	         color_ptr = &bgColor;
	      }
	   }
	*/
	AddWidget(scroller);

	load_button = new GUI_Button(this, 105, 8, 40, 16, "Load", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(load_button);

	save_button = new GUI_Button(this, 175, 8, 40, 16, "Save", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(save_button);

	cancel_button = new GUI_Button(this, 245, 8, 55, 16, "Cancel", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0); //154
	AddWidget(cancel_button);

	filelist.close();

	if (Game::get_game()->is_armageddon() || Game::get_game()->get_event()->using_control_cheat())
		save_button->Hide();

	cursor_tile = Game::get_game()->get_tile_manager()->get_gump_cursor_tile();

	return true;
}


SaveDialog::~SaveDialog() {
}

void SaveDialog::Display(bool full_redraw) {
	GUI_Dialog::Display(full_redraw);
	if (show_cursor) {
		screen->blit(cursor_x, cursor_y, (unsigned char *)cursor_tile->data, 8, 16, 16, 16, true);
		screen->update(cursor_x, cursor_y, 16, 16);
	}
}

void SaveDialog::set_cursor_pos(uint8 index_num) {
	switch (index_num) {
	case 0:
		cursor_x = 117;
		cursor_y = 8;
		break;
	case 1:
		cursor_x = 188;
		cursor_y = 8;
		break;
	case 2:
		cursor_x = 264;
		cursor_y = 8;
		break;
	case 3:
		cursor_x = 146;
		cursor_y = 26;
		break;
	case 4:
		cursor_x = 146;
		cursor_y = 78;
		break;
	default:
	case 5:
		cursor_x = 146;
		cursor_y = 130;
		break;
	}
	cursor_x += area.left;
	cursor_y += area.top;
}

GUI_status SaveDialog::close_dialog() {
	Delete(); // mark dialog as deleted. it will be freed by the GUI object
	return callback_object->callback(SAVEDIALOG_CB_DELETE, this, this);
}

GUI_status SaveDialog::MouseWheel(sint32 x, sint32 y) {
	if (y > 0) {
		scroller->move_up();
	} else if (y < 0) {
		scroller->move_down();
	}
	return GUI_YUM;
}

GUI_status SaveDialog::MouseDown(int x, int y, MouseButton button) {
	return GUI_YUM;
}

GUI_status SaveDialog::KeyDown(const Common::KeyState &key) {
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case EAST_KEY:
		if (cursor_loc == CURSOR_AT_TOP) {
			if (index == 2)
				set_cursor_pos(index = 0);
			else
				set_cursor_pos(++index);
			break;
		} // else fall through
	case MSGSCROLL_DOWN_KEY:
		scroller->page_down();
		break;

	case WEST_KEY:
		if (cursor_loc == CURSOR_AT_TOP) {
			if (index == 0)
				set_cursor_pos(index = 2);
			else
				set_cursor_pos(--index);
			break;
		} // else fall through
	case MSGSCROLL_UP_KEY:
		scroller->page_up();
		break;

	case NORTH_KEY:
		if (cursor_loc == CURSOR_AT_SLOTS && save_index != 3) { // not at top of save slots
			set_cursor_pos(--save_index);
			break;
		}
		scroller->move_up();
		break;
	case SOUTH_KEY:
		if (cursor_loc == CURSOR_AT_SLOTS && save_index != 5) { // not at bottom of save slots
			set_cursor_pos(++save_index);
			break;
		}
		scroller->move_down();
		break;
	case HOME_KEY:
		scroller->page_up(true);
		break;
	case END_KEY:
		scroller->page_down(true);
		break;
	case TOGGLE_CURSOR_KEY:
		if (cursor_loc == CURSOR_AT_TOP) {
			cursor_loc = CURSOR_HIDDEN;
			show_cursor = false;
		} else {
			if (cursor_loc == CURSOR_HIDDEN) {
				set_cursor_pos(save_index);
				cursor_loc = CURSOR_AT_SLOTS;
			} else { // if CURSOR_AT_SLOTS
				set_cursor_pos(index);
				cursor_loc = CURSOR_AT_TOP;
			}
			show_cursor = true;
		}
		break;
	case DO_ACTION_KEY: {
		if (show_cursor == false)
			break;
		if (cursor_loc == CURSOR_AT_SLOTS) // editing save description and selecting the slot
			show_cursor = false;
		uint16 x = cursor_x * screen->get_scale_factor();
		uint16 y = (cursor_y + 8) * screen->get_scale_factor();

		Common::Event fake_event;
		fake_event.mouse.x = x;
		fake_event.mouse.y = y;
		fake_event.type = Common::EVENT_LBUTTONDOWN;
		GUI::get_gui()->HandleEvent(&fake_event);

		fake_event.mouse.x = x; //GUI::HandleEvent divides by scale so we need to restore it
		fake_event.mouse.y = y;
		fake_event.type = Common::EVENT_LBUTTONUP;
		GUI::get_gui()->HandleEvent(&fake_event);
		break;
	}
	case CANCEL_ACTION_KEY:
		return close_dialog();
	default:
		keybinder->handle_always_available_keys(a);
		break;
	}
	return GUI_YUM;
}

GUI_status SaveDialog::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if (caller == (GUI_CallBack *)cancel_button)
		return close_dialog();

	if (caller == (GUI_CallBack *)load_button) {
		if (selected_slot != NULL && callback_object->callback(SAVEDIALOG_CB_LOAD, this, selected_slot) == GUI_YUM)
			close_dialog();
		return GUI_YUM;
	}

	if (caller == (GUI_CallBack *)save_button) {
		if (selected_slot != NULL && callback_object->callback(SAVEDIALOG_CB_SAVE, this, selected_slot) == GUI_YUM)
			close_dialog();
		return GUI_YUM;
	}

	if (dynamic_cast<SaveSlot *>(caller)) {
		if (msg == SAVESLOT_CB_SELECTED) {
			if (selected_slot != NULL)
				selected_slot->deselect();

			selected_slot = (SaveSlot *)caller;
		}

		if (msg == SAVESLOT_CB_SAVE) {
			if (callback_object->callback(SAVEDIALOG_CB_SAVE, this, caller) == GUI_YUM) //caller = slot to save in
				close_dialog();
		}
	}
	/*
	 if(caller == (GUI_CallBack *)no_button)
	   return no_callback_object->callback(YESNODIALOG_CB_NO, this, this);
	*/
	return GUI_PASS;
}

} // End of namespace Ultima6
} // End of namespace Ultima
