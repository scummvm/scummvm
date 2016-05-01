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

namespace Scumm {

Moonbase::Moonbase() {
	_fowSentinelImage = -1;
	_fowSentinelState = -1;
	_fowSentinelConditionBits = 0;
}

Moonbase::~Moonbase() {
}

void Moonbase::renderFOW() {
	warning("STUB: renderFOW()");
}

void Moonbase::blitT14WizImage(uint8 *dst, int dstw, int dsth, int dstPitch, const Common::Rect *clipBox,
		 uint8 *wizd, int srcx, int srcy, int rawROP, int paramROP) {
	Common::Rect rScreen(dstw, dsth);
	if (clipBox) {
		Common::Rect clip(clipBox->left, clipBox->top, clipBox->right, clipBox->bottom);
		if (rScreen.intersects(clip)) {
			rScreen.clip(clip);
		} else {
			return;
		}
	}

	int width = READ_LE_UINT16(wizd + 0x8 + 0);
	int height = READ_LE_UINT16(wizd + 0x8 + 2);

	int headerSize = READ_LE_UINT32(wizd + 0x4);
	uint8 *dataPointer = &wizd[0x8 + headerSize];

	warning("w: %d, h: %d, size: %d", width, height, headerSize);

	for (int i = 0; i < height; i++) {
		uint16 lineSize      = READ_LE_UINT16(dataPointer + 0);
		uint8 *singlesOffset = dataPointer + READ_LE_UINT16(dataPointer + 2);
		uint8 *quadsOffset   = dataPointer + READ_LE_UINT16(dataPointer + 4);

		int linecount = width;
		byte *dst1 = dst;

		while (1) {
			int code = *dataPointer - 2;

			if (code == 0) { // quad
				for (int c = 0; c < 4; c++) {
					WRITE_LE_UINT16(dst1, READ_LE_UINT16(quadsOffset));
					singlesOffset += 2;
					dst1 += 2;
					linecount--;
				}
			} else if (code < 0) { // single
				WRITE_LE_UINT16(dst1, READ_LE_UINT16(singlesOffset));
				singlesOffset += 2;
				dst1 += 2;
				linecount--;
			} else { // skip
				if ((code & 1) == 0) {
					code >>= 1;
					dst1 += code * 2;
					linecount -= code;
				} else { // special case
					uint16 color = READ_LE_UINT16(dataPointer);
					WRITE_LE_UINT16(dst, color);
					singlesOffset += 2;
					dst1 += 2;
				}
			}

			if (linecount <= 0)
				break;
		}

		dataPointer += lineSize;
		dst += dstPitch;
	}
}

} // End of namespace Scumm
