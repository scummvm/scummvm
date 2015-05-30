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

#ifndef SHERLOCK_PEOPLE_H
#define SHERLOCK_PEOPLE_H

#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/queue.h"
#include "sherlock/objects.h"

namespace Sherlock {

enum PeopleId {
	PLAYER			= 0,
	AL				= 0,
	PEG				= 1,
	MAX_CHARACTERS		= 6,
	MAX_NPC			= 5,
	MAX_NPC_PATH	= 200
};

// Animation sequence identifiers for characters
enum {
	WALK_RIGHT = 0, WALK_DOWN = 1, WALK_LEFT = 2, WALK_UP = 3, STOP_LEFT = 4,
	STOP_DOWN = 5, STOP_RIGHT = 6, STOP_UP = 7, WALK_UPRIGHT = 8,
	WALK_DOWNRIGHT = 9, WALK_UPLEFT = 10, WALK_DOWNLEFT = 11,
	STOP_UPRIGHT = 12, STOP_UPLEFT = 13, STOP_DOWNRIGHT = 14,
	STOP_DOWNLEFT = 15, TALK_RIGHT = 6, TALK_LEFT = 4
};

enum {
	MAP_UP = 1, MAP_UPRIGHT = 2, MAP_RIGHT = 1, MAP_DOWNRIGHT = 4,
	MAP_DOWN = 5, MAP_DOWNLEFT = 6, MAP_LEFT = 2, MAP_UPLEFT = 8
};

struct PersonData {
	const char *_name;
	const char *_portrait;
	const byte *_stillSequences;
	const byte *_talkSequences;

	PersonData(const char *name, const char *portrait, const byte *stillSequences, const byte *talkSequences) :
		_name(name), _portrait(portrait), _stillSequences(stillSequences), _talkSequences(talkSequences) {}
};

class Person : public Sprite {
public:
	bool _walkLoaded;
	Common::String _portrait;

	// NPC related fields
	int _npcIndex;
	int _npcStack;
	bool _npcPause;
	byte _npcPath[MAX_NPC_PATH];
	Common::String _npcName;
	int _tempX;
	int _tempScaleVal;

	// Rose Tattoo fields
	Common::String _walkVGSName;		// Name of walk library person is using
public:
	Person();

	/**
	 * Clear the NPC related data
	 */
	void clearNPC();
};

class SherlockEngine;

class People {
private:
	SherlockEngine *_vm;
	Person _data[MAX_CHARACTERS];
	int _oldWalkSequence;
	int _srcZone, _destZone;
public:
	Common::Array<PersonData> _characters;
	ImageFile *_talkPics;
	Common::Point _walkDest;
	Common::Point _hSavedPos;
	int _hSavedFacing;
	Common::Queue<Common::Point> _walkTo;
	Person &_player;
	bool _holmesOn;
	bool _portraitLoaded;
	bool _portraitsOn;
	Object _portrait;
	bool _clearingThePortrait;
	bool _allowWalkAbort;
	int _portraitSide;
	bool _speakerFlip;
	bool _holmesFlip;
	int _holmesQuotient;
	bool _forceWalkReload;
	bool _useWalkLib;

	int _walkControl;
public:
	People(SherlockEngine *vm);
	~People();

	Person &operator[](PeopleId id) {
		assert(id < MAX_CHARACTERS);
		return _data[id];
	}
	Person &operator[](int idx) {
		assert(idx < MAX_CHARACTERS);
		return _data[idx];
	}

	/**
	 * Reset the player data
	 */
	void reset();

	/**
	 * Load the walking images for Sherlock
	 */
	bool loadWalk();

	/**
	 * If the walk data has been loaded, then it will be freed
	 */
	bool freeWalk();

	/**
	 * Set the variables for moving a character from one poisition to another
	 * in a straight line - goAllTheWay must have been previously called to
	 * check for any obstacles in the path.
	 */
	void setWalking();

	/**
	 * Bring a moving character to a standing position. If the Scalpel chessboard
	 * is being displayed, then the chraracter will always face down.
	 */
	void gotoStand(Sprite &sprite);

	/**
	 * Walk to the co-ordinates passed, and then face the given direction
	 */
	void walkToCoords(const Common::Point &destPos, int destDir);

	/**
	 * Called to set the character walking to the current cursor location.
	 * It uses the zones and the inter-zone points to determine a series
	 * of steps to walk to get to that position.
	 */
	void goAllTheWay();

	/**
	 * Finds the scene background object corresponding to a specified speaker
	 */
	int findSpeaker(int speaker);

	/**
	 * Turn off any currently active portraits, and removes them from being drawn
	 */
	void clearTalking();

	/**
	 * Setup the data for an animating speaker portrait at the top of the screen
	 */
	void setTalking(int speaker);

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Common::Serializer &s);
};

} // End of namespace Sherlock

#endif
