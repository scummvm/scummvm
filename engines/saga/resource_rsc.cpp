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
 * $URL$
 * $Id$
 *
 */

// RSC Resource file management module (SAGA 1, used in ITE)

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/sndres.h"

#include "engines/advancedDetector.h"

namespace Saga {

struct MacResource {
	int16 id;
	int32 dataOffset;
	MacResource() : id(0), dataOffset(0) {}
};

class MacResourceArray : public Common::Array<MacResource> {
};

struct MacResType {
	uint32 id;
	int16 maxItemId;
	int16 offset;
	MacResourceArray resources;
	MacResType() : id(0), maxItemId(0), offset(0) {
	}
};

class MacResTypeArray : public Common::Array<MacResType> {
};

#define ID_MIDI     MKID_BE('Midi')

bool ResourceContext_RSC::loadMac() {
	int32 macDataSize, macResSizePad, macResOffset;
	uint32 macMapOffset, macDataOffset;

	MacResTypeArray macResTypes;

	bool notSagaContext = false;

	// Sanity check
	if (_fileSize < RSC_MIN_FILESIZE + MAC_BINARY_HEADER_SIZE)
		return false;

	_file.seek(82);
	if (_file.readByte() != 0)
		return false;

	macDataSize = _file.readSint32BE();
	macResOffset = MAC_BINARY_HEADER_SIZE + (((macDataSize + 127) >> 7) << 7);

	macResSizePad = (((_file.readSint32BE() + 127) >> 7) << 7);
	_file.seek(macResOffset);

	macDataOffset = _file.readUint32BE() + macResOffset;
	macMapOffset = _file.readUint32BE() + macResOffset;
	// Used for sanity checks
	uint32 macDataLength = _file.readUint32BE();
	uint32 macMapLength = _file.readUint32BE();

	if (macDataOffset >= (uint)_fileSize || macMapOffset >= (uint)_fileSize ||
		macDataLength + macMapLength > (uint)_fileSize)
			return false;

	_file.seek(macMapOffset + 22);

	_file.readUint16BE();	// resAttr
	int16 typeOffset = _file.readUint16BE();
	_file.readUint16BE();	// nameOffset
	int16 numTypes = _file.readUint16BE() + 1;
	macResTypes.resize(numTypes);

	_file.seek(macMapOffset + typeOffset + 2);

	for (MacResTypeArray::iterator k = macResTypes.begin(); k != macResTypes.end(); ++k) {
		k->id = _file.readUint32BE();
		int16 items = _file.readUint16BE() + 1;
		k->resources.resize(items);
		k->offset = _file.readUint16BE();
	}

	for (MacResTypeArray::iterator k = macResTypes.begin(); k != macResTypes.end(); ++k) {
		_file.seek(k->offset + macMapOffset + typeOffset);

		for (MacResourceArray::iterator j = k->resources.begin(); j != k->resources.end(); ++j) {
			j->id = _file.readUint16BE();
			_file.readUint16BE();	// nameOffset
			j->dataOffset = _file.readUint32BE();
			_file.readUint32BE();	// macResSize

			j->dataOffset &= 0xFFFFFF;
			if (j->id > k->maxItemId)
				k->maxItemId = j->id;
		}
	}

	//
	for (MacResTypeArray::iterator k = macResTypes.begin(); k != macResTypes.end(); ++k) {
		//getting offsets & sizes of midi
		if (((_fileType & GAME_MUSICFILE_GM) > 0) && (k->id == ID_MIDI)) {

			_table.resize(k->maxItemId + 1);
			for (MacResourceArray::iterator j = k->resources.begin(); j != k->resources.end(); ++j) {
				_file.seek(macDataOffset + j->dataOffset);
				_table[j->id].size = _file.readUint32BE();
				_table[j->id].offset = _file.pos();
			}
			notSagaContext = true;
			break;
		}
	}

	if ((!notSagaContext) && (!loadRes(MAC_BINARY_HEADER_SIZE, macDataSize))) {
		return false;
	}

	return true;
}

uint32 Resource_RSC::convertResourceId(uint32 resourceId) {

	if (_vm->isMacResources()) {
		if (resourceId > 1537) {
			return resourceId - 2;
		} else {
			if (resourceId == 1535 || resourceId == 1536) {
				error("Wrong resource number %d for Mac ITE", resourceId);
			}
		}
	}

	return resourceId;
}

} // End of namespace Saga
