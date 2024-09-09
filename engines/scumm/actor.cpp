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

#include "common/system.h"	// for setFocusRectangle/clearFocusRectangle
#include "common/scummsys.h"
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/actor_he.h"
#include "scumm/akos.h"
#include "scumm/boxes.h"
#include "scumm/charset.h"
#include "scumm/costume.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm_v7.h"
#include "scumm/scumm_v0.h"
#include "scumm/he/sound_he.h"
#include "scumm/he/sprite_he.h"
#include "scumm/usage_bits.h"
#include "scumm/util.h"

namespace Scumm {

byte Actor::kInvalidBox = 0;

static const byte v0ActorDemoTalk[25] = {
	0x00,
	0x06, // Syd
	0x06, // Razor
	0x06, // Dave
	0x06, // Michael
	0x06, // Bernard
	0x06, // Wendy
	0x00, // Jeff
	0x46, // Radiation Suit
	0x06, // Dr Fred
	0x06, // Nurse Edna
	0x06, // Weird Ed
	0x06, // Dead Cousin Ted
	0xE2, // Purple Tentacle
	0xE2, // Green Tentacle
	0x06, // Meteor police
	0xC0, // Meteor
	0x06, // Mark Eteer
	0x06, // Talkshow Host
	0x00, // Plant
	0xC0, // Meteor Radiation
	0xC0, // Edsel (small, outro)
	0x00, // Meteor (small, intro)
	0x06, // Sandy (Lab)
	0x06, // Sandy (Cut-Scene)
};

static const byte v0ActorTalk[25] = {
	0x00,
	0x06, // Syd
	0x06, // Razor
	0x06, // Dave
	0x06, // Michael
	0x06, // Bernard
	0x06, // Wendy
	0x00, // Jeff
	0x46, // Radiation Suit
	0x06, // Dr Fred
	0x06, // Nurse Edna
	0x06, // Weird Ed
	0x06, // Dead Cousin Ted
	0xFF, // Purple Tentacle
	0xFF, // Green Tentacle
	0x06, // Meteor police
	0xC0, // Meteor
	0x06, // Mark Eteer
	0x06, // Talkshow Host
	0x00, // Plant
	0xC0, // Meteor Radiation
	0xC0, // Edsel (small, outro)
	0x00, // Meteor (small, intro)
	0x06, // Sandy (Lab)
	0x06, // Sandy (Cut-Scene)
};

static const byte v0WalkboxSlantedModifier[0x16] = {
	0x00,0x01,0x02,0x03,0x03,0x04,0x05,0x06,
	0x06,0x07,0x08,0x09,0x09,0x0A,0x0B,
	0x0C,0x0C,0x0D,0x0E,0x0F,0x10,0x10
};

Actor::Actor(ScummEngine *scumm, int id) :
	_vm(scumm), _number(id), _visible(false), _shadowMode(0), _flip(false), _frame(0), _walkbox(0), _talkPosX(0), _talkPosY(0),
	_talkScript(0), _walkScript(0), _ignoreTurns(false), _drawToBackBuf(false), _layer(0), _heOffsX(0), _heOffsY(0), _heSkipLimbs(false),
	_heCondMask(0), _hePaletteNum(0), _heShadow(0), _elevation(0), _facing(0), _targetFacing(0), _speedx(0), _speedy(0),
	_animProgress(0), _animSpeed(0), _costumeNeedsInit(false) {
		assert(_vm != nullptr);
}

ActorHE::ActorHE(ScummEngine *scumm, int id) : Actor(scumm,id) {
	for (int i = 0; i < ARRAYSIZE(_screenUpdateTableMin); i++) {
		_screenUpdateTableMin[i] = 0;
	}

	for (int i = 0; i < ARRAYSIZE(_screenUpdateTableMin); i++) {
		_screenUpdateTableMax[i] = 0;
	}
}

void ActorHE::initActor(int mode) {
	Actor::initActor(mode);

	if (mode == -1) {
		_heOffsX = _heOffsY = 0;
		_heSkipLimbs = false;
		memset(_heTalkQueue, 0, sizeof(_heTalkQueue));
	}

	if (mode == 1) {
		clearActorUpdateInfo();
	}

	if (mode == 1 || mode == -1) {
		_heCondMask = 1;
		_heNoTalkAnimation = 0;
		_heSkipLimbs = false;
	} else if (mode == 2) {
		_heCondMask = 1;
		_heSkipLimbs = false;
	}

	_heShadow = 0;
	_hePaletteNum = 0;
	_generalFlags = 0;
	_heTalking = false;

	if (_vm->_game.heversion >= 61)
		_flip = 0;

	((ScummEngine_v60he *)_vm)->setActorClippingRect(_number, -1, -1, -1, -1);

	_auxActor = 0;
	_auxEraseX1 = 0;
	_auxEraseY1 = 0;
	_auxEraseX2 = -1;
	_auxEraseY2 = -1;
}

void Actor::initActor(int mode) {

	if (mode == -1) {
		_top = _bottom = 0;
		_needRedraw = false;
		_needBgReset = false;
		_costumeNeedsInit = false;
		_visible = false;
		_flip = false;
		_speedx = 8;
		_speedy = 2;
		_frame = 0;
		_walkbox = 0;
		_animProgress = 0;
		_drawToBackBuf = false;
		memset(_animVariable, 0, sizeof(_animVariable));
		memset(_palette, 0, sizeof(_palette));
		memset(_sound, 0, sizeof(_sound));
		_cost.reset();
		_walkdata.reset();
		_walkdata.point3.x = 32000;
		_walkScript = 0;
	}

	if (mode == 1 || mode == -1) {
		_costume = 0;
		_room = 0;
		_pos.x = 0;
		_pos.y = 0;
		_facing = 180;
	} else if (mode == 2) {
		_facing = 180;
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
	_lastValidX = 0;
	_lastValidY = 0;

	_shadowMode = 0;
	_layer = 0;

	stopActorMoving();

	setActorWalkSpeed(8, 2);

	_animSpeed = 0;
	if (_vm->_game.version >= 6)
		_animProgress = 0;

	_ignoreBoxes = false;
	_forceClip = 0;
	_ignoreTurns = false;

	_talkFrequency = 256;
	_talkPan = 64;
	_talkVolume = 127;

	_initFrame = 1;
	_walkFrame = 2;
	_standFrame = 3;
	_talkStartFrame = 4;
	_talkStopFrame = 5;

	_walkScript = 0;
	_talkScript = 0;

	_vm->_classData[_number] = 0;
}

void Actor_v2::initActor(int mode) {
	Actor_v3::initActor(mode);

	_speedx = 1;
	_speedy = 1;

	_initFrame = 2;
	_walkFrame = 0;
	_standFrame = 1;
	_talkStartFrame = 5;
	_talkStopFrame = 4;
}

void Actor_v3::initActor(int mode) {
	if (mode == -1) {
		_stepX = 1;
		_stepThreshold = 0;
	}
	Actor::initActor(mode);
}

void Actor_v7::initActor(int mode) {
	if (mode == 1 || mode == -1)
		_visible = false;

	Actor::initActor(mode);

	_forceClip = 100;
	_vm->_classData[_number] = _vm->_classData[0];
}

void Actor_v0::initActor(int mode) {
	Actor_v2::initActor(mode);

	_costCommandNew = 0xFF;
	_costCommand = 0xFF;
	_miscflags = 0;
	_speaking = 0;

	_walkCountModulo = 0;
	_newWalkBoxEntered = false;
	_walkDirX = 0;
	_walkDirY = 0;
	_walkYCountGreaterThanXCount = 0;
	_walkXCount = 0;
	_walkXCountInc = 0;
	_walkYCount = 0;
	_walkYCountInc = 0;
	_walkMaxXYCountInc = 0;

	_tmp_WalkBox = 0;
	_tmp_NewWalkBoxEntered = 0;

	_animFrameRepeat = 0;
	for (int i = 0; i < 8; ++i) {
		_limbFrameRepeatNew[i] = 0;
		_limbFrameRepeat[i] = 0;
		_limb_flipped[i] = false;
	}

	walkBoxQueueReset();

	if (_vm->_game.features & GF_DEMO) {
		_sound[0] = v0ActorDemoTalk[_number];
	} else {
		_sound[0] = v0ActorTalk[_number];
	}
}

void Actor_v0::walkBoxQueueReset() {
	_walkboxHistory.clear();
	_walkboxQueueIndex = 0;

	for (uint i = 0; i < ARRAYSIZE(_walkboxQueue); ++i) {
		_walkboxQueue[i] = kInvalidBox;
	}
}

bool Actor_v0::walkBoxQueueAdd(int box) {

	if (_walkboxQueueIndex == ARRAYSIZE(_walkboxQueue))
		return false;

	_walkboxQueue[_walkboxQueueIndex++] = box;
	_walkboxHistory.push_back(box);
	return true;
}

void Actor_v0::walkboxQueueReverse() {
	int j = ARRAYSIZE(_walkboxQueue) - 1;

	while (_walkboxQueue[j] == kInvalidBox && j >= 1)
		--j;

	if (j <= 1)
		return;

	for (int i = 1; i < j && j >= 1 ; ++i, --j) {

		byte tmp = _walkboxQueue[i];

		_walkboxQueue[i] = _walkboxQueue[j];
		_walkboxQueue[j] = tmp;
	}
}

bool Actor_v0::walkBoxQueueFind(int box) {

	for (uint i = 0; i < _walkboxHistory.size(); ++i) {
		if (box == _walkboxHistory[i])
			return true;
	}

	return false;
}

bool Actor_v0::walkBoxQueuePrepare() {
	walkBoxQueueReset();
	int BoxFound = _walkbox;

	if (BoxFound == _walkdata.destbox) {

		_newWalkBoxEntered = true;
		return true;
	}

	// Build a series of walkboxes from our current position, to the target
	do {
		// Add the current box to the queue
		if (!walkBoxQueueAdd(BoxFound))
			return false;

		// Loop until we find a walkbox which hasn't been tested
		while (_walkboxQueueIndex > 0) {

			// Check if the dest box is a direct neighbour
			if ((BoxFound = _vm->getNextBox(BoxFound, _walkdata.destbox)) == kInvalidBox) {

				// Its not, start hunting through this boxes immediate connections
				byte* boxm = _vm->getBoxConnectionBase(_walkboxQueue[_walkboxQueueIndex - 1]);

				// Attempt to find one, which we havn't already used
				for (; *boxm != kInvalidBox; ++boxm) {
					if (walkBoxQueueFind(*boxm) != true)
						break;
				}

				BoxFound = *boxm;
			}

			// Found one?
			if (BoxFound != kInvalidBox) {

				// Did we find a connection to the final walkbox
				if (BoxFound == _walkdata.destbox) {

					_newWalkBoxEntered = true;

					walkBoxQueueAdd(BoxFound);

					walkboxQueueReverse();
					return true;
				}

				// Nope, check the next box
				break;
			}

			// Drop this box, its useless to us
			_walkboxQueue[--_walkboxQueueIndex] = kInvalidBox;

			BoxFound = _walkboxQueue[_walkboxQueueIndex - 1];
		}

	} while (_walkboxQueueIndex > 0);

	return false;
}

void Actor::setBox(int box) {
	_walkbox = box;
	setupActorScale();
}

void Actor_v3::setupActorScale() {
	// WORKAROUND bug #2556: Under certain circumstances, it is possible
	// for Henry Sr. to reach the front side of Castle Brunwald (following
	// Indy there). But it seems the game has no small costume for Henry,
	// hence he is shown as a giant, triple in size compared to Indy.
	// To workaround this, we override the scale of Henry. Since V3 games
	// like Indy3 don't use the costume scale otherwise, this works fine.
	// The scale factor 0x50 was determined by some guess work.
	//
	// TODO: I can't reproduce this with the EGA DOS, EGA Macintosh and
	// VGA DOS English releases, since Indy says he'd "better not" go back
	// to the front of the castle at this point (script 77-201), as long
	// as a special Bit is set for this (and it's set in room 21 entry
	// script when Henry escapes from his room). Maybe there's a problem
	// in the German release (and then it'd probably be better to restore
	// that safeguard instead, since the game clearly doesn't expect you
	// to go back inside the castle), but so far I haven't been able to
	// replicate the issue with my German copy. I will dig deeper, but
	// I think that fingolfin was right when he said that this looks more
	// like a bug in the game data, back in 2011.  -dwa
	if (_number == 2 && _costume == 7 && _vm->_game.id == GID_INDY3 && _vm->_currentRoom == 12 && _vm->enhancementEnabled(kEnhMinorBugFixes)) {
		_scalex = 0x50;
		_scaley = 0x50;
	} else {
		// TODO: The following could probably be removed
		_scalex = 0xFF;
		_scaley = 0xFF;
	}
}

void Actor::setupActorScale() {
	if (_ignoreBoxes)
		return;

	// For some boxes, we ignore the scaling and use whatever values the
	// scripts set. This is used e.g. in the Mystery Vortex in Sam&Max.
	// Older games used the flag 0x20 differently, though.
	if (_vm->_game.id == GID_SAMNMAX && (_vm->getBoxFlags(_walkbox) & kBoxIgnoreScale))
		return;

	_boxscale = _vm->getBoxScale(_walkbox);

	uint16 scale = _vm->getScale(_walkbox, _pos.x, _pos.y);
	assert(scale <= 0xFF);

	_scalex = _scaley = (byte)scale;
}


#pragma mark -
#pragma mark --- Actor walking ---
#pragma mark -


void ScummEngine::walkActors() {
	for (int i = 1; i < _numActors; ++i) {
		if (_actors[i]->isInCurrentRoom())
			_actors[i]->walkActor();
	}
}

void Actor::stopActorMoving() {
	if (_walkScript)
		_vm->stopScript(_walkScript);

	if (_vm->_game.version == 0) {
		_moving = 2;
		setDirection(_facing);
	} else {
		_moving = 0;
	}
}

void Actor::setActorWalkSpeed(uint newSpeedX, uint newSpeedY) {
	if (newSpeedX == _speedx && newSpeedY == _speedy)
		return;

	_speedx = newSpeedX;
	_speedy = newSpeedY;

	if (_moving) {
		if (_vm->_game.version == 8 && (_moving & MF_IN_LEG) == 0)
			return;
		calcMovementFactor(_walkdata.next);
	}
}

int getAngleFromPos(int x, int y) {
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

int Actor::calcMovementFactor(const Common::Point& next) {
	int diffX, diffY;
	int32 deltaXFactor, deltaYFactor;

	if (_pos == next)
		return 0;

	diffX = next.x - _pos.x;
	diffY = next.y - _pos.y;

	deltaYFactor = _speedy << 16;
	if (diffY < 0)
		deltaYFactor = -deltaYFactor;

	deltaXFactor = deltaYFactor * diffX;
	if (diffY != 0) {
		deltaXFactor /= diffY;
	} else {
		deltaYFactor = 0;
	}

	// We used to have ABS(deltaXFactor >> 16) for the calculation here, which
	// caused bug no. https://bugs.scummvm.org/ticket/14582
	// For SCUMM4-6 it is obvious from disam that they do the division by 0x10000.
	// SCUMM7/8 original code gives the impression of using deltaXFactor >> 16 at
	// first glance, but it really doesn't. It is a more complicated operation
	// which amounts to the exact same thing as the following...
	if ((uint)ABS(deltaXFactor / 0x10000) > _speedx) {
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

	_walkdata.xfrac = 0;
	_walkdata.yfrac = 0;
	_walkdata.cur = _pos;
	_walkdata.next = next;
	_walkdata.deltaXFactor = deltaXFactor;
	_walkdata.deltaYFactor = deltaYFactor;

	if (_vm->_game.version >= 7) {
		_walkdata.facing = ((int)(atan2((double)deltaXFactor, (double)-deltaYFactor) * 180 / M_PI) + 360) % 360;
		startWalkAnim((_moving & MF_IN_LEG) ? 2 : 1, _walkdata.facing);
		_moving |= MF_IN_LEG;
	} else {
		_targetFacing = (ABS(diffY) * 3 > ABS(diffX)) ? (deltaYFactor > 0 ? 180 : 0) : (deltaXFactor > 0 ? 90 : 270);
	}

	return actorWalkStep();
}

int Actor_v3::calcMovementFactor(const Common::Point& next) {
	int32 deltaXFactor, deltaYFactor;

	if (_pos == next) {
		if (_vm->_game.version == 2)
			_moving |= MF_IN_LEG;
		return 0;
	}

	int diffX = next.x - _pos.x;
	int diffY = next.y - _pos.y;

	if (_vm->_game.version == 3) {
		// These two lines fix bug #1052 (INDY3: Hitler facing wrong directions in the Berlin scene).
		// I can't see anything like this in the original SCUMM1/2 code, so I limit this to SCUMM3.
		if (!(_moving & MF_LAST_LEG) && (int)_speedx > ABS(diffX) && (int)_speedy > ABS(diffY))
			return 0;

		_stepX = ((ABS(diffY) / (int)_speedy) >> 1) > (ABS(diffX) / (int)_speedx) ? _speedy + 1 : _speedx;
	}

	_stepThreshold = MAX(ABS(diffY) / _speedy, ABS(diffX) / _stepX);
	deltaXFactor = (int32)_stepX;
	deltaYFactor = (int32)_speedy;

	if (diffX < 0)
		deltaXFactor = -deltaXFactor;
	if (diffY < 0)
		deltaYFactor = -deltaYFactor;

	_walkdata.xfrac = _walkdata.xAdd = deltaXFactor ? diffX / deltaXFactor : 0;
	_walkdata.yfrac = _walkdata.yAdd = deltaYFactor ? diffY / deltaYFactor : 0;
	_walkdata.cur = _pos;
	_walkdata.next = next;
	_walkdata.deltaXFactor = deltaXFactor;
	_walkdata.deltaYFactor = deltaYFactor;
	_walkdata.facing = diffX >= 0 ? (diffY >= 0 ? 1 : 0) : (diffY >= 0 ? 2 : 3);

	// The x/y distance ratio which determines whether to face up/down instead of left/right is different for SCUMM1/2 and SCUMM3.
	_targetFacing = oldDirToNewDir(((ABS(diffY) * _facingXYratio) > ABS(diffX)) ? 3 - (diffY >= 0 ? 1 : 0) : (diffX >= 0 ? 1 : 0));

	if (_vm->_game.version > 2)
		return actorWalkStep();

	_moving &= ~MF_IN_LEG;
	if (_facing != _targetFacing)
		_moving |= MF_TURN;


	if (_walkFrame != _frame || _facing != _targetFacing)
		startWalkAnim(1, _facing);

	return (_moving & MF_TURN) ? 0 : actorWalkStep();
}

int Actor::actorWalkStep() {
	_needRedraw = true;

	if (_vm->_game.heversion >= 70) {
		_needBgReset = true;
	}

	if (_vm->_game.version < 7) {
		int nextFacing = updateActorDirection(true);
		if ((_walkFrame != _frame && !(_moving & MF_IN_LEG)) || _facing != nextFacing)
			startWalkAnim(1, nextFacing);
		_moving |= MF_IN_LEG;
	}

	if (_walkbox != _walkdata.curbox && _vm->checkXYInBoxBounds(_walkdata.curbox, _pos.x, _pos.y))
		setBox(_walkdata.curbox);

	int distX = ABS(_walkdata.next.x - _walkdata.cur.x);
	int distY = ABS(_walkdata.next.y - _walkdata.cur.y);

	if (ABS(_pos.x - _walkdata.cur.x) >= distX && ABS(_pos.y - _walkdata.cur.y) >= distY) {
		// I have checked that only the v7/8 games have this different (non-)handling of the moving flag. Our code was
		// correct for the lower versions. For COMI this fixes one part of the issues that caused ticket #4424 (wrong
		// movement data being reported by ScummEngine_v8::o8_wait()).
		if (_vm->_game.version < 7)
			_moving &= ~MF_IN_LEG;
		return 0;
	}

	int tmpX = (_pos.x << 16) + _walkdata.xfrac + (_walkdata.deltaXFactor >> 8) * _scalex;
	_walkdata.xfrac = (uint16)tmpX;
	_pos.x = (tmpX >> 16);

	int tmpY = (_pos.y << 16) + _walkdata.yfrac + (_walkdata.deltaYFactor >> 8) * _scaley;
	_walkdata.yfrac = (uint16)tmpY;
	_pos.y = (tmpY >> 16);

	if (ABS(_pos.x - _walkdata.cur.x) > distX)
		_pos.x = _walkdata.next.x;

	if (ABS(_pos.y - _walkdata.cur.y) > distY)
		_pos.y = _walkdata.next.y;

	if (_vm->_game.version >= 4 && _vm->_game.version <= 6 && _pos == _walkdata.next) {
		_moving &= ~MF_IN_LEG;
		return 0;
	}

	return 1;
}

int Actor_v2::actorWalkStep() {
	_needRedraw = true;

	if ((_walkdata.xfrac += _walkdata.xAdd) >= _stepThreshold) {
		if (_pos.x != _walkdata.next.x)
			_pos.x += _walkdata.deltaXFactor;
		_walkdata.xfrac -= _stepThreshold;
	}
	if ((_walkdata.yfrac += _walkdata.yAdd) >= _stepThreshold) {
		if (_pos.y != _walkdata.next.y)
			_pos.y += _walkdata.deltaYFactor;
		_walkdata.yfrac -= _stepThreshold;
	}

	if (_pos == _walkdata.next)
		_moving |= MF_IN_LEG;

	return 0;
}

int Actor_v3::actorWalkStep() {
	_needRedraw = true;

	int nextFacing = updateActorDirection(true);
	if (!(_moving & MF_IN_LEG) || _facing != nextFacing) {
		if (_walkFrame != _frame || _facing != nextFacing)
			startWalkAnim(1, nextFacing);

		_moving |= MF_IN_LEG;
		// The next line fixes bug #12278 for ZAK FM-TOWNS (SCUMM3).
		return 1;
	}

	if (_walkdata.next.x - (int)_stepX <= _pos.x && _walkdata.next.x + (int)_stepX >= _pos.x)
		_pos.x = _walkdata.next.x;
	if (_walkdata.next.y - (int)_speedy <= _pos.y && _walkdata.next.y + (int)_speedy >= _pos.y)
		_pos.y = _walkdata.next.y;

	if (_walkbox != _walkdata.curbox && _vm->checkXYInBoxBounds(_walkdata.curbox, _pos.x, _pos.y))
		setBox(_walkdata.curbox);

	if (_pos == _walkdata.next) {
		_moving &= ~MF_IN_LEG;
		return 0;
	}

	if ((_walkdata.xfrac += _walkdata.xAdd) >= _stepThreshold) {
		if (_pos.x != _walkdata.next.x)
			_pos.x += _walkdata.deltaXFactor;
		_walkdata.xfrac -= _stepThreshold;
	}
	if ((_walkdata.yfrac += _walkdata.yAdd) >= _stepThreshold) {
		if (_pos.y != _walkdata.next.y)
			_pos.y += _walkdata.deltaYFactor;
		_walkdata.yfrac -= _stepThreshold;
	}

	return 1;
}

bool Actor_v0::calcWalkDistances() {
	_walkDirX = 0;
	_walkDirY = 0;
	_walkYCountGreaterThanXCount = 0;
	uint16 A = 0;

	if (_CurrentWalkTo.x >= _tmp_NewPos.x) {
		A = _CurrentWalkTo.x - _tmp_NewPos.x;
		_walkDirX = 1;
	} else {
		A = _tmp_NewPos.x - _CurrentWalkTo.x;
	}

	_walkXCountInc = A;

	if (_CurrentWalkTo.y >= _tmp_NewPos.y) {
		A = _CurrentWalkTo.y - _tmp_NewPos.y;
		_walkDirY = 1;
	} else {
		A = _tmp_NewPos.y - _CurrentWalkTo.y;
	}

	_walkYCountInc = A;
	if (!_walkXCountInc && !_walkYCountInc)
		return true;

	if (_walkXCountInc <= _walkYCountInc)
		_walkYCountGreaterThanXCount = 1;

	// 2FCC
	A = _walkXCountInc;
	if (A <= _walkYCountInc)
		A = _walkYCountInc;

	_walkMaxXYCountInc = A;
	_walkXCount = _walkXCountInc;
	_walkYCount = _walkYCountInc;
	_walkCountModulo = _walkMaxXYCountInc;

	return false;
}

/* Calculate the result of moving X+1 or X-1 */
byte Actor_v0::actorWalkXCalculate() {
	byte A = _walkXCount;
	A += _walkXCountInc;
	if (A >= _walkCountModulo) {
		if (!_walkDirX) {
			_tmp_NewPos.x--;
		} else {
			_tmp_NewPos.x++;
		}

		A -= _walkCountModulo;
	}
	// 2EAC
	_walkXCount = A;
	setActorToTempPosition();
	if (updateWalkbox() == kInvalidBox) {
		// 2EB9
		setActorToOriginalPosition();

		return 3;
	}
	// 2EBF
	if (_tmp_NewPos.x == _CurrentWalkTo.x)
		return 1;

	return 0;
}

/* Calculate the result of moving Y+1 or Y-1 */
byte Actor_v0::actorWalkYCalculate() {
	byte A = _walkYCount;
	A += _walkYCountInc;
	if (A >= _walkCountModulo) {
		if (!_walkDirY) {
			_tmp_NewPos.y--;
		} else {
			_tmp_NewPos.y++;
		}

		A -= _walkCountModulo;
	}
	// 2EEB
	_walkYCount = A;
	setActorToTempPosition();
	if (updateWalkbox() == kInvalidBox) {
		// 2EF8
		setActorToOriginalPosition();
		return 4;
	}
	// 2EFE
	if (_walkYCountInc != 0) {
		if (_walkYCountInc == 0xFF) {
			setActorToOriginalPosition();
			return 4;
		}
	}
	// 2F0D
	if (_CurrentWalkTo.y == _tmp_NewPos.y)
		return 1;

	return 0;
}

void Actor::startWalkActor(int destX, int destY, int dir) {
	AdjustBoxResult abr;

	if (!isInCurrentRoom() && _vm->_game.version >= 7) {
		debugC(DEBUG_ACTORS, "startWalkActor: attempting to walk actor %d who is not in this room", _number);
		return;
	}

	if (_vm->_game.version <= 4) {
		abr.x = destX;
		abr.y = destY;
		abr.box = kInvalidBox;
	} else {
		abr = adjustXYToBeInBox(destX, destY);
	}

	if (!isInCurrentRoom() && _vm->_game.version <= 6) {
		_pos.x = abr.x;
		_pos.y = abr.y;
		if (!_ignoreTurns && dir != -1)
			_facing = dir;
		return;
	}

	if (_vm->_game.version <= 2) {
		abr = adjustXYToBeInBox(abr.x, abr.y);
		if (_pos.x == abr.x && _pos.y == abr.y && (dir == -1 || _facing == dir))
			return;
	} else {
		if (_ignoreBoxes) {
			abr.box = kInvalidBox;
			_walkbox = kInvalidBox;
		} else {
			if (_vm->_game.version < 7) {
				if (_vm->checkXYInBoxBounds(_walkdata.destbox, abr.x, abr.y)) {
					abr.box = _walkdata.destbox;
				} else {
					abr = adjustXYToBeInBox(abr.x, abr.y);
				}
			}
			if (_moving && _walkdata.destdir == dir && _walkdata.dest.x == abr.x && _walkdata.dest.y == abr.y)
				return;
		}

		if (_pos.x == abr.x && _pos.y == abr.y) {
			if (dir != _facing)
				turnToDirection(dir);
			return;
		}
	}

	_walkdata.dest.x = abr.x;
	_walkdata.dest.y = abr.y;
	_walkdata.destbox = abr.box;
	_walkdata.destdir = dir;
	_walkdata.point3.x = 32000;
	_walkdata.curbox = _walkbox;

	if (_vm->_game.version == 0) {
		((Actor_v0 *)this)->walkBoxQueuePrepare();

	} else if (_vm->_game.version <= 2) {
		_moving = (_moving & ~MF_LAST_LEG) | MF_IN_LEG | MF_NEW_LEG;
	} else {
		_moving = (_moving & MF_IN_LEG) | MF_NEW_LEG;
	}

}

void Actor::startWalkAnim(int cmd, int angle) {
	if (_vm->_game.version >= 7)
		angle = remapDirection(normalizeAngle(_vm->_costumeLoader->hasManyDirections(_costume), angle == -1 ? _walkdata.facing : angle), false);
	else if (angle == -1)
		angle = _facing;

	if (_walkScript) {
		int args[NUM_SCRIPT_LOCAL];
		memset(args, 0, sizeof(args));
		args[0] = _number;
		args[1] = cmd;
		args[2] = angle;
		_vm->runScript(_walkScript, 1, 0, args);
	} else {
		if (_vm->_game.version >= 7 || cmd == 3)
			turnToDirection(angle);
		else
			setDirection(angle);

		if (cmd == 1)
			startAnimActor(_walkFrame);  /* start walk */
		else if (cmd == 3)
			startAnimActor(_standFrame); /* stop walk */
	}
}

void Actor::walkActor() {
	int new_dir, next_box;
	Common::Point foundPath;

	if (!_moving)
		return;

	if (!(_moving & MF_NEW_LEG)) {
		if (_moving & MF_IN_LEG && actorWalkStep())
			return;

		if (_moving & MF_LAST_LEG) {
			_moving = 0;
			setBox(_walkdata.destbox);
			if (_vm->_game.version <= 6) {
				startAnimActor(_standFrame);
				if (_targetFacing != _walkdata.destdir)
					turnToDirection(_walkdata.destdir);
			} else {
				startWalkAnim(3, _walkdata.destdir);
			}
			return;
		}

		if (_moving & MF_TURN) {
			if (_vm->_game.version <= 6) {
				new_dir = updateActorDirection(false);
				if (_facing != new_dir)
					setDirection(new_dir);
				else
					_moving = 0;
			}
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

		next_box = _vm->getNextBox(_walkbox, _walkdata.destbox);
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

void Actor_v0::walkActor() {
	actorSetWalkTo();

	_needRedraw = true;
	if (_NewWalkTo != _CurrentWalkTo) {
		_CurrentWalkTo = _NewWalkTo;

UpdateActorDirection:;
		_tmp_NewPos = _pos;

		byte tmp = calcWalkDistances();
		_moving &= 0xF0;
		_moving |= tmp;

		if (!_walkYCountGreaterThanXCount) {
			if (_walkDirX) {
				_targetFacing = getAngleFromPos(V12_X_MULTIPLIER*1, V12_Y_MULTIPLIER*0);
			} else {
				_targetFacing = getAngleFromPos(V12_X_MULTIPLIER*-1, V12_Y_MULTIPLIER*0);
			}
		} else {
			if (_walkDirY) {
				_targetFacing = getAngleFromPos(V12_X_MULTIPLIER*0, V12_Y_MULTIPLIER*1);
			} else {
				_targetFacing = getAngleFromPos(V12_X_MULTIPLIER*0, V12_Y_MULTIPLIER*-1);
			}
		}

		directionUpdate();

		// Need to turn again?
		if (_moving & 0x80)
			return;

		animateActor(newDirToOldDir(_facing));

	} else {
		// 2A0A
		if ((_moving & 0x7F) != 1) {

			if (_NewWalkTo == _pos)
				return;
		}
	}

	// 2A9A: Nothing to do
	if (_moving == 2)
		return;

	// Reached Target
	if ((_moving & 0x0F) == 1)
		return stopActorMoving();

	// 2AAD: Turn actor?
	if (_moving & 0x80) {
		directionUpdate();

		// Turn again?
		if (_moving & 0x80)
			return;

		// Start Walk animation
		animateActor(newDirToOldDir(_facing));
	}

	// Walk X
	if ((_moving & 0x0F) == 3) {
	WalkX:;
		setActorToTempPosition();

		if (!_walkDirX) {
			_pos.x--;
		} else {
			_pos.x++;
		}

		// 2C51
		// Does this move us into the walkbox?
		if (updateWalkbox() != kInvalidBox) {

			// Yes, Lets update our direction
			setActorToOriginalPosition();
			goto UpdateActorDirection;
		}

		setActorToOriginalPosition();

		// Have we reached Y Target?
		if (_CurrentWalkTo.y == _tmp_NewPos.y) {
			stopActorMoving();
			return;
		}

		// Lets check one more pixel up or down
		if (!_walkDirY) {
			_tmp_NewPos.y--;
		} else {
			_tmp_NewPos.y++;
		}

		setActorToTempPosition();

		// Are we still inside an invalid walkbox?
		if (updateWalkbox() == kInvalidBox) {
			setActorToOriginalPosition();
			stopActorMoving();
			return;
		}

		// Found a valid walkbox
		return;
	}

	// 2ADA: Walk Y
	if ((_moving & 0x0F) == 4) {
		setActorToTempPosition();

		if (!_walkDirY) {
			_pos.y--;
		} else {
			_pos.y++;
		}

		// Moved out of walkbox?
		if (updateWalkbox() == kInvalidBox) {
			// 2CC7
			setActorToOriginalPosition();

			// Reached X?
			if (_CurrentWalkTo.x == _tmp_NewPos.x) {
				stopActorMoving();
				return;
			}

			// Lets check one more pixel to left or right
			if (!_walkDirX) {
				_tmp_NewPos.x--;
			} else {
				_tmp_NewPos.x++;
			}

			setActorToTempPosition();

			// Still in an invalid walkbox?
			if (updateWalkbox() == kInvalidBox) {

				setActorToOriginalPosition();
				stopActorMoving();
			}

			return;
		} else {

			// Lets update our direction
			setActorToOriginalPosition();
			goto UpdateActorDirection;
		}
	}

	if ((_moving & 0x0F) == 0) {
		// 2AE8
		byte A = actorWalkXCalculate();

		// Will X movement reach destination
		if (A == 1) {

			A = actorWalkYCalculate();

			// Will Y movement also reach destination?
			if (A == 1) {
				_moving &= 0xF0;
				_moving |= A;
			} else {
				if (A == 4)
					stopActorMoving();
			}

			return;

		} else {
			// 2B0C: Moving X will put us in an invalid walkbox
			if (A == 3) {
				_moving &= 0xF0;
				_moving |= A;

				if (_walkDirY) {
					_targetFacing = getAngleFromPos(V12_X_MULTIPLIER*0, V12_Y_MULTIPLIER*1);
				} else {
					_targetFacing = getAngleFromPos(V12_X_MULTIPLIER*0, V12_Y_MULTIPLIER*-1);
				}

				directionUpdate();
				animateActor(newDirToOldDir(_facing));

				// FIXME: During the hands-free-demo in the library (room 5), Purple Tentacle gets stuck following Sandy due to the corner of the stairs,
				//        This is due to distance, and walkbox gap/layout. This works fine with the original engine, because it 'brute forces'
				//        another pixel move in the walk direction before giving up, allowing us to move enough pixels to hit the next walkbox.
				//        Why this fails with the return is because script-10 is executing a 'walkActorToActor' every cycle, which restarts the movement process
				//        As a work around, we implement the original engine behaviour only for Purple Tentacle in the Demo. Doing this for other actors
				//        causes a skipping effect while transitioning walkboxes (the original has another bug in this situation, in which the actor just changes direction for 1 frame during this moment)
				if ((_vm->_game.features & GF_DEMO) && _number == 13)
					goto WalkX;

				return;

			} else {
				// 2B39: Moving X was ok, do we also move Y
				A = actorWalkYCalculate();

				// Are we in a valid walkbox?
				if (A != 4)
					return;

				// No, we need to change direction
				_moving &= 0xF0;
				_moving |= A;

				if (_walkDirX) {
					_targetFacing = getAngleFromPos(V12_X_MULTIPLIER*1, V12_Y_MULTIPLIER*0);
				} else {
					_targetFacing = getAngleFromPos(V12_X_MULTIPLIER*-1, V12_Y_MULTIPLIER*0);
				}

				directionUpdate();
				animateActor(newDirToOldDir(_facing));

				return;
			}
		}
	}
}

void Actor_v2::walkActor() {
	if (_moving & MF_TURN) {
		int newDir = updateActorDirection(false);
		if (_targetFacing == newDir)
			_moving &= ~MF_TURN;
		setDirection(newDir);
		return;
	}

	if (!(_moving & MF_NEW_LEG))
		return;

	if (!(_moving & MF_IN_LEG)) {
		actorWalkStep();
	} else {
		if (_moving & MF_LAST_LEG) {
			_moving = MF_TURN;
			startAnimActor(_standFrame);
			if (_walkdata.destdir != -1)
				turnToDirection(_walkdata.destdir);
		} else {
			Common::Point foundPath, tmp;
			setBox(_walkdata.curbox);
			if (_walkbox == _walkdata.destbox) {
				foundPath = _walkdata.dest;
				_moving |= MF_LAST_LEG;
			} else {
				int nextBox = _vm->getNextBox(_walkbox, _walkdata.destbox);
				if (nextBox < 0) {
					_moving |= MF_LAST_LEG;
					return;
				}

				// Can't walk through locked boxes
				int flags = _vm->getBoxFlags(nextBox);
				if ((flags & kBoxLocked) && !((flags & kBoxPlayerOnly) && !isPlayer())) {
					_moving |= MF_LAST_LEG;
					_walkdata.destdir = -1;
				}

				_walkdata.curbox = nextBox;

				getClosestPtOnBox(_vm->getBoxCoordinates(_walkdata.curbox), _pos.x, _pos.y, tmp.x, tmp.y);
				getClosestPtOnBox(_vm->getBoxCoordinates(_walkbox), tmp.x, tmp.y, foundPath.x, foundPath.y);
			}
			calcMovementFactor(foundPath);
		}
	}
}

void Actor_v3::walkActor() {
	Common::Point p2, p3;	// Gate locations
	int new_dir, next_box;

	if (!_moving)
		return;

	if (!(_moving & MF_NEW_LEG)) {
		if (_moving & MF_IN_LEG && actorWalkStep())
			return;

		if (_moving & MF_LAST_LEG) {
			_moving = 0;
			startAnimActor(_standFrame);
			if (_targetFacing != _walkdata.destdir)
				turnToDirection(_walkdata.destdir);
			return;
		}

		if (_moving & MF_TURN) {
			new_dir = updateActorDirection(false);
			if (_facing != new_dir) {
				setDirection(new_dir);
			} else {
				// WORKAROUND for bug #4594 ("SCUMM: Zak McKracken - Zak keeps walk animation without moving")
				// This bug also happens with the original SCUMM3 (ZAK FM-TOWNS) interpreter (unlike SCUMM1/2
				// where the actors are apparently supposed to continue walking after being turned). We have
				// to stop the walking animation here...
				// This also fixes bug #4601 ("SCUMM: Zak McKracken (FM-Towns) - shopkeeper keeps walking"),
				// although that one does not happen with the original interpreter.
				if (_vm->_game.id == GID_ZAK && _moving == MF_TURN)
					startAnimActor(_standFrame);
				_moving = 0;
			}
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

		next_box = _vm->getNextBox(_walkbox, _walkdata.destbox);
		if (next_box < 0) {
			_moving |= MF_LAST_LEG;
			return;
		}

		// This is version specific for ZAK FM-TOWNS. The flags check that is present in later SCUMM versions does not exist
		// in SCUMM3. I have looked at disams of ZAK FM-TOWNS, LOOM FM-TOWNS, LOOM DOS EGA, INDY3 FM-TOWNS, INDY3 DOS VGA.
		if (_vm->_game.id == GID_ZAK) {
			// Check for equals, not for a bit mask (otherwise: bug no. 13399)
			if (_vm->getBoxFlags(next_box) == kBoxLocked) {
				_moving |= MF_LAST_LEG;
				return;
			}
		}

		_walkdata.curbox = next_box;

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
		if (calcMovementFactor(p3))
			return;

		setBox(_walkdata.curbox);
	} while (1);

	_moving |= MF_LAST_LEG;
	calcMovementFactor(_walkdata.dest);
}

void Actor_v7::walkActor() {
	if (!(_moving & MF_FROZEN))
		Actor::walkActor();

	if (_moving & MF_TURN) {
		int newDir = updateActorDirection();
		if (_facing != newDir)
			setDirection(newDir);
		else
			_moving &= ~MF_TURN;
	}
}

#pragma mark -
#pragma mark --- Actor direction ---
#pragma mark -


int Actor::remapDirection(int dir, bool is_walking) {
	int specdir;
	byte flags;
	byte mask;
	bool flipX;
	bool flipY;

	if ((_vm->_game.version < 5 || !_ignoreBoxes) && (_vm->_game.version < 7 || isInCurrentRoom())) {
		if (_walkbox != kOldInvalidBox) {
			assert(_walkbox < ARRAYSIZE(_vm->_extraBoxFlags));
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
			if (_vm->_game.version >= 7) {
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
			if (_vm->_game.version >= 7) {
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
		default:
			break;
		}

		// MM v0 stores flags as a part of the mask
		if (_vm->_game.version == 0) {
			mask = _vm->getMaskFromBox(_walkbox);
			// face the wall if climbing/descending a ladder
			if ((mask & 0x8C) == 0x84)
				return 0;
		}
	}

	dir = (dir + 360) % 360;

	// OR 0x400 to signal direction interpolation should be done
	if (_vm->_game.version < 7)
		dir |= 0x400;

	return dir;
}

int Actor_v2::remapDirection(int dir, bool is_walking) {
	if (_vm->_game.version == 0)
		return Actor::remapDirection(dir, is_walking);

	static const byte remapTable1[] = {
		0x04, 0x01, 0x02, 0x00, 0x01, 0x02, 0x03, 0x00,
		0x06, 0x01, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00,
		0x07, 0x00, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00,
		0x05, 0x00, 0x02, 0x00, 0x01, 0x02, 0x03, 0x00
	};

	static const byte remapTable2[] = {
		0x00, 0x01, 0x03, 0x02, 0x03, 0x00, 0x02, 0x00,
		0x00, 0x01, 0x03, 0x02, 0x01, 0x03, 0x01, 0x02,
		0x00, 0x01, 0x03, 0x02, 0x01, 0x00, 0x02, 0x02,
		0x00, 0x01, 0x03, 0x02, 0x03, 0x03, 0x01, 0x01
	};

	static const byte remapTable3[] = {
		0x00, 0x00, 0x02, 0x00, 0x01, 0x03, 0x02, 0x00,
		0x01, 0x01, 0x02, 0x00, 0x01, 0x03, 0x02, 0x00,
		0x02, 0x01, 0x02, 0x00, 0x01, 0x03, 0x02, 0x00,
		0x03, 0x00, 0x03, 0x00, 0x01, 0x03, 0x02, 0x00
	};

	if (_moving & ~MF_TURN)
		_targetFacing = oldDirToNewDir(remapTable2[newDirToOldDir(dir) * 8 + remapTable1[_walkdata.facing * 8 + (_vm->getBoxFlags(_walkbox) & 7)]]);
	else 
		_targetFacing = oldDirToNewDir(remapTable3[newDirToOldDir(dir) * 8 + (_vm->getBoxFlags(_walkbox) & 7)]);

	return _targetFacing | 0x400;
}

int Actor::updateActorDirection(bool is_walking) {
	static const uint8 actorTurnInterpolateTable[] = { 0, 2, 2, 3, 2, 1, 2, 3, 0, 1, 2, 1, 0, 1, 0, 3 };

	if ((_vm->_game.version == 6) && _ignoreTurns)
		return _facing;

	int dir = remapDirection(_targetFacing, is_walking);
	if (dir & 0x400)
		dir = oldDirToNewDir(actorTurnInterpolateTable[newDirToOldDir(dir & 0x3ff) | (newDirToOldDir(_facing) << 2)]);

	return dir;
}

int Actor_v7::updateActorDirection() {
	int dirType = _vm->_costumeLoader->hasManyDirections(_costume);
	int from = toSimpleDir(dirType, _facing);
	int to = toSimpleDir(dirType, _targetFacing);
	int num = dirType ? 8 : 4;

	// Turn left or right, depending on which is shorter.
	int diff = to - from;
	if (ABS(diff) > (num >> 1))
		diff = -diff;

	if (diff > 0) {
		to = from + 1;
	} else if (diff < 0) {
		to = from - 1;
	}

	return fromSimpleDir(dirType, (to + num) % num);
}

void Actor::setDirection(int direction) {
	uint aMask;
	int i;
	uint16 vald;

	direction = (direction + 360) % 360;

	// Do nothing if actor is already facing in the given direction
	if (_facing == direction)
		return;

	_facing = direction;

	// If there is no costume set for this actor, we are finished
	if (_costume == 0)
		return;

	// Verified for v3-v6 and HE
	if (!isInCurrentRoom() && _vm->_game.version >= 3 && _vm->_game.version <= 6)
		return;

	// Update the costume for the new direction (and mark the actor for redraw)
	aMask = 0x8000;
	for (i = 0; i < 16; i++, aMask >>= 1) {
		vald = _cost.frame[i];
		if (vald == 0xFFFF)
			continue;
		if (!(_vm->_game.features & GF_NEW_COSTUMES)) {
			// Fix bug mentioned here: https://github.com/scummvm/scummvm/pull/3795/
			// For versions 1 to 6 we need to store the direction info in the frame array (like
			// the original interpreters do). I haven't found any signs that v7/8 require it, though.
			// I haven't checked HE, but since it uses the same AKOS costumes as v7/8 I leave that
			// as it is...
			if ((vald & 3) == newDirToOldDir(_facing)) {
				// v1/2 skip the frame only if everything is equal...
				if (_vm->_game.version > 2 || (vald >> 2) == _frame)
					continue;
			}
			vald >>= 2;
			if (_vm->_game.version < 3)
				_frame = vald;
		}
		_vm->_costumeLoader->costumeDecodeData(this, vald, (_vm->_game.version <= 2) ? 0xFFFF : aMask);
	}

	_needRedraw = true;
}

void Actor_v0::setDirection(int direction) {
	int dir = newDirToOldDir(direction);
	int res = 0;

	switch (dir) {
		case 0:
			res = 4;	// Left
			break;

		case 1:
			res = 5;	// Right
			break;

		case 2:
			res = 6;	// Face Camera
			break;

		default:
			res = 7;	// Face Away
			break;
	}

	_animFrameRepeat = -1;
	animateActor(res);
}

void Actor::faceToObject(int obj) {
	int x2, y2, dir, width;

	if (!isInCurrentRoom())
		return;

	if (_vm->getObjectOrActorXY(obj, x2, y2) == -1)
		return;

	if (_vm->_game.version > 4) {
		dir = (x2 > _pos.x) ? 90 : 270;
	} else {
		_vm->getObjectOrActorWidth(obj, width);
		dir = (_pos.x < x2) ? 1 : 0;
		if (abs(_pos.x - x2) < width / 2)
			dir = (_pos.y > y2) ? 3 : 2;

		dir = oldDirToNewDir(dir);
	}

	turnToDirection(dir);
}

void Actor::turnToDirection(int newdir) {
	if (newdir == -1 || _ignoreTurns)
		return;

	_targetFacing = newdir;

	if (_vm->_game.version == 0)
		setDirection(newdir);
	else if (_vm->_game.version <= 2)
		_moving |= MF_TURN;
	else
		_moving = MF_TURN;
}

void Actor_v7::turnToDirection(int newdir) {
	if (newdir == -1 || _ignoreTurns)
		return;

	newdir = remapDirection((newdir + 360) % 360, false);
	_moving &= ~MF_TURN;

	if (isInCurrentRoom() && !_ignoreBoxes) {
		byte flags = _vm->getBoxFlags(_walkbox);
		if ((flags & kBoxXFlip) || isInClass(kObjectClassXFlip))
			newdir = 360 - newdir;
		if ((flags & kBoxYFlip) || isInClass(kObjectClassYFlip))
			newdir = 180 - newdir;
	}

	if (newdir != _facing) {
		_moving |= MF_TURN;
		_targetFacing = newdir;
	}
}


#pragma mark -
#pragma mark --- Actor position ---
#pragma mark -


void ScummEngine::putActors() {
	Actor *a;
	int i;

	for (i = 1; i < _numActors; i++) {
		a = _actors[i];
		if (a && a->isInCurrentRoom())
			a->putActor();
	}
}

void Actor::putActor(int dstX, int dstY, int newRoom) {
	if (_visible && _vm->_currentRoom != newRoom && _vm->getTalkingActor() == _number) {
		_vm->stopTalk();
	}

	// WORKAROUND: The green transparency of the tank in the Hall of Oddities
	// is positioned one pixel too far to the left. This appears to be a bug
	// in the original game as well.
	if (_vm->_game.id == GID_SAMNMAX && newRoom == 16 && _number == 5 && dstX == 235 && dstY == 236 && _vm->enhancementEnabled(kEnhMinorBugFixes))
		dstX++;

	_pos.x = dstX;
	_pos.y = dstY;
	_room = newRoom;
	_needRedraw = true;

	if (_vm->_game.heversion >= 70)
		_needBgReset = true;

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
#ifdef ENABLE_HE
			if (_vm->_game.heversion >= 71)
				((ScummEngine_v71he *)_vm)->heQueueEraseAuxActor((ActorHE *)this);
#endif
			hideActor();
		}
	} else {
		if (isInCurrentRoom())
			showActor();
	}

	if (_vm->_game.version == 0) {

		((Actor_v0 *)this)->_newWalkBoxEntered = false;
		((Actor_v0 *)this)->_CurrentWalkTo = _pos;
		((Actor_v0 *)this)->_NewWalkTo = _pos;
	}

	// V0-V1 Maniac always sets the actor to face the camera upon entering a room
	if (_vm->_game.id == GID_MANIAC && _vm->_game.version <= 1 && _vm->_game.platform != Common::kPlatformNES)
		setDirection(oldDirToNewDir(2));
}

static bool inBoxQuickReject(const BoxCoords &box, int x, int y, int threshold) {
	int t;

	t = x - threshold;
	if (t > box.ul.x && t > box.ur.x && t > box.lr.x && t > box.ll.x)
		return true;

	t = x + threshold;
	if (t < box.ul.x && t < box.ur.x && t < box.lr.x && t < box.ll.x)
		return true;

	t = y - threshold;
	if (t > box.ul.y && t > box.ur.y && t > box.lr.y && t > box.ll.y)
		return true;

	t = y + threshold;
	if (t < box.ul.y && t < box.ur.y && t < box.lr.y && t < box.ll.y)
		return true;

	return false;
}

static int checkXYInBoxBounds(int boxnum, int x, int y, int &destX, int &destY) {
	BoxCoords box = g_scumm->getBoxCoordinates(boxnum);
	int xmin, xmax;

	// We are supposed to determine the point (destX,destY) contained in
	// the given box which is closest to the point (x,y), and then return
	// some kind of "distance" between the two points.

	// First, we determine destY and a range (xmin to xmax) in which destX
	// is contained.
	if (y < box.ul.y) {
		// Point is above the box
		destY = box.ul.y;
		xmin = box.ul.x;
		xmax = box.ur.x;
	} else if (y >= box.ll.y) {
		// Point is below the box
		destY = box.ll.y;
		xmin = box.ll.x;
		xmax = box.lr.x;
	} else if ((x >= box.ul.x) && (x >= box.ll.x) && (x < box.ur.x) && (x < box.lr.x)) {
		// Point is strictly inside the box
		destX = x;
		destY = y;
		xmin = xmax = x;
	} else {
		// Point is to the left or right of the box,
		// so the y coordinate remains unchanged
		destY = y;
		int ul = box.ul.x;
		int ll = box.ll.x;
		int ur = box.ur.x;
		int lr = box.lr.x;
		int top = box.ul.y;
		int bottom = box.ll.y;
		int cury;

		// Perform a binary search to determine the x coordinate.
		// Note: It would be possible to compute this value in a
		// single step simply by calculating the slope of the left
		// resp. right side and using that to find the correct
		// result. However, the original engine did use the search
		// approach, so we do that, too.
		do {
			xmin = (ul + ll) / 2;
			xmax = (ur + lr) / 2;
			cury = (top + bottom) / 2;

			if (cury < y) {
				top = cury;
				ul = xmin;
				ur = xmax;
			} else if (cury > y) {
				bottom = cury;
				ll = xmin;
				lr = xmax;
			}
		} while (cury != y);
	}

	// Now that we have limited the value of destX to a fixed
	// interval, it's a trivial matter to finally determine it.
	if (x < xmin) {
		destX = xmin;
	} else if (x > xmax) {
		destX = xmax;
	} else {
		destX = x;
	}

	// Compute the distance of the points. We measure the
	// distance with a granularity of 8x8 blocks only (hence
	// yDist must be divided by 4, as we are using 8x2 pixels
	// blocks for actor coordinates).
	int xDist = ABS(x - destX);
	int yDist = ABS(y - destY) / 4;
	int dist;

	if (g_scumm->_game.version == 0)
		xDist *= 2;

	if (xDist < yDist)
		dist = (xDist >> 1) + yDist;
	else
		dist = (yDist >> 1) + xDist;

	return dist;
}

AdjustBoxResult Actor_v0::adjustPosInBorderWalkbox(AdjustBoxResult box) {
	AdjustBoxResult Result = box;
	BoxCoords BoxCoord = _vm->getBoxCoordinates(box.box);

	byte boxMask = _vm->getMaskFromBox(box.box);
	if (!(boxMask & 0x80))
		return Result;

	int16 A;
	boxMask &= 0x7C;
	if (boxMask == 0x0C)
		A = 2;
	else {
		if (boxMask != 0x08)
			return Result;

		A = 1;
	}

	// 1BC6
	byte Modifier = box.y - BoxCoord.ul.y;
	assert(Modifier < 0x16);

	if (A == 1) {
		// 1BCF
		A = BoxCoord.ur.x - v0WalkboxSlantedModifier[ Modifier ];
		if (A < box.x)
			return box;

		if (A <= 0xA0)
			A = 0;

		Result.x = A;
	} else {
		// 1BED
		A = BoxCoord.ul.x + v0WalkboxSlantedModifier[ Modifier ];

		if (A < box.x || A == box.x)
			Result.x = A;
	}

	return Result;
}

AdjustBoxResult Actor_v0::adjustXYToBeInBox(int dstX, int dstY) {
	AdjustBoxResult Result = Actor_v2::adjustXYToBeInBox(dstX, dstY);

	if (Result.box == kInvalidBox)
		return Result;

	return adjustPosInBorderWalkbox(Result);
}

AdjustBoxResult Actor_v2::adjustXYToBeInBox(const int dstX, const int dstY) {
	AdjustBoxResult abr;

	abr.x = dstX;
	abr.y = dstY;
	abr.box = kInvalidBox;

	int numBoxes = _vm->getNumBoxes() - 1;
	int bestDist = 0xFF;
	for (int i = 0; i <= numBoxes; i++) {
		// MM v0 prioritizes lower boxes, other engines higher boxes
		int box = (_vm->_game.version == 0 ? i : numBoxes - i);
		int foundX, foundY;
		int flags = _vm->getBoxFlags(box);
		if ((flags & kBoxInvisible) && !((flags & kBoxPlayerOnly) && !isPlayer()))
			continue;
		int dist = checkXYInBoxBounds(box, dstX, dstY, foundX, foundY);	// also merged with getClosestPtOnBox
		if (dist == 0) {
			abr.x = foundX;
			abr.y = foundY;
			abr.box = box;

			break;
		}
		if (dist < bestDist) {
			bestDist = dist;
			abr.x = foundX;
			abr.y = foundY;
			abr.box = box;
		}
	}

	return abr;
}

AdjustBoxResult Actor::adjustXYToBeInBox(int dstX, int dstY) {
	const uint thresholdTable[] = { 30, 80, 0 };
	AdjustBoxResult abr;
	int16 tmpX, tmpY;
	int tmpDist, bestDist, threshold, numBoxes;
	byte flags, bestBox;
	int box;
	const int firstValidBox = (_vm->_game.features & GF_SMALL_HEADER) ? 0 : 1;

	// The original v3-4 interpreters register and use the last valid (X,Y) values
	// for the current actor. During the execution of the current function, if
	// the routine can't find a bestDist which is smaller than the init value
	// (0xFFFF), the rest of the code flow just happens to reuse the last valid
	// coordinates obtained from a previous call of the function.
	// This sounds like pseudo-undefined behavior caused by an oversight, but we
	// can get along with it as it appears we actually need this to happen...
	//
	// By simulating what v3 and v4 disasms did, we correctly fix bug #2377.
	// The originals relied on global variables containing the latest valid
	// coordinates modified in here and in startWalkActor, but registering
	// the modifications only in here seems to be enough to cover this edge case.
	//
	// HE and v5-6 games appear to actually have undefined behavior, since they
	// create something akin to our AdjustBoxResult structure right inside the
	// current function, so this means that the coordinates inside it potentially
	// might never be initialized if bestDist is never modified.
	// The same thing applies to v7-8, but it's really unlikely that any distance
	// would end up being higher than 0x7FFFFFFF...

	bool isOldSystem = _vm->_game.version <= 4;

	abr.x = isOldSystem ? _lastValidX : dstX;
	abr.y = isOldSystem ? _lastValidY : dstY;
	abr.box = kInvalidBox;

	if (_ignoreBoxes) {
		abr.x = dstX;
		abr.y = dstY;
		return abr;
	}

	for (int tIdx = 0; tIdx < ARRAYSIZE(thresholdTable); tIdx++) {
		threshold = thresholdTable[tIdx];

		numBoxes = _vm->getNumBoxes() - 1;
		if (numBoxes < firstValidBox)
			return abr;

		bestDist = (_vm->_game.version >= 7) ? 0x7FFFFFFF : 0xFFFF;
		bestBox = kInvalidBox;

		// We iterate (backwards) over all boxes, searching the one closest
		// to the desired coordinates.
		for (box = numBoxes; box >= firstValidBox; box--) {
			flags = _vm->getBoxFlags(box);

			// Skip over invisible boxes
			if ((flags & kBoxInvisible) && !((flags & kBoxPlayerOnly) && !isPlayer()))
				continue;

			// For increased performance, we perform a quick test if
			// the coordinates can even be within a distance of 'threshold'
			// pixels of the box.
			if (threshold > 0 && inBoxQuickReject(_vm->getBoxCoordinates(box), dstX, dstY, threshold))
				continue;

			// Check if the point is contained in the box. If it is,
			// we don't have to search anymore.
			if (_vm->checkXYInBoxBounds(box, dstX, dstY)) {
				_lastValidX = dstX;
				_lastValidY = dstY;
				abr.x = dstX;
				abr.y = dstY;
				abr.box = box;
				return abr;
			}

			// Find the point in the box which is closest to our point.
			tmpDist = getClosestPtOnBox(_vm->getBoxCoordinates(box), dstX, dstY, tmpX, tmpY);

			// Check if the box is closer than the previous boxes.
			if (tmpDist < bestDist) {
				_lastValidX = tmpX;
				_lastValidY = tmpY;
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
	_cost.soundPos = 0;

	if (_walkbox != kInvalidBox) {
		byte flags = _vm->getBoxFlags(_walkbox);
		if (flags & 7) {
			turnToDirection(_facing);
		}
	}
}

int ScummEngine::getActorFromPos(int x, int y) {
	if (!testGfxAnyUsageBits(x / 8))
		return 0;

	for (int i = 1; i < _numActors; i++) {
		int16 y1 = _actors[i]->_top;
		int16 y2 = _actors[i]->_bottom;

		if (_game.version <= 2) {
			if (i == VAR(VAR_EGO))
				continue;
			y2 = _actors[i]->getPos().y;
			y1 = _actors[i]->getPos().y - 40 * V12_Y_MULTIPLIER;

			if (_game.version < 2 && _game.id == GID_MANIAC) {
				// I have found this only in MMv1. The other v1/v2 games have leftovers of this
				// (they read the elevation value from the array, but then don't use that value).
				// I have no way to check MMv0 (which also uses this opcode), but I assume it's
				// more likely that it also has this.
				y2 = (byte)(y2 - _actors[i]->getElevation());
				y1 = (byte)(y1 - _actors[i]->getElevation());
			} else {
				// Yes, it's really like this in the original code. And it works as intended for
				// e. g. bug #15277 ("MANIAC: Man-Eating Plant should not be selectable as actor")
				if ((uint16)y1 > 128)
					y1 = 1;
			}
		}

		if (testGfxUsageBit(x / 8, i) && !getClass(i, kObjectClassUntouchable) && y >= y1 && y <= y2)
			return i;
	}

	return 0;
}

int ScummEngine_v70he::getActorFromPos(int x, int y) {
	int curActor, i;

	if (!testGfxAnyUsageBits(x / 8))
		return 0;

	curActor = 0;
	for (i = 1; i < _numActors; i++) {
		if (testGfxUsageBit(x / 8, i) && !getClass(i, kObjectClassUntouchable)
			&& y >= _actors[i]->_top && y <= _actors[i]->_bottom
			&& (_actors[i]->getPos().y > _actors[curActor]->getPos().y || curActor == 0))
				curActor = i;
	}

	return curActor;
}


#pragma mark -
#pragma mark --- TODO ---
#pragma mark -


void Actor::hideActor() {
	if (!_visible)
		return;

	if (_moving) {
		stopActorMoving();
		startAnimActor(_standFrame);
	}
	_visible = false;
	_cost.soundCounter = 0;
	_cost.soundPos = 0;
	_needRedraw = false;
	_needBgReset = true;
}

void ActorHE::hideActor() {
	Actor::hideActor();
	_auxActor = 0;
	_auxEraseX1 = 0;
	_auxEraseY1 = 0;
	_auxEraseX2 = -1;
	_auxEraseY2 = -1;
}

void Actor::showActor() {
	if (_vm->_currentRoom == 0 || _visible)
		return;

	adjustActorPos();

	_vm->ensureResourceLoaded(rtCostume, _costume);

	if (_vm->_game.version == 0) {
		Actor_v0 *a = ((Actor_v0 *)this);

		a->_costCommand = a->_costCommandNew = 0xFF;
		_walkdata.dest = a->_CurrentWalkTo;

		for (int i = 0; i < 8; ++i) {
			a->_limbFrameRepeat[i] = 0;
			a->_limbFrameRepeatNew[i] = 0;
		}

		_cost.reset();

		a->_animFrameRepeat = 1;
		a->_speaking = 0;

		startAnimActor(_standFrame);
		_visible = true;
		return;

	} else if (_vm->_game.version <= 2) {
		_cost.reset();
		startAnimActor(_standFrame);
		startAnimActor(_initFrame);
		startAnimActor(_talkStopFrame);
	} else {
		if (_costumeNeedsInit) {
			startAnimActor(_initFrame);
			_costumeNeedsInit = false;
		}
	}

	stopActorMoving();
	_visible = true;
	_needRedraw = true;
}

void ScummEngine::showActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i]->isInCurrentRoom())
			_actors[i]->showActor();
	}
}

/* Used in Scumm v5 only. Play sounds associated with actors */
void ScummEngine::playActorSounds() {
	int i, j;
	int sound;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i]->_cost.soundCounter && _actors[i]->isInCurrentRoom()) {
			_currentScript = 0xFF;
			if (_game.version == 0) {
				sound = _actors[i]->_sound[0] & 0x3F;
			} else {
				sound = _actors[i]->_sound[0];
			}
			// fast mode will flood the queue with walk sounds
			if (!_fastMode) {
				_sound->startSound(sound);
			}
			for (j = 1; j < _numActors; j++) {
				_actors[j]->_cost.soundCounter = 0;
			}
			return;
		}
	}
}

bool ScummEngine::isValidActor(int id) const {
	return id >= 0 && id < _numActors && _actors[id]->_number == id;
}

Actor *ScummEngine::derefActor(int id, const char *errmsg) const {
	if (id == 0)
		debugC(DEBUG_ACTORS, "derefActor(0, \"%s\") in script %d, opcode 0x%x",
			errmsg, _currentScript != 0xFF ? vm.slot[_currentScript].number : -1, _opcode);

	if (!isValidActor(id)) {
		if (errmsg)
			error("Invalid actor %d in %s", id, errmsg);
		else
			error("Invalid actor %d", id);
	}
	return _actors[id];
}

Actor *ScummEngine::derefActorSafe(int id, const char *errmsg) const {
	if (id == 0)
		debugC(DEBUG_ACTORS, "derefActorSafe(0, \"%s\") in script %d, opcode 0x%x",
			errmsg, _currentScript != 0xFF ? vm.slot[_currentScript].number : -1, _opcode);

	if (!isValidActor(id)) {
		debugC(DEBUG_ACTORS, "Invalid actor %d in %s (script %d, opcode 0x%x)",
			 id, errmsg, _currentScript != 0xFF ? vm.slot[_currentScript].number : -1, _opcode);
		return nullptr;
	}
	return _actors[id];
}


#pragma mark -
#pragma mark --- Actor drawing ---
#pragma mark -


void ScummEngine::processActors() {
	int numactors = 0;

#ifdef ENABLE_HE
	if (_game.heversion >= 71 && ((ScummEngine_v71he *)this)->_disableActorDrawingFlag)
		return;
#endif

	// Make a list of all actors in this room
	for (int i = 1; i < _numActors; i++) {
		if (_game.version == 8 && _actors[i]->_layer < 0)
			continue;
		if (_actors[i]->isInCurrentRoom()) {
			_sortedActors[numactors++] = _actors[i];
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
	// past, including bugs #912, #1055, and #1864.
	//
	// Note that Sam & Max uses a stable sorting method. Older games don't
	// and, according to cyx, neither do newer ones. At least not FT and
	// COMI. See bug #2064 for more details.

	if (_game.id == GID_SAMNMAX) {
		for (int j = 0; j < numactors; ++j) {
			for (int i = 0; i < numactors; ++i) {
				int sc_actor1 = _sortedActors[j]->getPos().y;
				int sc_actor2 = _sortedActors[i]->getPos().y;
				if (sc_actor1 == sc_actor2) {
					sc_actor1 += _sortedActors[j]->_number;
					sc_actor2 += _sortedActors[i]->_number;
				}
				if (sc_actor1 < sc_actor2) {
					SWAP(_sortedActors[i], _sortedActors[j]);
				}
			}
		}
	} else if (_game.heversion >= 90) {
		for (int j = 0; j < numactors; ++j) {
			for (int i = 0; i < numactors; ++i) {
				int sc_actor1 = _sortedActors[j]->_layer;
				int sc_actor2 = _sortedActors[i]->_layer;
				if (sc_actor1 < sc_actor2) {
					SWAP(_sortedActors[i], _sortedActors[j]);
				} else if (sc_actor1 == sc_actor2) {
					sc_actor1 = _sortedActors[j]->getPos().y;
					sc_actor2 = _sortedActors[i]->getPos().y;
					if (sc_actor1 < sc_actor2) {
						SWAP(_sortedActors[i], _sortedActors[j]);
					}
				}
			}
		}
	} else if (_game.version == 0) {
		for (int j = 0; j < numactors; ++j) {
			for (int i = 0; i < numactors; ++i) {
				// Note: the plant is handled different in v0, the y value is not used.
				// In v1/2 this is done by the actor's elevation instead.
				int sc_actor1 = (_sortedActors[j]->_number == 19 ? 0 : _sortedActors[j]->getPos().y);
				int sc_actor2 = (_sortedActors[i]->_number == 19 ? 0 : _sortedActors[i]->getPos().y);
				if (sc_actor1 < sc_actor2) {
					SWAP(_sortedActors[i], _sortedActors[j]);
				}
			}
		}
	} else {
		for (int j = 0; j < numactors; ++j) {
			for (int i = 0; i < numactors; ++i) {
				int sc_actor1 = _sortedActors[j]->getPos().y - _sortedActors[j]->_layer * 2000;
				int sc_actor2 = _sortedActors[i]->getPos().y - _sortedActors[i]->_layer * 2000;
				if (sc_actor1 < sc_actor2) {
					SWAP(_sortedActors[i], _sortedActors[j]);
				}
			}
		}
	}

	// Finally draw the now sorted actors
	Actor **end = _sortedActors + numactors;
	for (Actor **ac = _sortedActors; ac != end; ++ac) {
		Actor *a = *ac;

		if (_game.version == 0) {
			// 0x057B
			Actor_v0 *a0 = (Actor_v0 *)a;
			if (a0->_speaking & 1) {
				a0->_speaking ^= 0xFE;
				++_V0Delay._actorRedrawCount;
			}
			// 0x22B5
			if (a0->_miscflags & kActorMiscFlagHide)
				continue;

			// Sound
			if (a0->_moving != 2 && _currentRoom != 1 && _currentRoom != 44) {
				if (a0->_cost.soundPos == 0)
					a0->_cost.soundCounter++;

				// Is this the correct location?
				// 0x073C
				if (a0->_sound[0] & 0x3F)
					a0->_cost.soundPos = (a0->_cost.soundPos + 1) % 3;
			}
		}
		// Draw and animate the actors, except those w/o a costume.
		// Note: We could 'optimize' this a little bit by only putting
		// actors with a costume into the _sortedActors array in the
		// first place. However, that would mess up the sorting, and
		// would hence cause regressions. See also the other big
		// comment further up in this method for some details.
		if (a->_costume) {

			// Unfortunately in V0, the 'animateCostume' call happens right after the call to 'walkActor' (which is before drawing the actor)...
			// doing it the other way with V0, causes animation glitches (when beginnning to walk, as the costume hasn't been updated).
			// Updating the costume directly after 'walkActor' and again, after drawing... causes frame skipping
			if (_game.version == 0) {
				a->animateCostume();
				a->drawActorCostume();
			} else {
				a->drawActorCostume();
				a->animateCostume();

				if (_game.heversion >= 80) {
					if (VAR_ALWAYS_REDRAW_ACTORS != 0xFF && VAR(VAR_ALWAYS_REDRAW_ACTORS) != 0)
						continue;
				}

				if (_game.heversion >= 71) {
					// Check if this new actor eclipsed another one...
					for (int i = 0; i < _gdi->_numStrips; i++) {
						int strip = _screenStartStrip + i;
						if (testGfxAnyUsageBits(strip)) {
							for (int j = 1; j < _numActors; j++) {
								if (testGfxUsageBit(strip, j) && testGfxOtherUsageBits(strip, j)) {
									_actors[j]->_needRedraw = true;
								}
							}
						}
					}
				}
			}
		}
	}
}

void ScummEngine_v6::processActors() {
	ScummEngine::processActors();

	if (_game.features & GF_NEW_COSTUMES)
		akos_processQueue();
}

#ifdef ENABLE_HE
void ScummEngine_v71he::processActors() {
	heFlushAuxEraseQueue();

	if (!_disableActorDrawingFlag)
		ScummEngine_v6::processActors();

	_fullRedraw = false;

	heFlushAuxQueues();
}

void ScummEngine_v90he::processActors() {
	heFlushAuxEraseQueue();

	_sprite->checkForForcedRedraws(false);
	_sprite->renderSprites(true);

	if (!_disableActorDrawingFlag)
		ScummEngine_v6::processActors();

	_fullRedraw = false;

	heFlushAuxQueues();

	_sprite->checkForForcedRedraws(true);
	_sprite->renderSprites(false);
}
#endif

// Used in Scumm v8, to allow the verb coin to be drawn over the inventory
// chest. I'm assuming that draw order won't matter here.
void ScummEngine::processUpperActors() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_actors[i]->isInCurrentRoom() && _actors[i]->_costume && _actors[i]->_layer < 0) {
			_actors[i]->drawActorCostume();
			_actors[i]->animateCostume();
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

	BaseCostumeRenderer *bcr = _vm->_costumeRenderer;
	prepareDrawActorCostume(bcr);

	// If the actor is partially hidden, redraw it next frame.
	if (bcr->drawCostume(_vm->_virtscr[kMainVirtScreen], _vm->_gdi->_numStrips, this, _drawToBackBuf) & 1) {
		_needRedraw = (_vm->_game.version <= 6);

		// TODO: Eventually check if true for HE6*
		if (_vm->_game.heversion >= 70)
			_needBgReset = true;
	}

	if (!hitTestMode) {
		// Record the vertical extent of the drawn actor
		_top = bcr->_drawTop;
		_bottom = bcr->_drawBottom;
	}
}


void Actor::prepareDrawActorCostume(BaseCostumeRenderer *bcr) {

	bcr->_actorID = _number;
	bcr->_actorX = _pos.x - _vm->_virtscr[kMainVirtScreen].xstart;
	bcr->_actorY = _pos.y - _elevation;

	if (_vm->_game.version == 4 && (_boxscale & 0x8000)) {
		bcr->_scaleX = bcr->_scaleY = _vm->getScaleFromSlot((_boxscale & 0x7fff) + 1, _pos.x, _pos.y);
	} else {
		bcr->_scaleX = _scalex;
		bcr->_scaleY = _scaley;
	}

	bcr->_shadowMode = _shadowMode;
	if (_vm->_game.version >= 5 && _vm->_game.heversion == 0) {
		bcr->_shadowTable = _vm->_shadowPalette;
	}

	bcr->setCostume(_costume, (_vm->_game.heversion == 0) ? 0 : _heShadow);
	bcr->setPalette(_palette);
	bcr->setFacing(this);

	if (_vm->_game.version >= 7) {

		bcr->_zbuf = _forceClip;
		if (bcr->_zbuf == 100) {
			bcr->_zbuf = _vm->getMaskFromBox(_walkbox);
			if (bcr->_zbuf > _vm->_gdi->_numZBuffer-1)
				bcr->_zbuf = _vm->_gdi->_numZBuffer-1;
		}

	} else {
		if (_forceClip)
			bcr->_zbuf = _forceClip;
		else if (isInClass(kObjectClassNeverClip))
			bcr->_zbuf = 0;
		else {
			bcr->_zbuf = _vm->getMaskFromBox(_walkbox);
			if (_vm->_game.version == 0)
				bcr->_zbuf &= 0x03;
			if (bcr->_zbuf > _vm->_gdi->_numZBuffer-1)
				bcr->_zbuf = _vm->_gdi->_numZBuffer-1;
		}

	}

	bcr->_drawTop = 0x7fffffff;
	bcr->_drawBottom = 0;
}

void ActorHE::prepareDrawActorCostume(BaseCostumeRenderer *bcr) {
	// HE palette number must be set, before setting the costume palette
	bcr->_paletteNum = _hePaletteNum;

	clearActorUpdateInfo();

	Actor::prepareDrawActorCostume(bcr);

	bcr->_actorX += _heOffsX;
	bcr->_actorY += _heOffsY;

	bcr->_clipOverride = _clipOverride;

	if (_vm->_game.heversion == 70) {
		bcr->_shadowTable = _vm->_HEV7ActorPalette;
	}

	bcr->_skipLimbs = (_heSkipLimbs != 0);

	if (_vm->_game.heversion >= 80 && _heNoTalkAnimation == 0 && _animProgress == 0) {
		if (_vm->getTalkingActor() == _number && !_vm->_string[0].no_talk_anim) {
			int talkState = -1;

			if (((SoundHE *)_vm->_sound)->isSoundCodeUsed(HSND_TALKIE_SLOT))
				talkState = ((SoundHE *)_vm->_sound)->getSoundVar(HSND_TALKIE_SLOT, 19);

			// Allow a talkie with tokens to kick into random mouth mode
			if (talkState == -1 || talkState == 0)
				talkState = _vm->_rnd.getRandomNumberRng(1, 10);

			assertRange(1, talkState, 13, "Talk state");
			setTalkCondition(talkState);
		} else {
			setTalkCondition(1);
		}
	}
	_heNoTalkAnimation = 0;
}

void Actor_v2::prepareDrawActorCostume(BaseCostumeRenderer *bcr) {
	Actor::prepareDrawActorCostume(bcr);

	bcr->_actorX = _pos.x;
	bcr->_actorY = _pos.y - _elevation;

	if (_vm->_game.version <= 2) {
		bcr->_actorX *= V12_X_MULTIPLIER;
		bcr->_actorY *= V12_Y_MULTIPLIER;
	}
	bcr->_actorX -= _vm->_virtscr[kMainVirtScreen].xstart;

	if (_vm->_game.platform == Common::kPlatformNES) {
		// In the NES version, when the actor is facing right,
		// we need to shift it 8 pixels to the left
		if (_facing == 90)
			bcr->_actorX -= 8;
	} else if (_vm->_game.version == 0) {
			bcr->_actorX += 12;
	} else if (_vm->_game.version <= 2) {
		// HACK: We have to adjust the x position by one strip (8 pixels) in
		// V2 games. However, it is not quite clear to me why. And to fully
		// match the original, it seems we have to offset by 2 strips if the
		// actor is facing left (270 degree).
		// V1 games are once again slightly different, here we only have
		// to adjust the 270 degree case...
		if (_facing == 270)
			bcr->_actorX += 16;
		else if (_vm->_game.version == 2)
			bcr->_actorX += 8;
	}
}

#ifdef ENABLE_SCUMM_7_8
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

void Actor::startAnimActor(int f) {
	if (_vm->_game.heversion > 99) {
		switch (f) {
		case HE100_CHORE_REDIRECT_INIT:
			f = _initFrame;
			break;
		case HE100_CHORE_REDIRECT_WALK:
			f = _walkFrame;
			break;
		case HE100_CHORE_REDIRECT_STAND:
			f = _standFrame;
			break;
		case HE100_CHORE_REDIRECT_START_TALK:
			f = _talkStartFrame;
			break;
		case HE100_CHORE_REDIRECT_STOP_TALK:
			f = _talkStopFrame;
			break;
		default:
			break;
		}
	} else {
		switch (f) {
		case CHORE_REDIRECT_INIT:
			f = _initFrame;
			break;
		case CHORE_REDIRECT_WALK:
			f = _walkFrame;
			break;
		case CHORE_REDIRECT_STAND:
			f = _standFrame;
			break;
		case CHORE_REDIRECT_START_TALK:
			f = _talkStartFrame;
			break;
		case CHORE_REDIRECT_STOP_TALK:
			f = _talkStopFrame;
			break;
		default:
			break;
		}
	}

	assert(f != 0x3E);

	if (isInCurrentRoom() && _costume != 0) {
		_animProgress = 0;
		_needRedraw = true;
		_cost.animCounter = 0;
		// V1 - V2 games don't seem to need a _cost.reset() at this point.
		// Causes Zak to lose his body in several scenes, see bug #1032
		if (_vm->_game.version >= 3 && f == _initFrame) {
			_cost.reset();
			if (_vm->_game.heversion != 0) {
				((ActorHE *)this)->_auxActor = 0;
				((ActorHE *)this)->_auxEraseX1 = 0;
				((ActorHE *)this)->_auxEraseY1 = 0;
				((ActorHE *)this)->_auxEraseX2 = -1;
				((ActorHE *)this)->_auxEraseY2 = -1;
			}
		}
		_vm->_costumeLoader->costumeDecodeData(this, f, (uint) - 1);
		_frame = f;
	}

	if (_vm->_game.heversion >= 70)
		_needBgReset = true;
}

void Actor_v0::startAnimActor(int f) {
	if (f == _talkStartFrame) {
		if (_sound[0] & 0x40)
			return;

		_speaking = 1;
		speakCheck();
		return;
	}

	if (f == _talkStopFrame) {

		_speaking = 0;
		return;
	}

	if (f == _standFrame)
		setDirection(_facing);
}

void Actor_v7::startAnimActor(int f) {
	if (_vm->_game.id == GID_FT && _vm->_game.platform == Common::kPlatformDOS && (_vm->_game.features & GF_DEMO)) {
		Actor::startAnimActor(f);
		return;
	}

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
	default:
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
}

void Actor::animateActor(int anim) {
	int chore, dir;

	if (_vm->_game.version >= 7 && !((_vm->_game.id == GID_FT) && (_vm->_game.features & GF_DEMO) && (_vm->_game.platform == Common::kPlatformDOS))) {

		if (anim == 0xFF)
			anim = 2000;

		chore = anim / 1000;
		dir = anim % 1000;

	} else {
		// Format of the input parameter:
		// - The 2 least significant bits are the direction
		// - The rest is the chore command to execute
		chore = anim >> 2;
		dir = oldDirToNewDir(anim & 3);

		// Convert into old chore code
		chore = 0x3F - chore + 2;

	}

	switch (chore) {
	case 2:				// stop walking
		if (isInCurrentRoom() ||
			!(_vm->_game.version >= 3 && _vm->_game.version <= 6)) {
			startAnimActor(_standFrame);
			stopActorMoving();
		}
		break;
	case 3:				// change direction immediately
		if (isInCurrentRoom() ||
			!(_vm->_game.version >= 3 && _vm->_game.version <= 6)) {
			_moving &= ~MF_TURN;
		}

		setDirection(dir);
		break;
	case 4:				// turn to new direction
		if (isInCurrentRoom() ||
			!(_vm->_game.version >= 3 && _vm->_game.version <= 6)) {
			turnToDirection(dir);
		}
		break;
	case 64:
		if (_vm->_game.version == 0) {
			_moving &= ~MF_TURN;
			setDirection(dir);
			break;
		}
		// fall through
	default:
		if (_vm->_game.version <= 2)
			startAnimActor(anim >> 2);
		else
			startAnimActor(anim);
	}
}

void Actor::animateCostume() {
	if (_costume == 0)
		return;

	_animProgress++;
	if (_animProgress >= _animSpeed) {
		_animProgress = 0;

		_vm->_costumeLoader->loadCostume(_costume);
		if (_vm->_costumeLoader->increaseAnims(this)) {
			_needRedraw = true;
			if (_vm->_game.heversion >= 70) {
				_needBgReset = true;
			}
		}
	}
}

void Actor_v0::limbFrameCheck(int limb) {
	if (_cost.frame[limb] == 0xFFFF)
		return;

	if (_cost.start[limb] == _cost.frame[limb])
		return;

	// 0x25A4
	_cost.start[limb] = _cost.frame[limb];

	_limbFrameRepeat[limb] = _limbFrameRepeatNew[limb];

	// 0x25C3
	_cost.animType[limb] = ((V0CostumeLoader *)_vm->_costumeLoader)->getFrame(this, limb);
	_cost.curpos[limb] = 0;

	_needRedraw = true;
}

void Actor_v0::animateCostume() {
	speakCheck();

	byte count = _vm->_costumeLoader->increaseAnims(this);

	if (count) {
		_vm->_V0Delay._actorLimbRedrawDrawCount += count;

		_needRedraw = true;
	}
}

void Actor_v0::speakCheck() {
	if (_sound[0] & 0x80)
		return;

	int cmd = newDirToOldDir(_facing);

	if (_speaking & 0x80)
		cmd += 0x0C;
	else
		cmd += 0x10;

	_animFrameRepeat = -1;
	animateActor(cmd);
}

#ifdef ENABLE_SCUMM_7_8
void Actor::animateLimb(int limb, int f) {
	// This methods is very similar to animateCostume().
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

		aksq = _vm->findResourceData(MKTAG('A','K','S','Q'), akos);
		akfo = _vm->findResourceData(MKTAG('A','K','F','O'), akos);

		size = _vm->getResourceDataSize(akfo) / 2;

		while (f--) {
			if (_cost.animType[limb] != AKAT_Empty)
				((ScummEngine_v6 *)_vm)->akos_increaseAnim(this, limb, aksq, (const uint16 *)akfo, size);
		}

//		_needRedraw = true;
//		_needBgReset = true;
	}
}
#endif

void ScummEngine::redrawAllActors() {
	for (int i = 1; i < _numActors; ++i) {
		_actors[i]->_needRedraw = true;
		_actors[i]->_needBgReset = true;
	}
}

void ScummEngine::setActorRedrawFlags() {
	// Redraw all actors if a full redraw was requested.
	// Also redraw all actors in COMI (see bug #1825 for details).
	if (_fullRedraw || _game.version == 8 || (VAR_ALWAYS_REDRAW_ACTORS != 0xFF && VAR(VAR_ALWAYS_REDRAW_ACTORS) != 0)) {
		for (int j = 1; j < _numActors; j++) {
			_actors[j]->_needRedraw = true;
		}
	} else {
		for (int i = 0; i < _gdi->_numStrips; i++) {
			int strip = _screenStartStrip + i;
			if (testGfxAnyUsageBits(strip)) {
				for (int j = 1; j < _numActors; j++) {
					if (testGfxUsageBit(strip, j) && testGfxOtherUsageBits(strip, j)) {
						_actors[j]->_needRedraw = true;
					}
				}
			}
		}
	}
}

void ScummEngine_v70he::setActorRedrawFlags() {
	if (_game.heversion >= 80 && (VAR_ALWAYS_REDRAW_ACTORS != 0xFF && VAR(VAR_ALWAYS_REDRAW_ACTORS) != 0)) {
		for (int i = 1; i < _numActors; i++) {
			if (_actors[i]->_costume) {
				_actors[i]->_needRedraw = true;
				_actors[i]->_needBgReset = true;
			}
		}

		return;
	}

	if (_game.heversion >= 90) {
		for (int j = 1; j < _numActors; j++) {
			if (_actors[j]->_costume && _actors[j]->_heShadow) {
				_actors[j]->_needRedraw = true;
				_actors[j]->_needBgReset = true;
			}
		}
	}

	bool repeatCheck = true;

	while (repeatCheck) {
		repeatCheck = false;

		for (int i = 0; i < _gdi->_numStrips; i++) {
			// Get actors on screen bits for this strip...
			int strip = _screenStartStrip + i;

			if (testGfxAnyUsageBits(strip)) {
				for (int act = 1; act < _numActors; act++) {
					if (!(_actors[act]->_needRedraw && _actors[act]->_needBgReset)) {
						if (testGfxUsageBit(strip, act) && testGfxOtherUsageBits(strip, act)) {
							if (testGfxObjectUsageBits(strip)) {
								if (!_actors[act]->_needRedraw)
									repeatCheck = true;

								if (!_actors[act]->_needBgReset)
									repeatCheck = true;

								_actors[act]->_needRedraw = true;
								_actors[act]->_needBgReset = true;
							} else {
								// Check for vertical overlap...
								for (int iact = 1; iact < _numActors; iact++) {
									if ((iact != act) && testGfxUsageBit(strip, iact)) {
										if (actorsOverlapInStrip(act, iact, i)) {
											// Check for animation as well as animating...
											if (_actors[act]->_needBgReset || _actors[iact]->_needBgReset ||
												_actors[act]->_needRedraw || _actors[iact]->_needRedraw) {

												if (!_actors[act]->_needRedraw)
													repeatCheck = true;

												if (!_actors[act]->_needBgReset)
													repeatCheck = true;

												_actors[act]->_needRedraw = true;
												_actors[act]->_needBgReset = true;

												repeatCheck = true;
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void ScummEngine::resetActorBgs() {
	for (int i = 0; i < _gdi->_numStrips; i++) {
		int strip = _screenStartStrip + i;
		clearGfxUsageBit(strip, USAGE_BIT_DIRTY);
		clearGfxUsageBit(strip, USAGE_BIT_RESTORED);

		for (int j = 1; j < _numActors; j++) {
			if (testGfxUsageBit(strip, j) &&
				((_actors[j]->_top != 0x7fffffff && _actors[j]->_needRedraw) || _actors[j]->_needBgReset)) {
				clearGfxUsageBit(strip, j);
				if ((_actors[j]->_bottom - _actors[j]->_top) >= 0)
					_gdi->resetBackground(_actors[j]->_top, _actors[j]->_bottom, i);
			}
		}
	}

	for (int i = 1; i < _numActors; i++) {
		_actors[i]->_needBgReset = false;
	}
}

void ScummEngine_v70he::resetActorBgs() {
	for (int i = 0; i < _gdi->_numStrips; i++) {
		int strip = _screenStartStrip + i;
		clearGfxUsageBit(strip, USAGE_BIT_DIRTY);
		clearGfxUsageBit(strip, USAGE_BIT_RESTORED);

		for (int j = 1; j < _numActors; j++) {
			if (!testGfxAnyUsageBits(strip))
				break;

			if (!testGfxUsageBit(strip, j))
				continue;

			int actorMin, actorMax;

			if (_screenWidth == 640) { // Hi-res
				if (((ActorHE *)_actors[j])->_screenUpdateTableMin[i] < ((ActorHE *)_actors[j])->_screenUpdateTableMax[i]) {
					actorMin = ((ActorHE *)_actors[j])->_screenUpdateTableMin[i];
					actorMax = ((ActorHE *)_actors[j])->_screenUpdateTableMax[i] + 1;
				} else {
					actorMin = 0x7fffffff;
					actorMax = 0;
				}
			} else {
				actorMin = _actors[j]->_top;
				actorMax = _actors[j]->_bottom;
			}

			// Kill the actors bit in this strip if told to erase
			if (_actors[j]->_needBgReset) {
				clearGfxUsageBit(strip, j);
			}

			if (actorMin != 0x7fffffff && _actors[j]->_needBgReset) {
#ifdef ENABLE_HE
				bool disableDrawing = _game.heversion >= 71 && (((ScummEngine_v71he *)this)->_disableActorDrawingFlag);
#else
				bool disableDrawing = false;
#endif
				if ((actorMax - actorMin) > 0 && !disableDrawing)
					_gdi->resetBackground(actorMin, actorMax, i);
			}
		}
	}

	for (int i = 1; i < _numActors; i++) {
		_actors[i]->_needBgReset = false;
	}
}

#ifdef ENABLE_HE
bool ScummEngine_v95he::prepareForActorErase() {
	for (int i = 1; i < _numActors; i++) {
		if (((ActorHE *)_actors[i])->_generalFlags & ACTOR_GENERAL_FLAG_IGNORE_ERASE) {
			_actors[i]->_needBgReset = false;
		}
	}

	for (int i = 1; i < _numActors; i++) {
		if (_actors[i]->_needBgReset) {
			return true;
		}
	}

	return false;
}

#define ACTOR_CONTIGUOUS_WITH_STRIP (!((jMax < actorMin) || (jMin > actorMax)))

void ScummEngine_v95he::resetActorBgs() {
	int jMin, jMax, lastStrip, actorMin, actorMax;

	if (!prepareForActorErase()) {
		return;
	}

	for (int i = 0; i < _gdi->_numStrips; i++) {
		int strip = _screenStartStrip + i;
		clearGfxUsageBit(strip, USAGE_BIT_DIRTY);
		clearGfxUsageBit(strip, USAGE_BIT_RESTORED);

		for (int act = 1; act < _numActors; act++) {
			if (!testGfxAnyUsageBits(strip))
				break;

			if (!testGfxUsageBit(strip, act))
				continue;

			if (!_actors[act]->_needBgReset)
				continue;

			lastStrip = i;
			actorMin = ((ActorHE *)_actors[act])->_screenUpdateTableMin[i];
			actorMax = ((ActorHE *)_actors[act])->_screenUpdateTableMax[i] + 1;

			for (int j = i; j < _gdi->_numStrips; j++) {
				jMin = ((ActorHE *)_actors[act])->_screenUpdateTableMin[i];
				jMax = ((ActorHE *)_actors[act])->_screenUpdateTableMax[i] + 1;

				if (testGfxOtherUsageBits(strip, act) && ((jMin) < (jMax)) && ACTOR_CONTIGUOUS_WITH_STRIP) {
					// Extend the restore area to include this strip
					lastStrip = j;
					actorMin = MIN<int>(actorMin, jMin);
					actorMax = MAX<int>(actorMax, jMax);
				} else {
					break;
				}
			}

			for (int j = i; j <= lastStrip; j++) {
				clearGfxUsageBit(strip, act);
			}

			if (actorMin != 0x7fffffff && _actors[act]->_needBgReset) {
				bool disableDrawing = (((ScummEngine_v71he *)this)->_disableActorDrawingFlag);
				if ((actorMax - actorMin) > 0 && !disableDrawing)
					_gdi->resetBackground(actorMin, actorMax, i);
			}
		}
	}

	for (int i = 1; i < _numActors; i++) {
		_actors[i]->_needBgReset = false;
	}
}
#endif

#undef ACTOR_CONTIGUOUS_WITH_STRIP

// HE specific
void ActorHE::drawActorToBackBuf(int x, int y) {
	int curTop = _top;
	int curBottom = _bottom;

	int screenUpdateTableMin[80];
	int screenUpdateTableMax[80];

	memcpy(screenUpdateTableMin, _screenUpdateTableMin, sizeof(screenUpdateTableMin));
	memcpy(screenUpdateTableMax, _screenUpdateTableMax, sizeof(screenUpdateTableMax));

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

	for (int i = 0; i < 80; i++) {
		if (screenUpdateTableMin[i] < _screenUpdateTableMin[i]) {
			_screenUpdateTableMin[i] = screenUpdateTableMin[i];
		}

		if (screenUpdateTableMax[i] > _screenUpdateTableMax[i]) {
			_screenUpdateTableMax[i] = screenUpdateTableMax[i];
		}
	}
}

void ActorHE::clearActorUpdateInfo() {
	for (int i = 0; i < _vm->_gdi->_numStrips; i++) {
		_screenUpdateTableMin[i] = _vm->_screenHeight;
		_screenUpdateTableMax[i] = 0;
	}
}

void ActorHE::setActorUpdateArea(int x1, int y1, int x2, int y2) {
	int startStrip, endStrip;

	if (y1 < 0) {
		y1 = 0;
	}

	if (y2 >= _vm->_screenHeight) {
		y2 = _vm->_screenHeight - 1;
	}

	startStrip = x1 / 8;
	if (startStrip < 0) {
		startStrip = 0;
	}

	if (startStrip >= _vm->_gdi->_numStrips) {
		return;
	}

	endStrip = x2 / 8;
	if (endStrip >= _vm->_gdi->_numStrips) {
		endStrip = _vm->_gdi->_numStrips - 1;
	}

	for (int strip = startStrip; strip <= endStrip; strip++) {
		if (y1 < _screenUpdateTableMin[strip]) {
			_screenUpdateTableMin[strip] = y1;
		}

		if (y2 > _screenUpdateTableMax[strip]) {
			_screenUpdateTableMax[strip] = y2;
		}
	}
}

bool ScummEngine_v60he::actorsOverlapInStrip(int actorA, int actorB, int stripNumber) {
	ActorHE *actA = (ActorHE *)_actors[actorA];
	ActorHE *actB = (ActorHE *)_actors[actorB];
	return !((actB->_screenUpdateTableMax[stripNumber] < actA->_screenUpdateTableMin[stripNumber]) ||
			 (actB->_screenUpdateTableMin[stripNumber] < actA->_screenUpdateTableMax[stripNumber]));
}


#pragma mark -
#pragma mark --- Actor talking ---
#pragma mark -


// V1 Maniac doesn't have a ScummVar for VAR_TALK_ACTOR, and just uses
// an internal variable. Emulate this to prevent overwriting script vars...
// Maniac NES (V1), however, DOES have a ScummVar for VAR_TALK_ACTOR
int ScummEngine::getTalkingActor() {
	if (_game.id == GID_MANIAC && _game.version <= 1 && !(_game.platform == Common::kPlatformNES))
		return _V1TalkingActor;
	else
		return VAR(VAR_TALK_ACTOR);
}

void ScummEngine::setTalkingActor(int i) {

	if (i == 255) {
		_system->clearFocusRectangle();
	} else {
		// Work out the screen co-ordinates of the actor
		int x = _actors[i]->getPos().x - (camera._cur.x - (_screenWidth >> 1));
		int y = _actors[i]->_top - (camera._cur.y - (_screenHeight >> 1));

		// Set the focus area to the calculated position
		// TODO: Make the size adjust depending on what it's focusing on.
		_system->setFocusRectangle(Common::Rect::center(x, y, 192, 128));
	}

	if (_game.id == GID_MANIAC && _game.version <= 1 && !(_game.platform == Common::kPlatformNES))
		_V1TalkingActor = i;
	else
		VAR(VAR_TALK_ACTOR) = i;
}

static const int v0MMActorTalkColor[25] = {
	1, 7, 2, 14, 8, 15, 3, 7, 7, 15, 1, 13, 1, 4, 5, 5, 4, 3, 1, 5, 1, 1, 1, 1, 7
};
static const int v1MMActorTalkColor[25] = {
	1, 7, 2, 14, 8, 1, 3, 7, 7, 12, 1, 13, 1, 4, 5, 5, 4, 3, 1, 5, 1, 1, 1, 7, 7
};

void ScummEngine::resetV1ActorTalkColor() {
	int i;

	for (i = 1; i < _numActors; i++) {
		if (_game.version == 0) {
			_actors[i]->_talkColor = v0MMActorTalkColor[i];
		} else {
			_actors[i]->_talkColor = v1MMActorTalkColor[i];
		}
	}
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::actorTalk(const byte *msg) {
	Actor *a;
	bool stringWrap = false;
	bool usingOldSystem = (_game.id == GID_FT) || (_game.id == GID_DIG && _game.features & GF_DEMO);

	// WORKAROUND bug #1493: In Puerto Pollo, if you have Guybrush examine
	// the church clock, he'll read out the current time. However, this was
	// disabled in some releases, possibly because of the poor results for
	// some languages (e.g. German, French). The check was done inside the
	// original interpreters, so we replicate their behavior.
	if (_game.id == GID_CMI && _language != Common::EN_ANY && _language != Common::IT_ITA && _language != Common::RU_RUS) {
		if (strncmp((const char *)msg, "/CKGT326/", 9) == 0)
			msg = (const byte *)"/VDSO325/Whoa! Look at the time. Gotta scoot.";

		// Reject every line which begins with the CKGT tag ("ClocK Guybrush Threepwood")
		if (strncmp((const char *)msg, "/CKGT", 5) == 0)
			return;
	}

	convertMessageToString(msg, _charsetBuffer, sizeof(_charsetBuffer));

	// Play associated speech, if any
	playSpeech((byte *)_lastStringTag);

	if (!usingOldSystem) {
		if (VAR(VAR_HAVE_MSG))
			stopTalk();
	} else {
		if (!_keepText)
			stopTalk();
	}
	if (_actorToPrintStrFor == 0xFF) {
		setTalkingActor(0xFF);
		_charsetColor = (byte)_string[0].color;
	} else {
		a = derefActor(_actorToPrintStrFor, "actorTalk");
		setTalkingActor(a->_number);
		if (!_string[0].no_talk_anim) {
			a->runActorTalkScript(a->_talkStartFrame);
		}
		_charsetColor = a->_talkColor;

		// This is what the original COMI CJK interpreter does here.
		if (_game.id == GID_CMI && _useCJKMode) {
			if (a->_number == 1 && _currentRoom == 15)
				_charsetColor = 28;
			else if (a->_talkColor == 22)
				_charsetColor = 5;
		}
	}

	_charsetBufPos = 0;
	_talkDelay = 0;
	_haveMsg = 1;
	if (usingOldSystem)
		VAR(VAR_HAVE_MSG) = 0xFF;
	_haveActorSpeechMsg = usingOldSystem ? true : (!_sound->isSoundRunning(kTalkSoundID));

	if (!usingOldSystem) {
		stringWrap = _string[0].wrapping;
		_string[0].wrapping = true;
	}

	displayDialog();

	if (!usingOldSystem) {
		if (_game.version == 8)
			VAR(VAR_HAVE_MSG) = (_string[0].no_talk_anim) ? 2 : 1;
		else
			VAR(VAR_HAVE_MSG) = 1;
		_string[0].wrapping = stringWrap;
	}
}
#endif

void ScummEngine::actorTalk(const byte *msg) {
	Actor *a;

	convertMessageToString(msg, _charsetBuffer, sizeof(_charsetBuffer));

	// I have commented out this workaround, since it did cause another
	// bug (#11480). It is not okay to skip the stopTalk() calls here.
	// Instead, I have added two checks from LOOM DOS EGA disasm (one
	// below and one in displayDialog()).
	// WORKAROUND for bugs #985 and #990
	/*if (_game.id == GID_LOOM) {
		if (!*_charsetBuffer)
			return;
	}*/

	if (_actorToPrintStrFor == 0xFF) {
		if (!_keepText) {
			stopTalk();
		}
		setTalkingActor(0xFF);
	} else {
		int oldact;

		a = derefActor(_actorToPrintStrFor, "actorTalk");

		if (!a->isInCurrentRoom()) {
			oldact = 0xFF;
		} else {
			if (!_keepText) {
				stopTalk();
			}
			setTalkingActor(a->_number);
			if (_game.heversion != 0)
				((ActorHE *)a)->_heTalking = true;
			// The second check is from LOOM DOS EGA disasm. It prevents weird speech animations
			// with empty strings (bug #990). The same code is present in displayDialog(). The FM-Towns
			// versions don't have such code, but I do not get the weird speech animations either.
			// So apparently it is not needed there.
			if (!_string[0].no_talk_anim && !(_game.id == GID_LOOM && _game.platform != Common::kPlatformFMTowns && !*_charsetBuffer)) {
				a->runActorTalkScript(a->_talkStartFrame);
				_useTalkAnims = true;
			}
			oldact = getTalkingActor();
		}
		if (oldact >= 0x80)
			return;
	}

	if (_game.heversion >= 72 || getTalkingActor() > 0x7F) {
		if (_game.platform == Common::kPlatformNES)
			_charsetColor = 0; // NES MM intro color is always 0
		else
			_charsetColor = (byte)_string[0].color;
	} else if (_game.platform == Common::kPlatformNES) {
		if (_NES_lastTalkingActor != getTalkingActor())
			_NES_talkColor ^= 1;
		_NES_lastTalkingActor = getTalkingActor();
		_charsetColor = _NES_talkColor;
	} else {
		a = derefActor(getTalkingActor(), "actorTalk(2)");
		_charsetColor = (_game.platform == Common::kPlatformApple2GS && !enhancementEnabled(kEnhVisualChanges)) ? 1 : a->_talkColor;
	}
	_charsetBufPos = 0;
	_talkDelay = 0;
	_haveMsg = 0xFF;
	VAR(VAR_HAVE_MSG) = 0xFF;
	if (VAR_CHARCOUNT != 0xFF)
		VAR(VAR_CHARCOUNT) = 0;
	_haveActorSpeechMsg = true;
	displayDialog();
}

void Actor::runActorTalkScript(int f) {
	if (_vm->_game.version == 8 && _vm->VAR(_vm->VAR_HAVE_MSG) == 2)
		return;

	if (_vm->_game.id == GID_FT && _vm->_string[0].no_talk_anim)
		return;

	if (!_vm->getTalkingActor() || _room != _vm->_currentRoom || _frame == f)
		return;

	if (_talkScript) {
		int script = _talkScript;
		int args[NUM_SCRIPT_LOCAL];
		memset(args, 0, sizeof(args));
		args[1] = f;
		args[0] = _number;

		_vm->runScript(script, 1, 0, args);
	} else {
		startAnimActor(f);
	}
}

void ScummEngine::stopTalk() {
	int act;

	_sound->stopTalkSound();

	_haveMsg = 0;
	_talkDelay = 0;
	_sound->_digiSndMode = DIGI_SND_MODE_EMPTY;

	act = getTalkingActor();
	if (act && act < 0x80) {
		Actor *a = derefActor(act, "stopTalk");
		if ((_game.version >= 7 && !_string[0].no_talk_anim) ||
			(_game.version <= 6 && a->isInCurrentRoom() && _useTalkAnims)) {
			a->runActorTalkScript(a->_talkStopFrame);
			_useTalkAnims = false;
		}
		if (_game.version <= 7 && _game.heversion == 0)
			setTalkingActor(0xFF);
		if (_game.heversion != 0) {
			((ActorHE *)a)->_heTalking = false;
		}
	}

	if ((_game.id == GID_DIG && !(_game.features & GF_DEMO)) || _game.id == GID_CMI) {
		setTalkingActor(0);
		VAR(VAR_HAVE_MSG) = 0;
	} else if (_game.heversion >= 60) {
		setTalkingActor(0);
	}

	_keepText = false;
	if (_game.version >= 7) {
#ifdef ENABLE_SCUMM_7_8
		((ScummEngine_v7 *)this)->clearSubtitleQueue();
#endif
	} else {
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
		if (_game.platform == Common::kPlatformFMTowns)
			towns_restoreCharsetBg();
		else
#endif
			restoreCharsetBg();
	}
}


#pragma mark -
#pragma mark --- TODO ---
#pragma mark -


void ActorHE::setActorCostume(int c) {
	if (_vm->_game.heversion >= 61 && (c == -1  || c == -2)) {
		_heSkipLimbs = (c == -1);
		_needRedraw = true;
		if (_vm->_game.heversion >= 70) {
			_needBgReset = true;
		}

		return;
	}

	// Based on disassembly. It seems that high byte is not used at all, though
	// it is attached to all horizontally flipped object, like left eye.
	if (_vm->_game.heversion >= 61 && _vm->_game.heversion <= 62)
		c &= 0xff;

	if (_vm->_game.features & GF_NEW_COSTUMES) {
#ifdef ENABLE_HE
		if (_vm->_game.heversion >= 71)
			((ScummEngine_v71he *)_vm)->heQueueEraseAuxActor(this);
#endif
		_auxActor = 0;
		_auxEraseX1 = 0;
		_auxEraseY1 = 0;
		_auxEraseX2 = -1;
		_auxEraseY2 = -1;

		if (_visible) {
			if (_vm->_game.heversion >= 60)
				_needRedraw = true;
		}
	}

	Actor::setActorCostume(c);

	if (_vm->_game.heversion >= 71 && _vm->getTalkingActor() == _number) {
		if (_vm->_game.heversion <= 95 || (_vm->_game.heversion >= 98 && _vm->VAR(_vm->VAR_SKIP_RESET_TALK_ACTOR) == 0)) {
			_vm->setTalkingActor(0);
		}
	}
}

void Actor::setActorCostume(int c) {
	int i;

	_costumeNeedsInit = true;

	if (_vm->_game.features & GF_NEW_COSTUMES) {
		memset(_animVariable, 0, sizeof(_animVariable));

		_costume = c;
		_cost.reset();

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
	if (_vm->_game.version <= 1)
		return;

	if (_vm->_game.features & GF_NEW_COSTUMES) {
		for (i = 0; i < 256; i++)
			_palette[i] = 0xFF;
	} else if (_vm->_game.features & GF_OLD_BUNDLE) {
		for (i = 0; i < 16; i++)
			_palette[i] = i;
	} else {
		for (i = 0; i < 32; i++)
			_palette[i] = 0xFF;
	}

	// Make stuff more visible on CGA. Based on disassembly. It is exactly the same in INDY3, LOOM and MI1 EGA.
	if (_vm->_renderMode == Common::kRenderCGA && _vm->_game.version > 2 && _vm->_game.version < 5) {
		_palette[6] = 5;
		_palette[7] = 15;
	}
}

static const char *const v0ActorNames_English[25] = {
	"Syd",
	"Razor",
	"Dave",
	"Michael",
	"Bernard",
	"Wendy",
	"Jeff",
	"", // Radiation Suit
	"Dr Fred",
	"Nurse Edna",
	"Weird Ed",
	"Dead Cousin Ted",
	"Purple Tentacle",
	"Green Tentacle",
	"", // Meteor Police
	"Meteor",
	"", // Mark Eteer
	"", // Talkshow Host
	"Plant",
	"", // Meteor Radiation
	"", // Edsel (small, outro)
	"", // Meteor (small, intro)
	"Sandy", // (Lab)
	"", // Sandy (Cut-Scene)
};

static const char *const v0ActorNames_German[25] = {
	"Syd",
	"Razor",
	"Dave",
	"Michael",
	"Bernard",
	"Wendy",
	"Jeff",
	"",
	"Dr.Fred",
	"Schwester Edna",
	"Weird Ed",
	"Ted",
	"Lila Tentakel",
	"Gr<nes Tentakel",
	"",
	"Meteor",
	"",
	"",
	"Pflanze",
	"",
	"",
	"",
	"Sandy",
	"",
};

const byte *Actor::getActorName() {
	const byte *ptr = nullptr;

	if (_vm->_game.version == 0) {
		if (_number) {
			switch (_vm->_language) {
			case Common::DE_DEU:
				ptr = (const byte *)v0ActorNames_German[_number - 1];
				break;
			default:
				ptr = (const byte *)v0ActorNames_English[_number - 1];
			}
		}
	} else {
		ptr = _vm->getResourceAddress(rtActorName, _number);
	}

	if (ptr == nullptr) {
		debugC(DEBUG_ACTORS, "Failed to find name of actor %d", _number);
	}
	return ptr;
}

int Actor::getAnimVar(byte var) const {
	assertRange(0, var, 26, "getAnimVar:");
	return _animVariable[var];
}

void Actor::setAnimVar(byte var, int value) {
	assertRange(0, var, 26, "setAnimVar:");
	_animVariable[var] = value;
}

void Actor::remapActorPaletteColor(int color, int new_color) {
	const byte *akos, *akpl;
	int akpl_size, i;
	byte akpl_color;

	akos = _vm->getResourceAddress(rtCostume, _costume);
	if (!akos) {
		debugC(DEBUG_ACTORS, "Actor::remapActorPaletteColor: Can't remap actor %d, costume %d not found", _number, _costume);
		return;
	}

	akpl = _vm->findResourceData(MKTAG('A','K','P','L'), akos);
	if (!akpl) {
		debugC(DEBUG_ACTORS, "Actor::remapActorPaletteColor: Can't remap actor %d, costume %d doesn't contain an AKPL block", _number, _costume);
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
		debugC(DEBUG_ACTORS, "Actor::remapActorPalette: Actor %d not in current room", _number);
		return;
	}

	akos = _vm->getResourceAddress(rtCostume, _costume);
	if (!akos) {
		debugC(DEBUG_ACTORS, "Actor::remapActorPalette: Can't remap actor %d, costume %d not found", _number, _costume);
		return;
	}

	akpl = _vm->findResourceData(MKTAG('A','K','P','L'), akos);
	if (!akpl) {
		debugC(DEBUG_ACTORS, "Actor::remapActorPalette: Can't remap actor %d, costume %d doesn't contain an AKPL block", _number, _costume);
		return;
	}

	// Get the number palette entries
	akpl_size = _vm->getResourceDataSize(akpl);

	rgbs = _vm->findResourceData(MKTAG('R','G','B','S'), akos);

	if (!rgbs) {
		debugC(DEBUG_ACTORS, "Actor::remapActorPalette: Can't remap actor %d costume %d doesn't contain an RGB block", _number, _costume);
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
	return isInClass(kObjectClassPlayer);
}

bool Actor_v2::isPlayer() {
	// isPlayer() is not supported by v0
	assert(_vm->_game.version != 0);
	// MM V1 PC uses VAR_EGO and not VARS 42 / 43. ZAK V1 does already have VARS 42 / 43 here.
	// For MM NES I do not have a disasm and the room I used to test it (MM room 24) also has
	// different box flags in the NES version, so it will not even call into this function.
	// However, I could at least confirm that VARS 42 and 43 are both set to 0, so apparently
	// not in use.
	return (_vm->_game.id == GID_MANIAC && _vm->_game.version == 1) ? (_number == _vm->VAR(_vm->VAR_EGO)) : (_vm->VAR(42) <= _number && _number <= _vm->VAR(43));
}

void ActorHE::setActorEraseType(int eraseValue) {
	if (eraseValue) {
		_generalFlags &= ~ACTOR_GENERAL_FLAG_IGNORE_ERASE;
	} else {
		_generalFlags |= ACTOR_GENERAL_FLAG_IGNORE_ERASE;
	}

	if (_vm->_game.heversion > 99 || _vm->_isHE995) {
		_needBgReset = true;
		_needRedraw = true;
	}
}

void ActorHE::setCondition(int slot, int set) {
	const int condMaskCode = (_vm->_game.heversion >= 85) ? 0x1FFF : 0x3FF;
	assertRange(1, slot, 32, "setCondition: Condition");
	if (set == 0) {
		_heCondMask &= ~(1 << (slot - 1));
	} else {
		_heCondMask |= 1 << (slot - 1);
	}
	if (_heCondMask & condMaskCode) {
		_heCondMask &= ~1;
	} else {
		_heCondMask |= 1;
	}
}

bool ActorHE::isConditionSet(int slot) const {
	assertRange(1, slot, 32, "isConditionSet: Condition");
	return (_heCondMask & (1 << (slot - 1))) != 0;
}

void ActorHE::setUserCondition(int slot, int set) {
	assertRange(1, slot, 16, "setUserCondition: Condition");
	setCondition(slot + 16, set);
}

bool ActorHE::isUserConditionSet(int slot) const {
	assertRange(1, slot, 16, "isUserConditionSet: Condition");
	return isConditionSet(slot + 16);
}

void ActorHE::setTalkCondition(int slot) {
	const int condMaskCode = (_vm->_game.heversion >= 85) ? 0x1FFF : 0x3FF;
	assertRange(1, slot, 16, "setTalkCondition: Condition");
	_heCondMask = (_heCondMask & ~condMaskCode) | 1;
	if (slot != 1) {
		setCondition(slot, 1);
	}
}

bool ActorHE::isTalkConditionSet(int slot) const {
	assertRange(1, slot, 16, "isTalkConditionSet: Condition");
	return isConditionSet(slot);
}

#ifdef ENABLE_HE
void ScummEngine_v71he::heFlushAuxEraseQueue() {
	if (_disableActorDrawingFlag) {
		_heAuxEraseActorIndex = 0;
		return;
	}

	// Erase any AUX frames that were marked to be erased...
	for (int i = 0; i < _heAuxEraseActorIndex; i++) {
		if (_heAuxEraseActorTable[i].y1 <= _heAuxEraseActorTable[i].y2) {
			Common::Rect blitRect(
				_heAuxEraseActorTable[i].x1, _heAuxEraseActorTable[i].y1,
				_heAuxEraseActorTable[i].x2, _heAuxEraseActorTable[i].y2);
			backgroundToForegroundBlit(blitRect);
		}
	}

	_heAuxEraseActorIndex = 0;
}

void ScummEngine_v71he::heFlushAuxQueues() {
	int x, y, w, h, type, whichActor;
	int updateRects, xOffset, yOffset;
	byte *costumeAddress;
	const byte *auxDataBlockPtr;
	const byte *auxDataPtr;
	const byte *auxFrameDataPtr;
	const byte *auxUpdateRectPtr;
	byte *foregroundBufferPtr;
	byte *backgroundBufferPtr;
	const byte *auxEraseRectPtr;
	VirtScreen *pvs = &_virtscr[kMainVirtScreen];

	if (_disableActorDrawingFlag) {
		_heAuxAnimTableIndex = 0;
		return;
	}

	// Render queued animations...
	for (int i = 0; i < _heAuxAnimTableIndex; i++) {
		whichActor = _heAuxAnimTable[i].actor;
		if (whichActor == -1)
			continue;

		ActorHE *a = (ActorHE *)derefActor(whichActor, "heFlushAuxQueues");
		costumeAddress = getResourceAddress(rtCostume, a->_costume);

		xOffset = a->_heOffsX + a->getPos().x - pvs->xstart;
		yOffset = a->_heOffsY + a->getPos().y;

		if (_game.heversion >= 72) {
			yOffset -= a->getElevation();
		}

		auxDataBlockPtr = findResourceData(MKTAG('A', 'K', 'A', 'X'), costumeAddress);
		if (!auxDataBlockPtr) {
			error("heFlushAuxQueue(): NO AKAX block actor %d!", whichActor);
		} else {
			auxDataBlockPtr -= _resourceHeaderSize;
		}

		auxDataPtr = findPalInPals(auxDataBlockPtr, _heAuxAnimTable[i].auxIndex);
		if (!auxDataPtr) {
			error("heFlushAuxQueue(): NO AUXD block actor %d!", whichActor);
		} else {
			auxDataPtr -= _resourceHeaderSize;
		}

		// Check the type of the AUXD block...
		auxFrameDataPtr = findResourceData(MKTAG('A', 'X', 'F', 'D'), auxDataPtr);
		if (!auxFrameDataPtr) {
			warning("heFlushAuxQueue(): NO AXFD block actor %d; ignoring...", whichActor);
			continue;
		}

		type = READ_LE_UINT16(auxFrameDataPtr);

		if ((type == AKOS_AUXD_TYPE_DRLE_FRAME) || (type == AKOS_AUXD_TYPE_SRLE_FRAME)) {
			x = xOffset + (int16)READ_LE_UINT16(auxFrameDataPtr + 2);
			y = yOffset + (int16)READ_LE_UINT16(auxFrameDataPtr + 4);
			w = READ_LE_UINT16(auxFrameDataPtr + 6);
			h = READ_LE_UINT16(auxFrameDataPtr + 8);

			auxFrameDataPtr += 10;

			// Call the render function to go to the main buffer...
			foregroundBufferPtr = pvs->getPixels(0, pvs->topline);
			backgroundBufferPtr = pvs->getBackPixels(0, pvs->topline);

			if (type == AKOS_AUXD_TYPE_SRLE_FRAME) {
				error("heFlushAuxQueue(): Unimplemented compression type actor %d!", whichActor);
			} else if (type == AKOS_AUXD_TYPE_DRLE_FRAME) {
				_wiz->auxDecompDRLEImage(
					(WizRawPixel *)foregroundBufferPtr, (WizRawPixel *)backgroundBufferPtr, auxFrameDataPtr,
					pvs->w, pvs->h, x, y, w, h, nullptr, nullptr);
			} else {
				error("heFlushAuxQueue(): Unimplemented compression type actor %d!", whichActor);
			}
		}

		// Add any update rects to the list for the final blit(s)
		auxUpdateRectPtr = findResourceData(MKTAG('A', 'X', 'U', 'R'), auxDataPtr);
		if (!auxUpdateRectPtr) {
			continue;
		}

		updateRects = READ_LE_UINT16(auxUpdateRectPtr);
		auxUpdateRectPtr += 2;

		for (int rectCounter = 0; rectCounter < updateRects; rectCounter++) {
			markRectAsDirty(
				kMainVirtScreen,
				xOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 0),
				xOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 4),
				yOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 2),
				yOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 6) + 1);

			auxUpdateRectPtr += 8;
		}

		// Set the actors erase info...
		auxEraseRectPtr = findResourceData(MKTAG('A', 'X', 'E', 'R'), auxDataPtr);
		if (!auxEraseRectPtr) {
			continue;
		}

		a->_auxActor = 1;
		a->_auxEraseX1 = xOffset + (int16)READ_LE_UINT16(auxEraseRectPtr + 0);
		a->_auxEraseY1 = yOffset + (int16)READ_LE_UINT16(auxEraseRectPtr + 2);
		a->_auxEraseX2 = xOffset + (int16)READ_LE_UINT16(auxEraseRectPtr + 4);
		a->_auxEraseY2 = yOffset + (int16)READ_LE_UINT16(auxEraseRectPtr + 6);

	}

	_heAuxAnimTableIndex = 0;
}

void ScummEngine_v90he::heFlushAuxQueues() {
	if (_game.heversion < 95) {
		ScummEngine_v71he::heFlushAuxQueues();
		return;
	}

	int x, y, w, h, type, whichActor;
	int updateRects, xOffset, yOffset;
	const byte *auxFrameDataPtr;
	const byte *auxUpdateRectPtr;
	WizRawPixel *foregroundBufferPtr;
	WizRawPixel *backgroundBufferPtr;
	const byte *auxEraseRectPtr;
	const byte *colorTablePtr;
	HEAnimAuxData auxInfo;
	int actorBits;
	const WizRawPixel *conversionTablePtr;
	VirtScreen *pvs = &_virtscr[kMainVirtScreen];

	if (_disableActorDrawingFlag) {
		_heAuxAnimTableIndex = 0;
		return;
	}

	// Render queued animations...
	for (int i = 0; i < _heAuxAnimTableIndex; i++, heAuxReleaseAuxDataInfo(&auxInfo)) {
		actorBits = 0;

		whichActor = _heAuxAnimTable[i].actor;
		if (whichActor == -1)
			continue;

		ActorHE *a = (ActorHE *)derefActor(whichActor, "heFlushAuxQueues");

		if (_game.heversion > 99 && a->_hePaletteNum) {
			conversionTablePtr = (WizRawPixel *)getHEPaletteSlot(a->_hePaletteNum);
		} else {
			conversionTablePtr = (WizRawPixel *)getHEPaletteSlot(1);
		}

		xOffset = a->_heOffsX + a->getPos().x - pvs->xstart;
		yOffset = a->_heOffsY + a->getPos().y;

		if (_game.heversion >= 72) {
			yOffset -= a->getElevation();
		}

		// Get the frame data ptr
		heAuxGetAuxDataInfo(&auxInfo, whichActor, _heAuxAnimTable[i].auxIndex);

		// Check the type of the AUXD block...
		auxFrameDataPtr = heAuxFindBlock(&auxInfo, MKTAG('A', 'X', 'F', 'D'));
		if (!auxFrameDataPtr) {
			warning("heFlushAuxQueue(): NO AXFD block actor %d; ignoring...", whichActor);
			continue;
		}

		type = READ_LE_UINT16(auxFrameDataPtr);

		if ((type == AKOS_AUXD_TYPE_DRLE_FRAME) ||
			(type == AKOS_AUXD_TYPE_SRLE_FRAME) ||
			(type == AKOS_AUXD_TYPE_WRLE_FRAME)) {
			x = xOffset + (int16)READ_LE_UINT16(auxFrameDataPtr + 2);
			y = yOffset + (int16)READ_LE_UINT16(auxFrameDataPtr + 4);
			w = READ_LE_UINT16(auxFrameDataPtr + 6);
			h = READ_LE_UINT16(auxFrameDataPtr + 8);

			auxFrameDataPtr += 10;

			// Call the render function to go to the main buffer...
			foregroundBufferPtr = (WizRawPixel *)pvs->getPixels(0, pvs->topline);
			backgroundBufferPtr = (WizRawPixel *)pvs->getBackPixels(0, pvs->topline);

			if (type == AKOS_AUXD_TYPE_SRLE_FRAME) {
				colorTablePtr = heAuxFindBlock(&auxInfo, MKTAG('C', 'L', 'R', 'S'));
				if (!colorTablePtr) {
					error("heFlushAuxQueue(): NO CLRS block actor %d!", whichActor);
				}

				colorTablePtr += _resourceHeaderSize;

				if ((x != 0) || (y != 0) || (w != 640) || (h != 480)) {
					error("heFlushAuxQueue(): Actor %d invalid (%d,%d)[%d,%d]", whichActor, x, y, w, h);
				}

				_wiz->auxDecompSRLEStream(
					foregroundBufferPtr, backgroundBufferPtr, colorTablePtr,
					auxFrameDataPtr, w * h,
					conversionTablePtr);
			} else if (type == AKOS_AUXD_TYPE_DRLE_FRAME) {
				_wiz->auxDecompDRLEImage(
					(WizRawPixel *)foregroundBufferPtr, (WizRawPixel *)backgroundBufferPtr, auxFrameDataPtr,
					pvs->w, pvs->h, x, y, w, h, nullptr, conversionTablePtr);
			} else if (AKOS_AUXD_TYPE_WRLE_FRAME == type) {
				if ((x != 0) || (w != 640)) {
					error("heFlushAuxQueue(): Actor %d invalid (%d,%d)[%d,%d]", whichActor, x, y, w, h);
				}

				// Where is the color table?
				colorTablePtr = auxFrameDataPtr;
				auxFrameDataPtr += 32;

				// Handle the uncompress
				_wiz->auxWRLEUncompressPixelStream(
					foregroundBufferPtr + (y * 640),
					colorTablePtr, auxFrameDataPtr, (w * h),
					conversionTablePtr);

				actorBits = a->_number;
				a->_needRedraw = true;

			} else {
				error("heFlushAuxQueue(): Unimplemented compression type actor %d!", whichActor);
			}
		}

		// Add any update rects to the list for the final blit(s)
		auxUpdateRectPtr = heAuxFindBlock(&auxInfo, MKTAG('A', 'X', 'U', 'R'));
		if (!auxUpdateRectPtr) {
			continue;
		}

		updateRects = READ_LE_UINT16(auxUpdateRectPtr);
		auxUpdateRectPtr += 2;

		for (int rectCounter = 0; rectCounter < updateRects; rectCounter++) {
			markRectAsDirty(
				kMainVirtScreen,
				xOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 0),
				xOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 4),
				yOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 2),
				yOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 6) + 1,
				actorBits);

			a->setActorUpdateArea(
				xOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 0),
				yOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 2),
				xOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 4),
				yOffset + (int16)READ_LE_UINT16(auxUpdateRectPtr + 6));

			auxUpdateRectPtr += 8;
		}

		// Set the actors erase info...
		auxEraseRectPtr = heAuxFindBlock(&auxInfo, MKTAG('A', 'X', 'E', 'R'));
		if (!auxEraseRectPtr) {
			continue;
		}

		a->_auxActor = 1;
		a->_auxEraseX1 = xOffset + (int16)READ_LE_UINT16(auxEraseRectPtr + 0);
		a->_auxEraseY1 = yOffset + (int16)READ_LE_UINT16(auxEraseRectPtr + 2);
		a->_auxEraseX2 = xOffset + (int16)READ_LE_UINT16(auxEraseRectPtr + 4);
		a->_auxEraseY2 = yOffset + (int16)READ_LE_UINT16(auxEraseRectPtr + 6);
	}

	_heAuxAnimTableIndex = 0;
}

const byte *ScummEngine_v90he::heAuxFindBlock(HEAnimAuxData *auxInfoPtr, int32 id) {
	const byte *resultPtr;

	// Search the external block thing
	if (auxInfoPtr->externalDataPtr) {
		resultPtr = findResourceData(id, auxInfoPtr->externalDataPtr);
		if (resultPtr)
			return resultPtr;
	}

	// Search the current block first
	resultPtr = findResourceData(id, auxInfoPtr->auxDataBlock);
	if (resultPtr)
		return resultPtr;

	// If the alt search isn't the same search there...
	if (auxInfoPtr->auxDataBlock == auxInfoPtr->auxDefaultSearchBlock) {
		return resultPtr;
	}

	// Search the default block
	return findResourceData(id, auxInfoPtr->auxDefaultSearchBlock);
}

void ScummEngine_v90he::heAuxReleaseAuxDataInfo(HEAnimAuxData *auxInfoPtr) {
	auxInfoPtr->auxDefaultSearchBlock = nullptr;
	auxInfoPtr->auxDataBlock = nullptr;

	if (auxInfoPtr->externalDataPtr) {
		free(auxInfoPtr->externalDataPtr);
		auxInfoPtr->externalDataPtr = nullptr;
	}
}

bool ScummEngine_v90he::heAuxProcessFileRelativeBlock(HEAnimAuxData *auxInfoPtr, const byte *dataBlockPtr) {
	error("heAuxProcessFileRelativeBlock(): This looks like a development path! If you end up here, please report it in our bug tracker!");
}

bool ScummEngine_v90he::heAuxProcessDisplacedBlock(HEAnimAuxData *auxInfoPtr, const byte *displacedBlockPtr) {
	error("heAuxProcessDisplacedBlock(): This looks like a development path! If you end up here, please report it in our bug tracker!");
}

void ScummEngine_v90he::heAuxGetAuxDataInfo(HEAnimAuxData *auxInfoPtr, int whichActor, int auxIndex) {
	const byte *fileRelativeDataBlockPtr;
	const byte *displacedBlockPtr;
	const byte *auxDataBlockPtr;
	const byte *auxDataPtr;
	byte *costumeAddress;

	// Store off some of the passed in info
	auxInfoPtr->externalDataPtr = nullptr;
	auxInfoPtr->actor = whichActor;

	// Get the interesting data
	ActorHE *a = (ActorHE *)derefActor(whichActor, "heAuxGetAuxDataInfo");
	costumeAddress = getResourceAddress(rtCostume, a->_costume);

	auxDataBlockPtr = findResourceData(MKTAG('A', 'K', 'A', 'X'), costumeAddress);
	if (!auxDataBlockPtr) {
		error("heAuxGetAuxDataInfo(): NO AKAX block actor %d!", whichActor);
	} else {
		auxDataBlockPtr -= _resourceHeaderSize;
	}

	auxDataPtr = findPalInPals(auxDataBlockPtr, auxIndex);
	if (!auxDataPtr) {
		error("heAuxGetAuxDataInfo(): NO AUXD block actor %d!", whichActor);
	} else {
		auxDataPtr -= _resourceHeaderSize;
	}

	// Check for other outside block types
	fileRelativeDataBlockPtr = findResourceData(MKTAG('F', 'R', 'E', 'L'), auxDataPtr);

	if (fileRelativeDataBlockPtr) {
		fileRelativeDataBlockPtr -= _resourceHeaderSize;
		if (!heAuxProcessFileRelativeBlock(auxInfoPtr, fileRelativeDataBlockPtr)) {
			error("heAuxGetAuxDataInfo(): Actor %d aux %d failed", whichActor, auxIndex);
		}
	}

	// This is where the DISP block will be processed!
	displacedBlockPtr = findResourceData(MKTAG('D', 'I', 'S', 'P'), auxDataPtr) ;

	if (displacedBlockPtr) {
		displacedBlockPtr -= _resourceHeaderSize;
		if (!heAuxProcessDisplacedBlock(auxInfoPtr, displacedBlockPtr)) {
			error("heAuxGetAuxDataInfo(): Actor %d aux %d failed", whichActor, auxIndex);
		}
	}

	// Fill in the data result
	auxInfoPtr->auxDefaultSearchBlock = costumeAddress;
	auxInfoPtr->auxDataBlock = auxDataPtr;
}

void ScummEngine_v71he::heQueueEraseAuxActor(ActorHE *a) {
	if (_heAuxEraseActorIndex >= ARRAYSIZE(_heAuxEraseActorTable)) {
		warning("heQueueEraseAuxActor(): Queue full, ignoring...");
		return;
	}

	if (a->_auxActor) {
		_heAuxEraseActorTable[_heAuxEraseActorIndex].actor = a->_number;
		_heAuxEraseActorTable[_heAuxEraseActorIndex].x1 = a->_auxEraseX1;
		_heAuxEraseActorTable[_heAuxEraseActorIndex].y1 = a->_auxEraseY1;
		_heAuxEraseActorTable[_heAuxEraseActorIndex].x2 = a->_auxEraseX2;
		_heAuxEraseActorTable[_heAuxEraseActorIndex].y2 = a->_auxEraseY2;
		_heAuxEraseActorIndex++;
	}
}

void ScummEngine_v71he::heQueueAnimAuxFrame(int actor, int auxIndex) {
	if (_heAuxAnimTableIndex >= ARRAYSIZE(_heAuxAnimTable)) {
		warning("HEQueueAnimAuxFrame(): Queue full, ignoring...");
		return;
	}

	_heAuxAnimTable[_heAuxAnimTableIndex].actor = actor;
	_heAuxAnimTable[_heAuxAnimTableIndex].auxIndex = auxIndex;
	_heAuxAnimTableIndex++;
}

#endif

void Actor_v0::animateActor(int anim) {
	int dir = -1;

	switch (anim) {
		case 0x00:
		case 0x04:
			dir = 0;
			break;

		case 0x01:
		case 0x05:
			dir = 1;
			break;

		case 0x02:
		case 0x06:
			dir = 2;
			break;

		case 0x03:
		case 0x07:
			dir = 3;
			break;

		default:
			break;
	}

	if (isInCurrentRoom()) {

		_costCommandNew = anim;
		_vm->_costumeLoader->costumeDecodeData(this, 0, 0);

		if (dir == -1)
			return;

		_facing = normalizeAngle(0, oldDirToNewDir(dir));

	} else {

		if (anim >= 4 && anim <= 7)
			_facing = normalizeAngle(0, oldDirToNewDir(dir));
	}
}

byte Actor_v0::updateWalkbox() {
	if (_vm->checkXYInBoxBounds(_walkbox, _pos.x, _pos.y))
		return 0;

	int numBoxes = _vm->getNumBoxes() - 1;
	for (int i = 0; i <= numBoxes; i++) {
		if (_vm->checkXYInBoxBounds(i, _pos.x, _pos.y) == true) {
			if (_walkdata.curbox == i) {
				setBox(i);
				directionUpdate();

				_newWalkBoxEntered = true;
				return i;
			}
		}
	}

	return kInvalidBox;
}

void Actor_v0::directionUpdate() {

	int nextFacing = updateActorDirection(true);
	if (_facing != nextFacing) {
		// 2A89
		setDirection(nextFacing);

		// Still need to turn?
		if (_facing != _targetFacing) {
			_moving |= 0x80;
			return;
		}
	}

	_moving &= ~0x80;
}

void Actor_v0::setActorToTempPosition() {
	_tmp_Pos = _pos;
	_pos = _tmp_NewPos;
	_tmp_WalkBox = _walkbox;
	_tmp_NewWalkBoxEntered = _newWalkBoxEntered;
}

void Actor_v0::setActorToOriginalPosition() {
	_pos = _tmp_Pos;
	_tmp_NewPos = _tmp_Pos;
	_walkbox = _tmp_WalkBox;
	_newWalkBoxEntered = _tmp_NewWalkBoxEntered;
}

void Actor_v0::actorSetWalkTo() {

	if (_newWalkBoxEntered == false)
		return;

	_newWalkBoxEntered = false;

	int nextBox = ((ScummEngine_v0 *)_vm)->walkboxFindTarget(this, _walkdata.destbox, _walkdata.dest);
	if (nextBox != kInvalidBox) {
		_walkdata.curbox = nextBox;
	}
}

void ScummEngine_v60he::setActorClippingRect(int actor, int x1, int y1, int x2, int y2) {
	if (actor == -1) {
		_defaultActorClipping.left = x1;
		_defaultActorClipping.top = y1;
		_defaultActorClipping.right = x2;
		_defaultActorClipping.bottom = y2;
	} else {
		ActorHE *a = (ActorHE *)derefActor(actor, "setActorClippingRect");
		if (x1 == -1 && y1 == -1 && x2 == -1 && y2 == -1) {
			a->_clipOverride.left = _defaultActorClipping.left;
			a->_clipOverride.top = _defaultActorClipping.top;
			a->_clipOverride.right = _defaultActorClipping.right;
			a->_clipOverride.bottom = _defaultActorClipping.bottom;
		} else {
			a->_clipOverride.left = x1;
			a->_clipOverride.top = y1;
			a->_clipOverride.right = x2;
			a->_clipOverride.bottom = y2;
		}
	}
}

void Actor_v0::saveLoadWithSerializer(Common::Serializer &s) {
	Actor::saveLoadWithSerializer(s);

	s.syncAsByte(_costCommand, VER(84));
	s.skip(1, VER(84), VER(89)); // _costFrame
	s.syncAsByte(_miscflags, VER(84));
	s.syncAsByte(_speaking, VER(84));
	s.skip(1, VER(84), VER(89)); // _speakingPrev
	s.skip(1, VER(89), VER(89)); // _limbTemp
	s.syncAsByte(_animFrameRepeat, VER(89));
	s.syncArray(_limbFrameRepeatNew, 8, Common::Serializer::SByte, VER(89));
	s.syncArray(_limbFrameRepeat, 8, Common::Serializer::SByte, VER(90));
	s.syncAsSint16LE(_CurrentWalkTo.x, VER(97));
	s.syncAsSint16LE(_CurrentWalkTo.y, VER(97));
	s.syncAsSint16LE(_NewWalkTo.x, VER(97));
	s.syncAsSint16LE(_NewWalkTo.y, VER(97));
	s.syncAsSByte(_walkCountModulo, VER(97));
	s.syncAsByte(_newWalkBoxEntered, VER(97));
	s.syncAsByte(_walkDirX, VER(97));
	s.syncAsByte(_walkDirY, VER(97));
	s.syncAsByte(_walkYCountGreaterThanXCount, VER(97));
	s.syncAsByte(_walkXCount, VER(97));
	s.syncAsByte(_walkXCountInc, VER(97));
	s.syncAsByte(_walkYCount, VER(97));
	s.syncAsByte(_walkYCountInc, VER(97));
	s.syncAsByte(_walkMaxXYCountInc, VER(97));

	s.syncBytes(_walkboxQueue, 16, VER(98));
	s.syncAsByte(_walkboxQueueIndex, VER(98));

	// When loading, we need to ensure the limbs are restarted
	if (s.isLoading()) {

		// valid costume command?
		if (_costCommand != 0xFF) {

			// Do we have a walkbox queue?
			if (_walkboxQueueIndex < 1) {
				_costCommand = 0xFF;

				// Standing Still
				setDirection(_facing);
				speakCheck();

			} else {
				// Force limb direction update
				_facing = 0;
				directionUpdate();

				// Begin walking
				animateActor(newDirToOldDir(_facing));
			}
		}
	}
}

void Actor::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.isLoading()) {
		// Not all actor data is saved; so when loading, we first reset
		// the actor, to ensure completely reproducible behavior (else,
		// some not saved value in the actor class can cause odd things)
		initActor(-1);
	}

	s.syncAsSint16LE(_pos.x, VER(8));
	s.syncAsSint16LE(_pos.y, VER(8));
	s.syncAsSint16LE(_heOffsX, VER(32));
	s.syncAsSint16LE(_heOffsY, VER(32));
	s.syncAsSint16LE(_top, VER(8));
	s.syncAsSint16LE(_bottom, VER(8));
	s.syncAsSint16LE(_elevation, VER(8));
	s.syncAsUint16LE(_width, VER(8));
	s.syncAsUint16LE(_facing, VER(8));
	s.syncAsUint16LE(_costume, VER(8));
	s.syncAsByte(_room, VER(8));
	s.syncAsByte(_talkColor, VER(8));
	s.syncAsSint16LE(_talkFrequency, VER(16));
	s.syncAsSint16LE(_talkPan, VER(24));
	s.syncAsSint16LE(_talkVolume, VER(29));
	s.syncAsUint16LE(_boxscale, VER(34));
	s.syncAsByte(_scalex, VER(8));
	s.syncAsByte(_scaley, VER(8));
	s.syncAsByte(_charset, VER(8));

	// Actor sound grew from 8 to 32 bytes and switched to uint16 in HE games
	s.syncArray(_sound, 8, Common::Serializer::Byte, VER(8), VER(36));
	s.syncArray(_sound, 32, Common::Serializer::Byte, VER(37), VER(61));
	s.syncArray(_sound, 32, Common::Serializer::Uint16LE, VER(62));

	// Actor animVariable grew from 8 to 27
	s.syncArray(_animVariable, 8, Common::Serializer::Uint16LE, VER(8), VER(40));
	s.syncArray(_animVariable, 27, Common::Serializer::Uint16LE, VER(41));

	s.syncAsUint16LE(_targetFacing, VER(8));
	s.syncAsByte(_moving, VER(8));
	s.syncAsByte(_ignoreBoxes, VER(8));
	s.syncAsByte(_forceClip, VER(8));
	s.syncAsByte(_initFrame, VER(8));
	s.syncAsByte(_walkFrame, VER(8));
	s.syncAsByte(_standFrame, VER(8));
	s.syncAsByte(_talkStartFrame, VER(8));
	s.syncAsByte(_talkStopFrame, VER(8));
	s.syncAsUint16LE(_speedx, VER(8));
	s.syncAsUint16LE(_speedy, VER(8));
	s.syncAsUint16LE(_cost.animCounter, VER(8));
	s.syncAsByte(_cost.soundCounter, VER(8));
	s.syncAsByte(_drawToBackBuf, VER(32));
	s.syncAsByte(_flip, VER(32));
	s.syncAsByte(_heSkipLimbs, VER(32));

	// Actor palette grew from 64 to 256 bytes and switched to uint16 in HE games
	s.syncArray(_palette, 64, Common::Serializer::Byte, VER(8), VER(9));
	s.syncArray(_palette, 256, Common::Serializer::Byte, VER(10), VER(79));
	s.syncArray(_palette, 256, Common::Serializer::Uint16LE, VER(80));

	s.skip(1, VER(8), VER(9)); // _mask
	s.syncAsByte(_shadowMode, VER(8));
	s.syncAsByte(_visible, VER(8));
	s.syncAsByte(_frame, VER(8));
	s.syncAsByte(_animSpeed, VER(8));
	s.syncAsByte(_animProgress, VER(8));
	s.syncAsByte(_walkbox, VER(8));
	s.syncAsByte(_needRedraw, VER(8));
	s.syncAsByte(_needBgReset, VER(8));
	s.syncAsByte(_costumeNeedsInit, VER(8));
	s.syncAsUint32LE(_heCondMask, VER(38));
	s.syncAsUint32LE(_hePaletteNum, VER(59));
	s.syncAsUint32LE(_heShadow, VER(59));

	s.syncAsSint16LE(_talkPosY, VER(8));
	s.syncAsSint16LE(_talkPosX, VER(8));
	s.syncAsByte(_ignoreTurns, VER(8));

	// Actor layer switched to int32 in HE games
	s.syncAsByte(_layer, VER(8), VER(57));
	s.syncAsSint32LE(_layer, VER(58));

	s.syncAsUint16LE(_talkScript, VER(8));
	s.syncAsUint16LE(_walkScript, VER(8));

	s.syncAsSint16LE(_walkdata.dest.x, VER(8));
	s.syncAsSint16LE(_walkdata.dest.y, VER(8));
	s.syncAsByte(_walkdata.destbox, VER(8));
	s.syncAsUint16LE(_walkdata.destdir, VER(8));
	s.syncAsByte(_walkdata.curbox, VER(8));
	s.syncAsSint16LE(_walkdata.cur.x, VER(8));
	s.syncAsSint16LE(_walkdata.cur.y, VER(8));
	s.syncAsSint16LE(_walkdata.next.x, VER(8));
	s.syncAsSint16LE(_walkdata.next.y, VER(8));
	s.syncAsSint32LE(_walkdata.deltaXFactor, VER(8));
	s.syncAsSint32LE(_walkdata.deltaYFactor, VER(8));
	s.syncAsUint16LE(_walkdata.xfrac, VER(8));
	s.syncAsUint16LE(_walkdata.yfrac, VER(8));
	s.syncAsSint16LE(_walkdata.facing, VER(111));

	s.syncAsUint16LE(_walkdata.point3.x, VER(42));
	s.syncAsUint16LE(_walkdata.point3.y, VER(42));

	s.syncBytes(_cost.animType, 16, VER(8));
	s.syncAsUint16LE(_cost.stopped, VER(8));
	s.syncArray(_cost.curpos, 16, Common::Serializer::Uint16LE, VER(8));
	s.syncArray(_cost.start, 16, Common::Serializer::Uint16LE, VER(8));
	s.syncArray(_cost.end, 16, Common::Serializer::Uint16LE, VER(8));
	s.syncArray(_cost.frame, 16, Common::Serializer::Uint16LE, VER(8));

	s.syncArray(_cost.heJumpOffsetTable, 16, Common::Serializer::Uint16LE, VER(65));
	s.syncArray(_cost.heJumpCountTable, 16, Common::Serializer::Uint16LE, VER(65));
	s.syncArray(_cost.heCondMaskTable, 16, Common::Serializer::Uint32LE, VER(65));

	if (s.isLoading() && _vm->_game.version <= 2 && s.getVersion() < VER(70)) {
		_pos.x >>= V12_X_SHIFT;
		_pos.y >>= V12_Y_SHIFT;

		_speedx >>= V12_X_SHIFT;
		_speedy >>= V12_Y_SHIFT;
		_elevation >>= V12_Y_SHIFT;

		if (_walkdata.dest.x != -1) {
			_walkdata.dest.x >>= V12_X_SHIFT;
			_walkdata.dest.y >>= V12_Y_SHIFT;
		}

		_walkdata.cur.x >>= V12_X_SHIFT;
		_walkdata.cur.y >>= V12_Y_SHIFT;

		_walkdata.next.x >>= V12_X_SHIFT;
		_walkdata.next.y >>= V12_Y_SHIFT;

		if (_walkdata.point3.x != 32000) {
			_walkdata.point3.x >>= V12_X_SHIFT;
			_walkdata.point3.y >>= V12_Y_SHIFT;
		}

		setDirection(_facing);
	}

	if (s.isLoading() && _vm->_game.version > 0 && !(_vm->_game.features & GF_NEW_COSTUMES) && s.getVersion() < VER(105)) {
		// For older saves, we can't reconstruct the frame's direction if it is different from the actor
		// direction, this is the best we can do. However, it seems to be relevant only for very rare
		// edge cases, anyway...
		for (int i = 0; i < 16; ++i) {
			if (_cost.frame[i] != 0xffff)
				_cost.frame[i] = (_cost.frame[i] << 2) | newDirToOldDir(_facing);
		}
	}

	// WORKAROUND: Post-load actor palette fixes for games that were saved with a different render mode
	// (concerns INDY3, LOOM and MI1EGA). The original interpreter does not fix this, savegames from
	// different videomodes will cause glitches there.
	if (s.isLoading() && (_vm->_game.version == 3 || _vm->_game.id == GID_MONKEY_EGA) && _vm->_game.platform == Common::kPlatformDOS) {
		// Loom is not really much of a problem here, since it has extensive scripted post-load
		// treatment in ScummEngine_v3::scummLoop_handleSaveLoad(). But there are situations
		// where it won't be triggered (basically savegames from places where the original does
		// not allow saving).  Indy3 is more dependant on this than Loom, since it does have much
		// less scripted post-load magic of its own. Monkey Island always needs this, since V4+
		// games don't do scripted loading of savegames (scripted post-load things) at all.
		bool cga = (_vm->_renderMode == Common::kRenderCGA);
		if (cga && _vm->_game.id == GID_MONKEY_EGA && _palette[6] == 0xFF && _palette[7] == 0xFF) {
			_palette[6] = 5;
			_palette[7] = 15;
		} else if ((cga && _palette[6] == 6 && _palette[7] == 7) || (!cga && _palette[6] == 5 && _palette[7] == 15)) {
			_palette[6] ^= 3;
			_palette[7] ^= 8;
		}
		// Extra fix for Bobbin in his normal costume.
		if (_vm->_game.id == GID_LOOM && _number == 1 && ((cga && _palette[8] == 8) || (!cga && _palette[8] == 0)))
			_palette[8] ^= 8;
	}
}

void Actor_v3::saveLoadWithSerializer(Common::Serializer &s) {
	Actor::saveLoadWithSerializer(s);

	int rev = (_vm->_game.version == 3 ? 101 : 102);

	if (s.isLoading() && s.getVersion() < VER(rev)) {
		int diffX = _walkdata.next.x - _pos.x;
		int diffY = _walkdata.next.y - _pos.y;

		if (_vm->_game.version < 3) {
			_stepThreshold = MAX(ABS(diffX), ABS(diffY));
			_walkdata.deltaXFactor = _walkdata.deltaYFactor = 1;
		} else {
			_stepX = ((ABS(diffY) / (int)_speedy) >> 1) >(ABS(diffX) / (int)_speedx) ? _speedy + 1 : _speedx;
			_stepThreshold = MAX(ABS(diffY) / _speedy, ABS(diffX) / _stepX);
			_walkdata.deltaXFactor = (int32)_stepX;
			_walkdata.deltaYFactor = (int32)_speedy;
		}

		if (diffX < 0)
			_walkdata.deltaXFactor = -_walkdata.deltaXFactor;
		if (diffY < 0)
			_walkdata.deltaYFactor = -_walkdata.deltaYFactor;
		_walkdata.xfrac = _walkdata.xAdd = _walkdata.deltaXFactor ? diffX / _walkdata.deltaXFactor : 0;
		_walkdata.yfrac = _walkdata.yAdd = _walkdata.deltaYFactor ? diffY / _walkdata.deltaYFactor : 0;

	} else {
		s.syncAsUint16LE(_walkdata.xAdd, VER(rev));
		s.syncAsUint16LE(_walkdata.yAdd, VER(rev));
		s.syncAsUint16LE(_stepX, VER(rev));
		s.syncAsUint16LE(_stepThreshold, VER(rev));
	}
}

} // End of namespace Scumm
