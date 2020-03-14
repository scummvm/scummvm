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

#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/misc/rect.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(GumpShapeArchive, ShapeArchive)

GumpShapeArchive::~GumpShapeArchive() {
	for (unsigned int i = 0; i < _gumpItemArea.size(); ++i)
		delete _gumpItemArea[i];
}

void GumpShapeArchive::loadGumpage(IDataSource *ds) {
	unsigned int total = ds->getSize() / 8;
	_gumpItemArea.resize(total + 1);
	for (unsigned int i = 1; i <= total; ++i) {
		int x, y, w, h;
		x = static_cast<int16>(ds->read2());
		y = static_cast<int16>(ds->read2());
		w = static_cast<int16>(ds->read2()) - x;
		h = static_cast<int16>(ds->read2()) - y;
		_gumpItemArea[i] = new Rect(x, y, w, h);
	}
}

Rect *GumpShapeArchive::getGumpItemArea(uint32 shapenum) {
	if (shapenum >= _gumpItemArea.size())
		return nullptr;
	return _gumpItemArea[shapenum];
}

} // End of namespace Ultima8
} // End of namespace Ultima
