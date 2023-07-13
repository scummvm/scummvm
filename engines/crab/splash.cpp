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


//=============================================================================
// Author:   Arvind
// Purpose:  Splash functions
//=============================================================================
#include "crab/splash.h"

namespace Crab {

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
Splash::Splash() {
	// Load the background
	background.load("res/gfx/pyrodactyl.png");

	setUI();
	load_complete = false;
	first_run = true;
}

//------------------------------------------------------------------------
// Purpose: Destructor
//------------------------------------------------------------------------
Splash::~Splash() {
	background.deleteImage();
}

//------------------------------------------------------------------------
// Purpose: Event/Input Independent InternalEvents
//------------------------------------------------------------------------
void Splash::internalEvents(bool &ShouldChangeState, GameStateID &NewStateID) {
	if (first_run == false) {
		g_engine->_loadingScreen->load();
		g_engine->_imageManager->init();
		g_engine->_textManager->Init();
		load_complete = true;
	}

	// Have we loaded everything? If yes, time to exit
	if (load_complete) {
		ShouldChangeState = true;
		NewStateID = GAMESTATE_MAIN_MENU;
		return;
	}
}

//------------------------------------------------------------------------
// Purpose: Drawing function
//------------------------------------------------------------------------
void Splash::draw() {
	background.draw(x, y);
	first_run = false;
}

//------------------------------------------------------------------------
// Purpose: Reset UI position
//------------------------------------------------------------------------
void Splash::setUI() {
	warning("ScreenSettings cur w : %d h : %d", g_engine->_screenSettings->cur.w, g_engine->_screenSettings->cur.h);
	x = (g_engine->_screenSettings->cur.w - background.w()) / 2;
	y = (g_engine->_screenSettings->cur.h - background.h()) / 2;
}

}
