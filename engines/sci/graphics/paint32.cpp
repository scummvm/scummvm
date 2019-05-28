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

#include "graphics/primitives.h"
#include "sci/engine/seg_manager.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/text32.h"

namespace Sci {

GfxPaint32::GfxPaint32(SegManager *segMan) :
	_segMan(segMan) {}

reg_t GfxPaint32::kernelAddLine(const reg_t planeObject, const Common::Point &startPoint, const Common::Point &endPoint, const int16 priority, const uint8 color, const LineStyle style, const uint16 pattern, const uint8 thickness) {
	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(planeObject);
	if (plane == nullptr) {
		error("kAddLine: Plane %04x:%04x not found", PRINT_REG(planeObject));
	}

	Common::Rect gameRect;
	reg_t bitmapId = makeLineBitmap(startPoint, endPoint, priority, color, style, pattern, thickness, gameRect);

	CelInfo32 celInfo;
	celInfo.type = kCelTypeMem;
	celInfo.bitmap = bitmapId;
	// SSCI stores the line color on `celInfo`, even though this is not a
	// `kCelTypeColor`, as a hack so that `kUpdateLine` can get the originally
	// used color
	celInfo.color = color;

	ScreenItem *screenItem = new ScreenItem(planeObject, celInfo, gameRect);
	screenItem->_priority = priority;
	screenItem->_fixedPriority = true;

	plane->_screenItemList.add(screenItem);

	return screenItem->_object;
}

void GfxPaint32::kernelUpdateLine(ScreenItem *screenItem, Plane *plane, const Common::Point &startPoint, const Common::Point &endPoint, const int16 priority, const uint8 color, const LineStyle style, const uint16 pattern, const uint8 thickness) {

	Common::Rect gameRect;
	reg_t bitmapId = makeLineBitmap(startPoint, endPoint, priority, color, style, pattern, thickness, gameRect);

	_segMan->freeBitmap(screenItem->_celInfo.bitmap);
	screenItem->_celInfo.bitmap = bitmapId;
	screenItem->_celInfo.color = color;
	screenItem->_position = startPoint;
	screenItem->_priority = priority;
	screenItem->update();
}

void GfxPaint32::kernelDeleteLine(const reg_t screenItemObject, const reg_t planeObject) {
	Plane *plane = g_sci->_gfxFrameout->getPlanes().findByObject(planeObject);
	if (plane == nullptr) {
		return;
	}

	ScreenItem *screenItem = plane->_screenItemList.findByObject(screenItemObject);
	if (screenItem == nullptr) {
		return;
	}

	_segMan->freeBitmap(screenItem->_celInfo.bitmap);
	g_sci->_gfxFrameout->deleteScreenItem(*screenItem, *plane);
}

void GfxPaint32::plotter(int x, int y, int color, void *data) {
	LineProperties &properties = *static_cast<LineProperties *>(data);
	byte *pixels = properties.bitmap->getPixels();

	const uint16 bitmapWidth  = properties.bitmap->getWidth();
	const uint16 bitmapHeight = properties.bitmap->getHeight();
	const uint32 index = bitmapWidth * y + x;

	// Only draw the points in the bitmap, and ignore the rest. SSCI scripts
	// can draw lines ending outside the visible area (e.g. negative
	// coordinates)
	if (x >= 0 && x < bitmapWidth && y >= 0 && y < bitmapHeight) {
		if (properties.solid) {
			pixels[index] = (uint8)color;
			return;
		}

		if (properties.horizontal && x != properties.lastAddress) {
			properties.lastAddress = x;
			++properties.patternIndex;
		} else if (!properties.horizontal && y != properties.lastAddress) {
			properties.lastAddress = y;
			++properties.patternIndex;
		}

		if (properties.pattern[properties.patternIndex]) {
			pixels[index] = (uint8)color;
		}

		if (properties.patternIndex == ARRAYSIZE(properties.pattern)) {
			properties.patternIndex = 0;
		}
	}
}

reg_t GfxPaint32::makeLineBitmap(const Common::Point &startPoint, const Common::Point &endPoint, const int16 priority, const uint8 color, const LineStyle style, uint16 pattern, uint8 thickness, Common::Rect &outRect) {
	const uint8 skipColor = color != kDefaultSkipColor ? kDefaultSkipColor : 0;

	// Line thickness is expected to be 2n + 1
	thickness = (MAX<uint8>(1, thickness) - 1) | 1;
	const uint8 halfThickness = thickness >> 1;

	const uint16 scriptWidth  = g_sci->_gfxFrameout->getScriptWidth();
	const uint16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

	outRect.left   = MIN<int16>(startPoint.x, endPoint.x);
	outRect.top    = MIN<int16>(startPoint.y, endPoint.y);
	outRect.right  = MAX<int16>(startPoint.x, endPoint.x) + 1;
	outRect.bottom = MAX<int16>(startPoint.y, endPoint.y) + 1;

	outRect.grow(halfThickness);
	outRect.clip(Common::Rect(scriptWidth, scriptHeight));

	reg_t bitmapId;
	SciBitmap &bitmap = *_segMan->allocateBitmap(&bitmapId, outRect.width(), outRect.height(), skipColor, 0, 0, scriptWidth, scriptHeight, 0, false, true);

	byte *pixels = bitmap.getPixels();
	memset(pixels, skipColor, bitmap.getWidth() * bitmap.getHeight());

	LineProperties properties;
	properties.bitmap = &bitmap;
	properties.solid = true;

	switch (style) {
	case kLineStyleSolid:
		pattern = 0xFFFF;
		properties.solid = true;
		break;
	case kLineStyleDashed:
		pattern = 0xFF00;
		properties.solid = false;
		break;
	case kLineStylePattern:
		properties.solid = pattern == 0xFFFF;
		break;
	default:
		break;
	}

	// Change coordinates to be relative to the bitmap
	const int16 x1 = startPoint.x - outRect.left;
	const int16 y1 = startPoint.y - outRect.top;
	const int16 x2 =   endPoint.x - outRect.left;
	const int16 y2 =   endPoint.y - outRect.top;

	if (!properties.solid) {
		for (int i = 0; i < ARRAYSIZE(properties.pattern); ++i) {
			properties.pattern[i] = (pattern & 0x8000);
			pattern <<= 1;
		}

		properties.patternIndex = 0;
		properties.horizontal = ABS(x2 - x1) > ABS(y2 - y1);
		properties.lastAddress = properties.horizontal ? x1 : y1;
	}

	if (thickness <= 1) {
		Graphics::drawLine(x1, y1, x2, y2, color, plotter, &properties);
	} else {
		Graphics::drawThickLine2(x1, y1, x2, y2, thickness, color, plotter, &properties);
	}

	return bitmapId;
}


} // End of namespace Sci
