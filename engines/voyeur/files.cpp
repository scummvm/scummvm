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

bool FilesManager::openBOLTLib(const Common::String &filename, BoltFile *&boltFile) {
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
byte *BoltFile::_curMemInfoPtr = NULL;
int BoltFile::_fromGroupFlag = 0;

BoltFile::BoltFile() {
	if (!_curFd.open("buoy.blt"))
		error("Could not open buoy.blt");
	_curFilePosition = 0;

	// Read in the file header
	byte header[16];
	_curFd.read(&header[0], 16);

	if (strncmp((const char *)&header[0], "BOLT", 4) != 0)
		error("Tried to load non-bolt file");

	int totalGroups = header[11] ? header[11] : 0x100;
	for (int i = 0; i < totalGroups; ++i)
		_groups.push_back(BoltGroup(_curFd.readStream(_curFd.size())));
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

	if ((id >> 16) == 0) {
		if (!_curGroupPtr->_loaded) {
			_curGroupPtr->load();
		} else {
			id &= 0xff00;
			for (int idx = 0; idx < count; ++idx, ++id) {
				byte *member = getBoltMember(id);
				assert(member);
			}
		}
	}

	resolveAll();
	--_fromGroupFlag;
	return true;
}

/*------------------------------------------------------------------------*/

BoltGroup::BoltGroup(Common::SeekableReadStream *f): _file(f) {
	byte buffer[BOLT_GROUP_SIZE];

	_groupPtr = NULL;

	_file->read(&buffer[0], BOLT_GROUP_SIZE);
	_loaded = false;
	_callInitGro = buffer[1] != 0;
	_count = buffer[3];
	_fileOffset = READ_LE_UINT32(&buffer[8]);
}

void BoltGroup::load() {
	_file->seek(_fileOffset);

	byte header[4];
	_file->read(&header[0], 4);

	// Read the entries
	int count = _count ? _count : 256;
	for (int i = 0; i < count; ++i)
		_entries.push_back(BoltEntry(_file));
}

/*------------------------------------------------------------------------*/

BoltEntry::BoltEntry(Common::SeekableReadStream *f): _file(f) {
	
}

void BoltEntry::load() {

}

} // End of namespace Voyeur
