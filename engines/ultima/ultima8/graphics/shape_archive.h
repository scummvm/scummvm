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

#ifndef ULTIMA8_GRAPHICS_SHAPEARCHIVE_H
#define ULTIMA8_GRAPHICS_SHAPEARCHIVE_H

#include "ultima/ultima8/filesys/archive.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class Shape;
struct ConvertShapeFormat;
struct Palette;

class ShapeArchive : public Archive {
public:
	ShapeArchive(uint16 id, Palette *pal = 0,
	             const ConvertShapeFormat *format = 0)
		: Archive(), _id(id), _format(format), _palette(pal) { }
	ShapeArchive(ArchiveFile *af, uint16 id, Palette *pal = 0,
	             const ConvertShapeFormat *format = 0)
		: Archive(af), _id(id), _format(format), _palette(pal) { }
	ShapeArchive(Common::SeekableReadStream *rs, uint16 id, Palette *pal = 0,
	             const ConvertShapeFormat *format = 0)
		: Archive(rs), _id(id), _format(format), _palette(pal) { }

	~ShapeArchive() override;

	Shape *getShape(uint32 shapenum);

	void cache(uint32 shapenum) override;
	void uncache(uint32 shapenum) override;
	bool isCached(uint32 shapenum) const override;

protected:
	uint16 _id;
	const ConvertShapeFormat *_format;
	Palette *_palette;
	Std::vector<Shape *> _shapes;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
