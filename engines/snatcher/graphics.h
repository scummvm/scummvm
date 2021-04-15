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

#ifndef SNATCHER_GRAPHICS_H
#define SNATCHER_GRAPHICS_H

#include "snatcher/render.h"
#include "common/platform.h"

class OSystem;

namespace Snatcher {

class Palette;

class GraphicsEngine {
public:
	GraphicsEngine(OSystem *system, Common::Platform platform);
	~GraphicsEngine();

	void enqueuePaletteEvent(const uint8 *data, uint32 curPos);

	void nextFrame();

	uint16 screenWidth() const { return _renderer ? _renderer->screenWidth() : 0; }
	uint16 screenHeight() const { return _renderer ? _renderer->screenHeight() : 0; }

private:
	Renderer *_renderer;
	Palette *_palette;
	OSystem *_system;
};

} // End of namespace Snatcher

#endif // SNATCHER_GRAPHICS_H
