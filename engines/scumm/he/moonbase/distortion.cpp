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

#include "scumm/he/intern_he.h"
#include "engines/scumm/he/moonbase/moonbase.h"

namespace Scumm {

enum {
	kBptHeaderSize = 8,

	kReflectionClipped = 0,
	kNotClipped = 1,
	kSpecializedNotClipped = 2
};

static void blitDistortionCore(
	Graphics::Surface *dstBitmap,
	const int x, const int y,
	const Graphics::Surface *distortionBitmap,
	const Common::Rect *optionalclipRectPtr,
	int transferOp,
	const Graphics::Surface *srcBitmap,
	Common::Rect *srcClipRect
) {
	Common::Rect clipRect(dstBitmap->w, dstBitmap->h);

	if (optionalclipRectPtr) {
		if (!clipRect.intersects(*optionalclipRectPtr))
			return;

		clipRect.clip(*optionalclipRectPtr);
	}

	Common::Rect distortionRect(distortionBitmap->w, distortionBitmap->h);
	Common::Rect dstRect(x, y, x + distortionRect.width(), y + distortionRect.height());

	if (!dstRect.intersects(clipRect))
		return;

	dstRect.clip(clipRect);

	distortionRect.moveTo(dstRect.left - x, dstRect.top - y);

	const byte *distortionPtr = (const byte *)distortionBitmap->getBasePtr(distortionRect.left, distortionRect.top);
	byte *dstPtr = (byte *)dstBitmap->getBasePtr(dstRect.left, dstRect.top);
	int cw = dstRect.width();
	int ch = dstRect.height();
	int idx = dstRect.left;
	int dy = dstRect.top;

	int baseX, baseY;
	const byte *srcData = (const byte *)srcBitmap->getBasePtr(0, 0);
	int srcPitch = srcBitmap->pitch;

	switch (transferOp) {
	case kReflectionClipped:
	case kNotClipped:
		baseX = -(0x1f / 2); // Half range
		baseY = -(0x1f / 2);
		break;

	case kSpecializedNotClipped:
	default:
		baseX = 0;
		baseY = 0;
	}

	while (--ch >= 0) {
		uint16 *d = (uint16 *)dstPtr;
		const uint16 *is = (const uint16 *)distortionPtr;
		int dx = idx;

		for (int i = cw; --i >= 0;) {
			uint16 p = READ_LE_UINT16(is);
			int sx = baseX + dx + ((p >> 5) & 0x1f); // G color
			int sy = baseY + dy + (p & 0x1f);        // B color;

			if (transferOp == kReflectionClipped) {
				if (sx < srcClipRect->left)
					sx -= (srcClipRect->left - sx);

				if (sx > srcClipRect->right)
					sx -= (sx - srcClipRect->right);

				sx = MAX<int>(srcClipRect->left, MIN<int>(sx, srcClipRect->right));

				if (sy < srcClipRect->top)
					sy -= (srcClipRect->top - sy);

				if (sy > srcClipRect->bottom)
					sy -= (sy - srcClipRect->bottom);

				sy = MAX<int>(srcClipRect->top, MIN<int>(sy, srcClipRect->bottom));
			}

			*d = *((const uint16 *)(srcData + sy * srcPitch + sx * 2));

			++d;
			++is;
			++dx;
		}

		dstPtr += dstBitmap->pitch;
		distortionPtr += distortionBitmap->pitch;

		++dy;
	}
}

void Moonbase::blitDistortion(byte *bufferData, const int bufferWidth, const int bufferHeight, const int bufferPitch,
		const Common::Rect *optionalClippingRect, byte *dataStream, const int x, const int y, byte *altSourceBuffer) {
	byte *sourcePixels = (altSourceBuffer) ? altSourceBuffer : bufferData;
	Common::Rect dstLimitsRect(bufferWidth, bufferHeight);
	Common::Rect clippedDstRect = dstLimitsRect;

	if (optionalClippingRect) {
		if (!dstLimitsRect.intersects(*optionalClippingRect))
			return;
		dstLimitsRect.clip(*optionalClippingRect);
	} else {
		clippedDstRect = dstLimitsRect;
	}

	int w = READ_LE_UINT16(dataStream + kBptHeaderSize + 0);
	int h = READ_LE_UINT16(dataStream + kBptHeaderSize + 2);
	Common::Rect srcLimitsRect(w, h);
	Common::Rect clippedSrcRect = srcLimitsRect;
	Common::Rect dstOperation(x, y, x + clippedSrcRect.width(), y + clippedSrcRect.height());

	if (!clippedDstRect.intersects(dstOperation))
		return;

	clippedDstRect.clip(dstOperation);

	int subBlockCount = READ_LE_UINT16(dataStream + kBptHeaderSize + 4);
	byte *subBlockStream = dataStream + kBptHeaderSize + READ_LE_UINT32(dataStream + 4);
	int cx1 = clippedDstRect.left;
	int cy1 = clippedDstRect.top;
	int cx2 = clippedDstRect.right - 1;
	int cy2 = clippedDstRect.bottom - 1;

	for (int i = 0; i < subBlockCount; i++) {
		byte *blockData = subBlockStream;
		uint32 blockSize = READ_LE_UINT32(blockData); blockData += 4;
		subBlockStream += blockSize;
		int xOffset = READ_LE_UINT16(blockData); blockData += 2;
		int yOffset = READ_LE_UINT16(blockData); blockData += 2;
		int width = READ_LE_UINT16(blockData); blockData += 2;
		int height = READ_LE_UINT16(blockData); blockData += 2;
		int l_reach = READ_LE_UINT16(blockData); blockData += 2;
		int r_reach = READ_LE_UINT16(blockData); blockData += 2;
		int t_reach = READ_LE_UINT16(blockData); blockData += 2;
		int b_reach = READ_LE_UINT16(blockData); blockData += 2;
		int distortionPitch = ((width * 2 + 7) / 8); // 2 for 555

		if (width == 0 && height == 0)
			continue;

		Graphics::Surface dstBitmap;
		dstBitmap.init(bufferWidth, bufferHeight, bufferPitch, bufferData, Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

		Graphics::Surface srcBitmap;
		srcBitmap.init(bufferWidth, bufferHeight, bufferPitch, sourcePixels, Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

		Graphics::Surface distortionBitmap;
		distortionBitmap.init(width, height, distortionPitch, blockData, Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

		Common::Rect srcClipRect(cx1, cy1, cx2, cy2);
		Common::Rect dstClipRect(cx1, cy1, cx2, cy2);

		int src_x = (x + xOffset);
		int src_y = (y + yOffset);

		Common::Rect srcReach((src_x - l_reach), (src_y - t_reach), (src_x + r_reach), (src_y + b_reach));
		Common::Rect srcLimits(srcBitmap.w, srcBitmap.h);

		if (!srcLimits.intersects(srcClipRect))
			return;

		srcLimits.clip(srcClipRect);

		if (!srcReach.intersects(srcLimits))
			return;

		srcReach.clip(srcLimits);

		if (srcLimits.contains(srcReach)) {
			if (srcBitmap.pitch == 1280) {
				blitDistortionCore(&dstBitmap, src_x, src_y, &distortionBitmap, &dstClipRect, kSpecializedNotClipped, &srcBitmap, 0);
			} else {
				blitDistortionCore(&dstBitmap, src_x, src_y, &distortionBitmap, &dstClipRect, kNotClipped, &srcBitmap, 0);
			}
		} else {
			blitDistortionCore(&dstBitmap, src_x, src_y, &distortionBitmap, &dstClipRect, kReflectionClipped, &srcBitmap, &srcLimits);
		}
	}
}

}
