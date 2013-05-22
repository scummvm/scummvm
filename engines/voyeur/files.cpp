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
int BoltFile::_curFilePosition = 0;
int BoltFile::_bufferEnd = 0;
int BoltFile::_bufferBegin = 0;
int BoltFile::_bytesLeft = 0;
int BoltFile::_bufSize = 0;
byte *BoltFile::_bufStart = NULL;
byte *BoltFile::_bufPos = NULL;
byte BoltFile::_decompressBuf[DECOMPRESS_SIZE];
int BoltFile::_historyIndex;
byte BoltFile::_historyBuffer[0x200];
int BoltFile::_runLength;
int BoltFile::_decompState;
int BoltFile::_runType;
int BoltFile::_runValue;
int BoltFile::_runOffset;

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

	if (!_curGroupPtr->_loaded) {
		// Load the group index
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
	} else if (!_curGroupPtr->_processed) {
		_curGroupPtr->_processed = true;
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

	if (_curGroupPtr->_processed) {
		// TODO: Figure out weird access type. Uncompressed read perhaps?
		//int fileDiff = _curGroupPtr->_fileOffset - _curMemberPtr->_fileOffset;

	} else {
		_bufStart = _decompressBuf;
		_bufSize = DECOMPRESS_SIZE;

		if (_curMemberPtr->_fileOffset < _bufferBegin || _curMemberPtr->_fileOffset >= _bufferEnd) {
			_bytesLeft = 0;
			_bufPos = _bufStart;
			_bufferBegin = -1;
			_bufferEnd = _curMemberPtr->_fileOffset;
		} else {
			_bufPos = _curMemberPtr->_fileOffset + _bufferBegin + _bufStart;
			_bufSize = ((_bufPos - _bufStart) << 16) >> 16; // TODO: Validate this
			_bytesLeft = _bufSize;
		}
	}

	_decompState = 0;
	_historyIndex = 0;
	initType();

	return _curMemberPtr->_data;
}

void BoltFile::initType() {
	_curMemberPtr->_data = decompress(0, _curMemberPtr->_size, _curMemberPtr->_mode);
}

#define NEXT_BYTE if (--_bytesLeft <= 0) nextBlock()

byte *BoltFile::decompress(byte *buf, int size, int mode) {
	if (!buf)
		buf = new byte[size];
	byte *bufP = buf;

	if (mode & 8) {
		_decompState = 1;
		_runType = 0;
		_runLength = size;
	}

	while (size > 0) {
		if (!_decompState) {
			NEXT_BYTE;
			byte nextByte = *_bufPos++;

			switch (nextByte & 0xC0) {
			case 0:
				_runType = 0;
				_runLength = 30 - (nextByte & 0x1f) + 1;
				break;
			case 0x40:
				_runType = 1;
				_runLength = 35 - (nextByte & 0x1f);
				NEXT_BYTE;
				_runOffset = *_bufPos++ + ((nextByte & 0x20) << 3);
				break;
			case 0x80:
				_runType = 1;
				_runLength = (nextByte & 0x20) ? ((32 - (nextByte & 0x1f)) << 2) + 2 :
					(32 - (nextByte & 0x1f)) << 2;
				NEXT_BYTE;
				_runOffset = *_bufPos++ << 1;
				break;
			default:
				_runType = 2;

				if (nextByte & 0x20) {
					_runLength = 0;
				} else {
					NEXT_BYTE;
					_runLength = ((32 - (nextByte & 0x1f)) + (*_bufPos++ << 5)) << 2;
					NEXT_BYTE;
					_bufPos++;
					NEXT_BYTE;
					_runValue = *_bufPos++;
				}
				break;
			}

			_runOffset = _historyIndex - _runOffset;
		}

		int runOffset = _runOffset & 0x1ff;
		int len;
		if (_runLength <= size) {
			len = _runLength;
			_decompState = 0;
		} else {
			_decompState = 1;
			_runLength = len = size;
			if (_runType == 1)
				_runOffset += len;
		}

		// Reduce the remaining size
		size -= len;

		// Handle the run lengths
		switch (_runType) {
		case 0:
			while (len-- > 0) {
				NEXT_BYTE;
				_historyBuffer[_historyIndex] = *_bufPos++;
				_historyIndex = (_historyIndex + 1) & 0x1ff;
			}
			break;
		case 1:
			while (len-- > 0) {
				_historyBuffer[_historyIndex] = _historyBuffer[runOffset];
				*bufP++ = _historyBuffer[runOffset];
				_historyIndex = (_historyIndex + 1) & 0x1ff;
			}
			break;
		default:
			while (len-- > 0) {
				_historyBuffer[_historyIndex] = _runValue;
				*bufP++ = _runValue;
				_historyIndex = (_historyIndex + 1) & 0x1ff;
			}
			break;
		}
	}

	return buf;
}

void BoltFile::nextBlock() {
	if (_curFilePosition != _bufferEnd)
		_curFd.seek(_bufferEnd);

	_bufferBegin = _bufferEnd;
	int bytesRead = _curFd.read(_bufStart, _bufSize);

	_bufferEnd = _curFilePosition = _bufferBegin + bytesRead;
	_bytesLeft = bytesRead - 1;
	_bufPos = _bufStart;
}

/*------------------------------------------------------------------------*/

BoltGroup::BoltGroup(Common::SeekableReadStream *f): _file(f) {
	byte buffer[BOLT_GROUP_SIZE];

	_loaded = false;

	_file->read(&buffer[0], BOLT_GROUP_SIZE);
	_processed = buffer[0] != 0;
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
