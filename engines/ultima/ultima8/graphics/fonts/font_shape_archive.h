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

#ifndef ULTIMA8_GRAPHICS_FONTS_FONTSHAPEARCHIVE_H
#define ULTIMA8_GRAPHICS_FONTS_FONTSHAPEARCHIVE_H

#include "ultima/ultima8/graphics/shape_archive.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class ShapeFont;

class FontShapeArchive : public ShapeArchive {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	FontShapeArchive(uint16 id_, Palette *pal_ = 0,
	                 const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(id_, pal_, format_) { }
	FontShapeArchive(ArchiveFile *af, uint16 id_, Palette *pal_ = 0,
	                 const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(af, id_, pal_, format_) { }
	FontShapeArchive(IDataSource *ds, uint16 id_, Palette *pal_ = 0,
	                 const ConvertShapeFormat *format_ = 0)
		: ShapeArchive(ds, id_, pal_, format_) { }
 
	~FontShapeArchive() override { }

	//! load HVLeads from u8.ini
	void setHVLeads();

	ShapeFont *getFont(uint32 fontnum);

	void cache(uint32 fontnum) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
