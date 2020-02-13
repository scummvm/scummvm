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

#ifndef SHERLOCK_SCALPEL_PEOPLE_H
#define SHERLOCK_SCALPEL_PEOPLE_H

#include "common/scummsys.h"
#include "sherlock/people.h"

namespace Sherlock {

class SherlockEngine;

namespace Scalpel {

// Animation sequence identifiers for characters
enum ScalpelSequences {
	WALK_RIGHT = 0, WALK_DOWN = 1, WALK_LEFT = 2, WALK_UP = 3, STOP_LEFT = 4,
	STOP_DOWN = 5, STOP_RIGHT = 6, STOP_UP = 7, WALK_UPRIGHT = 8,
	WALK_DOWNRIGHT = 9, WALK_UPLEFT = 10, WALK_DOWNLEFT = 11,
	STOP_UPRIGHT = 12, STOP_UPLEFT = 13, STOP_DOWNRIGHT = 14,
	STOP_DOWNLEFT = 15, TALK_RIGHT = 6, TALK_LEFT = 4
};

class ScalpelPerson : public Person {
public:
	ScalpelPerson() : Person() {}
	~ScalpelPerson() override {}

	/**
	 * Synchronize the data for a savegame
	 */
	virtual void synchronize(Serializer &s);

	/**
	* This adjusts the sprites position, as well as its animation sequence:
	*/
	void adjustSprite() override;

	/**
	 * Bring a moving character to a standing position
	 */
	void gotoStand() override;

	/**
	 * Set the variables for moving a character from one poisition to another
	 * in a straight line
	 */
	void setWalking() override;

	/**
	 * Walk to the co-ordinates passed, and then face the given direction
	 */
	void walkToCoords(const Point32 &destPos, int destDir) override;

	/**
	 * Get the source position for a character potentially affected by scaling
	 */
	Common::Point getSourcePoint() const override;
};

class ScalpelPeople : public People {
public:
	ScalpelPeople(SherlockEngine *vm);
	~ScalpelPeople() override {}

	ScalpelPerson &operator[](PeopleId id) { return *(ScalpelPerson  *)_data[id]; }
	ScalpelPerson  &operator[](int idx) { return *(ScalpelPerson  *)_data[idx]; }

	/**
	 * Setup the data for an animating speaker portrait at the top of the screen
	 */
	void setTalking(int speaker);

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s) override;

	/**
	 * Change the sequence of the scene background object associated with the specified speaker.
	 */
	void setTalkSequence(int speaker, int sequenceNum = 1) override;

	/**
	 * Restrict passed point to zone using Sherlock's positioning rules
	 */
	const Common::Point restrictToZone(int zoneId, const Common::Point &destPos) override;

	/**
	 * Load the walking images for Sherlock
	 */
	bool loadWalk() override;

	/**
	 * If the specified speaker is a background object, it will set it so that it uses
	 * the Listen Sequence (specified by the sequence number). If the current sequence
	 * has an Allow Talk Code in it, the _gotoSeq field will be set so that the object
	 * begins listening as soon as it hits the Allow Talk Code. If there is no Abort Code,
	 * the Listen Sequence will begin immediately.
	 * @param speaker		Who is speaking
	 * @param sequenceNum	Which listen sequence to use
	 */
	void setListenSequence(int speaker, int sequenceNum = 1) override;
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
