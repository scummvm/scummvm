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
			if (!people._walkTo.empty()) {
				people._walkDest = people._walkTo.pop();
				people.setWalking();
			} else {
				people.gotoStand(*this);
			}
		}
	}

	if (_type == CHARACTER && !map._active) {
		if ((_position.y / FIXED_INT_MULTIPLIER) > LOWER_LIMIT) {
			_position.y = LOWER_LIMIT * FIXED_INT_MULTIPLIER;
			people.gotoStand(*this);
		}

		if ((_position.y / FIXED_INT_MULTIPLIER) < UPPER_LIMIT) {
			_position.y = UPPER_LIMIT * FIXED_INT_MULTIPLIER;
			people.gotoStand(*this);
		}

		if ((_position.x / FIXED_INT_MULTIPLIER) < LEFT_LIMIT) {
			_position.x = LEFT_LIMIT * FIXED_INT_MULTIPLIER;
			people.gotoStand(*this);
		}

		if ((_position.x / FIXED_INT_MULTIPLIER) > RIGHT_LIMIT) {
			_position.x = RIGHT_LIMIT * FIXED_INT_MULTIPLIER;
			people.gotoStand(*this);
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

			if (exit->_people.x != 0) {
				people._hSavedPos = exit->_people;
				people._hSavedFacing = exit->_peopleDir;

				if (people._hSavedFacing > 100 && people._hSavedPos.x < 1)
					people._hSavedPos.x = 100;
			}
		}
	}
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
	s.syncAsByte(_holmesOn);
	s.syncAsSint32LE(_data[PLAYER]->_position.x);
	s.syncAsSint32LE(_data[PLAYER]->_position.y);
	s.syncAsSint16LE(_data[PLAYER]->_sequenceNumber);
	s.syncAsSint16LE(_holmesQuotient);

	if (s.isLoading()) {
		_hSavedPos = _data[PLAYER]->_position;
		_hSavedFacing = _data[PLAYER]->_sequenceNumber;
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
			}
			else {
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

void ScalpelPeople::gotoStand(Sprite &sprite) {
	ScalpelMap &map = *(ScalpelMap *)_vm->_map;
	_walkTo.clear();
	sprite._walkCount = 0;

	switch (sprite._sequenceNumber) {
	case Scalpel::WALK_UP:
		sprite._sequenceNumber = STOP_UP;
		break;
	case WALK_DOWN:
		sprite._sequenceNumber = STOP_DOWN;
		break;
	case TALK_LEFT:
	case WALK_LEFT:
		sprite._sequenceNumber = STOP_LEFT;
		break;
	case TALK_RIGHT:
	case WALK_RIGHT:
		sprite._sequenceNumber = STOP_RIGHT;
		break;
	case WALK_UPRIGHT:
		sprite._sequenceNumber = STOP_UPRIGHT;
		break;
	case WALK_UPLEFT:
		sprite._sequenceNumber = STOP_UPLEFT;
		break;
	case WALK_DOWNRIGHT:
		sprite._sequenceNumber = STOP_DOWNRIGHT;
		break;
	case WALK_DOWNLEFT:
		sprite._sequenceNumber = STOP_DOWNLEFT;
		break;
	default:
		break;
	}

	// Only restart frame at 0 if the sequence number has changed
	if (_oldWalkSequence != -1 || sprite._sequenceNumber == Scalpel::STOP_UP)
		sprite._frameNumber = 0;

	if (map._active) {
		sprite._sequenceNumber = 0;
		_data[PLAYER]->_position.x = (map[map._charPoint].x - 6) * FIXED_INT_MULTIPLIER;
		_data[PLAYER]->_position.y = (map[map._charPoint].y + 10) * FIXED_INT_MULTIPLIER;
	}

	_oldWalkSequence = -1;
	_allowWalkAbort = true;
}


} // End of namespace Scalpel

} // End of namespace Sherlock
