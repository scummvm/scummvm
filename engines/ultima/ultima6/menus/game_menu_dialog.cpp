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

#include "nuvieDefs.h"

#include "GUI.h"
#include "GUI_types.h"
#include "GUI_button.h"
#include "GUI_CallBack.h"
#include "GUI_area.h"

#include "GUI_Dialog.h"
#include "GameMenuDialog.h"
#include "VideoDialog.h"
#include "AudioDialog.h"
#include "GameplayDialog.h"
#include "InputDialog.h"
#include "CheatsDialog.h"
#include "Event.h"
#include "Keys.h"

namespace Ultima {
namespace Ultima6 {

#define GMD_WIDTH 150

#ifdef HAVE_JOYSTICK_SUPPORT
#include "JoystickDialog.h"

#define GMD_HEIGHT 135 // 13 bigger
#else
#define GMD_HEIGHT 122
#endif

GameMenuDialog::GameMenuDialog(GUI_CallBack *callback)
	: GUI_Dialog(Game::get_game()->get_game_x_offset() + (Game::get_game()->get_game_width() - GMD_WIDTH) / 2,
	             Game::get_game()->get_game_y_offset() + (Game::get_game()->get_game_height() - GMD_HEIGHT) / 2,
	             GMD_WIDTH, GMD_HEIGHT, 244, 216, 131, GUI_DIALOG_UNMOVABLE) {
	callback_object = callback;
	init();
	grab_focus();
}

bool GameMenuDialog::init() {
	int width = 132;
	int height = 12;
	int buttonX = 9;
	uint8 buttonY = 9;
	uint8 row_h = 13;
	b_index_num = -1;
	last_index = 0;
	GUI *gui = GUI::get_gui();

	saveLoad_button = new GUI_Button(this, buttonX, buttonY, width, height, "Load/Save Game", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(saveLoad_button);
	button_index[last_index] = saveLoad_button;
	video_button = new GUI_Button(this, buttonX, buttonY += row_h, width, height, "Video Options", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(video_button);
	button_index[last_index += 1] = video_button;
	audio_button = new GUI_Button(this, buttonX, buttonY += row_h, width, height, "Audio Options", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(audio_button);
	button_index[last_index += 1] = audio_button;
	input_button = new GUI_Button(this, buttonX, buttonY += row_h, width, height, "Input Options", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(input_button);
	button_index[last_index += 1] = input_button;
#ifdef HAVE_JOYSTICK_SUPPORT
	joystick_button = new GUI_Button(this, buttonX, buttonY += row_h, width, height, "Joystick Options", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(joystick_button);
	button_index[last_index += 1] = joystick_button;
#endif
	gameplay_button = new GUI_Button(this, buttonX, buttonY += row_h, width, height, "Gameplay Options", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(gameplay_button);
	button_index[last_index += 1] = gameplay_button;
	cheats_button = new GUI_Button(this, buttonX, buttonY += row_h, width, height, "Cheats", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(cheats_button);
	button_index[last_index += 1] = cheats_button;
	continue_button = new GUI_Button(this, buttonX, buttonY += row_h, width, height, "Back to Game", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(continue_button);
	button_index[last_index += 1] = cheats_button;
	quit_button = new GUI_Button(this, buttonX, buttonY += row_h, width, height, "Quit", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(quit_button);
	button_index[last_index += 1] = quit_button;

	return true;
}

GameMenuDialog::~GameMenuDialog() {
}

GUI_status GameMenuDialog::close_dialog() {
	Delete(); // mark dialog as deleted. it will be freed by the GUI object
	callback_object->callback(GAMEMENUDIALOG_CB_DELETE, this, this);
	GUI::get_gui()->unlock_input();
	return GUI_YUM;
}

GUI_status GameMenuDialog::KeyDown(SDL_Keysym key) {
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case NORTH_KEY:
		if (b_index_num != -1)
			button_index[b_index_num]->set_highlighted(false);

		if (b_index_num <= 0)
			b_index_num = last_index;
		else
			b_index_num = b_index_num - 1;
		button_index[b_index_num]->set_highlighted(true);
		break;
	case SOUTH_KEY:
		if (b_index_num != -1)
			button_index[b_index_num]->set_highlighted(false);

		if (b_index_num == last_index)
			b_index_num = 0;
		else
			b_index_num += 1;
		button_index[b_index_num]->set_highlighted(true);
		break;
	case DO_ACTION_KEY:
		if (b_index_num != -1) return button_index[b_index_num]->Activate_button();
		break;
	case CANCEL_ACTION_KEY:
		return close_dialog();
	default:
		keybinder->handle_always_available_keys(a);
		break;
	}
	return GUI_YUM;
}

GUI_status GameMenuDialog::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	GUI *gui = GUI::get_gui();

	if (caller == this) {
		close_dialog();
	} else if (caller == (GUI_CallBack *)saveLoad_button) {
		Game::get_game()->get_event()->saveDialog();
	} else if (caller == (GUI_CallBack *)video_button) {
		GUI_Widget *video_dialog;
		video_dialog = (GUI_Widget *) new VideoDialog((GUI_CallBack *)this);
		GUI::get_gui()->AddWidget(video_dialog);
		gui->lock_input(video_dialog);
	} else if (caller == (GUI_CallBack *)audio_button) {
		GUI_Widget *audio_dialog;
		audio_dialog = (GUI_Widget *) new AudioDialog((GUI_CallBack *)this);
		GUI::get_gui()->AddWidget(audio_dialog);
		gui->lock_input(audio_dialog);
	} else if (caller == (GUI_CallBack *)input_button) {
		GUI_Widget *input_dialog;
		input_dialog = (GUI_Widget *) new InputDialog((GUI_CallBack *)this);
		GUI::get_gui()->AddWidget(input_dialog);
		gui->lock_input(input_dialog);
#ifdef HAVE_JOYSTICK_SUPPORT
	} else if (caller == (GUI_CallBack *)joystick_button) {
		GUI_Widget *joystick_dialog;
		joystick_dialog = (GUI_Widget *) new JoystickDialog((GUI_CallBack *)this);
		GUI::get_gui()->AddWidget(joystick_dialog);
		gui->lock_input(joystick_dialog);
#endif
	} else if (caller == (GUI_CallBack *)gameplay_button) {
		GUI_Widget *gameplay_dialog;
		gameplay_dialog = (GUI_Widget *) new GameplayDialog((GUI_CallBack *)this);
		GUI::get_gui()->AddWidget(gameplay_dialog);
		gui->lock_input(gameplay_dialog);
	} else if (caller == (GUI_CallBack *)cheats_button) {
		GUI_Widget *cheats_dialog;
		cheats_dialog = (GUI_Widget *) new CheatsDialog((GUI_CallBack *)this);
		GUI::get_gui()->AddWidget(cheats_dialog);
		gui->lock_input(cheats_dialog);
	} else if (caller == (GUI_CallBack *)continue_button) {
		return close_dialog();
	} else if (caller == (GUI_CallBack *)quit_button) {
		Game::get_game()->get_event()->quitDialog();
	} else {
		gui->lock_input(this);
		return GUI_PASS;
	}
	return GUI_YUM;
}

} // End of namespace Ultima6
} // End of namespace Ultima
