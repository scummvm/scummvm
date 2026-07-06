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

#include "hollywood/font.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "graphics/surface.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const char *const kExecutableName = "MONSTERS.EXE";
const char *const kResource000Name = "RESOURCE.000";
const uint32 kOriginalCharacterMapVa = 0x00501540;
const uint kCharacterMapSize = 0x100;
const uint kResource000HeaderByteCount = 1;
const uint kResource000OffsetTableSize = 400;
const uint kResource000SizeTableSize = 400;
const uint kResource000FontOffsetEntry = 0xb0;
const uint kGlyphDescriptorSize = 5;
const byte kFontSpaceGlyph = 0xfe;
const byte kFontUnsupportedGlyph = 0xff;
const int kOriginalSpeechLineHeight = 20;
const byte kFallbackSpanishCharacterMap[kCharacterMapSize] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xfe, 0x53, 0xff, 0x59, 0xff, 0x5a, 0xff, 0xff, 0x44, 0x45, 0xff, 0x47, 0x4d, 0x46, 0x4f, 0x4b,
	0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x50, 0x4e, 0x49, 0x48, 0x4a, 0x55,
	0x5b, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x57, 0x4c, 0x58, 0xff, 0xff,
	0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x29,
	0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x5e, 0xff, 0x36, 0xff, 0xff, 0x60, 0xff, 0x5f, 0xff, 0xff, 0x61, 0xff, 0xff, 0x62, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x63, 0xff, 0x64, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x35, 0x37, 0x38, 0x39, 0x28, 0x56, 0xff, 0xff, 0x54, 0xff, 0xff, 0xff, 0xff, 0x52, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5d, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5c, 0x51, 0xff, 0xff
};

HollywoodFont::HollywoodFont() :
		_shadowColor(0),
		_maxCharWidth(0),
		_loaded(false) {
}

bool HollywoodFont::load() {
	if (_loaded)
		return true;

	Common::File file;
	if (!file.open(Common::Path(kResource000Name))) {
		warning("Failed to open %s for Hollywood font", kResource000Name);
		return false;
	}

	if (!loadCharacterMap(Common::Path(kExecutableName)))
		return false;

	const uint32 startupTableOffset = kResource000HeaderByteCount;
	if ((uint32)file.size() < startupTableOffset + kResource000OffsetTableSize + kResource000SizeTableSize) {
		warning("%s is too small for Hollywood font tables", kResource000Name);
		return false;
	}

	Common::Array<byte> offsetTable;
	Common::Array<byte> sizeTable;
	offsetTable.resize(kResource000OffsetTableSize);
	sizeTable.resize(kResource000SizeTableSize);

	file.seek(startupTableOffset);
	if (file.read(offsetTable.data(), offsetTable.size()) != offsetTable.size() ||
			file.read(sizeTable.data(), sizeTable.size()) != sizeTable.size()) {
		warning("Failed to read %s startup tables for Hollywood font", kResource000Name);
		return false;
	}

	const uint32 glyphPixelsOffset = READ_LE_UINT32(offsetTable.data() + kResource000FontOffsetEntry);
	const uint32 glyphPixelsSize = READ_LE_UINT32(sizeTable.data() + kResource000FontOffsetEntry);
	const uint32 glyphDescriptorSize = READ_LE_UINT32(sizeTable.data() + kResource000FontOffsetEntry + 4);
	if (glyphDescriptorSize % kGlyphDescriptorSize != 0 ||
			glyphPixelsOffset > (uint32)file.size() ||
			glyphPixelsSize > (uint32)file.size() - glyphPixelsOffset ||
			glyphDescriptorSize > (uint32)file.size() - glyphPixelsOffset - glyphPixelsSize) {
		warning("%s has invalid Hollywood font table bounds", kResource000Name);
		return false;
	}

	_glyphPixels.resize(glyphPixelsSize);
	file.seek(glyphPixelsOffset);
	if (file.read(_glyphPixels.data(), _glyphPixels.size()) != _glyphPixels.size()) {
		warning("Failed to read Hollywood font glyph pixels");
		return false;
	}

	Common::Array<byte> descriptorData;
	descriptorData.resize(glyphDescriptorSize);
	if (file.read(descriptorData.data(), descriptorData.size()) != descriptorData.size()) {
		warning("Failed to read Hollywood font glyph descriptors");
		return false;
	}

	_glyphs.resize(glyphDescriptorSize / kGlyphDescriptorSize);
	_maxCharWidth = 0;
	for (uint i = 0; i < _glyphs.size(); ++i) {
		const uint offset = i * kGlyphDescriptorSize;
		GlyphDescriptor &glyph = _glyphs[i];
		glyph.offset = READ_LE_UINT16(descriptorData.data() + offset);
		glyph.yOffset = descriptorData[offset + 2];
		glyph.height = descriptorData[offset + 3];
		glyph.width = descriptorData[offset + 4];

		if (glyph.width >= 2)
			_maxCharWidth = MAX<int>(_maxCharWidth, glyph.width - 2);
	}
	_maxCharWidth = MAX<int>(_maxCharWidth, 8);

	_loaded = true;
	return true;
}

int HollywoodFont::getFontHeight() const {
	return kOriginalSpeechLineHeight;
}

int HollywoodFont::getMaxCharWidth() const {
	return _maxCharWidth;
}

int HollywoodFont::getCharWidth(uint32 chr) const {
	const byte glyphId = mapCharacter(chr);
	if (glyphId == kFontSpaceGlyph)
		return 8;

	const GlyphDescriptor *glyph = getGlyph(chr);
	if (!glyph || glyph->width < 2)
		return 0;

	return glyph->width - 2;
}

void HollywoodFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (!dst || dst->format.bytesPerPixel != 1)
		return;

	const GlyphDescriptor *glyph = getGlyph(chr);
	if (!glyph)
		return;

	const uint pixelCount = (uint)glyph->width * glyph->height;
	if (glyph->offset + pixelCount > _glyphPixels.size())
		return;

	for (uint row = 0; row < glyph->height; ++row) {
		const int dstY = y + glyph->yOffset + row;
		if (dstY < 0 || dstY >= dst->h)
			continue;

		for (uint column = 0; column < glyph->width; ++column) {
			const int dstX = x + column;
			if (dstX < 0 || dstX >= dst->w)
				continue;

			const byte pixel = _glyphPixels[glyph->offset + row * glyph->width + column];
			if (pixel == 1)
				*((byte *)dst->getBasePtr(dstX, dstY)) = (byte)color;
			else if (pixel == 2 || pixel == 3)
				*((byte *)dst->getBasePtr(dstX, dstY)) = _shadowColor;
		}
	}
}

byte HollywoodFont::mapCharacter(uint32 chr) const {
	if (!_loaded || _characterMap.size() != kCharacterMapSize)
		return kFontUnsupportedGlyph;

	return _characterMap[chr & 0xff];
}

const HollywoodFont::GlyphDescriptor *HollywoodFont::getGlyph(uint32 chr) const {
	const byte glyphId = mapCharacter(chr);
	if (glyphId == kFontSpaceGlyph || glyphId == kFontUnsupportedGlyph || glyphId >= _glyphs.size())
		return nullptr;

	return &_glyphs[glyphId];
}

bool HollywoodFont::loadCharacterMap(const Common::Path &exeName) {
	_characterMap.resize(kCharacterMapSize);
	if (!readPeDataAtVa(exeName, kOriginalCharacterMapVa, _characterMap)) {
		debugC(1, kDebugResources, "Using built-in Hollywood font character map; %s is not a PE resource source",
			exeName.toString().c_str());
		loadFallbackCharacterMap();
	} else if (!isValidCharacterMap(_characterMap)) {
		warning("Ignoring invalid Hollywood font character map in %s; using built-in map",
			exeName.toString().c_str());
		loadFallbackCharacterMap();
	}

	return true;
}

void HollywoodFont::loadFallbackCharacterMap() {
	_characterMap.resize(kCharacterMapSize);
	memcpy(_characterMap.data(), kFallbackSpanishCharacterMap, kCharacterMapSize);
}

bool HollywoodFont::isValidCharacterMap(const Common::Array<byte> &characterMap) const {
	if (characterMap.size() != kCharacterMapSize || characterMap[' '] != kFontSpaceGlyph)
		return false;

	bool seenGlyphs[256] = {};
	uint distinctGlyphCount = 0;
	const char *const requiredGlyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,!?";
	for (const char *chr = requiredGlyphs; *chr; ++chr) {
		const byte glyphId = characterMap[(byte)*chr];
		if (glyphId == kFontSpaceGlyph || glyphId == kFontUnsupportedGlyph)
			return false;

		if (!seenGlyphs[glyphId]) {
			seenGlyphs[glyphId] = true;
			++distinctGlyphCount;
		}
	}

	return distinctGlyphCount >= 60;
}

bool HollywoodFont::readPeDataAtVa(const Common::Path &exeName, uint32 virtualAddress, Common::Array<byte> &destination) const {
	Common::File file;
	if (!file.open(exeName))
		return false;

	if (file.size() < 0x40)
		return false;

	file.seek(0x3c);
	const uint32 peHeaderOffset = file.readUint32LE();
	if (peHeaderOffset + 0x18 > (uint32)file.size())
		return false;

	file.seek(peHeaderOffset);
	if (file.readUint32BE() != MKTAG('P', 'E', 0, 0))
		return false;

	file.seek(peHeaderOffset + 6);
	const uint16 sectionCount = file.readUint16LE();
	file.seek(peHeaderOffset + 20);
	const uint16 optionalHeaderSize = file.readUint16LE();
	const uint32 optionalHeaderOffset = peHeaderOffset + 24;
	if (optionalHeaderOffset + optionalHeaderSize > (uint32)file.size() || optionalHeaderSize < 32)
		return false;

	file.seek(optionalHeaderOffset);
	if (file.readUint16LE() != 0x10b)
		return false;

	file.seek(optionalHeaderOffset + 28);
	const uint32 imageBase = file.readUint32LE();
	if (virtualAddress < imageBase)
		return false;

	const uint32 rva = virtualAddress - imageBase;
	const uint32 sectionTableOffset = optionalHeaderOffset + optionalHeaderSize;
	if (sectionTableOffset + sectionCount * 40 > (uint32)file.size())
		return false;

	for (uint sectionIndex = 0; sectionIndex < sectionCount; ++sectionIndex) {
		const uint32 sectionOffset = sectionTableOffset + sectionIndex * 40;
		file.seek(sectionOffset + 8);
		const uint32 virtualSize = file.readUint32LE();
		const uint32 virtualAddressBase = file.readUint32LE();
		const uint32 rawDataSize = file.readUint32LE();
		const uint32 rawDataOffset = file.readUint32LE();
		const uint32 mappedSize = MAX<uint32>(virtualSize, rawDataSize);

		if (rva < virtualAddressBase || rva + destination.size() > virtualAddressBase + mappedSize)
			continue;

		const uint32 fileOffset = rawDataOffset + (rva - virtualAddressBase);
		if (fileOffset > (uint32)file.size() || destination.size() > (uint32)file.size() - fileOffset)
			return false;

		file.seek(fileOffset);
		return file.read(destination.data(), destination.size()) == destination.size();
	}

	return false;
}

} // End of namespace Hollywood
