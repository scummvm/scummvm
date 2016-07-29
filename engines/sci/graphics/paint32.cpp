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
	SciBitmap &bitmap = *_segMan->lookupBitmap(bitmapId);

	CelInfo32 celInfo;
	celInfo.type = kCelTypeMem;
	celInfo.bitmap = bitmapId;
	// SSCI stores the line color on `celInfo`, even though
	// this is not a `kCelTypeColor`, as a hack so that
	// `kUpdateLine` can get the originally used color
	celInfo.color = color;

	ScreenItem *screenItem = new ScreenItem(planeObject, celInfo, Common::Rect(startPoint.x, startPoint.y, startPoint.x + bitmap.getWidth(), startPoint.y + bitmap.getHeight()));
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

	const uint32 index = properties.bitmap->getWidth() * y + x;

	if (index < properties.bitmap->getDataSize()) {
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
	} else {
		warning("GfxPaint32::plotter: Attempted to write out of bounds (%u >= %u)", index, properties.bitmap->getDataSize());
	}
}

reg_t GfxPaint32::makeLineBitmap(const Common::Point &startPoint, const Common::Point &endPoint, const int16 priority, const uint8 color, const LineStyle style, uint16 pattern, uint8 thickness, Common::Rect &outRect) {
	const uint8 skipColor = color != kDefaultSkipColor ? kDefaultSkipColor : 0;

	// Thickness is expected to be 2n+1
	thickness = ((MAX((uint8)1, thickness) - 1) | 1);
	const uint8 halfThickness = thickness >> 1;

	outRect.left = (startPoint.x < endPoint.x ? startPoint.x : endPoint.x) - halfThickness;
	outRect.top = (startPoint.y < endPoint.y ? startPoint.y : endPoint.y) - halfThickness;
	outRect.right = (startPoint.x > endPoint.x ? startPoint.x : endPoint.x) + halfThickness + 1;
	outRect.bottom = (startPoint.y > endPoint.y ? startPoint.y : endPoint.y) + halfThickness + 1;

	reg_t bitmapId;
	SciBitmap &bitmap = *_segMan->allocateBitmap(&bitmapId, outRect.width(), outRect.height(), skipColor, 0, 0, g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth, g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight, 0, false, true);

	byte *pixels = bitmap.getPixels();
	memset(pixels, skipColor, bitmap.getWidth() * bitmap.getHeight());

	LineProperties properties;
	properties.bitmap = &bitmap;

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
	}

	const Common::Rect drawRect(
		startPoint.x - outRect.left,
		startPoint.y - outRect.top,
		endPoint.x - outRect.left,
		endPoint.y - outRect.top
	);

	if (!properties.solid) {
		for (int i = 0; i < ARRAYSIZE(properties.pattern); ++i) {
			properties.pattern[i] = (pattern & 0x8000);
			pattern <<= 1;
		}

		properties.patternIndex = 0;
		properties.horizontal = ABS(drawRect.right - drawRect.left) > ABS(drawRect.bottom - drawRect.top);
		properties.lastAddress = properties.horizontal ? drawRect.left : drawRect.top;
	}

	if (thickness <= 1) {
		Graphics::drawLine(drawRect.left, drawRect.top, drawRect.right, drawRect.bottom, color, plotter, &properties);
	} else {
		Graphics::drawThickLine2(drawRect.left, drawRect.top, drawRect.right, drawRect.bottom, thickness, color, plotter, &properties);
	}

	return bitmapId;
}


} // End of namespace Sci
