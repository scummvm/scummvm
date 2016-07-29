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

#ifndef SCI_GRAPHICS_PAINT32_H
#define SCI_GRAPHICS_PAINT32_H

namespace Sci {
class Plane;
class SciBitmap;
class ScreenItem;
class SegManager;

enum LineStyle {
	kLineStyleSolid,
	kLineStyleDashed,
	kLineStylePattern
};

/**
 * Paint32 class, handles painting/drawing for SCI32 (SCI2+) games
 */
class GfxPaint32 {
public:
	GfxPaint32(SegManager *segMan);

private:
	SegManager *_segMan;

#pragma mark -
#pragma mark Line drawing
public:
	reg_t kernelAddLine(const reg_t planeObject, const Common::Point &startPoint, const Common::Point &endPoint, const int16 priority, const uint8 color, const LineStyle style, const uint16 pattern, const uint8 thickness);
	void kernelUpdateLine(ScreenItem *screenItem, Plane *plane, const Common::Point &startPoint, const Common::Point &endPoint, const int16 priority, const uint8 color, const LineStyle style, const uint16 pattern, const uint8 thickness);
	void kernelDeleteLine(const reg_t screenItemObject, const reg_t planeObject);

private:
	typedef struct {
		SciBitmap *bitmap;
		bool pattern[16];
		uint8 patternIndex;
		bool solid;
		bool horizontal;
		int lastAddress;
	} LineProperties;

	static void plotter(int x, int y, int color, void *data);

	reg_t makeLineBitmap(const Common::Point &startPoint, const Common::Point &endPoint, const int16 priority, const uint8 color, const LineStyle style, const uint16 pattern, const uint8 thickness, Common::Rect &outRect);
};

} // End of namespace Sci

#endif
