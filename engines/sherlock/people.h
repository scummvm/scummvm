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
#include "common/queue.h"
#include "sherlock/objects.h"
#include "sherlock/saveload.h"

namespace Sherlock {

enum PeopleId {
	PLAYER			= 0,
	MAX_NPC			= 5,
	MAX_NPC_PATH	= 200
};

enum {
	MAP_UP = 1, MAP_UPRIGHT = 2, MAP_RIGHT = 1, MAP_DOWNRIGHT = 4,
	MAP_DOWN = 5, MAP_DOWNLEFT = 6, MAP_LEFT = 2, MAP_UPLEFT = 8
};

#define NUM_IN_WALK_LIB 10
extern const char *const WALK_LIB_NAMES[10];

#define MAX_CHARACTERS (IS_SERRATED_SCALPEL ? 1 : 6)

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
	bool _npcMoved;
	bool _resetNPCPath;
	int _savedNpcSequence;
	int _savedNpcFrame;
	int _tempX;
	int _tempScaleVal;
	bool _updateNPCPath;

	// Rose Tattoo fields
	Common::String _walkVGSName;		// Name of walk library person is using
public:
	Person();
	virtual ~Person() {}

	/**
	 * Clear the NPC related data
	 */
	void clearNPC();

	/**
	 * Update the NPC
	 */
	void updateNPC();
};

class SherlockEngine;

class People {
protected:
	SherlockEngine *_vm;
	Common::Array<Person *> _data;
	int _oldWalkSequence;
	int _srcZone, _destZone;

	People(SherlockEngine *vm);
public:
	Common::Array<PersonData> _characters;
	ImageFile *_talkPics;
	Common::Point _walkDest;
	Point32 _hSavedPos;
	int _hSavedFacing;
	Common::Queue<Common::Point> _walkTo;
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
	static People *init(SherlockEngine *vm);
	virtual ~People();

	Person &operator[](PeopleId id) { return *_data[id]; }
	Person &operator[](int idx) { return *_data[idx]; }

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
	 * Walk to the co-ordinates passed, and then face the given direction
	 */
	void walkToCoords(const Point32 &destPos, int destDir);

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
	 * Synchronize the data for a savegame
	 */
	virtual void synchronize(Serializer &s) = 0;

	/**
	 * Change the sequence of the scene background object associated with the current speaker.
	 */
	virtual void setTalkSequence(int speaker, int sequenceNum = 1) = 0;

	/**
	 * Bring a moving character to a standing position. If the Scalpel chessboard
	 * is being displayed, then the chraracter will always face down.
	 */
	virtual void gotoStand(Sprite &sprite) = 0;
};

} // End of namespace Sherlock

#endif
