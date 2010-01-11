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

#include "common/scummsys.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/util.h"

#include "common/macresman.h"

namespace Common {

MacResManager::MacResManager(Common::String fileName) : _fileName(fileName), _resOffset(-1) {
	_resFile.open(_fileName);

	if (!_resFile.isOpen()) {
		error("Cannot open file %s", _fileName.c_str());
	}

	if (!init())
		error("Resource fork is missing in file '%s'", _fileName.c_str());
}

MacResManager::~MacResManager() {
	for (int i = 0; i < _resMap.numTypes; i++) {
		for (int j = 0; j < _resTypes[i].items; j++) {
			if (_resLists[i][j].nameOffset != -1) {
				delete _resLists[i][j].name;
			}
		}
		delete _resLists[i];
	}

	delete _resLists;
	delete _resTypes;

	_resFile.close();
}

#define MBI_INFOHDR 128
#define MBI_ZERO1 0
#define MBI_NAMELEN 1
#define MBI_ZERO2 74
#define MBI_ZERO3 82
#define MBI_DFLEN 83
#define MBI_RFLEN 87
#define MAXNAMELEN 63

bool MacResManager::init() {
	byte infoHeader[MBI_INFOHDR];
	int32 data_size, rsrc_size;
	int32 data_size_pad, rsrc_size_pad;
	int filelen;

	filelen = _resFile.size();
	_resFile.read(infoHeader, MBI_INFOHDR);

	// Maybe we have MacBinary?
	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out fork lengths
		data_size = READ_BE_UINT32(infoHeader + MBI_DFLEN);
		rsrc_size = READ_BE_UINT32(infoHeader + MBI_RFLEN);

		data_size_pad = (((data_size + 127) >> 7) << 7);
		rsrc_size_pad = (((rsrc_size + 127) >> 7) << 7);

		// Length check
		int sumlen =  MBI_INFOHDR + data_size_pad + rsrc_size_pad;

		if (sumlen == filelen)
			_resOffset = MBI_INFOHDR + data_size_pad;
	}

	if (_resOffset == -1) // MacBinary check is failed
		_resOffset = 0; // Maybe we have dumped fork?

	_resFile.seek(_resOffset);

	_dataOffset = _resFile.readUint32BE() + _resOffset;
	_mapOffset = _resFile.readUint32BE() + _resOffset;
	_dataLength = _resFile.readUint32BE();
	_mapLength = _resFile.readUint32BE();

	// do sanity check
	if (_dataOffset >= filelen || _mapOffset >= filelen ||
		_dataLength + _mapLength  > filelen) {
		_resOffset = -1;
		return false;
	}

	debug(7, "got header: data %d [%d] map %d [%d]",
		_dataOffset, _dataLength, _mapOffset, _mapLength);

	readMap();

	return true;
}

MacResIDArray MacResManager::getResIDArray(const char *typeID) {
	int typeNum = -1;
	MacResIDArray res;

	for (int i = 0; i < _resMap.numTypes; i++)
		if (strcmp(_resTypes[i].id, typeID) == 0) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return res;
	
	res.resize(_resTypes[typeNum].items);

	for (int i = 0; i < _resTypes[typeNum].items; i++)
		res[i] = _resLists[typeNum][i].id;

	return res;
}

char *MacResManager::getResName(const char *typeID, int16 resID) {
	int i;
	int typeNum = -1;

	for (i = 0; i < _resMap.numTypes; i++)
		if (strcmp(_resTypes[i].id, typeID) == 0) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return NULL;

	for (i = 0; i < _resTypes[typeNum].items; i++)
		if (_resLists[typeNum][i].id == resID)
			return _resLists[typeNum][i].name;

	return NULL;
}

byte *MacResManager::getResource(const char *typeID, int16 resID, int *size) {
	int i;
	int typeNum = -1;
	int resNum = -1;
	byte *buf;
	int len;

	for (i = 0; i < _resMap.numTypes; i++)
		if (strcmp(_resTypes[i].id, typeID) == 0) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return NULL;

	for (i = 0; i < _resTypes[typeNum].items; i++)
		if (_resLists[typeNum][i].id == resID) {
			resNum = i;
			break;
		}

	if (resNum == -1)
		return NULL;

	_resFile.seek(_dataOffset + _resLists[typeNum][resNum].dataOffset);

	len = _resFile.readUint32BE();
	buf = (byte *)malloc(len);

	_resFile.read(buf, len);

	*size = len;

	return buf;
}

void MacResManager::readMap() {
	int	i, j, len;

	_resFile.seek(_mapOffset + 22);

	_resMap.resAttr = _resFile.readUint16BE();
	_resMap.typeOffset = _resFile.readUint16BE();
	_resMap.nameOffset = _resFile.readUint16BE();
	_resMap.numTypes = _resFile.readUint16BE();
	_resMap.numTypes++;

	_resFile.seek(_mapOffset + _resMap.typeOffset + 2);
	_resTypes = new ResType[_resMap.numTypes];

	for (i = 0; i < _resMap.numTypes; i++) {
		_resFile.read(_resTypes[i].id, 4);
		_resTypes[i].id[4] = 0;
		_resTypes[i].items = _resFile.readUint16BE();
		_resTypes[i].offset = _resFile.readUint16BE();
		_resTypes[i].items++;

		debug(8, "resType: <%s> items: %d offset: %d (0x%x)", _resTypes[i].id, _resTypes[i].items,  _resTypes[i].offset, _resTypes[i].offset);
	}

	_resLists = new ResPtr[_resMap.numTypes];

	for (i = 0; i < _resMap.numTypes; i++) {
		_resLists[i] = new Resource[_resTypes[i].items];
		_resFile.seek(_resTypes[i].offset + _mapOffset + _resMap.typeOffset);

		for (j = 0; j < _resTypes[i].items; j++) {
			ResPtr resPtr = _resLists[i] + j;

			resPtr->id = _resFile.readUint16BE();
			resPtr->nameOffset = _resFile.readUint16BE();
			resPtr->dataOffset = _resFile.readUint32BE();
			_resFile.readUint32BE();
			resPtr->name = 0;

			resPtr->attr = resPtr->dataOffset >> 24;
			resPtr->dataOffset &= 0xFFFFFF;
		}

		for (j = 0; j < _resTypes[i].items; j++) {
			if (_resLists[i][j].nameOffset != -1) {
				_resFile.seek(_resLists[i][j].nameOffset + _mapOffset + _resMap.nameOffset);

				len = _resFile.readByte();
				_resLists[i][j].name = new char[len + 1];
				_resLists[i][j].name[len] = 0;
				_resFile.read(_resLists[i][j].name, len);
			}
		}
	}
}

void MacResManager::convertCursor(byte *data, int datasize, byte **cursor, int *w, int *h,
					 int *hotspot_x, int *hotspot_y, int *keycolor, bool colored, byte **palette, int *palSize) {
	Common::MemoryReadStream dis(data, datasize);
	int i, b;
	byte imageByte;
	byte *iconData;
	int numBytes;
	int pixelsPerByte, bpp;
	int ctSize;
	byte bitmask;
	int iconRowBytes, iconBounds[4];
	int ignored;
	int iconDataSize;

	dis.readUint16BE(); // type
	dis.readUint32BE(); // offset to pixel map
	dis.readUint32BE(); // offset to pixel data
	dis.readUint32BE(); // expanded cursor data
	dis.readUint16BE(); // expanded data depth
	dis.readUint32BE(); // reserved

	// Grab B/W icon data
	*cursor = (byte *)malloc(16 * 16);
	for (i = 0; i < 32; i++) {
		imageByte = dis.readByte();
		for (b = 0; b < 8; b++)
			cursor[0][i*8+b] = (byte)((imageByte & (0x80 >> b)) > 0? 0x0F: 0x00);
	}

	// Apply mask data
	for (i = 0; i < 32; i++) {
		imageByte = dis.readByte();
		for (b = 0; b < 8; b++)
			if ((imageByte & (0x80 >> b)) == 0)
				cursor[0][i*8+b] = 0xff;
	}

	*hotspot_y = dis.readUint16BE();
	*hotspot_x = dis.readUint16BE();
	*w = *h = 16;

	// Use b/w cursor on backends which don't support cursor palettes
	if (!colored)
		return;

	dis.readUint32BE(); // reserved
	dis.readUint32BE(); // cursorID

	// Color version of cursor
	dis.readUint32BE(); // baseAddr

	// Keep only lowbyte for now
	dis.readByte();
	iconRowBytes = dis.readByte();

	if (!iconRowBytes)
		return;

	iconBounds[0] = dis.readUint16BE();
	iconBounds[1] = dis.readUint16BE();
	iconBounds[2] = dis.readUint16BE();
	iconBounds[3] = dis.readUint16BE();

	dis.readUint16BE(); // pmVersion
	dis.readUint16BE(); // packType
	dis.readUint32BE(); // packSize

	dis.readUint32BE(); // hRes
	dis.readUint32BE(); // vRes

	dis.readUint16BE(); // pixelType
	dis.readUint16BE(); // pixelSize
	dis.readUint16BE(); // cmpCount
	dis.readUint16BE(); // cmpSize

	dis.readUint32BE(); // planeByte
	dis.readUint32BE(); // pmTable
	dis.readUint32BE(); // reserved

	// Pixel data for cursor
	iconDataSize =  iconRowBytes * (iconBounds[3] - iconBounds[1]);
	iconData = (byte *)malloc(iconDataSize);
	dis.read(iconData, iconDataSize);

	// Color table
	dis.readUint32BE(); // ctSeed
	dis.readUint16BE(); // ctFlag
	ctSize = dis.readUint16BE() + 1;

	*palette = (byte *)malloc(ctSize * 4);

	// Read just high byte of 16-bit color
	for (int c = 0; c < ctSize; c++) {
		// We just use indices 0..ctSize, so ignore color ID
		dis.readUint16BE(); // colorID[c]

		palette[0][c * 4 + 0] = dis.readByte();
		ignored = dis.readByte();

		palette[0][c * 4 + 1] = dis.readByte();
		ignored = dis.readByte();

		palette[0][c * 4 + 2] = dis.readByte();
		ignored = dis.readByte();

		palette[0][c * 4 + 3] = 0;
	}

	*palSize = ctSize;

	numBytes = (iconBounds[2] - iconBounds[0]) * (iconBounds[3] - iconBounds[1]);

	pixelsPerByte = (iconBounds[2] - iconBounds[0]) / iconRowBytes;
	bpp           = 8 / pixelsPerByte;

	// build a mask to make sure the pixels are properly shifted out
	bitmask = 0;
	for (int m = 0; m < bpp; m++) {
		bitmask <<= 1;
		bitmask  |= 1;
	}

	// Extract pixels from bytes
	for (int j = 0; j < iconDataSize; j++)
		for (b = 0; b < pixelsPerByte; b++) {
			int idx = j * pixelsPerByte + (pixelsPerByte - 1 - b);

			if (cursor[0][idx] != 0xff) // if mask is not there
				cursor[0][idx] = (byte)((iconData[j] >> (b * bpp)) & bitmask);
		}

	free(iconData);

	assert(datasize - dis.pos() == 0);
}

} // End of namespace Common
