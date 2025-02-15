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

#include "m4/riddle/gui/game_menu.h"
#include "m4/riddle/vars.h"
#include "graphics/thumbnail.h"
#include "m4/adv_r/other.h"
#include "m4/adv_r/adv_background.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_player.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/gui/gui_event.h"
#include "m4/gui/hotkeys.h"
#include "m4/graphics/gr_line.h"
#include "m4/graphics/gr_sprite.h"
#include "m4/graphics/krn_pal.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/mem/mem.h"
#include "m4/platform/keys.h"
#include "m4/m4.h"

#include "m4/burger/gui/game_menu.h"

namespace M4 {
namespace Riddle {
namespace GUI {

/*-------------------- GAME MENU --------------------*/

#define GAME_MENU_X		212
#define GAME_MENU_Y		160

#define GM_TAG_QUIT     1
#define GM_QUIT_X		13
#define GM_QUIT_Y		31
#define GM_QUIT_W		26
#define GM_QUIT_H		26

#define GM_TAG_MAIN     2
#define GM_MAIN_X		108
#define GM_MAIN_Y		31
#define GM_MAIN_W		26
#define GM_MAIN_H		26

#define GM_TAG_OPTIONS	3
#define GM_OPTIONS_X	162
#define GM_OPTIONS_Y	31
#define GM_OPTIONS_W	26
#define GM_OPTIONS_H	26

#define GM_TAG_RESUME	4
#define GM_RESUME_X		54
#define GM_RESUME_Y		94
#define GM_RESUME_W		26
#define GM_RESUME_H		26

#define GM_TAG_SAVE		5
#define GM_SAVE_X		108
#define GM_SAVE_Y		94
#define GM_SAVE_W		26
#define GM_SAVE_H		26

#define GM_TAG_LOAD		6
#define GM_LOAD_X		162
#define GM_LOAD_Y		94
#define GM_LOAD_W		26
#define GM_LOAD_H		26

void GameMenu::show(RGB8 *myPalette) {
	if (!_G(menuSystemInitialized)) {
		guiMenu::initialize(myPalette);
	}

	// Keep the memory tidy
	PurgeMem();
	CompactMem();

	// Load in the game menu sprites
	if (!guiMenu::loadSprites("gamemenu", GM_TOTAL_SPRITES))
		error("Error loading gamemenu");

	_GM(gameMenu) = guiMenu::create(_GM(menuSprites)[GM_DIALOG_BOX],
		GAME_MENU_X, GAME_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	assert(_GM(gameMenu));

	menuItemButton::add(_GM(gameMenu), GM_TAG_QUIT,
		GM_QUIT_X, GM_QUIT_Y, GM_QUIT_W, GM_QUIT_H, cbQuitGame);
	menuItemButton::add(_GM(gameMenu), GM_TAG_MAIN,
		GM_MAIN_X, GM_MAIN_Y, GM_MAIN_W, GM_MAIN_H, cbMainMenu);
	menuItemButton::add(_GM(gameMenu), GM_TAG_OPTIONS, GM_OPTIONS_X, GM_OPTIONS_Y, GM_OPTIONS_W, GM_OPTIONS_H, cbOptions);
	menuItemButton::add(_GM(gameMenu), GM_TAG_RESUME, GM_RESUME_X, GM_RESUME_Y, GM_RESUME_W, GM_RESUME_H, cbResume);

	if (!_GM(gameMenuFromMain)) {
		menuItemButton::add(_GM(gameMenu), GM_TAG_SAVE, GM_SAVE_X, GM_SAVE_Y, GM_SAVE_W, GM_SAVE_H, cbSave);
	} else {
		menuItemButton::add(_GM(gameMenu), GM_TAG_SAVE, GM_SAVE_X, GM_SAVE_Y, GM_SAVE_W, GM_SAVE_H, cbSave, menuItemButton::BTN_TYPE_GM_GENERIC, true);
	}

	// See if there are any games to load
	if (g_engine->savesExist()) {
		menuItemButton::add(_GM(gameMenu), GM_TAG_LOAD, GM_LOAD_X, GM_LOAD_Y, GM_LOAD_W, GM_LOAD_H, cbLoad);
	} else {
		menuItemButton::add(_GM(gameMenu), GM_TAG_LOAD, GM_LOAD_X, GM_LOAD_Y, GM_LOAD_W, GM_LOAD_H, cbLoad, menuItemButton::BTN_TYPE_GM_GENERIC, true);
	}

	// Configure the game so pressing <esc> will cause the menu to disappear and the game to resume
	guiMenu::configure(_GM(gameMenu), cbResume, cbResume);

	vmng_screen_show((void *)_GM(gameMenu));
	LockMouseSprite(0);
}

void GameMenu::destroyGameMenu() {
	if (!_GM(gameMenu))
		return;

	// Remove the screen from the gui
	vmng_screen_dispose(_GM(gameMenu));

	// Destroy the menu resources
	guiMenu::destroy(_GM(gameMenu));

	// Unload the menu sprites
	guiMenu::unloadSprites();
}

void GameMenu::cbQuitGame(void *, void *) {
	// Destroy the game menu
	destroyGameMenu();

	// Shutdown the menu system
	guiMenu::shutdown(false);

	// Set the global that will cause the entire game to exit to dos
	_G(kernel).going = false;
}

void GameMenu::cbMainMenu(void *, void *) {
	// Destroy the game menu
	destroyGameMenu();

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
	_G(game).setRoom(494);
}

void GameMenu::cbResume(void *, void *) {
	// Destroy the game menu
	destroyGameMenu();

	// Shutdown the menu system
	guiMenu::shutdown(true);
}

void GameMenu::cbOptions(void *, void *) {
	// Destroy the game menu
	destroyGameMenu();

	_GM(buttonClosesDialog) = true;

	// Create the options menu
	OptionsMenu::show();
}

void GameMenu::cbSave(void *, void *) {
	// TODO
}

void GameMenu::cbLoad(void *, void *) {
	// TODO
}

/*-------------------- OPTIONS MENU --------------------*/

#define OPTIONS_MENU_X 212
#define OPTIONS_MENU_Y 160

#define OM_TAG_GAMEMENU		1
#define OM_GAMEMENU_X		14
#define OM_GAMEMENU_Y		94
#define OM_GAMEMENU_W		26
#define OM_GAMEMENU_H		26

#define OM_TAG_DIGI			2
#define OM_DIGI_X			70
#define OM_DIGI_Y			29
#define OM_DIGI_W			108
#define OM_DIGI_H			14

#define OM_TAG_MIDI			3
#define OM_MIDI_X			70
#define OM_MIDI_Y			72
#define OM_MIDI_W			108
#define OM_MIDI_H			14

#define OM_TAG_SCROLLING	4
#define OM_SCROLLING_X		131
#define OM_SCROLLING_Y		113
#define OM_SCROLLING_W		39
#define OM_SCROLLING_H		39

void OptionsMenu::show() {
	// Load in the options menu sprites
	if (!guiMenu::loadSprites("opmenu", OM_TOTAL_SPRITES))
		error("Error loading opmenu");

	_GM(opMenu) = guiMenu::create(_GM(menuSprites)[OM_DIALOG_BOX],
		OPTIONS_MENU_X, OPTIONS_MENU_Y, MENU_DEPTH | SF_GET_ALL | SF_BLOCK_ALL | SF_IMMOVABLE);
	assert(_GM(opMenu));

	int digiPercent = digi_get_overall_volume();
	int midiPercent = midi_get_overall_volume();

	menuItemButton::add(_GM(opMenu), OM_TAG_GAMEMENU,
		OM_GAMEMENU_X, OM_GAMEMENU_Y, OM_GAMEMENU_W, OM_GAMEMENU_H,
		cbGameMenu, menuItemButton::BTN_TYPE_GM_GENERIC);
	menuItemHSlider::add(_GM(opMenu), OM_TAG_DIGI,
		OM_DIGI_X, OM_DIGI_Y, OM_DIGI_W, OM_DIGI_H,
		digiPercent, (CALLBACK)cbSetDigi, true);
	menuItemHSlider::add(_GM(opMenu), OM_TAG_MIDI,
		OM_MIDI_X, OM_MIDI_Y, OM_MIDI_W, OM_MIDI_H,
		midiPercent, (CALLBACK)cbSetMidi, true);
	menuItemButton::add(_GM(opMenu), OM_TAG_SCROLLING,
		OM_SCROLLING_X, OM_SCROLLING_Y, OM_SCROLLING_W, OM_SCROLLING_H, (CALLBACK)cbScrolling,
		_G(kernel).cameraPans() ? menuItemButton::BTN_TYPE_OM_SCROLLING_ON :
			menuItemButton::BTN_TYPE_OM_SCROLLING_OFF);

	guiMenu::configure(_GM(opMenu), cbGameMenu, cbGameMenu);
	vmng_screen_show((void *)_GM(opMenu));
	LockMouseSprite(0);
}

void OptionsMenu::destroyOptionsMenu() {
	// Remove the screen from the gui
	vmng_screen_dispose(_GM(opMenu));

	// Destroy the menu resources
	guiMenu::destroy(_GM(opMenu));

	// Unload the menu sprites
	guiMenu::unloadSprites();
}

void OptionsMenu::cbGameMenu(void *, void *) {
	destroyOptionsMenu();

	GameMenu::show(nullptr);
}

void OptionsMenu::cbScrolling(M4::GUI::menuItemButton *myItem, M4::GUI::guiMenu *) {
	_G(kernel).camera_pan_instant = myItem->buttonType ==
		menuItemButton::BTN_TYPE_OM_SCROLLING_ON;
}

void OptionsMenu::cbSetDigi(M4::GUI::menuItemHSlider *myItem, M4::GUI::guiMenu *) {
	digi_set_overall_volume(myItem->percent);
}

void OptionsMenu::cbSetMidi(M4::GUI::menuItemHSlider *myItem, M4::GUI::guiMenu *) {
	midi_set_overall_volume(myItem->percent);
}

/*-------------------- ACCESS METHODS --------------------*/

void CreateGameMenu(RGB8 *myPalette) {
	if ((!player_commands_allowed()) || (!INTERFACE_VISIBLE) ||
		_G(pal_fade_in_progress) || _G(menuSystemInitialized)) {
		return;
	}

	GameMenu::show(myPalette);
}

} // namespace GUI
} // namespace Riddle
} // namespace M4
