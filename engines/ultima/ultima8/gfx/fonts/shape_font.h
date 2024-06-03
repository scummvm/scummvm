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

#ifndef ULTIMA8_GFX_FONTS_SHAPEFONT_H
#define ULTIMA8_GFX_FONTS_SHAPEFONT_H

#include "ultima/ultima8/gfx/fonts/font.h"
#include "ultima/ultima8/gfx/shape.h"

namespace Ultima {
namespace Ultima8 {

class ShapeFont : public Font, public Shape {
	int _height;
	int _baseLine;
	int _vLead;
	int _hLead;
	bool _crusaderCharMap;

public:
	ShapeFont(const uint8 *data, uint32 size, const ConvertShapeFormat *format,
	          const uint16 flexId, const uint32 shapenum);
	~ShapeFont() override;

	int getHeight() override;
	int getBaseline() override;
	int getBaselineSkip() override;

	int getWidth(char c);
	int getVlead() const {
		return _vLead;
	}
	int getHlead() const {
		return _hLead;
	}

	void setVLead(int vl) {
		_vLead = vl;
	}
	void setHLead(int hl) {
		_hLead = hl;
	}

	int charToFrameNum(char c) const;

	void getStringSize(const Std::string &text,
		int32 &width, int32 &height) override;

	RenderedText *renderText(const Std::string &text,
		unsigned int &remaining, int32 width = 0, int32 height = 0,
		TextAlign align = TEXT_LEFT, bool u8specials = false,
		bool pagebreaks = false,
		Std::string::size_type cursor = Std::string::npos) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
