/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/actor.h"
#include "saga2/band.h"

namespace Saga2 {

//----------------------------------------------------------------------
//	BandList constructor -- simply place each element of the array in
//	the inactive list

BandList::BandList() {
	for (int i = 0; i < kNumBands; i++)
		_list[i] = nullptr;
}

//----------------------------------------------------------------------
//	BandList destructor

BandList::~BandList() {
	for (int i = 0; i < kNumBands; i++)
		delete _list[i];
}

void BandList::read(Common::InSaveFile *in) {
	int16 bandCount;
	//  Get the count of bands and increment the buffer pointer
	bandCount = in->readSint16LE();

	//  Iterate through the archive data, reconstructing the Bands
	for (int i = 0; i < bandCount; i++) {
		BandID      id;

		//  Retrieve the Band's id number
		id = in->readSint16LE();
		debugC(3, kDebugSaveload, "Loading Band %d", id);

		_list[id] = new Band(in);
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes necessary to archive this TaskList

int32 BandList::archiveSize() {
	int32               size = sizeof(int16);

	for (int i = 0; i < kNumBands; i++)
		if (_list[i])
			size += sizeof(BandID) + _list[i]->archiveSize();

	return size;
}

void BandList::write(Common::MemoryWriteStreamDynamic *out) {
	int16 bandCount = 0;

	//  Count the active bands
	for (int i = 0; i < kNumBands; i++)
		if (_list[i])
			bandCount++;

	//  Store the band count in the archive buffer
	out->writeSint16LE(bandCount);
	debugC(3, kDebugSaveload, "... bandCount = %d", bandCount);

	//  Iterate through the bands, archiving each
	for (int i = 0; i < kNumBands; i++) {
		if (_list[i]) {
			//  Store the Band's id number
			out->writeSint16LE(i);
			debugC(3, kDebugSaveload, "Saving Band %d", i);

			_list[i]->write(out);
		}
	}
}

//----------------------------------------------------------------------
//	Place a Band into the active list and return its address

Band *BandList::newBand() {
	for (int i = 0; i < kNumBands; i++) {
		if (!_list[i]) {
			_list[i] = new Band();

			return _list[i];
		}
	}

	return nullptr;
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
		if (_list[i] == b) {
			warning("Band %d (%p) already added", i, (void *)b);

			return;
		}
	}

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

Band *newBand() {
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
	if (g_vm->_bandList == nullptr)
		return nullptr;

	return g_vm->_bandList->getBandAddress(id);
}

//----------------------------------------------------------------------
//	Initialize the bandList

void initBands() {
}

void saveBands(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving Bands");


	outS->write("BAND", 4);
	CHUNK_BEGIN;
	g_vm->_bandList->write(out);
	CHUNK_END;
}

void loadBands(Common::InSaveFile *in, int32 chunkSize) {
	debugC(2, kDebugSaveload, "Loading Bands");

	//  If there is no saved data, simply call the default constructor
	if (chunkSize == 0) {
		g_vm->_bandList = new BandList;
		return;
	}

	//  Reconstruct taskList from archived data
	g_vm->_bandList = new BandList;
	g_vm->_bandList->read(in);

	// Reconstruct followers for actors
	for (int i = 0; i < kActorCount; ++i) {
		BandID id = g_vm->_act->_actorList[i]->_followersID;
		g_vm->_act->_actorList[i]->_followers = id != NoBand
	                             ?   getBandAddress(id)
	                             :   nullptr;
	}
}

//----------------------------------------------------------------------
//	Cleanup the bandList

void cleanupBands() {
	for (int i = 0; i < BandList::kNumBands; i++) {
		if (g_vm->_bandList->_list[i]) {
			delete g_vm->_bandList->_list[i];
			g_vm->_bandList->_list[i] = nullptr;
		}
	}
}

/* ===================================================================== *
   Band member functions
 * ===================================================================== */

Band::Band() : _leader(nullptr), _memberCount(0) {
	g_vm->_bandList->addBand(this);

	for (int i = 0; i < kMaxBandMembers; i++)
		_members[i] = nullptr;
}
Band::Band(Actor *l) : _leader(l), _memberCount(0) {
	g_vm->_bandList->addBand(this);

	for (int i = 0; i < kMaxBandMembers; i++)
		_members[i] = nullptr;
}

Band::Band(Common::InSaveFile *in) {
	ObjectID leaderID = in->readUint16LE();

	//  Restore the leader pointer
	assert(isActor(leaderID));
	_leader = (Actor *)GameObject::objectAddress(leaderID);

	debugC(4, kDebugSaveload, "... leaderID = %d", leaderID);

	//  Restore the member count
	_memberCount = in->readSint16LE();
	assert(_memberCount < ARRAYSIZE(_members));

	debugC(4, kDebugSaveload, "... _memberCount = %d", _memberCount);

	for (int i = 0; i < kMaxBandMembers; i++)
		_members[i] = nullptr;

	//  Restore the member pointers
	for (int i = 0; i < _memberCount; i++) {
		ObjectID id = in->readUint16LE();
		assert(isActor(id));
		_members[i] = (Actor *)GameObject::objectAddress(id);

		debugC(4, kDebugSaveload , "... id = %d", id);
	}
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in a
//	buffer

int32 Band::archiveSize() {
	return      sizeof(ObjectID)                     //  leader ID
	            +   sizeof(_memberCount)
	            +   sizeof(ObjectID) * _memberCount;      //  members' ID's
}

void Band::write(Common::MemoryWriteStreamDynamic *out) {
	//  Store the leader's ID
	out->writeUint16LE(_leader->thisID());

	debugC(4, kDebugSaveload, "... _leader->thisID() = %d", _leader->thisID());

	//  Store the member count
	out->writeSint16LE(_memberCount);

	debugC(4, kDebugSaveload, "... _memberCount = %d", _memberCount);

	//  Store the members' ID's
	for (int i = 0; i < _memberCount; i++) {
		out->writeUint16LE(_members[i]->thisID());
		debugC(4, kDebugSaveload, "... _members[%d]->thisID() = %d", i, _members[i]->thisID());
	}
}

} // end of namespace Saga2
