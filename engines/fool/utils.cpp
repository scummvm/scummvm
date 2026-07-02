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

#include "common/memstream.h"
#include "fool/fool.h"
#include "fool/toolbox.h"
#include "fool/utils.h"

namespace Fool {

Common::Rect readRect(Common::SeekableReadStream &stream) {
	Common::Rect result;
	result.top = stream.readSint16BE();
	result.left = stream.readSint16BE();
	result.bottom = stream.readSint16BE();
	result.right = stream.readSint16BE();
	return result;
}

Common::Point readPoint(Common::SeekableReadStream &stream) {
	Common::Point result;
	result.y = stream.readSint16BE();
	result.x = stream.readSint16BE();
	return result;
}

PolyHandle readPolygon(Common::SeekableReadStream &stream) {
	PolyHandle result(new Polygon);
	result->polySize = stream.readUint16BE();
	result->polyBBox = readRect(stream);
	for (int i = 10; i < result->polySize; i += 4) {
		result->polyPoints.push_back(readPoint(stream));
	}
	return result;
}

Region readRegion(Common::SeekableReadStream &stream) {
	Region region;
	region.rgnSize = stream.readUint16BE();
	if (debugChannelSet(8, kDebugGraphics)) {
		debugC(8, kDebugGraphics, "readRegion: region data");
		stream.hexdump(region.rgnSize - 2);
	}
	if (stream.size() - stream.pos() < region.rgnSize - 2) {
		warning("readRegion: not enough data to match size");
	}
	region.rgnBBox = readRect(stream);
	for (int i = 10; i < region.rgnSize; i+=2) {
		region.rgnData.push_back(stream.readSint16BE());
	}
	return region;
}

Pattern readPattern(Common::SeekableReadStream &stream) {
	Pattern result;
	for (int i = 0; i < 8; i++) {
		result.data[i] = stream.readByte();
	}
	return result;
}

PixMap readPixMap(Common::SeekableReadStream &stream, bool hasBaseAddr) {
	PixMap pixMap;
	if (hasBaseAddr) {
		pixMap.baseAddr = stream.readUint32BE();
	}
	uint16 rowBytes = stream.readUint16BE();
	pixMap.rowBytes = rowBytes & 0x3fff;
	pixMap._isBitMap = !(rowBytes & 0x8000);
	pixMap.bounds = readRect(stream);
	if (!pixMap._isBitMap) {
		pixMap.pmVersion = stream.readUint16BE();
		pixMap.packType = stream.readUint16BE();
		pixMap.packSize = stream.readUint32BE();
		pixMap.hRes = stream.readUint32BE();
		pixMap.vRes = stream.readUint32BE();
		pixMap.pixelType = stream.readUint16BE();
		pixMap.pixelSize = stream.readUint16BE();
		pixMap.cmpCount = stream.readUint16BE();
		pixMap.cmpSize = stream.readUint16BE();
		pixMap.planeBytes = stream.readUint32BE();
		pixMap.pmTable = stream.readUint32BE();
		pixMap.pmReserved = stream.readUint32BE();
	}
	return pixMap;
}

BitMap readBitsRectMono(Common::SeekableReadStream &stream, PixMap &pixMap, bool compressed) {
	BitMap result(new Graphics::ManagedSurface());
	result->create(pixMap.bounds.width(), pixMap.bounds.height(), Graphics::PixelFormat::createFormatCLUT8());

	Common::Rect outputRect = result->getBounds();

	if (!compressed) {
		Common::BitStream8MSB bs(stream);
		// rows are word-aligned
		int overflowBits = (pixMap.bounds.width() % 16 == 0) ? 0 : (16 - (pixMap.bounds.width() % 16));
		debugC(5, kDebugLevelGGraphics, "readBitsRectMono: width %d, overflow %d", pixMap.bounds.width(), overflowBits);

		for (int y = pixMap.bounds.top; y < pixMap.bounds.bottom; y++) {
			int yPos = y - pixMap.bounds.top;

			for (int x = pixMap.bounds.left; x < pixMap.bounds.right; x++) {
				int xPos = x - pixMap.bounds.left;

				uint bit = bs.getBit();
				if (outputRect.contains(xPos, yPos)) {
					result->setPixel(xPos, yPos,		bit);
				}
			}
			bs.skip(overflowBits);
		}

		return result;
	}

	for (int y = pixMap.bounds.top; y < pixMap.bounds.bottom; y++) {
		int yPos = y - pixMap.bounds.top;
		int x = 0;

		byte rowBytes = stream.readByte();
		byte readBytes = 0;

		while (readBytes < rowBytes) {
			byte rowBuf[128];
			byte bufLen;

			byte value = stream.readByte();
			readBytes++;

			if (value >= 128) {
				bufLen = (256 - value) + 1;
				byte repeatValue = stream.readByte();
				memset(rowBuf, repeatValue, bufLen);
				readBytes++;
			} else {
				bufLen = value + 1;
				stream.read(rowBuf, bufLen);
				readBytes += bufLen;
			}

			Common::MemoryReadStream ms(rowBuf, bufLen);
			Common::BitStream8MSB bs(ms);

			for (int i = 0; i < 8 * bufLen; i++) {
				int xPos = x;

				uint bit = bs.getBit();

				if (outputRect.contains(xPos, yPos)) {
					result->setPixel(xPos, yPos, bit);
				}

				x++;
			}
		}
	}
	return result;
}
Common::Rect blitMono(const BitMap &src, BitMap &dst, const BitMap &mask, const Common::Point &dstPos, SourceMode mode) {
	Common::Rect dstRect = src->getBounds();
	dstRect.moveTo(dstPos);
	dstRect.clip(dst->getBounds());
	Common::Rect srcRect = dstRect;
	srcRect.translate(-dstPos.x, -dstPos.y);

	uint32 black = g_engine->_wm._colorBlack;
	uint32 white = g_engine->_wm._colorWhite;
	if (mode == kSrcCopy && !mask) {
		// fast blit
		dst->blitFrom(*src, srcRect, Common::Point(srcRect.left + dstPos.x, srcRect.top + dstPos.y));
	} else {
		// per-pixel blit
		for (int y = srcRect.top; y < srcRect.bottom; y++) {
			byte *source = (byte *)src->getBasePtr(srcRect.left, y);
			byte *target = (byte *)dst->getBasePtr(srcRect.left + dstPos.x, dstPos.y + y);
			byte *maskSource = mask ? (byte *)mask->getBasePtr(srcRect.left, y) : nullptr;
			for (int x = srcRect.left; x < srcRect.right; x++) {
				if (mask && !*maskSource) {
					source++;
					target++;
					maskSource++;
					continue;
				}
				switch (mode) {
				case kSrcCopy:
					*target = (*source == black) ? black : white;
					break;
				case kSrcOr:
					*target = (*target == black) || (*source == black) ? black : white;
					break;
				case kSrcXor:
					*target = (*target == black) ^ (*source == black) ? black : white;
					break;
				case kSrcBic:
					*target = (*target == black) && (*source != black) ? black : white;
					break;
				case kNotSrcCopy:
					*target = (*source != black) ? black : white;
					break;
				case kNotSrcOr:
					*target = (*target == black) || (*source != black) ? black : white;
					break;
				case kNotSrcXor:
					*target = (*target == black) ^ (*source != black) ? black : white;
					break;
				case kNotSrcBic:
					*target = (*target == black) && (*source == black) ? black : white;
					break;
				default:
					break;
				}
				source++;
				target++;
				if (mask)
					maskSource++;
			}
		}
	}
	return dstRect;
}

Common::Rect blitMono(const BitMap &src, BitMap &dst, const BitMap &mask, const Common::Point &dstPos, PatternMode mode) {
	if ((mode >= 8) && (mode < 16)) {
		// If the mode is one of the source transfer modes (or negative), no drawing is performed.
		// Source: Inside Macintosh I-170
		return blitMono(src, dst, mask, dstPos, (SourceMode)((int)mode & 0x7));
	}
	warning("blitMono: Called with an invalid PatternMode, ignoring");
	return Common::Rect();
}

Graphics::ManagedSurface *createRemappedSurface(const Graphics::Surface *surface, const byte *palette, uint colorCount) {
	Graphics::ManagedSurface *s = new Graphics::ManagedSurface();
	s->create(surface->w, surface->h, Graphics::PixelFormat::createFormatCLUT8());

	byte paletteMap[256];
	memset(paletteMap, 0, sizeof(paletteMap));

	const byte monoPalette[] = {
		0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00
	};

	if (colorCount == 0) {
		colorCount = 2;
		palette = monoPalette;
	}

	for (uint i = 0; i < colorCount; i++) {
		int r = palette[3 * i];
		int g = palette[3 * i + 1];
		int b = palette[3 * i + 2];

		uint32 c;

		c = g_engine->_wm.findBestColor(r, g, b);
		paletteMap[i] = c;
	}

	// Colors outside the palette are not remapped.

	for (uint i = colorCount; i < 256; i++)
		paletteMap[i] = i;

	if (palette) {
		for (int y = 0; y < s->h; y++) {
			for (int x = 0; x < s->w; x++) {
				uint color = surface->getPixel(x, y);
				if (color > colorCount)
					color = g_engine->_wm._colorBlack;
				else
					color = paletteMap[color];

				s->setPixel(x, y, color);
			}
		}
	} else {
		s->copyFrom(*surface);
	}

	return s;
}



} // End of namespace Fool
