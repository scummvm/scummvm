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

#include "common/archive.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/stream.h"

#include "graphics/surface.h"

#include "prince/object.h"
#include "prince/resource.h"

namespace Prince {

Object::Object() : _surface(nullptr), _x(0), _y(0), _z(0), _flags(0), _width(0),
	_height(0), _zoomTime(0), _zoomSurface(nullptr)
{
}

Object::~Object() {
	if (_surface != nullptr) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}
	if (_zoomSurface != nullptr) {
		_zoomSurface->free();
		delete _zoomSurface;
		_zoomSurface = nullptr;
	}
}

void Object::loadSurface(Common::SeekableReadStream &stream) {
	stream.skip(4);
	_width = stream.readUint16LE();
	_height = stream.readUint16LE();
	_surface = new Graphics::Surface();
	_surface->create(_width, _height, Graphics::PixelFormat::createFormatCLUT8());

	for (int h = 0; h < _surface->h; h++) {
		stream.read(_surface->getBasePtr(0, h), _surface->w);
	}
}

bool Object::loadFromStream(Common::SeekableReadStream &stream) {
	int32 pos = stream.pos();
	uint16 x = stream.readUint16LE();
	if (x == 0xFFFF) {
		return false;
	}
	_x = x;
	_y = stream.readSint16LE(); // skull mini-game has some signed y coords

	const Common::String obStreamName = Common::String::format("OB%02d", stream.readUint16LE());
	Common::SeekableReadStream *obStream = SearchMan.createReadStreamForMember(obStreamName);
	if (obStream) {
		obStream = Resource::getDecompressedStream(obStream);

		loadSurface(*obStream);
	}
	delete obStream;

	_flags = stream.readUint16LE();
	_z = stream.readUint16LE();

	stream.seek(pos + 16);

	return true;
}

void Object::setData(AttrId dataId, int32 value) {
	switch (dataId) {
	case kObjectX:
		_x = value;
		break;
	case kObjectY:
		_y = value;
		break;
	default:
		assert(false);
	}
}

int32 Object::getData(AttrId dataId) {
	switch (dataId) {
	case kObjectX:
		return _x;
	case kObjectY:
		return _y;
	default:
		assert(false);
		return 0;
	}
}

} // End of namespace Prince
