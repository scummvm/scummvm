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
// Purpose:  A basic screen that flashes an image for a time interval
//=============================================================================
#ifndef CRAB_SPLASH_H
#define CRAB_SPLASH_H

#include "crab/crab.h"
#include "crab/image/ImageManager.h"
//#include "crab/LoadingScreen.h"
//#include "crab/music/MusicManager.h"
//#include "crab/image/Image.h"
#include "crab/ScreenSettings.h"
//#include "crab/text/TextManager.h"
#include "crab/common_header.h"
#include "crab/gamestate_container.h"
#include "crab/gamestates.h"
#include "crab/timer.h"

namespace Crab {
//------------------------------------------------------------------------
// Purpose: Splash screen class
//------------------------------------------------------------------------
class Splash : public GameState {
	pyrodactyl::image::Image background;

	int x, y;
	bool first_run, load_complete;

public:
	Splash();
	~Splash();
	void HandleEvents(Common::Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) {}
	void InternalEvents(bool &ShouldChangeState, GameStateID &NewStateID);
	void Draw();
	void SetUI();

	// We don't need to save game state here
	void AutoSave() {}
};

}

#endif // CRAB_SPLASH_H
