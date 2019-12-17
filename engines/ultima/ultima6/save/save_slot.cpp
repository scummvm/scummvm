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

#include "SDL.h"
#include "nuvieDefs.h"
#include "U6misc.h"

#include "GUI.h"
#include "GUI_types.h"
#include "GUI_text.h"
#include "GUI_TextInput.h"

#include "GUI_CallBack.h"

#include "Game.h"
#include "NuvieIOFile.h"
#include "MapWindow.h"

#include "SaveGame.h"
#include "SaveSlot.h"
#include "SaveManager.h"
#include "SaveDialog.h"
#include "GUI_Scroller.h"

namespace Ultima {
namespace Ultima6 {

SaveSlot::SaveSlot(GUI_CallBack *callback, GUI_Color bg_color) : GUI_Widget(NULL, 0, 0, 266, NUVIE_SAVESLOT_HEIGHT) {
	callback_object = callback;
	background_color = bg_color;

	selected = false;
	new_save = false;
	thumbnail = NULL;
	textinput_widget = NULL;
}

SaveSlot::~SaveSlot() {
	if (thumbnail)
		SDL_FreeSurface(thumbnail);
}

bool SaveSlot::init(const char *directory, std::string *file) {
	GUI *gui = GUI::get_gui();

	if (file != NULL) {
		filename.assign(file->c_str());
		if (!load_info(directory))
			return false;
	} else {
		filename.assign(""); //empty save slot.
		if (directory != NULL) {
			save_description.assign("Original Game Save");
		} else {
			save_description.assign("New Save.");
			new_save = true;
		}
	}

	textinput_widget = new GUI_TextInput(MAPWINDOW_THUMBNAIL_SIZE + 2, 2, 255, 255, 255, (char *)save_description.c_str(), gui->get_font(), 26, 2, this);
	AddWidget((GUI_Widget *)textinput_widget);

	return true;
}

bool SaveSlot::load_info(const char *directory) {
	NuvieIOFileRead loadfile;
	SaveGame *savegame;
	SaveHeader *header;
	GUI_Widget *widget;
	GUI *gui = GUI::get_gui();
	std::string full_path;
	char buf[30];
	char gender;
	uint8 i;

	savegame = new SaveGame(Game::get_game()->get_config());

	build_path(directory, filename.c_str(), full_path);

	if (loadfile.open(full_path.c_str()) == false || savegame->check_version(&loadfile) == false) {
		DEBUG(0, LEVEL_ERROR, "Reading header from %s\n", filename.c_str());
		delete savegame;
		return false;
	}

	header = savegame->load_info(&loadfile);

	save_description = header->save_description;

	thumbnail = SDL_ConvertSurface(header->thumbnail, header->thumbnail->format, SDL_SWSURFACE);

	if (header->player_gender == 0)
		gender = 'm';
	else
		gender = 'f';

	sprintf(buf, "%s (%c)", header->player_name.c_str(), gender);
	if (strlen(buf) < 17) {
		for (i = strlen(buf); i < 17; i++)
			buf[i] = ' ';

		buf[17] = '\0';
	}

	strcat(buf, " Day:");

	widget = (GUI_Widget *) new GUI_Text(MAPWINDOW_THUMBNAIL_SIZE + 2, 20, 200, 200, 200, buf, gui->get_font()); //evil const cast lets remove this
	AddWidget(widget);

	sprintf(buf, "Lvl:%d %3d/%3d/%3d  XP:%d", header->level, header->str, header->dex, header->intelligence, header->exp);

	widget = (GUI_Widget *) new GUI_Text(MAPWINDOW_THUMBNAIL_SIZE + 2, 29, 200, 200, 200, buf, gui->get_font()); //evil const cast lets remove this
	AddWidget(widget);


	sprintf(buf, "%s (%d save", (char *)filename.c_str(), header->num_saves);
	if (header->num_saves > 1)
		strcat(buf, "s");

	strcat(buf, ")");

	widget = (GUI_Widget *) new GUI_Text(MAPWINDOW_THUMBNAIL_SIZE + 2, 38, 200, 200, 200, buf, gui->get_font());
	AddWidget(widget);

	delete savegame;

	return true;
}

std::string *SaveSlot::get_filename() {
	return &filename;
}

void SaveSlot::deselect() {
	selected = false;
	textinput_widget->set_text(save_description.c_str()); //revert to original text
}

std::string SaveSlot::get_save_description() {
	return std::string(textinput_widget->get_text());
}

/* Map the color to the display */
void SaveSlot::SetDisplay(Screen *s) {
	GUI_Widget::SetDisplay(s);
	background_color.map_color(surface);
}

void SaveSlot::Display(bool full_redraw) {
	SDL_Rect framerect = area;
	SDL_Rect destrect = area;
	if (selected) {
		GUI *gui = GUI::get_gui();
		SDL_FillRect(surface, &framerect, gui->get_selected_color()->sdl_color);
	} else
		SDL_FillRect(surface, &framerect, background_color.sdl_color);

	if (thumbnail) {
		SDL_BlitSurface(thumbnail, NULL, surface, &destrect);
	}

	DisplayChildren();

}

GUI_status SaveSlot::KeyDown(SDL_Keysym key) {

//if(key.sym == SDLK_ESCAPE)
//  return GUI_YUM;

	/*
	return no_callback_object->callback(YESNODIALOG_CB_NO, this, this);
	*/
	return GUI_PASS;
}

GUI_status SaveSlot::MouseWheel(sint32 x, sint32 y) {
	SaveManager *save_manager = Game::get_game()->get_save_manager();
	if (y > 0) {
		save_manager->get_dialog()->get_scroller()->move_up();
	} else if (y < 0) {
		save_manager->get_dialog()->get_scroller()->move_down();
	}
	return GUI_YUM;
}

GUI_status SaveSlot::MouseDown(int x, int y, int button) {
	if (selected != true) {
		selected = true;
		callback_object->callback(SAVESLOT_CB_SELECTED, this, NULL);
	}

	return GUI_YUM;
}

GUI_status SaveSlot::MouseUp(int x, int y, int button) {
	return GUI_PASS;
}

GUI_status SaveSlot::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if (msg == TEXTINPUT_CB_TEXT_READY) { //we should probably make sure the caller is a GUI_TextInput object
		return callback_object->callback(SAVESLOT_CB_SAVE, this, this);
	}

	return GUI_PASS;
}

} // End of namespace Ultima6
} // End of namespace Ultima
