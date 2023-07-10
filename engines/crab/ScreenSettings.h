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

#include "crab/GameParam.h"
#include "crab/common_header.h"
#include "crab/loaders.h"


namespace Crab {

#if 0
// Our rendering context
extern SDL_Renderer *gRenderer;

// Our window
extern SDL_Window *gWindow;
#endif

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
	int fps;

	// The current screen dimensions
	Dimension cur;

	// The minimum dimension where we draw the image without scaling
	// Any lower than this and we draw at the minimum resolution, then scale it down
	// Dimension min;

	// Is the window full-screen?
	bool fullscreen;

	// Does the window have a border?
	bool border;

	// Is vertical sync enabled?
	bool vsync;

	// The brightness of a window
	float gamma;

	// The video flags
	uint32 videoflags;

	// should we save on exit?
	bool save_on_exit;

	// This flag is true if we want to load high quality images, false otherwise
	bool quality;

	// Is the mouse trapped within the window
	bool mouse_trap;

	// The text speed (used only for popup text)
	float text_speed;

	ScreenAttributes() {
		fps = 60;
		fullscreen = false;
		border = true;
		vsync = true;
		save_on_exit = true;
		quality = true;
		mouse_trap = false;
		gamma = 1.0f;
		text_speed = 1.0f;
		videoflags = 0; //SDL_WINDOW_SHOWN;
	}
};

// Screen attributes
class ScreenSettings : public ScreenAttributes {
	// A backup of the attributes
	ScreenAttributes backup;

public:
	// The desktop dimensions
	Dimension desktop;

	// True if we are in game, false otherwise
	bool in_game;

	// Set to true when we have to call SetUI() for rearranging UI after a resolution change
	bool change_interface;

	// The version of the settings
	unsigned int version;

	ScreenSettings() {
		in_game = false;
		version = 0;
		change_interface = false;
	}
	~ScreenSettings() {}

	bool ValidDimension(Dimension d) { return d.w <= desktop.w && d.h <= desktop.h; }

	void SetVideoFlags();
	void CreateBackup();
	void RestoreBackup();

	void SetResolution();
	void SetFullscreen();
	void SetWindowBorder();
	void SetVsync();
	void SetGamma() { }//SDL_SetWindowBrightness(gWindow, gamma); }
	void SetMouseTrap();

	void load(rapidxml::xml_node<char> *node);
	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
};

}

#endif // CRAB_SCREENSETTINGS_H
