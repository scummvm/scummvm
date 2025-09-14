/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA8_GFX_GUMPSHAPEARCHIVE_H
#define ULTIMA8_GFX_GUMPSHAPEARCHIVE_H

#include "ultima/ultima8/gfx/shape_archive.h"
#include "common/rect.h"

namespace Ultima {
namespace Ultima8 {

class GumpShapeArchive : public ShapeArchive {
public:
	GumpShapeArchive(uint16 id, Palette *pal = 0,
	                 const ConvertShapeFormat *format = 0)
		: ShapeArchive(id, pal, format) { }
	GumpShapeArchive(Common::SeekableReadStream *rs, uint16 id, Palette *pal = 0,
	                 const ConvertShapeFormat *format = 0)
		: ShapeArchive(rs, id, pal, format) { }

	~GumpShapeArchive() override;

	void loadGumpage(Common::SeekableReadStream *rs);
	Common::Rect32 *getGumpItemArea(uint32 shapenum);

protected:
	Std::vector<Common::Rect32 *> _gumpItemArea;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
