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
#define ADL_DISPLAY_A2_H

namespace Adl {

class Display_A2 : public Display {
public:
	Display_A2();
	~Display_A2() override;

	enum {
		kGfxWidth = 280,
		kGfxHeight = 192,
		kGfxPitch = kGfxWidth / 7,
		kGfxSize = kGfxPitch * kGfxHeight,
		kTextWidth = 40,
		kTextHeight = 24,
		kSplitHeight = 32
	};

	void init() override;

	// Graphics
	uint getGfxWidth() const { return kGfxWidth; }
	uint getGfxHeight() const { return kGfxHeight; }
	uint getGfxPitch() const { return kGfxPitch; }
	void loadFrameBuffer(Common::ReadStream &stream, byte *dst) const ;
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

protected:
	class TextReader {
	public:
		static uint16 getBits(const Display_A2 *display, uint y, uint x) {
			const uint charPos = (y >> 3) * kTextWidth + x;
			byte m = display->_textBuf[charPos];

			if (display->_showCursor && charPos == display->_cursorPos)
				m = (m & 0x3f) | 0x40;

			byte b = _font[m & 0x3f][y % 8];

			if (!(m & 0x80) && (!(m & 0x40) || display->_blink))
				b = ~b;

			return b & 0x7f;
		}

		static uint8 getStartY(const Display_A2 *display) {
			if (display->_mode == kModeText)
				return 0;
			else
				return kGfxHeight - kSplitHeight;
		}

		static uint8 getEndY(const Display_A2 *display) { return kGfxHeight; }
	};

	class GfxReader {
	public:
		static uint16 getBits(const Display_A2 *display, uint y, uint x) {
			return display->_frameBuf[y * kGfxPitch + x];
		}

		static uint8 getStartY(const Display_A2 *display) { return 0; }

		static uint8 getEndY(const Display_A2 *display) {
			if (display->_mode == kModeGraphics)
				return kGfxHeight;
			else
				return kGfxHeight - kSplitHeight;
		}
	};

	byte *_frameBuf;
	bool _showCursor;
	bool _enableColor;
	bool _enableScanlines;
	bool _enableMonoText;
	bool _blink;

private:
	void writeFrameBuffer(const Common::Point &p, byte color, byte mask);

	static const byte _font[64][8];
};

Display_A2 *Display_A2_create();

} // End of namespace Adl

#endif
