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

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/ne_exe.h"
#include "common/str.h"
#include "common/stream.h"

namespace Common {

NECursor::NECursor() {
	_width    = 0;
	_height   = 0;
	_hotspotX = 0;
	_hotspotY = 0;
	_surface  = 0;
	memset(_palette, 0, 256 * 4);
}

NECursor::~NECursor() {
	clear();
}

uint16 NECursor::getWidth() const {
	return _width;
}

uint16 NECursor::getHeight() const {
	return _height;
}

uint16 NECursor::getHotspotX() const {
	return _hotspotX;
}

uint16 NECursor::getHotspotY() const {
	return _hotspotY;
}

void NECursor::setDimensions(uint16 width, uint16 height) {
	_width  = width;
	_height = height;
}

void NECursor::setHotspot(uint16 x, uint16 y) {
	_hotspotX = x;
	_hotspotY = y;
}

bool NECursor::readCursor(SeekableReadStream &stream, uint32 count) {
	clear();

	SeekableReadStream *bitmap = stream.readStream(count);
	_surface = new byte[_width * _height];

	uint32 width  = _width;
	uint32 height = _height * 2;

	// Sanity checks
	assert((width > 0) && (height > 0));

	// Check header size
	if (bitmap->readUint32LE() != 40)
		return false;

	// Check dimensions
	if (bitmap->readUint32LE() != width)
		return false;
	if (bitmap->readUint32LE() != height)
		return false;

	// Color planes
	if (bitmap->readUint16LE() != 1)
		return false;
	// Bits per pixel
	if (bitmap->readUint16LE() != 1)
		return false;
	// Compression
	if (bitmap->readUint32LE() != 0)
		return false;

	// Image size + X resolution + Y resolution
	bitmap->skip(12);

	uint32 numColors = bitmap->readUint32LE();

	if (numColors == 0)
		numColors = 2;
	else if (numColors > 2)
		return false;

	// Assert that enough data is there for the whole cursor
	if ((uint32)bitmap->size() < 40 + numColors * 4 + width * height / 8)
		return false;

	// Height includes AND-mask and XOR-mask
	height /= 2;

	// Standard palette: transparent, black, white
	_palette[8] = 0xff;
	_palette[9] = 0xff;
	_palette[10] = 0xff;

	// Reading the palette
	bitmap->seek(40);
	for (uint32 i = 0 ; i < numColors; i++) {
		_palette[(i + 1) * 4 + 2] = bitmap->readByte();
		_palette[(i + 1) * 4 + 1] = bitmap->readByte();
		_palette[(i + 1) * 4 + 0] = bitmap->readByte();
		bitmap->readByte();
	}

	// Reading the bitmap data
	uint32 dataSize = bitmap->size() - 40 - numColors * 4;
	byte *initialSource = new byte[dataSize];
	bitmap->read(initialSource, dataSize);
	const byte *srcP = initialSource;
	const byte *srcM = srcP + ((width * height) / 8);
	byte *dest = _surface + width * (height - 1);

	for (uint32 i = 0; i < height; i++) {
		byte *rowDest = dest;

		for (uint32 j = 0; j < (width / 8); j++) {
			byte p = srcP[j];
			byte m = srcM[j];

			for (int k = 0; k < 8; k++, rowDest++, p <<= 1, m <<= 1) {
				if ((m & 0x80) != 0x80) {
					if ((p & 0x80) == 0x80)
						*rowDest = 2;
					else
						*rowDest = 1;
				} else
					*rowDest = 0;
			}
		}

		dest -= width;
		srcP += width / 8;
		srcM += width / 8;
	}

	delete bitmap;
	delete[] initialSource;
	return true;
}

void NECursor::clear() {
	delete[] _surface; _surface = 0;
}

NEResourceID &NEResourceID::operator=(String string) {
	_name = string;
	_idType = kIDTypeString;
	return *this;
}

NEResourceID &NEResourceID::operator=(uint16 x) {
	_id = x;
	_idType = kIDTypeNumerical;
	return *this;
}

bool NEResourceID::operator==(const String &x) const {
	return _idType == kIDTypeString && _name.equalsIgnoreCase(x);
}

bool NEResourceID::operator==(const uint16 &x) const {
	return _idType == kIDTypeNumerical && _id == x;
}

bool NEResourceID::operator==(const NEResourceID &x) const {
	if (_idType != x._idType)
		return false;
	if (_idType == kIDTypeString)
		return _name.equalsIgnoreCase(x._name);
	if (_idType == kIDTypeNumerical)
		return _id == x._id;
	return true;
}

String NEResourceID::getString() const {
	if (_idType != kIDTypeString)
		return "";

	return _name;
}

uint16 NEResourceID::getID() const {
	if (_idType != kIDTypeNumerical)
		return 0xffff;

	return _idType;
}

String NEResourceID::toString() const {
	if (_idType == kIDTypeString)
		return _name;
	else if (_idType == kIDTypeNumerical)
		return String::format("%04x", _id);

	return "";
}

NEResources::NEResources() {
	_exe = 0;
}

NEResources::~NEResources() {
	clear();
}

void NEResources::clear() {
	if (_exe) {
		delete _exe;
		_exe = 0;
	}

	_resources.clear();

	for (uint32 i = 0; i < _cursors.size(); i++)
		for (uint32 j = 0; j < _cursors[i].cursors.size(); j++)
			delete _cursors[i].cursors[j];

	_cursors.clear();
}

const Array<NECursorGroup> &NEResources::getCursors() const {
	return _cursors;
}

bool NEResources::loadFromEXE(const String &fileName) {
	if (fileName.empty())
		return false;

	File *file = new File();

	if (!file->open(fileName)) {
		delete file;
		return false;
	}

	return loadFromEXE(file);
}

bool NEResources::loadFromEXE(SeekableReadStream *stream) {
	clear();

	if (!stream)
		return false;

	_exe = stream;

	uint32 offsetResourceTable = getResourceTableOffset();
	if (offsetResourceTable == 0xFFFFFFFF)
		return false;
	if (offsetResourceTable == 0)
		return true;

	if (!readResourceTable(offsetResourceTable))
		return false;

	if (!readCursors())
		return false;

	return true;
}

bool NEResources::loadFromCompressedEXE(const String &fileName) {
	// Based on http://www.cabextract.org.uk/libmspack/doc/szdd_kwaj_format.html

	File file;

	if (!file.open(fileName))
		return false;

	// First part of the signature
	if (file.readUint32BE() != MKID_BE('SZDD'))
		return false;

	// Second part of the signature
	if (file.readUint32BE() != 0x88F02733)
		return false;

	// Compression mode must be 'A'
	if (file.readByte() != 'A')
		return false;

	file.readByte(); // file name character change
	uint32 unpackedLength = file.readUint32LE();

	byte *window = new byte[0x1000];
	int pos = 0x1000 - 16;
	memset(window, 0x20, 0x1000); // Initialize to all spaces

	byte *unpackedData = (byte *)malloc(unpackedLength);
	byte *dataPos = unpackedData;

	// Apply simple LZSS decompression
	for (;;) {
		byte controlByte = file.readByte();

		if (file.eos())
			break;

		for (byte i = 0; i < 8; i++) {
			if (controlByte & (1 << i)) {
				*dataPos++ = window[pos++] = file.readByte();
				pos &= 0xFFF;
			} else {
				int matchPos = file.readByte();
				int matchLen = file.readByte();
				matchPos |= (matchLen & 0xF0) << 4;
				matchLen = (matchLen & 0xF) + 3;
				while (matchLen--) {
					*dataPos++ = window[pos++] = window[matchPos++];
					pos &= 0xFFF;
					matchPos &= 0xFFF;
				}
			}
				
		}
	}

	delete[] window;
	SeekableReadStream *stream = new MemoryReadStream(unpackedData, unpackedLength);

	return loadFromEXE(stream);
}

uint32 NEResources::getResourceTableOffset() {
	if (!_exe)
		return 0xFFFFFFFF;

	if (!_exe->seek(0))
		return 0xFFFFFFFF;

	//                          'MZ'
	if (_exe->readUint16BE() != 0x4D5A)
		return 0xFFFFFFFF;

	if (!_exe->seek(60))
		return 0xFFFFFFFF;

	uint32 offsetSegmentEXE = _exe->readUint16LE();
	if (!_exe->seek(offsetSegmentEXE))
		return 0xFFFFFFFF;

	//                          'NE'
	if (_exe->readUint16BE() != 0x4E45)
		return 0xFFFFFFFF;

	if (!_exe->seek(offsetSegmentEXE + 36))
		return 0xFFFFFFFF;

	uint32 offsetResourceTable = _exe->readUint16LE();
	if (offsetResourceTable == 0)
		// No resource table
		return 0;

	// Offset relative to the segment _exe header
	offsetResourceTable += offsetSegmentEXE;
	if (!_exe->seek(offsetResourceTable))
		return 0xFFFFFFFF;

	return offsetResourceTable;
}

static const char *s_resTypeNames[] = {
	"", "cursor", "bitmap", "icon", "menu", "dialog", "string",
	"font_dir", "font", "accelerator", "rc_data", "msg_table",
	"group_cursor", "group_icon", "version", "dlg_include",
	"plug_play", "vxd", "ani_cursor", "ani_icon", "html",
	"manifest"
};

bool NEResources::readResourceTable(uint32 offset) {
	if (!_exe)
		return false;

	if (!_exe->seek(offset))
		return false;

	uint32 align = 1 << _exe->readUint16LE();

	uint16 typeID = _exe->readUint16LE();
	while (typeID != 0) {
		uint16 resCount = _exe->readUint16LE();

		_exe->skip(4); // reserved

		for (int i = 0; i < resCount; i++) {
			Resource res;

			// Resource properties
			res.offset = _exe->readUint16LE() * align;
			res.size   = _exe->readUint16LE() * align;
			res.flags  = _exe->readUint16LE();
			uint16 id  = _exe->readUint16LE();
			res.handle = _exe->readUint16LE();
			res.usage  = _exe->readUint16LE();

			res.type = typeID;

			if ((id & 0x8000) == 0)
				res.id = getResourceString(*_exe, offset + id);
			else
				res.id = id & 0x7FFF;

			if (typeID & 0x8000 && ((typeID & 0x7FFF) < ARRAYSIZE(s_resTypeNames)))
				debug(2, "Found resource %s %s", s_resTypeNames[typeID & 0x7FFF], res.id.toString().c_str());
			else
				debug(2, "Found resource %04x %s", typeID, res.id.toString().c_str());

			_resources.push_back(res);
		}

		typeID = _exe->readUint16LE();
	}

	return true;
}

String NEResources::getResourceString(SeekableReadStream &exe, uint32 offset) {
	uint32 curPos = exe.pos();

	if (!exe.seek(offset)) {
		exe.seek(curPos);
		return "";
	}

	uint8 length = exe.readByte();

	String string;
	for (uint16 i = 0; i < length; i++)
		string += (char)exe.readByte();

	exe.seek(curPos);
	return string;
}

const NEResources::Resource *NEResources::findResource(uint16 type, NEResourceID id) const {
	for (List<Resource>::const_iterator it = _resources.begin(); it != _resources.end(); ++it)
		if (it->type == type && it->id == id)
			return &*it;

	return 0;
}

SeekableReadStream *NEResources::getResource(uint16 type, NEResourceID id) {
	const Resource *res = findResource(type, id);

	if (!res)
		return 0;

	_exe->seek(res->offset);
	return _exe->readStream(res->size);
}

const Array<NEResourceID> NEResources::getIDList(uint16 type) const {
	Array<NEResourceID> idArray;

	for (List<Resource>::const_iterator it = _resources.begin(); it != _resources.end(); ++it)
		if (it->type == type)
			idArray.push_back(it->id);

	return idArray;
}

bool NEResources::readCursors() {
	uint32 cursorCount = 0;

	for (List<Resource>::const_iterator it = _resources.begin(); it != _resources.end(); ++it)
		if (it->type == kNEGroupCursor)
			cursorCount++;

	if (cursorCount == 0) {
		_cursors.clear();
		return true;
	}

	_cursors.resize(cursorCount);

	Array<NECursorGroup>::iterator cursorGroup = _cursors.begin();
	for (List<Resource>::const_iterator it = _resources.begin(); it != _resources.end(); ++it) {
		if (it->type == kNEGroupCursor) {
			if (!readCursorGroup(*cursorGroup, *it))
				return false;

			++cursorGroup;
		}
	}

	return true;
}

bool NEResources::readCursorGroup(NECursorGroup &group, const Resource &resource) {
	if (!_exe)
		return false;

	if (resource.size <= 6)
		return false;

	if (!_exe->seek(resource.offset))
		return false;

	byte *data = new byte[resource.size];

	if (!_exe->read(data, resource.size)) {
		delete[] data;
		return false;
	}

	uint32 cursorCount = READ_LE_UINT16(data + 4);
	if (resource.size < (6 + cursorCount * 16)) {
		delete[] data;
		return false;
	}

	group.cursors.resize(cursorCount);

	uint32 offset = 6;
	for (uint32 i = 0; i < cursorCount; i++) {
		group.cursors[i] = new NECursor();
		NECursor *cursor = group.cursors[i];

		// Plane count
		if (READ_LE_UINT16(data + offset + 4) != 1) {
			delete[] data;
			return false;
		}

		// Bit count
		if (READ_LE_UINT16(data + offset + 6) != 1) {
			delete[] data;
			return false;
		}

		uint32 id = READ_LE_UINT32(data + offset + 12);
		const Resource *cursorResource = findResource(kNECursor, id);
		if (!cursorResource) {
			delete[] data;
			return false;
		}

		cursor->setDimensions(READ_LE_UINT16(data + offset), READ_LE_UINT16(data + offset + 2) / 2);

		uint32 dataSize = READ_LE_UINT32(data + offset +  8);
		if (!readCursor(*cursor, *cursorResource, dataSize)) {
			delete[] data;
			return false;
		}

		offset += 16;
	}

	group.id = resource.id;

	delete[] data;
	return true;
}

bool NEResources::readCursor(NECursor &cursor, const Resource &resource, uint32 size) {
	if (!_exe)
		return false;

	if (size <= 4)
		return false;
	if (resource.size < size)
		return false;

	if (!_exe->seek(resource.offset))
		return false;

	uint32 hotspotX = _exe->readUint16LE();
	uint32 hotspotY = _exe->readUint16LE();
	cursor.setHotspot(hotspotX, hotspotY);

	size -= 4;

	if (!cursor.readCursor(*_exe, size))
		return false;

	return true;
}

} // End of namespace Common
