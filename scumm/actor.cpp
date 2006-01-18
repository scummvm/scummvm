/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/akos.h"
#include "scumm/boxes.h"
#include "scumm/charset.h"
#include "scumm/costume.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/saveload.h"
#include "scumm/sound.h"
#include "scumm/sprite_he.h"
#include "scumm/usage_bits.h"
#include "scumm/util.h"

namespace Scumm {

byte Actor::kInvalidBox = 0;
ScummEngine *Actor::_vm = 0;

void Actor::initActorClass(ScummEngine *scumm) {
	_vm = scumm;
	if (_vm->_features & GF_SMALL_HEADER) {
		kInvalidBox = 255;
	}
}

Actor::Actor() {
	assert(_vm != 0);
	_number = 0;

	initActor(-1);
}

void Actor::initActor(int mode) {
	if (mode == -1) {
		_offsX = _offsY = 0;
		_top = _bottom = 0;
		_needRedraw = _needBgReset = _costumeNeedsInit = _visible = false;
		_flip = false;
		_speedx = 8;
		_speedy = 2;
		_frame = 0;
		_walkbox = 0;
		_animProgress = 0;
		_heSkipLimbs = false;
		_drawToBackBuf = false;
		memset(_animVariable, 0, sizeof(_animVariable));
		memset(_palette, 0, sizeof(_palette));
		memset(_sound, 0, sizeof(_sound));
		memset(&_cost, 0, sizeof(CostumeData));
		memset(&_walkdata, 0, sizeof(ActorWalkData));
		_walkdata.point3.x = 32000;
		_walkScript = 0;
		memset(_heTalkQueue, 0, sizeof(_heTalkQueue));

		mode = 1;
	}

	if (mode == 1) {
		_costume = 0;
		_room = 0;
		_pos.x = 0;
		_pos.y = 0;
		_facing = 180;
		_heCondMask = 1;
		_heNoTalkAnimation = 0;
		if (_vm->_version >= 7)
			_visible = false;
		_heSkipLimbs = false;
	} else if (mode == 2) {
		_facing = 180;
		_heCondMask = 1;
		_heSkipLimbs = false;
	}
	_elevation = 0;
	_width = 24;
	_talkColor = 15;
	_talkPosX = 0;
	_talkPosY = -80;
	_boxscale = _scaley = _scalex = 0xFF;
	_charset = 0;
	memset(_sound, 0, sizeof(_sound));
	_targetFacing = _facing;

	stopActorMoving();

	_heXmapNum = 0;
	_shadowMode = 0;
	_layer = 0;

	setActorWalkSpeed(8, 2);
	_animSpeed = 0;
	if (_vm->_version >= 6)
		_animProgress = 0;

	_ignoreBoxes = false;
	_forceClip = (_vm->_version >= 7) ? 100 : 0;
	_ignoreTurns = false;

	if (_vm->_heversion >= 61)
		_flip = 0;

	_talkFrequency = 256;
	_talkPan = 64;
	_talkVolume = 127;

	if (_vm->_version <= 2) {
		_initFrame = 2;
		_walkFrame = 0;
		_standFrame = 1;
		_talkStartFrame = 5;
		_talkStopFrame = 4;
	} else {
		_initFrame = 1;
		_walkFrame = 2;
		_standFrame = 3;
		_talkStartFrame = 4;
		_talkStopFrame = 5;
	}

	_heTalking = false;
	_walkScript = 0;
	_talkScript = 0;

	_clipOverride = _vm->_actorClipOverride;

	_auxBlock.reset();
	_hePaletteNum = 0;

	_vm->_classData[_number] = (_vm->_version >= 7) ? _vm->_classData[0] : 0;
}

void Actor::stopActorMoving() {
	if (_walkScript)
		_vm->stopScript(_walkScript);
	_moving = 0;
}

void Actor::setActorWalkSpeed(uint newSpeedX, uint newSpeedY) {
	if (newSpeedX == _speedx && newSpeedY == _speedy)
		return;

	_speedx = newSpeedX;
	_speedy = newSpeedY;

	if (_moving) {
		calcMovementFactor(_walkdata.next);
	}
}

int ScummEngine::getAngleFromPos(int x, int y) const {
	if (_gameId == GID_DIG || _gameId == GID_CMI) {
		double temp = atan2((double)x, (double)-y);
		return normalizeAngle((int)(temp * 180 / PI));
	} else {
		if (ABS(y) * 2 < ABS(x)) {
			if (x > 0)
				return 90;
			return 270;
		} else {
			if (y > 0)
				return 180;
			return 0;
		}
	}
}

int Actor::calcMovementFactor(const Common::Point& next) {
	Common::Point _actorPos(_pos);
	int diffX, diffY;
	int32 deltaXFactor, deltaYFactor;

	if (_actorPos == next)
		return 0;

	diffX = next.x - _actorPos.x;
	diffY = next.y - _actorPos.y;
	deltaYFactor = _speedy << 16;

	if (diffY < 0)
		deltaYFactor = -deltaYFactor;

	deltaXFactor = deltaYFactor * diffX;
	if (diffY != 0) {
		deltaXFactor /= diffY;
	} else {
		deltaYFactor = 0;
	}

	if ((uint) ABS((int)(deltaXFactor >> 16)) > _speedx) {
		deltaXFactor = _speedx << 16;
		if (diffX < 0)
			deltaXFactor = -deltaXFactor;

		deltaYFactor = deltaXFactor * diffY;
		if (diffX != 0) {
			deltaYFactor /= diffX;
		} else {
			deltaXFactor = 0;
		}
	}

	_walkdata.cur = _actorPos;
	_walkdata.next = next;
	_walkdata.deltaXFactor = deltaXFactor;
	_walkdata.deltaYFactor = deltaYFactor;
	_walkdata.xfrac = 0;
	_walkdata.yfrac = 0;

	_targetFacing = _vm->getAngleFromPos(deltaXFactor, deltaYFactor);

	return actorWalkStep();
}

int Actor::remapDirection(int dir, bool is_walking) {
	int specdir;
	byte flags;
	bool flipX;
	bool flipY;

	// FIXME - It seems that at least in The Dig the original code does
	// check _ignoreBoxes here. However, it breaks some animations in Loom,
	// causing Bobbin to face towards the camera instead of away from it
	// in some places: After the tree has been destroyed by lightning, and
	// when entering the dark tunnels beyond the dragon's lair at the very
	// least. Possibly other places as well.
	//
	// The Dig also checks if the actor is in the current room, but that's
	// not necessary here because we never call the function unless the
	// actor is in the current room anyway.

	if (!_ignoreBoxes || (_vm->_gameId == GID_LOOM || _vm->_gameId == GID_LOOM256)) {
		specdir = _vm->_extraBoxFlags[_walkbox];
		if (specdir) {
			if (specdir & 0x8000) {
				dir = specdir & 0x3FFF;
			} else {
				specdir = specdir & 0x3FFF;
				if (specdir - 90 < dir && dir < specdir + 90)
					dir = specdir;
				else
					dir = specdir + 180;
			}
		}

		flags = _vm->getBoxFlags(_walkbox);

		flipX = (_walkdata.deltaXFactor > 0);
		flipY = (_walkdata.deltaYFactor > 0);

		// Check for X-Flip
		if ((flags & kBoxXFlip) || isInClass(kObjectClassXFlip)) {
			dir = 360 - dir;
			flipX = !flipX;
		}
		// Check for Y-Flip
		if ((flags & kBoxYFlip) || isInClass(kObjectClassYFlip)) {
			dir = 180 - dir;
			flipY = !flipY;
		}

		switch (flags & 7) {
		case 1:
			if (_vm->_version >= 7) {
				if (dir < 180)
					return 90;
				else
					return 270;
			} else {
				if (is_walking)	                       // Actor is walking
					return flipX ? 90 : 270;
				else	                               // Actor is standing/turning
					return (dir == 90) ? 90 : 270;
			}
		case 2:
			if (_vm->_version >= 7) {
				if (dir > 90 && dir < 270)
					return 180;
				else
					return 0;
			} else {
				if (is_walking)	                       // Actor is walking
					return flipY ? 180 : 0;
				else	                               // Actor is standing/turning
					return (dir == 0) ? 0 : 180;
			}
		case 3:
			return 270;
		case 4:
			return 90;
		case 5:
			return 0;
		case 6:
			return 180;
		}
	}
	// OR 1024 in to signal direction interpolation should be done
	return normalizeAngle(dir) | 1024;
}

int Actor::updateActorDirection(bool is_walking) {
	int from;
	bool dirType = false;
	int dir;
	bool shouldInterpolate;

	if ((_vm->_version == 6) && _ignoreTurns)
		return _facing;

	dirType = (_vm->_version >= 7) ? _vm->_costumeLoader->hasManyDirections(_costume) : false;

	from = toSimpleDir(dirType, _facing);
	dir = remapDirection(_targetFacing, is_walking);

	if (_vm->_version >= 7)
		// Direction interpolation interfers with walk scripts in Dig; they perform
		// (much better) interpolation themselves.
		shouldInterpolate = false;
	else
		shouldInterpolate = (dir & 1024) ? true : false;
	dir &= 1023;

	if (shouldInterpolate) {
		int to = toSimpleDir(dirType, dir);
		int num = dirType ? 8 : 4;

		// Turn left or right, depending on which is shorter.
		int diff = to - from;
		if (ABS(diff) > (num >> 1))
			diff = -diff;

		if (diff > 0) {
			to = from + 1;
		} else if (diff < 0){
			to = from - 1;
		}

		dir = fromSimpleDir(dirType, (to + num) % num);
	}

	return dir;
}

void Actor::setBox(int box) {
	_walkbox = box;
	setupActorScale();
}

int Actor::actorWalkStep() {
	int tmpX, tmpY;
	Common::Point _actorPos;
	int distX, distY;
	int nextFacing;

	_needRedraw = true;

	nextFacing = updateActorDirection(true);
	if (!(_moving & MF_IN_LEG) || _facing != nextFacing) {
		if (_walkFrame != _frame || _facing != nextFacing) {
			startWalkAnim(1, nextFacing);
		}
		_moving |= MF_IN_LEG;
	}

	_actorPos = _pos;

	if (_walkbox != _walkdata.curbox && _vm->checkXYInBoxBounds(_walkdata.curbox, _actorPos.x, _actorPos.y)) {
		setBox(_walkdata.curbox);
	}

	distX = ABS(_walkdata.next.x - _walkdata.cur.x);
	distY = ABS(_walkdata.next.y - _walkdata.cur.y);

	if (ABS(_actorPos.x - _walkdata.cur.x) >= distX && ABS(_actorPos.y - _walkdata.cur.y) >= distY) {
		_moving &= ~MF_IN_LEG;
		return 0;
	}

	tmpX = (_actorPos.x << 16) + _walkdata.xfrac + (_walkdata.deltaXFactor >> 8) * _scalex;
	_walkdata.xfrac = (uint16)tmpX;
	_actorPos.x = (tmpX >> 16);

	tmpY = (_actorPos.y << 16) + _walkdata.yfrac + (_walkdata.deltaYFactor >> 8) * _scaley;
	_walkdata.yfrac = (uint16)tmpY;
	_actorPos.y = (tmpY >> 16);

	if (ABS(_actorPos.x - _walkdata.cur.x) > distX) {
		_actorPos.x = _walkdata.next.x;
	}

	if (ABS(_actorPos.y - _walkdata.cur.y) > distY) {
		_actorPos.y = _walkdata.next.y;
	}

	_pos = _actorPos;
	return 1;
}


void Actor::setupActorScale() {

	if (_vm->_features & GF_NO_SCALING) {
		_scalex = 0xFF;
		_scaley = 0xFF;
		return;
	}

	if (_ignoreBoxes)
		return;

	// For some boxes, we ignore the scaling and use whatever values the
	// scripts set. This is used e.g. in the Mystery Vortex in Sam&Max.
	// Older games used the flag 0x20 differently, though.
	if (_vm->_gameId == GID_SAMNMAX && (_vm->getBoxFlags(_walkbox) & kBoxIgnoreScale))
		return;

	_boxscale = _vm->getBoxScale(_walkbox);

	uint16 scale = _vm->getScale(_walkbox, _pos.x, _pos.y);
	assert(scale <= 0xFF);

	_scalex = _scaley = (byte)scale;
}

void Actor::startAnimActor(int f) {
	if (_vm->_version >= 7 && !((_vm->_gameId == GID_FT) && (_vm->_features & GF_DEMO) && (_vm->_platform == Common::kPlatformPC))) {
		switch (f) {
		case 1001:
			f = _initFrame;
			break;
		case 1002:
			f = _walkFrame;
			break;
		case 1003:
			f = _standFrame;
			break;
		case 1004:
			f = _talkStartFrame;
			break;
		case 1005:
			f = _talkStopFrame;
			break;
		}

		if (_costume != 0) {
			_animProgress = 0;
			_needRedraw = true;
			if (f == _initFrame)
				_cost.reset();
			_vm->_costumeLoader->costumeDecodeData(this, f, (uint) - 1);
			_frame = f;
		}
	} else {
		switch (f) {
		case 0x38:
			f = _initFrame;
			break;
		case 0x39:
			f = _walkFrame;
			break;
		case 0x3A:
			f = _standFrame;
			break;
		case 0x3B:
			f = _talkStartFrame;
			break;
		case 0x3C:
			f = _talkStopFrame;
			break;
		}

		assert(f != 0x3E);

		if (isInCurrentRoom() && _costume != 0) {
			_animProgress = 0;
			_cost.animCounter = 0;
			_needRedraw = true;
			// V1 - V2 games don't seem to need a _cost.reset() at this point.
			// Causes Zak to lose his body in several scenes, see bug #771508
			if (_vm->_version >= 3 && f == _initFrame) {
				_cost.reset();
				_auxBlock.reset();
			}
			_vm->_costumeLoader->costumeDecodeData(this, f, (uint) - 1);
			_frame = f;
		}
	}
}

void Actor::animateActor(int anim) {
	int cmd, dir;

	if (_vm->_version >= 7 && !((_vm->_gameId == GID_FT) && (_vm->_features & GF_DEMO) && (_vm->_platform == Common::kPlatformPC))) {

		if (anim == 0xFF)
			anim = 2000;

		cmd = anim / 1000;
		dir = anim % 1000;

	} else {

		cmd = anim / 4;
		dir = oldDirToNewDir(anim % 4);

		// Convert into old cmd code
		cmd = 0x3F - cmd + 2;

	}

	switch (cmd) {
	case 2:				// stop walking
		startAnimActor(_standFrame);
		stopActorMoving();
		break;
	case 3:				// change direction immediatly
		_moving &= ~MF_TURN;
		setDirection(dir);
		break;
	case 4:				// turn to new direction
		turnToDirection(dir);
		break;
	default:
		if (_vm->_version <= 2)
			startAnimActor(anim / 4);
		else
			startAnimActor(anim);
	}
}

void Actor::setDirection(int direction) {
	uint aMask;
	int i;
	uint16 vald;

	// Do nothing if actor is already facing in the given direction
	if (_facing == direction)
		return;

	// Normalize the angle
	_facing = normalizeAngle(direction);

	// If there is no costume set for this actor, we are finished
	if (_costume == 0)
		return;

	// Update the costume for the new direction (and mark the actor for redraw)
	aMask = 0x8000;
	for (i = 0; i < 16; i++, aMask >>= 1) {
		vald = _cost.frame[i];
		if (vald == 0xFFFF)
			continue;
		_vm->_costumeLoader->costumeDecodeData(this, vald, (_vm->_version <= 2) ? 0xFFFF : aMask);
	}

	_needRedraw = true;
}

void Actor::drawActorToBackBuf(int x, int y) {
	int curTop = _top;
	int curBottom = _bottom;

	_pos.x = x;
	_pos.y = y;

	_drawToBackBuf = true;
	_needRedraw = true;
	drawActorCostume();

	_drawToBackBuf = false;
	_needRedraw = true;
	drawActorCostume();
	_needRedraw = false;

	if (_top > curTop)
		_top = curTop;
	if (_bottom < curBottom)
		_bottom = curBottom;
}


void Actor::putActor(int dstX, int dstY, byte newRoom) {
	if (_visible && _vm->_currentRoom != newRoom && _vm->getTalkingActor() == _number) {
		_vm->stopTalk();
	}

	// WORKAROUND: The green transparency of the tank in the Hall of Oddities is
	// is positioned one pixel too far to the left. This appears to be a
	// bug in the original game as well.
	if (_vm->_gameId == GID_SAMNMAX && newRoom == 16 && _number == 5 && dstX == 235 && dstY == 236)
		dstX++;

	_pos.x = dstX;
	_pos.y = dstY;
	_room = newRoom;
	_needRedraw = true;

	if (_vm->VAR(_vm->VAR_EGO) == _number) {
		_vm->_egoPositioned = true;
	}

	if (_visible) {
		if (isInCurrentRoom()) {
			if (_moving) {
				stopActorMoving();
				startAnimActor(_standFrame);
			}
			adjustActorPos();
		} else {
#ifndef DISABLE_HE
			if (_vm->_heversion >= 71)
				((ScummEngine_v71he *)_vm)->queueAuxBlock(this);
#endif
			hideActor();
		}
	} else {
		if (isInCurrentRoom())
			showActor();
	}
}

int Actor::getActorXYPos(int &xPos, int &yPos) const {
	if (!isInCurrentRoom())
		return -1;

	xPos = _pos.x;
	yPos = _pos.y;
	return 0;
}

AdjustBoxResult Actor::adjustXYToBeInBox(int dstX, int dstY) {
	const uint thresholdTable[] = { 30, 80, 0 };
	AdjustBoxResult abr;
	int16 tmpX, tmpY;
	int tmpDist, bestDist, threshold, numBoxes;
	byte flags, bestBox;
	int box;
	const int firstValidBox = (_vm->_features & GF_SMALL_HEADER) ? 0 : 1;

	abr.x = dstX;
	abr.y = dstY;
	abr.box = kInvalidBox;

	if (_ignoreBoxes)
		return abr;

	for (int tIdx = 0; tIdx < ARRAYSIZE(thresholdTable); tIdx++) {
		threshold = thresholdTable[tIdx];

		numBoxes = _vm->getNumBoxes() - 1;
		if (numBoxes < firstValidBox)
			return abr;

		bestDist = (_vm->_version >= 7) ? 0x7FFFFFFF : 0xFFFF;
		if (_vm->_version <= 2)
			bestDist *= 8*2;	// Adjust for the fact that we multiply x by 8 and y by 2
		bestBox = kInvalidBox;

		// We iterate (backwards) over all boxes, searching the one closest
		// to the desired coordinates.
		for (box = numBoxes; box >= firstValidBox; box--) {
			flags = _vm->getBoxFlags(box);

			// Skip over invisible boxes
			if (flags & kBoxInvisible && !(flags & kBoxPlayerOnly && !isPlayer()))
				continue;

			// For increased performance, we perform a quick test if
			// the coordinates can even be within a distance of 'threshold'
			// pixels of the box.
			if (threshold > 0 && _vm->inBoxQuickReject(box, dstX, dstY, threshold))
				continue;

			// Check if the point is contained in the box. If it is,
			// we don't have to search anymore.
			if (_vm->checkXYInBoxBounds(box, dstX, dstY)) {
				abr.x = dstX;
				abr.y = dstY;
				abr.box = box;
				return abr;
			}

			// Find the point in the box which is closest to our point.
			tmpDist = _vm->getClosestPtOnBox(box, dstX, dstY, tmpX, tmpY);

			// Check if the box is closer than the previous boxes.
			if (tmpDist < bestDist) {
				abr.x = tmpX;
				abr.y = tmpY;

				if (tmpDist == 0) {
					abr.box = box;
					return abr;
				}
				bestDist = tmpDist;
				bestBox = box;
			}
		}

		// If the closest ('best') box we found is within the threshold, or if
		// we are on the last run (i.e. threshold == 0), return that box.
		if (threshold == 0 || threshold * threshold >= bestDist) {
			abr.box = bestBox;
			return abr;
		}
	}

	return abr;
}

void Actor::adjustActorPos() {
	AdjustBoxResult abr;

	abr = adjustXYToBeInBox(_pos.x, _pos.y);

	_pos.x = abr.x;
	_pos.y = abr.y;
	_walkdata.destbox = abr.box;

	setBox(abr.box);

	_walkdata.dest.x = -1;

	stopActorMoving();
	_cost.soundCounter = 0;

	if (_walkbox != kInvalidBox) {
		byte flags = _vm->getBoxFlags(_walkbox);
		if (flags & 7) {
			turnToDirection(_facing);
		}
	}
}

void Actor::faceToObject(int obj) {
	int x2, y2, dir;

	if (!isInCurrentRoom())
		return;

	if (_vm->getObjectOrActorXY(obj, x2, y2) == -1)
		return;

	dir = (x2 > _pos.x) ? 90 : 270;
	turnToDirection(dir);
}

void Actor::turnToDirection(int newdir) {
	if (newdir == -1 || _ignoreTurns)
		return;

	_moving &= ~MF_TURN;

	if (newdir != _facing) {
		if (_vm->_version <= 3)
			_moving = MF_TURN;
		else
			_moving |= MF_TURN;
		_targetFacing = newdir;
	}
}

void Actor::hideActor() {
	if (!_visible)
		return;

	if (_moving) {
		stopActorMoving();
		startAnimActor(_standFrame);
	}
	_visible = false;
	_cost.soundCounter = 0;
	_needRedraw = false;
	_needBgReset = true;
	_auxBlock.reset();
}

void Actor::showActor() {
	if (_vm->_currentRoom == 0 || _visible)
		return;

	adjustActorPos();

	_vm->ensureResourceLoaded(rtCostume, _costume);

	if (_costumeNeedsInit) {
		startAnimActor(_initFrame);
		if (_vm->_version <= 2) {
			startAnimActor(_standFrame);
			startAnimActor(_talkStopFrame);
		}
		_costumeNeedsInit = false;
	}

	// FIXME: Evil hack to work around bug #770717
	if (!_moving && _vm->_version <= 2)
		startAnimActor(_standFrame);

	stopActorMoving();
	_visible = true;
	_needRedraw = true;
}

// V1 Maniac doesn't have a ScummVar for VAR_TALK_ACTOR, and just uses
// an internal variable. Emulate this to prevent overwriting script vars...
// Maniac NES (V1), however, DOES have a ScummVar for VAR_TALK_ACTOR
int ScummEngine::getTalkingActor() {
	if (_gameId == GID_MANIAC && _version == 1 && !(_platform == Common::kPlatformNES))
		return _V1TalkingActor;
	else
		return VAR(VAR_TALK_ACTOR);
}

void ScummEngine::setTalkingActor(int value) {
	if (_gameId == GID_MANIAC && _version == 1 && !(_platform == Common::kPlatformNES))
		_V1TalkingActor = value;
	else
		VAR(VAR_TALK_ACTOR) = value;
}

void ScummEngine::putActors() {
	Actor *a;
	int i;

	for (i = 1; i < _numActors; i++) {
		a = &_actors[i];
		if (a && a->isInCurrentRoom())
			a->putActor(a->_pos.x, a->_pos.y, a->_room);
	}
}

static const int c64MMActorTalkColor[25] = {
	1, 7, 2, 14, 8, 15, 3, 7, 7, 15, 1, 13, 1, 4, 5, 5, 4, 3, 1, 5, 1, 1, 1, 1, 7
};
static const int v1MMActorTalkColor[25] = {
	1, 7, 2, 14, 8, 1, 3, 7, 7, 12, 1, 13, 1, 4, 5, 5, 4, 3, 1, 5, 1, 1, 1, 7, 7
};

void ScummEngine::setupV1ActorTalkColor() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_platform == Common::kPlatformC64) {
			_actors[i]._talkColor = c64MMActorTalkColor[i];
		} else {
			_actors[i]._talkColor = v1MMActorTalkColor[i];
		}
	}
}

void ScummEngine::showActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom())
			_actors[i].showActor();
	}
}

void ScummEngine::walkActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom())
			if (_version <= 3)
				_actors[i].walkActorOld();
			else
				_actors[i].walkActor();
	}
}

/* Used in Scumm v5 only. Play sounds associated with actors */
void ScummEngine::playActorSounds() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i]._cost.soundCounter && _actors[i].isInCurrentRoom() && _actors[i]._sound) {
			_currentScript = 0xFF;
			_sound->addSoundToQueue(_actors[i]._sound[0]);
			for (i = 1; i < _numActors; i++) {
				_actors[i]._cost.soundCounter = 0;
			}
			return;
		}
	}
}

bool ScummEngine::isValidActor(int id) const {
	return id >= 0 && id < _numActors && _actors[id]._number == id;
}

Actor *ScummEngine::derefActor(int id, const char *errmsg) const {
	if (id == 0)
		debugC(DEBUG_ACTORS, "derefActor(0, \"%s\") in script %d, opcode 0x%x",
			errmsg, vm.slot[_currentScript].number, _opcode);

	if (!isValidActor(id)) {
		if (errmsg)
			error("Invalid actor %d in %s", id, errmsg);
		else
			error("Invalid actor %d", id);
	}
	return &_actors[id];
}

Actor *ScummEngine::derefActorSafe(int id, const char *errmsg) const {
	if (id == 0)
		debugC(DEBUG_ACTORS, "derefActorSafe(0, \"%s\") in script %d, opcode 0x%x",
			errmsg, vm.slot[_currentScript].number, _opcode);

	if (!isValidActor(id)) {
		debugC(DEBUG_ACTORS, "Invalid actor %d in %s (script %d, opcode 0x%x)",
			 id, errmsg, vm.slot[_currentScript].number, _opcode);
		return NULL;
	}
	return &_actors[id];
}

void ScummEngine::processActors() {
	int numactors = 0;

	// Make a list of all actors in this room
	for (int i = 1; i < _numActors; i++) {
		if (_version == 8 && _actors[i]._layer < 0)
			continue;
		if (_actors[i].isInCurrentRoom()) {
			_sortedActors[numactors++] = &_actors[i];
		}
	}
	if (!numactors) {
		return;
	}

	// Sort actors by position before drawing them (to ensure that actors
	// in front are drawn after those "behind" them).
	//
	// Note: This algorithm works exactly the way the original engine did.
	// Please resist any urge to 'optimize' this. Many of the games rely on
	// the quirks of this particular sorting algorithm, and since we are
	// dealing with far less than 100 objects being sorted here, any
	// 'optimization' wouldn't yield a useful gain anyway.
	//
	// In particular, changing this loop caused a number of bugs in the
	// past, including bugs #758167, #775097, and #1093867.
	//
	// Note that Sam & Max uses a stable sorting method. Older games don't
	// and, according to cyx, neither do newer ones. At least not FT and
	// COMI. See bug #1220168 for more details.

	if (_gameId == GID_SAMNMAX) {
		for (int j = 0; j < numactors; ++j) {
			for (int i = 0; i < numactors; ++i) {
				int sc_actor1 = _sortedActors[j]->_pos.y;
				int sc_actor2 = _sortedActors[i]->_pos.y;
				if (sc_actor1 == sc_actor2) {
					sc_actor1 += _sortedActors[j]->_number;
					sc_actor2 += _sortedActors[i]->_number;
				}
				if (sc_actor1 < sc_actor2) {
					SWAP(_sortedActors[i], _sortedActors[j]);
				}
			}
		}
	} else {
		for (int j = 0; j < numactors; ++j) {
			for (int i = 0; i < numactors; ++i) {
				int sc_actor1 = _sortedActors[j]->_pos.y - _sortedActors[j]->_layer * 2000;
				int sc_actor2 = _sortedActors[i]->_pos.y - _sortedActors[i]->_layer * 2000;
				if (sc_actor1 < sc_actor2) {
					SWAP(_sortedActors[i], _sortedActors[j]);
				}
			}
		}
	}

	// Finally draw the now sorted actors
	Actor** end = _sortedActors + numactors;
	for (Actor** ac = _sortedActors; ac != end; ++ac) {
		Actor* a = *ac;
		// Draw and animate the actors, except those w/o a costume.
		// Note: We could 'optimize' this a little bit by only putting
		// actors with a costume into the _sortedActors array in the
		// first place. However, that would mess up the sorting, and
		// would hence cause regressions. See also the other big
		// comment further up in this method for some details.
		if (a->_costume) {
			a->drawActorCostume();
			a->animateCostume();
		}
	}

	if (_features & GF_NEW_COSTUMES)
		akos_processQueue();
}

#ifndef DISABLE_HE
void ScummEngine_v71he::processActors() {
	preProcessAuxQueue();

	if (!_skipProcessActors)
		ScummEngine::processActors();

	_fullRedraw = false;

	postProcessAuxQueue();
}

void ScummEngine_v90he::processActors() {
	preProcessAuxQueue();

	_sprite->setRedrawFlags(false);
	_sprite->processImages(true);

	if (!_skipProcessActors)
		ScummEngine::processActors();

	_fullRedraw = false;

	postProcessAuxQueue();

	_sprite->setRedrawFlags(true);
	_sprite->processImages(false);
}
#endif

// Used in Scumm v8, to allow the verb coin to be drawn over the inventory
// chest. I'm assuming that draw order won't matter here.
void ScummEngine::processUpperActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i].isInCurrentRoom() && _actors[i]._costume && _actors[i]._layer < 0) {
			CHECK_HEAP
			_actors[i].drawActorCostume();
			CHECK_HEAP
			_actors[i].animateCostume();
		}
	}
}

void Actor::drawActorCostume(bool hitTestMode) {
	if (_costume == 0)
		return;

	if (!hitTestMode) {
		if (!_needRedraw)
			return;

		_needRedraw = false;
	}

	setupActorScale();

	BaseCostumeRenderer* bcr = _vm->_costumeRenderer;

	bcr->_actorID = _number;

	bcr->_actorX = _pos.x + _offsX - _vm->virtscr[0].xstart;
	bcr->_actorY = _pos.y + _offsY - _elevation;

	if (_vm->_platform == Common::kPlatformNES) {
		// In the NES version, when the actor is facing right,
		// we need to shift it 8 pixels to the left
		if (_facing == 90)
			bcr->_actorX -= 8;
	} else if (_vm->_version <= 2) {
		// HACK: We have to adjust the x position by one strip (8 pixels) in
		// V2 games. However, it is not quite clear to me why. And to fully
		// match the original, it seems we have to offset by 2 strips if the
		// actor is facing left (270 degree).
		// V1 games are once again slightly different, here we only have
		// to adjust the 270 degree case...
		if (_facing == 270)
			bcr->_actorX += 16;
		else if (_vm->_version == 2)
			bcr->_actorX += 8;
	}

	bcr->_clipOverride = _clipOverride;

	if (_vm->_version == 4 && _boxscale & 0x8000) {
		bcr->_scaleX = bcr->_scaleY = _vm->getScaleFromSlot((_boxscale & 0x7fff) + 1, _pos.x, _pos.y);
	} else {
		bcr->_scaleX = _scalex;
		bcr->_scaleY = _scaley;
	}

	bcr->_shadow_mode = _shadowMode;
	if (_vm->_version >= 5 && _vm->_heversion == 0) {
		bcr->_shadow_table = _vm->_shadowPalette;
	} else if (_vm->_heversion == 70) {
		bcr->_shadow_table = _vm->_HEV7ActorPalette;
	}

	bcr->setCostume(_costume, _heXmapNum);
	bcr->setPalette(_palette);
	bcr->setFacing(this);

	if (_vm->_version >= 7) {

		bcr->_zbuf = _forceClip;
		if (bcr->_zbuf == 100) {
			bcr->_zbuf = _vm->getMaskFromBox(_walkbox);
			if (bcr->_zbuf > _vm->gdi._numZBuffer-1)
				bcr->_zbuf = _vm->gdi._numZBuffer-1;
		}

	} else {
		if (_forceClip)
			bcr->_zbuf = _forceClip;
		else if (isInClass(kObjectClassNeverClip))
			bcr->_zbuf = 0;
		else {
			bcr->_zbuf = _vm->getMaskFromBox(_walkbox);
			if (bcr->_zbuf > _vm->gdi._numZBuffer-1)
				bcr->_zbuf = _vm->gdi._numZBuffer-1;
		}

	}

	bcr->_draw_top = 0x7fffffff;
	bcr->_draw_bottom = 0;

	bcr->_skipLimbs = (_heSkipLimbs != 0);
	bcr->_paletteNum = _hePaletteNum;

	if (_vm->_heversion >= 80 && _heNoTalkAnimation == 0 && _animProgress == 0) {
		if (_vm->getTalkingActor() == _number && !_vm->_string[0].no_talk_anim) {
			int talkState = 0;

			if (_vm->_sound->isSoundCodeUsed(1))
				talkState = _vm->_sound->getSoundVar(1, 19);
			if (talkState == 0)
				talkState = _vm->_rnd.getRandomNumberRng(1, 10);

			checkRange(13, 1, talkState, "Talk state %d out of range");
			setTalkCondition(talkState);
		} else {
			setTalkCondition(1);
		}
	}
	_heNoTalkAnimation = 0;

	// If the actor is partially hidden, redraw it next frame.
	// Only done for pre-AKOS, though.
	if (bcr->drawCostume(_vm->virtscr[0], _vm->gdi._numStrips, this, _drawToBackBuf) & 1) {
		_needRedraw = (_vm->_version <= 6);
	}

	if (!hitTestMode) {
		// Record the vertical extent of the drawn actor
		_top = bcr->_draw_top;
		_bottom = bcr->_draw_bottom;
	}
}

#ifndef DISABLE_SCUMM_7_8
bool Actor::actorHitTest(int x, int y) {
	AkosRenderer *ar = (AkosRenderer *)_vm->_costumeRenderer;

	ar->_actorHitX = x;
	ar->_actorHitY = y;
	ar->_actorHitMode = true;
	ar->_actorHitResult = false;

	drawActorCostume(true);

	ar->_actorHitMode = false;

	return ar->_actorHitResult;
}
#endif

void Actor::animateCostume() {
	if (_costume == 0)
		return;

	_animProgress++;
	if (_animProgress >= _animSpeed) {
		_animProgress = 0;

		_vm->_costumeLoader->loadCostume(_costume);
		if (_vm->_costumeLoader->increaseAnims(this)) {
			_needRedraw = true;
		}
	}
}

#ifndef DISABLE_SCUMM_7_8
void Actor::animateLimb(int limb, int f) {
	// This methods is very similiar to animateCostume().
	// However, instead of animating *all* the limbs, it only animates
	// the specified limb to be at the frame specified by "f".

	if (!f)
		return;

	_animProgress++;
	if (_animProgress >= _animSpeed) {
		_animProgress = 0;

		if (_costume == 0)
			return;

		const byte *aksq, *akfo;
		uint size;
		byte *akos = _vm->getResourceAddress(rtCostume, _costume);
		assert(akos);

		aksq = _vm->findResourceData(MKID('AKSQ'), akos);
		akfo = _vm->findResourceData(MKID('AKFO'), akos);

		size = _vm->getResourceDataSize(akfo) / 2;

		while (f--) {
			if (_cost.active[limb] != 0)
				_vm->akos_increaseAnim(this, limb, aksq, (const uint16 *)akfo, size);
		}

//		_needRedraw = true;
//		_needBgReset = true;
	}
}
#endif

void ScummEngine::redrawAllActors() {
	int j;

	for (j = 1; j < _numActors; j++) {
		_actors[j]._needRedraw = true;
		_actors[j]._needBgReset = true;
	}
}

void ScummEngine::setActorRedrawFlags() {
	int i, j;

	// Redraw all actors if a full redraw was requested.
	// Also redraw all actors in COMI (see bug #1066329 for details).
	if (_fullRedraw || _version == 8 || (VAR_REDRAW_ALL_ACTORS != 0xFF && VAR(VAR_REDRAW_ALL_ACTORS) != 0)) {
		for (j = 1; j < _numActors; j++) {
			_actors[j]._needRedraw = true;
		}
	} else {
		for (i = 0; i < gdi._numStrips; i++) {
			int strip = _screenStartStrip + i;
			if (testGfxAnyUsageBits(strip)) {
				for (j = 1; j < _numActors; j++) {
					if (testGfxUsageBit(strip, j) && testGfxOtherUsageBits(strip, j)) {
						_actors[j]._needRedraw = true;
					}
				}
			}
		}
	}
}

void ScummEngine::resetActorBgs() {
	int i, j;

	for (i = 0; i < gdi._numStrips; i++) {
		int strip = _screenStartStrip + i;
		clearGfxUsageBit(strip, USAGE_BIT_DIRTY);
		clearGfxUsageBit(strip, USAGE_BIT_RESTORED);
		for (j = 1; j < _numActors; j++) {
			if (testGfxUsageBit(strip, j) &&
				((_actors[j]._top != 0x7fffffff && _actors[j]._needRedraw) || _actors[j]._needBgReset)) {
				clearGfxUsageBit(strip, j);
				if ((_actors[j]._bottom - _actors[j]._top) >= 0)
					gdi.resetBackground(_actors[j]._top, _actors[j]._bottom, i);
			}
		}
	}

	for (i = 1; i < _numActors; i++) {
		_actors[i]._needBgReset = false;
	}
}

int ScummEngine::getActorFromPos(int x, int y) {
	int i;

	if (!testGfxAnyUsageBits(x / 8))
		return 0;

	for (i = 1; i < _numActors; i++) {
		if (testGfxUsageBit(x / 8, i) && !getClass(i, kObjectClassUntouchable)
			&& y >= _actors[i]._top && y <= _actors[i]._bottom) {
			if (_version > 2 || i != VAR(VAR_EGO))
				return i;
		}
	}

	return 0;
}

#ifndef DISABLE_HE
int ScummEngine_v70he::getActorFromPos(int x, int y) {
	int curActor, i;

	if (!testGfxAnyUsageBits(x / 8))
		return 0;

	curActor = 0;
	for (i = 1; i < _numActors; i++) {
		if (testGfxUsageBit(x / 8, i) && !getClass(i, kObjectClassUntouchable)
			&& y >= _actors[i]._top && y <= _actors[i]._bottom
			&& (_actors[i]._pos.y > _actors[curActor]._pos.y || curActor == 0)) 
				curActor = i;
	}

	return curActor;
}
#endif

#ifndef DISABLE_SCUMM_7_8
void ScummEngine_v7::actorTalk(const byte *msg) {
	Actor *a;

	convertMessageToString(msg, _charsetBuffer, sizeof(_charsetBuffer));

	// Play associated speech, if any
	playSpeech((byte *)_lastStringTag);

	if ((_version == 7 && !_keepText) || (_version == 8 && VAR(VAR_HAVE_MSG))) {
		stopTalk();
	}
	if (_actorToPrintStrFor == 0xFF) {
		setTalkingActor(0xFF);
	} else {
		a = derefActor(_actorToPrintStrFor, "actorTalk");
		setTalkingActor(a->_number);
		if (!_string[0].no_talk_anim) {
			a->runActorTalkScript(a->_talkStartFrame);
			_useTalkAnims = true;
		}
	}

	if (getTalkingActor() > 0x7F) {
		_charsetColor = (byte)_string[0].color;
	} else {
		a = derefActor(getTalkingActor(), "actorTalk(2)");
		_charsetColor = a->_talkColor;
	}
	_charsetBufPos = 0;
	_talkDelay = 0;
	_haveMsg = 1;
	if (_version == 7)
		VAR(VAR_HAVE_MSG) = 0xFF;
	_haveActorSpeechMsg = true;
	CHARSET_1();
	if (_version == 8)
		VAR(VAR_HAVE_MSG) = (_string[0].no_talk_anim) ? 2 : 1;
}
#endif

void ScummEngine::actorTalk(const byte *msg) {
	Actor *a;

	convertMessageToString(msg, _charsetBuffer, sizeof(_charsetBuffer));

	// FIXME: Workaround for bugs #770039 and #770049
	if (_gameId == GID_LOOM || _gameId == GID_LOOM256) {
		if (!*_charsetBuffer)
			return;
	}

	if (_actorToPrintStrFor == 0xFF) {
		if (!_keepText) {
			stopTalk();
		}
		setTalkingActor(0xFF);
	} else {
		int oldact;

		// FIXME: Workaround for bug #770724
		if (_gameId == GID_LOOM && _roomResource == 23 &&
			vm.slot[_currentScript].number == 232 && _actorToPrintStrFor == 0) {
			_actorToPrintStrFor = 2;	// Could be anything from 2 to 5. Maybe compare to original?
		}

		a = derefActor(_actorToPrintStrFor, "actorTalk");
		if (!a->isInCurrentRoom()) {
			oldact = 0xFF;
		} else {
			if (!_keepText) {
				stopTalk();
			}
			setTalkingActor(a->_number);
			a->_heTalking = true;
			if (!_string[0].no_talk_anim) {
				a->runActorTalkScript(a->_talkStartFrame);
				_useTalkAnims = true;
			}
			oldact = getTalkingActor();
		}
		if (oldact >= 0x80)
			return;
	}

	if (_heversion >= 72 || getTalkingActor() > 0x7F) {
		_charsetColor = (byte)_string[0].color;
	} else if (_platform == Common::kPlatformNES) {
		if (_NES_lastTalkingActor != getTalkingActor())
			_NES_talkColor ^= 1;
		_NES_lastTalkingActor = getTalkingActor();
		_charsetColor = _NES_talkColor;
	} else {
		a = derefActor(getTalkingActor(), "actorTalk(2)");
		_charsetColor = a->_talkColor;
	}
	_charsetBufPos = 0;
	_talkDelay = 0;
	_haveMsg = 0xFF;
	VAR(VAR_HAVE_MSG) = 0xFF;
	if (VAR_CHARCOUNT != 0xFF)
		VAR(VAR_CHARCOUNT) = 0;
	_haveActorSpeechMsg = true;
	CHARSET_1();
}

void Actor::runActorTalkScript(int f) {
	if (_vm->_version == 8 && _vm->VAR(_vm->VAR_HAVE_MSG) == 2)
		return;

	if (_talkScript) {
		int script = _talkScript;
		int args[16];
		memset(args, 0, sizeof(args));
		args[1] = f;
		args[0] = _number;

		_vm->runScript(script, 1, 0, args);
	} else {
		if (_frame != f)
			startAnimActor(f);
	}
}

void ScummEngine::stopTalk() {
	int act;

	_sound->stopTalkSound();

	_haveMsg = 0;
	_talkDelay = 0;

	act = getTalkingActor();
	if (act && act < 0x80) {
		Actor *a = derefActor(act, "stopTalk");
		if ((_version >= 7 && !_string[0].no_talk_anim) ||
			(_version <= 6 && a->isInCurrentRoom() && _useTalkAnims)) {
			a->runActorTalkScript(a->_talkStopFrame);
			_useTalkAnims = false;
		}
		if (_version <= 7 && _heversion == 0)
			setTalkingActor(0xFF);
		a->_heTalking = false;
	}
	if (_version == 8 || _heversion >= 60)
		setTalkingActor(0);
	if (_version == 8)
		VAR(VAR_HAVE_MSG) = 0;
	_keepText = false;
	if (_version >= 7) {
#ifndef DISABLE_SCUMM_7_8
		((ScummEngine_v7 *)this)->clearSubtitleQueue();
#endif
	} else {
		_charset->restoreCharsetBg();
	}
}

void Actor::setActorCostume(int c) {
	int i;

	if (_vm->_heversion >= 61 && (c == -1  || c == -2)) {
		_heSkipLimbs = (c == -1);
		_needRedraw = true;
		return;
	}

	// Based on disassembly. It seems that high byte is not used at all, though
	// it is attached to all horizontally flipped object, like left eye.
	if (_vm->_heversion == 61)
		c &= 0xff;

	_costumeNeedsInit = true;

	if (_vm->_features & GF_NEW_COSTUMES) {
		memset(_animVariable, 0, sizeof(_animVariable));

#ifndef DISABLE_HE
		if (_vm->_heversion >= 71)
			((ScummEngine_v71he *)_vm)->queueAuxBlock(this);
#endif

		_costume = c;
		_cost.reset();
		_auxBlock.reset();

		if (_visible) {
			if (_costume) {
				_vm->ensureResourceLoaded(rtCostume, _costume);
			}
			startAnimActor(_initFrame);
		}
	} else {
		if (_visible) {
			hideActor();
			_cost.reset();
			_costume = c;
			showActor();
		} else {
			_costume = c;
			_cost.reset();
		}
	}


	// V1 zak uses palette[] as a dynamic costume color array.
	if (_vm->_version == 1)
		return;

	if (_vm->_features & GF_NEW_COSTUMES) {
		for (i = 0; i < 256; i++)
			_palette[i] = 0xFF;
	} else if (_vm->_features & GF_OLD_BUNDLE) {
		for (i = 0; i < 16; i++)
			_palette[i] = i;

		// Make stuff more visible on CGA. Based on disassembly
		if (_vm->_renderMode == Common::kRenderCGA && _vm->_version > 2) {
			_palette[6] = 5;
			_palette[7] = 15;
		}
	} else {
		for (i = 0; i < 32; i++)
			_palette[i] = 0xFF;
	}

	if (_vm->_heversion >= 71 && _vm->getTalkingActor() == _number) {
		if (_vm->_heversion <= 95 || (_vm->_heversion >= 98 && _vm->VAR(_vm->VAR_SKIP_RESET_TALK_ACTOR) == 0)) {
			//_vm->setTalkingActor(0);
		}
	}
}

void Actor::startWalkActor(int destX, int destY, int dir) {
	AdjustBoxResult abr;

	if (_vm->_version <= 3) {
		abr.x = destX;
		abr.y = destY;
	} else {
		abr = adjustXYToBeInBox(destX, destY);
	}

	if (!isInCurrentRoom()) {
		_pos.x = abr.x;
		_pos.y = abr.y;
		if (!(_vm->_version == 6 && _ignoreTurns) && dir != -1)
			setDirection(dir);
		return;
	}

	if (_ignoreBoxes) {
		abr.box = kInvalidBox;
		_walkbox = kInvalidBox;
	} else {
		if (_vm->checkXYInBoxBounds(_walkdata.destbox, abr.x, abr.y)) {
			abr.box = _walkdata.destbox;
		} else {
			abr = adjustXYToBeInBox(abr.x, abr.y);
		}
		if (_moving && _walkdata.destdir == dir && _walkdata.dest.x == abr.x && _walkdata.dest.y == abr.y)
			return;
	}

	if (_pos.x == abr.x && _pos.y == abr.y) {
		turnToDirection(dir);
		return;
	}

	_walkdata.dest.x = abr.x;
	_walkdata.dest.y = abr.y;
	_walkdata.destbox = abr.box;
	_walkdata.destdir = dir;
	_moving = (_moving & MF_IN_LEG) | MF_NEW_LEG;
	_walkdata.point3.x = 32000;

	_walkdata.curbox = _walkbox;
}

void Actor::startWalkAnim(int cmd, int angle) {
	if (angle == -1)
		angle = _facing;

	/* Note: walk scripts aren't required to make the Dig
	 * work as usual
	 */
	if (_walkScript) {
		int args[16];
		memset(args, 0, sizeof(args));
		args[0] = _number;
		args[1] = cmd;
		args[2] = angle;
		_vm->runScript(_walkScript, 1, 0, args);
	} else {
		switch (cmd) {
		case 1:										/* start walk */
			setDirection(angle);
			startAnimActor(_walkFrame);
			break;
		case 2:										/* change dir only */
			setDirection(angle);
			break;
		case 3:										/* stop walk */
			turnToDirection(angle);
			startAnimActor(_standFrame);
			break;
		}
	}
}

void Actor::walkActor() {
	int new_dir, next_box;
	Common::Point foundPath;

	if (_vm->_version >= 7) {
		if (_moving & MF_FROZEN) {
			if (_moving & MF_TURN) {
				new_dir = updateActorDirection(false);
				if (_facing != new_dir)
					setDirection(new_dir);
				else
					_moving &= ~MF_TURN;
			}
			return;
		}
	}

	if (!_moving)
		return;

	if (!(_moving & MF_NEW_LEG)) {
		if (_moving & MF_IN_LEG && actorWalkStep())
			return;

		if (_moving & MF_LAST_LEG) {
			_moving = 0;
			setBox(_walkdata.destbox);
			startWalkAnim(3, _walkdata.destdir);
			return;
		}

		if (_moving & MF_TURN) {
			new_dir = updateActorDirection(false);
			if (_facing != new_dir)
				setDirection(new_dir);
			else
				_moving = 0;
			return;
		}

		setBox(_walkdata.curbox);
		_moving &= MF_IN_LEG;
	}

	_moving &= ~MF_NEW_LEG;
	do {

		if (_walkbox == kInvalidBox) {
			setBox(_walkdata.destbox);
			_walkdata.curbox = _walkdata.destbox;
			break;
		}

		if (_walkbox == _walkdata.destbox)
			break;

		next_box = _vm->getPathToDestBox(_walkbox, _walkdata.destbox);
		if (next_box < 0) {
			_walkdata.destbox = _walkbox;
			_moving |= MF_LAST_LEG;
			return;
		}

		_walkdata.curbox = next_box;

		if (findPathTowards(_walkbox, next_box, _walkdata.destbox, foundPath))
			break;

		if (calcMovementFactor(foundPath))
			return;

		setBox(_walkdata.curbox);
	} while (1);

	_moving |= MF_LAST_LEG;
	calcMovementFactor(_walkdata.dest);
}

/*
void Actor::walkActorV12() {
	Common::Point foundPath, tmp;
	int new_dir, next_box;

	if (_moving & MF_TURN) {
		new_dir = updateActorDirection(false);
		if (_facing != new_dir)
			setDirection(new_dir);
		else
			_moving = 0;
		return;
	}

	if (!_moving)
		return;

	if (_moving & MF_IN_LEG) {
		actorWalkStep();
	} else {
		if (_moving & MF_LAST_LEG) {
			_moving = 0;
			startWalkAnim(3, _walkdata.destdir);
		} else {
			setBox(_walkdata.curbox);
			if (_walkbox == _walkdata.destbox) {
				foundPath = _walkdata.dest;
				_moving |= MF_LAST_LEG;
			} else {
				next_box = _vm->getPathToDestBox(_walkbox, _walkdata.destbox);
				if (next_box < 0) {
					_moving |= MF_LAST_LEG;
					return;
				}

				// Can't walk through locked boxes
				int flags = _vm->getBoxFlags(next_box);
				if (flags & kBoxLocked && !(flags & kBoxPlayerOnly && !isPlayer())) {
					_moving |= MF_LAST_LEG;
				}

				_walkdata.curbox = next_box;

				_vm->getClosestPtOnBox(_walkdata.curbox, x, y, tmp.x, tmp.y);
				_vm->getClosestPtOnBox(_walkbox, tmp.x, tmp.y, foundPath.x, foundPath.y);
			}
			calcMovementFactor(foundPath);
		}
	}
}
*/

void Actor::walkActorOld() {
	Common::Point p2, p3;	// Gate locations
	int new_dir, next_box;

	if (!_moving)
		return;

	if (!(_moving & MF_NEW_LEG)) {
		if (_moving & MF_IN_LEG && actorWalkStep())
			return;

		if (_moving & MF_LAST_LEG) {
			_moving = 0;
			startWalkAnim(3, _walkdata.destdir);
			return;
		}

		if (_moving & MF_TURN) {
			new_dir = updateActorDirection(false);
			if (_facing != new_dir)
				setDirection(new_dir);
			else
				_moving = 0;
			return;
		}

		if (_walkdata.point3.x != 32000) {
			if (calcMovementFactor(_walkdata.point3)) {
				_walkdata.point3.x = 32000;
				return;
			}
			_walkdata.point3.x = 32000;
		}

		setBox(_walkdata.curbox);
		_moving &= MF_IN_LEG;
	}

	_moving &= ~MF_NEW_LEG;
	do {
		if (_walkbox == kInvalidBox) {
			setBox(_walkdata.destbox);
			_walkdata.curbox = _walkdata.destbox;
			break;
		}

		if (_walkbox == _walkdata.destbox)
			break;

		next_box = _vm->getPathToDestBox(_walkbox, _walkdata.destbox);

		// WORKAROUND: To fully fix bug #774783, we add a special case
		// here, resulting in a different next_box value for Hitler.
		if ((_vm->_gameId == GID_INDY3) && _vm->_roomResource == 46 && _walkbox == 1 && _walkdata.destbox == 0 && _number == 9)
			next_box = 1;

		if (next_box < 0) {
			_moving |= MF_LAST_LEG;
			return;
		}

		// Can't walk through locked boxes
		int flags = _vm->getBoxFlags(next_box);
		if (flags & kBoxLocked && !(flags & kBoxPlayerOnly && !isPlayer())) {
			_moving |= MF_LAST_LEG;
// FIXME: Work in progress
//			_walkdata.destdir = _facing;
			return;
		}

		_walkdata.curbox = next_box;

		if (_vm->_version <= 2) {
			_vm->getClosestPtOnBox(_walkdata.curbox, _pos.x, _pos.y, p2.x, p2.y);
			_vm->getClosestPtOnBox(_walkbox, p2.x, p2.y, p3.x, p3.y);
// FIXME: Work in progress
//			calcMovementFactor(p3);
//			return;
		} else {
			findPathTowardsOld(_walkbox, next_box, _walkdata.destbox, p2, p3);
			if (p2.x == 32000 && p3.x == 32000) {
				break;
			}

			if (p2.x != 32000) {
				if (calcMovementFactor(p2)) {
					_walkdata.point3 = p3;
					return;
				}
			}
		}
		if (calcMovementFactor(p3))
			return;

		setBox(_walkdata.destbox);
	} while (1);

	_moving |= MF_LAST_LEG;
	calcMovementFactor(_walkdata.dest);
}

byte *Actor::getActorName() {
	byte *ptr = _vm->getResourceAddress(rtActorName, _number);
	if (ptr == NULL) {
		debug(0, "Failed to find name of actor %d", _number);
	}
	return ptr;
}

int Actor::getAnimVar(byte var) const {
	checkRange(26, 0, var, "getAnimVar %d out of range(r)");
	return _animVariable[var];
}

void Actor::setAnimVar(byte var, int value) {
	checkRange(26, 0, var, "setAnimVar %d out of range(r)");
	_animVariable[var] = value;
}

void Actor::remapActorPaletteColor(int color, int new_color) {
	const byte *akos, *akpl;
	int akpl_size, i;
	byte akpl_color;

	akos = _vm->getResourceAddress(rtCostume, _costume);
	if (!akos) {
		debug(0, "Can't remap actor %d, costume %d not found", _number, _costume);
		return;
	}

	akpl = _vm->findResourceData(MKID('AKPL'), akos);
	if (!akpl) {
		debug(0, "Can't remap actor %d, costume %d doesn't contain an AKPL block", _number, _costume);
		return;
	}

	// Get the number palette entries
	akpl_size = _vm->getResourceDataSize(akpl);

	for (i = 0; i < akpl_size; i++) {
		akpl_color = *akpl++;
		if (akpl_color == color) {
			_palette[i] = new_color;
			return;
		}
	}
}

void Actor::remapActorPalette(int r_fact, int g_fact, int b_fact, int threshold) {
	const byte *akos, *rgbs, *akpl;
	int akpl_size, i;
	int r, g, b;
	byte akpl_color;

	if (!isInCurrentRoom()) {
		debugC(DEBUG_ACTORS, "Remap actor %d not in current room", _number);
		return;
	} else if (_costume < 1 || _costume >= _vm->_numCostumes - 1) {
		debugC(DEBUG_ACTORS, "Remap actor %d invalid costume %d", _number, _costume);
		return;
	}

	akos = _vm->getResourceAddress(rtCostume, _costume);
	if (!akos) {
		debug(0, "Can't remap actor %d, costume %d not found", _number, _costume);
		return;
	}

	akpl = _vm->findResourceData(MKID('AKPL'), akos);
	if (!akpl) {
		debug(0, "Can't remap actor %d, costume %d doesn't contain an AKPL block", _number, _costume);
		return;
	}

	// Get the number palette entries
	akpl_size = _vm->getResourceDataSize(akpl);

	rgbs = _vm->findResourceData(MKID('RGBS'), akos);

	if (!rgbs) {
		debugC(DEBUG_ACTORS, "Can't remap actor %d costume %d doesn't contain an RGB block", _number, _costume);
		return;
	}

	for (i = 0; i < akpl_size; i++) {
		r = *rgbs++;
		g = *rgbs++;
		b = *rgbs++;

		akpl_color = *akpl++;

		// allow remap of generic palette entry?
		if (!_shadowMode || akpl_color >= 16) {
			r = (r * r_fact) >> 8;
			g = (g * g_fact) >> 8;
			b = (b * b_fact) >> 8;
			_palette[i] = _vm->remapPaletteColor(r, g, b, threshold);
		}
	}
}

void Actor::classChanged(int cls, bool value) {
	if (cls == kObjectClassAlwaysClip)
		_forceClip = value;
	if (cls == kObjectClassIgnoreBoxes)
		_ignoreBoxes = value;
}

bool Actor::isInClass(int cls) {
	return _vm->getClass(_number, cls);
}

bool Actor::isPlayer() {
	if (_vm->_version <= 2)
		return _vm->VAR(42) <= _number && _number <= _vm->VAR(43);
	else
		return isInClass(kObjectClassPlayer);
}

void Actor::setUserCondition(int slot, int set) {
	const int condMaskCode = (_vm->_heversion >= 90) ? 0x1FFF : 0x3FF;
	checkRange(32, 1, slot, "Condition %d out of range");
	if (set == 0) {
		_heCondMask &= ~(1 << (slot + 0xF));
	} else {
		_heCondMask |= 1 << (slot + 0xF);
	}
	if (_heCondMask & condMaskCode) {
		_heCondMask &= ~1;
	} else {
		_heCondMask |= 1;
	}
}

bool Actor::isUserConditionSet(int slot) const {
	checkRange(32, 1, slot, "Condition %d out of range");
	return (_heCondMask & (1 << (slot + 0xF))) != 0;
}

void Actor::setTalkCondition(int slot) {
	const int condMaskCode = (_vm->_heversion >= 90) ? 0x1FFF : 0x3FF;
	checkRange(32, 1, slot, "Condition %d out of range");
	_heCondMask = (_heCondMask & ~condMaskCode) | 1;
	if (slot != 1) {
		_heCondMask |= 1 << (slot - 1);
		if (_heCondMask & condMaskCode) {
			_heCondMask &= ~1;
		} else {
			_heCondMask |= 1;
		}
	}
}

bool Actor::isTalkConditionSet(int slot) const {
	checkRange(32, 1, slot, "Condition %d out of range");
	return (_heCondMask & (1 << (slot - 1))) != 0;
}

#ifndef DISABLE_HE
void ScummEngine_v71he::preProcessAuxQueue() {
	if (!_skipProcessActors) {
		for (int i = 0; i < _auxBlocksNum; ++i) {
			AuxBlock *ab = &_auxBlocks[i];
			if (ab->r.top <= ab->r.bottom) {
				gdi.copyVirtScreenBuffers(ab->r);
			}
		}
	}
	_auxBlocksNum = 0;
}

void ScummEngine_v71he::postProcessAuxQueue() {
	if (!_skipProcessActors) {
		for (int i = 0; i < _auxEntriesNum; ++i) {
			AuxEntry *ae = &_auxEntries[i];
			if (ae->actorNum != -1) {
				Actor *a = derefActor(ae->actorNum, "postProcessAuxQueue");
				const uint8 *cost = getResourceAddress(rtCostume, a->_costume);
				int dy = a->_offsY + a->_pos.y - a->getElevation();
				int dx = a->_offsX + a->_pos.x;

				const uint8 *akax = findResource(MKID('AKAX'), cost);
				assert(akax);
				const uint8 *auxd = findPalInPals(akax, ae->subIndex) - _resourceHeaderSize;
				assert(auxd);
				const uint8 *frel = findResourceData(MKID('FREL'), auxd);
				if (frel) {
					error("unhandled FREL block");
				}
				const uint8 *disp = findResourceData(MKID('DISP'), auxd);
				if (disp) {
					error("unhandled DISP block");
				}
				const uint8 *axfd = findResourceData(MKID('AXFD'), auxd);
				assert(axfd);

				uint16 comp = READ_LE_UINT16(axfd);
				if (comp != 0) {
					int x = (int16)READ_LE_UINT16(axfd + 2) + dx;
					int y = (int16)READ_LE_UINT16(axfd + 4) + dy;
					int w = (int16)READ_LE_UINT16(axfd + 6);
					int h = (int16)READ_LE_UINT16(axfd + 8);
					VirtScreen *pvs = &virtscr[kMainVirtScreen];
					uint8 *dst1 = pvs->getPixels(0, pvs->topline);
					uint8 *dst2 = pvs->getBackPixels(0, pvs->topline);
					switch (comp) {
					case 1:
						Wiz::copyAuxImage(dst1, dst2, axfd + 10, pvs->w, pvs->h, x, y, w, h);
						break;
					default:
						error("unimplemented compression type %d", comp);
					}
				}
				const uint8 *axur = findResourceData(MKID('AXUR'), auxd);
				if (axur) {
					uint16 n = READ_LE_UINT16(axur); axur += 2;
					while (n--) {
						int x1 = (int16)READ_LE_UINT16(axur + 0) + dx;
						int y1 = (int16)READ_LE_UINT16(axur + 2) + dy;
						int x2 = (int16)READ_LE_UINT16(axur + 4) + dx;
						int y2 = (int16)READ_LE_UINT16(axur + 6) + dy;
						markRectAsDirty(kMainVirtScreen, x1, x2, y1, y2 + 1);
						axur += 8;
					}
				}
				const uint8 *axer = findResourceData(MKID('AXER'), auxd);
				if (axer) {
					a->_auxBlock.visible  = true;
					a->_auxBlock.r.left   = (int16)READ_LE_UINT16(axer + 0) + dx;
					a->_auxBlock.r.top    = (int16)READ_LE_UINT16(axer + 2) + dy;
					a->_auxBlock.r.right  = (int16)READ_LE_UINT16(axer + 4) + dx;
					a->_auxBlock.r.bottom = (int16)READ_LE_UINT16(axer + 6) + dy;
				}
			}
		}
	}
	_auxEntriesNum = 0;
}

void ScummEngine_v71he::queueAuxBlock(Actor *a) {
	if (!a->_auxBlock.visible)
		return;

	assert(_auxBlocksNum < ARRAYSIZE(_auxBlocks));
	_auxBlocks[_auxBlocksNum] = a->_auxBlock;
	++_auxBlocksNum;
}

void ScummEngine_v71he::queueAuxEntry(int actorNum, int subIndex) {
	assert(_auxEntriesNum < ARRAYSIZE(_auxEntries));
	AuxEntry *ae = &_auxEntries[_auxEntriesNum];
	ae->actorNum = actorNum;
	ae->subIndex = subIndex;
	++_auxEntriesNum;
}
#endif


void Actor::saveLoadWithSerializer(Serializer *ser) {
	static const SaveLoadEntry actorEntries[] = {
		MKLINE(Actor, _pos.x, sleInt16, VER(8)),
		MKLINE(Actor, _pos.y, sleInt16, VER(8)),
		MKLINE(Actor, _offsX, sleInt16, VER(32)),
		MKLINE(Actor, _offsY, sleInt16, VER(32)),
		MKLINE(Actor, _top, sleInt16, VER(8)),
		MKLINE(Actor, _bottom, sleInt16, VER(8)),
		MKLINE(Actor, _elevation, sleInt16, VER(8)),
		MKLINE(Actor, _width, sleUint16, VER(8)),
		MKLINE(Actor, _facing, sleUint16, VER(8)),
		MKLINE(Actor, _costume, sleUint16, VER(8)),
		MKLINE(Actor, _room, sleByte, VER(8)),
		MKLINE(Actor, _talkColor, sleByte, VER(8)),
		MKLINE(Actor, _talkFrequency, sleInt16, VER(16)),
		MKLINE(Actor, _talkPan, sleInt16, VER(24)),
		MKLINE(Actor, _talkVolume, sleInt16, VER(29)),
		MKLINE(Actor, _boxscale, sleUint16, VER(34)),
		MKLINE(Actor, _scalex, sleByte, VER(8)),
		MKLINE(Actor, _scaley, sleByte, VER(8)),
		MKLINE(Actor, _charset, sleByte, VER(8)),

		// Actor sound grew from 8 to 32 bytes and switched to uint16 in HE games
		MKARRAY_OLD(Actor, _sound[0], sleByte, 8, VER(8), VER(36)),
		MKARRAY_OLD(Actor, _sound[0], sleByte, 32, VER(37), VER(61)),
		MKARRAY(Actor, _sound[0], sleUint16, 32, VER(62)),

		// Actor animVariable grew from 8 to 27
		MKARRAY_OLD(Actor, _animVariable[0], sleUint16, 8, VER(8), VER(40)),
		MKARRAY(Actor, _animVariable[0], sleUint16, 27, VER(41)),

		MKLINE(Actor, _targetFacing, sleUint16, VER(8)),
		MKLINE(Actor, _moving, sleByte, VER(8)),
		MKLINE(Actor, _ignoreBoxes, sleByte, VER(8)),
		MKLINE(Actor, _forceClip, sleByte, VER(8)),
		MKLINE(Actor, _initFrame, sleByte, VER(8)),
		MKLINE(Actor, _walkFrame, sleByte, VER(8)),
		MKLINE(Actor, _standFrame, sleByte, VER(8)),
		MKLINE(Actor, _talkStartFrame, sleByte, VER(8)),
		MKLINE(Actor, _talkStopFrame, sleByte, VER(8)),
		MKLINE(Actor, _speedx, sleUint16, VER(8)),
		MKLINE(Actor, _speedy, sleUint16, VER(8)),
		MKLINE(Actor, _cost.animCounter, sleUint16, VER(8)),
		MKLINE(Actor, _cost.soundCounter, sleByte, VER(8)),
		MKLINE(Actor, _drawToBackBuf, sleByte, VER(32)),
		MKLINE(Actor, _flip, sleByte, VER(32)),
		MKLINE(Actor, _heSkipLimbs, sleByte, VER(32)),

		// Actor palette grew from 64 to 256 bytes
		MKARRAY_OLD(Actor, _palette[0], sleByte, 64, VER(8), VER(9)),
		MKARRAY(Actor, _palette[0], sleByte, 256, VER(10)),

		MK_OBSOLETE(Actor, _mask, sleByte, VER(8), VER(9)),
		MKLINE(Actor, _shadowMode, sleByte, VER(8)),
		MKLINE(Actor, _visible, sleByte, VER(8)),
		MKLINE(Actor, _frame, sleByte, VER(8)),
		MKLINE(Actor, _animSpeed, sleByte, VER(8)),
		MKLINE(Actor, _animProgress, sleByte, VER(8)),
		MKLINE(Actor, _walkbox, sleByte, VER(8)),
		MKLINE(Actor, _needRedraw, sleByte, VER(8)),
		MKLINE(Actor, _needBgReset, sleByte, VER(8)),
		MKLINE(Actor, _costumeNeedsInit, sleByte, VER(8)),
		MKLINE(Actor, _heCondMask, sleUint32, VER(38)),
		MKLINE(Actor, _hePaletteNum, sleUint32, VER(59)),
		MKLINE(Actor, _heXmapNum, sleUint32, VER(59)),

		MKLINE(Actor, _talkPosY, sleInt16, VER(8)),
		MKLINE(Actor, _talkPosX, sleInt16, VER(8)),
		MKLINE(Actor, _ignoreTurns, sleByte, VER(8)),

		// Actor layer switched to int32 in HE games
		MKLINE_OLD(Actor, _layer, sleByte, VER(8), VER(57)),
		MKLINE(Actor, _layer, sleInt32, VER(58)),

		MKLINE(Actor, _talkScript, sleUint16, VER(8)),
		MKLINE(Actor, _walkScript, sleUint16, VER(8)),

		MKLINE(Actor, _walkdata.dest.x, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.dest.y, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.destbox, sleByte, VER(8)),
		MKLINE(Actor, _walkdata.destdir, sleUint16, VER(8)),
		MKLINE(Actor, _walkdata.curbox, sleByte, VER(8)),
		MKLINE(Actor, _walkdata.cur.x, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.cur.y, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.next.x, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.next.y, sleInt16, VER(8)),
		MKLINE(Actor, _walkdata.deltaXFactor, sleInt32, VER(8)),
		MKLINE(Actor, _walkdata.deltaYFactor, sleInt32, VER(8)),
		MKLINE(Actor, _walkdata.xfrac, sleUint16, VER(8)),
		MKLINE(Actor, _walkdata.yfrac, sleUint16, VER(8)),

		MKLINE(Actor, _walkdata.point3.x, sleUint16, VER(42)),
		MKLINE(Actor, _walkdata.point3.y, sleUint16, VER(42)),

		MKARRAY(Actor, _cost.active[0], sleByte, 16, VER(8)),
		MKLINE(Actor, _cost.stopped, sleUint16, VER(8)),
		MKARRAY(Actor, _cost.curpos[0], sleUint16, 16, VER(8)),
		MKARRAY(Actor, _cost.start[0], sleUint16, 16, VER(8)),
		MKARRAY(Actor, _cost.end[0], sleUint16, 16, VER(8)),
		MKARRAY(Actor, _cost.frame[0], sleUint16, 16, VER(8)),
		MKEND()
	};

	if (ser->isLoading()) {
		// Not all actor data is saved; so when loading, we first reset
		// the actor, to ensure completely reproducible behaviour (else,
		// some not saved value in the actor class can cause odd things)
		initActor(-1);
	}

	ser->saveLoadEntries(this, actorEntries);
}

} // End of namespace Scumm
