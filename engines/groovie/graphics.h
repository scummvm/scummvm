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

#ifndef GROOVIE_GRAPHICS_H
#define GROOVIE_GRAPHICS_H

#include "graphics/surface.h"

namespace Groovie {

class GroovieEngine;

class GraphicsMan {
public:
	GraphicsMan(GroovieEngine *vm);
	~GraphicsMan();

	// Buffers
	void update();
	void change();
	void mergeFgAndBg();
	void switchToFullScreen(bool fullScreen);
	bool isFullScreen() { return (_foreground.h == 480); }
	void updateScreen(Graphics::Surface *source);
	void saveScreen();
	void restoreScreen();
	Graphics::Surface _foreground;	// The main surface that most things are drawn to
	Graphics::Surface _background;	// Used occasionally, mostly (only?) in puzzles
	Graphics::Surface _savedground; // Buffer to save and restore the current screen. Used when opening the gamebook in 11H

	// Palette fading
	bool isFading();
	void fadeIn(byte *pal);
	void fadeOut();

private:
	GroovieEngine *_vm;

	bool _changed;

	// Palette fading
	void applyFading(int step);
	int _fading;
	byte _paletteFull[256 * 3];
	uint32 _fadeStartTime;
};

} // End of Groovie namespace

#endif // GROOVIE_GRAPHICS_H
