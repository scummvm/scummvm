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

#ifndef GRAPHICS_VIEWS_GFX_SURFACE_H
#define GRAPHICS_VIEWS_GFX_SURFACE_H

#include "common/ptr.h"
#include "common/rect.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "graphics/views/rect.h"

namespace Graphics {
namespace Views {

class GfxSurface : public Graphics::ManagedSurface {
private:
	Common::SharedPtr<Graphics::Font> _font;
	Common::Point _textPos;
	byte _textColor = 255;
	byte _bgColor = 0;
	bool _scrollable = false;

	void newLine();

public:
	GfxSurface();
	GfxSurface(Graphics::ManagedSurface &surf, const Common::Rect &bounds);
	~GfxSurface();

	/**
	 * Write some text to the surface
	 */
	void writeString(const Common::Point &pt, const Common::String &str,
		Graphics::TextAlign align = Graphics::kTextAlignLeft);
	void writeString(const Common::String &str, Graphics::TextAlign align = Graphics::kTextAlignLeft);
	void writeString(const Common::Point &pt, const char *str, ...);
	void writeString(const char *str, ...);

	/**
	 * Write a character to the surface
	 * @param chr	Character to write
	 */
	void writeChar(uint32 chr);
	void writeChar(const Common::Point &pt, uint32 chr);

	/**
	 * Get the width of a string in pixels
	 * @param str	String to measure
	 * @return		Width of string
	*/
	int getStringWidth(const Common::String &str) const;

	void setTextPos(const Common::Point &pt);
	const Common::Point &getTextPos() const {
		return _textPos;
	}

	/**
	 * Set the text color
	 * @param color		New color palette index
	 * @return			Previous color
	 */
	byte setColor(byte color);
	void setColor(byte fgColor, byte bgColor);

	/**
	 * Swap foreground and background text color
	 */
	void reverseColor();

	/**
	 * Set whether surface area can scroll when text is writing beyond the bottom of the surface
	 * @param flag 
	 */
	void setScrollable(bool flag) {
		_scrollable = flag;
	}

	/**
	 * Custom blitting using xor of the destination rather than a straight copy
	 */
	void xorBlitFrom(const ManagedSurface &src, const Common::Point &destPos);
	void xorBlitFrom(const ManagedSurface &src, const Common::Rect &srcRect, const Common::Point &destPos);
};

} // namespace Views
} // namespace Graphics

#endif
