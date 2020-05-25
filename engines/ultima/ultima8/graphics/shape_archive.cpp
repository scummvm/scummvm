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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/graphics/shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/convert/convert_shape.h"

namespace Ultima {
namespace Ultima8 {

ShapeArchive::~ShapeArchive() {
	Archive::uncache();
}

Shape *ShapeArchive::getShape(uint32 shapenum) {
	if (shapenum >= _count)
		return nullptr;
	cache(shapenum);

	return _shapes[shapenum];
}

void ShapeArchive::cache(uint32 shapenum) {
	if (shapenum >= _count) return;
	if (_shapes.empty()) _shapes.resize(_count);

	if (_shapes[shapenum]) return;

	uint32 shpsize;
	uint8 *data = getRawObject(shapenum, &shpsize);

	if (!data || shpsize == 0) return;

	// Auto detect format
	if (!_format) {
		_format = Shape::DetectShapeFormat(data, shpsize);
		if (_format) pout << "Detected Shape Format: " << _format->_name << Std::endl;
	}

	if (!_format) {
		delete [] data;
		perr << "Error: Unable to detect shape format for flex." << Std::endl;
		return;
	}

	Shape *shape = new Shape(data, shpsize, _format, _id, shapenum);
	if (_palette) shape->setPalette(_palette);

	_shapes[shapenum] = shape;
}

void ShapeArchive::uncache(uint32 shapenum) {
	if (shapenum >= _count) return;
	if (_shapes.empty()) return;

	delete _shapes[shapenum];
	_shapes[shapenum] = nullptr;
}

bool ShapeArchive::isCached(uint32 shapenum) const {
	if (shapenum >= _count) return false;
	if (_shapes.empty()) return false;

	return (_shapes[shapenum] != nullptr);
}

} // End of namespace Ultima8
} // End of namespace Ultima
