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

#include "saga2/saga2.h"
#include "saga2/actor.h"
#include "saga2/band.h"
#include "saga2/savefile.h"

namespace Saga2 {

/* ===================================================================== *
   BandList class
 * ===================================================================== */

const int kNumBands = 32;

//  Manages the memory used for the Band's.  There will only be one
//  global instantiation of this class
class BandList {
public:
	Band *_list[kNumBands];

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
	Band *newBand(void);
	Band *newBand(BandID id);

	void addBand(Band *band);

	//  Place a Band back into the inactive list.
	void deleteBand(Band *p);

	//  Return the specified Band's ID
	BandID getBandID(Band *b) {
		for (int i = 0; i < kNumBands; i++)
			if (_list[i] == b)
				return i;

		error("BandList::getBandID(): Unknown band");
	}

	//  Return a pointer to a Band given a BandID
	Band *getBandAddress(BandID id) {
		assert(id >= 0 && id < kNumBands);
		return _list[id];
	}
};

//----------------------------------------------------------------------
//	BandList constructor -- simply place each element of the array in
//	the inactive list

BandList::BandList(void) {
	for (int i = 0; i < kNumBands; i++)
		_list[i] = nullptr;
}

//----------------------------------------------------------------------
//	BandList destructor

BandList::~BandList(void) {
	for (int i = 0; i < kNumBands; i++)
		delete _list[i];
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

		_list[id] = new Band(&buf);
	}

	return buf;
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to archive this TaskList

int32 BandList::archiveSize(void) {
	int32               size = sizeof(int16);

	for (int i = 0; i < kNumBands; i++)
		if (_list[i])
			size += sizeof(BandID) + _list[i]->archiveSize();

	return size;
}

//----------------------------------------------------------------------
//	Make an archive of the BandList in an archive buffer

void *BandList::archive(void *buf) {
	int16 bandCount = 0;

	//  Count the active bands
	for (int i = 0; i < kNumBands; i++)
		if (_list[i])
			bandCount++;

	//  Store the band count in the archive buffer
	*((int16 *)buf) = bandCount;
	buf = (int16 *)buf + 1;

	//  Iterate through the bands, archiving each
	for (int i = 0; i < kNumBands; i++) {
		if (_list[i]) {
			//  Store the Band's id number
			*((BandID *)buf) = i;
			buf = (BandID *)buf + 1;

			buf = _list[i]->archive(buf);
		}
	}

	return buf;
}

//----------------------------------------------------------------------
//	Place a Band into the active list and return its address

Band *BandList::newBand(void) {
	for (int i = 0; i < kNumBands; i++) {
		if (!_list[i]) {
			_list[i] = new Band();

			return _list[i];
		}
	}

	return NULL;
}

//----------------------------------------------------------------------
//	Place a specific Band into the active list and return its address

Band *BandList::newBand(BandID id) {
	assert(id >= 0 && id < kNumBands);

	if (_list[id])
		delete _list[id];

	_list[id] = new Band();

	return _list[id];
}

 void BandList::addBand(Band *b) {
	for (int i = 0; i < kNumBands; i++) {
		if (!_list[i]) {
			_list[i] = b;

			return;
		}
	}

	error("BandList::addBand(): Too many bands, > %d", kNumBands);
}

//----------------------------------------------------------------------
//	Remove the specified Band from the active list and place it back
//	into the inactive list

void BandList::deleteBand(Band *p) {
	int id = getBandID(p);

	_list[id] = nullptr;
}

/* ===================================================================== *
   Misc. band management functions
 * ===================================================================== */

//----------------------------------------------------------------------
//	Call the bandList member function newBand() to get a pointer to a
//	new Band

Band *newBand(void) {
	return g_vm->_bandList->newBand();
}

Band *newBand(BandID id) {
	return g_vm->_bandList->newBand(id);
}

//----------------------------------------------------------------------
//	Call the bandList member function deleteBand() to dispose of a
//	previously allocated Band

void deleteBand(Band *p) {
	g_vm->_bandList->deleteBand(p);
}

//----------------------------------------------------------------------
//	Return the specified Band's ID

BandID getBandID(Band *b) {
	return g_vm->_bandList->getBandID(b);
}

//----------------------------------------------------------------------
//	Return a pointer to a Band given a BandID

Band *getBandAddress(BandID id) {
	return g_vm->_bandList->getBandAddress(id);
}

//----------------------------------------------------------------------
//	Initialize the bandList

void initBands(void) {
	g_vm->_bandList = new BandList();
}

//----------------------------------------------------------------------
//	Save the BandList to save file

void saveBands(SaveFileConstructor &saveGame) {
	int32   archiveBufSize;
	void    *archiveBuffer;

	archiveBufSize = g_vm->_bandList->archiveSize();

	archiveBuffer = malloc(archiveBufSize);
	if (archiveBuffer == NULL)
		error("Unable to allocate band archive buffer");

	g_vm->_bandList->archive(archiveBuffer);

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
		g_vm->_bandList = new BandList;
		return;
	}

	void *archiveBuffer;
	void *bufferPtr;

	archiveBuffer = malloc(saveGame.getChunkSize());
	if (archiveBuffer == NULL)
		error("Unable to allocate task archive buffer");

	//  Read the archived task data
	saveGame.read(archiveBuffer, saveGame.getChunkSize());

	bufferPtr = archiveBuffer;

	//  Reconstruct taskList from archived data
	g_vm->_bandList = new BandList;
	g_vm->_bandList->restore(bufferPtr);

	free(archiveBuffer);
}

//----------------------------------------------------------------------
//	Cleanup the bandList

void cleanupBands(void) {
	delete g_vm->_bandList;
	g_vm->_bandList = nullptr;
}

/* ===================================================================== *
   Band member functions
 * ===================================================================== */

Band::Band() : leader(nullptr), memberCount(0) {
	g_vm->_bandList->addBand(this);
}
Band::Band(Actor *l) : leader(l), memberCount(0) {
	g_vm->_bandList->addBand(this);
}

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

	g_vm->_bandList->addBand(this);
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
