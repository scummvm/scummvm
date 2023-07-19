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
	_background.load("res/gfx/pyrodactyl.png");

	setUI();
	_loadComplete = false;
	_firstRun = true;
}

//------------------------------------------------------------------------
// Purpose: Destructor
//------------------------------------------------------------------------
Splash::~Splash() {
	_background.deleteImage();
}

//------------------------------------------------------------------------
// Purpose: Event/Input Independent InternalEvents
//------------------------------------------------------------------------
void Splash::internalEvents(bool &shouldChangeState, GameStateID &newStateId) {
	if (_firstRun == false) {
		g_engine->_loadingScreen->load();
		g_engine->_imageManager->init();
		g_engine->_textManager->init();
		_loadComplete = true;
	}

	// Have we loaded everything? If yes, time to exit
	if (_loadComplete) {
		shouldChangeState = true;
		newStateId = GAMESTATE_MAIN_MENU;
		return;
	}
}

//------------------------------------------------------------------------
// Purpose: Drawing function
//------------------------------------------------------------------------
void Splash::draw() {
	_background.draw(_x, _y);
	_firstRun = false;
}

//------------------------------------------------------------------------
// Purpose: Reset UI position
//------------------------------------------------------------------------
void Splash::setUI() {
	warning("ScreenSettings cur w : %d h : %d", g_engine->_screenSettings->cur.w, g_engine->_screenSettings->cur.h);
	_x = (g_engine->_screenSettings->cur.w - _background.w()) / 2;
	_y = (g_engine->_screenSettings->cur.h - _background.h()) / 2;
}

}
