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

#include "common/file.h"
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/dark/dark.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

extern byte kC64Palette[16][3];

static const byte kDarkC64CompassMask[] = {0xfe, 0xfa, 0x69, 0xf5, 0xea};
static const byte kDarkC64CompassColor1[] = {0xfc, 0x1f, 0xf1, 0xcb, 0x3b};
static const byte kDarkC64CompassColor2[] = {0x0b, 0x0c, 0x0c, 0x0f, 0x0c};
static const byte kDarkC64ModeIndicatorPalette[2][3][4] = {
	{{0, 11, 15, 1}, {0, 11, 15, 2}, {0, 15, 1, 11}},
	{{0, 15, 12, 11}, {0, 15, 12, 11}, {0, 15, 2, 11}}
};

static bool isDarkC64BrokenCompassArea(uint16 areaID) {
	return areaID == 1 || areaID == 18 || areaID == 27 || areaID == 28;
}

static int getDarkC64CompassTarget(float yaw) {
	int target = int(yaw / 5.0f + 0.5f);
	target = (target + 18) % 72;
	if (target < 0)
		target += 72;
	return target;
}

static const byte *getDarkC64IndicatorSprite(const byte *spriteData, byte pointer) {
	assert(pointer >= 3 && pointer <= 10);
	return spriteData + (pointer - 3) * 64;
}

static void blitDarkC64IndicatorSprite(Graphics::Surface *surface, const byte *sprite, byte color, const Graphics::PixelFormat &pixelFormat) {
	uint32 pixel = pixelFormat.ARGBToColor(0xFF, kC64Palette[color][0], kC64Palette[color][1], kC64Palette[color][2]);
	for (int y = 0; y < 21; y++) {
		for (int byteIndex = 0; byteIndex < 3; byteIndex++) {
			byte value = sprite[y * 3 + byteIndex];
			for (int bit = 0; bit < 8; bit++) {
				if ((value & (0x80 >> bit)) == 0)
					continue;
				surface->setPixel(byteIndex * 8 + bit, y, pixel);
			}
		}
	}
}

static Graphics::Surface *composeDarkC64Indicator(const byte *spriteData, byte basePointer, byte overlayPointer1, byte overlayColor1, byte overlayPointer2, byte overlayColor2, bool includeSprite6, const Graphics::PixelFormat &pixelFormat) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(24, 21, pixelFormat);
	surface->fillRect(Common::Rect(0, 0, surface->w, surface->h), pixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00));

	blitDarkC64IndicatorSprite(surface, getDarkC64IndicatorSprite(spriteData, basePointer), 2, pixelFormat);
	blitDarkC64IndicatorSprite(surface, getDarkC64IndicatorSprite(spriteData, 4), 12, pixelFormat);
	if (overlayPointer1)
		blitDarkC64IndicatorSprite(surface, getDarkC64IndicatorSprite(spriteData, overlayPointer1), overlayColor1, pixelFormat);
	if (overlayPointer2)
		blitDarkC64IndicatorSprite(surface, getDarkC64IndicatorSprite(spriteData, overlayPointer2), overlayColor2, pixelFormat);
	if (includeSprite6)
		blitDarkC64IndicatorSprite(surface, getDarkC64IndicatorSprite(spriteData, 6), 8, pixelFormat);
	return surface;
}

static void loadDarkC64CompassTable(Common::SeekableReadStream *file, uint32 offset, Common::Array<byte> &table) {
	table.resize(80);
	file->seek(offset);
	file->read(table.data(), table.size());
}

static void loadDarkC64Indicators(Common::SeekableReadStream *file, uint32 offset, Common::Array<Graphics::Surface *> &indicators, const Graphics::PixelFormat &pixelFormat) {
	byte spriteData[8 * 64];
	file->seek(offset);
	file->read(spriteData, sizeof(spriteData));

	// $8161 has three distinct body states: crawl (pointer 3 with layers 5/7),
	// walk (pointer 3 with layers disabled), and fly (pointer 8 with 5/7 then 9/10).
	indicators.push_back(composeDarkC64Indicator(spriteData, 3, 0, 0, 0, 0, false, pixelFormat));
	indicators.push_back(composeDarkC64Indicator(spriteData, 3, 0, 0, 0, 0, false, pixelFormat));
	indicators.push_back(composeDarkC64Indicator(spriteData, 8, 5, 7, 7, 8, true, pixelFormat));
	indicators.push_back(composeDarkC64Indicator(spriteData, 8, 9, 7, 10, 8, true, pixelFormat));
}

static void loadDarkC64ModeFrames(Common::SeekableReadStream *file, uint32 offset, Common::Array<Graphics::ManagedSurface *> &frames, const Graphics::PixelFormat &pixelFormat) {
	byte data[6 * 48];
	file->seek(offset);
	file->read(data, sizeof(data));

	for (int frameIndex = 0; frameIndex < 6; frameIndex++) {
		Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
		surface->create(24, 16, pixelFormat);
		surface->fillRect(Common::Rect(0, 0, surface->w, surface->h),
			pixelFormat.ARGBToColor(0xFF, kC64Palette[0][0], kC64Palette[0][1], kC64Palette[0][2]));

		for (int band = 0; band < 2; band++) {
			for (int cell = 0; cell < 3; cell++) {
				for (int row = 0; row < 8; row++) {
					byte bitmapByte = data[frameIndex * 48 + band * 24 + cell * 8 + row];
					for (int pair = 0; pair < 4; pair++) {
						byte color = kDarkC64ModeIndicatorPalette[band][cell][(bitmapByte >> (6 - pair * 2)) & 0x03];
						uint32 pixel = pixelFormat.ARGBToColor(0xFF, kC64Palette[color][0], kC64Palette[color][1], kC64Palette[color][2]);
						int x = cell * 8 + pair * 2;
						int y = band * 8 + row;
						surface->setPixel(x, y, pixel);
						surface->setPixel(x + 1, y, pixel);
					}
				}
			}
		}
		frames.push_back(surface);
	}
}

void DarkEngine::initC64() {
	_viewArea = Common::Rect(32, 24, 288, 127);
}

void DarkEngine::drawC64Compass(Graphics::Surface *surface) {
	if (!_currentArea || _c64CompassTable.size() < 80)
		return;

	int target = getDarkC64CompassTarget(_yaw);
	if (!_c64CompassInitialized) {
		_c64CompassPosition = target;
		_c64CompassInitialized = true;
	}

	if (isDarkC64BrokenCompassArea(_currentArea->getAreaID())) {
		if ((_ticks & 1) == 0)
			_c64CompassPosition = (_c64CompassPosition + 71) % 72;
	} else {
		int delta = (_c64CompassPosition - target + 72) % 72;
		if (delta != 0) {
			if (delta < 37)
				_c64CompassPosition = (_c64CompassPosition + 71) % 72;
			else
				_c64CompassPosition = (_c64CompassPosition + 1) % 72;
		}
	}

	int start = (_c64CompassPosition + 72 - 20) % 72;
	int tableOffset = start & 0xf8;
	int shift = (start & 0x07) >> 1;

	for (int cell = 0; cell < 5; cell++) {
		byte color1 = kDarkC64CompassColor2[cell] & 0x0f;
		byte color2 = kDarkC64CompassColor1[cell] >> 4;
		byte color3 = kDarkC64CompassColor1[cell] & 0x0f;
		byte mask = kDarkC64CompassMask[cell];

		for (int row = 0; row < 8; row++) {
			int index = tableOffset + cell * 8 + row;
			if (index >= 72)
				index -= 72;

			byte bitmapByte = _c64CompassTable[index];
			if (shift > 0) {
				byte nextByte = _c64CompassTable[index + 8];
				bitmapByte = ((bitmapByte << (2 * shift)) | (nextByte >> (8 - 2 * shift))) & 0xff;
			}
			bitmapByte &= mask;

			for (int pair = 0; pair < 4; pair++) {
				byte pixelPair = (bitmapByte >> (6 - pair * 2)) & 0x03;
				byte color = 0;
				if (pixelPair == 1)
					color = color2;
				else if (pixelPair == 2)
					color = color3;
				else if (pixelPair == 3)
					color = color1;

				uint32 pixel = _gfx->_texturePixelFormat.ARGBToColor(0xFF, kC64Palette[color][0], kC64Palette[color][1], kC64Palette[color][2]);
				surface->setPixel(256 + cell * 8 + pair * 2, 144 + row, pixel);
				surface->setPixel(256 + cell * 8 + pair * 2 + 1, 144 + row, pixel);
			}
		}
	}
}

void DarkEngine::drawC64ModeIndicator(Graphics::Surface *surface) {
	if (_c64ModeFrames.size() < 6)
		return;

	const Graphics::ManagedSurface *frame = _shootMode ? _c64ModeFrames[0] : _c64ModeFrames[1];

	surface->copyRectToSurface(*frame, 264, 176, Common::Rect(frame->w, frame->h));
}

void DarkEngine::loadAssetsC64FullGame() {
	Common::File file;
	file.open("darkside.c64.data");

	if (_variant & GF_C64_TAPE) {
		int size = file.size();

		byte *buffer = (byte *)malloc(size * sizeof(byte));
		file.read(buffer, file.size());

		_extraBuffer = decompressC64RLE(buffer, &size, 0xdf);
		// size should be the size of the decompressed data
		Common::MemoryReadStream dfile(_extraBuffer, size, DisposeAfterUse::NO);

		loadMessagesFixedSize(&dfile, 0x1edf, 16, 27);
		loadFonts(&dfile, 0xc3e);
		loadGlobalObjects(&dfile, 0x20bd, 23);
		load8bitBinary(&dfile, 0x9b3e, 16);
		loadDarkC64CompassTable(&dfile, 0x7e37, _c64CompassTable);
		loadDarkC64Indicators(&dfile, 0xadba, _indicators, _gfx->_texturePixelFormat);
		loadDarkC64ModeFrames(&dfile, 0xd2f6, _c64ModeFrames, _gfx->_texturePixelFormat);
	} else if (_variant & GF_C64_DISC) {
		loadMessagesFixedSize(&file, 0x16a3, 16, 27);
		loadFonts(&file, 0x402);

		// It is unclear why this C64 has this byte changed at 0x1881
		// Once the game is loaded, it will be set to 0x66
		// and the game will work
		file.seek(0x1881);
		_extraBuffer = (byte *)malloc(0x300 * sizeof(byte));
		file.read(_extraBuffer, 0x300);
		_extraBuffer[0] = 0x66;
		Common::MemoryReadStream stream(_extraBuffer, 0x300, DisposeAfterUse::NO);
		loadGlobalObjects(&stream, 0x0, 23);
		load8bitBinary(&file, 0x8914, 16);
		loadDarkC64CompassTable(&file, 0x7c5a, _c64CompassTable);
		loadDarkC64Indicators(&file, 0xb2d4, _indicators, _gfx->_texturePixelFormat);
		loadDarkC64ModeFrames(&file, 0xc0d1, _c64ModeFrames, _gfx->_texturePixelFormat);
	} else
		error("Unknown C64 variant %x", _variant);

	// The color map from the data is not correct,
	// so we'll just hardcode the one that we found in the executable

	for (int i = 0; i < 15; i++) {
		_colorMap[i][0] = 0;
		_colorMap[i][1] = 0;
		_colorMap[i][2] = 0;
		_colorMap[i][3] = 0;
	}

	_colorMap[1][0] = 0x55;
	_colorMap[1][1] = 0x55;
	_colorMap[1][2] = 0x55;
	_colorMap[1][3] = 0x55;

	_colorMap[2][0] = 0xaa;
	_colorMap[2][1] = 0xaa;
	_colorMap[2][2] = 0xaa;
	_colorMap[2][3] = 0xaa;

	_colorMap[3][0] = 0xff;
	_colorMap[3][1] = 0xff;
	_colorMap[3][2] = 0xff;
	_colorMap[3][3] = 0xff;

	_colorMap[4][0] = 0x44;
	_colorMap[4][1] = 0x11;
	_colorMap[4][2] = 0x44;
	_colorMap[4][3] = 0x11;

	_colorMap[5][0] = 0x88;
	_colorMap[5][1] = 0x22;
	_colorMap[5][2] = 0x88;
	_colorMap[5][3] = 0x22;

	_colorMap[6][0] = 0xcc;
	_colorMap[6][1] = 0x33;
	_colorMap[6][2] = 0xcc;
	_colorMap[6][3] = 0x33;

	_colorMap[7][0] = 0x66;
	_colorMap[7][1] = 0x99;
	_colorMap[7][2] = 0x66;
	_colorMap[7][3] = 0x99;

	_colorMap[8][0] = 0x77;
	_colorMap[8][1] = 0xdd;
	_colorMap[8][2] = 0x77;
	_colorMap[8][3] = 0xdd;

	_colorMap[9][0] = 0xbb;
	_colorMap[9][1] = 0xee;
	_colorMap[9][2] = 0xbb;
	_colorMap[9][3] = 0xee;

	_colorMap[10][0] = 0x5a;
	_colorMap[10][1] = 0xa5;
	_colorMap[10][2] = 0x5a;
	_colorMap[10][3] = 0xa5;

	// TODO
	_colorMap[12][0] = 0x00;
	_colorMap[12][1] = 0x00;
	_colorMap[12][2] = 0x00;
	_colorMap[12][3] = 0x00;

	_colorMap[13][0] = 0x77;
	_colorMap[13][1] = 0xdd;
	_colorMap[13][2] = 0x77;
	_colorMap[13][3] = 0xdd;

	// TODO
	_colorMap[14][0] = 0xcc;
	_colorMap[14][1] = 0xcc;
	_colorMap[14][2] = 0xcc;
	_colorMap[14][3] = 0xcc;

	Graphics::Surface *surf = loadBundledImage("dark_border");
	surf->convertToInPlace(_gfx->_texturePixelFormat);
	_border = new Graphics::ManagedSurface();
	_border->copyFrom(*surf);
	surf->free();
	delete surf;

	file.close();
	file.open("darkside.c64.title.bitmap");

	Common::File colorFile1;
	colorFile1.open("darkside.c64.title.colors1");
	Common::File colorFile2;
	colorFile2.open("darkside.c64.title.colors2");

	_title = loadAndConvertDoodleImage(&file, &colorFile1, &colorFile2, (byte *)&kC64Palette);

	// Only one SID instance can be active at a time; music is the default.
	// Create the inactive player first so its SID is destroyed before
	// the active player's SID is created.
	_playerC64Sfx = new DarkSideC64SFXPlayer();
	_playerC64Sfx->destroySID();
	_playerMusic = new DarkSideC64MusicPlayer();
}

void DarkEngine::playSoundC64(int index) {
	debugC(1, kFreescapeDebugMedia, "Playing Dark Side C64 SFX %d", index);
	if (_playerC64Sfx && _c64UseSFX)
		_playerC64Sfx->playSfx(index);
}

void DarkEngine::toggleC64Sound() {
	if (_c64UseSFX) {
		if (_playerC64Sfx)
			_playerC64Sfx->destroySID();
		if (_playerMusic)
			_playerMusic->startMusic();
		_c64UseSFX = false;
	} else {
		if (_playerMusic)
			_playerMusic->stopMusic();
		if (_playerC64Sfx)
			_playerC64Sfx->initSID();
		_c64UseSFX = true;
	}
}

void DarkEngine::drawC64UI(Graphics::Surface *surface) {
	uint8 r, g, b;
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xAA, 0xAA, 0xAA);

	uint32 color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	int ecds = _gameStateVars[kVariableActiveECDs];
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 206, 137 + 8, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 206, 145 + 8, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 206, 153 + 8, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 68 + 5 + 5, 168 + 9, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 70, 177 + 8, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 86, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%3d%%", ecds), 198, 8, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 120, 185, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else
		drawStringInSurface(_currentArea->_name, 120, 185, front, back, surface);

	int energy = _gameStateVars[k8bitVariableEnergy]; // called fuel in this game
	int shield = _gameStateVars[k8bitVariableShield];

	_gfx->readFromPalette(6, r, g, b); // Violet Blue
	uint32 outBarColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(14, r, g, b); // Violet
	uint32 inBarColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	_gfx->readFromPalette(3, r, g, b); // Light Blue
	uint32 lineColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Rect coverBar;
	coverBar = Common::Rect(64, 144, 135, 151);
	surface->fillRect(coverBar, back);

	if (shield >= 0) {
		Common::Rect shieldBar;

		shieldBar = Common::Rect(64, 144, 127 - (_maxShield - shield), 151);
		surface->fillRect(shieldBar, outBarColor);

		shieldBar = Common::Rect(64, 146, 127 - (_maxShield - shield), 149);
		surface->fillRect(shieldBar, inBarColor);
		if (shield >= 1)
			surface->drawLine(64, 147, 127 - (_maxShield - shield) - 1, 147, lineColor);
	}

	coverBar = Common::Rect(64, 144 + 8, 127, 159);
	surface->fillRect(coverBar, back);

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(64, 144 + 8, 127 - (_maxEnergy - energy), 159);
		surface->fillRect(energyBar, outBarColor);

		energyBar = Common::Rect(64, 146 + 8, 127 - (_maxEnergy - energy), 157);
		surface->fillRect(energyBar, inBarColor);
		if (energy >= 1)
			surface->drawLine(64, 147 + 8, 127 - (_maxEnergy - energy) - 1, 155, lineColor);
	}
	drawBinaryClock(surface, 304, 124, front, back);
	drawC64Compass(surface);
	drawVerticalCompass(surface, 17, 77, _pitch, front);
	surface->fillRect(Common::Rect(152, 148 - 5, 184, 176 - 15), _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00));
	drawIndicator(surface, 160 - 1, 148 - 1);
	drawC64ModeIndicator(surface);
}

} // End of namespace Freescape
