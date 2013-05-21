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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "voyeur/files.h"

namespace Voyeur {

FilesManager::FilesManager() {
	_decompressSize = 0x7000;
}

bool FilesManager::openBoltLib(const Common::String &filename, BoltFile *&boltFile) {
	if (boltFile != NULL) {
		_curLibPtr = boltFile;
		return true;
	}

	// TODO: Specific library classes for buoy.blt versus stampblt.blt 
	// Create the bolt file interface object and load the index
	boltFile = _curLibPtr = new BoltFile();
	return true;
}

/*------------------------------------------------------------------------*/

#define BOLT_GROUP_SIZE 16

BoltFile *BoltFile::_curLibPtr = NULL;
BoltGroup *BoltFile::_curGroupPtr = NULL;
BoltEntry *BoltFile::_curMemberPtr = NULL;
byte *BoltFile::_curMemInfoPtr = NULL;
int BoltFile::_fromGroupFlag = 0;
byte BoltFile::_xorMask = 0;
bool BoltFile::_encrypt = false;

BoltFile::BoltFile() {
	if (!_curFd.open("bvoy.blt"))
		error("Could not open buoy.blt");
	_curFilePosition = 0;

	// Read in the file header
	byte header[16];
	_curFd.read(&header[0], 16);

	if (strncmp((const char *)&header[0], "BOLT", 4) != 0)
		error("Tried to load non-bolt file");

	int totalGroups = header[11] ? header[11] : 0x100;
	for (int i = 0; i < totalGroups; ++i)
		_groups.push_back(BoltGroup(&_curFd));
}

BoltFile::~BoltFile() {
	_curFd.close();
}

bool BoltFile::getBoltGroup(uint32 id) {
	++_fromGroupFlag;
	_curLibPtr = this;
	_curGroupPtr = &_groups[(id >> 8) & 0xff];
	int count = _curGroupPtr->_count ? _curGroupPtr->_count : 256;

	if (_curGroupPtr->_groupPtr) {
		// Group already loaded
		_curMemInfoPtr = _curGroupPtr->_groupPtr;
	} else {
		// Load the group
		_curGroupPtr->load();
	}

	if (_curGroupPtr->_callInitGro)
		initGro();

	if ((id >> 16) != 0) {
		id &= 0xff00;
		for (int idx = 0; idx < count; ++idx, ++id) {
			byte *member = getBoltMember(id);
			assert(member);
		}
	} else if (!_curGroupPtr->_loaded) {
		_curGroupPtr->load();
	}

	resolveAll();
	--_fromGroupFlag;
	return true;
}

byte *BoltFile::memberAddr(uint32 id) {
	BoltGroup &group = _groups[id >> 8];
	if (!group._loaded)
		return NULL;

	return group._entries[id & 0xff]._data;
}

byte *BoltFile::getBoltMember(uint32 id) {
	_curLibPtr = this;

	// Get the group, and load it's entry list if not already loaded
	_curGroupPtr = &_groups[(id >> 8) & 0xff << 4];
	if (!_curGroupPtr->_loaded)
		_curGroupPtr->load();

	// Get the entry
	_curMemberPtr = &_curGroupPtr->_entries[id & 0xff];
	if (_curMemberPtr->_field1)
		initMem(_curMemberPtr->_field1);

	// Return the data for the entry if it's already been loaded
	if (_curMemberPtr->_data)
		return _curMemberPtr->_data;

	_xorMask = _curMemberPtr->_xorMask;
	_encrypt = (_curMemberPtr->_mode & 0x10) != 0;

	if (_curGroupPtr->_loaded) {

	}
	//TODO

	return NULL;
}

/*------------------------------------------------------------------------*/

BoltGroup::BoltGroup(Common::SeekableReadStream *f): _file(f) {
	byte buffer[BOLT_GROUP_SIZE];

	_groupPtr = NULL;

	_file->read(&buffer[0], BOLT_GROUP_SIZE);
	_loaded = false;
	_callInitGro = buffer[1] != 0;
	_count = buffer[3] ? buffer[3] : 256;	// TODO: Added this in. Check it's okay
	_fileOffset = READ_LE_UINT32(&buffer[8]);
}

void BoltGroup::load() {
	_file->seek(_fileOffset);

	// Read the entries
	for (int i = 0; i < _count; ++i)
		_entries.push_back(BoltEntry(_file));

	_loaded = true;
}

/*------------------------------------------------------------------------*/

BoltEntry::BoltEntry(Common::SeekableReadStream *f): _file(f) {
	_data = NULL;

	byte buffer[16];
	_file->read(&buffer[0], 16);
	_mode = buffer[0];
	_field1 = buffer[1];
	_field3 = buffer[3];
	_xorMask = buffer[4] & 0xff;	// TODO: Is this right??
	_size = READ_LE_UINT32(&buffer[4]);
	_fileOffset = READ_LE_UINT32(&buffer[8]); 
}

BoltEntry::~BoltEntry() {
	delete[] _data;
}

void BoltEntry::load() {
}

} // End of namespace Voyeur
