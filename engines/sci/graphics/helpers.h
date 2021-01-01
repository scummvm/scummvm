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
#include "sci/detection.h"
#include "sci/engine/vm_types.h"
#include "sci/graphics/helpers_detection_enums.h" // for enum ViewType

namespace Sci {

// Cache limits
#define MAX_CACHED_CURSORS 10
#define MAX_CACHED_FONTS 20
#define MAX_CACHED_VIEWS 50

enum ShakeDirection {
	kShakeVertical   = 1,
	kShakeHorizontal = 2
};

typedef int GuiResourceId; // is a resource-number and -1 means no parameter given

typedef int16 TextAlignment;

#define PORTS_FIRSTWINDOWID 2
#define PORTS_FIRSTSCRIPTWINDOWID 3

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

	void saveLoadWithSerializer(Common::Serializer &ser) override {
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
 * Multiplies a rectangle by two ratios with default
 * rounding. Modifies the rect directly. Uses inclusive
 * rectangle rounding.
 */
inline void mulinc(Common::Rect &rect, const Common::Rational &ratioX, const Common::Rational &ratioY) {
	rect.left = (rect.left * ratioX).toInt();
	rect.top = (rect.top * ratioY).toInt();
	rect.right = ((rect.right - 1) * ratioX).toInt() + 1;
	rect.bottom = ((rect.bottom - 1) * ratioY).toInt() + 1;
}

/**
 * Multiplies a number by a rational number, rounding up to
 * the nearest whole number.
 */
inline int mulru(const int value, const Common::Rational &ratio, const int extra = 0) {
	int num = (value + extra) * ratio.getNumerator();
	int result = num / ratio.getDenominator();
	if (num > ratio.getDenominator() && num % ratio.getDenominator()) {
		++result;
	}
	return result - extra;
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
 */
inline void mulru(Common::Rect &rect, const Common::Rational &ratioX, const Common::Rational &ratioY, const int extra) {
	rect.left = mulru(rect.left, ratioX);
	rect.top = mulru(rect.top, ratioY);
	rect.right = mulru(rect.right - 1, ratioX, extra) + 1;
	rect.bottom = mulru(rect.bottom - 1, ratioY, extra) + 1;
}

/**
 * Determines the parts of `r` that aren't overlapped by `other`.
 * Returns -1 if `r` and `other` have no intersection.
 * Returns number of returned parts (in `outRects`) otherwise.
 * (In particular, this returns 0 if `r` is contained in `other`.)
 */
inline int splitRects(Common::Rect r, const Common::Rect &other, Common::Rect(&outRects)[4]) {
	if (!r.intersects(other)) {
		return -1;
	}

	int splitCount = 0;
	if (r.top < other.top) {
		Common::Rect &t = outRects[splitCount++];
		t = r;
		t.bottom = other.top;
		r.top = other.top;
	}

	if (r.bottom > other.bottom) {
		Common::Rect &t = outRects[splitCount++];
		t = r;
		t.top = other.bottom;
		r.bottom = other.bottom;
	}

	if (r.left < other.left) {
		Common::Rect &t = outRects[splitCount++];
		t = r;
		t.right = other.left;
		r.left = other.left;
	}

	if (r.right > other.right) {
		Common::Rect &t = outRects[splitCount++];
		t = r;
		t.left = other.right;
	}

	return splitCount;
}

typedef Graphics::Surface Buffer;
#endif

struct Color {
	byte used;
	byte r, g, b;

#ifdef ENABLE_SCI32
	bool operator==(const Color &other) const {
		return used == other.used && r == other.r && g == other.g && b == other.b;
	}
	inline bool operator!=(const Color &other) const {
		return !operator==(other);
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

struct PaletteMod {
	int8 r, g, b;
};

struct PicMod {
	uint16 id;
	byte multiplier;
};

struct ViewMod {
	uint16 id;
	int16 loop;
	int16 cel;
	byte multiplier;
};

struct SciFxMod {
	SciGameId gameId;
	const PaletteMod *paletteMods;
	const int paletteModsSize;
	const PicMod *picMods;
	const int picModsSize;
	const ViewMod *viewMods;
	const int viewModsSize;
};

struct PalSchedule {
	byte from;
	uint32 schedule;
};

} // End of namespace Sci

#endif
