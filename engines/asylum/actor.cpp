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
#include "asylum/sceneres.h"

namespace Asylum {

Actor::Actor() {
	_graphic         = 0;
	currentAction   = 0;
	_currentWalkArea = 0;
}

Actor::~Actor() {
	delete _graphic;

	// free _resources?
}

void Actor::setPosition(uint32 targetX, uint32 targetY) {
	boundingRect.left = targetX;
	boundingRect.top  = targetY;

	x = targetX;
	y = targetY;
}

void Actor::visible(bool value) {
	if (value) //	TODO - enums for flags (0x01 is visible)
		flags |= 0x01;
	else
		flags &= 0xFFFFFFFE;
}

void Actor::setDirection(int dir) {
	direction = dir;
	setActionByIndex(dir);
}

void Actor::setRawResources(uint8 *data) {
	byte *dataPtr = data;

	for (uint32 i = 0; i < 60; i++){
		_resources[i] = READ_UINT32(dataPtr);
		dataPtr += 4;
	}
}

void Actor::setAction(int action) {
	assert(_resPack);

	if (action == currentAction)
		return;

	currentAction = action;

	delete _graphic;
	int act = (action < 100) ? action : action - 100;

	_graphic = new GraphicResource(_resPack, _resources[act]);

	// Flip horizontally if necessary
	if (currentAction > 100) {
		for (uint32 i = 0; i < _graphic->getFrameCount(); i++) {
			GraphicFrame *frame = _graphic->getFrame(i);
			byte *buffer = (byte *)frame->surface.pixels;

			for (int tmpY = 0; tmpY < frame->surface.h; tmpY++) {
				int w = frame->surface.w / 2;
				for (int tmpX = 0; tmpX < w; tmpX++) {
					SWAP(buffer[tmpY * frame->surface.pitch + tmpX],
						 buffer[tmpY * frame->surface.pitch + frame->surface.w - 1 - tmpX]);
				}
			}
		}
	}

	frameNum = 0;
}

void Actor::setActionByIndex(int index) {
    setAction(_resources[index] & 0xFFFF);
}

GraphicFrame *Actor::getFrame() {
	assert(_graphic);

	GraphicFrame *frame = _graphic->getFrame(frameNum);

	if (frameNum < _graphic->getFrameCount() - 1) {
		frameNum++;
	}else{
		frameNum = 0;
	}

	// HACK: frame 1 of the "walk west" animation is misplaced
	if ((currentAction == kWalkW || currentAction == kWalkE) && frameNum == 1)
		frameNum++;

	return frame;
}

void Actor::drawActorAt(uint16 curX, uint16 curY) {
	GraphicFrame *frame = getFrame();

	Shared.getScreen()->copyRectToScreenWithTransparency(
			((byte *)frame->surface.pixels),
			frame->surface.w,
			curX,
			curY,
			frame->surface.w,
			frame->surface.h );
	x = curX;
	y = curY;
}

void Actor::drawActor() {
	GraphicFrame *frame = getFrame();

	Shared.getScreen()->copyToBackBufferWithTransparency(
			((byte *)frame->surface.pixels),
			frame->surface.w,
			x,
			y - frame->surface.h,
			frame->surface.w,
			frame->surface.h );
}

void Actor::setWalkArea(ActionArea *target) {
	if (_currentWalkArea != target) {
		ScriptMan.setScriptIndex(target->actionListIdx1);
		_currentWalkArea = target;
		debugC(kDebugLevelScripts, "%s", target->name);
	}
}

void Actor::walkTo(uint16 curX, uint16 curY) {
	int newAction = currentAction;

	// step is the increment by which to move the
	// actor in a given direction
	int step = 2;

	uint16 newX = x;
	uint16 newY = y;
	bool   done = false;

	// Walking left...
	if (curX < x) {
		newAction = kWalkW;
		newX -= step;
		if (ABS(curY - y) <= 30)
			done = true;
	}

	// Walking right...
	if (curX > x) {
		newAction = kWalkE;
		newX += step;
		if (ABS(curY - y) <= 30)
			done = true;
	}

	// Walking up...
	if (curY < y && !done) {
		if (newAction != currentAction && newAction == kWalkW && x - curX > 30)
			newAction = kWalkNW;	// up left
		else if (newAction != currentAction && newAction == kWalkE && curX - x > 30)
			newAction = kWalkNE;	// up right
		else
			newAction = kWalkN;

		newY -= step;
	}

	// Walking down...
	if (curY > y && !done) {
		if (newAction != currentAction && newAction == kWalkW && x - curX > 30)
			newAction = kWalkSW;	// down left
		else if (newAction != currentAction && newAction == kWalkE && curX - x > 30)
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
	// actuallcurY make it playable. The logic is currently VERY rigid, so you have to have
	// the actor at the PERFECT spot to be able to intersect a walk region and move to
	// the next one.

	int availableAreas[5];
	int areaPtr = 0;
	ActionArea *area;

	// Check what valid walk region(s) is/are currently available
	for (uint32 a = 0; a < Shared.getScene()->getResources()->getWorldStats()->numActions; a++) {
		if (Shared.getScene()->getResources()->getWorldStats()->actions[a].actionType == 0) {
			area = &Shared.getScene()->getResources()->getWorldStats()->actions[a];
			PolyDefinitions poly = Shared.getScene()->getResources()->getGamePolygons()->polygons[area->polyIdx];
			if (Shared.pointInPoly(&poly, x, y)) {
				availableAreas[areaPtr] = a;
				areaPtr++;

				setWalkArea(&Shared.getScene()->getResources()->getWorldStats()->actions[a]);

				if (areaPtr > 5)
					error("More than 5 overlapping walk regions found. Increase buffer");

			}
		}
	}

	// Check that we can walk in the current direction within any of the available
	// walkable regions
	for (int i = 0; i < areaPtr; i++) {
		area = &Shared.getScene()->getResources()->getWorldStats()->actions[availableAreas[i]];
		PolyDefinitions *region = &Shared.getScene()->getResources()->getGamePolygons()->polygons[area->polyIdx];
		if (Shared.pointInPoly(region, newX, newY)) {
			x = newX;
			y = newY;
			break;
		}
	}

	setAction(newAction);
	drawActor();
}

void Actor::update_4072A0(int param) {
	switch (param) {

	case 5:
		int dir = direction;
		if (dir > 4)
			direction = 8 - dir;

		setAction(dir + 5);
		break;

	}

	field_40 = param;
}

void Actor::setPosition_40A260(uint32 newX, uint32 newY, int newDirection, int frame) {
	x1 = newX - x2;
	y1 = newY - y2;

	if (direction != 8) {
		// TODO implement the propert character_setDirection() functionality
		setAction(newDirection);
	}
	if (frame > 0)
		frameNum = frame;
}

void Actor::faceTarget(int targetId, int targetType) {
	int newX2, newY2;

	printf("faceTarget: id %d type %d\n", targetId, targetType);

	if (targetType) {
		if (targetType == 1) {
			int actionIdx = Shared.getScene()->getResources()->getActionAreaIndexById(targetId);
			if (actionIdx == -1) {
				warning("No ActionArea found for id %d", targetId);
				return;
			}

			uint32 polyIdx = Shared.getScene()->getResources()->getWorldStats()->actions[actionIdx].polyIdx;
			PolyDefinitions *poly = &Shared.getScene()->getResources()->getGamePolygons()->polygons[polyIdx];

			newX2 = poly->boundingRect.left + (poly->boundingRect.right - poly->boundingRect.left) / 2;
			newY2 = poly->boundingRect.top + (poly->boundingRect.bottom - poly->boundingRect.top) / 2;
		} else {
			if (targetType == 2) {
				newX2 = x2 + x1;
				newY2 = y2 + y1;
			} else {
				newX2 = newY2 = targetId;
			}
		}
	} else {
		int barrierIdx = Shared.getScene()->getResources()->getBarrierIndexById(targetId);
		if (barrierIdx == -1) {
			warning("No Barrier found for id %d", targetId);
			return;
		}

		BarrierItem *barrier = Shared.getScene()->getResources()->getBarrierByIndex(barrierIdx);
		GraphicResource *gra = new GraphicResource(_resPack, barrier->resId);

		// FIXME
		// The original actually grabs the current frame of the target
		// barrier. I'm wondering if that's unnecessary since I'm assuming
		// the dimensions of each frame should be the same.
		// Investigate, though I don't think it'll be necessary since
		// what we're trying to accomplish is a character rotation calclulation,
		// and a size difference of a few pixels "shouldn't" affect this
		// too much
		GraphicFrame *fra = gra->getFrame(0);
		delete gra;

		newX2 = (fra->surface.w >> 1) + barrier->x; // TODO (x/y + 1704 * barrier) (not sure what this is pointing to)
		newY2 = (fra->surface.h >> 1) + barrier->y; // Check .text:004088A2 for more details
	}

	int newAngle = Shared.getAngle(x2 + x1, y2 + y1, newX2, newY2);

	printf("Angle calculated as %d\n", newAngle);

	setDirection(newAngle);
}

} // end of namespace Asylum
