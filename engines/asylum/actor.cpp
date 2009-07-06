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

#include "common/endian.h"

namespace Asylum {

MainActor::MainActor(uint8 *data) {
	byte *dataPtr = data;

	for (uint32 i = 0; i < 60; i++){
		_resources[i] = READ_UINT32(dataPtr);
		dataPtr += 4;
	}

	_resPack = 0;
	_graphic = 0;
	_actorX = _actorY = 0;
	_currentAction = 0;
}

MainActor::~MainActor() {
	delete _graphic;
}

void MainActor::setAction(int action) {
	assert(_resPack);

	if (action == _currentAction)
		return;

	_currentAction = action;

	delete _graphic;
	int act = (action < 100) ? action : action - 100;
	_graphic = new GraphicResource(_resPack, _resources[act]);

	// Flip horizontally if necessary
	if (_currentAction > 100) {
		for (uint32 i = 0; i < _graphic->getFrameCount(); i++) {
			GraphicFrame *frame = _graphic->getFrame(i);
			byte *buffer = (byte *)frame->surface.pixels;

			for (int y = 0; y < frame->surface.h; y++) {
				int w = frame->surface.w / 2;
				for (int x = 0; x < w; x++) {
					SWAP(buffer[y * frame->surface.pitch + x], 
						 buffer[y * frame->surface.pitch + frame->surface.w - 1 - x]);
				}
			}
		}
	}

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

	// HACK: frame 1 of the "walk west" animation is misplaced
	if ((_currentAction == kWalkW || _currentAction == kWalkE) && _currentFrame == 1)
		_currentFrame++;

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

	_actorX = x;
	_actorY = y;
}

void MainActor::drawActor(Screen *screen) {
	GraphicFrame *frame = getFrame();

	screen->copyToBackBufferWithTransparency(
			((byte *)frame->surface.pixels),
			frame->surface.w,
			_actorX,
			_actorY - frame->surface.h,
			frame->surface.w,
			frame->surface.h );
}

void MainActor::walkTo(Screen *screen, uint16 x, uint16 y) {
	// TODO: pathfinding! The character can walk literally anywhere
	int newAction = _currentAction;

	// Walking left...
	if (x < _actorX) {
		newAction = kWalkW;
		_actorX--;
		if (ABS(y - _actorY) <= 30) {
			setAction(newAction);
			drawActor(screen);
			return;
		}
	}

	// Walking right...
	if (x > _actorX) {
		newAction = kWalkE;
		_actorX++;
		if (ABS(y - _actorY) <= 30) {
			setAction(newAction);
			drawActor(screen);
			return;
		}
	}

	// Walking up...
	if (y < _actorY) {
		if (newAction != _currentAction && newAction == kWalkW && _actorX - x > 30)
			newAction = kWalkNW;	// up left
		else if (newAction != _currentAction && newAction == kWalkE && x - _actorX > 30)
			newAction = kWalkNE;	// up right
		else
			newAction = kWalkN;
		_actorY--;
	}

	// Walking down...
	if (y > _actorY) {
		if (newAction != _currentAction && newAction == kWalkW && _actorX - x > 30)
			newAction = kWalkSW;	// down left
		else if (newAction != _currentAction && newAction == kWalkE && x - _actorX > 30)
			newAction = kWalkSE;	// down right
		else
			newAction = kWalkS;
		_actorY++;
	}

	setAction(newAction);
	drawActor(screen);
}

} // end of namespace Asylum
