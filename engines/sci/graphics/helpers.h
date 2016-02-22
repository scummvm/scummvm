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

#ifndef SCI_GRAPHICS_HELPERS_H
#define SCI_GRAPHICS_HELPERS_H

#include "common/endian.h"	// for READ_LE_UINT16
#include "common/rect.h"
#include "common/serializer.h"
#ifdef ENABLE_SCI32
#include "common/rational.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#endif
#include "sci/engine/vm_types.h"

namespace Sci {

// Cache limits
#define MAX_CACHED_CURSORS 10
#define MAX_CACHED_FONTS 20
#define MAX_CACHED_VIEWS 50

#define SCI_SHAKE_DIRECTION_VERTICAL 1
#define SCI_SHAKE_DIRECTION_HORIZONTAL 2

typedef int GuiResourceId; // is a resource-number and -1 means no parameter given

typedef int16 TextAlignment;

#define PORTS_FIRSTWINDOWID 2
#define PORTS_FIRSTSCRIPTWINDOWID 3

#ifdef ENABLE_SCI32
#define PRINT_RECT(x) (x).left,(x).top,(x).right,(x).bottom
#endif

struct Port {
	uint16 id;
	int16 top, left;
	Common::Rect rect;
	int16 curTop, curLeft;
	int16 fontHeight;
	GuiResourceId fontId;
	bool greyedOutput;
	int16 penClr, backClr;
	int16 penMode;
	uint16 counterTillFree;

	Port(uint16 theId) : id(theId), top(0), left(0),
		curTop(0), curLeft(0),
		fontHeight(0), fontId(0), greyedOutput(false),
		penClr(0), backClr(0xFF), penMode(0), counterTillFree(0) {
	}

	bool isWindow() const { return id >= PORTS_FIRSTWINDOWID && id != 0xFFFF; }
};

struct Window : public Port, public Common::Serializable {
	Common::Rect dims; // client area of window
	Common::Rect restoreRect; // total area of window including borders
	uint16 wndStyle;
	uint16 saveScreenMask;
	reg_t hSaved1;
	reg_t hSaved2;
	Common::String title;
	bool bDrawn;

	Window(uint16 theId) : Port(theId),
		wndStyle(0), saveScreenMask(0),
		hSaved1(NULL_REG), hSaved2(NULL_REG),
		bDrawn(false) {
	}

	void syncRect(Common::Serializer &ser, Common::Rect &targetRect) {
		ser.syncAsSint16LE(targetRect.top);
		ser.syncAsSint16LE(targetRect.left);
		ser.syncAsSint16LE(targetRect.bottom);
		ser.syncAsSint16LE(targetRect.right);
	}

	virtual void saveLoadWithSerializer(Common::Serializer &ser) {
		ser.syncAsUint16LE(id);
		ser.syncAsSint16LE(top);
		ser.syncAsSint16LE(left);
		syncRect(ser, rect);
		ser.syncAsSint16LE(curTop);
		ser.syncAsSint16LE(curLeft);
		ser.syncAsSint16LE(fontHeight);
		ser.syncAsSint32LE(fontId);
		ser.syncAsByte(greyedOutput);
		ser.syncAsSint16LE(penClr);
		ser.syncAsSint16LE(backClr);
		ser.syncAsSint16LE(penMode);
		ser.syncAsUint16LE(counterTillFree);
		syncRect(ser, dims);
		syncRect(ser, restoreRect);
		ser.syncAsUint16LE(wndStyle);
		ser.syncAsUint16LE(saveScreenMask);
		if (ser.isLoading()) {
			// The hunk table isn't saved, so we just set both pointers to NULL
			hSaved1 = NULL_REG;
			hSaved2 = NULL_REG;
		}
		ser.syncString(title);
		ser.syncAsByte(bDrawn);
	}
};

#ifdef ENABLE_SCI32
/**
 * Multiplies a rectangle by two ratios with default
 * rounding. Modifies the rect directly.
 */
inline void mul(Common::Rect &rect, const Common::Rational &ratioX, const Common::Rational &ratioY) {
	rect.left = (rect.left * ratioX).toInt();
	rect.top = (rect.top * ratioY).toInt();
	rect.right = (rect.right * ratioX).toInt();
	rect.bottom = (rect.bottom * ratioY).toInt();
}

/**
 * Multiplies a number by a rational number, rounding up to
 * the nearest whole number.
 */
inline int mulru(const int value, const Common::Rational &ratio) {
	int num = value * ratio.getNumerator();
	int result = num / ratio.getDenominator();
	if (num > ratio.getDenominator() && num % ratio.getDenominator()) {
		++result;
	}
	return result;
}

/**
 * Multiplies a point by two rational numbers for X and Y,
 * rounding up to the nearest whole number. Modifies the
 * point directly.
 */
inline void mulru(Common::Point &point, const Common::Rational &ratioX, const Common::Rational &ratioY) {
	point.x = mulru(point.x, ratioX);
	point.y = mulru(point.y, ratioY);
}

/**
 * Multiplies a point by two rational numbers for X and Y,
 * rounding up to the nearest whole number. Modifies the
 * rect directly.
 *
 * @note In SCI engine, the bottom-right corner of rects
 * received an additional one pixel during the
 * multiplication in order to round up to include the
 * bottom-right corner. Since ScummVM rects do not include
 * the bottom-right corner, doing this ends up making rects
 * a pixel too wide/tall depending upon the remainder.
 */
inline void mulru(Common::Rect &rect, const Common::Rational &ratioX, const Common::Rational &ratioY) {
	rect.left = mulru(rect.left, ratioX);
	rect.top = mulru(rect.top, ratioY);
	rect.right = mulru(rect.right, ratioX);
	rect.bottom = mulru(rect.bottom, ratioY);
}

struct Buffer : public Graphics::Surface {
	uint16 screenWidth;
	uint16 screenHeight;
	uint16 scriptWidth;
	uint16 scriptHeight;

	Buffer(const uint16 width, const uint16 height, uint8 *const pix) :
		screenWidth(width),
		screenHeight(height),
		// TODO: These values are not correct for all games. Script
		// dimensions were hard-coded per game in the original
		// interpreter. Search all games for their internal script
		// dimensions and set appropriately. (This code does not
		// appear to exist at all in SCI3, which uses 640x480.)
		scriptWidth(320),
		scriptHeight(200) {
		init(width, height, width, pix, Graphics::PixelFormat::createFormatCLUT8());
	}

	void clear(const uint8 value) {
		memset(pixels, value, w * h);
	}

	inline uint8 *getAddress(const uint16 x, const uint16 y) {
		return (uint8 *)getBasePtr(x, y);
	}

	inline uint8 *getAddressSimRes(const uint16 x, const uint16 y) {
		return (uint8*)pixels + (y * w * screenHeight / scriptHeight) + (x * screenWidth / scriptWidth);
	}

	bool isNull() {
		return pixels == nullptr;
	}
};
#endif

struct Color {
	byte used;
	byte r, g, b;

#ifdef ENABLE_SCI32
	bool operator==(const Color &other) const {
		return used == other.used && r == other.r && g == other.g && b == other.b;
	}
	inline bool operator!=(const Color &other) const {
		return !(*this == other);
	}
#endif
};

struct Palette {
	byte mapping[256];
	uint32 timestamp;
	Color colors[256];
	byte intensity[256];

#ifdef ENABLE_SCI32
	bool operator==(const Palette &other) const {
		for (int i = 0; i < ARRAYSIZE(colors); ++i) {
			if (colors[i] != other.colors[i]) {
				return false;
			}
		}

		return true;
	}
	inline bool operator!=(const Palette &other) const {
		return !(*this == other);
	}
#endif
};

struct PalSchedule {
	byte from;
	uint32 schedule;
};

// Game view types, sorted by the number of colors
enum ViewType {
	kViewUnknown,   // uninitialized, or non-SCI
	kViewEga,       // EGA SCI0/SCI1 and Amiga SCI0/SCI1 ECS 16 colors
	kViewAmiga,     // Amiga SCI1 ECS 32 colors
	kViewAmiga64,   // Amiga SCI1 AGA 64 colors (i.e. Longbow)
	kViewVga,       // VGA SCI1 256 colors
	kViewVga11      // VGA SCI1.1 and newer 256 colors
};

} // End of namespace Sci

#endif
