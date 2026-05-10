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

#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/path.h"
#include "common/rect.h"

#include "alg/alg.h"
#include "alg/graphics.h"

namespace Alg {

Graphics::Surface *AlgGraphics::loadVgaBackground(const Common::Path &path, uint8 *palette) {
	Common::File vgaFile;
	if (!vgaFile.open(path)) {
		error("AlgGraphics::loadVgaBackground(): Can't open background file '%s'", path.toString().c_str());
	}
	uint16 width = vgaFile.readUint16LE();
	uint16 height = vgaFile.readUint16LE();
	uint8 paletteEntries = vgaFile.readByte();
	uint8 paletteStart = vgaFile.readByte();
	assert(width >= 317 && width <= 320);
	assert(height == 200);
	assert(paletteStart == 0x10);
	for (uint32 i = paletteStart * 3; i < (paletteStart + paletteEntries) * 3U; i += 3) {
		palette[i] = vgaFile.readByte();
		palette[i + 1] = vgaFile.readByte();
		palette[i + 2] = vgaFile.readByte();
	}
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	vgaFile.read(surface->getPixels(), width * height);
	Common::Rect backgroundRect = Common::Rect(0, 0, width, height);
	surface->flipVertical(backgroundRect);
	vgaFile.close();
	return surface;
}

// for "normal" ani images
Common::Array<Graphics::Surface *> *AlgGraphics::loadAniImage(const Common::Path &path, uint8 *palette) {
	Common::Array<Graphics::Surface *> *images = new Common::Array<Graphics::Surface *>();
	Common::File aniFile;
	if (!aniFile.open(path)) {
		error("AlgGraphics::loadAniImage(): Can't open image file '%s'", path.toString().c_str());
	}
	uint8 paletteEntries = aniFile.readByte();
	uint8 paletteStart = aniFile.readByte();
	for (uint32 i = paletteStart * 3; i < (paletteStart + paletteEntries) * 3U; i += 3) {
		palette[i] = aniFile.readByte();
		palette[i + 1] = aniFile.readByte();
		palette[i + 2] = aniFile.readByte();
	}
	uint16 length, width, height;
	while (aniFile.pos() < aniFile.size()) {
		width = height = 0;
		uint64 aniSectionOffset = aniFile.pos();
		while (aniFile.pos() < aniFile.size()) {
			length = aniFile.readUint16LE();
			if (length == 0) {
				break;
			}
			width = length;
			height++;
			aniFile.skip(2 + length);
		}
		if (width > 0) {
			aniFile.seek(aniSectionOffset, SEEK_SET);
			Graphics::Surface *aniImage = new Graphics::Surface();
			aniImage->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
			for (uint16 y = 0; y < height; y++) {
				aniFile.skip(4);
				aniFile.read(aniImage->getBasePtr(0, y), width);
			}
			images->push_back(aniImage);
		}
	}
	aniFile.close();
	return images;
}

// for ani images that use relative positioning.
// because these are meant to be drawn directly onto a 320x200 screen, they use relative offsets assuming that resolution.
// as we don't always want to draw directly to screen, we draw to the center of a virtual screen and then copy from a centered subrect.
Common::Array<Graphics::Surface *> *AlgGraphics::loadScreenCoordAniImage(const Common::Path &path, uint8 *palette) {
	Common::Array<Graphics::Surface *> *images = new Common::Array<Graphics::Surface *>();
	Common::File aniFile;
	if (!aniFile.open(path)) {
		error("AlgGraphics::loadScreenCoordAniImage(): Can't open image file '%s'", path.toString().c_str());
	}
	uint8 paletteEntries = aniFile.readByte();
	uint8 paletteStart = aniFile.readByte();
	for (uint32 i = paletteStart * 3; i < (paletteStart + paletteEntries) * 3U; i += 3) {
		palette[i] = aniFile.readByte();
		palette[i + 1] = aniFile.readByte();
		palette[i + 2] = aniFile.readByte();
	}
	uint16 length = 0;
	int16 offset = 0;
	uint32 dest = 0;
	uint32 x = 0, y = 0;
	while (aniFile.pos() < aniFile.size()) {
		Graphics::Surface *renderTarget = new Graphics::Surface();
		renderTarget->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
		uint32 centerOffset = (renderTarget->w * renderTarget->h / 2) + (renderTarget->w / 2);
		while (true) {
			length = aniFile.readUint16LE();
			if (length == 0) {
				break;
			}
			offset = aniFile.readSint16LE();
			dest = centerOffset + offset;
			y = dest / renderTarget->w;
			x = dest - (y * renderTarget->w);
			aniFile.read(renderTarget->getBasePtr(x, y), length);
		}
		Graphics::Surface *aniImage = new Graphics::Surface();
		aniImage->create(96, 96, Graphics::PixelFormat::createFormatCLUT8());
		Common::Rect subSectionRect;
		subSectionRect.left = (renderTarget->w / 2) - (aniImage->w / 2);
		subSectionRect.top = (renderTarget->h / 2) - (aniImage->h / 2);
		subSectionRect.right = (renderTarget->w / 2) + (aniImage->w / 2);
		subSectionRect.bottom = (renderTarget->h / 2) + (aniImage->h / 2);
		aniImage->copyRectToSurface(*renderTarget, 0, 0, subSectionRect);
		images->push_back(aniImage);
		renderTarget->free();
		delete renderTarget;
	}
	aniFile.close();
	return images;
}

void AlgGraphics::drawImage(Graphics::Surface *dst, Graphics::Surface *src, int32 x, int32 y) {
	int32 dstX = x;
	int32 dstY = y;
	Common::Rect subRect = Common::Rect(0, 0, src->w, src->h);
	if (dstX < 0) {
		subRect.left -= dstX;
		dstX = 0;
	}
	if (dstY < 0) {
		subRect.top -= dstY;
		dstY = 0;
	}
	if (dstX + src->w > dst->w) {
		subRect.right -= dstX + src->w - dst->w;
	}
	if (dstY + src->h > dst->h) {
		subRect.bottom -= dstY + src->h - dst->h;
	}
	dst->copyRectToSurfaceWithKey(*src, dstX, dstY, subRect, 0x00);
}

void AlgGraphics::drawImageCentered(Graphics::Surface *dst, Graphics::Surface *src, int32 x, int32 y) {
	int32 dstX = x - (src->w / 2);
	int32 dstY = y - (src->h / 2);
	drawImage(dst, src, dstX, dstY);
}

} // End of namespace Alg
