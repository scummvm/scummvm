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

#include "asylum/resources/actor.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/screen.h"
#include "asylum/system/config.h"

#include "asylum/staticres.h"

#include "common/endian.h"

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

void Actor::visible(bool value) {
	if (value) //	TODO - enums for flags (0x01 is visible)
		flags |= 0x01;
	else
		flags &= 0xFFFFFFFE;

	stopSound();
}

/*
void Actor::setDirection(int32 dir) {
	direction = dir;
	setActionByIndex(dir);
}
*/

void Actor::setRawResources(uint8 *data) {
	byte *dataPtr = data;

	for (int32 i = 0; i < 60; i++) {
		_resources[i] = (int32)READ_LE_UINT32(dataPtr);
		dataPtr += 4;
	}
}

void Actor::setAction(int32 action) {
	assert(_resPack);

	if (action == currentAction)
		return;

	currentAction = action;

	delete _graphic;
	int32 act = (action < 100) ? action : action - 100;

	_graphic = new GraphicResource(_resPack, _resources[act]);

	// Flip horizontally if necessary
	if (currentAction > 100) {
		for (uint32 i = 0; i < _graphic->getFrameCount(); i++) {
			GraphicFrame *frame = _graphic->getFrame(i);
			byte *buffer = (byte *)frame->surface.pixels;

			for (int32 tmpY = 0; tmpY < frame->surface.h; tmpY++) {
				int32 w = frame->surface.w / 2;
				for (int32 tmpX = 0; tmpX < w; tmpX++) {
					SWAP(buffer[tmpY * frame->surface.pitch + tmpX],
					     buffer[tmpY * frame->surface.pitch + frame->surface.w - 1 - tmpX]);
				}
			}
		}
	}

	frameNum = 0;
}

void Actor::setActionByIndex(int32 index) {
	setAction(_resources[index] & 0xFFFF);
}

GraphicFrame *Actor::getFrame() {
	assert(_graphic);

	GraphicFrame *frame = _graphic->getFrame(frameNum);

	if (frameNum < _graphic->getFrameCount() - 1) {
		frameNum++;
	} else {
		frameNum = 0;
	}

	// HACK: frame 1 of the "walk west" animation is misplaced
	if ((currentAction == kWalkW || currentAction == kWalkE) && frameNum == 1)
		frameNum++;

	return frame;
}

void Actor::drawActorAt(int32 curX, int32 curY) {
	GraphicFrame *frame = getFrame();

	WorldStats *ws = _scene->worldstats();

	_scene->vm()->screen()->copyRectToScreenWithTransparency(
	    ((byte *)frame->surface.pixels),
	    frame->surface.w,
	    curX - ws->targetX,
	    curY - ws->targetY,
	    frame->surface.w,
	    frame->surface.h);
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
	    frame->surface.h);
}

void Actor::setWalkArea(ActionArea *target) {
	if (_currentWalkArea != target) {
		// FIXME
		//_scene->actions()->setScriptByIndex(target->actionListIdx1);
		_currentWalkArea = target;
		debugC(kDebugLevelScripts, "%s", target->name);
	}
}

void Actor::walkTo(int32 curX, int32 curY) {
	int32 newAction = currentAction;
	WorldStats *ws = _scene->worldstats();

	// step is the increment by which to move the
	// actor in a given direction
	int32 step = 2;

	int32 newX = x;
	int32 newY = y;
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
	// Show registration point32 from which we're calculating the
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

	int32 availableAreas[5];
	int32 areaPtr = 0;
	ActionArea *area;

	// Check what valid walk region(s) is/are currently available
	for (int32 a = 0; a < ws->numActions; a++) {
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
	for (int32 i = 0; i < areaPtr; i++) {
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

void Actor::stopSound() {
	if (soundResId && _scene->vm()->sound()->isPlaying(soundResId))
		_scene->vm()->sound()->stopSound(soundResId);
}

void Actor::setPosition(int32 newX, int32 newY, int32 newDirection, int32 frame) {
	x1 = newX - x2;
	y1 = newY - y2;

	if (direction != 8) {
		// TODO implement the propert character_setDirection() functionality
		setAction(newDirection);
	}
	if (frame > 0)
		frameNum = frame;
}

void Actor::faceTarget(int32 targetId, int32 targetType) {
	int32 newX2, newY2;

	printf("faceTarget: id %d type %d\n", targetId, targetType);

	if (targetType) {
		if (targetType == 1) {
			int32 actionIdx = _scene->worldstats()->getActionAreaIndexById(targetId);
			if (actionIdx == -1) {
				warning("No ActionArea found for id %d", targetId);
				return;
			}

			int32 polyIdx = _scene->worldstats()->actions[actionIdx].polyIdx;
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
		int32 barrierIdx = _scene->worldstats()->getBarrierIndexById(targetId);
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

	int32 newAngle = getAngle(x2 + x1, y2 + y1, newX2, newY2);

	printf("Angle calculated as %d\n", newAngle);

	// TODO set player direction
	//setDirection(newAngle);
}

int32 Actor::getAngle(int32 ax1, int32 ay1, int32 ax2, int32 ay2) {
	int32 v5 = (ax2 << 16) - (ax1 << 16);
	int32 v6 = 0;
	int32 v4 = (ay1 << 16) - (ay2 << 16);

	if (v5 < 0) {
		v6 = 2;
		v5 = -v5;
	}

	if (v4 < 0) {
		v6 |= 1;
		v4 = -v4;
	}

	int32 v7;
	int32 v8 = -1;

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

	int32 result;

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

void Actor::updateDirection() {
	if(field_970) {
		// TODO
		// This update is only ever done if action script 0x5D is called, and
		// the resulting switch sets field_970. Investigate 401A30 for further
		// details
		error("[Actor::updateDirection] logic not implemented");
	}
}

void Actor::updateStatus(ActorStatus actorStatus) {
	GraphicResource *gra;

	switch (actorStatus) {
	case kActorStatus16:
		frameNum = 0;
		if (direction > 4)
			direction = 8 - direction;
		grResId = grResTable[15];
		gra = new GraphicResource(_resPack, grResId);
		frameCount = gra->getFrameCount();
		delete gra;
		break;
	case kActorStatus18:
		if (_scene->worldstats()->numChapter > 2) {
			frameNum = 0;
			if (_index > 12)
				grResId = grResTable[30];
			if (_scene->getActorIndex() == _index) {
				gra = new GraphicResource(_resPack, grResId);
				frameNum = gra->getFrameCount() - 1;
				delete gra;
			}

			if (_index == 11) {
				// TODO check a global variable (that likely
				// relates to direction) to see if it's > 4,
				// and if so, subtract it from 8. Then use this
				// to set actor[11].grResId
			}

			// FIXME I know this seems wasteful, but it's how the
			// original worked. I guess this is to set the framecount
			// regardless of the actorIndex value, though it assumes
			// the actor's grResId has been set.
			gra = new GraphicResource(_resPack, grResId);
			frameCount = gra->getFrameCount();
			delete gra;
		}
		break;
	case kActorStatus15:
		// TODO Refactor, because this is identical to case 16,
		// other than a different grResTable index
		frameNum = 0;
		if (direction > 4)
			direction = 8 - direction;
		grResId = grResTable[10];
		gra = new GraphicResource(_resPack, grResId);
		frameCount = gra->getFrameCount();
		delete gra;
		break;
	case kActorStatus9:
		// TODO Check if there is an encounter currently
		// active (via the global at .data:00543504)
		// FIXME skipping for now
		if (0) {
			if (_scene->vm()->getRandomBit() == 1 && defaultDirectionLoaded(15)) {
				frameNum = 0;
				if (direction > 4)
					direction = 8 - direction;
				grResId = grResTable[15];
				gra = new GraphicResource(_resPack, grResId);
				frameCount = gra->getFrameCount();
				delete gra;
			} else {
				frameNum = 0;
				if (direction > 4)
					direction = 8 - direction;
				grResId = grResTable[10];
				gra = new GraphicResource(_resPack, grResId);
				frameCount = gra->getFrameCount();
				delete gra;
			}
		}
		break;
	case kActorStatusEnabled:
	case kActorStatus6:
	case kActorStatus14:
		frameNum = 0;
		if (direction > 4)
			direction = 8 - direction;
		grResId = grResTable[15];
		gra = new GraphicResource(_resPack, grResId);
		frameCount = gra->getFrameCount();
		delete gra;
		break;
	case kActorStatus1:
	case kActorStatus12:
		// TODO check if sceneNumber == 2 && actorIndex == _playerActorInde
		// && field_40 equals/doesn't equal a bunch of values,
		// then set direction like other cases
		break;
	case kActorStatus2:
	case kActorStatus13:
		frameNum = 0;
		if (direction > 4)
			direction = 8 - direction;
		grResId = grResTable[direction];
		gra = new GraphicResource(_resPack, grResId);
		frameCount = gra->getFrameCount();
		delete gra;
		break;
	case kActorStatusDisabled:
		frameNum = 0;
		if (direction > 4)
			direction = 8 - direction;
		grResId = grResTable[direction];
		gra = new GraphicResource(_resPack, grResId);
		frameCount = gra->getFrameCount();
		delete gra;
		// TODO set word_446EE4 to -1. This global seems to
		// be used with screen blitting
		break;
	case kActorStatus3:
	case kActorStatus19:
		// TODO check if the actor's name is equal to
		// "Big Crow"???
		break;
	case kActorStatus7:
		if (_scene->worldstats()->numChapter == 2 && _index == 10 && _scene->vm()->isGameFlagSet(279)) {
			Actor *act0 = _scene->getActor(0);
			act0->x1 = x2 + x1 - act0->x2;
			act0->y1 = y2 + y1 - act0->y2;
			act0->direction = 4;
			_scene->setActorIndex(0);
			// TODO disableCharacterVisible(actorIndex)
			// TODO enableActorVisible(0)
			_scene->vm()->clearGameFlag(279);
			// TODO some cursor update
		}
		break;
	case kActorStatus8:
	case kActorStatus10:
	case kActorStatus17:
		frameNum = 0;
		if (direction > 4)
			direction = 8 - direction;
		grResId = grResTable[20];
		gra = new GraphicResource(_resPack, grResId);
		frameCount = gra->getFrameCount();
		delete gra;
		break;
	}

	status = actorStatus;
}

void Actor::setDirection(int actorDirection) {
	direction = (actorDirection > 4) ? 8 - actorDirection : actorDirection;
	int32 grResId;

	if (field_944 != 5) {
		switch (status) {
		case 0x04:
		case 0x05:
		case 0x0E: {
			grResId = grResTable[direction + 5];
			// FIXME this seems kind of wasteful just to grab a frame count
			GraphicResource *gra = new GraphicResource(_resPack, grResId);
			grResId = grResId;
			frameCount = gra->getFrameCount();
			delete gra;
				   }
				   break;
		case 0x12:
			if (_scene->worldstats()->numChapter == 2) {
				if (_scene->getActorIndex() == 11) {
					// NOTE this is supposed to explicitely point to the actor 11 reference,
					// (_ws->actors[11])
					// but I'm assuming if control drops through to here, getActor() would
					// pull the right object because the _playerActorIndex should == 11
					if (direction > 4)
						grResId = grResTable[8 - direction];
					else
						grResId = grResTable[direction];
				}
			}
			break;
		case 0x01:
		case 0x02:
		case 0x0C:
			grResId = grResTable[direction];
			break;
		case 0x08:
			grResId = grResTable[direction + 20];
			break;
		default:
			warning ("[setActorDirection] default case hit with status of %d", status);
		}
	}
}

void Actor::update() {
	if (visible()) {
		// printf("Actor updateType = 0x%02X\n", actor->updateType);

		switch (status) {

		case 0x10:
			if (_scene->worldstats()->numChapter == 2) {
				// TODO: updateCharacterSub14()
			} else if (_scene->worldstats()->numChapter == 1) {
				if (_scene->getActorIndex() == _index) {
					// TODO: updateActorSub21();
				}
			}
			break;
		case 0x11:
			if (_scene->worldstats()->numChapter == 2) {
				// TODO: put code here
			} else if (_scene->worldstats()->numChapter == 11) {
				if (_scene->getActorIndex() == _index) {
					// TODO: put code here
				}
			}
			break;
		case 0xF:
			if (_scene->worldstats()->numChapter == 2) {
				// TODO: put code here
			} else if (_scene->worldstats()->numChapter == 11) {
				// TODO: put code here
			}
			break;
		case 0x12:
			if (_scene->worldstats()->numChapter == 2) {
				// TODO: put code here
			}
			break;
		case 0x5: {
			frameNum = (frameNum + 1) % frameCount;

			if (_scene->vm()->getTick() - tickValue1 > 300) {
				if (_scene->vm()->getRandom(100) < 50) {
					// TODO: check sound playing
				}
				tickValue1 = _scene->vm()->getTick();
			}
				  }
				  break;
		case 0xC:
			if (_scene->worldstats()->numChapter == 2) {
				// TODO: put code here
			} else if (_scene->worldstats()->numChapter == 11) {
				// TODO: put code here
			}
		case 0x1:
			// TODO: do actor direction
			break;
		case 0x2:
		case 0xD:
			// TODO: do actor direction
			break;
		case 0x3:
		case 0x13:
			// TODO: updateCharacterSub05();
			break;
		case 0x7:
			// TODO: something
			break;
		case 0x4:
			if (field_944 != 5) {
				//updateActorSub01(actor);
			}
			break;
		case 0xE:
			// TODO: updateCharacterSub02(1, actorIdx);
			break;
		case 0x15:
			// TODO: updateCharacterSub06(1, actorIdx);
			break;
		case 0x9:
			// TODO: updateCharacterSub03(1, actorIdx);
			break;
		case 0x6:
		case 0xA:
			frameNum = (frameNum + 1) % frameCount;
			break;
		case 0x8:
			// TODO: actor sound
			break;
		default:
			break;
		}
	}
}


bool Actor::defaultDirectionLoaded(int grResTableIdx) {
	return grResTable[grResTableIdx] != grResTable[5];
}

} // end of namespace Asylum
