
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

#ifndef M4_BURGER_GUI_GAME_MENU_H
#define M4_BURGER_GUI_GAME_MENU_H

#include "graphics/surface.h"
#include "m4/m4_types.h"
#include "m4/graphics/gr_buff.h"
#include "m4/gui/gui_menu_items.h"
#include "m4/gui/gui_univ.h"

namespace M4 {
namespace Burger {
namespace GUI {

using M4::GUI::guiMenu;
using M4::GUI::menuItemButton;
using M4::GUI::menuItemMsg;
using M4::GUI::menuItemHSlider;
using M4::GUI::menuItemVSlider;
using M4::GUI::menuItemTextField;
using M4::GUI::Sprite;
using M4::GUI::CALLBACK;
using M4::GUI::ItemHandlerFunction;

//GAME MENU FUNCTIONS
extern void CreateGameMenu(RGB8 *myPalette);
extern void CreateOptionsMenu(RGB8 *myPalette);
extern void CreateF2SaveMenu(RGB8 *myPalette);
extern void CreateLoadMenu(RGB8 *myPalette);
extern void CreateF3LoadMenu(RGB8 *myPalette);

//routines used by the main menu
void CreateLoadMenuFromMain(RGB8 *myPalette);
void CreateGameMenuFromMain(RGB8 *myPalette);

//======================================
//
//		Game menu enums and defines
//

enum game_menu_button_tags {
	GM_TAG_QUIT = 1,
	GM_TAG_OPTIONS = 2,
	GM_TAG_RESUME = 3,
	GM_TAG_SAVE = 4,
	GM_TAG_LOAD = 5,
	GM_TAG_MAIN = 6
};

//======================================
//
//		Save/Load menu enums and defines
//
#define SAVE_LOAD_MENU_X		145
#define SAVE_LOAD_MENU_Y		 10
#define SAVE_LOAD_MENU_W		344
#define SAVE_LOAD_MENU_H		460

#define SL_SAVE_X			214
#define SL_SAVE_Y			384
#define SL_SAVE_W			 74
#define SL_SAVE_H		    43

#define SL_LOAD_X			214
#define SL_LOAD_Y			384
#define SL_LOAD_W			 74
#define SL_LOAD_H		    43

#define SL_UP_X			292
#define SL_UP_Y			255
#define SL_UP_W			 20
#define SL_UP_H		    17

#define SL_DOWN_X			293
#define SL_DOWN_Y			363
#define SL_DOWN_W			 20
#define SL_DOWN_H		    17

#define SL_SLIDER_X			291
#define SL_SLIDER_Y			255
#define SL_SLIDER_W			 23
#define SL_SLIDER_H		   127

#define SL_CANCEL_X			139	
#define SL_CANCEL_Y			384	
#define SL_CANCEL_W			 74	
#define SL_CANCEL_H		    43	

#define SL_SAVE_LABEL_X			 50
#define SL_SAVE_LABEL_Y			241
#define SL_SAVE_LABEL_W			 70
#define SL_SAVE_LABEL_H		    16

#define SL_LOAD_LABEL_X			 50
#define SL_LOAD_LABEL_Y			241
#define SL_LOAD_LABEL_W			 70
#define SL_LOAD_LABEL_H		    16

#define SL_SCROLL_FIELD_X 		 50
#define SL_SCROLL_FIELD_Y 		256
#define SL_SCROLL_FIELD_W 		238
#define SL_SCROLL_FIELD_H 		121

#define SL_SCROLL_LINE_W 		238
#define SL_SCROLL_LINE_H 		 15	//was 16

#define SL_THUMBNAIL_X			 66
#define SL_THUMBNAIL_Y			 28

/**
 * Error menu enums and defines
 */
enum error_menu_sprites {
	EM_DIALOG_BOX,

	EM_RETURN_BTN_NORM,
	EM_RETURN_BTN_OVER,
	EM_RETURN_BTN_PRESS,

	EM_TOTAL_SPRITES
};

enum error_menu_tags {
	EM_TAG_RETURN = 1
};
#define ERROR_MENU_X	 100
#define ERROR_MENU_Y	 100
#define ERROR_MENU_W	 100
#define ERROR_MENU_H	 100

#define EM_RETURN_X	  15
#define EM_RETURN_Y	  15
#define EM_RETURN_W	  15
#define EM_RETURN_H	  15


void CreateGameMenuMain(RGB8 *myPalette);

} // namespace GUI
} // namespace Burger
} // namespace M4

#endif
