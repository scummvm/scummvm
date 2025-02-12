
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
#include "m4/gui/gui_menu.h"
#include "m4/gui/gui_univ.h"

namespace M4 {
namespace Burger {
namespace GUI {

using M4::GUI::guiMenu;
using M4::GUI::menuItem;
using M4::GUI::Sprite;
using M4::GUI::CALLBACK;
using M4::GUI::ItemHandlerFunction;

// SPECIFIC ITEM FUNCTIONS

// Messages
menuItem *menu_MsgAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, bool transparent = false);
void menu_DisableMsg(menuItem *myItem, int32 tag, guiMenu *myMenu);
void menu_EnableMsg(menuItem *myItem, int32 tag, guiMenu *myMenu);

// Buttons
bool button_Handler(void *theItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem);
menuItem *menu_ButtonAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, CALLBACK callback = nullptr,
	int32 buttonType = 0, bool ghosted = false, bool transparent = false,
	const char *prompt = nullptr, ItemHandlerFunction i_handler = button_Handler);
void menu_DisableButton(menuItem *myItem, int32 tag, guiMenu *myMenu);
void menu_EnableButton(menuItem *myItem, int32 tag, guiMenu *myMenu);

// Horizontal sliders
menuItem *menu_HSliderAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h,
	int32 initPercent = 0, CALLBACK callback = nullptr, bool transparent = false);

// Vertical sliders
menuItem *menu_VSliderAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h,
	int32 initPercent = 0, CALLBACK callback = nullptr, bool transparent = false);
void menu_DisableVSlider(menuItem *myItem, int32 tag, guiMenu *myMenu);
void menu_EnableVSlider(menuItem *myItem, int32 tag, guiMenu *myMenu);

// Textfields
menuItem *menu_TextFieldAdd(guiMenu *myMenu, int32 tag, int32 x, int32 y, int32 w, int32 h, int32 initFlags,
	const char *prompt = nullptr, int32 specialtag = 0, CALLBACK callback = nullptr, bool transparent = false);

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
//		gamemenu module defines
//

// 128 very light green
// 129 light green
// 130 medium green
// 131 dark green
// 133 light red
// 136 red
// 142 dark red
// 186 purple
// 206 dark grey
// 236 very dark purple

#define TEXT_COLOR_GREY_HILITE		192  
#define TEXT_COLOR_GREY_FOREGROUND  210
#define TEXT_COLOR_GREY_SHADOW		229  

#define TEXT_COLOR_NORM_HILITE		3  
#define TEXT_COLOR_NORM_FOREGROUND	2  
#define TEXT_COLOR_NORM_SHADOW		1  

#define TEXT_COLOR_OVER_HILITE		3  
#define TEXT_COLOR_OVER_FOREGROUND	2  
#define TEXT_COLOR_OVER_SHADOW		1

#define TEXT_COLOR_PRESS_HILITE		3	 
#define TEXT_COLOR_PRESS_FOREGROUND 2  		
#define TEXT_COLOR_PRESS_SHADOW		1  

#define SLIDER_BAR_COLOR	129



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

enum save_load_menu_sprites {

	SL_DIALOG_BOX,
	SL_EMPTY_THUMB,

	SL_SAVE_BTN_GREY,
	SL_SAVE_BTN_NORM,
	SL_SAVE_BTN_OVER,
	SL_SAVE_BTN_PRESS,

	SL_LOAD_BTN_GREY,
	SL_LOAD_BTN_NORM,
	SL_LOAD_BTN_OVER,
	SL_LOAD_BTN_PRESS,

	SL_CANCEL_BTN_NORM,
	SL_CANCEL_BTN_OVER,
	SL_CANCEL_BTN_PRESS,

	SL_UP_BTN_GREY,
	SL_UP_BTN_NORM,
	SL_UP_BTN_OVER,
	SL_UP_BTN_PRESS,

	SL_DOWN_BTN_GREY,
	SL_DOWN_BTN_NORM,
	SL_DOWN_BTN_OVER,
	SL_DOWN_BTN_PRESS,

	SL_SAVE_LABEL,
	SL_LOAD_LABEL,

	SL_SLIDER_BTN_NORM,
	SL_SLIDER_BTN_OVER,
	SL_SLIDER_BTN_PRESS,

	SL_LINE_NORM,
	SL_LINE_OVER,
	SL_LINE_PRESS,

	SL_SCROLL_BAR,

	SL_TOTAL_SPRITES
};

enum save_load_menu_item_tags {
	SL_TAG_SAVE = 100,
	SL_TAG_SAVE_LABEL,
	SL_TAG_LOAD,
	SL_TAG_LOAD_LABEL,
	SL_TAG_CANCEL,
	SL_TAG_VSLIDER,
	SL_TAG_THUMBNAIL
};

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
#define SL_THUMBNAIL_W			215
#define SL_THUMBNAIL_H		   162

/**
 * Options menu enums and defines
 */
enum options_menu_sprites {

	OM_DIALOG_BOX,

	OM_SLIDER_BTN_NORM,
	OM_SLIDER_BTN_OVER,
	OM_SLIDER_BTN_PRESS,

	OM_SLIDER_BAR,

	OM_DONE_BTN_GREY,
	OM_DONE_BTN_NORM,
	OM_DONE_BTN_OVER,
	OM_DONE_BTN_PRESS,

	OM_CANCEL_BTN_NORM,
	OM_CANCEL_BTN_OVER,
	OM_CANCEL_BTN_PRESS,

	OM_TOTAL_SPRITES
};

#define OPTIONS_MENU_X		175
#define OPTIONS_MENU_Y		100
#define OPTIONS_MENU_W		298
#define OPTIONS_MENU_H		218

enum option_menu_item_tags {
	OM_TAG_DONE = 1,
	OM_TAG_CANCEL,
	OM_TAG_DIGI,
	OM_TAG_DIGESTABILITY,
};

#define OM_DONE_X 168
#define OM_DONE_Y 141
#define OM_DONE_W  74
#define OM_DONE_H  43

#define OM_CANCEL_X  93
#define OM_CANCEL_Y 141
#define OM_CANCEL_W  74
#define OM_CANCEL_H  43

#define OM_DIGI_X	  47
#define OM_DIGI_Y	  64
#define OM_DIGI_W	 212
#define OM_DIGI_H	  24

#define OM_DIGESTABILITY_X	  47
#define OM_DIGESTABILITY_Y	 104
#define OM_DIGESTABILITY_W	 212
#define OM_DIGESTABILITY_H	  24

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
