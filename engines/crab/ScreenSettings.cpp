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

#include "crab/crab.h"
#include "crab/ScreenSettings.h"

namespace Crab {

// Our global renderer
//SDL_Renderer *gRenderer = nullptr;
Graphics::Surface *gRendererSurface = nullptr;

// Our global window object
//SDL_Window *gWindow = nullptr;

void ScreenSettings::load(rapidxml::xml_node<char> *node) {
	loadNum(_cur.w, "w", node);
	loadNum(_cur.h, "h", node);
	loadNum(_fps, "fps", node);
	loadNum(_gamma, "gamma", node);
	loadNum(_textSpeed, "text_speed", node);

	loadBool(_vsync, "vsync", node);
	loadBool(_border, "border", node);
	loadBool(_fullscreen, "fullscreen", node);
	loadBool(_saveOnExit, "save_on_exit", node);
	loadBool(_mouseTrap, "mouse_trap", node);
	loadBool(_quality, "quality", node);

	setVideoFlags();
	createBackup();
}

void ScreenSettings::setVideoFlags() {
#if 0
	if (fullscreen) {
		// Window border doesn't matter in fullscreen mode
		videoflags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN;
	} else {
		if (border)
			videoflags = SDL_WINDOW_SHOWN;
		else
			videoflags = SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS;
	}
#endif
}

void ScreenSettings::createBackup() {
	_backup = *this;
}

void ScreenSettings::restoreBackup() {
	_fps = _backup._fps;
	_cur = _backup._cur;
	_fullscreen = _backup._fullscreen;
	_border = _backup._border;
	_vsync = _backup._vsync;
	_gamma = _backup._gamma;
	_videoflags = _backup._videoflags;
	_saveOnExit = _backup._saveOnExit;
	_quality = _backup._quality;
	_mouseTrap = _backup._mouseTrap;
	_textSpeed = _backup._textSpeed;
}

void ScreenSettings::setResolution() {
#if 0
	// Check the current window flags
	Uint32 flags = SDL_GetWindowFlags(gWindow);

	// Are we in fullscreen mode right now?
	if ((flags & SDL_WINDOW_FULLSCREEN) == SDL_WINDOW_FULLSCREEN) {
		// Switch to windowed mode when changing resolution, then switch back once it is changed
		SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_SHOWN);
		SDL_SetWindowSize(gWindow, cur.w, cur.h);
		SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
	} else {
		SDL_SetWindowSize(gWindow, cur.w, cur.h);
		SDL_SetWindowPosition(gWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	change_interface = true;
#endif
}

void ScreenSettings::setFullscreen() {
	setVideoFlags();
#if 0
	SDL_SetWindowFullscreen(gWindow, videoflags);
#endif
}

void ScreenSettings::setWindowBorder() {
#if 0
	if (border)
		SDL_SetWindowBordered(gWindow, SDL_TRUE);
	else
		SDL_SetWindowBordered(gWindow, SDL_FALSE);
#endif
}

void ScreenSettings::setVsync() {
#if 0
	if (vsync)
		SDL_SetHint("SDL_HINT_RENDER_VSYNC", "1");
	else
		SDL_SetHint("SDL_HINT_RENDER_VSYNC", "0");
#endif
}

void ScreenSettings::setMouseTrap() {
#if 0
	if (mouse_trap)
		SDL_SetRelativeMouseMode(SDL_TRUE);
	else
		SDL_SetRelativeMouseMode(SDL_FALSE);
#endif
}

void ScreenSettings::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	root->append_attribute(doc.allocate_attribute("version", g_engine->_stringPool->Get(_version)));

	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "screen");
	child->append_attribute(doc.allocate_attribute("w", g_engine->_stringPool->Get(_cur.w)));
	child->append_attribute(doc.allocate_attribute("h", g_engine->_stringPool->Get(_cur.h)));
	child->append_attribute(doc.allocate_attribute("fps", g_engine->_stringPool->Get(_fps)));

	child->append_attribute(doc.allocate_attribute("gamma", g_engine->_stringPool->FGet(_gamma)));
	child->append_attribute(doc.allocate_attribute("text_speed", g_engine->_stringPool->FGet(_textSpeed)));

	saveBool(_vsync, "vsync", doc, child);
	saveBool(_border, "border", doc, child);
	saveBool(_fullscreen, "fullscreen", doc, child);
	saveBool(_saveOnExit, "save_on_exit", doc, child);
	saveBool(_quality, "quality", doc, child);
	saveBool(_mouseTrap, "mouse_trap", doc, child);

	root->append_node(child);
	createBackup();
}

} // End of namespace Crab
