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

void WidthHeight::load(Common::SeekableReadStream &stream) {
	_width = stream.readSint16LE();
	_height = stream.readSint16LE();
	
	debug(5, "WidthHeight::load() _width: %d; _height: %d",
		_width, _height);
}

void SurfInfo::load(Common::SeekableReadStream &stream) {
	_pixelSize = stream.readUint32LE();
	_dimensions.load(stream);
	
	debug(5, "SurfInfo::load() _pixelSize: %d",
		_pixelSize);
}

void loadPoint(Common::SeekableReadStream &stream, Common::Point &pt) {
	pt.x = stream.readSint16LE();
	pt.y = stream.readSint16LE();
	
	debug(5, "loadPoint() x: %d; y: %d",
		pt.x, pt.y);
}

} // End of namespace Illusions
