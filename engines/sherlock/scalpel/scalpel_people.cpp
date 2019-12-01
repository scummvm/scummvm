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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/scalpel/scalpel_map.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Scalpel {

// Walk speeds
#define MWALK_SPEED 2
#define XWALK_SPEED 4
#define YWALK_SPEED 1

/*----------------------------------------------------------------*/

void ScalpelPerson::adjustSprite() {
	Map &map = *_vm->_map;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Talk &talk = *_vm->_talk;

	if (_type == INVALID || (_type == CHARACTER && scene._animating))
		return;

	if (!talk._talkCounter && _type == CHARACTER && _walkCount) {
		// Handle active movement for the sprite
		_position += _delta;
		--_walkCount;

		if (!_walkCount) {
			// If there any points left for the character to walk to along the
			// route to a destination, then move to the next point
			if (!people[HOLMES]._walkTo.empty()) {
				_walkDest = people[HOLMES]._walkTo.pop();
				setWalking();
			} else {
				gotoStand();
			}
		}
	}

	if (_type == CHARACTER && !map._active) {
		if ((_position.y / FIXED_INT_MULTIPLIER) > LOWER_LIMIT) {
			_position.y = LOWER_LIMIT * FIXED_INT_MULTIPLIER;
			gotoStand();
		}

		if ((_position.y / FIXED_INT_MULTIPLIER) < UPPER_LIMIT) {
			_position.y = UPPER_LIMIT * FIXED_INT_MULTIPLIER;
			gotoStand();
		}

		if ((_position.x / FIXED_INT_MULTIPLIER) < LEFT_LIMIT) {
			_position.x = LEFT_LIMIT * FIXED_INT_MULTIPLIER;
			gotoStand();
		}

		if ((_position.x / FIXED_INT_MULTIPLIER) > RIGHT_LIMIT) {
			_position.x = RIGHT_LIMIT * FIXED_INT_MULTIPLIER;
			gotoStand();
		}
	} else if (!map._active) {
		_position.y = CLIP((int)_position.y, (int)UPPER_LIMIT, (int)LOWER_LIMIT);
		_position.x = CLIP((int)_position.x, (int)LEFT_LIMIT, (int)RIGHT_LIMIT);
	}

	if (!map._active || (map._frameChangeFlag = !map._frameChangeFlag))
		++_frameNumber;

	if (_frameNumber >= (int)_walkSequences[_sequenceNumber]._sequences.size() ||
			_walkSequences[_sequenceNumber][_frameNumber] == 0) {
		switch (_sequenceNumber) {
		case STOP_UP:
		case STOP_DOWN:
		case STOP_LEFT:
		case STOP_RIGHT:
		case STOP_UPRIGHT:
		case STOP_UPLEFT:
		case STOP_DOWNRIGHT:
		case STOP_DOWNLEFT:
			// We're in a stop sequence, so reset back to the last frame, so
			// the character is shown as standing still
			--_frameNumber;
			break;

		default:
			// Move 1 past the first frame - we need to compensate, since we
			// already passed the frame increment
			_frameNumber = 1;
			break;
		}
	}

	// Update the _imageFrame to point to the new frame's image
	setImageFrame();

	// Check to see if character has entered an exit zone
	if (!_walkCount && scene._walkedInScene && scene._goToScene == -1) {
		Common::Rect charRect(_position.x / FIXED_INT_MULTIPLIER - 5, _position.y / FIXED_INT_MULTIPLIER - 2,
			_position.x / FIXED_INT_MULTIPLIER + 5, _position.y / FIXED_INT_MULTIPLIER + 2);
		Exit *exit = scene.checkForExit(charRect);

		if (exit) {
			scene._goToScene = exit->_scene;

			if (exit->_newPosition.x != 0) {
				people._savedPos = exit->_newPosition;

				if (people._savedPos._facing > 100 && people._savedPos.x < 1)
					people._savedPos.x = 100;
			}
		}
	}
}

void ScalpelPerson::gotoStand() {
	ScalpelMap &map = *(ScalpelMap *)_vm->_map;
	People &people = *_vm->_people;
	_walkTo.clear();
	_walkCount = 0;

	switch (_sequenceNumber) {
	case Scalpel::WALK_UP:
		_sequenceNumber = STOP_UP;
		break;
	case WALK_DOWN:
		_sequenceNumber = STOP_DOWN;
		break;
	case TALK_LEFT:
	case WALK_LEFT:
		_sequenceNumber = STOP_LEFT;
		break;
	case TALK_RIGHT:
	case WALK_RIGHT:
		_sequenceNumber = STOP_RIGHT;
		break;
	case WALK_UPRIGHT:
		_sequenceNumber = STOP_UPRIGHT;
		break;
	case WALK_UPLEFT:
		_sequenceNumber = STOP_UPLEFT;
		break;
	case WALK_DOWNRIGHT:
		_sequenceNumber = STOP_DOWNRIGHT;
		break;
	case WALK_DOWNLEFT:
		_sequenceNumber = STOP_DOWNLEFT;
		break;
	default:
		break;
	}

	// Only restart frame at 0 if the sequence number has changed
	if (_oldWalkSequence != -1 || _sequenceNumber == Scalpel::STOP_UP)
		_frameNumber = 0;

	if (map._active) {
		_sequenceNumber = 0;
		people[HOLMES]._position.x = (map[map._charPoint].x - 6) * FIXED_INT_MULTIPLIER;
		people[HOLMES]._position.y = (map[map._charPoint].y + 10) * FIXED_INT_MULTIPLIER;
	}

	_oldWalkSequence = -1;
	people._allowWalkAbort = true;
}

void ScalpelPerson::setWalking() {
	Map &map = *_vm->_map;
	Scene &scene = *_vm->_scene;
	int oldDirection, oldFrame;
	Common::Point speed, delta;

	// Flag that player has now walked in the scene
	scene._walkedInScene = true;

	// Stop any previous walking, since a new dest is being set
	_walkCount = 0;
	oldDirection = _sequenceNumber;
	oldFrame = _frameNumber;

	// Set speed to use horizontal and vertical movement
	if (map._active) {
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
		int temp;
		if (_walkDest.x >= (temp = _imageFrame->_frame.w / 2))
			_walkDest.x -= temp;

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
				_sequenceNumber = (map._active ? (int)MAP_LEFT : (int)WALK_LEFT);
				_delta.x = speed.x * -FIXED_INT_MULTIPLIER;
			} else {
				_sequenceNumber = (map._active ? (int)MAP_RIGHT : (int)WALK_RIGHT);
				_delta.x = speed.x * FIXED_INT_MULTIPLIER;
			}

			// See if the x delta is too small to be divided by the speed, since
			// this would cause a divide by zero error
			if (delta.x >= speed.x) {
				// Det the delta y
				_delta.y = (delta.y * FIXED_INT_MULTIPLIER) / (delta.x / speed.x);
				if (_walkDest.y < (_position.y / FIXED_INT_MULTIPLIER))
					_delta.y = -_delta.y;

				// Set how many times we should add the delta to the player's position
				_walkCount = delta.x / speed.x;
			} else {
				// The delta x was less than the speed (ie. we're really close to
				// the destination). So set delta to 0 so the player won't move
				_delta = Point32(0, 0);
				_position = Point32(_walkDest.x * FIXED_INT_MULTIPLIER, _walkDest.y * FIXED_INT_MULTIPLIER);

				_walkCount = 1;
			}

			// See if the sequence needs to be changed for diagonal walking
			if (_delta.y > 150) {
				if (!map._active) {
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
				}
			} else if (_delta.y < -150) {
				if (!map._active) {
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
			}
		} else {
			// Major movement is vertical, so set the sequence for up and down,
			// and set the delta Y depending on the direction
			if (_walkDest.y < (_position.y / FIXED_INT_MULTIPLIER)) {
				_sequenceNumber = WALK_UP;
				_delta.y = speed.y * -FIXED_INT_MULTIPLIER;
			} else {
				_sequenceNumber = WALK_DOWN;
				_delta.y = speed.y * FIXED_INT_MULTIPLIER;
			}

			// If we're on the overhead map, set the sequence so we keep moving
			// in the same direction
			if (map._active)
				_sequenceNumber = (oldDirection == -1) ? MAP_RIGHT : oldDirection;

			// Set the delta x
			_delta.x = (delta.x * FIXED_INT_MULTIPLIER) / (delta.y / speed.y);
			if (_walkDest.x < (_position.x / FIXED_INT_MULTIPLIER))
				_delta.x = -_delta.x;

			_walkCount = delta.y / speed.y;
		}
	}

	// See if the new walk sequence is the same as the old. If it's a new one,
	// we need to reset the frame number to zero so its animation starts at
	// its beginning. Otherwise, if it's the same sequence, we can leave it
	// as is, so it keeps the animation going at wherever it was up to
	if (_sequenceNumber != _oldWalkSequence)
		_frameNumber = 0;
	_oldWalkSequence = _sequenceNumber;

	if (!_walkCount)
		gotoStand();

	// If the sequence is the same as when we started, then Holmes was
	// standing still and we're trying to re-stand him, so reset Holmes'
	// rame to the old frame number from before it was reset to 0
	if (_sequenceNumber == oldDirection)
		_frameNumber = oldFrame;
}

void ScalpelPerson::walkToCoords(const Point32 &destPos, int destDir) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Talk &talk = *_vm->_talk;

	CursorId oldCursor = events.getCursor();
	events.setCursor(WAIT);

	_walkDest = Common::Point(destPos.x / FIXED_INT_MULTIPLIER + 10, destPos.y / FIXED_INT_MULTIPLIER);
	people._allowWalkAbort = true;
	goAllTheWay();

	// Keep calling doBgAnim until the walk is done
	do {
		events.pollEventsAndWait();
		scene.doBgAnim();
	} while (!_vm->shouldQuit() && _walkCount);

	if (!talk._talkToAbort) {
		// Put character exactly on destination position, and set direction
		_position = destPos;
		_sequenceNumber = destDir;
		gotoStand();

		// Draw Holmes facing the new direction
		scene.doBgAnim();

		if (!talk._talkToAbort)
			events.setCursor(oldCursor);
	}
}

Common::Point ScalpelPerson::getSourcePoint() const {
	return Common::Point(_position.x / FIXED_INT_MULTIPLIER + frameWidth() / 2,
		_position.y / FIXED_INT_MULTIPLIER);
}

void ScalpelPerson::synchronize(Serializer &s) {
	if (_walkCount)
		gotoStand();

	s.syncAsSint32LE(_position.x);
	s.syncAsSint32LE(_position.y);
}

/*----------------------------------------------------------------*/

ScalpelPeople::ScalpelPeople(SherlockEngine *vm) : People(vm) {
	_data.push_back(new ScalpelPerson());
}

void ScalpelPeople::setTalking(int speaker) {
	Resources &res = *_vm->_res;

	// If no speaker is specified, then we can exit immediately
	if (speaker == -1)
		return;

	if (_portraitsOn) {
		delete _talkPics;
		Common::String filename = Common::String::format("%s.vgs", _characters[speaker]._portrait);
		_talkPics = new ImageFile(filename);

		// Load portrait sequences
		Common::SeekableReadStream *stream = res.load("sequence.txt");
		stream->seek(speaker * MAX_FRAME);

		int idx = 0;
		do {
			_portrait._sequences[idx] = stream->readByte();
			++idx;
		} while (idx < 2 || _portrait._sequences[idx - 2] || _portrait._sequences[idx - 1]);

		delete stream;

		_portrait._maxFrames = idx;
		_portrait._frameNumber = 0;
		_portrait._sequenceNumber = 0;
		_portrait._images = _talkPics;
		_portrait._imageFrame = &(*_talkPics)[0];
		_portrait._position = Common::Point(_portraitSide, 10);
		_portrait._delta = Common::Point(0, 0);
		_portrait._oldPosition = Common::Point(0, 0);
		_portrait._goto = Common::Point(0, 0);
		_portrait._flags = 5;
		_portrait._status = 0;
		_portrait._misc = 0;
		_portrait._allow = 0;
		_portrait._type = ACTIVE_BG_SHAPE;
		_portrait._name = " ";
		_portrait._description = " ";
		_portrait._examine = " ";
		_portrait._walkCount = 0;

		if (_holmesFlip || _speakerFlip) {
			_portrait._flags |= 2;

			_holmesFlip = false;
			_speakerFlip = false;
		}

		if (_portraitSide == 20)
			_portraitSide = 220;
		else
			_portraitSide = 20;

		_portraitLoaded = true;
	}
}

void ScalpelPeople::synchronize(Serializer &s) {
	(*this)[HOLMES].synchronize(s);
	s.syncAsSint16LE(_holmesQuotient);
	s.syncAsByte(_holmesOn);

	if (s.isLoading()) {
		_savedPos = _data[HOLMES]->_position;
		_savedPos._facing = _data[HOLMES]->_sequenceNumber;
	}
}

void ScalpelPeople::setTalkSequence(int speaker, int sequenceNum) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;

	// If no speaker is specified, then nothing needs to be done
	if (speaker == -1)
		return;

	if (speaker) {
		int objNum = people.findSpeaker(speaker);
		if (objNum != -1) {
			Object &obj = scene._bgShapes[objNum];

			if (obj._seqSize < MAX_TALK_SEQUENCES) {
				warning("Tried to copy too many talk frames");
			} else {
				for (int idx = 0; idx < MAX_TALK_SEQUENCES; ++idx) {
					obj._sequences[idx] = people._characters[speaker]._talkSequences[idx];
					if (idx > 0 && !obj._sequences[idx] && !obj._sequences[idx - 1])
						return;

					obj._frameNumber = 0;
					obj._sequenceNumber = 0;
				}
			}
		}
	}
}

bool ScalpelPeople::loadWalk() {
	bool result = false;

	if (_data[HOLMES]->_walkLoaded) {
		return false;
	} else {
		if (!IS_3DO) {
			_data[HOLMES]->_images = new ImageFile("walk.vgs");
		} else {
			// Load walk.anim on 3DO, which is a cel animation file
			_data[HOLMES]->_images = new ImageFile3DO("walk.anim", kImageFile3DOType_CelAnimation);
		}
		_data[HOLMES]->setImageFrame();
		_data[HOLMES]->_walkLoaded = true;

		result = true;
	}

	_forceWalkReload = false;
	return result;
}

const Common::Point ScalpelPeople::restrictToZone(int zoneId, const Common::Point &destPos) {
	Scene &scene = *_vm->_scene;
	Common::Point walkDest = destPos;

	// The destination isn't in a zone
	if (walkDest.x >= (SHERLOCK_SCREEN_WIDTH - 1))
		walkDest.x = SHERLOCK_SCREEN_WIDTH - 2;

	// Trace a line between the centroid of the found closest zone to
	// the destination, to find the point at which the zone will be left
	const Common::Rect &destRect = scene._zones[zoneId];
	const Common::Point destCenter((destRect.left + destRect.right) / 2,
		(destRect.top + destRect.bottom) / 2);
	const Common::Point delta = walkDest - destCenter;
	Point32 pt(destCenter.x * FIXED_INT_MULTIPLIER, destCenter.y * FIXED_INT_MULTIPLIER);

	// Move along the line until the zone is left
	do {
		pt += delta;
	} while (destRect.contains(pt.x / FIXED_INT_MULTIPLIER, pt.y / FIXED_INT_MULTIPLIER));

	// Set the new walk destination to the last point that was in the
	// zone just before it was left
	return Common::Point((pt.x - delta.x * 2) / FIXED_INT_MULTIPLIER,
		(pt.y - delta.y * 2) / FIXED_INT_MULTIPLIER);
}

void ScalpelPeople::setListenSequence(int speaker, int sequenceNum) {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;

	// Don't bother doing anything if no specific speaker is specified
	if (speaker == -1)
		return;

	if (speaker) {
		int objNum = people.findSpeaker(speaker);
		if (objNum != -1) {
			Object &obj = scene._bgShapes[objNum];

			if (obj._seqSize < MAX_TALK_SEQUENCES) {
				warning("Tried to copy too few still frames");
			} else {
				for (uint idx = 0; idx < MAX_TALK_SEQUENCES; ++idx) {
					obj._sequences[idx] = people._characters[speaker]._stillSequences[idx];
					if (idx > 0 && !people._characters[speaker]._talkSequences[idx] &&
						!people._characters[speaker]._talkSequences[idx - 1])
						break;
				}

				obj._frameNumber = 0;
				obj._seqTo = 0;
			}
		}
	}
}

} // End of namespace Scalpel

} // End of namespace Sherlock
