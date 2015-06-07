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

#ifndef SHERLOCK_TATTOO_PEOPLE_H
#define SHERLOCK_TATTOO_PEOPLE_H

#include "common/scummsys.h"
#include "sherlock/people.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

// Animation sequence identifiers for characters
enum TattooSequences {
	// Walk Sequences Numbers for NPCs
	WALK_UP			= 0,
	WALK_UPRIGHT		= 1,
	WALK_RIGHT		= 2,
	WALK_DOWNRIGHT	= 3,
	WALK_DOWN		= 4,
	WALK_DOWNLEFT	= 5,
	WALK_LEFT		= 6,
	WALK_UPLEFT		= 7,

	// Stop Sequences Numbers for NPCs
	STOP_UP			= 8,
	STOP_UPRIGHT		= 9,
	STOP_RIGHT		= 10,
	STOP_DOWNRIGHT	= 11,
	STOP_DOWN		= 12,
	STOP_DOWNLEFT	= 13,
	STOP_LEFT		= 14,
	STOP_UPLEFT		= 15,

	// NPC Talk Sequence Numbers
	TALK_UPRIGHT		= 16,
	TALK_RIGHT		= 17,
	TALK_DOWNRIGHT	= 18,
	TALK_DOWNLEFT	= 19,
	TALK_LEFT		= 20,
	TALK_UPLEFT		= 21,

	// NPC Listen Sequence Numbers
	LISTEN_UPRIGHT	= 22,      
	LISTEN_RIGHT		= 23,
	LISTEN_DOWNRIGHT	= 24,
	LISTEN_DOWNLEFT	= 25,
	LISTEN_LEFT		= 26,
	LISTEN_UPLEFT	= 27
};

class TattooPeople : public People {
public:
	TattooPeople(SherlockEngine *vm) : People(vm) {}
	virtual ~TattooPeople() {}

	/**
	 * If the specified speaker is a background object, it will set it so that it uses 
	 * the Listen Sequence (specified by the sequence number). If the current sequence 
	 * has an Allow Talk Code in it, the _gotoSeq field will be set so that the object 
	 * begins listening as soon as it hits the Allow Talk Code. If there is no Abort Code, 
	 * the Listen Sequence will begin immediately.
	 * @param speaker		Who is speaking
	 * @param sequenceNum	Which listen sequence to use
	 */
	void setListenSequence(int speaker, int sequenceNum);

	/**
	 * If the specified speaker is a background object, this will set it so that it uses 
	 * the Talk Sequence specified. If the current sequence has an Allow Talk Code in it,
	 * _gotoSeq will be set so that the object begins talking as soon as it hits the 
	 * Allow Talk Code. If there is no Abort Code, the Talk Sequence will begin immediately.                                  
	 */
	void setTalkSequence(int speaker, int sequenceNum);
};

} // End of namespace Scalpel

} // End of namespace Sherlock


#endif
