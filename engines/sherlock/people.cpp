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

#include "sherlock/people.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel_people.h"
#include "sherlock/tattoo/tattoo_people.h"

namespace Sherlock {

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

// Rose Tattoo walk image libraries
// Walk resources within WALK.LIB
const char *const WALK_LIB_NAMES[NUM_IN_WALK_LIB] = {
	"SVGAWALK.VGS",
	"COATWALK.VGS",
	"WATSON.VGS",
	"NOHAT.VGS",
	"TUPRIGHT.VGS",
	"TRIGHT.VGS",
	"TDOWNRG.VGS",
	"TWUPRIGH.VGS",
	"TWRIGHT.VGS",
	"TWDOWNRG.VGS"
};

/*----------------------------------------------------------------*/

Person::Person() : Sprite() {
	_walkLoaded = false;
	_oldWalkSequence = -1;
	_srcZone = _destZone = 0;
}

void Person::goAllTheWay() {
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Common::Point srcPt(_position.x / FIXED_INT_MULTIPLIER + frameWidth() / 2,
		_position.y / FIXED_INT_MULTIPLIER);

	// Get the zone the player is currently in
	_srcZone = scene.whichZone(srcPt);
	if (_srcZone == -1)
		_srcZone = scene.closestZone(srcPt);

	// Get the zone of the destination
	_destZone = scene.whichZone(people._walkDest);
	if (_destZone == -1) {
		_destZone = scene.closestZone(people._walkDest);

		// The destination isn't in a zone
		if (people._walkDest.x >= (SHERLOCK_SCREEN_WIDTH - 1))
			people._walkDest.x = SHERLOCK_SCREEN_WIDTH - 2;

		// Trace a line between the centroid of the found closest zone to
		// the destination, to find the point at which the zone will be left
		const Common::Rect &destRect = scene._zones[_destZone];
		const Common::Point destCenter((destRect.left + destRect.right) / 2,
			(destRect.top + destRect.bottom) / 2);
		const Common::Point delta = people._walkDest - destCenter;
		Point32 pt(destCenter.x * FIXED_INT_MULTIPLIER, destCenter.y * FIXED_INT_MULTIPLIER);

		// Move along the line until the zone is left
		do {
			pt += delta;
		} while (destRect.contains(pt.x / FIXED_INT_MULTIPLIER, pt.y / FIXED_INT_MULTIPLIER));

		// Set the new walk destination to the last point that was in the
		// zone just before it was left
		people._walkDest = Common::Point((pt.x - delta.x * 2) / FIXED_INT_MULTIPLIER,
			(pt.y - delta.y * 2) / FIXED_INT_MULTIPLIER);
	}

	// Only do a walk if both zones are acceptable
	if (_srcZone == -2 || _destZone == -2)
		return;

	// If the start and dest zones are the same, walk directly to the dest point
	if (_srcZone == _destZone) {
		setWalking();
	} else {
		// Otherwise a path needs to be formed from the path information
		int i = scene._walkDirectory[_srcZone][_destZone];

		// See if we need to use a reverse path
		if (i == -1)
			i = scene._walkDirectory[_destZone][_srcZone];

		int count = scene._walkData[i];
		++i;

		// See how many points there are between the src and dest zones
		if (!count || count == -1) {
			// There are none, so just walk to the new zone
			setWalking();
		} else {
			// There are points, so set up a multi-step path between points
			// to reach the given destination
			_walkTo.clear();

			if (scene._walkDirectory[_srcZone][_destZone] != -1) {
				i += 3 * (count - 1);
				for (int idx = 0; idx < count; ++idx, i -= 3) {
					_walkTo.push(Common::Point(READ_LE_UINT16(&scene._walkData[i]),
						scene._walkData[i + 2]));
				}
			} else {
				for (int idx = 0; idx < count; ++idx, i += 3) {
					_walkTo.push(Common::Point(READ_LE_UINT16(&scene._walkData[i]), scene._walkData[i + 2]));
				}
			}

			// Final position
			_walkTo.push(people._walkDest);

			// Start walking
			people._walkDest = _walkTo.pop();
			setWalking();
		}
	}
}

void Person::walkToCoords(const Point32 &destPos, int destDir) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Talk &talk = *_vm->_talk;

	CursorId oldCursor = events.getCursor();
	events.setCursor(WAIT);

	people._walkDest = Common::Point(destPos.x / FIXED_INT_MULTIPLIER + 10, destPos.y / FIXED_INT_MULTIPLIER);
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

/*----------------------------------------------------------------*/

People *People::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelPeople(vm);
	else
		return new Tattoo::TattooPeople(vm);
}

People::People(SherlockEngine *vm) : _vm(vm) {
	_holmesOn = true;
	_allowWalkAbort = false;
	_portraitLoaded = false;
	_portraitsOn = true;
	_clearingThePortrait = false;
	_talkPics = nullptr;
	_portraitSide = 0;
	_speakerFlip = false;
	_holmesFlip = false;
	_holmesQuotient = 0;
	_hSavedPos = Point32(-1, -1);
	_hSavedFacing = -1;
	_forceWalkReload = false;
	_useWalkLib = false;
	_walkControl = 0;

	_portrait._sequences = new byte[32];
}

People::~People() {
	for (uint idx = 0; idx < _data.size(); ++idx) {
		if (_data[idx]->_walkLoaded)
			delete _data[idx]->_images;
		delete _data[idx];
	}

	delete _talkPics;
	delete[] _portrait._sequences;
}

void People::reset() {
	_data[HOLMES]->_description = "Sherlock Holmes!";

	// Note: Serrated Scalpel only uses a single Person slot for Sherlock.. Watson is handled by scene sprites
	int count = IS_SERRATED_SCALPEL ? 1 : MAX_CHARACTERS;
	for (int idx = 0; idx < count; ++idx) {
		Person &p = *_data[idx];

		p._type = (idx == 0) ? CHARACTER : INVALID;
		if (IS_SERRATED_SCALPEL)
			p._position = Point32(100 * FIXED_INT_MULTIPLIER, 110 * FIXED_INT_MULTIPLIER);
		else
			p._position = Point32(36 * FIXED_INT_MULTIPLIER, 29 * FIXED_INT_MULTIPLIER);

		p._sequenceNumber = IS_SERRATED_SCALPEL ? Scalpel::STOP_DOWNRIGHT : Tattoo::STOP_DOWNRIGHT;
		p._imageFrame = nullptr;
		p._frameNumber = 1;
		p._delta = Point32(0, 0);
		p._oldPosition = Common::Point(0, 0);
		p._oldSize = Common::Point(0, 0);
		p._misc = 0;
		p._walkCount = 0;
		p._pickUp = "";
		p._allow = 0;
		p._noShapeSize = Common::Point(0, 0);
		p._goto = Common::Point(0, 0);
		p._status = 0;
		p._seqTo = 0;
		p._seqCounter = p._seqCounter2 = 0;
		p._seqStack = 0;
		p._gotoSeq = p._talkSeq = 0;
		p._restoreSlot = 0;
		p._startSeq = 0;
		p._altImages = nullptr;
		p._altSeq = 0;
		p._centerWalk = true;
		p._adjust = Common::Point(0, 0);

		// Load the default walk sequences
		p._walkTo.clear();
		p._oldWalkSequence = -1;
		p._walkSequences.clear();
		if (IS_SERRATED_SCALPEL) {
			p._walkSequences.resize(MAX_HOLMES_SEQUENCE);
			for (int seqIdx = 0; seqIdx < MAX_HOLMES_SEQUENCE; ++seqIdx) {
				p._walkSequences[seqIdx]._sequences.clear();

				const byte *pSrc = &CHARACTER_SEQUENCES[seqIdx][0];
				do {
					p._walkSequences[seqIdx]._sequences.push_back(*pSrc);
				} while (*pSrc++);
			}
		}
	}
}

bool People::freeWalk() {
	bool result = false;

	for (int idx = 0; idx < MAX_CHARACTERS; ++idx) {
		if (_data[idx]->_walkLoaded) {
			delete _data[idx]->_images;
			_data[idx]->_images = nullptr;
			
			_data[idx]->_walkLoaded = false;
			result = true;
		}
	}

	return result;
}

int People::findSpeaker(int speaker) {
	Scene &scene = *_vm->_scene;
	const char *portrait = _characters[speaker]._portrait;

	for (int idx = 0; idx < (int)scene._bgShapes.size(); ++idx) {
		Object &obj = scene._bgShapes[idx];

		if (obj._type == ACTIVE_BG_SHAPE) {
			Common::String name(obj._name.c_str(), obj._name.c_str() + 4);

			if (name.equalsIgnoreCase(portrait)
				&& obj._name[4] >= '0' && obj._name[4] <= '9')
				return idx;
		}
	}

	return -1;
}

void People::clearTalking() {
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;

	if (_portraitsOn) {
		Common::Point pt = _portrait._position;
		int width, height;
		_portrait._imageFrame = _talkPics ? &(*_talkPics)[0] : (ImageFrame *)nullptr;

		// Flag portrait for removal, and save the size of the frame to use erasing it
		_portrait._type = REMOVE;
		_portrait._delta.x = width = _portrait.frameWidth();
		_portrait._delta.y = height = _portrait.frameHeight();

		delete _talkPics;
		_talkPics = nullptr;

		// Flag to let the talk code know not to interrupt on the next doBgAnim
		_clearingThePortrait = true;
		scene.doBgAnim();
		_clearingThePortrait = false;

		screen.slamArea(pt.x, pt.y, width, height);

		if (!talk._talkToAbort)
			_portraitLoaded = false;
	}
}

void People::synchronize(Serializer &s) {
	s.syncAsByte(_holmesOn);

	if (IS_SERRATED_SCALPEL) {
		s.syncAsSint16LE(_data[HOLMES]->_position.x);
		s.syncAsSint16LE(_data[HOLMES]->_position.y);
		s.syncAsSint16LE(_data[HOLMES]->_sequenceNumber);
	} else {
		for (uint idx = 0; idx < _data.size(); ++idx) {
			Person &p = *_data[idx];
			s.syncAsSint16LE(p._position.x);
			s.syncAsSint16LE(p._position.y);
			s.syncAsSint16LE(p._sequenceNumber);
			s.syncAsSint16LE(p._type);
			s.syncString(p._walkVGSName);
			s.syncString(p._description);
			s.syncString(p._examine);
		}
	}

	s.syncAsSint16LE(_holmesQuotient);

	if (s.isLoading()) {
		_hSavedPos = _data[HOLMES]->_position;
		_hSavedFacing = _data[HOLMES]->_sequenceNumber;
	}
}

} // End of namespace Sherlock
