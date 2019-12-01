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

#include "ultima8/filesys/Archive.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class Shape;
struct ConvertShapeFormat;
namespace Pentagram {
struct Palette;
}

class ShapeArchive : public Pentagram::Archive {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	ShapeArchive(uint16 id_, Pentagram::Palette *pal_ = 0,
	             const ConvertShapeFormat *format_ = 0)
		: Archive(), id(id_), format(format_), palette(pal_) { }
	ShapeArchive(ArchiveFile *af, uint16 id_, Pentagram::Palette *pal_ = 0,
	             const ConvertShapeFormat *format_ = 0)
		: Archive(af), id(id_), format(format_), palette(pal_) { }
	ShapeArchive(IDataSource *ds, uint16 id_, Pentagram::Palette *pal_ = 0,
	             const ConvertShapeFormat *format_ = 0)
		: Archive(ds), id(id_), format(format_), palette(pal_) { }
	ShapeArchive(const std::string &path, uint16 id_,
	             Pentagram::Palette *pal_ = 0,
	             const ConvertShapeFormat *format_ = 0)
		: Archive(path), id(id_), format(format_), palette(pal_) { }

	virtual ~ShapeArchive();

	Shape *getShape(uint32 shapenum);

	virtual void cache(uint32 shapenum);
	virtual void uncache(uint32 shapenum);
	virtual bool isCached(uint32 shapenum);

protected:
	uint16 id;
	const ConvertShapeFormat *format;
	Pentagram::Palette *palette;
	std::vector<Shape *> shapes;
};

} // End of namespace Ultima8

#endif
