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

#include "darkseed/darkseed.h"
#include "darkseed/kofont.h"

namespace Darkseed {

KoFont::KoFont() {
	Common::File fontData;
	if (!fontData.open("k16.bin")) {
		error("Error: failed to open k16.bin");
	}

	loadFontDataSet(_fontDataSet1, 191, fontData);
	loadFontDataSet(_fontDataSet2, 85, fontData);
	loadFontDataSet(_fontDataSet3, 109, fontData);

	fontData.close();
}

void KoFont::loadFontDataSet(Common::Array<Common::Array<uint8> > &dataSet, int size, Common::File &file) {
	dataSet.resize(size);

	for (int i = 0; i < size; i++) {
		dataSet[i].resize(16 * 16, 0);
		loadFontGlyph(dataSet[i], file);
	}
}

static constexpr uint8 kFontPal[4] = {0, 2, 4, 11};

void KoFont::loadFontGlyph(Common::Array<uint8> &pixels, Common::File &file) {
	// Unpack 2bpp font data into 8bpp
	for (int i = 0; i < 64; i++) {
		uint8 byte = file.readByte();
		for (int j = 0; j < 4; j++) {
			pixels[i * 4 + j] = kFontPal[(byte >> (3 - j) * 2) & 3];
		}
	}
}


int KoFont::getFontHeight() const {
	return 16;
}

int KoFont::getMaxCharWidth() const {
	return 16;
}

int KoFont::getCharWidth(uint32 chr) const {
	return getMaxCharWidth();
}

void KoFont::createGlyph(uint8 *pixels, uint32 chr) const {
	uint16 param1, param2, param3;
	extractKoIndexComponents(chr, &param1, &param2, &param3);
	if (param1 < 191) {
		addToGlyph(pixels, param1);
	}
	if (param2 < 85) {
		addToGlyph(pixels, param2 + 191);
	}
	if (param3 < 109) {
		addToGlyph(pixels, param3 + 276);
	}
}

void KoFont::addToGlyph(uint8 *destPixels, int16 index) const {
	if (index < 192) {
		addPixels(destPixels, _fontDataSet1[index]);
	} else if (index < 277) {
		addPixels(destPixels, _fontDataSet2[index - 191]);
	} else {
		addPixels(destPixels, _fontDataSet3[index - 276]);
	}
}

void KoFont::addPixels(uint8 *destPixels, const Common::Array<uint8> &pixels) const {
	for (int i = 0; i < pixels.size(); i++) {
		if (pixels[i] != 0) {
			destPixels[i] = pixels[i];
		}
	}
}

int16 SHORT_ARRAY_1000_01ca[32] = {
	-1, 0, 32, 352,
	672, 992, 1312, 1632,
	1952, 2272, 2592, 2912,
	3232, 3552, 3872, 4192,
	4512, 4832, 5152, 5472,
	5792, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1};

int16 SHORT_ARRAY_1000_020a[32] = {
	-1, -1, 0, 128,
	256, 384, 512, 640,
	-1, -1, 768, 896,
	1024, 1152, 1280, 1408,
	-1, -1, 1536, 1664,
	1792, 1920, 2048, 2176,
	-1, -1, 2304, 2432,
	2560, 2688, -1, -1};

int16 SHORT_ARRAY_1000_024a[32] = {
	-1, 0, 128, 256,
	384, 512, 640, 768,
	896, 1024, 1152, 1280,
	1408, 1536, 1664, 1792,
	1920, 2048, -1, 2176,
	2304, 2432, 2560, 2688,
	2816, 2944, 3072, 3200,
	3328, 3456, -1, -1};

int16 SHORT_ARRAY_1000_028a[32] = {
	-1, 0, 0, 64,
	64, 64, 64, 64,
	64, 64, 64, 64,
	64, 64, 64, 64,
	64, 0, 64, 64,
	64, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1};

int16 SHORT_ARRAY_1000_02ca[32] = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 64, 192, 192,
	0, 0, 192, 64,
	128, 256, 256, 256,
	0, 0, 128, 64,
	192, 0, 0, 0};

int16 SHORT_ARRAY_1000_030a[32] = {
	0, 0, 0, 0,
	64, 0, 64, 32,
	0, 0, 64, 32,
	64, 96, 0, 64,
	0, 0, 32, 96,
	96, 32, 64, 32,
	0, 0, 96, 96,
	32, 32, 0, 0};

int16 SHORT_ARRAY_1000_034a[32] = {
	-1, 0, 32, 32,
	32, 32, 32, 32,
	32, 32, 32, 32,
	32, 32, 32, 32,
	32, 32, -1, 32,
	32, 32, 32, 32,
	32, 32, 32, 32,
	32, 32, -1, -1};

void KoFont::extractKoIndexComponents(uint32 charIdx, uint16 *param_2, uint16 *param_3, uint16 *param_4) const {
	int uVar1;
	int iVar2;
	int uVar3;
	int uVar4;
	int uVar5;

	iVar2 = (charIdx & 31) * 2;
	uVar5 = (charIdx << 1) >> 5 & 62;
	uVar4 = (charIdx << 1) >> 10 & 62;
	uVar1 = SHORT_ARRAY_1000_020a[uVar5/2];
	if (uVar1 > 0) {
		uVar1 += SHORT_ARRAY_1000_028a[uVar4/2] + SHORT_ARRAY_1000_034a[iVar2/2] - 3;
	}
	uVar4 = SHORT_ARRAY_1000_01ca[uVar4/2];
	if (uVar4 > 0) {
		uVar4 += SHORT_ARRAY_1000_02ca[uVar5/2] + SHORT_ARRAY_1000_034a[iVar2/2];
	}
	uVar3 = SHORT_ARRAY_1000_024a[iVar2/2];
	if (uVar3 > 0) {
		uVar3 += SHORT_ARRAY_1000_030a[uVar5/2] - 3;
	}
	*param_2 = uVar4 >> 5;
	*param_3 = (uVar1 >> 5) - 2;
	*param_4 = (uVar3 >> 5) - 2;
}

void KoFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	uint8 pixels[256];
	memset(pixels, 0, 256);
	createGlyph(pixels, chr);
	g_engine->_screen->copyRectToSurfaceWithKey(pixels, 16, x, y, 16, 16,0xf);
}



} // namespace Darkseed
