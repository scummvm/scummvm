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

#include "graphics/fonts/bdf.h"

#include "common/file.h"
#include "common/endian.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

#define DRAWDEBUG 0

namespace Graphics {

BdfFont::BdfFont(const BdfFontData &data, DisposeAfterUse::Flag dispose)
	: _data(data), _dispose(dispose) {
}

BdfFont::~BdfFont() {
	if (_dispose == DisposeAfterUse::YES) {
		for (int i = 0; i < _data.numCharacters; ++i)
			delete[] _data.bitmaps[i];
		delete[] _data.bitmaps;
		delete[] _data.advances;
		delete[] _data.boxes;
		delete[] _data.familyName;
		delete[] _data.slant;
	}
}

const char *BdfFont::getFamilyName() const {
	return _data.familyName;
}

const char *BdfFont::getFontSlant() const {
	return _data.slant;
}

int BdfFont::getFontHeight() const {
	return _data.height;
}

int BdfFont::getFontAscent() const {
	return _data.ascent;
}

int BdfFont::getFontSize() const {
	return _data.size;
}

int BdfFont::getMaxCharWidth() const {
	return _data.maxAdvance;
}

int BdfFont::getCharWidth(uint32 chr) const {
	// In case all font have the same advance value, we use the maximum.
	if (!_data.advances)
		return _data.maxAdvance;

	const int ch = mapToIndex(chr);
	// In case no mapping exists, we use the maximum advance.
	if (ch < 0)
		return _data.maxAdvance;
	else
		return _data.advances[ch];
}


template<typename PixelType>
void drawCharIntern(byte *ptr, uint pitch, const byte *src, int h, int width, int minX, int maxX, const PixelType color) {
	byte data = 0;
	while (h--) {
		PixelType *dst = (PixelType *)ptr;

		for (int x = 0; x < width; ++x) {
			if (!(x % 8))
				data = *src++;

			if (x >= minX && x <= maxX && (data & 0x80))
				dst[x] = color;

			data <<= 1;
		}

		ptr += pitch;
	}
}

int BdfFont::mapToIndex(uint32 ch) const {
	// Check whether the character is included
	if (_data.firstCharacter <= (int)ch && (int)ch <= _data.firstCharacter + _data.numCharacters) {
		if (_data.bitmaps[ch - _data.firstCharacter])
			return ch - _data.firstCharacter;
	}

	return _data.defaultCharacter - _data.firstCharacter;
}

void BdfFont::drawChar(Surface *dst, uint32 chr, const int tx, const int ty, const uint32 color) const {
	assert(dst != 0);

	// TODO: Where is the relation between the max advance being smaller or
	// equal to 50 and the decision of the theme designer?
	// asserting _data.maxAdvance <= 50: let the theme designer decide what looks best
	//assert(_data.maxAdvance <= 50);
	assert(dst->format.bytesPerPixel == 1 || dst->format.bytesPerPixel == 2 || dst->format.bytesPerPixel == 4);

	const int idx = mapToIndex(chr);
	if (idx < 0)
		return;

	int width, height, xOffset, yOffset;

	// Get the bounding box of the character
	if (!_data.boxes) {
		width = _data.defaultBox.width;
		height = _data.defaultBox.height;
		xOffset = _data.defaultBox.xOffset;
		yOffset = _data.defaultBox.yOffset;
	} else {
		width = _data.boxes[idx].width;
		height = _data.boxes[idx].height;
		xOffset = _data.boxes[idx].xOffset;
		yOffset = _data.boxes[idx].yOffset;
	}

	int y = ty + _data.ascent - yOffset - height;
	int x = tx + xOffset;

	const byte *src = _data.bitmaps[idx];

	const int bytesPerRow = (width + 7) / 8;
	const int originalWidth = width;

	// Make sure we do not draw outside the surface
	if (y < 0) {
		src -= y * bytesPerRow;
		height += y;
		y = 0;
	}

	if (y + height > dst->h)
		height = dst->h - y;

	if (height <= 0)
		return;

	int xStart = 0;
	if (x < 0) {
		xStart = -x;
		width += x;
		x = 0;
	}

	if (x + width > dst->w)
		width = dst->w - x;

	if (width <= 0)
		return;

	const int xEnd = xStart + width - 1;

	byte *ptr = (byte *)dst->getBasePtr(x, y);

	if (dst->format.bytesPerPixel == 1)
		drawCharIntern<byte>(ptr, dst->pitch, src, height, originalWidth, xStart, xEnd, color);
	else if (dst->format.bytesPerPixel == 2)
		drawCharIntern<uint16>(ptr, dst->pitch, src, height, originalWidth, xStart, xEnd, color);
	else if (dst->format.bytesPerPixel == 4)
		drawCharIntern<uint32>(ptr, dst->pitch, src, height, originalWidth, xStart, xEnd, color);
}

namespace {

inline byte hexToInt(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return 0;
}

byte *loadCharacter(Common::SeekableReadStream &stream, int &encoding, int &advance, BdfBoundingBox &box) {
	Common::String line;
	byte *bitmap = 0;

	while (true) {
		line = stream.readLine();
		line.trim(); 	// BDF files created from unifont tools (make hex)
						// have a rogue space character after the "BITMAP" label

		if (stream.err() || stream.eos()) {
			warning("BdfFont::loadCharacter: Premature end of file");
			delete[] bitmap;
			return 0;
		}

		if (line.hasPrefix("ENCODING ")) {
			if (sscanf(line.c_str(), "ENCODING %d", &encoding) != 1) {
				warning("BdfFont::loadCharacter: Invalid ENCODING");
				delete[] bitmap;
				return 0;
			}
		} else if (line.hasPrefix("DWIDTH ")) {
			int yAdvance;
			if (sscanf(line.c_str(), "DWIDTH %d %d", &advance, &yAdvance) != 2) {
				warning("BdfFont::loadCharacter: Invalid DWIDTH");
				delete[] bitmap;
				return 0;
			}

			if (yAdvance != 0) {
				warning("BdfFont::loadCharacter: Character %d has an y advance of %d", encoding, yAdvance);
				delete[] bitmap;
				return 0;
			}

			if (advance < 0) {
				warning("BdfFont::loadCharacter: Character %d has an x advance of %d", encoding, advance);
				delete[] bitmap;
				return 0;
			}
		} else if (line.hasPrefix("BBX ")) {
			int width, height, xOffset, yOffset;
			if (sscanf(line.c_str(), "BBX %d %d %d %d",
			           &width, &height, &xOffset, &yOffset) != 4) {
				warning("BdfFont::loadCharacter: Invalid BBX");
				delete[] bitmap;
				return 0;
			}

			box.width = width;
			box.height = height;
			box.xOffset = xOffset;
			box.yOffset = yOffset;
		} else if (line == "BITMAP") {
			const uint bytesPerRow = (box.width + 7) / 8;

			if (bitmap) {
				warning("Bdf::loadCharacter(): Double BITMAP definitions");
				delete[] bitmap;
			}

			byte *dst = bitmap = new byte[box.height * bytesPerRow];

			for (int y = 0; y < box.height; ++y) {
				line = stream.readLine();
				if (stream.err() || stream.eos()) {
					warning("BdfFont::loadCharacter: Premature end of file");
					delete[] bitmap;
					return 0;
				}

				if (line.size() != 2 * bytesPerRow) {
					warning("BdfFont::loadCharacter: Pixel line has wrong size");
					delete[] bitmap;
					return 0;
				}

				for (uint x = 0; x < bytesPerRow; ++x) {
					char nibble1 = line[x * 2 + 0];
					char nibble2 = line[x * 2 + 1];
					*dst++ = (hexToInt(nibble1) << 4) | hexToInt(nibble2);
				}
			}
		} else if (line == "ENDCHAR") {
			return bitmap;
		}
	}
}

void freeBitmaps(byte **bitmaps, int size) {
	for (int i = 0; i < size; ++i)
		delete[] bitmaps[i];
}

} // End of anonymous namespace

BdfFont *BdfFont::loadFont(Common::SeekableReadStream &stream) {
	BdfFontData font;
	memset(&font, 0, sizeof(font));
	font.ascent = -1;
	font.defaultCharacter = -1;

	// We only load the first 256 characters
	font.numCharacters = 256;
	byte **bitmaps = new byte *[font.numCharacters];
	memset(bitmaps, 0, sizeof(byte *) * font.numCharacters);
	byte *advances = new byte[font.numCharacters];
	BdfBoundingBox *boxes = new BdfBoundingBox[font.numCharacters];
	char *familyName = nullptr;
	char *slant = nullptr;

	int descent = -1;

	Common::String line;
	while (true) {
		line = stream.readLine();
		if (stream.err() || stream.eos()) {
			warning("BdfFont::loadFont: Premature end of file");
			freeBitmaps(bitmaps, font.numCharacters);
			delete[] bitmaps;
			delete[] advances;
			delete[] boxes;
			delete[] familyName;
			delete[] slant;
			return 0;
		}

		// Only parse and handle declarations we actually need
		if (line.hasPrefix("FONTBOUNDINGBOX ")) {
			int width, height, xOffset, yOffset;
			if (sscanf(line.c_str(), "FONTBOUNDINGBOX %d %d %d %d",
			           &width, &height, &xOffset, &yOffset) != 4) {
				warning("BdfFont::loadFont: Invalid FONTBOUNDINGBOX");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				delete[] familyName;
				delete[] slant;
				return 0;
			}

			font.defaultBox.width = width;
			font.defaultBox.height = height;
			font.defaultBox.xOffset = xOffset;
			font.defaultBox.yOffset = yOffset;
		} else if (line.hasPrefix("PIXEL_SIZE ")) {
			if (sscanf(line.c_str(), "PIXEL_SIZE %d", &font.size) != 1) {
				warning("BdfFont::loadFont: Invalid PIXEL_SIZE");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				delete[] familyName;
				delete[] slant;
				return 0;
			}
		} else if (line.hasPrefix("FONT_ASCENT ")) {
			if (sscanf(line.c_str(), "FONT_ASCENT %d", &font.ascent) != 1) {
				warning("BdfFont::loadFont: Invalid FONT_ASCENT");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				delete[] familyName;
				delete[] slant;
				return 0;
			}
		} else if (line.hasPrefix("FONT_DESCENT ")) {
			if (sscanf(line.c_str(), "FONT_DESCENT %d", &descent) != 1) {
				warning("BdfFont::loadFont: Invalid FONT_DESCENT");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				delete[] familyName;
				delete[] slant;
				return 0;
			}
		} else if (line.hasPrefix("DEFAULT_CHAR ")) {
			if (sscanf(line.c_str(), "DEFAULT_CHAR %d", &font.defaultCharacter) != 1) {
				warning("BdfFont::loadFont: Invalid DEFAULT_CHAR");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				delete[] familyName;
				delete[] slant;
				return 0;
			}
		} else if (line.hasPrefix("STARTCHAR ")) {
			BdfBoundingBox box = font.defaultBox;
			int encoding = -1;
			int advance = -1;
			byte *bitmap = loadCharacter(stream, encoding, advance, box);

			// Ignore all characters above 255.
			if (encoding < 0 || encoding >= font.numCharacters) {
				delete[] bitmap;
				continue;
			}

			// Calculate the max advance
			if (advance > font.maxAdvance)
				font.maxAdvance = advance;

			if (!bitmap) {
				warning("BdfFont::loadFont: Character %d invalid", encoding);
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				delete[] familyName;
				delete[] slant;
				return 0;
			}

			bitmaps[encoding] = bitmap;
			advances[encoding] = advance;
			boxes[encoding] = box;
		} else if (line.hasPrefix("FAMILY_NAME \"")) {
			if (familyName != nullptr) {
				warning("BdfFont::loadFont: Duplicated FAMILY_NAME");
				delete[] familyName;
			}
			familyName = new char[line.size()];
			Common::strlcpy(familyName, line.c_str() + 13, line.size() - 12);	// strlcpy() copies at most size-1 characters and then add a '\0'
			char *p = &familyName[strlen(familyName)];
			while (p != familyName && *p != '"')
				p--;
			if (p == familyName) {
				warning("BdfFont::loadFont: Invalid FAMILY_NAME");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				delete[] familyName;
				delete[] slant;
				return 0;
			}
			*p = '\0'; // Remove last quote
		} else if (line.hasPrefix("SLANT \"")) {
			if (slant != nullptr) {
				warning("BdfFont::loadFont: Duplicated SLANT");
				delete[] slant;
			}
			slant = new char[line.size()];
			Common::strlcpy(slant, line.c_str() + 7, line.size() - 6);  // strlcpy() copies at most size-1 characters and then add a '\0'
			char *p = &slant[strlen(slant)];
			while (p != slant && *p != '"')
				p--;
			if (p == slant) {
				warning("BdfFont::loadFont: Invalid SLANT");
				freeBitmaps(bitmaps, font.numCharacters);
				delete[] bitmaps;
				delete[] advances;
				delete[] boxes;
				delete[] familyName;
				delete[] slant;
				return 0;
			}
			*p = '\0'; // Remove last quote
		} else if (line == "ENDFONT") {
			break;
		}
	}

	if (font.ascent < 0 || descent < 0) {
		warning("BdfFont::loadFont: Invalid ascent or descent");
		freeBitmaps(bitmaps, font.numCharacters);
		delete[] bitmaps;
		delete[] advances;
		delete[] boxes;
		delete[] familyName;
		delete[] slant;
		return 0;
	}

	font.height = font.ascent + descent;

	font.bitmaps = bitmaps;
	font.advances = advances;
	font.boxes = boxes;
	font.familyName = familyName;
	font.slant = slant;

	int firstCharacter = font.numCharacters;
	int lastCharacter = -1;
	bool hasFixedBBox = true;
	bool hasFixedAdvance = true;

	for (int i = 0; i < font.numCharacters; ++i) {
		if (!font.bitmaps[i])
			continue;

		if (i < firstCharacter)
			firstCharacter = i;

		if (i > lastCharacter)
			lastCharacter = i;

		if (font.advances[i] != font.maxAdvance)
			hasFixedAdvance = false;

		const BdfBoundingBox &bbox = font.boxes[i];
		if (bbox.width != font.defaultBox.width
		    || bbox.height != font.defaultBox.height
		    || bbox.xOffset != font.defaultBox.xOffset
		    || bbox.yOffset != font.defaultBox.yOffset)
			hasFixedBBox = false;
	}

	if (lastCharacter == -1) {
		warning("BdfFont::loadFont: No glyphs found");
		delete[] font.bitmaps;
		delete[] font.advances;
		delete[] font.boxes;
		delete[] familyName;
		delete[] slant;
		return 0;
	}

	// Free the advance table, in case all glyphs use the same advance
	if (hasFixedAdvance) {
		delete[] font.advances;
		font.advances = 0;
	}

	// Free the box table, in case all glyphs use the same box
	if (hasFixedBBox) {
		delete[] font.boxes;
		font.boxes = 0;
	}

	// Adapt for the fact that we never use encoding 0.
	if (font.defaultCharacter < firstCharacter
	    || font.defaultCharacter > lastCharacter)
		font.defaultCharacter = -1;

	font.firstCharacter = firstCharacter;

	const int charsAvailable = lastCharacter - firstCharacter + 1;
	// Try to compact the tables
	if (charsAvailable < font.numCharacters) {
		byte **newBitmaps = new byte *[charsAvailable];
		boxes = 0;
		advances = 0;
		if (!hasFixedBBox)
			boxes = new BdfBoundingBox[charsAvailable];
		if (!hasFixedAdvance)
			advances = new byte[charsAvailable];

		for (int i = 0; i < charsAvailable; ++i) {
			const int encoding = i + firstCharacter;
			if (font.bitmaps[encoding]) {
				newBitmaps[i] = bitmaps[encoding];

				if (!hasFixedBBox)
					boxes[i] = font.boxes[encoding];
				if (!hasFixedAdvance)
					advances[i] = font.advances[encoding];
			} else {
				newBitmaps[i] = 0;
			}
		}

		delete[] font.bitmaps;
		font.bitmaps = newBitmaps;
		delete[] font.advances;
		font.advances = advances;
		delete[] font.boxes;
		font.boxes = boxes;

		font.numCharacters = charsAvailable;
	}

	return new BdfFont(font, DisposeAfterUse::YES);
}

#define BDF_FONTCACHE_TAG MKTAG('S', 'V', 'F', 'C')
#define BDF_FONTCACHE_VERSION 1

bool BdfFont::cacheFontData(const BdfFont &font, const Common::String &filename) {
	Common::DumpFile cacheFile;
	if (!cacheFile.open(filename)) {
		warning("BdfFont::cacheFontData: Couldn't open file '%s' for writing", filename.c_str());
		return false;
	}

	const BdfFontData &data = font._data;

	cacheFile.writeUint32BE(BDF_FONTCACHE_TAG);
	cacheFile.writeUint32BE(BDF_FONTCACHE_VERSION);
	cacheFile.writeUint16BE(data.maxAdvance);
	cacheFile.writeByte(data.height);
	cacheFile.writeByte(data.defaultBox.width);
	cacheFile.writeByte(data.defaultBox.height);
	cacheFile.writeSByte(data.defaultBox.xOffset);
	cacheFile.writeSByte(data.defaultBox.yOffset);
	cacheFile.writeByte(data.ascent);
	cacheFile.writeUint16BE(data.firstCharacter);
	cacheFile.writeSint16BE(data.defaultCharacter);
	cacheFile.writeUint16BE(data.numCharacters);

	for (int i = 0; i < data.numCharacters; ++i) {
		const BdfBoundingBox &box = data.boxes ? data.boxes[i] : data.defaultBox;
		if (data.bitmaps[i]) {
			const int bytes = ((box.width + 7) / 8) * box.height;
			cacheFile.writeUint32BE(bytes);
			cacheFile.write(data.bitmaps[i], bytes);
		} else {
			cacheFile.writeUint32BE(0);
		}
	}

	if (data.advances) {
		cacheFile.writeByte(0xFF);
		cacheFile.write(data.advances, data.numCharacters);
	} else {
		cacheFile.writeByte(0x00);
	}

	if (data.boxes) {
		cacheFile.writeByte(0xFF);

		for (int i = 0; i < data.numCharacters; ++i) {
			const BdfBoundingBox &box = data.boxes[i];
			cacheFile.writeByte(box.width);
			cacheFile.writeByte(box.height);
			cacheFile.writeSByte(box.xOffset);
			cacheFile.writeSByte(box.yOffset);
		}
	} else {
		cacheFile.writeByte(0x00);
	}

	return !cacheFile.err();
}

BdfFont *BdfFont::loadFromCache(Common::SeekableReadStream &stream) {
	const uint32 magic = stream.readUint32BE();
	if (magic != BDF_FONTCACHE_TAG)
		return nullptr;

	const uint32 version = stream.readUint32BE();
	if (version != BDF_FONTCACHE_VERSION)
		return nullptr;

	BdfFontData data;

	data.maxAdvance = stream.readUint16BE();
	data.height = stream.readByte();
	data.defaultBox.width = stream.readByte();
	data.defaultBox.height = stream.readByte();
	data.defaultBox.xOffset = stream.readSByte();
	data.defaultBox.yOffset = stream.readSByte();
	data.ascent = stream.readByte();
	data.firstCharacter = stream.readUint16BE();
	data.defaultCharacter = stream.readSint16BE();
	data.numCharacters = stream.readUint16BE();

	if (stream.err() || stream.eos())
		return nullptr;

	if (data.numCharacters == 0) {
		warning("BdfFont::loadFromCache(): Requested to load 0 characters font");
		return nullptr;
	}

	byte **bitmaps = new byte *[data.numCharacters];
	byte *advances = 0;
	BdfBoundingBox *boxes = 0;
	for (int i = 0; i < data.numCharacters; ++i) {
		uint32 size = stream.readUint32BE();

		if (stream.err() || stream.eos()) {
			for (int j = 0; j < i; ++j)
				delete[] bitmaps[i];
			delete[] bitmaps;
			return nullptr;
		}

		if (size) {
			bitmaps[i] = new byte[size];
			stream.read(bitmaps[i], size);
		} else {
			bitmaps[i] = 0;
		}
	}


	if (stream.readByte() == 0xFF) {
		advances = new byte[data.numCharacters];
		stream.read(advances, data.numCharacters);
	}

	if (stream.readByte() == 0xFF) {
		boxes = new BdfBoundingBox[data.numCharacters];
		for (int i = 0; i < data.numCharacters; ++i) {
			boxes[i].width = stream.readByte();
			boxes[i].height = stream.readByte();
			boxes[i].xOffset = stream.readSByte();
			boxes[i].yOffset = stream.readSByte();
		}
	}

	if (stream.eos() || stream.err()) {
		for (int i = 0; i < data.numCharacters; ++i)
			delete[] bitmaps[i];
		delete[] bitmaps;
		delete[] advances;
		delete[] boxes;
		return nullptr;
	}

	data.bitmaps = bitmaps;
	data.advances = advances;
	data.boxes = boxes;
	data.familyName = nullptr;
	data.slant = nullptr;
	data.size = data.height;
	return new BdfFont(data, DisposeAfterUse::YES);
}

static Common::Rect bdfBoxToRect(int ascent, const BdfBoundingBox &bbox) {
	int top = ascent - bbox.yOffset - bbox.height;
	int left = bbox.xOffset;
	int bottom = top + bbox.height;
	int right = left + bbox.width;

	return Common::Rect(left, top, right, bottom);
}

static BdfBoundingBox rectToBdfBox(int ascent, const Common::Rect &rect) {
	BdfBoundingBox bbox;

	bbox.yOffset = ascent - rect.top - rect.height(); 
	bbox.xOffset = rect.left;
	bbox.height = rect.height();
	bbox.width = rect.width();

	return bbox;
}

static BdfBoundingBox scaleBdfBoundingBox(int srcAscent, float srcReferencePointX, float srcReferencePointY, int destAscent, float destReferencePointX, float destReferencePointY, const BdfBoundingBox &srcBBox, float scale) {
	Common::Rect srcRect = bdfBoxToRect(srcAscent, srcBBox);

	int destLeft = static_cast<int>(floorf((static_cast<float>(srcRect.left) - srcReferencePointX) * scale + destReferencePointX));
	int destRight = static_cast<int>(ceilf((static_cast<float>(srcRect.right) - srcReferencePointX) * scale + destReferencePointX));
	int destTop = static_cast<int>(floorf((static_cast<float>(srcRect.top) - srcReferencePointY) * scale + destReferencePointY));
	int destBottom = static_cast<int>(ceilf((static_cast<float>(srcRect.bottom) - srcReferencePointY) * scale + destReferencePointY));

	return rectToBdfBox(destAscent, Common::Rect(destLeft, destTop, destRight, destBottom));
}

BdfFont *BdfFont::scaleFont(const BdfFont *src, int newSize) {
	if (!src) {
		warning("BdfFont::scaleFont(): Empty font reference in scale font");
		return nullptr;
	}

	if (src->getFontSize() == 0) {
		warning("BdfFont::scaleFont(): Requested to scale 0 size font");
		return nullptr;
	}

	if (src->_data.numCharacters == 0) {
		warning("BdfFont::scaleFont(): Requested to scale 0 characters font");
		return nullptr;
	}

	// This is approximate and may not be ideal for all fonts!
	const int scaleCenterYRelativeToBaseline = -1;

	Graphics::Surface srcSurf;
	srcSurf.create(MAX(src->getFontSize() * 2, newSize * 2), MAX(src->getFontSize() * 2, newSize * 2), PixelFormat::createFormatCLUT8());
	int dstGraySize = newSize * 20 * newSize;
	int *dstGray = (int *)malloc(dstGraySize * sizeof(int));

	float scale = (float)newSize / (float)src->getFontSize();

	int scaledAscent = (int)(roundf((float)src->_data.ascent * scale));

	// The reference point is the center point of scaling in the old and new glyph.
	// The coordinate values are relative to the top-left corner of the draw coordinate pixel.
	// Since the baseline is located at the top edge of the pixel that is +ascent from the draw
	// coordinate, we use the intersection of that edge and the left edge of the draw coordinate
	// as the reference point.  This seems to produce good results.
	float srcReferencePointX = 0.0f;
	float srcReferencePointY = static_cast<float>(src->_data.ascent);

	float destReferencePointX = 0.0f;
	float destReferencePointY = static_cast<float>(scaledAscent);

	BdfFontData data;

	data.maxAdvance = (int)(roundf((float)src->_data.maxAdvance * scale));
	data.height = (int)(roundf((float)src->_data.height * scale));
	data.size = (int)(roundf((float)src->_data.size * scale));
	data.ascent = scaledAscent;
	data.defaultBox = scaleBdfBoundingBox(src->_data.ascent, srcReferencePointX, srcReferencePointY, data.ascent, destReferencePointX, destReferencePointY, src->_data.defaultBox, scale);

	data.defaultBox.width = (int)(roundf((float)src->_data.defaultBox.width * scale));
	data.defaultBox.height = (int)(roundf((float)src->_data.defaultBox.height * scale));
	data.defaultBox.xOffset = (int)(roundf((float)src->_data.defaultBox.xOffset * scale));
	data.defaultBox.yOffset = (int)(roundf((float)src->_data.defaultBox.yOffset * scale));
	data.firstCharacter = src->_data.firstCharacter;
	data.defaultCharacter = src->_data.defaultCharacter;
	data.numCharacters = src->_data.numCharacters;
	uint sz = 1 + strlen(src->_data.familyName);
	char *familyName = new char[sz];
	Common::strcpy_s(familyName, sz, src->_data.familyName);
	data.familyName = familyName;
	sz = 1 + strlen(src->_data.slant);
	char *slant = new char[sz];
	Common::strcpy_s(slant, sz, src->_data.slant);
	data.slant = slant;

	Common::Array<Common::Rect> srcRects;

	if (src->_data.boxes) {
		srcRects.resize(data.numCharacters);

		BdfBoundingBox *boxes = new BdfBoundingBox[data.numCharacters];
		for (int i = 0; i < data.numCharacters; ++i)
			boxes[i] = scaleBdfBoundingBox(src->_data.ascent, srcReferencePointX, srcReferencePointY, data.ascent, destReferencePointX, destReferencePointY, src->_data.boxes[i], scale);
		data.boxes = boxes;
	} else {
		// if the sources have null boxes
		data.boxes = nullptr;
	}

	if (src->_data.advances) {
		byte *advances = new byte[data.numCharacters];
		for (int i = 0; i < data.numCharacters; ++i) {
			advances[i] = (int)(roundf((float)src->_data.advances[i] * scale));
		}
		data.advances = advances;
	} else {
		// if the sources have null advances
		data.advances = nullptr;
	}

	byte **bitmaps = new byte *[data.numCharacters];
	for (int i = 0; i < data.numCharacters; i++) {
		const BdfBoundingBox &box = data.boxes ? data.boxes[i] : data.defaultBox;
		const BdfBoundingBox &srcBox = data.boxes ? src->_data.boxes[i] : src->_data.defaultBox;

#if DRAWDEBUG
		int ccc = 'L';
#endif
		if (src->_data.bitmaps[i]) {
			int grayLevel = 10; //box.height * box.width / 3;
			int dstPitch = (box.width + 7) / 8 ;
			const int bytes = dstPitch * box.height;
			bitmaps[i] = new byte[bytes + 1];

#if DRAWDEBUG
			if (i == ccc) {
				Common::Rect srcRect = bdfBoxToRect(src->_data.ascent, srcBox);
				debugN("Ascent: %i\n", static_cast<int>(src->_data.ascent));
				debugN("Source box: (%i,%i) - (%i,%i)\n", static_cast<int>(srcRect.left), static_cast<int>(srcRect.top), static_cast<int>(srcRect.right), static_cast<int>(srcRect.bottom));
				debugN("Source bitmap:\n");

				int srcPitch = (srcBox.width + 7) / 8;
				for (int y = 0; y < srcBox.height; y++) {
					for (int x = 0; x < srcBox.width; x++) {
						byte b = src->_data.bitmaps[i][y * srcPitch + x / 8];
						b >>= 7 - (x % 8);
						b &= 1;

						debugN("%c", b ? '@' : '_');
					}
					debugN(" %i\n", static_cast<int>(y + srcRect.top));
				}
			}
#endif

			Common::Rect rect = bdfBoxToRect(data.ascent, box);
			Common::Rect srcRect = bdfBoxToRect(src->_data.ascent, srcBox);

			byte *ptr = bitmaps[i];
			const byte *srcPtr = src->_data.bitmaps[i];
			int destPitch = (rect.width() + 7) / 8;
			int srcPitch = (srcRect.width() + 7) / 8;

			memset(ptr, 0, destPitch * rect.height());

			float rcpScale = 1.0f / scale;

#if DRAWDEBUG
			if (i == ccc) {
				debugN("New ascent: %i\n", static_cast<int>(data.ascent));
				debugN("Dest box: (%i,%i) - (%i,%i)\n", static_cast<int>(rect.left), static_cast<int>(rect.top), static_cast<int>(rect.right), static_cast<int>(rect.bottom));
			}
#endif

			for (int destRelY = 0; destRelY < rect.height(); destRelY++) {

				int destY = destRelY + rect.top;
				float destPixelCenterY = destY + 0.5f;

				float srcPixelCenterY = (destPixelCenterY - destReferencePointY) * rcpScale + srcReferencePointY;

				int srcY = static_cast<int>(floorf(srcPixelCenterY));

				if (srcY < srcRect.top || srcY >= srcRect.bottom)
					continue;

				int srcRelY = srcY - srcRect.top;

				byte *rowPtr = ptr + destPitch * destRelY;
				const byte *srcRowPtr = srcPtr + srcPitch * srcRelY;

				for (int destRelX = 0; destRelX < rect.width(); destRelX++) {

					int destX = destRelX + rect.left;
					float destPixelCenterX = destX + 0.5f;

					float srcPixelCenterX = (destPixelCenterX - destReferencePointX) * rcpScale + srcReferencePointX;

					int srcX = static_cast<int>(floorf(srcPixelCenterX));

					if (srcX < srcRect.left || srcX >= srcRect.right)
						continue;

					int srcRelX = srcX - srcRect.left;

					if ((srcRowPtr[srcRelX / 8] << (srcRelX % 8)) & 0x80)
						rowPtr[destRelX / 8] |= (0x80 >> (destRelX % 8));
				}
			}

#if DRAWDEBUG
			if (i == ccc) {
				for (int y = 0; y < box.height; y++) {
					const byte *srcRow = (const byte *)&bitmaps[i][y * dstPitch];
					for (int x = 0; x < box.width; x++) {
						int sx = x;
						debugN("%c", (srcRow[sx / 8] & (0x80 >> (sx % 8))) ? '#' : '_');
					}
					debugN(" %i\n", static_cast<int>(y + rect.top));
				}
			}
#endif
		} else {
			bitmaps[i] = nullptr;
		}
	}
	data.bitmaps = bitmaps;
	free(dstGray);
	srcSurf.free();

	return new BdfFont(data, DisposeAfterUse::YES);
}

} // End of namespace Graphics
