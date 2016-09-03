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
 */

#include "common/scummsys.h"

#include "zvision/graphics/cursors/cursor.h"

#include "common/str.h"
#include "common/file.h"

namespace ZVision {

ZorkCursor::ZorkCursor()
	: _width(0),
	  _height(0),
	  _hotspotX(0),
	  _hotspotY(0) {
}

ZorkCursor::ZorkCursor(ZVision *engine, const Common::String &fileName)
	: _width(0),
	  _height(0),
	  _hotspotX(0),
	  _hotspotY(0) {
	Common::File file;
	if (!engine->getSearchManager()->openFile(file, fileName))
		error("Cursor file %s does not exist", fileName.c_str());

	uint32 magic = file.readUint32BE();
	if (magic != MKTAG('Z', 'C', 'R', '1')) {
		warning("%s is not a Zork Cursor file", fileName.c_str());
		return;
	}

	_hotspotX = file.readUint16LE();
	_hotspotY = file.readUint16LE();
	_width = file.readUint16LE();
	_height = file.readUint16LE();

	uint dataSize = _width * _height * sizeof(uint16);
	_surface.create(_width, _height, engine->_resourcePixelFormat);
	uint32 bytesRead = file.read(_surface.getPixels(), dataSize);
	assert(bytesRead == dataSize);

#ifndef SCUMM_LITTLE_ENDIAN
	int16 *buffer = (int16 *)_surface.getPixels();
	for (uint32 i = 0; i < dataSize / 2; ++i)
		buffer[i] = FROM_LE_16(buffer[i]);
#endif
}

ZorkCursor::ZorkCursor(const ZorkCursor &other) {
	_width = other._width;
	_height = other._height;
	_hotspotX = other._hotspotX;
	_hotspotY = other._hotspotY;

	_surface.copyFrom(other._surface);
}

ZorkCursor &ZorkCursor::operator=(const ZorkCursor &other) {
	_width = other._width;
	_height = other._height;
	_hotspotX = other._hotspotX;
	_hotspotY = other._hotspotY;

	_surface.free();
	_surface.copyFrom(other._surface);

	return *this;
}

ZorkCursor::~ZorkCursor() {
	_surface.free();
}

} // End of namespace ZVision
