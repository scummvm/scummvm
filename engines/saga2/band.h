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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_BAND_H
#define SAGA2_BAND_H

namespace Saga2 {

class Actor;
class Band;
const int       maxBandMembers = 32;

/* ===================================================================== *
   Function prototypes
 * ===================================================================== */

//  Allocate a new band
Band *newBand(void);
Band *newBand(BandID id);

//  Delete a previously allocated band
void deleteBand(Band *p);

//  Get a band's ID given its address
BandID getBandID(Band *b);
//  Get a band's address given its ID
Band *getBandAddress(BandID id);

//  Initialize the band list
void initBands(void);
//  Save the active band structures in a save file
void saveBands(SaveFileConstructor &saveGame);
//  Load the band structures from a save file
void loadBands(SaveFileReader &saveGame);
//  Cleanup the band list
void cleanupBands(void);

/* ===================================================================== *
   Band class
 * ===================================================================== */

class Band {
	Actor       *leader;

	int16       memberCount;
	Actor       *members[maxBandMembers];

public:

	Band();
	Band(Actor *l);

	//  Constructor -- reconstruct from archive buffer
	Band(void **buf);

	~Band() { deleteBand(this); }

	//  Return the number of bytes needed to archive this object in a
	//  buffer
	int32 archiveSize(void);

	//  Archive this object in a buffer
	void *archive(void *buf);

	Actor *getLeader(void) {
		return leader;
	}

	bool add(Actor *newMember) {
		if (memberCount < ARRAYSIZE(members)) {
			members[memberCount++] = newMember;
			return true;
		} else
			return false;
	}

	void remove(Actor *member) {
		int     i;

		for (i = 0; i < memberCount; i++) {
			if (members[i] == member) {
				memberCount--;

				for (; i < memberCount; i++)
					members[i] = members[i + 1];

				break;
			}
		}
	}

	void remove(int index) {
		assert(index < memberCount);

		int     i;

		memberCount--;

		for (i = index; i < memberCount; i++)
			members[i] = members[i + 1];
	}

	int size(void) {
		return memberCount;
	}
	Actor *const &operator [](int index) {
		return members[index];
	}
};

} // end of namespace Saga2

#endif
