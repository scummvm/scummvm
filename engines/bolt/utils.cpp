/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bolt/bolt.h"

#include "common/config-manager.h"

namespace Bolt {

void BoltEngine::displayColors(byte *palette, int16 page, int16 flags) {
	byte localPalette[384];
	int16 startIndex = 0, endIndex = 0;

	if (!palette)
		return;

	if (flags == 0) {
		// Indices 0-127
		startIndex = (int16)READ_UINT16(palette + 2);
		endIndex = MIN<int16>((int16)READ_UINT16(palette + 4), 127);
	} else if (flags == 1) {
		// Indices 128-255
		startIndex = MAX<int16>((int16)READ_UINT16(palette + 2), 128);
		endIndex = (int16)READ_UINT16(palette + 4);
	}

	// Copy RGB triplets from palette resource...
	byte *src = palette + startIndex * 3 + 6;
	byte *dst = localPalette;

	for (int16 i = startIndex; i <= endIndex; i++) {
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
	}

	XPPicDesc picDesc;

	picDesc.pixelData = nullptr;
	picDesc.paletteStart = (flags != 0) ? startIndex - 128 : startIndex;
	picDesc.paletteCount = endIndex - startIndex + 1;
	picDesc.palette = localPalette;

	_xp->displayPic(&picDesc, 0, 0, page);
}

byte BoltEngine::getPixel(byte *sprite, int16 localX, int16 localY) {
	int16 sprH = READ_UINT16(sprite + 0x0A);
	byte *pixels = getResolvedPtr(sprite, 0x12);
	return pixels[localX * sprH + localY];
}

void BoltEngine::boltPict2Pict(XPPicDesc *dest, byte *boltSprite) {
	dest->pixelData = getResolvedPtr(boltSprite, 0x12);
	dest->width = (int16)READ_UINT16(boltSprite + 0x0A);
	dest->height = (int16)READ_UINT16(boltSprite + 0x0C);
	dest->palette = nullptr;
	dest->paletteStart = 0;
	dest->paletteCount = 0;
	dest->flags = 0;

	if (boltSprite[1] & 0x01)
		dest->flags |= 1;

	if (boltSprite[0] & 0x02)
		dest->flags |= 2;
}

void BoltEngine::displayPic(byte *boltSprite, int16 xOff, int16 yOff, int16 page) {
	XPPicDesc localDesc;

	if (!boltSprite)
		return;

	boltPict2Pict(&localDesc, boltSprite);

	_xp->displayPic(&localDesc,
		(int16)READ_UINT16(boltSprite + 0x06) + xOff,
		(int16)READ_UINT16(boltSprite + 0x08) + yOff,
		page);
}

bool BoltEngine::pointInRect(Common::Rect *rect, int16 x, int16 y) {
	// Slightly different from our rect.contains() method...
	return rect->left <= x && x <= rect->right && rect->top <= y && y <= rect->bottom;
}

const char *BoltEngine::assetPath(const char *fileName) {
	if (Common::File::exists(fileName)) {
		return fileName;
	} else if (Common::File::exists(Common::Path(Common::String::format("ASSETS/%s", fileName)))) {
		return Common::Path(Common::String::format("ASSETS/%s", fileName)).toString().c_str();
	} else {
		error("BoltEngine::assetPath(): Invalid game path");
	}
}

void BoltEngine::boltCycleToXPCycle(byte *srcData, XPCycleState *cycleDesc) {
	int16 cycleIdx = 0;

	for (int16 i = 0; i < 4 && cycleIdx < 4; i++) {
		int16 type = (int16)READ_UINT16(srcData + i * 2);
		if (type != 1)
			continue;

		byte *cycleData = getResolvedPtr(srcData, 8 + i * 4);
		if (!cycleData)
			continue;

		if (cycleData[5] == 0) {
			cycleDesc[cycleIdx].startIndex = (int16)READ_UINT16(cycleData);
			cycleDesc[cycleIdx].endIndex = (int16)READ_UINT16(cycleData + 2);
		} else {
			cycleDesc[cycleIdx].startIndex = (int16)READ_UINT16(cycleData + 2);
			cycleDesc[cycleIdx].endIndex = (int16)READ_UINT16(cycleData);
		}

		cycleDesc[cycleIdx].delay = (int16)(cycleData[4] * (int32)1000 / 60);
		cycleIdx++;
	}

	while (cycleIdx < 4) {
		cycleDesc[cycleIdx].startIndex = 0;
		cycleDesc[cycleIdx].endIndex = 0;
		cycleDesc[cycleIdx].delay = 0;
		cycleIdx++;
	}
}

void BoltEngine::unpackColors(int16 count, byte *packedColors) {
	byte *src = packedColors;
	byte *dst = packedColors;

	while (count-- > 0) {
		dst[0] = src[1];
		dst[1] = src[2];
		dst[2] = src[3];
		dst += 3;
		src += 4;
	}
}

} // End of namespace Bolt
