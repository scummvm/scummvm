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

#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo_scene.h"
#include "sherlock/tattoo/tattoo_talk.h"
#include "sherlock/tattoo/tattoo_user_interface.h"
#include "sherlock/tattoo/tattoo.h"

namespace Sherlock {

namespace Tattoo {

#define FACING_PLAYER 16
#define NUM_ADJUSTED_WALKS 21
#define CHARACTERS_INDEX 256

struct AdjustWalk {
	char _vgsName[9];
	int _xAdjust;
	int _flipXAdjust;
	int _yAdjust;
};

static const AdjustWalk ADJUST_WALKS[NUM_ADJUSTED_WALKS] = {
	{ "TUPRIGHT", -7, -19, 6 },
	{ "TRIGHT", 8, -14, 0 },
	{ "TDOWNRG", 14, -12, 0 },
	{ "TWUPRIGH", 12, 4, 2 },
	{ "TWRIGHT", 31, -14, 0 },
	{ "TWDOWNRG", 6, -24, 0 },
	{ "HTUPRIGH", 2, -20, 0 },
	{ "HTRIGHT", 28, -20, 0 },
	{ "HTDOWNRG", 8, -2, 0 },
	{ "GTUPRIGH", 4, -12, 0 },
	{ "GTRIGHT", 12, -16, 0 },
	{ "GTDOWNRG", 10, -18, 0 },
	{ "JTUPRIGH", 8, -10, 0 },
	{ "JTRIGHT", 22, -6, 0 },
	{ "JTDOWNRG", 4, -20, 0 },
	{ "CTUPRIGH", 10, 0, 0 },
	{ "CTRIGHT", 26, -22, 0 },
	{ "CTDOWNRI", 16, 4, 0 },
	{ "ITUPRIGH", 0, 0, 0 },
	{ "ITRIGHT", 20, 0, 0 },
	{ "ITDOWNRG", 8, 0, 0 }
};

static const int WALK_SPEED_X[99] = {
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 98, 90, 90, 90, 90, 90, 91, 90, 90,
	90, 90,100, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,100, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,103, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90
};

static const int WALK_SPEED_Y[99] = {
	28, 28, 28, 28, 28, 28, 28, 28, 28, 32, 32, 32, 28, 28, 28, 28, 28, 26, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	32, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 31, 28, 28, 28, 28, 28, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28
};

static const int WALK_SPEED_DIAG_X[99] = {
	50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 90, 50, 50, 50, 50, 50, 50, 50, 50, 50,
	50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
};

/*----------------------------------------------------------------*/

SavedNPCPath::SavedNPCPath() {
	Common::fill(&_path[0], &_path[MAX_NPC_PATH], 0);
	_npcIndex = 0;
	_npcPause = 0;
	_npcFacing = 0;
	_lookHolmes = false;
}

SavedNPCPath::SavedNPCPath(byte path[MAX_NPC_PATH], int npcIndex, int npcPause, const Point32 &position,
	int npcFacing, bool lookHolmes) : _npcIndex(npcIndex), _npcPause(npcPause), _position(position),
		_npcFacing(npcFacing), _lookHolmes(lookHolmes) {
	Common::copy(&path[0], &path[MAX_NPC_PATH], &_path[0]);
}

/*----------------------------------------------------------------*/

TattooPerson::TattooPerson() : Person() {
	Common::fill(&_npcPath[0], &_npcPath[MAX_NPC_PATH], 0);
	_tempX = _tempScaleVal = 0;
	_npcIndex = 0;
	_npcMoved = false;
	_npcFacing = -1;
	_resetNPCPath = true;
	_savedNpcSequence = 0;
	_savedNpcFrame = 0;
	_updateNPCPath = true;
	_npcPause = 0;
	_lookHolmes = false;
}

TattooPerson::~TattooPerson() {
	delete _altImages;
}

void TattooPerson::freeAltGraphics() {
	delete _altImages;
	_altImages = nullptr;
	_altSeq = 0;
}

void TattooPerson::adjustSprite() {
	People &people = *_vm->_people;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	if (_type == INVALID)
		return;

	if (_type == CHARACTER && _status) {
		// Sprite waiting to move, so restart walk
		_walkCount = _status;
		_status = 0;

		_walkDest = _walkTo.front();
		setWalking();
	} else if (_type == CHARACTER && _walkCount) {
		if (_walkCount > 10) {
			_walkDest = _nextDest;
			setWalking();
		}

		_position += _delta;
		if (_walkCount)
			--_walkCount;

		if (!_walkCount) {
			// If there are remaining points to walk, move to the next one
			if (!_walkTo.empty()) {
				_walkDest = _walkTo.pop();
				setWalking();
			} else {
				gotoStand();
			}
		}
	}

	if (_type != CHARACTER) {
		if (_position.y > SHERLOCK_SCREEN_HEIGHT)
			_position.y = SHERLOCK_SCREEN_HEIGHT;

		if (_position.y < UPPER_LIMIT)
			_position.y = UPPER_LIMIT;

		if (_position.x < LEFT_LIMIT)
			_position.x = LEFT_LIMIT;

		if (_position.x > RIGHT_LIMIT)
			_position.x = RIGHT_LIMIT;
	}

	int frameNum = _frameNumber;
	if (frameNum == -1)
		frameNum = 0;
	int idx = _walkSequences[_sequenceNumber][frameNum];
	if (idx > _maxFrames)
		idx = 1;

	// Set the image frame
	if (_altSeq)
		_imageFrame = &(*_altImages)[idx - 1];
	else
		_imageFrame = &(*_images)[idx - 1];

	// See if the player has come to a stop after clicking on an Arrow zone to leave the scene.
	// If so, this will set up the exit information for the scene transition
	if (!_walkCount && ui._exitZone != -1 && scene._walkedInScene && scene._goToScene == -1 &&
			!_description.compareToIgnoreCase(people[HOLMES]._description)) {
		Exit &exit = scene._exits[ui._exitZone];
		scene._goToScene = exit._scene;

		if (exit._newPosition.x != 0) {
			people._savedPos = exit._newPosition;

			if (people._savedPos._facing > 100 && people._savedPos.x < 1)
				people._savedPos.x = 100;
		}
	}
}

void TattooPerson::gotoStand() {
	TattooPeople &people = *(TattooPeople *)_vm->_people;

	// If the misc field is set, then we're running a special talk sequence, so don't interrupt it.
	if (_misc)
		return;

	_walkTo.clear();
	_walkCount = 0;
	int oldFacing = _sequenceNumber;

	// If the person was talking or listening, just return it to the standing sequence
	// in the direction they were pointing
	if (_sequenceNumber >= TALK_UPRIGHT && _sequenceNumber <= LISTEN_UPLEFT) {
		switch (_sequenceNumber) {
		case TALK_UPRIGHT:
		case LISTEN_UPRIGHT:
			_sequenceNumber = STOP_UPRIGHT;
			break;
		case TALK_RIGHT:
		case LISTEN_RIGHT:
			_sequenceNumber = STOP_RIGHT;
			break;
		case TALK_DOWNRIGHT:
		case LISTEN_DOWNRIGHT:
			_sequenceNumber = STOP_DOWNRIGHT;
			break;
		case TALK_DOWNLEFT:
		case LISTEN_DOWNLEFT:
			_sequenceNumber = STOP_DOWNLEFT;
			break;
		case TALK_LEFT:
		case LISTEN_LEFT:
			_sequenceNumber = STOP_LEFT;
			break;
		case TALK_UPLEFT:
		case LISTEN_UPLEFT:
			_sequenceNumber = STOP_UPLEFT;
			break;
		default:
			break;
		}

		if (_seqTo) {
			// Reset to previous value
			_walkSequences[oldFacing]._sequences[_frameNumber] = _seqTo;
			_seqTo = 0;
		}

		// Set the Frame number to the last frame so we don't move
		_frameNumber = 0;

		checkWalkGraphics();

		_oldWalkSequence = -1;
		people._allowWalkAbort = true;
		return;
	}

	// If the sprite that is stopping is an NPC and he is supposed to face a certain direction
	// when he stops, set that direction here
	int npc = -1;
	for (int idx = 1; idx < MAX_CHARACTERS; ++idx) {
		if (_imageFrame == people[idx]._imageFrame)
			npc = idx;
	}

	if (npc != -1 && people[npc]._npcFacing != -1) {
		if (people[npc]._npcFacing == FACING_PLAYER) {
			// See where Holmes is with respect to the NPC (x coords)
			if (people[HOLMES]._position.x < people[npc]._position.x)
				people[npc]._npcFacing = STOP_LEFT;
			else
				people[npc]._npcFacing = STOP_RIGHT;

			// See where Holmes is with respect to the NPC (y coords)
			if (people[HOLMES]._position.y < people[npc]._position.y - (10 * FIXED_INT_MULTIPLIER)) {
				// Holmes is above the NPC so reset the facing to the diagonal ups
				if (people[npc]._npcFacing == STOP_RIGHT)
					people[npc]._npcFacing = STOP_UPRIGHT;
				else
					people[npc]._npcFacing = STOP_UPLEFT;
			} else {
				if (people[HOLMES]._position.y > people[npc]._position.y + (10 * FIXED_INT_MULTIPLIER)) {
					// Holmes is below the NPC so reset the facing to the diagonal downs
					if (people[npc]._npcFacing == STOP_RIGHT)
						people[npc]._npcFacing = STOP_DOWNRIGHT;
					else
						people[npc]._npcFacing = STOP_DOWNLEFT;
				}
			}
		}

		_sequenceNumber = people[npc]._npcFacing;
	} else {
		switch (_sequenceNumber) {
		case WALK_UP: _sequenceNumber = STOP_UP;			break;
		case WALK_UPRIGHT: _sequenceNumber = STOP_UPRIGHT;	break;
		case WALK_RIGHT: _sequenceNumber = STOP_RIGHT;		break;
		case WALK_DOWNRIGHT: _sequenceNumber = STOP_DOWNRIGHT; break;
		case WALK_DOWN: _sequenceNumber = STOP_DOWN;		break;
		case WALK_DOWNLEFT: _sequenceNumber = STOP_DOWNLEFT;break;
		case WALK_LEFT: _sequenceNumber = STOP_LEFT;		break;
		case WALK_UPLEFT: _sequenceNumber = STOP_UPLEFT;	break;
		default: break;
		}
	}

	// Only restart the frame number at 0 if the new sequence is different from the last sequence
	// so we don't let Holmes repeat standing.
	if (_oldWalkSequence != -1) {
		if (_seqTo) {
			// Reset to previous value
			_walkSequences[oldFacing]._sequences[_frameNumber] = _seqTo;
			_seqTo = 0;
		}

		_frameNumber = 0;
	}

	checkWalkGraphics();

	_oldWalkSequence = -1;
	people._allowWalkAbort = true;
}

void TattooPerson::setWalking() {
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	int oldDirection, oldFrame;
	Common::Point delta;
	_nextDest = _walkDest;

	// Flag that player has now walked in the scene
	scene._walkedInScene = true;

	// Stop any previous walking, since a new dest is being set
	_walkCount = 0;
	oldDirection = _sequenceNumber;
	oldFrame = _frameNumber;

	// Set speed to use horizontal and vertical movement
	int scaleVal = scene.getScaleVal(_position);
	Common::Point speed(MAX(WALK_SPEED_X[scene._currentScene - 1] * SCALE_THRESHOLD / scaleVal, 2),
		MAX(WALK_SPEED_Y[scene._currentScene - 1] * SCALE_THRESHOLD / scaleVal, 2));
	Common::Point diagSpeed(MAX(WALK_SPEED_DIAG_X[scene._currentScene - 1] * SCALE_THRESHOLD / scaleVal, 2),
		MAX((WALK_SPEED_Y[scene._currentScene - 1] - 2) * SCALE_THRESHOLD / scaleVal, 2));

	// If the player is already close to the given destination that no walking is needed,
	// move to the next  straight line segment in the overall walking route, if there is one
	for (;;) {
		if (_centerWalk || !_walkTo.empty()) {
			// Since we want the player to be centered on the ultimate destination, and the player
			// is drawn from the left side, move the cursor half the width of the player to center it
			delta = Common::Point(_position.x / FIXED_INT_MULTIPLIER - _walkDest.x,
				_position.y / FIXED_INT_MULTIPLIER - _walkDest.y);

			int dir;
			if (ABS(delta.x) > ABS(delta.y))
				dir = (delta.x < 0) ? WALK_LEFT : WALK_RIGHT;
			else
				dir = (delta.y < 0) ? WALK_UP : WALK_DOWN;

			scaleVal = scene.getScaleVal(Point32(_walkDest.x * FIXED_INT_MULTIPLIER,
				_walkDest.y * FIXED_INT_MULTIPLIER));
			_walkDest.x -= _stopFrames[dir]->sDrawXSize(scaleVal) / 2;
		}

		delta = Common::Point(
			ABS(_position.x / FIXED_INT_MULTIPLIER - _walkDest.x),
			ABS(_position.y / FIXED_INT_MULTIPLIER - _walkDest.y)
			);

		// If we're ready to move a sufficient distance, that's it. Otherwise,
		// move onto the next portion of the walk path, if there is one
		if ((delta.x > 3 || delta.y > 0) || _walkTo.empty())
			break;

		// Pop next walk segment off the walk route stack
		_walkDest = _walkTo.pop();
	}

	// If a sufficient move is being done, then start the move
	if (delta.x > 3 || delta.y) {
		// See whether the major movement is horizontal or vertical
		if (delta.x >= delta.y) {
			// Set the initial frame sequence for the left and right, as well
			// as setting the delta x depending on direction
			if (_walkDest.x < (_position.x / FIXED_INT_MULTIPLIER)) {
				_sequenceNumber = WALK_LEFT;
				_delta.x = speed.x * -(FIXED_INT_MULTIPLIER / 10);
			} else {
				_sequenceNumber = WALK_RIGHT;
				_delta.x = speed.x * (FIXED_INT_MULTIPLIER / 10);
			}

			// See if the x delta is too small to be divided by the speed, since
			// this would cause a divide by zero error
			if ((delta.x * 10) >= speed.x) {
				// Det the delta y
				_delta.y = (delta.y * FIXED_INT_MULTIPLIER) / ((delta.x * 10) / speed.x);
				if (_walkDest.y < (_position.y / FIXED_INT_MULTIPLIER))
					_delta.y = -_delta.y;

				// Set how many times we should add the delta to the player's position
				_walkCount = (delta.x * 10) / speed.x;
			} else {
				// The delta x was less than the speed (ie. we're really close to
				// the destination). So set delta to 0 so the player won't move
				_delta = Point32(0, 0);
				_position = Point32(_walkDest.x * FIXED_INT_MULTIPLIER, _walkDest.y * FIXED_INT_MULTIPLIER);

				_walkCount = 1;
			}

			// See if the sequence needs to be changed for diagonal walking
			if (_delta.y > 1500) {
				if (_sequenceNumber == WALK_LEFT || _sequenceNumber == WALK_RIGHT) {
					_delta.x = _delta.x / speed.x * diagSpeed.x;
					_delta.y = (delta.y * FIXED_INT_MULTIPLIER) / (delta.x * 10 / diagSpeed.x);

					_walkCount = delta.x * 10 / diagSpeed.x;
				}

				switch (_sequenceNumber) {
				case WALK_LEFT:
					_sequenceNumber = WALK_DOWNLEFT;
					break;
				case WALK_RIGHT:
					_sequenceNumber = WALK_DOWNRIGHT;
					break;
				default:
					break;
				}
			} else if (_delta.y < -1500) {
				if (_sequenceNumber == WALK_LEFT || _sequenceNumber == WALK_RIGHT) {
					_delta.x = _delta.x / speed.x * diagSpeed.x;
					_delta.y = -1 * (delta.y * FIXED_INT_MULTIPLIER) / (delta.x * 10 / diagSpeed.x);

					_walkCount = (delta.x * 10) / diagSpeed.x;
				}

				switch (_sequenceNumber) {
				case WALK_LEFT:
					_sequenceNumber = WALK_UPLEFT;
					break;
				case WALK_RIGHT:
					_sequenceNumber = WALK_UPRIGHT;
					break;
				default:
					break;
				}
			}
		} else {
			// Major movement is vertical, so set the sequence for up and down,
			// and set the delta Y depending on the direction
			if (_walkDest.y < (_position.y / FIXED_INT_MULTIPLIER)) {
				_sequenceNumber = WALK_UP;
				_delta.y = speed.y * -(FIXED_INT_MULTIPLIER / 10);
			} else {
				speed.y = diagSpeed.y;
				_sequenceNumber = WALK_DOWN;
				_delta.y = speed.y * (FIXED_INT_MULTIPLIER / 10);
			}

			// Set the delta x
			if (delta.y * 10 / speed.y)
				_delta.x = (delta.x * FIXED_INT_MULTIPLIER) / (delta.y * 10 / speed.y);
			else
				_delta.x = (delta.x * FIXED_INT_MULTIPLIER) / delta.y;

			if (_walkDest.x < _position.y / FIXED_INT_MULTIPLIER)
				_delta.x = -_delta.x;

			// Set how many times we should add the delta's to the players position
			if (delta.y * 10 / speed.y)
				_walkCount = delta.y * 10 / speed.y;
			else
				_walkCount = delta.y;
		}
	}

	// See if the new walk sequence is the same as the old. If it's a new one,
	// we need to reset the frame number to zero so its animation starts at
	// its beginning. Otherwise, if it's the same sequence, we can leave it
	// as is, so it keeps the animation going at wherever it was up to
	if (_sequenceNumber != _oldWalkSequence) {
		if (_seqTo) {
			// Reset to previous value
			_walkSequences[oldDirection]._sequences[_frameNumber] = _seqTo;
			_seqTo = 0;
		}
		_frameNumber = 0;
	}

	checkWalkGraphics();
	_oldWalkSequence = _sequenceNumber;

	if (!_walkCount && _walkTo.empty())
		gotoStand();

	// If the sequence is the same as when we started, then Holmes was standing still and we're trying
	// to re-stand him, so reset Holmes' rame to the old frame number from before it was reset to 0
	if (_sequenceNumber == oldDirection)
		_frameNumber = oldFrame;
}

void TattooPerson::walkToCoords(const Point32 &destPos, int destDir) {
	TattooEngine &vm = *(TattooEngine *)_vm;
	Events &events = *_vm->_events;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	Talk &talk = *_vm->_talk;

	CursorId oldCursor = events.getCursor();
	events.setCursor(WAIT);

	_walkDest = Common::Point(destPos.x / FIXED_INT_MULTIPLIER, destPos.y / FIXED_INT_MULTIPLIER);

	bool isHolmes = this == &people[HOLMES];
	if (isHolmes) {
		people._allowWalkAbort = true;
	} else {
		// Clear the path Variables
		_npcIndex = _npcPause = 0;
		Common::fill(_npcPath, _npcPath + 100, 0);
		_npcFacing = destDir;
	}

	_centerWalk = false;

	// Only move the person if they're going an appreciable distance
	if (ABS(_walkDest.x - (_position.x / FIXED_INT_MULTIPLIER)) > 8 ||
			ABS(_walkDest.y - (_position.y / FIXED_INT_MULTIPLIER)) > 4) {
		goAllTheWay();

		do {
			// Keep doing animations whilst walk is in progress
			events.wait(1);
			scene.doBgAnim();

			if (events.kbHit()) {
				Common::KeyState keyState = events.getKey();

				if (keyState.keycode == Common::KEYCODE_ESCAPE && vm._runningProlog) {
					vm.setFlags(-76);
					vm.setFlags(396);
					scene._goToScene = 1;
					talk._talkToAbort = true;
				}
			}
		} while (!_vm->shouldQuit() && _walkCount && !talk._talkToAbort);
	}

	_centerWalk = true;
	if (!isHolmes)
		_updateNPCPath = true;

	if (!talk._talkToAbort) {
		// put character exactly on right spot
		_position = destPos;

		if (_sequenceNumber != destDir) {
			// Facing character to correct ending direction
			_sequenceNumber = destDir;
			gotoStand();
		}

		if (!isHolmes)
			_updateNPCPath = false;

		// Secondary walking wait loop
		bool done = false;
		while (!done && !_vm->shouldQuit()) {
			events.wait(1);
			scene.doBgAnim();

			// See if we're past the initial goto stand sequence
			for (int idx = 0; idx < _frameNumber; ++idx) {
				if (_walkSequences[_sequenceNumber][idx] == 0) {
					done = true;
					break;
				}
			}

			if (events.kbHit()) {
				Common::KeyState keyState = events.getKey();

				if (keyState.keycode == Common::KEYCODE_ESCAPE && vm._runningProlog) {
					vm.setFlags(-76);
					vm.setFlags(396);
					scene._goToScene = 1;
					talk._talkToAbort = true;
				}
			}
		}

		if (!isHolmes)
			_updateNPCPath = true;

		if (!talk._talkToAbort)
			events.setCursor(oldCursor);
	}
}

void TattooPerson::clearNPC() {
	Common::fill(&_npcPath[0], &_npcPath[MAX_NPC_PATH], 0);
	_npcIndex = 0;
	_pathStack.clear();
	_npcName = "";
}

void TattooPerson::updateNPC() {
	People &people = *_vm->_people;
	Talk &talk = *_vm->_talk;

	// If the NPC isn't on, or it's in Talk or Listen Mode, then return without doing anything
	if (_type != CHARACTER || _sequenceNumber >= TALK_UPRIGHT)
		return;

	// If the NPC is paused, just decrement his pause counter and exit
	if (_npcPause) {
		// Decrement counter
		--_npcPause;

		// Now see if we need to update the NPC's frame sequence so that he faces Holmes
		if (_lookHolmes) {
			// See where Holmes is with respect to the NPC (x coordinate)
			_npcFacing =  (people[HOLMES]._position.x < _position.x) ? STOP_LEFT : STOP_RIGHT;

			// See where Holmes is with respect to the NPC (y coordinate)
			if (people[HOLMES]._position.y < (_position.y - 10 * FIXED_INT_MULTIPLIER)) {
				// Holmes is above the NPC so reset the facing to a diagonal up
				_npcFacing = (_npcFacing == STOP_RIGHT) ? STOP_UPRIGHT : STOP_UPLEFT;
			} else  if (people[HOLMES]._position.y > (_position.y + 10 * FIXED_INT_MULTIPLIER)) {
				// Holmes is below the NPC so reset the facing to a diagonal down
				_npcFacing = (_npcFacing == STOP_RIGHT) ? STOP_DOWNRIGHT : STOP_DOWNLEFT;
			}

			// See if we need to set the old_walk_sequence so the NPC will put his arms
			// up if he turns another way
			if (_sequenceNumber != _npcFacing)
				_oldWalkSequence = _sequenceNumber;

			gotoStand();
		}
	} else {
		// Reset the look flag so the NPC won't face Holmes anymore
		_lookHolmes = false;

		// See if the NPC is stopped or not. Don't do anything if he's moving
		if (!_walkCount) {
			// If there is no new command, reset the path back to the beginning
			if (!_npcPath[_npcIndex])
				_npcIndex = 0;

			// The NPC is stopped and any pause he was doing is done. We can now see what
			// the next command in the NPC path is.

			// Scan Past any NPC Path Labels since they do nothing except mark places for If's and Goto's
			while (_npcPath[_npcIndex] == NPCPATH_PATH_LABEL)
				_npcIndex += 2;

			if (_npcPath[_npcIndex]) {
				_npcFacing = -1;

				switch (_npcPath[_npcIndex]) {
				case NPCPATH_SET_DEST: {
					// Set the NPC's new destination
					int xp = (_npcPath[_npcIndex + 1] - 1) * 256 + _npcPath[_npcIndex + 2] - 1;
					if (xp > 16384)
						xp = -1 * (xp - 16384);
					_walkDest.x = xp;
					_walkDest.y = (_npcPath[_npcIndex + 3] - 1) * 256 + _npcPath[_npcIndex + 4] - 1;
					_npcFacing = _npcPath[_npcIndex + 5] - 1;

					goAllTheWay();
					_npcIndex += 6;
					break;
				}

				case NPCPATH_PAUSE:
					// Set the NPC to pause where he is
					_npcPause = (_npcPath[_npcIndex + 1] - 1) * 256 + _npcPath[_npcIndex + 2] - 1;
					_npcIndex += 3;
					break;

				case NPCPATH_SET_TALK_FILE: {
					// Set the NPC's Talk File to use if Holmes talks to them
					++_npcIndex;

					_npcName = "";
					for (int idx = 0; idx < 8; ++idx) {
						if (_npcPath[_npcIndex + idx] != '~')
							_npcName += _npcPath[_npcIndex + idx];
						else
							break;
					}

					_npcIndex += 8;
					break;
				}

				case NPCPATH_CALL_TALK_FILE: {
					// Call a Talk File
					++_npcIndex;

					Common::String name;
					for (int idx = 0; idx < 8; ++idx) {
						if (_npcPath[_npcIndex + idx] != '~')
							name += _npcPath[_npcIndex + idx];
						else
							break;
					}

					_npcIndex += 8;
					talk.talkTo(name);
					break;
				}

				case NPCPATH_TAKE_NOTES:
					// Set the NPC to Pause where he is and set his frame sequences
					// so he takes notes while he's paused
					_npcPause = (_npcPath[_npcIndex + 1] - 1) * 256 + _npcPath[_npcIndex + 2] - 1;
					_npcIndex += 3;
					break;

				case NPCPATH_FACE_HOLMES:
					// Set the NPC to Pause where he is and set his look flag so he will always face Holmes
					// while he is paused
					_npcPause = (_npcPath[_npcIndex + 1] - 1) * 256 + _npcPath[_npcIndex + 2] - 1;
					_lookHolmes = true;
					_npcIndex += 3;
					break;

				//case NPCPATH_PATH_LABEL:		// No implementation needed here

				case NPCPATH_GOTO_LABEL: {
					// Goto NPC Path Label
					int label = _npcPath[_npcIndex + 1];
					_npcIndex = 0;

					// Scan through NPC path data to find the label
					bool found = false;
					while (!found) {
						switch (_npcPath[_npcIndex]) {
						case NPCPATH_SET_DEST:
							_npcIndex += 6;
							break;
						case NPCPATH_PAUSE:
						case NPCPATH_TAKE_NOTES:
						case NPCPATH_FACE_HOLMES:
							_npcIndex += 3;
							break;
						case NPCPATH_SET_TALK_FILE:
						case NPCPATH_CALL_TALK_FILE:
							_npcIndex += 8;
							break;
						case NPCPATH_PATH_LABEL:
							if (_npcPath[_npcIndex + 1] == label)
								found = true;
							_npcIndex += 2;
							break;
						case NPCPATH_GOTO_LABEL:
							_npcIndex += 2;
							break;
						case NPCPATH_IFFLAG_GOTO_LABEL:
							_npcIndex += 4;
							break;
						default:
							break;
						}
					}
					break;
				}

				case NPCPATH_IFFLAG_GOTO_LABEL: {
					// If FLAG then Goto Label
					int flag = (_npcPath[_npcIndex + 1] - 1) * 256 + _npcPath[_npcIndex + 2] - 1 - (_npcPath[_npcIndex + 2] == 1 ? 1 : 0);

					// Set the value the flag should be for the if statement to succeed
					bool flagVal = flag < 16384;

					int label = _npcPath[_npcIndex + 3];
					_npcIndex += 4;

					// If the flag is set Correctly, move the NPC Index to the given label
					if (_vm->readFlags(flag & 16383) == flagVal) {
						_npcIndex = 0;
						bool found = false;
						while (!found)
						{
							switch (_npcPath[_npcIndex])
							{
							case NPCPATH_SET_DEST:
								_npcIndex += 6;
								break;
							case NPCPATH_PAUSE:
							case NPCPATH_TAKE_NOTES:
							case NPCPATH_FACE_HOLMES:
								_npcIndex += 3;
								break;
							case NPCPATH_SET_TALK_FILE:
							case NPCPATH_CALL_TALK_FILE:
								_npcIndex += 8;
								break;
							case NPCPATH_PATH_LABEL:
								if (_npcPath[_npcIndex + 1] == label)
									found = true;
								_npcIndex += 2;
								break;
							case NPCPATH_GOTO_LABEL:
								_npcIndex += 2;
								break;
							case NPCPATH_IFFLAG_GOTO_LABEL:
								_npcIndex += 4;
								break;
							default:
								break;
							}
						}
					}

					break;
				}

				default:
					break;
				}
			}
		}
	}
}

void TattooPerson::pushNPCPath() {
	assert(_pathStack.size() < 2);
	SavedNPCPath savedPath(_npcPath, _npcIndex, _npcPause, _position, _sequenceNumber, _lookHolmes);
	_pathStack.push(savedPath);
}

void TattooPerson::pullNPCPath() {
	// Pop the stack entry and restore the fields
	SavedNPCPath path = _pathStack.pop();
	Common::copy(&path._path[0], &path._path[MAX_NPC_PATH], &_npcPath[0]);
	_npcIndex = path._npcIndex;
	_npcPause = path._npcPause;

	// Handle the first case if the NPC was paused
	if (_npcPause) {
		_npcFacing = path._npcFacing;
		_lookHolmes = path._lookHolmes;

		// See if the NPC has moved from where they originally were
		if (path._position != _position) {
			_walkDest = Point32(path._position.x / FIXED_INT_MULTIPLIER, path._position.y / FIXED_INT_MULTIPLIER);
			goAllTheWay();
			_npcPause = 0;
			_npcIndex -= 3;
		} else {
			// See if we need to set the old walk sequence so the NPC will put his arms up if he turns another way
			if (_npcFacing != _sequenceNumber)
				_oldWalkSequence = _sequenceNumber;

			gotoStand();
		}
	} else {
		// Handle the second case if the NPC was in motion
		_npcIndex -= 6;
	}
}

Common::Point TattooPerson::getSourcePoint() const {
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	int scaleVal = scene.getScaleVal(_position);

	return Common::Point(_position.x / FIXED_INT_MULTIPLIER +
		(_imageFrame ? _imageFrame->sDrawXSize(scaleVal) / 2 : 0), _position.y / FIXED_INT_MULTIPLIER);
}

void TattooPerson::setObjTalkSequence(int seq) {
	assert(seq != -1 && _type == CHARACTER);

	if (_seqTo) {
		// reset to previous value
		_walkSequences[_sequenceNumber]._sequences[_frameNumber] = _seqTo;
		_seqTo = 0;
	}

	_sequenceNumber = _gotoSeq;
	_frameNumber = 0;
	checkWalkGraphics();
}

void TattooPerson::checkWalkGraphics() {
	People &people = *_vm->_people;

	if (_images == nullptr) {
		freeAltGraphics();
		return;
	}

	Common::Path filename(Common::String::format("%s.vgs", _walkSequences[_sequenceNumber]._vgsName.c_str()));

	// Set the adjust depending on if we have to fine tune the x position of this particular graphic
	_adjust.x = _adjust.y = 0;

	for (int idx = 0; idx < NUM_ADJUSTED_WALKS; ++idx) {
		if (!scumm_strnicmp(_walkSequences[_sequenceNumber]._vgsName.c_str(), ADJUST_WALKS[idx]._vgsName,
			strlen(ADJUST_WALKS[idx]._vgsName))) {
			if (_walkSequences[_sequenceNumber]._horizFlip)
				_adjust.x = ADJUST_WALKS[idx]._flipXAdjust;
			else
				_adjust.x = ADJUST_WALKS[idx]._xAdjust;

			_adjust.y = ADJUST_WALKS[idx]._yAdjust;
			break;
		}
	}

	// See if we're already using Alternate Graphics
	if (_altSeq) {
		// See if the VGS file called for is different than the alternate graphics already loaded
		if (_walkSequences[_sequenceNumber]._vgsName.compareToIgnoreCase(_walkSequences[_altSeq - 1]._vgsName)) {
			// Different AltGraphics, Free the old ones
			freeAltGraphics();
		}
	}

	// If there is no Alternate Sequence set, see if we need to load a new one
	if (!_altSeq) {
		int npcNum = -1;
		// Find which NPC this is so we can check the name of the graphics loaded
		for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
			if (this == &people[idx]) {
				npcNum = idx;
				break;
			}
		}

		if (npcNum != -1) {
			// See if the VGS file called for is different than the main graphics which are already loaded
			if (!filename.equalsIgnoreCase(people[npcNum]._walkVGSName)) {
				// See if this is one of the more used Walk Graphics stored in WALK.LIB
				for (int idx = 0; idx < NUM_IN_WALK_LIB; ++idx) {
					if (filename.equalsIgnoreCase(WALK_LIB_NAMES[idx])) {
						people._useWalkLib = true;
						break;
					}
				}

				_altImages = new ImageFile(filename);
				people._useWalkLib = false;

				_altSeq = _sequenceNumber + 1;
			}
		}
	}

	// If this is a different seqeunce from the current sequence, reset the appropriate variables
	if (_sequences != &_walkSequences[_sequenceNumber]._sequences[0]) {
		_seqTo = _seqCounter = _seqCounter2 = _seqStack = _startSeq = 0;
		_sequences = &_walkSequences[_sequenceNumber]._sequences[0];
		_seqSize = _walkSequences[_sequenceNumber]._sequences.size();

		// WORKAROUND: Occasionally when switching to a new walk sequence the existing frame number may be outside
		// the allowed range for the new sequence. In such cases, reset the frame number
		if (_frameNumber < 0 || _frameNumber >= (int)_seqSize || _walkSequences[_sequenceNumber][_frameNumber] == 0)
			_frameNumber = 0;
	}

	setImageFrame();
}

void TattooPerson::synchronize(Serializer &s) {
	if (s.isSaving()) {
		SpriteType type = (_type == INVALID && _walkLoaded) ? HIDDEN_CHARACTER : _type;
		s.syncAsSint16LE(type);
	} else {
		if (_walkCount)
			gotoStand();

		s.syncAsSint16LE(_type);
	}

	s.syncAsSint32LE(_position.x);
	s.syncAsSint32LE(_position.y);
	if (s.isSaving()) {
		Common::String path(_walkVGSName.toString('/'));
		s.syncString(path);
	} else {
		Common::String path;
		s.syncString(path);
		_walkVGSName = Common::Path(path);
	}
	s.syncString(_description);
	s.syncString(_examine);

	// NPC specific properties
	s.syncBytes(&_npcPath[0], MAX_NPC_PATH);
	s.syncString(_npcName);
	s.syncAsSint32LE(_npcPause);
	s.syncAsByte(_lookHolmes);
	s.syncAsByte(_updateNPCPath);
	if (s.isLoading())
		_npcIndex = 0;

	// Verbs
	for (int idx = 0; idx < 2; ++idx)
		_use[idx].synchronize(s);
}

void TattooPerson::walkHolmesToNPC() {
	Events &events = *_vm->_events;
	TattooScene &scene = *(TattooScene *)_vm->_scene;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	TattooPerson &holmes = people[HOLMES];

	// Save the character's details
	pushNPCPath();

	// If the NPC is moving, stop him at his current position
	if (_walkCount) {
		// Reset the facing so the NPC will stop facing the direction he was going,
		// rather than the direction he was supposed to when he finished wlaking
		_npcFacing = -1;
		gotoStand();
	}

	int scaleVal = scene.getScaleVal(_position);
	ImageFrame &imgFrame = (*holmes._images)[0];

	// Clear the path variables
	memset(_npcPath, 0, 100);

	// Set the NPC path so he pauses for 250 while looking at Holmes
	_npcPath[0] = 6;
	_npcPath[1] = 1;
	_npcPath[2] = 251;
	_npcIndex = 0;
	_npcPause = 250;
	_lookHolmes = true;

	// See where Holmes is with respect to the NPC (x coords)
	if (holmes._position.x < _position.x) {
		holmes._walkDest.x = MAX(_position.x / FIXED_INT_MULTIPLIER - imgFrame.sDrawXSize(scaleVal), 0);
	} else {
		holmes._walkDest.x = MIN(_position.x / FIXED_INT_MULTIPLIER + imgFrame.sDrawXSize(scaleVal) * 2,
			screen._backBuffer1.width() - 1);
	}

	// See where Holmes is with respect to the NPC (y coords)
	if (holmes._position.y < (_position.y - imgFrame.sDrawXSize(scaleVal) * 500)) {
		holmes._walkDest.y = MAX(_position.y / FIXED_INT_MULTIPLIER - imgFrame.sDrawXSize(scaleVal) / 2, 0);
	} else {
		if (holmes._position.y > (_position.y + imgFrame.sDrawXSize(scaleVal) * 500)) {
			// Holmes is below the NPC
			holmes._walkDest.y = MIN(_position.y / FIXED_INT_MULTIPLIER + imgFrame.sDrawXSize(scaleVal) / 2,
				SHERLOCK_SCREEN_HEIGHT - 1);
		} else {
			// Holmes is roughly on the same Y as the NPC
			holmes._walkDest.y = _position.y / FIXED_INT_MULTIPLIER;
		}
	}

	events.setCursor(WAIT);

	_walkDest.x += 10;
	people._allowWalkAbort = true;
	holmes.goAllTheWay();

	// Do doBgAnim should be called over and over until walk is done
	do {
		events.wait(1);
		scene.doBgAnim();
	} while (holmes._walkCount);

	if (!talk._talkToAbort) {
		// Setup correct direction for Holmes to face
		// See where Holmes is with respect to the NPC (x coords)
		int facing = (holmes._position.x < _position.x) ? STOP_RIGHT : STOP_LEFT;

		// See where Holmes is with respect to the NPC (y coords)
		if (holmes._position.y < (_position.y - (10 * FIXED_INT_MULTIPLIER))) {
			// Holmes is above the NPC. Reset the facing to the diagonal downs
			facing = (facing == STOP_RIGHT) ? STOP_DOWNRIGHT : STOP_DOWNLEFT;
		} else {
			if (holmes._position.y > (_position.y + 10 * FIXED_INT_MULTIPLIER)) {
				// Holmes is below the NPC. Reset the facing to the diagonal ups
				facing = (facing == STOP_RIGHT) ? STOP_UPRIGHT : STOP_UPLEFT;
			}
		}

		holmes._sequenceNumber = facing;
		holmes.gotoStand();

		events.setCursor(ARROW);
	}
}

void TattooPerson::walkBothToCoords(const PositionFacing &holmesDest, const PositionFacing &npcDest) {
	Events &events = *_vm->_events;
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Scene &scene = *_vm->_scene;
	Talk &talk = *_vm->_talk;
	TattooPerson &holmes = people[HOLMES];
	bool holmesStopped = false, npcStopped = false;

	// Save the current cursor and change to the wait cursor
	CursorId oldCursor = events.getCursor();
	events.setCursor(WAIT);

	holmes._centerWalk = false;
	_centerWalk = false;

	// Start Holmes walking to his dest
	holmes._walkDest = Common::Point(holmesDest.x / FIXED_INT_MULTIPLIER + 10, holmesDest.y / FIXED_INT_MULTIPLIER);
	people._allowWalkAbort = true;
	holmes.goAllTheWay();

	// Start the NPC walking to their dest
	_walkDest = Common::Point(npcDest.x / FIXED_INT_MULTIPLIER + 10, npcDest.y / FIXED_INT_MULTIPLIER);
	goAllTheWay();

	// Clear the path variables
	_npcIndex = _npcPause = 0;
	Common::fill(&_npcPath[0], &_npcPath[100], 0);
	_npcFacing = npcDest._facing;

	// Now loop until both stop walking
	do {
		events.pollEvents();
		scene.doBgAnim();

		if (!holmes._walkCount && !holmesStopped) {
			// Holmes finished walking
			holmesStopped = true;

			// Ensure Holmes is on the exact destination spot
			holmes._position = holmesDest;
			holmes._sequenceNumber = holmesDest._facing;
			holmes.gotoStand();
		}

		if (!_walkCount && !npcStopped) {
			// NPC finished walking
			npcStopped = true;

			// Ensure NPC is on the exact destination spot
			_position = npcDest;
			_sequenceNumber = npcDest._facing;
			gotoStand();
		}

	} while (!_vm->shouldQuit() && (holmes._walkCount || _walkCount));

	holmes._centerWalk = true;
	_centerWalk = true;

	// Do one last frame draw so that the last person to stop will be drawn in their final position
	scene.doBgAnim();

	_updateNPCPath = true;

	if (!talk._talkToAbort)
		// Restore original mouse cursor
		events.setCursor(oldCursor);
}

void TattooPerson::centerScreenOnPerson() {
	Screen &screen = *_vm->_screen;
	TattooUserInterface &ui = *(TattooUserInterface *)_vm->_ui;

	ui._targetScroll.x = CLIP(_position.x / FIXED_INT_MULTIPLIER - SHERLOCK_SCREEN_WIDTH / 2,
		0, screen._backBuffer1.width() - SHERLOCK_SCREEN_WIDTH);
	screen._currentScroll = ui._targetScroll;

	// Reset the default look position to the center of the screen
	ui._lookPos = screen._currentScroll + Common::Point(SHERLOCK_SCREEN_WIDTH / 2, SHERLOCK_SCREEN_HEIGHT / 2);
}

/*----------------------------------------------------------------*/

TattooPeople::TattooPeople(SherlockEngine *vm) : People(vm) {
	for (int idx = 0; idx < 6; ++idx)
		_data.push_back(new TattooPerson());
}

void TattooPeople::setListenSequence(int speaker, int sequenceNum) {
	Scene &scene = *_vm->_scene;

	// If no speaker is specified, then nothing needs to be done
	if (speaker == -1)
		return;

	int objNum = findSpeaker(speaker);
	if (objNum < CHARACTERS_INDEX && objNum != -1) {
		// See if the Object has to wait for an Abort Talk Code
		Object &obj = scene._bgShapes[objNum];
		if (obj.hasAborts())
			obj._gotoSeq = sequenceNum;
		else
			obj.setObjTalkSequence(sequenceNum);
	} else if (objNum != -1) {
		objNum -= CHARACTERS_INDEX;
		TattooPerson &person = (*this)[objNum];

		int newDir = person._sequenceNumber;
		switch (person._sequenceNumber) {
		case WALK_UP:
		case STOP_UP:
		case WALK_UPRIGHT:
		case STOP_UPRIGHT:
		case TALK_UPRIGHT:
		case LISTEN_UPRIGHT:
			newDir = LISTEN_UPRIGHT;
			break;
		case WALK_RIGHT:
		case STOP_RIGHT:
		case TALK_RIGHT:
		case LISTEN_RIGHT:
			newDir = LISTEN_RIGHT;
			break;
		case WALK_DOWNRIGHT:
		case STOP_DOWNRIGHT:
		case TALK_DOWNRIGHT:
		case LISTEN_DOWNRIGHT:
			newDir = LISTEN_DOWNRIGHT;
			break;
		case WALK_DOWN:
		case STOP_DOWN:
		case WALK_DOWNLEFT:
		case STOP_DOWNLEFT:
		case TALK_DOWNLEFT:
		case LISTEN_DOWNLEFT:
			newDir = LISTEN_DOWNLEFT;
			break;
		case WALK_LEFT:
		case STOP_LEFT:
		case TALK_LEFT:
		case LISTEN_LEFT:
			newDir = LISTEN_LEFT;
			break;
		case WALK_UPLEFT:
		case STOP_UPLEFT:
		case TALK_UPLEFT:
		case LISTEN_UPLEFT:
			newDir = LISTEN_UPLEFT;
			break;

		default:
			break;
		}

		// See if the NPC's Seq has to wait for an Abort Talk Code
		if (person.hasAborts()) {
			person._gotoSeq = newDir;
		}  else {
			if (person._seqTo) {
				// Reset to previous value
				person._walkSequences[person._sequenceNumber]._sequences[person._frameNumber] = person._seqTo;
				person._seqTo = 0;
			}

			person._sequenceNumber = newDir;
			person._frameNumber = 0;
			person.checkWalkGraphics();
		}
	}
}

void TattooPeople::setTalkSequence(int speaker, int sequenceNum) {
	TattooPeople &people = *(TattooPeople *)_vm->_people;
	Scene &scene = *_vm->_scene;
	TattooTalk &talk = *(TattooTalk *)_vm->_talk;

	// If no speaker is specified, then nothing needs to be done
	if (speaker == -1)
		return;

	int objNum = people.findSpeaker(speaker);
	if (objNum != -1 && objNum < CHARACTERS_INDEX) {
		Object &obj = scene._bgShapes[objNum];

		// See if the Object has to wait for an Abort Talk Code
		if (obj.hasAborts()) {
			talk.pushSequenceEntry(&obj);
			obj._gotoSeq = sequenceNum;
		} else {
			obj.setObjTalkSequence(sequenceNum);
		}
	} else if (objNum != -1) {
		objNum -= CHARACTERS_INDEX;
		TattooPerson &person = people[objNum];
		int newDir = person._sequenceNumber;

		switch (newDir) {
		case WALK_UP:
		case STOP_UP:
		case WALK_UPRIGHT:
		case STOP_UPRIGHT:
		case TALK_UPRIGHT:
		case LISTEN_UPRIGHT:
			newDir = TALK_UPRIGHT;
			break;
		case WALK_RIGHT:
		case STOP_RIGHT:
		case TALK_RIGHT:
		case LISTEN_RIGHT:
			newDir = TALK_RIGHT;
			break;
		case WALK_DOWNRIGHT:
		case STOP_DOWNRIGHT:
		case TALK_DOWNRIGHT:
		case LISTEN_DOWNRIGHT:
			newDir = TALK_DOWNRIGHT;
			break;
		case WALK_DOWN:
		case STOP_DOWN:
		case WALK_DOWNLEFT:
		case STOP_DOWNLEFT:
		case TALK_DOWNLEFT:
		case LISTEN_DOWNLEFT:
			newDir = TALK_DOWNLEFT;
			break;
		case WALK_LEFT:
		case STOP_LEFT:
		case TALK_LEFT:
		case LISTEN_LEFT:
			newDir = TALK_LEFT;
			break;
		case WALK_UPLEFT:
		case STOP_UPLEFT:
		case TALK_UPLEFT:
		case LISTEN_UPLEFT:
			newDir = TALK_UPLEFT;
			break;
		default:
			break;
		}

		// See if the NPC's sequence has to wait for an Abort Talk Code
		if (person.hasAborts()) {
			person._gotoSeq = newDir;
		} else {
			if (person._seqTo) {
				// Reset to previous value
				person._walkSequences[person._sequenceNumber]._sequences[person._frameNumber] = person._seqTo;
				person._seqTo = 0;
			}

			person._sequenceNumber = newDir;
			person._frameNumber = 0;
			person.checkWalkGraphics();
		}
	}
}


int TattooPeople::findSpeaker(int speaker) {
	speaker &= 0x7f;
	int result = People::findSpeaker(speaker);
	const char *portrait = _characters[speaker]._portrait;

	// Fallback that Rose Tattoo uses if no speaker was found
	if (result == -1) {
		bool flag = _vm->readFlags(FLAG_PLAYER_IS_HOLMES);

		if (_data[HOLMES]->_type == CHARACTER && ((speaker == HOLMES && flag) || (speaker == WATSON && !flag)))
			// Return the offset index for the first character
			return 0 + CHARACTERS_INDEX;

		// Otherwise, scan through the list of the remaining characters to find a name match
		for (uint idx = 1; idx < _data.size(); ++idx) {
			TattooPerson &p = (*this)[idx];

			if (p._type == CHARACTER) {
				if (scumm_strnicmp(portrait, p._npcName.c_str(), 4) == 0
					&& Common::isDigit(p._npcName[4]))
					return idx + CHARACTERS_INDEX;
			}
		}
	}

	return result;
}

void TattooPeople::synchronize(Serializer &s) {
	s.syncAsByte(_holmesOn);

	for (uint idx = 0; idx < _data.size(); ++idx)
		(*this)[idx].synchronize(s);

	s.syncAsSint16LE(_holmesQuotient);

	if (s.isLoading()) {
		_savedPos.x = _data[HOLMES]->_position.x;
		_savedPos.y = _data[HOLMES]->_position.y;
		_savedPos._facing = _data[HOLMES]->_sequenceNumber;
	}
}

bool TattooPeople::loadWalk() {
	Resources &res = *_vm->_res;
	bool result = false;

	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		Person &person = *_data[idx];

		if (!person._walkLoaded && (person._type == CHARACTER || person._type == HIDDEN_CHARACTER)) {
			if (person._type == HIDDEN_CHARACTER)
				person._type = INVALID;

			// See if this is one of the more used Walk Graphics stored in WALK.LIB
			for (int libNum = 0; libNum < NUM_IN_WALK_LIB; ++libNum) {
				if (person._walkVGSName.equalsIgnoreCase(WALK_LIB_NAMES[libNum])) {
					_useWalkLib = true;
					break;
				}
			}

			// Load the images for the character
			person._images = new ImageFile(person._walkVGSName, false);
			person._maxFrames = person._images->size();

			// Load walk sequence data
			Common::String baseName(person._walkVGSName.baseName());
			baseName = Common::String(baseName.c_str(), strchr(baseName.c_str(), '.'));
			baseName += ".SEQ";
			Common::Path fname = person._walkVGSName.getParent().appendComponent(baseName);

			// Load the walk sequence data
			Common::SeekableReadStream *stream = res.load(fname, _useWalkLib ? "walk.lib" : "vgs.lib");

			person._walkSequences.resize(stream->readByte());

			for (uint seqNum = 0; seqNum < person._walkSequences.size(); ++seqNum)
				person._walkSequences[seqNum].load(*stream);

			// Close the sequences resource
			delete stream;
			_useWalkLib = false;

			person._sequences = &person._walkSequences[person._sequenceNumber]._sequences[0];
			person._seqSize = person._walkSequences[person._sequenceNumber]._sequences.size();
			person._frameNumber = 0;
			person.setImageFrame();

			// Set the stop Frames pointers
			for (int dirNum = 0; dirNum < 8; ++dirNum) {
				int count = 0;
				while (person._walkSequences[dirNum + 8][count] != 0)
					++count;
				count += 2;
				count = person._walkSequences[dirNum + 8][count] - 1;
				person._stopFrames[dirNum] = &(*person._images)[count];
			}

			result = true;
			person._walkLoaded = true;
		} else if (person._type != CHARACTER) {
			person._walkLoaded = false;
		}
	}

	_forceWalkReload = false;
	return result;
}


void TattooPeople::pullNPCPaths() {
	for (int idx = 1; idx < MAX_CHARACTERS; ++idx) {
		TattooPerson &p = (*this)[idx];
		if (p._npcMoved) {
			while (!p._pathStack.empty())
				p.pullNPCPath();
		}
	}
}

const Common::Point TattooPeople::restrictToZone(int zoneId, const Common::Point &destPos) {
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Common::Rect &r = scene._zones[zoneId];

	if (destPos.x < 0 || destPos.x > screen._backBuffer1.width())
		return destPos;
	else if (destPos.y < r.top && r.left < destPos.x && destPos.x < r.right)
		return Common::Point(destPos.x, r.top);
	else if (destPos.y > r.bottom && r.left < destPos.x && destPos.x < r.right)
		return Common::Point(destPos.x, r.bottom);
	else if (destPos.x < r.left && r.top < destPos.y && destPos.y < r.bottom)
		return Common::Point(r.left, destPos.y);
	else if (destPos.x > r.right && r.top < destPos.y && destPos.y < r.bottom)
		return Common::Point(r.right, destPos.y);

	// Find which corner of the zone the point is closet to
	if (destPos.x <= r.left) {
		return Common::Point(r.left, (destPos.y <= r.top) ? r.top : r.bottom);
	} else {
		return Common::Point(r.right, (destPos.y <= r.top) ? r.top : r.bottom);
	}
}


} // End of namespace Tattoo

} // End of namespace Sherlock
