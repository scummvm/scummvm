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

struct MacResMap {
	int16 resAttr;
	int16 typeOffset;
	int16 nameOffset;
	int16 numTypes;
};

struct MacResource {
	int16 id;
	int16 nameOffset;
	byte attr;
	int32 dataOffset;
	byte name[255];
	MacResource() : id(0), nameOffset(0), attr(0), dataOffset(0) {
		name[0] = 0;
	}
};

class MacResourceArray : public Common::Array<MacResource> {
};

struct MacResType {
	uint32 id;
	int16 items;
	int16 maxItemId;
	int16 offset;
	MacResourceArray resources;
	MacResType() : id(0), items(0), maxItemId(0), offset(0) {
	}
};

class MacResTypeArray : public Common::Array<MacResType> {
};

#define ID_MIDI     MKID_BE('Midi')

bool ResourceContext_RSC::loadMac() {
	int32 macDataSize, macDataSizePad;
	int32 macResSize, macResSizePad;
	int32 macResOffset;

	uint32 macMapLength;
	uint32 macDataLength;
	uint32 macMapOffset;
	uint32 macDataOffset;

	MacResMap macResMap;
	MacResTypeArray macResTypes;

	byte macNameLen;
	bool notSagaContext = false;

	if (_fileSize < RSC_MIN_FILESIZE + MAC_BINARY_HEADER_SIZE) {
		return false;
	}

	if (_file.readByte() != 0) {
		return false;
	}
	_file.readByte(); //MAX Name Len
	_file.seek(74);
	if (_file.readByte() != 0) {
		return false;
	}
	_file.seek(82);
	if (_file.readByte() != 0) {
		return false;
	}

	macDataSize = _file.readSint32BE();
	macResSize = _file.readSint32BE();
	macDataSizePad = (((macDataSize + 127) >> 7) << 7);
	macResSizePad = (((macResSize + 127) >> 7) << 7);

	macResOffset = MAC_BINARY_HEADER_SIZE + macDataSizePad;
	_file.seek(macResOffset);

	macDataOffset = _file.readUint32BE() + macResOffset;
	macMapOffset = _file.readUint32BE() + macResOffset;
	macDataLength = _file.readUint32BE();
	macMapLength = _file.readUint32BE();

	if (macDataOffset >= (uint)_fileSize || macMapOffset >= (uint)_fileSize ||
		macDataLength + macMapLength > (uint)_fileSize) {
			return false;
	}

	_file.seek(macMapOffset + 22);

	macResMap.resAttr = _file.readUint16BE();
	macResMap.typeOffset = _file.readUint16BE();
	macResMap.nameOffset = _file.readUint16BE();
	macResMap.numTypes = _file.readUint16BE();
	macResMap.numTypes++;

	_file.seek(macMapOffset + macResMap.typeOffset + 2);

	macResTypes.resize(macResMap.numTypes);

	for (MacResTypeArray::iterator k = macResTypes.begin(); k != macResTypes.end(); ++k) {
		k->id = _file.readUint32BE();
		k->items = _file.readUint16BE();
		k->offset = _file.readUint16BE();
		k->items++;
		k->resources.resize(k->items);
	}

	for (MacResTypeArray::iterator k = macResTypes.begin(); k != macResTypes.end(); ++k) {
		_file.seek(k->offset + macMapOffset + macResMap.typeOffset);

		for (MacResourceArray::iterator j = k->resources.begin(); j != k->resources.end(); ++j) {
			j->id = _file.readUint16BE();
			j->nameOffset = _file.readUint16BE();
			j->dataOffset = _file.readUint32BE();
			macResSize = _file.readUint32BE();

			j->attr = j->dataOffset >> 24;
			j->dataOffset &= 0xFFFFFF;
			if (j->id > k->maxItemId) {
				k->maxItemId = j->id;
			}
		}

		for (MacResourceArray::iterator j = k->resources.begin(); j != k->resources.end(); ++j) {
			if (j->nameOffset != -1) {
				_file.seek(j->nameOffset + macMapOffset + macResMap.nameOffset);
				macNameLen = _file.readByte();
				_file.read(j->name, macNameLen);
			}
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
