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

#include "adl/display.h"

#ifndef ADL_DISPLAY_A2_H
#	define ADL_DISPLAY_A2_H

namespace Adl {

class Display_A2 : public Display {
public:
	Display_A2();
	virtual ~Display_A2();

	enum {
		kGfxWidth = 280,
		kGfxHeight = 192,
		kGfxPitch = kGfxWidth / 7,
		kGfxSize = kGfxWidth * kGfxHeight,
		kTextWidth = 40,
		kTextHeight = 24
	};

	void init() override;
	bool saveThumbnail(Common::WriteStream &out) override;

	// Graphics
	uint getGfxWidth() const { return kGfxWidth; }
	uint getGfxHeight() const { return kGfxHeight; }
	uint getGfxPitch() const { return kGfxPitch; }
	void loadFrameBuffer(Common::ReadStream &stream, byte *dst);
	void loadFrameBuffer(Common::ReadStream &stream);
	void putPixel(const Common::Point &p, byte color);
	void setPixelByte(const Common::Point &p, byte color);
	void setPixelBit(const Common::Point &p, byte color);
	void setPixelPalette(const Common::Point &p, byte color);
	byte getPixelByte(const Common::Point &p) const;
	bool getPixelBit(const Common::Point &p) const;
	void clear(byte color);

	// Text
	char asciiToNative(char c) const override { return c | 0x80; }
	void printChar(char c) override;
	void showCursor(bool enable) override;

private:
	void writeFrameBuffer(const Common::Point &p, byte color, byte mask);
	void updateTextSurface() override;
	void updateGfxSurface() override;

	void showScanlines(bool enable);
	void drawChar(byte c, int x, int y);
	void createFont();

	byte *_frameBuf;
	bool _scanlines;
	bool _monochrome;
	Graphics::Surface *_font;
	bool _showCursor;
	uint32 _startMillis;
};

} // End of namespace Adl

#endif
