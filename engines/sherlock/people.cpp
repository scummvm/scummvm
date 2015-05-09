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

const char PORTRAITS[MAX_PEOPLE][5] = {
	{ "HOLM" },			// Sherlock Holmes
	{ "WATS" },			// Dr. Watson
	{ "LEST" },			// Inspector Lestrade
	{ "CON1" },			// Constable O'Brien
	{ "CON2" },			// Constable Lewis
	{ "SHEI" },			// Sheila Parker
	{ "HENR" },			// Henry Carruthers
	{ "LESL" },			// Lesley (flower girl)
	{ "USH1" },			// Usher #1
	{ "USH2" },			// Usher #2
	{ "FRED" },			// Fredrick Epstein
	{ "WORT" },			// Mrs. Worthington
	{ "COAC" },			// Coach
	{ "PLAY" },			// Player
	{ "WBOY" },			// Tim (Waterboy)
	{ "JAME" },			// James Sanders
	{ "BELL" },			// Belle (perfumerie)
	{ "GIRL" },			// Cleaning Girl (perfumerie)
	{ "EPST" },			// Epstien in the Opera Balcony
	{ "WIGG" },			// Wiggins
	{ "PAUL" },			// Paul (Brumwell / Carroway)
	{ "BART" },			// Bartender
	{ "DIRT" },			// Dirty Drunk
	{ "SHOU" },			// Shouting Drunk
	{ "STAG" },			// Staggering Drunk
	{ "BOUN" },			// Bouncer
	{ "SAND" },			// James Sanders - At Home
	{ "CORO" },			// The Coroner
	{ "EQUE" },			// The Equestrian Shop Keeper
	{ "GEOR" },			// George Blackwood
	{ "LARS" },			// Lars
	{ "PARK" },			// Sheila Parker (happy)
	{ "CHEM" },			// Chemist
	{ "GREG" },			// Inspector Gregson
	{ "LAWY" },			// Jacob Farthington Lawyer
	{ "MYCR" },			// Mycroft
	{ "SHER" },			// Old Sherman
	{ "CHMB" },			// Richard Chemist Stock boy
	{ "BARM" },			// Barman
	{ "DAND" },			// Dandy Player
	{ "ROUG" },			// Rough-looking Player
	{ "SPEC" },			// Spectator
	{ "HUNT" },			// Robert Hunt
	{ "VIOL" },			// Violet Secretary
	{ "PETT" },			// Pettigrew
	{ "APPL" },			// Augie (apple seller)
	{ "ANNA" },			// Anna Carroway
	{ "GUAR" },			// Guard
	{ "ANTO" },			// Antonio Caruso
	{ "TOBY" },			// Toby the Dog
	{ "KING" },			// Simon Kingsley
	{ "ALFR" },			// Alfred Tobacco Clerk
	{ "LADY" },			// Lady Brumwell
	{ "ROSA" },			// Madame Rosa
	{ "LADB" },			// Lady Brumwell
	{ "MOOR" },			// Joseph Moorehead
	{ "BEAL" },			// Mrs. Beale
	{ "LION" },			// Felix the Lion
	{ "HOLL" },			// Hollingston
	{ "CALL" },			// Constable Callaghan
	{ "JERE" },			// Sergeant Jeremy Duncan
	{ "LORD" },			// Lord Brumwell
	{ "NIGE" },			// Nigel Jameson
	{ "JONA" },			// Jonas (newspaper seller)
	{ "DUGA" },			// Constable Dugan
	{ "INSP" }			// Inspector Lestrade (Scotland Yard)
};

const char  *const NAMES[MAX_PEOPLE] = {
	"Sherlock Holmes",
	"Dr. Watson",
	"Inspector Lestrade",
	"Constable O'Brien",
	"Constable Lewis",
	"Sheila Parker",
	"Henry Carruthers",
	"Lesley",
	"An Usher",
	"An Usher",
	"Fredrick Epstein",
	"Mrs. Worthington",
	"The Coach",
	"A Player",
	"Tim",
	"James Sanders",
	"Belle",
	"Cleaning Girl",
	"Fredrick Epstein",
	"Wiggins",
	"Paul",
	"The Bartender",
	"A Dirty Drunk",
	"A Shouting Drunk",
	"A Staggering Drunk",
	"The Bouncer",
	"James Sanders",
	"The Coroner",
	"Reginald Snipes",
	"George Blackwood",
	"Lars",
	"Sheila Parker",
	"The Chemist",
	"Inspector Gregson",
	"Jacob Farthington",
	"Mycroft",
	"Old Sherman",
	"Richard",
	"The Barman",
	"A Dandy Player",
	"A Rough-looking Player",
	"A Spectator",
	"Robert Hunt",
	"Violet",
	"Pettigrew",
	"Augie",
	"Anna Carroway",
	"A Guard",
	"Antonio Caruso",
	"Toby the Dog",
	"Simon Kingsley",
	"Alfred",
	"Lady Brumwell",
	"Madame Rosa",
	"Lady Brumwell",
	"Joseph Moorehead",
	"Mrs. Beale",
	"Felix",
	"Hollingston",
	"Constable Callaghan",
	"Sergeant Duncan",
	"Lord Brumwell",
	"Nigel Jaimeson",
	"Jonas",
	"Constable Dugan",
	"Inspector Lestrade"
};

/*----------------------------------------------------------------*/

People::People(SherlockEngine *vm) : _vm(vm), _player(_data[0]) {
	_walkLoaded = false;
	_holmesOn = true;
	_oldWalkSequence = -1;
	_allowWalkAbort = false;
	_portraitLoaded = false;
	_portraitsOn = true;
	_clearingThePortrait = false;
	_srcZone = _destZone = 0;
	_talkPics = nullptr;
	_portraitSide = 0;
	_speakerFlip = false;
	_holmesFlip = false;
	_holmesQuotient = 0;
	_hSavedPos = Common::Point(-1, -1);
	_hSavedFacing = -1;

	_portrait._sequences = new byte[32];
}

People::~People() {
	if (_walkLoaded)
		delete _data[PLAYER]._images;
	delete _talkPics;
	delete[] _portrait._sequences;
}

/**
 * Reset the player data
 */
void People::reset() {
	// Note: The engine has theoretical support for two player charactersm but only the first one is used.
	// Watson is, instead, handled by a different sprite in each scene, with a very simple initial movement, if any
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

	// Reset any walk path in progress when Sherlock leaves scenes
	_walkTo.clear();
}

/**
 * Load the walking images for Sherlock
 */
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
	Map &map = *_vm->_map;
	Scene &scene = *_vm->_scene;
	int oldDirection, oldFrame;
	Common::Point speed, delta;

	// Flag that player has now walked in the scene
	scene._walkedInScene = true;

	// Stop any previous walking, since a new dest is being set
	_player._walkCount = 0;
	oldDirection = _player._sequenceNumber;
	oldFrame = _player._frameNumber;

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
	}

	// If a sufficient move is being done, then start the move
	if (delta.x > 3 || delta.y) {
		// See whether the major movement is horizontal or vertical
		if (delta.x >= delta.y) {
			// Set the initial frame sequence for the left and right, as well
			// as setting the delta x depending on direction
			if (_walkDest.x < (_player._position.x / 100)) {
				_player._sequenceNumber = (map._active ? (int)MAP_LEFT : (int)WALK_LEFT);
				_player._delta.x = speed.x * -100;
			} else {
				_player._sequenceNumber = (map._active ? (int)MAP_RIGHT : (int)WALK_RIGHT);
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
				if (!map._active) {
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
				if (!map._active) {
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
			if (map._active)
				_player._sequenceNumber = (oldDirection == -1) ? MAP_RIGHT : oldDirection;

			// Set the delta x
			_player._delta.x = (delta.x * 100) / (delta.y / speed.y);
			if (_walkDest.x < (_player._position.x / 100))
				_player._delta.x = -_player._delta.x;

			_player._walkCount = delta.y / speed.y;
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
	Map &map = *_vm->_map;
	_walkTo.clear();
	sprite._walkCount = 0;

	switch (sprite._sequenceNumber) {
	case WALK_UP:
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
	if (_oldWalkSequence != -1 || sprite._sequenceNumber == STOP_UP)
		sprite._frameNumber = 0;

	if (map._active) {
		sprite._sequenceNumber = 0;
		_player._position.x = (map[map._charPoint].x -  6) * 100;
		_player._position.y = (map[map._charPoint].x + 10) * 100;
	}

	_oldWalkSequence = -1;
	_allowWalkAbort = true;
}

/**
 * Walk to the co-ordinates passed, and then face the given direction
 */
void People::walkToCoords(const Common::Point &destPos, int destDir) {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;
	Talk &talk = *_vm->_talk;

	CursorId oldCursor = events.getCursor();
	events.setCursor(WAIT);

	_walkDest = Common::Point(destPos.x / 100 + 10, destPos.y / 100);
	_allowWalkAbort = true;
	goAllTheWay();

	// Keep calling doBgAnim until the walk is done
	do {
		events.pollEventsAndWait();
		scene.doBgAnim();
	} while (!_vm->shouldQuit() && _player._walkCount);

	if (!talk._talkToAbort) {
		// Put player exactly on destination position, and set direction
		_player._position = destPos;
		_player._sequenceNumber = destDir;
		gotoStand(_player);

		// Draw Holmes facing the new direction
		scene.doBgAnim();

		if (!talk._talkToAbort)
			events.setCursor(oldCursor);
	}
}

/**
 * Called to set the character walking to the current cursor location.
 * It uses the zones and the inter-zone points to determine a series
 * of steps to walk to get to that position.
 */
void People::goAllTheWay() {
	Scene &scene = *_vm->_scene;
	Common::Point srcPt(_player._position.x / 100 + _player.frameWidth() / 2,
		_player._position.y / 100);

	// Get the zone the player is currently in
	_srcZone = scene.whichZone(srcPt);
	if (_srcZone == -1)
		_srcZone = scene.closestZone(srcPt);

	// Get the zone of the destination
	_destZone = scene.whichZone(_walkDest);
	if (_destZone == -1) {
		_destZone = scene.closestZone(_walkDest);

		// The destination isn't in a zone
		if (_walkDest.x >= (SHERLOCK_SCREEN_WIDTH - 1))
			_walkDest.x = SHERLOCK_SCREEN_WIDTH - 2;

		// Trace a line between the centroid of the found closest zone to
		// the destination, to find the point at which the zone will be left
		const Common::Rect &destRect = scene._zones[_destZone];
		const Common::Point destCenter((destRect.left + destRect.right) / 2,
			(destRect.top + destRect.bottom) / 2);
		const Common::Point delta = _walkDest - destCenter;
		Common::Point pt(destCenter.x * 100, destCenter.y * 100);

		// Move along the line until the zone is left
		do {
			pt += delta;
		} while (destRect.contains(pt.x / 100, pt.y / 100));

		// Set the new walk destination to the last point that was in the
		// zone just before it was left
		_walkDest = Common::Point((pt.x - delta.x * 2) / 100,
			(pt.y - delta.y * 2) / 100);
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
			_walkTo.push(_walkDest);

			// Start walking
			_walkDest = _walkTo.pop();
			setWalking();
		}
	}
}

/**
 * Finds the scene background object corresponding to a specified speaker
 */
int People::findSpeaker(int speaker) {
	Scene &scene = *_vm->_scene;

	for (int idx = 0; idx < (int)scene._bgShapes.size(); ++idx) {
		Object &obj = scene._bgShapes[idx];

		if (obj._type == ACTIVE_BG_SHAPE) {
			Common::String name(obj._name.c_str(), obj._name.c_str() + 4);

			if (scumm_stricmp(PORTRAITS[speaker], name.c_str()) == 0
				&& obj._name[4] >= '0' && obj._name[4] <= '9')
				return idx;
		}
	}

	return -1;
}

/**
 * Turn off any currently active portraits, and removes them from being drawn
 */
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

/**
 * Setup the data for an animating speaker portrait at the top of the screen
 */
void People::setTalking(int speaker) {
	Resources &res = *_vm->_res;

	// If no speaker is specified, then we can exit immediately
	if (speaker == -1)
		return;

	if (_portraitsOn) {
		delete _talkPics;
		Common::String filename = Common::String::format("%s.vgs", PORTRAITS[speaker]);
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

/**
 * Synchronize the data for a savegame
 */
void People::synchronize(Common::Serializer &s) {
	s.syncAsByte(_holmesOn);
	s.syncAsSint16LE(_player._position.x);
	s.syncAsSint16LE(_player._position.y);
	s.syncAsSint16LE(_player._sequenceNumber);
	s.syncAsSint16LE(_holmesQuotient);

	if (s.isLoading()) {
		_hSavedPos = _player._position;
		_hSavedFacing = _player._sequenceNumber;
	}
}

} // End of namespace Sherlock
