/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/actor.h"
#include "asylum/screen.h"

namespace Asylum {

MainActor::MainActor(uint8 *data) {
	for (uint32 i = 0; i < 60; i++){
		_resources[i] = data[i * 4];
	}

	_graphic = 0;
}

MainActor::~MainActor() {
	delete _graphic;
}

void MainActor::setAction(ResourcePack *res, int action) {
	delete _graphic;
	_graphic = new GraphicResource(res, _resources[action]);
	_currentFrame = 0;
}

GraphicFrame *MainActor::getFrame() {
	assert(_graphic);

	GraphicFrame *frame = _graphic->getFrame(_currentFrame);

	if (_currentFrame < _graphic->getFrameCount() - 1) {
		_currentFrame++;
	}else{
		_currentFrame = 0;
	}

	return frame;
}

void MainActor::drawActorAt(Screen *screen, uint16 x, uint16 y) {
	GraphicFrame *frame = getFrame();

	screen->copyRectToScreenWithTransparency(
			((byte *)frame->surface.pixels),
			frame->surface.w,
			x,
			y,
			frame->surface.w,
			frame->surface.h );
}

} // end of namespace Asylum
