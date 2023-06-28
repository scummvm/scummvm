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

#ifndef CRAB_APP_H
#define CRAB_APP_H

#include "crab/crab.h"
#include "crab/image/ImageManager.h"
#include "crab/music/MusicManager.h"
#include "crab/common_header.h"
#include "crab/game.h"
#include "crab/gamestates.h"
#include "crab/gamestate_container.h"
#include "crab/ScreenSettings.h"
#include "crab/mainmenu.h"
#include "crab/splash.h"
#include "crab/timer.h"

namespace Crab {

class App {
private:
	Game *_game;

	void LoadSettings(const Common::String &filename);
public:
	App(void) {
#if 0
		// OS X .app files need to set the working directory
		char *working_directory = SDL_GetBasePath();
		chdir(working_directory);
		SDL_free(working_directory);
#endif
	}
	~App(void);

	bool Init();
	void Run();
	Game *GetGame() const {
		return _game;
	}
};

} // End of namespace Crab

#endif // CRAB_APP_H
