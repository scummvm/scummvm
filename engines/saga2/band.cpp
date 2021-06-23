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

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/actor.h"
#include "saga2/band.h"
#include "saga2/savefile.h"
#include "saga2/dlist.h"

namespace Saga2 {

/* ===================================================================== *
   BandList class
 * ===================================================================== */

const int numBands = 32;

//  Manages the memory used for the Band's.  There will only be one
//  global instantiation of this class
class BandList {

	struct BandPlaceHolder : public DNode {
		uint8 buf[sizeof(Band)];

		Band *getBand(void) {
			return (Band *)&buf;
		}
	};

	DList                       list,       //  allocated Bands
	                            free;       //  unallocated Bands

	BandPlaceHolder             array[numBands];

public:
	//  Constructor -- initial construction
	BandList(void);

	//  Destructor
	~BandList(void);

	//  Reconstruct from an archive buffer
	void *restore(void *buf);

	//  Return the number of bytes necessary to archive this task list
	//  in a buffer
	int32 archiveSize(void);

	//  Create an archive of the task list in an archive buffer
	void *archive(void *buf);

	//  Place a Band from the inactive list into the active
	//  list.
	void *newBand(void);
	void *newBand(BandID id);

	//  Place a Band back into the inactive list.
	void deleteBand(void *p);

	//  Return the specified Band's ID
	BandID getBandID(Band *b) {
		BandPlaceHolder     *bp;

		warning("FIXME: BandPlaceHolder::getBandID(): unsafe pointer arithmetics");
		bp = ((BandPlaceHolder *)((uint8 *)b - offsetof(BandPlaceHolder, buf)));
		return bp - array;
	}

	//  Return a pointer to a Band given a BandID
	Band *getBandAddress(BandID id) {
		assert(id >= 0 && id < numBands);
		return array[id].getBand();
	}
};

//----------------------------------------------------------------------
//	BandList constructor -- simply place each element of the array in
//	the inactive list

BandList::BandList(void) {
	int i;

	for (i = 0; i < ARRAYSIZE(array); i++)
		free.addTail(array[i]);
}

//----------------------------------------------------------------------
//	BandList destructor

BandList::~BandList(void) {
	BandPlaceHolder     *bp;
	BandPlaceHolder     *nextBP;

	for (bp = (BandPlaceHolder *)list.first();
	        bp != NULL;
	        bp = nextBP) {
		//  Save the address of the next in the list
		nextBP = (BandPlaceHolder *)bp->next();

		delete bp->getBand();
	}
}

//----------------------------------------------------------------------
//	Reconstruct from an archive buffer

void *BandList::restore(void *buf) {
	int16               i,
	                    bandCount;

	//  Get the count of bands and increment the buffer pointer
	bandCount = *((int16 *)buf);
	buf = (int16 *)buf + 1;

	//  Iterate through the archive data, reconstructing the Bands
	for (i = 0; i < bandCount; i++) {
		BandID      id;

		//  Retreive the Band's id number
		id = *((BandID *)buf);
		buf = (BandID *)buf + 1;

		new (id)Band(&buf);
	}

	return buf;
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to archive this TaskList

int32 BandList::archiveSize(void) {
	int32               size = sizeof(int16);
	BandPlaceHolder     *bp;

	for (bp = (BandPlaceHolder *)list.first();
	        bp != NULL;
	        bp = (BandPlaceHolder *)bp->next())
		size += sizeof(BandID) + bp->getBand()->archiveSize();

	return size;
}

//----------------------------------------------------------------------
//	Make an archive of the BandList in an archive buffer

void *BandList::archive(void *buf) {
	int16               bandCount = 0;
	BandPlaceHolder     *bp;

	//  Count the active bands
	for (bp = (BandPlaceHolder *)list.first();
	        bp != NULL;
	        bp = (BandPlaceHolder *)bp->next())
		bandCount++;

	//  Store the band count in the archive buffer
	*((int16 *)buf) = bandCount;
	buf = (int16 *)buf + 1;

	//  Iterate through the bands, archiving each
	for (bp = (BandPlaceHolder *)list.first();
	        bp != NULL;
	        bp = (BandPlaceHolder *)bp->next()) {
		Band    *b = bp->getBand();

		//  Store the Band's id number
		*((BandID *)buf) = bp - array;
		buf = (BandID *)buf + 1;

		buf = b->archive(buf);
	}

	return buf;
}

//----------------------------------------------------------------------
//	Place a Band into the active list and return its address

void *BandList::newBand(void) {
	BandPlaceHolder     *bp;

	//  Grab a band holder from the inactive list
	bp = (BandPlaceHolder *)free.remHead();

	if (bp != NULL) {
		//  Place the place holder into the active list
		list.addTail(*bp);

		return bp->buf;
	}

	return NULL;
}

//----------------------------------------------------------------------
//	Place a specific Band into the active list and return its address

void *BandList::newBand(BandID id) {
	assert(id >= 0 && id < ARRAYSIZE(array));

	BandPlaceHolder     *bp;

	//  Grab the band place holder from the inactive list
	bp = (BandPlaceHolder *)&array[id];
	bp->remove();

	//  Place the place holder into the active list
	list.addTail(*bp);

	return bp->buf;
}

//----------------------------------------------------------------------
//	Remove the specified Band from the active list and place it back
//	into the inactive list

void BandList::deleteBand(void *p) {
	BandPlaceHolder     *bp;

	warning("FIXME: BandList::deleteBand(): unsafe pointer arithmetics");

	//  Convert the pointer to the Band to a pointer to the
	//  BandPlaceHolder
	bp = (BandPlaceHolder *)((uint8 *)p - offsetof(BandPlaceHolder, buf));

	//  Remove the band place holder from the active list
	bp->remove();

	//  Place it into the inactive list
	free.addTail(*bp);
}

/* ===================================================================== *
   Global BandList instantiation
 * ===================================================================== */

//	This is a statically allocated buffer large enough to hold a BandList.
//	The bandList is a BandList reference to this area of memory.  The
//	reason that I did this in this manner is to prevent the BandList
//	constructor from being called until it is expicitly called using an
//	overloaded new call.  The overloaded new call will simply return a
//	pointer to the bandListBuffer in order to construct the BandList in
//	place.

static uint8 bandListBuffer[sizeof(BandList)];

static BandList &bandList = *((BandList *)bandListBuffer);

/* ===================================================================== *
   Misc. band management functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Call the bandList member function newBand() to get a pointer to a
//	new Band

void *newBand(void) {
	return bandList.newBand();
}

void *newBand(BandID id) {
	return bandList.newBand(id);
}

//----------------------------------------------------------------------
//	Call the bandList member function deleteBand() to dispose of a
//	previously allocated Band

void deleteBand(void *p) {
	bandList.deleteBand(p);
}

//----------------------------------------------------------------------
//	Return the specified Band's ID

BandID getBandID(Band *b) {
	return bandList.getBandID(b);
}

//----------------------------------------------------------------------
//	Return a pointer to a Band given a BandID

Band *getBandAddress(BandID id) {
	return bandList.getBandAddress(id);
}

//----------------------------------------------------------------------
//	Initialize the bandList

void initBands(void) {
	//  Simply call the default constructor for the band list
	new (&bandList) BandList;
}

//----------------------------------------------------------------------
//	Save the BandList to save file

void saveBands(SaveFileConstructor &saveGame) {
	int32   archiveBufSize;
	void    *archiveBuffer;

	archiveBufSize = bandList.archiveSize();

	archiveBuffer = malloc(archiveBufSize);
	if (archiveBuffer == NULL)
		error("Unable to allocate band archive buffer");

	bandList.archive(archiveBuffer);

	saveGame.writeChunk(
	    MKTAG('B', 'A', 'N', 'D'),
	    archiveBuffer,
	    archiveBufSize);

	free(archiveBuffer);
}

//----------------------------------------------------------------------
//	Load the bandList from a save file

void loadBands(SaveFileReader &saveGame) {
	//  If there is no saved data, simply call the default constructor
	if (saveGame.getChunkSize() == 0) {
		new (&bandList) BandList;
		return;
	}

	void    *archiveBuffer;
	void    *bufferPtr;

	archiveBuffer = malloc(saveGame.getChunkSize());
	if (archiveBuffer == NULL)
		error("Unable to allocate task archive buffer");

	//  Read the archived task data
	saveGame.read(archiveBuffer, saveGame.getChunkSize());

	bufferPtr = archiveBuffer;

	//  Reconstruct taskList from archived data
	new (&bandList) BandList;
	bandList.restore(bufferPtr);

	free(archiveBuffer);
}

//----------------------------------------------------------------------
//	Cleanup the bandList

void cleanupBands(void) {
	//  Simply call the bandList's destructor
	bandList.~BandList();
}

/* ===================================================================== *
   Band member functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Constructor -- reconstruct from archive buffer

Band::Band(void **buf) {
	void        *bufferPtr = *buf;
	int16       i;

	//  Restore the leader pointer
	assert(isActor(*((ObjectID *)bufferPtr)));
	leader = (Actor *)GameObject::objectAddress(*((ObjectID *)bufferPtr));
	bufferPtr = (ObjectID *)bufferPtr + 1;

	//  Restore the member count
	assert(*((int16 *)bufferPtr) < ARRAYSIZE(members));
	memberCount = *((int16 *)bufferPtr);
	bufferPtr = (int16 *)bufferPtr + 1;

	//  Restore the member pointers
	for (i = 0; i < memberCount; i++) {
		assert(isActor(*((ObjectID *)bufferPtr)));
		members[i] = (Actor *)GameObject::objectAddress(
		                   *((ObjectID *)bufferPtr));
		bufferPtr = (ObjectID *)bufferPtr + 1;
	}

	*buf = bufferPtr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a
//	buffer

int32 Band::archiveSize(void) {
	return      sizeof(ObjectID)                     //  leader ID
	            +   sizeof(memberCount)
	            +   sizeof(ObjectID) * memberCount;      //  members' ID's
}

//----------------------------------------------------------------------
//	Archive this object in a buffer

void *Band::archive(void *buf) {
	int16       i;

	//  Store the leader's ID
	*((ObjectID *)buf) = leader->thisID();
	buf = (ObjectID *)buf + 1;

	//  Store the member count
	*((int16 *)buf) = memberCount;
	buf = (int16 *)buf + 1;

	//  Store the members' ID's
	for (i = 0; i < memberCount; i++) {
		*((ObjectID *)buf) = members[i]->thisID();
		buf = (ObjectID *)buf + 1;
	}

	return buf;
}

} // end of namespace Saga2
