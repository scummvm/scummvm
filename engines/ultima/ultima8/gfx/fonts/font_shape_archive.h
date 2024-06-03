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

#ifndef ULTIMA8_GFX_FONTS_FONTSHAPEARCHIVE_H
#define ULTIMA8_GFX_FONTS_FONTSHAPEARCHIVE_H

#include "ultima/ultima8/gfx/shape_archive.h"

namespace Ultima {
namespace Ultima8 {

class ShapeFont;

class FontShapeArchive : public ShapeArchive {
public:
	FontShapeArchive(uint16 id, Palette *pal = 0,
	                 const ConvertShapeFormat *format = 0)
		: ShapeArchive(id, pal, format) { }
	FontShapeArchive(Common::SeekableReadStream *rs, uint16 id, Palette *pal = 0,
	                 const ConvertShapeFormat *format = 0)
		: ShapeArchive(rs, id, pal, format) { }

	~FontShapeArchive() override { }

	//! load HVLeads from u8.ini
	void setHVLeads();

	ShapeFont *getFont(uint32 fontnum);

	void cache(uint32 fontnum) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
