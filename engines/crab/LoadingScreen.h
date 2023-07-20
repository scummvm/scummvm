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
		pyrodactyl::image::Image _bg;

		Screen(rapidxml::xml_node<char> *node) {
			_bg.load(node, "bg");
		}

		void clear() {
			_bg.deleteImage();
		}

		void draw() {
			_bg.draw((g_engine->_screenSettings->_cur.w - _bg.w()) / 2, (g_engine->_screenSettings->_cur.h - _bg.h()) / 2);
		}
	};

	// The different loading screens
	Common::Array<Screen> _screen;

	// The current loading screen
	unsigned int _cur;

	// The text image (says loading)
	pyrodactyl::image::Image _text;

public:
	LoadingScreen() { _cur = 0; };
	~LoadingScreen(){};

	void change() {
		_cur = g_engine->getRandomNumber(_screen.size() - 1);
	}

	void load();
	void draw();
	void dim();
	void quit();
};

} // End of namespace Crab

#endif // CRAB_LOADINGSCREEN_H
