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
	HOLMES			= 0,
	WATSON			= 1,
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
protected:
	/**
	 * Get the source position for a character potentially affected by scaling
	 */
	virtual Common::Point getSourcePoint() const = 0;
public:
	Common::Queue<Common::Point> _walkTo;
	int _srcZone, _destZone;
	bool _walkLoaded;
	Common::String _portrait;
	Common::Point _walkDest;
	Common::String _npcName;

	// Rose Tattoo fields
	Common::String _walkVGSName;		// Name of walk library person is using
public:
	Person();
	~Person() override {}

	/**
	 * Called to set the character walking to the current cursor location.
	 * It uses the zones and the inter-zone points to determine a series
	 * of steps to walk to get to that position.
	 */
	void goAllTheWay();

	/**
	 * Walk to the co-ordinates passed, and then face the given direction
	 */
	virtual void walkToCoords(const Point32 &destPos, int destDir) = 0;

	/**
	 * Center the visible screen so that the person is in the center of the screen
	 */
	virtual void centerScreenOnPerson() {}
};

class SherlockEngine;

class People {
protected:
	SherlockEngine *_vm;
	Common::Array<Person *> _data;

	People(SherlockEngine *vm);
public:
	Common::Array<PersonData> _characters;
	ImageFile *_talkPics;
	PositionFacing _savedPos;
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
	 * If the walk data has been loaded, then it will be freed
	 */
	bool freeWalk();

	/**
	 * Turn off any currently active portraits, and removes them from being drawn
	 */
	void clearTalking();

	/**
	* Finds the scene background object corresponding to a specified speaker
	*/
	virtual int findSpeaker(int speaker);

	/**
	 * Synchronize the data for a savegame
	 */
	virtual void synchronize(Serializer &s) = 0;

	/**
	 * Change the sequence of the scene background object associated with the current speaker.
	 */
	virtual void setTalkSequence(int speaker, int sequenceNum = 1) = 0;

	/**
	 * Load the walking images for Sherlock
	 */
	virtual bool loadWalk() = 0;

	/**
	 * Restrict passed point to zone using Sherlock's positioning rules
	 */
	virtual const Common::Point restrictToZone(int zoneId, const Common::Point &destPos) = 0;

	/**
	 * If the specified speaker is a background object, it will set it so that it uses
	 * the Listen Sequence (specified by the sequence number). If the current sequence
	 * has an Allow Talk Code in it, the _gotoSeq field will be set so that the object
	 * begins listening as soon as it hits the Allow Talk Code. If there is no Abort Code,
	 * the Listen Sequence will begin immediately.
	 * @param speaker		Who is speaking
	 * @param sequenceNum	Which listen sequence to use
	 */
	virtual void setListenSequence(int speaker, int sequenceNum = 1) = 0;
};

} // End of namespace Sherlock

#endif
