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

#ifndef SCUMM_CHARSET_V7_H
#define SCUMM_CHARSET_V7_H

#ifdef ENABLE_SCUMM_7_8

#include "common/rect.h"

namespace Scumm {

enum TextStyleFlags {
	kStyleAlignLeft		=	0x00,
	kStyleAlignCenter	=	0x01,
	kStyleAlignRight	=	0x02,
	kStyleWordWrap		=	0x04
	// There are more flags, but I'll add them only as needed
};

class GlyphRenderer_v7 {
public:
	virtual ~GlyphRenderer_v7() {};

	virtual int draw2byte(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, uint16 chr) = 0;
	virtual int drawChar(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags, byte chr) = 0;
	virtual int getCharWidth(uint16 chr) const = 0;
	virtual int getCharHeight(uint16 chr) const = 0;
	virtual int getFontHeight() const = 0;
	virtual int setFont(int id) = 0;
	virtual bool newStyleWrapping() const = 0;
};

} // End of namespace Scumm

#endif
#endif
