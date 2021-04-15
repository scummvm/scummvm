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

#include "snatcher/palette.h"
#include "snatcher/render.h"
#include "snatcher/graphics.h"

#include "common/system.h"

namespace Snatcher {

GraphicsEngine::GraphicsEngine(OSystem *system, Common::Platform platform) : _system(system), _renderer(0) {
	_renderer = Renderer::create(platform);
	_palette = Palette::create(_system->getPaletteManager(), platform);
	assert(_renderer);
}

GraphicsEngine::~GraphicsEngine() {
	delete _renderer;
	delete _palette;
}

void GraphicsEngine::enqueuePaletteEvent(const uint8 *data, uint32 curPos) {
	_palette->enqueueEvent(data, curPos);
}

void GraphicsEngine::nextFrame() {
	//if (!_skipManyEvents) {
		//_blockPalEvent = false;
		_palette->processEventQueue();
	//}
	_system->updateScreen();
}

} // End of namespace Snatcher
