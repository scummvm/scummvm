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

namespace Asylum {

Actor::Actor() {
	_graphic         = 0;
	currentAction    = 0;
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

void Actor::drawActorAt(uint32 curX, uint32 curY) {
	GraphicFrame *frame = getFrame();

    WorldStats *ws = _scene->worldstats();

	_scene->vm()->screen()->copyRectToScreenWithTransparency(
			((byte *)frame->surface.pixels),
			frame->surface.w,
            curX - ws->targetX,
			curY - ws->targetY,
			frame->surface.w,
			frame->surface.h );
	x = curX;
	y = curY;
}

void Actor::drawActor() {
	GraphicFrame *frame = getFrame();
    WorldStats *ws = _scene->worldstats();

	_scene->vm()->screen()->copyToBackBufferWithTransparency(
			((byte *)frame->surface.pixels),
			frame->surface.w,
            x - ws->targetX,
			y - frame->surface.h - ws->targetY,
			frame->surface.w,
			frame->surface.h );
}

void Actor::setWalkArea(ActionArea *target) {
	if (_currentWalkArea != target) {
		_scene->actions()->setScriptByIndex(target->actionListIdx1);
		_currentWalkArea = target;
		debugC(kDebugLevelScripts, "%s", target->name);
	}
}

void Actor::walkTo(uint32 curX, uint32 curY) {
	int newAction = currentAction;
    WorldStats *ws = _scene->worldstats();

	// step is the increment by which to move the
	// actor in a given direction
	int step = 2;

	uint32 newX = x;
	uint32 newY = y;
	bool   done = false;

	// Walking left...
	if (curX < x) {
		newAction = kWalkW;
		newX -= step;
		if (ABS((int32)curY - (int32)y) <= 30)
			done = true;
	}

	// Walking right...
	if (curX > x) {
		newAction = kWalkE;
		newX += step;
		if (ABS((int32)curY - (int32)y) <= 30)
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

    _scene->vm()->screen()->copyRectToScreen((byte*)surface.pixels, 5, newX - ws->targetX, newY - ws->targetY, 5, 5);

	surface.free();

	// TODO Basic pathfinding implementation is done. Now it needs to be refined to
	// actuallcurY make it playable. The logic is currently VERY rigid, so you have to have
	// the actor at the PERFECT spot to be able to intersect a walk region and move to
	// the next one.

	int availableAreas[5];
	int areaPtr = 0;
	ActionArea *area;

	// Check what valid walk region(s) is/are currently available
	for (uint32 a = 0; a < ws->numActions; a++) {
		if (ws->actions[a].actionType == 0) {
			area = &ws->actions[a];
			PolyDefinitions poly = _scene->polygons()->entries[area->polyIdx];
            if (poly.contains(x, y)) {
				availableAreas[areaPtr] = a;
				areaPtr++;

				setWalkArea(&ws->actions[a]);

				if (areaPtr > 5)
					error("More than 5 overlapping walk regions found. Increase buffer");

			}
		}
	}

	// Check that we can walk in the current direction within any of the available
	// walkable regions
	for (int i = 0; i < areaPtr; i++) {
		area = &ws->actions[availableAreas[i]];
		PolyDefinitions *region = &_scene->polygons()->entries[area->polyIdx];
		if (region->contains(newX, newY)) {
			x = newX;
			y = newY;
			break;
		}
	}

	setAction(newAction);
	drawActor();
}

void Actor::update_4072A0(int param) {
    int newGrId = 0;
    int newDir = 0;

	switch (param) {

    case 4:
    case 6:
    case 14:   
        if(this->direction > 4)
            newDir = 8 - this->direction;
        else
            newDir = this->direction;
        newGrId = this->grResTable[newDir + 5];
        break;

	case 5:
		newDir = this->direction;
		if (newDir > 4)
			this->direction = 8 - newDir;

		setAction(newDir + 5);
		break;

	}

    this->grResId = newGrId;

    GraphicResource *gra = new GraphicResource(_resPack, this->grResId);
    this->frameCount = gra->getFrameCount();
    this->frameNum   = 0;
    delete gra;

	this->updateType = param;
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
			int actionIdx = _scene->worldstats()->getActionAreaIndexById(targetId);
			if (actionIdx == -1) {
				warning("No ActionArea found for id %d", targetId);
				return;
			}

			uint32 polyIdx = _scene->worldstats()->actions[actionIdx].polyIdx;
			PolyDefinitions *poly = &_scene->polygons()->entries[polyIdx];

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
		int barrierIdx = _scene->worldstats()->getBarrierIndexById(targetId);
		if (barrierIdx == -1) {
			warning("No Barrier found for id %d", targetId);
			return;
		}

		Barrier *barrier = _scene->worldstats()->getBarrierByIndex(barrierIdx);
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

	int newAngle = getAngle(x2 + x1, y2 + y1, newX2, newY2);

	printf("Angle calculated as %d\n", newAngle);

	setDirection(newAngle);
}

int Actor::getAngle(int ax1, int ay1, int ax2, int ay2) {
	int32 v5 = (ax2 << 16) - (ax1 << 16);
	int v6 = 0;
	int v4 = (ay1 << 16) - (ay2 << 16);

	if (v5 < 0) {
		v6 = 2;
		v5 = -v5;
	}

	if (v4 < 0) {
		v6 |= 1;
		v4 = -v4;
	}

	int v7;
	int v8 = -1;

	if (v5) {
		v7 = (v4 << 8) / v5;

		if (v7 < 0x100)
			v8 = angleTable01[v7];
		if (v7 < 0x1000 && v8 < 0)
			v8 = angleTable02[v7 >> 4];
		if (v7 < 0x10000 && v8 < 0)
			v8 = angleTable03[v7 >> 8];
	} else {
		v8 = 90;
	}

	switch (v6) {
	case 1:
		v8 = 360 - v8;
		break;
	case 2:
		v8 = 180 - v8;
		break;
	case 3:
		v8 += 180;
		break;
	}

	if (v8 >= 360)
		v8 -= 360;

	int result;

	if (v8 < 157 || v8 >= 202) {
		if (v8 < 112 || v8 >= 157) {
			if (v8 < 67 || v8 >= 112) {
				if (v8 < 22 || v8 >= 67) {
					if ((v8 < 0 || v8 >= 22) && (v8 < 337 || v8 > 359)) {
						if (v8 < 292 || v8 >= 337) {
							if (v8 < 247 || v8 >= 292) {
								if (v8 < 202 || v8 >= 247) {
									error("getAngle returned a bad angle: %d.", v8);
									result = ax1;
								} else {
									result = 3;
								}
							} else {
								result = 4;
							}
						} else {
							result = 5;
						}
					} else {
						result = 6;
					}
				} else {
					result = 7;
				}
			} else {
				result = 0;
			}
		} else {
			result = 1;
		}
	} else {
		result = 2;
	}

	return result;
}

} // end of namespace Asylum
