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

#include "crab/ScreenSettings.h"

namespace Crab {

// Our global renderer
//SDL_Renderer *gRenderer = nullptr;
Graphics::Surface *gRendererSurface = nullptr;

// Our global window object
//SDL_Window *gWindow = nullptr;

void ScreenSettings::Load(rapidxml::xml_node<char> *node) {
	loadNum(cur.w, "w", node);
	loadNum(cur.h, "h", node);
	loadNum(fps, "fps", node);
	loadNum(gamma, "gamma", node);
	loadNum(text_speed, "text_speed", node);

	loadBool(vsync, "vsync", node);
	loadBool(border, "border", node);
	loadBool(fullscreen, "fullscreen", node);
	loadBool(save_on_exit, "save_on_exit", node);
	loadBool(mouse_trap, "mouse_trap", node);
	loadBool(quality, "quality", node);

	SetVideoFlags();
	CreateBackup();
}

void ScreenSettings::SetVideoFlags() {
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

void ScreenSettings::CreateBackup() {
	backup = *this;
}

void ScreenSettings::RestoreBackup() {
	fps = backup.fps;
	cur = backup.cur;
	fullscreen = backup.fullscreen;
	border = backup.border;
	vsync = backup.vsync;
	gamma = backup.gamma;
	videoflags = backup.videoflags;
	save_on_exit = backup.save_on_exit;
	quality = backup.quality;
	mouse_trap = backup.mouse_trap;
	text_speed = backup.text_speed;
}

void ScreenSettings::SetResolution() {
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

void ScreenSettings::SetFullscreen() {
	SetVideoFlags();
#if 0
	SDL_SetWindowFullscreen(gWindow, videoflags);
#endif
}

void ScreenSettings::SetWindowBorder() {
#if 0
	if (border)
		SDL_SetWindowBordered(gWindow, SDL_TRUE);
	else
		SDL_SetWindowBordered(gWindow, SDL_FALSE);
#endif
}

void ScreenSettings::SetVsync() {
#if 0
	if (vsync)
		SDL_SetHint("SDL_HINT_RENDER_VSYNC", "1");
	else
		SDL_SetHint("SDL_HINT_RENDER_VSYNC", "0");
#endif
}

void ScreenSettings::SetMouseTrap() {
#if 0
	if (mouse_trap)
		SDL_SetRelativeMouseMode(SDL_TRUE);
	else
		SDL_SetRelativeMouseMode(SDL_FALSE);
#endif
}

void ScreenSettings::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	root->append_attribute(doc.allocate_attribute("version", gStrPool->Get(version)));

	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "screen");
	child->append_attribute(doc.allocate_attribute("w", gStrPool->Get(cur.w)));
	child->append_attribute(doc.allocate_attribute("h", gStrPool->Get(cur.h)));
	child->append_attribute(doc.allocate_attribute("fps", gStrPool->Get(fps)));

	child->append_attribute(doc.allocate_attribute("gamma", gStrPool->FGet(gamma)));
	child->append_attribute(doc.allocate_attribute("text_speed", gStrPool->FGet(text_speed)));

	saveBool(vsync, "vsync", doc, child);
	saveBool(border, "border", doc, child);
	saveBool(fullscreen, "fullscreen", doc, child);
	saveBool(save_on_exit, "save_on_exit", doc, child);
	saveBool(quality, "quality", doc, child);
	saveBool(mouse_trap, "mouse_trap", doc, child);

	root->append_node(child);
	CreateBackup();
}

} // End of namespace Crab
