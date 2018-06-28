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

#include "illusions/graphics.h"

namespace Illusions {

// WidthHeight

void WidthHeight::load(Common::SeekableReadStream &stream) {
	_width = stream.readSint16LE();
	_height = stream.readSint16LE();

	debug(5, "WidthHeight::load() _width: %d; _height: %d",
		_width, _height);
}

// SurfInfo

void SurfInfo::load(Common::SeekableReadStream &stream) {
	_pixelSize = stream.readUint32LE();
	_dimensions.load(stream);

	debug(5, "SurfInfo::load() _pixelSize: %d",
		_pixelSize);
}

// NamedPoint

void NamedPoint::load(Common::SeekableReadStream &stream) {
	_namedPointId = stream.readUint32LE();
	loadPoint(stream, _pt);
}

// NamedPoints

bool NamedPoints::findNamedPoint(uint32 namedPointId, Common::Point &pt) {
	for (ItemsIterator it = _namedPoints.begin(); it != _namedPoints.end(); ++it) {
		if ((*it)._namedPointId == namedPointId) {
			pt = (*it)._pt;
			return true;
		}
	}
	return false;
}

void NamedPoints::load(uint count, Common::SeekableReadStream &stream) {
	_namedPoints.reserve(count);
	for (uint i = 0; i < count; ++i) {
		NamedPoint namedPoint;
		namedPoint.load(stream);
		_namedPoints.push_back(namedPoint);
		debug(0, "namedPoint(%08X, %d, %d)", namedPoint._namedPointId, namedPoint._pt.x, namedPoint._pt.y);
	}
}

void loadPoint(Common::SeekableReadStream &stream, Common::Point &pt) {
	pt.x = stream.readSint16LE();
	pt.y = stream.readSint16LE();
	debug(0, "loadPoint() x: %d; y: %d", pt.x, pt.y);
}

} // End of namespace Illusions
