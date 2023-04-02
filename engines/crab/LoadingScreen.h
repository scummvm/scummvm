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

#ifndef CRAB_LOADINGSCREEN_H
#define CRAB_LOADINGSCREEN_H

#include "crab/image/Image.h"
#include "crab/common_header.h"
#include "crab/timer.h"
#include "crab/vectors.h"

namespace Crab {

class LoadingScreen {
	struct Screen {
		// The background image
		pyrodactyl::image::Image bg;

		Screen(rapidxml::xml_node<char> *node) { bg.Load(node, "bg"); }
		void Delete() { bg.Delete(); }
		void Draw() { bg.Draw((gScreenSettings.cur.w - bg.W()) / 2, (gScreenSettings.cur.h - bg.H()) / 2); }
	};

	// The different loading screens
	std::vector<Screen> screen;

	// The current loading screen
	unsigned int cur;

	// The text image (says loading)
	pyrodactyl::image::Image text;

public:
	LoadingScreen() { cur = 0; };
	~LoadingScreen(){};

	void Change() {
	//	cur = gRandom.Num() % screen.size();
	}

	void Load();
	void Draw();
	void Dim();
	void Quit();
};

extern LoadingScreen gLoadScreen;

} // End of namespace Crab

#endif // CRAB_LOADINGSCREEN_H
