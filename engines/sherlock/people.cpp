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
 */

#include "sherlock/people.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

// Walk speeds
#define MWALK_SPEED 2
#define XWALK_SPEED 4
#define YWALK_SPEED 1

// Characer animation sequences
static const uint8 CHARACTER_SEQUENCES[MAX_HOLMES_SEQUENCE][MAX_FRAME] = {
	{ 29, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Right
	{ 22, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Down 
	{ 29, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Left
	{ 15, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Up
	{ 42, 1, 2, 3, 4, 5, 0 },			// Goto Stand Right
	{ 47, 1, 2, 3, 4, 5, 0 },			// Goto Stand Down
	{ 42, 1, 2, 3, 4, 5, 0 },			// Goto Stand Left
	{ 36, 1, 0 },						// Goto Stand Up
	{ 8, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Up Right
	{ 1, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Down Right
	{ 8, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Up Left
	{ 1, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Down Left
	{ 37, 1, 2, 3, 4, 5, 0 },			// Goto Stand Up Right
	{ 37, 1, 2, 3, 4, 5, 0 },			// Goto Stand Up Left
	{ 52, 1, 2, 3, 4, 0 },				// Goto Stand Down Right
	{ 52, 1, 2, 3, 4, 0 }				// Goto Stand Down Left
};


People::People(SherlockEngine *vm) : _vm(vm), _player(_data[0]) {
	_walkLoaded = false;
	_holmesOn = true;
	_oldWalkSequence = -1;
	_allowWalkAbort = false;
	_portraitLoaded = false;
	_clearingThePortrait = false;
}

People::~People() {
	if (_walkLoaded)
		delete _data[PLAYER]._images;
}

void People::reset() {
	Sprite &p = _data[PLAYER];

	p._description = "Sherlock Holmes!";
	p._type = CHARACTER;
	p._position = Common::Point(10000, 11000);
	p._sequenceNumber = STOP_DOWNRIGHT;
	p._sequences = &CHARACTER_SEQUENCES;
	p._imageFrame = nullptr;
	p._frameNumber = 1;
	p._delta = Common::Point(0, 0);
	p._oldPosition = Common::Point(0, 0);
	p._oldSize = Common::Point(0, 0);
	p._misc = 0;
	p._walkCount = 0;
	p._pickUp = "";
	p._allow = 0;
	p._noShapeSize = Common::Point(0, 0);
	p._goto = Common::Point(0, 0);
	p._status = 0;
}

bool People::loadWalk() {
	if (_walkLoaded) {
		return false;
	} else {
		_data[PLAYER]._images = new ImageFile("walk.vgs");
		_data[PLAYER].setImageFrame();
		_walkLoaded = true;

		return true;
	}
}

/**
 * If the walk data has been loaded, then it will be freed
 */
bool People::freeWalk() {
	if (_walkLoaded) {
		delete _player._images;
		_player._images = nullptr;

		_walkLoaded = false;
		return true;
	} else {
		return false;
	}
}

/**
* Set the variables for moving a character from one poisition to another
* in a straight line - goAllTheWay must have been previously called to
* check for any obstacles in the path.
*/
void People::setWalking() {
	Scene &scene = *_vm->_scene;
	int oldDirection, oldFrame;
	Common::Point speed, delta;
	int temp;

	// Flag that player has now walked in the scene
	scene._walkedInScene = true;

	// Stop any previous walking, since a new dest is being set
	_player._walkCount = 0;
	oldDirection = _player._sequenceNumber;
	oldFrame = _player._frameNumber;

	// Set speed to use horizontal and vertical movement
	if (_vm->_onChessboard) {
		speed = Common::Point(MWALK_SPEED, MWALK_SPEED);
	} else {
		speed = Common::Point(XWALK_SPEED, YWALK_SPEED);
	}

	// If the player is already close to the given destination that no
	// walking is needed, move to the next straight line segment in the
	// overall walking route, if there is one
	for (;;) {
		// Since we want the player to be centered on the destination they
		// clicked, but characters draw positions start at their left, move
		// the destination half the character width to draw him centered
		if (_walkDest.x >= (temp = _player._imageFrame->_frame.w / 2))
			_walkDest.x -= temp;

		delta = Common::Point(
			ABS(_player._position.x / 100 - _walkDest.x),
			ABS(_player._position.y / 100 - _walkDest.y)
		);

		// If we're ready to move a sufficient distance, that's it. Otherwise,
		// move onto the next portion of the walk path, if there is one
		if ((delta.x > 3 || delta.y > 0) || _walkTo.empty())
			break;

		// Pop next walk segment off the walk route stack
		_walkDest = _walkTo.pop();
	} while (!_vm->shouldQuit());

	// If a sufficient move is being done, then start the move
	if (delta.x > 3 || delta.y) {
		// See whether the major movement is horizontal or vertical
		if (delta.x >= delta.y) {
			// Set the initial frame sequence for the left and right, as well
			// as settting the delta x depending on direction
			if (_walkDest.x < (_player._position.x / 100)) {
				_player._sequenceNumber = _vm->_onChessboard ? MAP_LEFT : WALK_LEFT;
				_player._delta.x = speed.x * -100;
			} else {
				_player._sequenceNumber = _vm->_onChessboard ? MAP_RIGHT : WALK_RIGHT;
				_player._delta.x = speed.x * 100;
			}

			// See if the x delta is too small to be divided by the speed, since
			// this would cause a divide by zero error
			if (delta.x >= speed.x) {
				// Det the delta y
				_player._delta.y = (delta.y * 100) / (delta.x / speed.x);
				if (_walkDest.y < (_player._position.y / 100))
					_player._delta.y = -_player._delta.y;

				// Set how many times we should add the delta to the player's position
				_player._walkCount = delta.x / speed.x;
			} else {
				// The delta x was less than the speed (ie. we're really close to
				// the destination). So set delta to 0 so the player won't move
				_player._delta = Common::Point(0, 0);
				_player._position = Common::Point(_walkDest.x * 100, _walkDest.y * 100);
				_player._walkCount = 1;
			}

			// See if the sequence needs to be changed for diagonal walking
			if (_player._delta.y > 150) {
				if (!_vm->_onChessboard) {
					switch (_player._sequenceNumber) {
					case WALK_LEFT:
						_player._sequenceNumber = WALK_DOWNLEFT;
						break;
					case WALK_RIGHT:
						_player._sequenceNumber = WALK_DOWNRIGHT;
						break;
					}
				}
			} else if (_player._delta.y < -150) {
				if (!_vm->_onChessboard) {
					switch (_player._sequenceNumber) {
					case WALK_LEFT:
						_player._sequenceNumber = WALK_UPLEFT;
						break;
					case WALK_RIGHT:
						_player._sequenceNumber = WALK_UPRIGHT;
						break;
					}
				}
			}
		} else {
			// Major movement is vertical, so set the sequence for up and down,
			// and set the delta Y depending on the direction
			if (_walkDest.y < (_player._position.y / 100)) {
				_player._sequenceNumber = WALK_UP;
				_player._delta.y = speed.y * -100;
			} else {
				_player._sequenceNumber = WALK_DOWN;
				_player._delta.y = speed.y * 100;
			}

			// If we're on the overhead map, set the sequence so we keep moving
			// in the same direction
			_player._sequenceNumber = (oldDirection == -1) ? MAP_RIGHT : oldDirection;

			// Set the delta x
			_player._delta.x = (delta.x * 100) / (delta.y / speed.y);
			if (_walkDest.x < (_player._position.x / 100))
				_player._delta.x = -_player._delta.x;
		}
	}

	// See if the new walk sequence is the same as the old. If it's a new one,
	// we need to reset the frame number to zero so it's animation starts at
	// it's beginning. Otherwise, if it's the same sequence, we can leave it
	// as is, so it keeps the animation going at wherever it was up to
	if (_player._sequenceNumber != _oldWalkSequence)
		_player._frameNumber = 0;
	_oldWalkSequence = _player._sequenceNumber;

	if (!_player._walkCount)
		gotoStand(_player);

	// If the sequence is the same as when we started, then Holmes was 
	// standing still and we're trying to re-stand him, so reset Holmes'
	// rame to the old frame number from before it was reset to 0
	if (_player._sequenceNumber == oldDirection)
		_player._frameNumber = oldFrame;
}

/**
 * Bring a moving character to a standing position. If the Scalpel chessboard 
 * is being displayed, then the chraracter will always face down.
 */
void People::gotoStand(Sprite &sprite) {
	Scene &scene = *_vm->_scene;
	_walkTo.clear();
	sprite._walkCount = 0;

	switch (sprite._sequenceNumber) {
	case WALK_UP:
		sprite._sequenceNumber = STOP_UP; break;
	case WALK_DOWN:
		sprite._sequenceNumber = STOP_DOWN; break;
	case TALK_LEFT:
	case WALK_LEFT:
		sprite._sequenceNumber = STOP_LEFT; break;
	case TALK_RIGHT:
	case WALK_RIGHT:
		sprite._sequenceNumber = STOP_RIGHT; break;
	case WALK_UPRIGHT:
		sprite._sequenceNumber = STOP_UPRIGHT; break;
	case WALK_UPLEFT:
		sprite._sequenceNumber = STOP_UPLEFT; break;
	case WALK_DOWNRIGHT:
		sprite._sequenceNumber = STOP_DOWNRIGHT; break;
	case WALK_DOWNLEFT:
		sprite._sequenceNumber = STOP_DOWNLEFT; break;
	default:
		break;
	}

	// Only restart frame at 0 if the sequence number has changed
	if (_oldWalkSequence != -1 || sprite._sequenceNumber == STOP_UP)
		sprite._frameNumber = 0;

	if (_vm->_onChessboard) {
		sprite._sequenceNumber = 0;
		_data[AL]._position.x = (_vm->_map[scene._charPoint].x -  6) * 100;
		_data[AL]._position.y = (_vm->_map[scene._charPoint].x + 10) * 100;
	}

	_oldWalkSequence = -1;
	_allowWalkAbort = true;
}

void People::walkToCoords(const Common::Point &destPos, int destDir) {
	// TODO
	warning("TODO: walkToCoords");
}

void People::goAllTheWay() {
	// TODO
}

} // End of namespace Sherlock
