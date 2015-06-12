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

#include "sherlock/tattoo/tattoo_people.h"
#include "sherlock/tattoo/tattoo_talk.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Tattoo {

void TattooPerson::adjustSprite() {
	// TODO
	warning("TODO: TattooPerson::adjustSprite");
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
	if (objNum < 256 && objNum != -1) {
		// See if the Object has to wait for an Abort Talk Code
		Object &obj = scene._bgShapes[objNum];
		if (obj.hasAborts())
			obj._gotoSeq = sequenceNum;
		else
			obj.setObjTalkSequence(sequenceNum);
	} else if (objNum != -1) {
		objNum -= 256;
		Person &person = *_data[objNum];

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
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	TattooTalk &talk = *(TattooTalk *)_vm->_talk;

	// If no speaker is specified, then nothing needs to be done
	if (speaker == -1)
		return;

	int objNum = people.findSpeaker(speaker);
	if (objNum != -1 && objNum < 256) {
		Object &obj = scene._bgShapes[objNum];

		// See if the Object has to wait for an Abort Talk Code
		if (obj.hasAborts()) {
			talk.pushTalkSequence(&obj);
			obj._gotoSeq = sequenceNum;
		}
		else {
			obj.setObjTalkSequence(sequenceNum);
		}
	}
	else if (objNum != -1) {
		objNum -= 256;
		Person &person = people[objNum];
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

void TattooPeople::synchronize(Serializer &s) {
	s.syncAsByte(_holmesOn);

	for (uint idx = 0; idx < _data.size(); ++idx) {
		Person &p = *_data[idx];
		s.syncAsSint32LE(p._position.x);
		s.syncAsSint32LE(p._position.y);
		s.syncAsSint16LE(p._sequenceNumber);
		s.syncAsSint16LE(p._type);
		s.syncString(p._walkVGSName);
		s.syncString(p._description);
		s.syncString(p._examine);
	}

	s.syncAsSint16LE(_holmesQuotient);

	if (s.isLoading()) {
		_hSavedPos = _data[PLAYER]->_position;
		_hSavedFacing = _data[PLAYER]->_sequenceNumber;
	}
}

void TattooPeople::gotoStand(Sprite &sprite) {
	error("TODO: gotoStand");
}

} // End of namespace Tattoo

} // End of namespace Sherlock
