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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/config-manager.h"

#include "freescape/freescape.h"
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/games/eclipse/opl.music.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void EclipseEngine::initDOS() {
	if (_renderMode == Common::kRenderHercG)
		_viewArea = Common::Rect(110, 79, 594, 242);
	else
		_viewArea = Common::Rect(40, 33, 280, 133);
	_soundIndexShoot = 18;
	_soundIndexCollide = 1;
	_soundIndexStepDown = 3;
	_soundIndexStepUp = 3;
	_soundIndexMenu = -1;
	_soundIndexStart = 9;
	_soundIndexAreaChange = 5;
}

void EclipseEngine::loadHeartFramesDOS(Common::SeekableReadStream *file, int restOffset, int beatOffset) {
	// Stores into _eclipseSprites[0] (beat) and [1] (rest).
	int offsets[2] = { beatOffset, restOffset };

	for (int f = 0; f < 2; f++) {
		file->seek(offsets[f]);
		int height = file->readByte();
		int widthBytes = file->readByte();

		if (_renderMode == Common::kRenderEGA) {
			// EGA: 4 plane pointers followed by monochrome data per plane.
			int planeFileOffsets[4];
			for (int p = 0; p < 4; p++)
				planeFileOffsets[p] = file->readUint16LE() + 0x200;

			Graphics::ManagedSurface clut8;
			clut8.create(widthBytes * 8, height, Graphics::PixelFormat::createFormatCLUT8());
			clut8.fillRect(Common::Rect(0, 0, widthBytes * 8, height), 0);

			for (int p = 0; p < 4; p++) {
				Graphics::ManagedSurface plane;
				plane.create(widthBytes * 8, height, Graphics::PixelFormat::createFormatCLUT8());
				plane.fillRect(Common::Rect(0, 0, widthBytes * 8, height), 0);

				file->seek(planeFileOffsets[p]);
				loadFrame(file, &plane, widthBytes, height, 1);

				for (int y = 0; y < height; y++)
					for (int x = 0; x < widthBytes * 8; x++)
						if (plane.getPixel(x, y))
							clut8.setPixel(x, y, clut8.getPixel(x, y) | (1 << p));
			}

			clut8.setPalette((byte *)kEGADefaultPalette, 0, 16);

			Graphics::Surface *converted = _gfx->convertImageFormatIfNecessary(&clut8);
			auto *surf = new Graphics::ManagedSurface();
			surf->copyFrom(*converted);
			converted->free();
			delete converted;
			_eclipseSprites.push_back(surf);
		} else if (_renderMode == Common::kRenderHercG) {
			// Hercules: one bit per pixel, greys are dithered in the artwork
			Graphics::ManagedSurface clut8;
			clut8.create(widthBytes * 8, height, Graphics::PixelFormat::createFormatCLUT8());

			for (int y = 0; y < height; y++)
				for (int col = 0; col < widthBytes; col++) {
					byte b = file->readByte();
					for (int px = 0; px < 8; px++)
						clut8.setPixel(col * 8 + px, y, (b >> (7 - px)) & 1);
				}

			clut8.setPalette((byte *)kHerculesPaletteGreen, 0, 2);

			Graphics::Surface *converted = _gfx->convertImageFormatIfNecessary(&clut8);
			auto *surf = new Graphics::ManagedSurface();
			surf->copyFrom(*converted);
			converted->free();
			delete converted;
			_eclipseSprites.push_back(surf);
		} else {
			// CGA: packed 2-bit pixels (4 pixels per byte), no planes.
			Graphics::ManagedSurface clut8;
			clut8.create(widthBytes * 4, height, Graphics::PixelFormat::createFormatCLUT8());

			for (int y = 0; y < height; y++)
				for (int col = 0; col < widthBytes; col++) {
					byte b = file->readByte();
					for (int px = 0; px < 4; px++)
						clut8.setPixel(col * 4 + px, y, (b >> (6 - px * 2)) & 3);
				}

			// Kept indexed, since the CGA palette changes per area
			auto *indexed = new Graphics::ManagedSurface();
			indexed->copyFrom(clut8);
			_heartFramesIndexed.push_back(indexed);
		}
	}
}

// The bundle has no CGA or Hercules ankh, so the EGA one is reduced to a mask and
// colored here. The solid ankh over collected slots is lifted from the border.
void EclipseEngine::loadAnkhCollectedMask() {
	int scale = _renderMode == Common::kRenderHercG ? 2 : 1;
	int srcX = _renderMode == Common::kRenderHercG ? 122 : 45;
	int srcY = _renderMode == Common::kRenderHercG ? 52 : 4;

	_ankhCollectedMask = new Graphics::ManagedSurface();
	_ankhCollectedMask->create(7 * scale, 12, Graphics::PixelFormat::createFormatCLUT8());

	for (int y = 0; y < 12; y++)
		for (int x = 0; x < 7 * scale; x++)
			_ankhCollectedMask->setPixel(x, y, _border->getPixel(srcX + x, srcY + y) ? 1 : 0);
}

void EclipseEngine::loadAnkhIndicatorMask() {
	// Hercules pixels are half as wide, so the original doubles every column
	int scale = _renderMode == Common::kRenderHercG ? 2 : 1;
	Graphics::Surface *ega = loadBundledImage("eclipse_ankh_indicator_ega", false);
	ega->convertToInPlace(_gfx->_texturePixelFormat);

	_ankhIndicatorMask = new Graphics::ManagedSurface();
	_ankhIndicatorMask->create(ega->w * scale, ega->h, Graphics::PixelFormat::createFormatCLUT8());

	for (int y = 0; y < ega->h; y++) {
		for (int x = 0; x < ega->w; x++) {
			uint8 r, g, b;
			ega->format.colorToRGB(ega->getPixel(x, y), r, g, b);
			for (int i = 0; i < scale; i++)
				_ankhIndicatorMask->setPixel(x * scale + i, y, (r || g || b) ? 1 : 0);
		}
	}

	ega->free();
	delete ega;
}

void EclipseEngine::updateAnkhIndicator(const byte *palette) {
	if (!_ankhIndicatorMask)
		return;

	for (auto &it : _indicators) {
		it->free();
		delete it;
	}
	_indicators.clear();

	// The original draws the ankhs with color 1 of the area palette
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, palette[3], palette[4], palette[5]);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, palette[0], palette[1], palette[2]);

	// [0] goes over the slots still to collect, [1] over the collected ones
	Graphics::ManagedSurface *masks[2] = { _ankhIndicatorMask, _ankhCollectedMask };

	for (int i = 0; i < 2; i++) {
		if (!masks[i])
			continue;

		Graphics::Surface *surface = new Graphics::Surface();
		surface->create(masks[i]->w, masks[i]->h, _gfx->_texturePixelFormat);

		for (int y = 0; y < surface->h; y++)
			for (int x = 0; x < surface->w; x++)
				surface->setPixel(x, y, masks[i]->getPixel(x, y) ? front : back);

		_indicators.push_back(surface);
	}
}

void EclipseEngine::loadAssetsDOSFullGame() {
	Common::File file;
	if (_renderMode == Common::kRenderEGA) {
		file.open("SCN1E.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		}
		file.close();
		file.open("TOTEE.EXE");

		if (!file.isOpen())
			error("Failed to open TOTEE.EXE");

		loadMessagesFixedSize(&file, 0x710f, 16, 20);
		_soundFx = loadSoundsFxDOS(&file, 0xd670, 5);
		_sound = loadSpeakerFxDOS(&file, 0x7396 + 0x200, 0x72a1 + 0x200, 20);
		loadFonts(&file, 0xd403);
		load8bitBinary(&file, 0x3ce0, 16);

		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);

		loadHeartFramesDOS(&file, 0x76AB, 0x76FD);

		_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

		for (auto &it : _indicators)
			it->convertToInPlace(_gfx->_texturePixelFormat);

	} else if (_renderMode == Common::kRenderCGA) {
		file.open("SCN1C.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kCGAPaletteRedGreenBright, 0, 4);
		}
		file.close();
		file.open("TOTEC.EXE");

		if (!file.isOpen())
			error("Failed to open TOTEC.EXE");

		loadMessagesFixedSize(&file, 0x594f, 16, 20);
		_soundFx = loadSoundsFxDOS(&file, 0xb9f0, 5);
		_sound = loadSpeakerFxDOS(&file, 0x5BD6 + 0x200, 0x5AE1 + 0x200, 20);
		loadFonts(&file, 0xb785);
		load8bitBinary(&file, 0x2530, 4);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kCGAPaletteRedGreen, 0, 4);
		loadAnkhCollectedMask();
		loadHeartFramesDOS(&file, 0x5F52, 0x5F84);
		swapPalette(_startArea);
		updateHeartFrames(_gfx->_palette);

		loadAnkhIndicatorMask();
		updateAnkhIndicator(_gfx->_palette);
	} else if (_renderMode == Common::kRenderHercG) {
		file.open("SCN1H.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kHerculesPaletteGreen, 0, 2);
		}
		file.close();
		file.open("TOTEH.EXE");

		if (!file.isOpen())
			error("Failed to open TOTEH.EXE");

		loadMessagesFixedSize(&file, 0x688b, 16, 20);
		_soundFx = loadSoundsFxDOS(&file, 0xc880, 5);
		_sound = loadSpeakerFxDOS(&file, 0x6b12 + 0x200, 0x6a1d + 0x200, 20);
		loadFonts(&file, 0xc609);
		load8bitBinary(&file, 0x3340, 4);

		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kHerculesPaletteGreen, 0, 2);

		loadAnkhCollectedMask();
		loadHeartFramesDOS(&file, 0x6E8E, 0x6EC0);

		// The Hercules palette never changes, so build the ankhs once
		loadAnkhIndicatorMask();
		updateAnkhIndicator((byte *)&kHerculesPaletteGreen);
	} else
		error("Invalid or unsupported render mode %s for Total Eclipse", Common::getRenderModeDescription(_renderMode));

	if (ConfMan.getBool("opl_music"))
		_playerMusic = new EclipseOPLMusicPlayer();
}

void EclipseEngine::drawDOSUI(Graphics::Surface *surface) {
	int score = _gameStateVars[k8bitVariableScore];
	int shield = _gameStateVars[k8bitVariableShield] * 100 / _maxShield;
	shield = shield < 0 ? 0 : shield;

	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	//uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0x00, 0x00);
	uint32 blue = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x55, 0x55, 0xFF);
	uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x55, 0xFF, 0x55);
	uint32 redish = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0x55, 0x55);
	//uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	uint32 pink = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0x55, 0xFF);
	uint32 cyan = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x55, 0xFF, 0xFF);

	uint32 color1, color2, color3;

    bool isCGAAltPalette = (_renderMode == Common::kRenderCGA && _currentArea && (_currentArea->_extraColor[0] & 0x01));

    if (_renderMode == Common::kRenderHercG) {
        uint8 r, g, b;
        _gfx->readFromPalette(1, r, g, b);
        color1 = color2 = color3 = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
    } else if (_renderMode == Common::kRenderEGA || isCGAAltPalette) {
        color1 = green;
        color2 = redish;
        color3 = yellow;
    } else {
        color1 = cyan;
        color2 = pink;
        color3 = white;
    }

	bool isHercules = _renderMode == Common::kRenderHercG;
	int textRow = isHercules ? 243 : 135;

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, isHercules ? 240 : 102, textRow, black, color3, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, isHercules ? 240 : 102, textRow, black, color3, surface);

	if (_renderMode == Common::kRenderEGA)
		drawScoreString(score, 136, 6, black, white, surface);
	else if (_renderMode == Common::kRenderCGA)
		drawScoreString(score, 136, 6, black, color2, surface);
	else if (isHercules)
		drawScoreString(score, 304, 54, black, color2, surface);

	// The Hercules build does not print the shield percentage on the heart
	if (!isHercules) {
		int x = 171;
		if (shield < 10)
			x = 179;
		else if (shield < 100)
			x = 175;

		Common::String shieldStr = Common::String::format("%d", shield);
		drawStringInSurface(shieldStr, x, 162, black, color2, surface);
	}

	drawStringInSurface(shiftStr("0", 'Z' - '$' + 1 - _angleRotationIndex), isHercules ? 192 : 79, textRow, black, color3, surface);
	drawStringInSurface(shiftStr("3", 'Z' - '$' + 1 - _playerStepIndex), isHercules ? 160 : 63, textRow, black, color3, surface);
	// The height is a pair of glyphs, which the shoot indicator below overwrites
	drawStringInSurface(shiftStr("67", 'Z' - '$' + 1 - _playerHeightNumber), isHercules ? 496 : 232, textRow, black, color3, surface);

	if (_shootingFrames > 0) {
		drawStringInSurface(shiftStr("4", 'Z' - '$' + 1), isHercules ? 496 : 232, textRow, black, color3, surface);
		drawStringInSurface(shiftStr("<", 'Z' - '$' + 1), isHercules ? 512 : 240, textRow, black, color3, surface);
	}
	if (isHercules)
		drawAnalogClock(surface, 212, 280, black, black, color1);
	else
		drawAnalogClock(surface, 90, 172, black, redish, white);

	Common::Rect jarBackground = isHercules ? Common::Rect(280, 273, 328, 301) : Common::Rect(124, 165, 148, 192);
	surface->fillRect(jarBackground, black);

	Common::Rect jarWater = isHercules
		? Common::Rect(280, 300 - _gameStateVars[k8bitVariableEnergy], 328, 301)
		: Common::Rect(124, 192 - _gameStateVars[k8bitVariableEnergy], 148, 192);

	if (isHercules)
		drawIndicator(surface, 122, 52, 24);
	else if (_renderMode == Common::kRenderCGA)
		drawIndicator(surface, 45, 4, 12);
	else
		drawIndicator(surface, 41, 4, 16);
	drawHeartIndicator(surface, isHercules ? 384 : 176, isHercules ? 276 : 168);
	if (_renderMode == Common::kRenderEGA) {
		surface->fillRect(jarWater, blue);
		drawEclipseIndicator(surface, 228, 0, color3, color1);
		surface->fillRect(Common::Rect(225, 168, 235, 187), white);
	}
	else if (_renderMode == Common::kRenderCGA) {
		surface->fillRect(jarWater, color1);
		drawEclipseIndicator(surface, 228, 0, color3, color2, color1);
		surface->fillRect(Common::Rect(225, 168, 235, 187), color3);
	}
	else if (isHercules) {
		// Hercules has no second ink, so the water level is a 50% stipple
		for (int wy = jarWater.top; wy < jarWater.bottom; wy++)
			for (int wx = jarWater.left + 1; wx < jarWater.right; wx += 2)
				surface->setPixel(wx, wy, color1);
		drawEclipseIndicator(surface, 488, 48, color1, color1, black);
		surface->fillRect(Common::Rect(482, 276, 502, 296), color3);
	}
	drawCompass(surface, isHercules ? 490 : 229, isHercules ? 286 : 177, _yaw, isHercules ? 20 : 10, black);
}

void EclipseEngine::playSoundFx(int index, bool sync, Sound::Type type) {
	if (_soundFx)
		_soundFx->playSound(index, type);
	else
		playSound(index, sync, type);
}


} // End of namespace Freescape
