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

#include "graphics/thumbnail.h"
#include "m4/burger/gui/game_menu.h"
#include "m4/burger/gui/interface.h"
#include "m4/adv_r/other.h"
#include "m4/adv_r/adv_player.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/gui_menu_items.h"
#include "m4/gui/hotkeys.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/mem/mem.h"
#include "m4/platform/keys.h"
#include "m4/burger/hotkeys.h"
#include "m4/burger/vars.h"
#include "m4/m4.h"

#include "common/config-manager.h"

namespace M4 {
namespace Burger {
namespace GUI {

//-------------------------------------   GAME MENU   -------------------------------------//

enum game_menu_button_tags {
	GM_TAG_QUIT = 1,
	GM_TAG_OPTIONS = 2,
	GM_TAG_RESUME = 3,
	GM_TAG_SAVE = 4,
	GM_TAG_LOAD = 5,
	GM_TAG_MAIN = 6
};

#define GAME_MENU_X		190
#define GAME_MENU_Y		100
#define GAME_MENU_W		260
#define GAME_MENU_H		198

#define GM_MAIN_X			 45
#define GM_MAIN_Y			 53
#define GM_MAIN_W			 24
#define GM_MAIN_H			 24

#define GM_OPTIONS_X		 45
#define GM_OPTIONS_Y		 94
#define GM_OPTIONS_W		 24
#define GM_OPTIONS_H		 24

#define GM_RESUME_X		 45
#define GM_RESUME_Y		135
#define GM_RESUME_W		 24
#define GM_RESUME_H		 24

#define GM_QUIT_X			141
#define GM_QUIT_Y			135
#define GM_QUIT_W			 24
#define GM_QUIT_H		    24

#define GM_SAVE_X			141
#define GM_SAVE_Y			 53
#define GM_SAVE_W			 24
#define GM_SAVE_H		    24

#define GM_LOAD_X			141
#define GM_LOAD_Y			 94
#define GM_LOAD_W			 24
#define GM_LOAD_H			 24



void GameMenu::cb_Game_Quit(void *, void *) {
	// Destroy the game menu
	destroyMenu();

	// Shutdown the menu system
	guiMenu::shutdown(false);

	// Set the global that will cause the entire game to exit to dos
	_G(kernel).going = false;
}

void GameMenu::cb_Game_Resume(void *, void *) {
	// Destroy the game menu
	destroyMenu();

	// Shutdown the menu system
	guiMenu::shutdown(true);
}

void GameMenu::cb_Game_Save(void *, void *) {
	// Destroy the game menu
	destroyMenu();
	guiMenu::shutdown(true);
	_GM(buttonClosesDialog) = true;

	// Create the save game menu
	g_engine->showSaveScreen();
}

void GameMenu::cb_Game_Load(void *, void *) {
	// Destroy the game menu
	destroyMenu();
	guiMenu::shutdown(true);
	_GM(buttonClosesDialog) = true;

	// Create the save game menu
	g_engine->showLoadScreen(M4Engine::kLoadFromGameDialog);
}

void GameMenu::cb_Game_Main(void *, void *) {
	// Destroy the game menu
	destroyMenu();

	if (!_GM(gameMenuFromMain)) {
		// Save the game so we can resume from here if possible
		if (_GM(interfaceWasVisible) && player_commands_allowed()) {
			other_save_game_for_resurrection();
		}

		// Make sure the interface does not reappear
		_GM(interfaceWasVisible) = false;

		// Shutdown the menu system
		guiMenu::shutdown(false);
	} else {
		guiMenu::shutdown(true);
	}

	// Go to the main menu
	_G(game).setRoom(_G(executing) == WHOLE_GAME ? 903 : 901);
}

void GameMenu::cb_Game_Options(void *, void *) {
	// Destroy the game menu
	destroyMenu();
	_GM(buttonClosesDialog) = true;

	// Create the options menu
	OptionsMenu::show(nullptr);
}

void GameMenu::destroyMenu() {
	if (!_GM(gameMenu)) {
		return;
	}

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(gameMenu));

	// Destroy the menu resources
	guiMenu::destroy(_GM(gameMenu));

	// Unload the menu sprites
	guiMenu::unloadSprites();
}

void GameMenu::show(RGB8 *myPalette) {
	if (!_G(menuSystemInitialized)) {
		guiMenu::initialize(myPalette);
	}

	// Load in the game menu sprites
	if (!guiMenu::loadSprites("gamemenu", GM_TOTAL_SPRITES)) {
		return;
	}

	_GM(gameMenu) = guiMenu::create(_GM(menuSprites)[GM_DIALOG_BOX], GAME_MENU_X, GAME_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(gameMenu)) {
		return;
	}

	menuItemButton::add(_GM(gameMenu), GM_TAG_MAIN, GM_MAIN_X, GM_MAIN_Y, GM_MAIN_W, GM_MAIN_H, cb_Game_Main);
	menuItemButton::add(_GM(gameMenu), GM_TAG_OPTIONS, GM_OPTIONS_X, GM_OPTIONS_Y, GM_OPTIONS_W, GM_OPTIONS_H, cb_Game_Options);
	menuItemButton::add(_GM(gameMenu), GM_TAG_RESUME, GM_RESUME_X, GM_RESUME_Y, GM_RESUME_W, GM_RESUME_H, cb_Game_Resume);
	menuItemButton::add(_GM(gameMenu), GM_TAG_QUIT, GM_QUIT_X, GM_QUIT_Y, GM_QUIT_W, GM_QUIT_H, cb_Game_Quit);

	if (!_GM(gameMenuFromMain)) {
		menuItemButton::add(_GM(gameMenu), GM_TAG_SAVE, GM_SAVE_X, GM_SAVE_Y, GM_SAVE_W, GM_SAVE_H, cb_Game_Save);
	} else {
		menuItemButton::add(_GM(gameMenu), GM_TAG_SAVE, GM_SAVE_X, GM_SAVE_Y, GM_SAVE_W, GM_SAVE_H, cb_Game_Save, menuItemButton::BTN_TYPE_GM_GENERIC, true);
	}

	// See if there are any games to load
	if (g_engine->savesExist()) {
		menuItemButton::add(_GM(gameMenu), GM_TAG_LOAD, GM_LOAD_X, GM_LOAD_Y, GM_LOAD_W, GM_LOAD_H, cb_Game_Load);
	} else {
		menuItemButton::add(_GM(gameMenu), GM_TAG_LOAD, GM_LOAD_X, GM_LOAD_Y, GM_LOAD_W, GM_LOAD_H, cb_Game_Load, menuItemButton::BTN_TYPE_GM_GENERIC, true);
	}

	// Configure the game so pressing <esc> will cause the menu to disappear and the game to resume
	guiMenu::configure(_GM(gameMenu), cb_Game_Resume, cb_Game_Resume);

	vmng_screen_show((void *)_GM(gameMenu));
	LockMouseSprite(0);
}


//----------------------------------   OPTIONS MENU   -------------------------------------//

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

void OptionsMenu::cb_Options_Game_Cancel(void *, void *) {
	// Reset values of items to what they were when options menu came up
	setDigiVolume(_GM(remember_digi_volume));
	_G(flags)[digestability] = _GM(remember_digestability);

	// Destroy the options menu
	destroyMenu();
	_GM(buttonClosesDialog) = true;

	// Create the options menu
	GameMenu::show(nullptr);
}

void OptionsMenu::cb_Options_Game_Done(void *, void *) {
	// Destroy the options menu
	destroyMenu();
	_GM(buttonClosesDialog) = true;

	// Create the options menu
	GameMenu::show(nullptr);
}

void OptionsMenu::cb_Options_Digi(menuItemHSlider *myItem, guiMenu *myMenu) {
	// Set the digi volume
	setDigiVolumePerc(myItem->percent);
	term_message("digi volume: %d", myItem->percent);

	// This scroller control has been moved, so make sure that the DONE button is not greyed out
	menuItemButton::enableButton(nullptr, OM_TAG_DONE, myMenu);
	guiMenu::itemRefresh(nullptr, OM_TAG_DONE, myMenu);

}

void OptionsMenu::cb_Options_Digestability(menuItemHSlider *myItem, guiMenu *myMenu) {
	term_message("digestability: %d", myItem->percent);
	_G(flags)[digestability] = myItem->percent;

	// This scroller control has been moved, so make sure that the DONE button is not greyed out
	menuItemButton::enableButton(nullptr, OM_TAG_DONE, myMenu);
	guiMenu::itemRefresh(nullptr, OM_TAG_DONE, myMenu);
}

void OptionsMenu::destroyMenu(void) {
	if (!_GM(opMenu))
		return;

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(opMenu));

	// Destroy the menu resources
	guiMenu::destroy(_GM(opMenu));

	// Unload the menu sprites
	guiMenu::unloadSprites();
}

void OptionsMenu::show(RGB8 *myPalette) {
	if (!_G(menuSystemInitialized)) {
		guiMenu::initialize(myPalette);
	}

	// Load in the game menu sprites
	if (!guiMenu::loadSprites("opmenu", OM_TOTAL_SPRITES)) {
		return;
	}

	_GM(opMenu) = guiMenu::create(_GM(menuSprites)[OM_DIALOG_BOX], OPTIONS_MENU_X, OPTIONS_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(opMenu)) {
		return;
	}

	menuItemButton::add(_GM(opMenu), OM_TAG_CANCEL, OM_CANCEL_X, OM_CANCEL_Y, OM_CANCEL_W, OM_CANCEL_H, cb_Options_Game_Cancel, menuItemButton::BTN_TYPE_OM_CANCEL);
	menuItemButton::add(_GM(opMenu), OM_TAG_DONE, OM_DONE_X, OM_DONE_Y, OM_DONE_W, OM_DONE_H, cb_Options_Game_Done, menuItemButton::BTN_TYPE_OM_DONE, true);
	menuItemHSlider::add(_GM(opMenu), OM_TAG_DIGI, OM_DIGI_X, OM_DIGI_Y, OM_DIGI_W, OM_DIGI_H, getDigiVolumePerc(),
		(CALLBACK)cb_Options_Digi, true);
	menuItemHSlider::add(_GM(opMenu), OM_TAG_DIGESTABILITY, OM_DIGESTABILITY_X, OM_DIGESTABILITY_Y,
		OM_DIGESTABILITY_W, OM_DIGESTABILITY_H, _G(flags)[digestability],
		(CALLBACK)cb_Options_Digestability, true);

	// Remember the values of the items in case the user cancels
	_GM(remember_digi_volume) = getDigiVolume();
	_GM(remember_digestability) = _G(flags)[digestability];

	// Configure the game so pressing <esc> will cause the menu to disappear and the gamemenu to reappear
	guiMenu::configure(_GM(opMenu), cb_Options_Game_Done, cb_Options_Game_Cancel);

	vmng_screen_show((void *)_GM(opMenu));
	LockMouseSprite(0);
}

void OptionsMenu::setDigiVolume(uint16 volume) {
	ConfMan.setInt("sfx_volume", volume);
	g_engine->syncSoundSettings();
}

void OptionsMenu::setDigiVolumePerc(uint8 volumePerc) {
	setDigiVolume(volumePerc * 256 / 100);
}

uint16 OptionsMenu::getDigiVolume() {
	return ConfMan.getInt("sfx_volume");
}

uint8 OptionsMenu::getDigiVolumePerc() {
	return getDigiVolume() * 100 / 256;
}

//------------------------------------------------------------------------------------//
//--------------------------------   ERR MENU   --------------------------------------//
//------------------------------------------------------------------------------------//

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


void ErrorMenu::cb_Err_Done(void *, void *) {
	// Destroy the game menu
	destroyMenu();

	// Shutdown the menu system
	guiMenu::shutdown(true);
}

void ErrorMenu::destroyMenu() {
	if (!_GM(errMenu)) {
		return;
	}

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(errMenu));

	// Destroy the menu resources
	guiMenu::destroy(_GM(errMenu));

	// Unload the menu sprites
	guiMenu::unloadSprites();
}

void ErrorMenu::show(RGB8 *myPalette) {
	if (!_G(menuSystemInitialized)) {
		guiMenu::initialize(myPalette);
	}

	// Load in the game menu sprites
	if (!guiMenu::loadSprites("errmenu", 5)) {
		return;
	}

	_GM(errMenu) = guiMenu::create(_GM(menuSprites)[EM_DIALOG_BOX], ERROR_MENU_X, ERROR_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(errMenu)) {
		return;
	}

	// Get the menu buffer
	Buffer *myBuff = _GM(errMenu)->menuBuffer->get_buffer();
	if (!myBuff) {
		return;
	}

	//write the err message
	gr_font_set_color(menuItem::TEXT_COLOR_NORM_FOREGROUND);
	gr_font_write(myBuff, "Save game failed!", 48, 8, 0, -1);

	gr_font_write(myBuff, "A disk error has", 48, 23, 0, -1);
	gr_font_write(myBuff, "occurred.", 48, 33, 0, -1);

	gr_font_write(myBuff, "Please ensure you", 48, 48, 0, -1);
	gr_font_write(myBuff, "have write access", 48, 58, 0, -1);
	gr_font_write(myBuff, "and sufficient", 48, 68, 0, -1);
	gr_font_write(myBuff, "disk space (40k).", 48, 78, 0, -1);

	_GM(errMenu)->menuBuffer->release();

	// Add the done button
	menuItemButton::add(_GM(errMenu), EM_TAG_RETURN, EM_RETURN_X, EM_RETURN_Y, EM_RETURN_W, EM_RETURN_H, cb_Err_Done);

	// Configure the game so pressing <esc> will cause the menu to disappear and the gamemenu to reappear
	guiMenu::configure(_GM(errMenu), cb_Err_Done, cb_Err_Done);

	vmng_screen_show((void *)_GM(errMenu));
	LockMouseSprite(0);
}

//--------------------------------   SAVE / LOAD MENU   -----------------------------------//

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


void SaveLoadMenu::show(RGB8 *myPalette, bool saveMenu) {
	ItemHandlerFunction	i_handler;
	bool buttonGreyed;

	if (!_G(menuSystemInitialized)) {
		guiMenu::initialize(myPalette);
	}

	// Load in the game menu sprites
	if (!guiMenu::loadSprites("slmenu", GUI::SaveLoadMenuBase::SL_TOTAL_SPRITES)) {
		return;
	}

	// Initialize some global vars
	_GM(firstSlotIndex) = 0;
	_GM(slotSelected) = -1;
	_GM(saveLoadThumbNail) = nullptr;
	_GM(thumbIndex) = 100;
	_GM(currMenuIsSave) = saveMenu;

	_GM(slMenu) = guiMenu::create(_GM(menuSprites)[GUI::SaveLoadMenuBase::SL_DIALOG_BOX], SAVE_LOAD_MENU_X, SAVE_LOAD_MENU_Y,
		MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	if (!_GM(slMenu)) {
		return;
	}

	if (_GM(currMenuIsSave)) {
		menuItemMsg::msgAdd(_GM(slMenu), SL_TAG_SAVE_LABEL, SL_SAVE_LABEL_X, SL_SAVE_LABEL_Y, SL_SAVE_LABEL_W, SL_SAVE_LABEL_H);
		menuItemButton::add(_GM(slMenu), SL_TAG_SAVE, SL_SAVE_X, SL_SAVE_Y, SL_SAVE_W, SL_SAVE_H,
			(CALLBACK)cb_SaveLoad_Save, menuItemButton::BTN_TYPE_SL_SAVE, true);
	} else {
		menuItemMsg::msgAdd(_GM(slMenu), SL_TAG_LOAD_LABEL, SL_LOAD_LABEL_X, SL_LOAD_LABEL_Y, SL_LOAD_LABEL_W, SL_LOAD_LABEL_H);
		menuItemButton::add(_GM(slMenu), SL_TAG_LOAD, SL_LOAD_X, SL_LOAD_Y, SL_LOAD_W, SL_LOAD_H,
			(CALLBACK)cb_SaveLoad_Load, menuItemButton::BTN_TYPE_SL_LOAD, true);
	}

	menuItemButton::add(_GM(slMenu), SL_TAG_CANCEL, SL_CANCEL_X, SL_CANCEL_Y, SL_CANCEL_W, SL_CANCEL_H,
		(CALLBACK)cb_SaveLoad_Cancel, menuItemButton::BTN_TYPE_SL_CANCEL);

	menuItemVSlider::add(_GM(slMenu), SL_TAG_VSLIDER, SL_SLIDER_X, SL_SLIDER_Y, SL_SLIDER_W, SL_SLIDER_H,
		0, (CALLBACK)cb_SaveLoad_VSlider);

	initializeSlotTables();

	if (_GM(currMenuIsSave)) {
		buttonGreyed = false;
		i_handler = (ItemHandlerFunction)menuItemButton::handler;
	} else {
		buttonGreyed = true;
		i_handler = (ItemHandlerFunction)load_Handler;
	}

	for (int32 i = 0; i < MAX_SLOTS_SHOWN; i++) {
		menuItemButton::add(_GM(slMenu), 1001 + i,
			SL_SCROLL_FIELD_X, SL_SCROLL_FIELD_Y + i * SL_SCROLL_LINE_H,
			SL_SCROLL_LINE_W, SL_SCROLL_LINE_H,
			(CALLBACK)cb_SaveLoad_Slot, menuItemButton::BTN_TYPE_SL_TEXT,
			buttonGreyed && (!_GM(slotInUse)[i]), true, _GM(slotTitles)[i], i_handler);
	}

	if (_GM(currMenuIsSave)) {
		// Create thumbnails. One in the original game format for displaying,
		// and the other in the ScummVM format for actually using in the save files
		_GM(saveLoadThumbNail) = menu_CreateThumbnail(&_GM(sizeofThumbData));
		_GM(_thumbnail).free();
		Graphics::createThumbnail(_GM(_thumbnail));

	} else {
		updateThumbnails(0, _GM(slMenu));
		_GM(saveLoadThumbNail) = _GM(menuSprites)[GUI::SaveLoadMenuBase::SL_EMPTY_THUMB_25];
	}

	menuItemMsg::msgAdd(_GM(slMenu), SL_TAG_THUMBNAIL, SL_THUMBNAIL_X, SL_THUMBNAIL_Y,
		M4::GUI::SaveLoadMenuBase::SL_THUMBNAIL_W,
		M4::GUI::SaveLoadMenuBase::SL_THUMBNAIL_H, false);

	if (_GM(currMenuIsSave)) {
		//<return> - if a slot has been selected, saves the game
		//<esc> - cancels and returns to the game menu
		guiMenu::configure(_GM(slMenu), (CALLBACK)cb_SaveLoad_Save, (CALLBACK)cb_SaveLoad_Cancel);
	} else {
		//<return> - if a slot has been selected, loads the selected game
		//<esc> - cancels and returns to the game menu
		guiMenu::configure(_GM(slMenu), (CALLBACK)cb_SaveLoad_Load, (CALLBACK)cb_SaveLoad_Cancel);
	}

	vmng_screen_show((void *)_GM(slMenu));
	LockMouseSprite(0);
}

void SaveLoadMenu::destroyMenu(bool saveMenu) {
	if (!_GM(slMenu)) {
		return;
	}

	// Determine whether the screen was the SAVE or the LOAD menu
	if (saveMenu) {

		// If SAVE, there should be a thumbnail to unload
		if (_GM(saveLoadThumbNail)) {
			DisposeHandle(_GM(saveLoadThumbNail)->sourceHandle);
			mem_free(_GM(saveLoadThumbNail));
			_GM(saveLoadThumbNail) = nullptr;
		}
	} else {
		// Else there may be up to 10 somewhere in the list to be unloaded
		for (int32 i = 0; i < MAX_SLOTS; i++) {
			unloadThumbnail(i);
		}
		_GM(saveLoadThumbNail) = nullptr;
	}

	// Destroy the screen
	vmng_screen_dispose(_GM(slMenu));
	guiMenu::destroy(_GM(slMenu));

	// Unload the save/load menu sprites
	guiMenu::unloadSprites();
}

void SaveLoadMenu::cb_SaveLoad_VSlider(menuItemVSlider *myItem, guiMenu *myMenu) {
	if (!myMenu || !myItem)
		return;

	if ((myItem->itemFlags & menuItemVSlider::VS_COMPONENT) != menuItemVSlider::VS_THUMB) {
		bool redraw = (DrawFunction)false;
		switch (myItem->itemFlags & menuItemVSlider::VS_COMPONENT) {
		case menuItemVSlider::VS_UP:
			if (_GM(firstSlotIndex) > 0) {
				_GM(firstSlotIndex)--;
				redraw = (DrawFunction)true;
			}
			break;

		case menuItemVSlider::VS_PAGE_UP:
			if (_GM(firstSlotIndex) > 0) {
				_GM(firstSlotIndex) = imath_max(_GM(firstSlotIndex) - 10, 0);
				redraw = (DrawFunction)true;
			}
			break;

		case menuItemVSlider::VS_PAGE_DOWN:
			if (_GM(firstSlotIndex) < 89) {
				_GM(firstSlotIndex) = imath_min(_GM(firstSlotIndex) + 10, 89);
				redraw = (DrawFunction)true;
			}
			break;

		case menuItemVSlider::VS_DOWN:
			if (_GM(firstSlotIndex) < 89) {
				_GM(firstSlotIndex)++;
				redraw = (DrawFunction)true;
			}
			break;
		}

		// See if we were able to set a new first slot index
		if (redraw) {
			setFirstSlot(_GM(firstSlotIndex), myMenu);

			// Calculate the new percent
			myItem->percent = (_GM(firstSlotIndex) * 100) / 89;

			// Calculate the new thumbY
			myItem->thumbY = myItem->minThumbY +
				((myItem->percent * (myItem->maxThumbY - myItem->minThumbY)) / 100);

			// Redraw the slider
			guiMenu::itemRefresh(myItem, -1, myMenu);
		}
	}

	// Else the callback came from the thumb - set the _GM(firstSlotIndex) based on the slider percent
	else {
		_GM(firstSlotIndex) = (myItem->percent * 89) / 100;
		setFirstSlot(_GM(firstSlotIndex), myMenu);
	}
}

void SaveLoadMenu::cb_SaveLoad_Save(void *, guiMenu *myMenu) {
	// If (slotSelected < 0) this callback is being executed by pressing return prematurely
	if (_GM(slotSelected) < 0) {
		return;
	}

	// First make the textfield NORM
	menuItemTextField *myText = (menuItemTextField *)guiMenu::getItem(2000, myMenu);
	if (!myText)
		return;

	myText->itemFlags = menuItemTextField::TF_NORM;

	// Set the vars
	_GM(slotInUse)[_GM(slotSelected) - 1] = true;
	Common::strcpy_s(_GM(slotTitles)[_GM(slotSelected) - 1], 80, myText->prompt);

	// Save the game
	bool saveGameFailed = !g_engine->saveGameFromMenu(_GM(slotSelected),
	                                                  myText->prompt, _GM(_thumbnail));

	// If the save game failed, bring up the err menu
	if (saveGameFailed) {
		// Kill the save menu
		destroyMenu(true);

		// Create the err menu
		ErrorMenu::show(nullptr);

		// Abort this procedure
		return;
	}

	// Kill the save menu
	destroyMenu(true);

	// Shutdown the menu system
	guiMenu::shutdown(true);
}

void SaveLoadMenu::cb_SaveLoad_Load(menuItemButton *, guiMenu *) {
	// If (slotSelected < 0) this callback is being executed by pressing return prematurely
	if (_GM(slotSelected) < 0) {
		return;
	}

	// Kill the menu
	destroyMenu(false);

	// Shutdown the menu system
	guiMenu::shutdown(false);

	// See if we need to reset the ESC, F2, and F3 hotkeys
	if (_GM(gameMenuFromMain)) {
		AddSystemHotkey(KEY_ESCAPE, Burger::Hotkeys::escape_key_pressed);
		AddSystemHotkey(KEY_F2, M4::Hotkeys::saveGame);
		AddSystemHotkey(KEY_F3, M4::Hotkeys::loadGame);
	}

	// Start the restore process
	_G(kernel).restore_slot = _GM(slotSelected);
	const KernelTriggerType oldMode = _G(kernel).trigger_mode;

	_G(kernel).trigger_mode = KT_DAEMON;
	kernel_trigger_dispatch_now(TRIG_RESTORE_GAME);
	_G(kernel).trigger_mode = oldMode;
}

void SaveLoadMenu::cb_SaveLoad_Cancel(menuItemButton *, guiMenu *myMenu) {
	// If a slot has been selected, cancel will re-enable all slots
	if (_GM(slotSelected) >= 0) {
		// Enable the prev buttons
		for (int32 i = 1001; i <= 1010; i++) {
			if (_GM(currMenuIsSave) || _GM(slotInUse)[i - 1001 + _GM(firstSlotIndex)]) {
				menuItemButton::enableButton(nullptr, i, myMenu);
				guiMenu::itemRefresh(nullptr, i, myMenu);
			}
		}

		// Find the textfield and use it's coords to place the button
		menuItem *myItem = guiMenu::getItem(2000, myMenu);
		int32 x = myItem->x1;
		int32 y = myItem->y1;
		int32 w = myItem->x2 - myItem->x1 + 1;
		int32 h = myItem->y2 - myItem->y1 + 1;

		// Delete the textfield
		guiMenu::itemDelete(myItem, 2000, myMenu);

		// Add the button back in
		if (_GM(currMenuIsSave)) {
			menuItemButton::add(myMenu, 1000 + _GM(slotSelected) - _GM(firstSlotIndex), x, y, w, h,
				(CALLBACK)cb_SaveLoad_Slot, menuItemButton::BTN_TYPE_SL_TEXT, false, true, _GM(slotTitles)[_GM(slotSelected) - 1]);
		} else {
			menuItemButton::add(myMenu, 1000 + _GM(slotSelected) - _GM(firstSlotIndex), x, y, w, h,
				(CALLBACK)cb_SaveLoad_Slot, menuItemButton::BTN_TYPE_SL_TEXT, false, true, _GM(slotTitles)[_GM(slotSelected) - 1],
				(ItemHandlerFunction)load_Handler);

			// Remove the thumbnail
			if (_GM(saveLoadThumbNail)) {
				_GM(saveLoadThumbNail) = _GM(menuSprites)[GUI::SaveLoadMenuBase::SL_EMPTY_THUMB_25];
				guiMenu::itemRefresh(nullptr, SL_TAG_THUMBNAIL, myMenu);
			}
		}
		setFirstSlot(_GM(firstSlotIndex), myMenu);

		// Enable the slider
		menuItemVSlider::enableVSlider(nullptr, SL_TAG_VSLIDER, myMenu);
		guiMenu::itemRefresh(nullptr, SL_TAG_VSLIDER, myMenu);

		// Disable the save/load button
		if (_GM(currMenuIsSave)) {
			menuItemButton::disableButton(nullptr, SL_TAG_SAVE, myMenu);
			guiMenu::itemRefresh(nullptr, SL_TAG_SAVE, myMenu);
		} else {
			menuItemButton::disableButton(nullptr, SL_TAG_LOAD, myMenu);
			guiMenu::itemRefresh(nullptr, SL_TAG_LOAD, myMenu);
		}

		// Reset the slot selected var
		_GM(slotSelected) = -1;

	} else {
		// Otherwise, back to the game menu

		// Destroy the menu
		destroyMenu(_GM(currMenuIsSave));

		if (_GM(saveLoadFromHotkey)) {
			// Shutdown the menu system
			guiMenu::shutdown(true);
		} else {
			// Create the game menu
			GameMenu::show(nullptr);
		}
	}

	_GM(buttonClosesDialog) = true;
}

void SaveLoadMenu::cb_SaveLoad_Slot(menuItemButton *myButton, guiMenu *myMenu) {
	char prompt[80];

	// Verify params
	if (!myMenu || !myButton)
		return;

	// Get the button
	Common::strcpy_s(prompt, 80, myButton->prompt);
	int32 specialTag = myButton->specialTag;

	// Set the globals
	_GM(slotSelected) = myButton->specialTag;
	_GM(deleteSaveDesc) = true;

	// Disable all other buttons
	for (int32 i = 1001; i <= 1010; i++) {
		if (i != myButton->tag) {
			menuItemButton::disableButton(nullptr, i, myMenu);
			guiMenu::itemRefresh(nullptr, i, myMenu);
		}
	}

	// Get the slot coords, and delete it
	int32 x = myButton->x1;
	int32 y = myButton->y1;
	int32 w = myButton->x2 - myButton->x1 + 1;
	int32 h = myButton->y2 - myButton->y1 + 1;
	guiMenu::itemDelete(myButton, -1, myMenu);

	if (_GM(currMenuIsSave)) {
		// Replace the current button with a textfield
		if (!strcmp(prompt, "<empty>")) {
			menuItemTextField::add(myMenu, 2000, x, y, w, h, menuItemTextField::TF_OVER,
				nullptr, specialTag, (CALLBACK)cb_SaveLoad_Save, true);
		} else {
			menuItemTextField::add(myMenu, 2000, x, y, w, h, menuItemTextField::TF_OVER,
				prompt, specialTag, (CALLBACK)cb_SaveLoad_Save, true);
		}
	} else {
		menuItemTextField::add(myMenu, 2000, x, y, w, h, menuItemTextField::TF_NORM,
			prompt, specialTag, (CALLBACK)cb_SaveLoad_Load, true);
	}

	// Disable the slider
	menuItemVSlider::disableVSlider(nullptr, SL_TAG_VSLIDER, myMenu);
	guiMenu::itemRefresh(nullptr, SL_TAG_VSLIDER, myMenu);

	// Enable the save/load button
	if (_GM(currMenuIsSave)) {
		menuItemButton::enableButton(nullptr, SL_TAG_SAVE, myMenu);
		guiMenu::itemRefresh(nullptr, SL_TAG_SAVE, myMenu);
	} else {
		menuItemButton::enableButton(nullptr, SL_TAG_LOAD, myMenu);
		guiMenu::itemRefresh(nullptr, SL_TAG_LOAD, myMenu);
	}
}

bool SaveLoadMenu::load_Handler(menuItemButton *myItem, int32 eventType, int32 event, int32 x, int32 y, void **currItem) {
	// Handle the event just like any other button
	bool handled = menuItemButton::handler(myItem, eventType, event, x, y, currItem);

	// If we've selected a slot, we want the thumbNail to remain on the menu permanently
	if (_GM(slotSelected) >= 0) {
		return handled;
	}

	// But if the event moved the mouse, we want to display the correct thumbNail;
	if ((eventType == EVENT_MOUSE) && ((event == _ME_move) || (event == _ME_L_drag) || (event == _ME_L_release) ||
		(event == _ME_doubleclick_drag) || (event == _ME_doubleclick_release))) {

		// Get the button
		if (!myItem)
			return handled;

		// This determines that we are over the button
		if ((myItem->itemFlags == menuItemButton::BTN_STATE_OVER) || (myItem->itemFlags == menuItemButton::BTN_STATE_PRESS)) {
			// See if the current _GM(saveLoadThumbNail) is pointing to the correct sprite
			if (_GM(saveLoadThumbNail) != _GM(thumbNails)[myItem->specialTag - 1]) {
				_GM(saveLoadThumbNail) = _GM(thumbNails)[myItem->specialTag - 1];
				guiMenu::itemRefresh(nullptr, SL_TAG_THUMBNAIL, (guiMenu *)myItem->myMenu);
			}
		}

		// Else we must determine whether the thumbnail needs to be replaced with the empty thumbnail.
		else {

			// If the mouse has moved outside of the entire range of all 10 buttons,
			//or it is over a button which is not hilited it is to be removed.
			if (menuItem::cursorInsideItem(myItem, x, y)
				|| (x < SL_SCROLL_FIELD_X)
				|| (x > SL_SCROLL_FIELD_X + SL_SCROLL_FIELD_W)
				|| (y < SL_SCROLL_FIELD_Y)
				|| (y > SL_SCROLL_FIELD_Y + SL_SCROLL_FIELD_H)) {

				// Remove the thumbnail
				if (_GM(saveLoadThumbNail)) {
					_GM(saveLoadThumbNail) = _GM(menuSprites)[GUI::SaveLoadMenuBase::SL_EMPTY_THUMB_25];
					guiMenu::itemRefresh(nullptr, SL_TAG_THUMBNAIL, (guiMenu *)myItem->myMenu);
				}
			}
		}
	}

	return handled;
}


void CreateGameMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(gameMenuFromMain) = false;
	GameMenu::show(myPalette);
}

void CreateGameMenuFromMain(RGB8 *myPalette) {
	if (_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(gameMenuFromMain) = true;
	GameMenu::show(myPalette);
}

void CreateSaveMenu(RGB8 *myPalette) {
	_GM(saveLoadFromHotkey) = false;
	SaveLoadMenu::show(myPalette, true);
}

void CreateF2SaveMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(saveLoadFromHotkey) = true;
	_GM(gameMenuFromMain) = false;
	SaveLoadMenu::show(myPalette, true);
}

void CreateLoadMenu(RGB8 *myPalette) {
	_GM(saveLoadFromHotkey) = false;
	SaveLoadMenu::show(myPalette, false);
}

void CreateF3LoadMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(saveLoadFromHotkey) = true;
	_GM(gameMenuFromMain) = false;
	SaveLoadMenu::show(myPalette, false);
}

void CreateLoadMenuFromMain(RGB8 *myPalette) {
	if (_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	_GM(saveLoadFromHotkey) = true;
	_GM(gameMenuFromMain) = true;
	SaveLoadMenu::show(myPalette, false);
}

} // namespace GUI
} // namespace Burger
} // namespace M4
