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

#ifndef SCI_GRAPHICS_SCIFONT_H
#define SCI_GRAPHICS_SCIFONT_H

#include "sci/graphics/helpers.h"
#include "sci/util.h"

namespace Sci {

#ifdef ENABLE_SCI32
enum {
	kSci32SystemFont = -1
};
#endif

class GfxFont {
public:
	GfxFont() {}
	virtual ~GfxFont() {}

	virtual GuiResourceId getResourceId() { return 0; }
	virtual byte getHeight() { return 0; }
	virtual bool isDoubleByte(uint16 chr) { return false; }
	virtual byte getCharWidth(uint16 chr) { return 0; }
	virtual byte getCharHeight(uint16 chr) { return 0; }
	virtual void draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput) {}
	virtual void drawToBuffer(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput, byte *buffer, int16 width, int16 height) {}
};


/**
 * Font class, handles loading of font resources and drawing characters to screen
 *  every font resource has its own instance of this class
 */
class GfxFontFromResource : public GfxFont {
public:
	GfxFontFromResource(ResourceManager *resMan, GfxScreen *screen, GuiResourceId resourceId);
	~GfxFontFromResource() override;

	GuiResourceId getResourceId() override;
	uint8 getHeight() override;
	uint8 getCharWidth(uint16 chr) override;
	void draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput) override;
#ifdef ENABLE_SCI32
	// SCI2/2.1 equivalent
	void drawToBuffer(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput, byte *buffer, int16 width, int16 height) override;
#endif

private:
	uint8 getCharHeight(uint16 chr) override;
	SciSpan<const byte> getCharData(uint16 chr);

	ResourceManager *_resMan;
	GfxScreen *_screen;

	Resource *_resource;
	SciSpan<const byte> _resourceData;
	GuiResourceId _resourceId;

	struct Charinfo {
		uint8 width, height;
		int16 offset;
	};

	uint8 _fontHeight;
	uint16 _numChars;
	Charinfo *_chars;
};

} // End of namespace Sci

#endif
