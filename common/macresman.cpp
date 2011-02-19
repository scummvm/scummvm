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
#include "common/util.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/macresman.h"
#include "common/md5.h"
#include "common/substream.h"
#include "common/memstream.h"

#ifdef MACOSX
#include "common/config-manager.h"
#include "backends/fs/stdiostream.h"
#endif

namespace Common {

#define MBI_INFOHDR 128
#define MBI_ZERO1 0
#define MBI_NAMELEN 1
#define MBI_ZERO2 74
#define MBI_ZERO3 82
#define MBI_DFLEN 83
#define MBI_RFLEN 87
#define MAXNAMELEN 63

MacResManager::MacResManager() {
	memset(this, 0, sizeof(MacResManager));
	close();
}

MacResManager::~MacResManager() {
	close();
}

void MacResManager::close() {
	_resForkOffset = -1;
	_mode = kResForkNone;

	for (int i = 0; i < _resMap.numTypes; i++) {
		for (int j = 0; j < _resTypes[i].items; j++)
			if (_resLists[i][j].nameOffset != -1)
				delete[] _resLists[i][j].name;

		delete[] _resLists[i];
	}

	delete[] _resLists; _resLists = 0;
	delete[] _resTypes; _resTypes = 0;
	delete _stream; _stream = 0;
}

bool MacResManager::hasDataFork() const {
	return !_baseFileName.empty();
}

bool MacResManager::hasResFork() const {
	return !_baseFileName.empty() && _mode != kResForkNone;
}

uint32 MacResManager::getResForkDataSize() const {
	if (!hasResFork())
		return 0;

	_stream->seek(_resForkOffset + 4);
	return _stream->readUint32BE();
}

String MacResManager::computeResForkMD5AsString(uint32 length) const {
	if (!hasResFork())
		return String();

	_stream->seek(_resForkOffset);
	uint32 dataOffset = _stream->readUint32BE() + _resForkOffset;
	/* uint32 mapOffset = */ _stream->readUint32BE();
	uint32 dataLength = _stream->readUint32BE();


	SeekableSubReadStream resForkStream(_stream, dataOffset, dataOffset + dataLength);
	return computeStreamMD5AsString(resForkStream, MIN<uint32>(length, _resForkSize));
}

bool MacResManager::open(String filename) {
	close();

#ifdef MACOSX
	// Check the actual fork on a Mac computer
	String fullPath = ConfMan.get("path") + "/" + filename + "/..namedfork/rsrc";
	SeekableReadStream *macResForkRawStream = StdioStream::makeFromPath(fullPath, false);

	if (macResForkRawStream && loadFromRawFork(*macResForkRawStream)) {
		_baseFileName = filename;
		return true;
	}

	delete macResForkRawStream;
#endif

	File *file = new File();

	// First, let's try to see if the Mac converted name exists
	if (file->open("._" + filename) && loadFromAppleDouble(*file)) {
		_baseFileName = filename;
		return true;
	}
	file->close();

	// Check .bin too
	if (file->open(filename + ".bin") && loadFromMacBinary(*file)) {
		_baseFileName = filename;
		return true;
	}
	file->close();

	// Maybe we have a dumped fork?
	if (file->open(filename + ".rsrc") && loadFromRawFork(*file)) {
		_baseFileName = filename;
		return true;
	}
	file->close();

	// Fine, what about just the data fork?
	if (file->open(filename)) {
		_baseFileName = filename;

		if (isMacBinary(*file)) {
			file->seek(0, SEEK_SET);
			if (loadFromMacBinary(*file))
				return true;
		}

		file->seek(0, SEEK_SET);
		_stream = file;
		return true;
	}

	delete file;

	// The file doesn't exist
	return false;
}

bool MacResManager::open(FSNode path, String filename) {
	close();

#ifdef MACOSX
	// Check the actual fork on a Mac computer
	String fullPath = path.getPath() + "/" + filename + "/..namedfork/rsrc";
	SeekableReadStream *macResForkRawStream = StdioStream::makeFromPath(fullPath, false);

	if (macResForkRawStream && loadFromRawFork(*macResForkRawStream)) {
		_baseFileName = filename;
		return true;
	}

	delete macResForkRawStream;
#endif

	// First, let's try to see if the Mac converted name exists
	FSNode fsNode = path.getChild("._" + filename);
	if (fsNode.exists() && !fsNode.isDirectory()) {
		SeekableReadStream *stream = fsNode.createReadStream();
		if (loadFromAppleDouble(*stream)) {
			_baseFileName = filename;
			return true;
		}
		delete stream;
	}

	// Check .bin too
	fsNode = path.getChild(filename + ".bin");
	if (fsNode.exists() && !fsNode.isDirectory()) {
		SeekableReadStream *stream = fsNode.createReadStream();
		if (loadFromMacBinary(*stream)) {
			_baseFileName = filename;
			return true;
		}
		delete stream;
	}

	// Maybe we have a dumped fork?
	fsNode = path.getChild(filename + ".rsrc");
	if (fsNode.exists() && !fsNode.isDirectory()) {
		SeekableReadStream *stream = fsNode.createReadStream();
		if (loadFromRawFork(*stream)) {
			_baseFileName = filename;
			return true;
		}
		delete stream;
	}

	// Fine, what about just the data fork?
	fsNode = path.getChild(filename);
	if (fsNode.exists() && !fsNode.isDirectory()) {
		SeekableReadStream *stream = fsNode.createReadStream();
		_baseFileName = filename;

		if (isMacBinary(*stream)) {
			stream->seek(0, SEEK_SET);
			if (loadFromMacBinary(*stream))
				return true;
		}

		stream->seek(0, SEEK_SET);
		_stream = stream;
		return true;
	}

	// The file doesn't exist
	return false;
}

bool MacResManager::loadFromAppleDouble(SeekableReadStream &stream) {
	if (stream.readUint32BE() != 0x00051607) // tag
		return false;

	stream.skip(20); // version + home file system

	uint16 entryCount = stream.readUint16BE();

	for (uint16 i = 0; i < entryCount; i++) {
		uint32 id = stream.readUint32BE();
		uint32 offset = stream.readUint32BE();
		uint32 length = stream.readUint32BE(); // length

		if (id == 2) {
			// Found the resource fork!
			_resForkOffset = offset;
			_mode = kResForkAppleDouble;
			_resForkSize = length;
			return load(stream);
		}
	}

	return false;
}

bool MacResManager::isMacBinary(SeekableReadStream &stream) {
	byte infoHeader[MBI_INFOHDR];
	int resForkOffset = -1;

	stream.read(infoHeader, MBI_INFOHDR);

	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out fork lengths
		uint32 dataSize = READ_BE_UINT32(infoHeader + MBI_DFLEN);
		uint32 rsrcSize = READ_BE_UINT32(infoHeader + MBI_RFLEN);

		uint32 dataSizePad = (((dataSize + 127) >> 7) << 7);
		uint32 rsrcSizePad = (((rsrcSize + 127) >> 7) << 7);

		// Length check
		if (MBI_INFOHDR + dataSizePad + rsrcSizePad == (uint32)stream.size()) {
			resForkOffset = MBI_INFOHDR + dataSizePad;
		}
	}

	if (resForkOffset < 0)
		return false;

	return true;
}

bool MacResManager::loadFromMacBinary(SeekableReadStream &stream) {
	byte infoHeader[MBI_INFOHDR];
	stream.read(infoHeader, MBI_INFOHDR);

	// Maybe we have MacBinary?
	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out fork lengths
		uint32 dataSize = READ_BE_UINT32(infoHeader + MBI_DFLEN);
		uint32 rsrcSize = READ_BE_UINT32(infoHeader + MBI_RFLEN);

		uint32 dataSizePad = (((dataSize + 127) >> 7) << 7);
		uint32 rsrcSizePad = (((rsrcSize + 127) >> 7) << 7);

		// Length check
		if (MBI_INFOHDR + dataSizePad + rsrcSizePad == (uint32)stream.size()) {
			_resForkOffset = MBI_INFOHDR + dataSizePad;
			_resForkSize = rsrcSize;
		}
	}

	if (_resForkOffset < 0)
		return false;

	_mode = kResForkMacBinary;
	return load(stream);
}

bool MacResManager::loadFromRawFork(SeekableReadStream &stream) {
	_mode = kResForkRaw;
	_resForkOffset = 0;
	_resForkSize = stream.size();
	return load(stream);
}

bool MacResManager::load(SeekableReadStream &stream) {
	if (_mode == kResForkNone)
		return false;

	stream.seek(_resForkOffset);

	_dataOffset = stream.readUint32BE() + _resForkOffset;
	_mapOffset = stream.readUint32BE() + _resForkOffset;
	_dataLength = stream.readUint32BE();
	_mapLength = stream.readUint32BE();

	// do sanity check
	if (_dataOffset >= (uint32)stream.size() || _mapOffset >= (uint32)stream.size() ||
		_dataLength + _mapLength  > (uint32)stream.size()) {
		_resForkOffset = -1;
		_mode = kResForkNone;
		return false;
	}

	debug(7, "got header: data %d [%d] map %d [%d]",
		_dataOffset, _dataLength, _mapOffset, _mapLength);

	_stream = &stream;

	readMap();
	return true;
}

SeekableReadStream *MacResManager::getDataFork() {
	if (!_stream)
		return NULL;

	if (_mode == kResForkMacBinary) {
		_stream->seek(MBI_DFLEN);
		uint32 dataSize = _stream->readUint32BE();
		return new SeekableSubReadStream(_stream, MBI_INFOHDR, MBI_INFOHDR + dataSize);
	}

	File *file = new File();
	if (file->open(_baseFileName))
		return file;
	delete file;

	return NULL;
}

MacResIDArray MacResManager::getResIDArray(uint32 typeID) {
	int typeNum = -1;
	MacResIDArray res;

	for (int i = 0; i < _resMap.numTypes; i++)
		if (_resTypes[i].id == typeID) {
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

MacResTagArray MacResManager::getResTagArray() {
	MacResTagArray tagArray;

	if (!hasResFork())
		return tagArray;

	tagArray.resize(_resMap.numTypes);

	for (uint32 i = 0; i < _resMap.numTypes; i++)
		tagArray[i] = _resTypes[i].id;

	return tagArray;
}

String MacResManager::getResName(uint32 typeID, uint16 resID) const {
	int typeNum = -1;

	for (int i = 0; i < _resMap.numTypes; i++)
		if (_resTypes[i].id == typeID) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return "";

	for (int i = 0; i < _resTypes[typeNum].items; i++)
		if (_resLists[typeNum][i].id == resID)
			return _resLists[typeNum][i].name;

	return "";
}

SeekableReadStream *MacResManager::getResource(uint32 typeID, uint16 resID) {
	int typeNum = -1;
	int resNum = -1;

	for (int i = 0; i < _resMap.numTypes; i++)
		if (_resTypes[i].id == typeID) {
			typeNum = i;
			break;
		}

	if (typeNum == -1)
		return NULL;

	for (int i = 0; i < _resTypes[typeNum].items; i++)
		if (_resLists[typeNum][i].id == resID) {
			resNum = i;
			break;
		}

	if (resNum == -1)
		return NULL;

	_stream->seek(_dataOffset + _resLists[typeNum][resNum].dataOffset);
	uint32 len = _stream->readUint32BE();

	// Ignore resources with 0 length
	if (!len)
		return 0;

	return _stream->readStream(len);
}

SeekableReadStream *MacResManager::getResource(const String &filename) {
	for (uint32 i = 0; i < _resMap.numTypes; i++) {
		for (uint32 j = 0; j < _resTypes[i].items; j++) {
			if (_resLists[i][j].nameOffset != -1 && filename.equalsIgnoreCase(_resLists[i][j].name)) {
				_stream->seek(_dataOffset + _resLists[i][j].dataOffset);
				uint32 len = _stream->readUint32BE();

				// Ignore resources with 0 length
				if (!len)
					return 0;

				return _stream->readStream(len);
			}
		}
	}

	return 0;
}

SeekableReadStream *MacResManager::getResource(uint32 typeID, const String &filename) {
	for (uint32 i = 0; i < _resMap.numTypes; i++) {
		if (_resTypes[i].id != typeID)
			continue;

		for (uint32 j = 0; j < _resTypes[i].items; j++) {
			if (_resLists[i][j].nameOffset != -1 && filename.equalsIgnoreCase(_resLists[i][j].name)) {
				_stream->seek(_dataOffset + _resLists[i][j].dataOffset);
				uint32 len = _stream->readUint32BE();

				// Ignore resources with 0 length
				if (!len)
					return 0;

				return _stream->readStream(len);
			}
		}
	}

	return 0;
}

void MacResManager::readMap() {
	_stream->seek(_mapOffset + 22);

	_resMap.resAttr = _stream->readUint16BE();
	_resMap.typeOffset = _stream->readUint16BE();
	_resMap.nameOffset = _stream->readUint16BE();
	_resMap.numTypes = _stream->readUint16BE();
	_resMap.numTypes++;

	_stream->seek(_mapOffset + _resMap.typeOffset + 2);
	_resTypes = new ResType[_resMap.numTypes];

	for (int i = 0; i < _resMap.numTypes; i++) {
		_resTypes[i].id = _stream->readUint32BE();
		_resTypes[i].items = _stream->readUint16BE();
		_resTypes[i].offset = _stream->readUint16BE();
		_resTypes[i].items++;

		debug(8, "resType: <%s> items: %d offset: %d (0x%x)", tag2str(_resTypes[i].id), _resTypes[i].items,  _resTypes[i].offset, _resTypes[i].offset);
	}

	_resLists = new ResPtr[_resMap.numTypes];

	for (int i = 0; i < _resMap.numTypes; i++) {
		_resLists[i] = new Resource[_resTypes[i].items];
		_stream->seek(_resTypes[i].offset + _mapOffset + _resMap.typeOffset);

		for (int j = 0; j < _resTypes[i].items; j++) {
			ResPtr resPtr = _resLists[i] + j;

			resPtr->id = _stream->readUint16BE();
			resPtr->nameOffset = _stream->readUint16BE();
			resPtr->dataOffset = _stream->readUint32BE();
			_stream->readUint32BE();
			resPtr->name = 0;

			resPtr->attr = resPtr->dataOffset >> 24;
			resPtr->dataOffset &= 0xFFFFFF;
		}

		for (int j = 0; j < _resTypes[i].items; j++) {
			if (_resLists[i][j].nameOffset != -1) {
				_stream->seek(_resLists[i][j].nameOffset + _mapOffset + _resMap.nameOffset);

				byte len = _stream->readByte();
				_resLists[i][j].name = new char[len + 1];
				_resLists[i][j].name[len] = 0;
				_stream->read(_resLists[i][j].name, len);
			}
		}
	}
}

void MacResManager::convertCrsrCursor(byte *data, int datasize, byte **cursor, int *w, int *h,
				int *hotspot_x, int *hotspot_y, int *keycolor, bool colored, byte **palette, int *palSize) {
	MemoryReadStream dis(data, datasize);
	int i, b;
	byte imageByte;
	byte *iconData;
	int pixelsPerByte, bpp;
	int ctSize;
	byte bitmask;
	int iconRowBytes, iconBounds[4];
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
	*keycolor = 0xff;

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

	*palette = (byte *)malloc(ctSize * 3);

	// Read just high byte of 16-bit color
	for (int c = 0; c < ctSize; c++) {
		// We just use indices 0..ctSize, so ignore color ID
		dis.readUint16BE(); // colorID[c]

		palette[0][c * 3 + 0] = dis.readByte();
		dis.readByte();

		palette[0][c * 3 + 1] = dis.readByte();
		dis.readByte();

		palette[0][c * 3 + 2] = dis.readByte();
		dis.readByte();
	}

	*palSize = ctSize;

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
