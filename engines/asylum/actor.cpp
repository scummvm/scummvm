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

#include "common/endian.h"

#include "asylum/actor.h"
#include "asylum/screen.h"
#include "asylum/shared.h"

namespace Asylum {

MainActor::MainActor(uint8 *data) {
	byte *dataPtr = data;

	for (uint32 i = 0; i < 60; i++){
		_resources[i] = READ_UINT32(dataPtr);
		dataPtr += 4;
	}

	_resPack         = 0;
	_graphic         = 0;
	_actorX          = 0;
	_actorY          = 0;
	_currentAction   = 0;
	_currentWalkArea = 0;
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

void MainActor::setActionByIndex(int index) {
    setAction(_resources[index] & 0xFFFF);
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

void MainActor::drawActorAt(uint16 x, uint16 y) {
	GraphicFrame *frame = getFrame();

	Shared.getScreen()->copyRectToScreenWithTransparency(
			((byte *)frame->surface.pixels),
			frame->surface.w,
			x,
			y,
			frame->surface.w,
			frame->surface.h );

	_actorX = x;
	_actorY = y;
}

void MainActor::drawActor() {
	GraphicFrame *frame = getFrame();

	Shared.getScreen()->copyToBackBufferWithTransparency(
			((byte *)frame->surface.pixels),
			frame->surface.w,
			_actorX,
			_actorY - frame->surface.h,
			frame->surface.w,
			frame->surface.h );
}

void MainActor::setWalkArea(ActionItem *target) {
	if (_currentWalkArea != target) {
		ScriptMan.setScriptIndex(target->actionListIdx1);
		_currentWalkArea = target;
		debugC(kDebugLevelScripts, "%s", target->name);
	}
}

void MainActor::walkTo(uint16 x, uint16 y) {
	int newAction = _currentAction;

	// step is the increment by which to move the
	// actor in a given direction
	int step = 2;

	uint16 newX = _actorX;
	uint16 newY = _actorY;
	bool   done = false;

	// Walking left...
	if (x < _actorX) {
		newAction = kWalkW;
		newX -= step;
		if (ABS(y - _actorY) <= 30)
			done = true;
	}

	// Walking right...
	if (x > _actorX) {
		newAction = kWalkE;
		newX += step;
		if (ABS(y - _actorY) <= 30)
			done = true;
	}

	// Walking up...
	if (y < _actorY && !done) {
		if (newAction != _currentAction && newAction == kWalkW && _actorX - x > 30)
			newAction = kWalkNW;	// up left
		else if (newAction != _currentAction && newAction == kWalkE && x - _actorX > 30)
			newAction = kWalkNE;	// up right
		else
			newAction = kWalkN;

		newY -= step;
	}

	// Walking down...
	if (y > _actorY && !done) {
		if (newAction != _currentAction && newAction == kWalkW && _actorX - x > 30)
			newAction = kWalkSW;	// down left
		else if (newAction != _currentAction && newAction == kWalkE && x - _actorX > 30)
			newAction = kWalkSE;	// down right
		else
			newAction = kWalkS;

		newY += step;
	}

	// DEBUGGING
	// Show registration point from which we're calculating the
	// actor's barrier hit-test
	Graphics::Surface surface;
	surface.create(5, 5, 1);
	Common::Rect rect;

	rect.top    = newY;
	rect.left   = newX;
	rect.right  = newX;
	rect.bottom = newY + 4;
	surface.frameRect(rect, 0x33);

	Shared.getScreen()->copyRectToScreen((byte*)surface.pixels, 5, newX, newY, 5, 5);

	surface.free();

	// TODO Basic pathfinding implementation is done. Now it needs to be refined to
	// actually make it playable. The logic is currently VERY rigid, so you have to have
	// the actor at the PERFECT spot to be able to intersect a walk region and move to
	// the next one.

	int availableAreas[5];
	int areaPtr = 0;
	ActionItem *area;

	// Check what valid walk region(s) is/are currently available
	for (uint32 a = 0; a < Shared.getScene()->getResources()->getWorldStats()->numActions; a++) {
		if (Shared.getScene()->getResources()->getWorldStats()->actions[a].actionType == 0) {
			area = &Shared.getScene()->getResources()->getWorldStats()->actions[a];
			PolyDefinitions poly = Shared.getScene()->getResources()->getGamePolygons()->polygons[area->polyIdx];
			if (Shared.pointInPoly(&poly, _actorX, _actorY)) {
				availableAreas[areaPtr] = a;
				areaPtr++;

				if (areaPtr > 5)
					error("More than 5 overlapping walk regions found. Increase buffer");
			}
		}
	}

	// Set the current walk region to the first available action area
	// in the collection
	setWalkArea(&Shared.getScene()->getResources()->getWorldStats()->actions[availableAreas[0]]);

	// Check that we can walk in the current direction within any of the available
	// walkable regions
	for (int i = 0; i < areaPtr; i++) {
		area = &Shared.getScene()->getResources()->getWorldStats()->actions[availableAreas[i]];
		PolyDefinitions *region = &Shared.getScene()->getResources()->getGamePolygons()->polygons[area->polyIdx];
		if (Shared.pointInPoly(region, newX, newY)) {
			_actorX = newX;
			_actorY = newY;
			break;
		}
	}

	setAction(newAction);
	drawActor();
}

} // end of namespace Asylum
