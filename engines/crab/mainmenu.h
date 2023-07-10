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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_MAINMENU_H
#define CRAB_MAINMENU_H

#include "crab/ui/AlphaImage.h"
#include "crab/ui/CreditScreen.h"
#include "crab/ui/FileMenu.h"
#include "crab/ui/ImageData.h"
#include "crab/ui/KeyBindMenu.h"
#include "crab/ui/ModMenu.h"
#include "crab/ui/OptionMenu.h"
#include "crab/ui/SlideShow.h"
#include "crab/ui/StateButton.h"
#include "crab/common_header.h"
#include "crab/input/cursor.h"
#include "crab/gamestate_container.h"
#include "crab/gamestates.h"
#include "crab/ui/menu.h"
#include "crab/ui/slider.h"
#include "crab/ui/textarea.h"

namespace Crab {

// #define UNREST_DEMO

//------------------------------------------------------------------------
// Purpose: Main Menu class
//------------------------------------------------------------------------
class MainMenu : public GameState {
	enum MenuState {
		STATE_NORMAL,
		STATE_OPTIONS,
		STATE_CREDITS,
		STATE_LOAD,
		STATE_DIFF,
		STATE_SAVENAME,
		STATE_MOD,
		STATE_HELP
	} state;

	// This image covers the whole screen and is drawn centered on screen
	pyrodactyl::ui::ImageData bg;

	// The lights on the background image
	Common::Array<pyrodactyl::ui::AlphaImage> lights;

	// The game logo
	pyrodactyl::ui::ImageData logo;

	// Data for the difficulty menu
	struct
	{
		pyrodactyl::ui::ImageData bg;
		pyrodactyl::ui::HoverInfo heading;
		pyrodactyl::ui::ButtonMenu menu;
	} diff;

	// Main menu
	pyrodactyl::ui::Menu<pyrodactyl::ui::StateButton> me_main;

	// The back button is common and is used for all menus here (except credits and main menu)
	pyrodactyl::ui::Button back;

	// Mod menu
	pyrodactyl::ui::ModMenu mod;

	// Game credits
	pyrodactyl::ui::CreditScreen credits;

	// The save game name prompt for iron man
	pyrodactyl::ui::TextArea save;

	// The background image and position of the prompt, along with the warning message displayed
	// and buttons for accept and cancel
	pyrodactyl::ui::ImageData bg_save;
	pyrodactyl::ui::HoverInfo warning;
	pyrodactyl::ui::Button accept, cancel;

	// Music for the main menu
	struct MainMenuMusic {
		pyrodactyl::music::MusicKey normal, credits;

		MainMenuMusic() {
			normal = -1;
			credits = -1;
		}
	} music_key;

#ifdef UNREST_DEMO
	// UI elements related to the demo
	pyrodactyl::ui::Button steam, direct;
#endif

public:
	MainMenu();
	~MainMenu() {}

	void HandleEvents(Common::Event &Event, bool &ShouldChangeState, GameStateID &NewStateID);
#if 0
	void HandleEvents(SDL_Event &Event, bool &ShouldChangeState, GameStateID &NewStateID);
#endif
	void InternalEvents(bool &ShouldChangeState, GameStateID &NewStateID);

	void draw();

	void ChangeState(MenuState ms, const bool &start = false);
	void SetUI();

	// We don't need to save game state here
	void AutoSave() {}
};

} // End of namespace Crab

#endif // CRAB_MAINMENU_H
