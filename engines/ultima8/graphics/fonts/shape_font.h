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

#ifndef ULTIMA8_GRAPHICS_FONTS_SHAPEFONT_H
#define ULTIMA8_GRAPHICS_FONTS_SHAPEFONT_H

#include "ultima8/graphics/fonts/font.h"
#include "Shape.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

class ShapeFont : public Pentagram::Font, public Shape {
	int height;
	int baseline;
	int vlead;
	int hlead;

public:
	ShapeFont(const uint8 *data, uint32 size, const ConvertShapeFormat *format,
	          const uint16 flexId, const uint32 shapenum);
	virtual ~ShapeFont();

	virtual int getHeight();
	virtual int getBaseline();
	virtual int getBaselineSkip();

	int getWidth(char c);
	int getVlead() const {
		return vlead;
	}
	int getHlead() const {
		return hlead;
	}

	void setVLead(int vl) {
		vlead = vl;
	}
	void setHLead(int hl) {
		hlead = hl;
	}

	virtual void getStringSize(const std::string &text,
	                           int &width, int &height);

	virtual RenderedText *renderText(const std::string &text,
	                                 unsigned int &remaining,
	                                 int width = 0, int height = 0,
	                                 TextAlign align = TEXT_LEFT,
	                                 bool u8specials = false,
	                                 std::string::size_type cursor
	                                 = std::string::npos);

	ENABLE_RUNTIME_CLASSTYPE();
};

} // End of namespace Ultima8

#endif
