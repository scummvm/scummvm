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

#ifndef CRAB_SCREENSETTINGS_H
#define CRAB_SCREENSETTINGS_H

#include "crab/rapidxml/rapidxml.hpp"

namespace Crab {

struct Dimension {
	int w, h;
	Dimension() {
		w = 1280;
		h = 720;
	}

	bool operator<(const Dimension &d) { return (w < d.w && h < d.h); }
};

class ScreenAttributes {
public:
	// Frames per second
	int _fps;

	// The current screen dimensions
	Dimension _cur;

	// The minimum dimension where we draw the image without scaling
	// Any lower than this and we draw at the minimum resolution, then scale it down
	// Dimension min;

	// Is the window full-screen?
	bool _fullscreen;

	// Does the window have a border?
	bool _border;

	// Is vertical sync enabled?
	bool _vsync;

	// The brightness of a window
	float _gamma;

	// The video flags
	uint32 _videoflags;

	// should we save on exit?
	bool _saveOnExit;

	// This flag is true if we want to load high quality images, false otherwise
	bool _quality;

	// Is the mouse trapped within the window
	bool _mouseTrap;

	// The text speed (used only for popup text)
	float _textSpeed;

	ScreenAttributes() {
		_fps = 60;
		_fullscreen = false;
		_border = true;
		_vsync = true;
		_saveOnExit = true;
		_quality = true;
		_mouseTrap = false;
		_gamma = 1.0f;
		_textSpeed = 1.0f;
		_videoflags = 0; //SDL_WINDOW_SHOWN;
	}
};

// Screen attributes
class ScreenSettings : public ScreenAttributes {

public:
	// The desktop dimensions
	Dimension _desktop;

	// True if we are in game, false otherwise
	bool _inGame;

	// Set to true when we have to call setUI() for rearranging UI after a resolution change
	bool _changeInterface;

	// The version of the settings
	uint _version;

	ScreenSettings() {
		_inGame = false;
		_version = 0;
		_changeInterface = false;
	}
	~ScreenSettings() {}

	bool validDimension(Dimension d) {
		return d.w <= _desktop.w && d.h <= _desktop.h;
	}

	void load(rapidxml::xml_node<char> *node);

	void toggleFullScreen();
	void toggleVsync();

	void saveState();
};

}

#endif // CRAB_SCREENSETTINGS_H
