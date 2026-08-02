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

#include "common/config-manager.h"
#include "common/file.h"

#include "freescape/freescape.h"
#include "freescape/games/dark/dark.h"
#include "freescape/games/dark/opl.music.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void DarkEngine::initDOS() {
	if (_renderMode == Common::kRenderEGA)
		_viewArea = Common::Rect(40, 24, 280, 125);
	else if (_renderMode == Common::kRenderCGA)
		_viewArea = Common::Rect(40, 24, 280, 125);
	else if (_renderMode == Common::kRenderHercG)
		_viewArea = Common::Rect(112, 72, 592, 232);
	else
		error("Invalid or unknown render mode");

	_maxEnergy = 79;
	_maxShield = 79;
}

// The DOS executables store posture sprites as packed bitmaps after a
// height/byte-width header. Offset them four rows within the HUD slot.
void DarkEngine::loadIndicatorsDOS(Common::SeekableReadStream *file) {
	const int cgaOffsets[4] = { 0x350a, 0x3490, 0x3416, 0x3395 };
	const int herculesOffsets[4] = { 0x4448, 0x43ce, 0x4354, 0x42d3 };
	const int *offsets = _renderMode == Common::kRenderHercG ? herculesOffsets : cgaOffsets;
	const int bitsPerPixel = _renderMode == Common::kRenderHercG ? 1 : 2;
	const int pixelsPerByte = 8 / bitsPerPixel;
	const int colorMask = (1 << bitsPerPixel) - 1;

	for (int i = 0; i < 4; i++) {
		file->seek(offsets[i]);
		int height = file->readByte();
		int widthBytes = file->readByte();

		auto *indexed = new Graphics::ManagedSurface();
		indexed->create(widthBytes * pixelsPerByte, height + 4, Graphics::PixelFormat::createFormatCLUT8());
		indexed->fillRect(Common::Rect(0, 0, indexed->w, indexed->h), 0);

		for (int y = 0; y < height; y++)
			for (int col = 0; col < widthBytes; col++) {
				byte b = file->readByte();
				for (int px = 0; px < pixelsPerByte; px++)
					indexed->setPixel(col * pixelsPerByte + px, y + 4, (b >> (8 - (px + 1) * bitsPerPixel)) & colorMask);
			}

		_indicatorsIndexed.push_back(indexed);
	}
}

void DarkEngine::updateIndicatorsDOS(const byte *palette) {
	for (auto &it : _indicators) {
		it->free();
		delete it;
	}
	_indicators.clear();

	uint32 colors[4];
	int colorCount = _renderMode == Common::kRenderHercG ? 2 : 4;
	for (int i = 0; i < colorCount; i++)
		colors[i] = _gfx->_texturePixelFormat.ARGBToColor(0xFF, palette[3 * i], palette[3 * i + 1], palette[3 * i + 2]);

	for (auto &indexed : _indicatorsIndexed) {
		Graphics::Surface *surface = new Graphics::Surface();
		surface->create(indexed->w, indexed->h, _gfx->_texturePixelFormat);

		for (int y = 0; y < surface->h; y++)
			for (int x = 0; x < surface->w; x++)
				surface->setPixel(x, y, colors[indexed->getPixel(x, y) & (colorCount - 1)]);

		_indicators.push_back(surface);
	}
}

void DarkEngine::loadAssetsDOSDemo() {
	Common::File file;
	if (_renderMode == Common::kRenderEGA) {
		file.open("SCN1E.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		}
		file.close();
		file.open("DSIDEE.EXE");

		if (!file.isOpen())
			error("Failed to open DSIDEE.EXE");

		_sound = loadSpeakerFxDOS(&file, 0x4837 + 0x200, 0x46e8 + 0x200, 20);
		loadMessagesFixedSize(&file, 0x4525, 16, 27);
		loadMessagesFixedSize(&file, 0x993f - 2, 308, 5);
		loadFonts(&file, 0xa598);
		loadGlobalObjects(&file, 0x3d04, 23);
		load8bitBinary(&file, 0xa700, 16);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);

		_indicators.push_back(loadBundledImage("dark_fallen_indicator"));
		_indicators.push_back(loadBundledImage("dark_crouch_indicator"));
		_indicators.push_back(loadBundledImage("dark_walk_indicator"));
		_indicators.push_back(loadBundledImage("dark_jet_indicator"));

		for (auto &it : _indicators)
			it->convertToInPlace(_gfx->_texturePixelFormat);

	} else if (_renderMode == Common::kRenderCGA) {
		file.open("SCN1C.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kCGAPalettePinkBlue, 0, 4);
		}
		file.close();
		file.open("DSIDEC.EXE");

		if (!file.isOpen())
			error("Failed to open DSIDEC.EXE");

		_sound = loadSpeakerFxDOS(&file, 0x3077 + 0x200, 0x2f28 + 0x200, 20);
		loadFonts(&file, 0x8907);
		loadMessagesFixedSize(&file, 0x2d65, 16, 27);
		loadMessagesFixedSize(&file, 0x7c3a, 308, 5);
		loadGlobalObjects(&file, 0x2554, 23);
		load8bitBinary(&file, 0x8a70, 4);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kCGAPalettePinkBlue, 0, 4);

		swapPalette(1);
	} else
		error("Invalid or unsupported render mode %s for Dark Side", Common::getRenderModeDescription(_renderMode));
}

void DarkEngine::loadAssetsDOSFullGame() {
	Common::File file;
	if (_renderMode == Common::kRenderEGA) {
		file.open("SCN1E.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		}
		file.close();
		file.open("DSIDEE.EXE");

		if (!file.isOpen())
			error("Failed to open DSIDEE.EXE");

		_sound = loadSpeakerFxDOS(&file, 0x4837 + 0x200, 0x46e8 + 0x200, 20);
		loadFonts(&file, 0xa113);
		loadMessagesFixedSize(&file, 0x4525, 16, 27);
		loadGlobalObjects(&file, 0x3d04, 23);
		load8bitBinary(&file, 0xa280, 16);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);

		_indicators.push_back(loadBundledImage("dark_fallen_indicator"));
		_indicators.push_back(loadBundledImage("dark_crouch_indicator"));
		_indicators.push_back(loadBundledImage("dark_walk_indicator"));
		_indicators.push_back(loadBundledImage("dark_jet_indicator"));

		for (auto &it : _indicators)
			it->convertToInPlace(_gfx->_texturePixelFormat);

	} else if (_renderMode == Common::kRenderCGA) {
		file.open("SCN1C.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kCGAPalettePinkBlue, 0, 4);
		}
		file.close();
		file.open("DSIDEC.EXE");

		if (!file.isOpen())
			error("Failed to open DSIDEC.EXE");

		_sound = loadSpeakerFxDOS(&file, 0x3077 + 0x200, 0x2f28 + 0x200, 20);
		loadFonts(&file, 0x8496);
		loadMessagesFixedSize(&file, 0x2d65, 16, 27);
		loadGlobalObjects(&file, 0x2554, 23);
		load8bitBinary(&file, 0x8600, 16);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kCGAPalettePinkBlue, 0, 4);
		loadIndicatorsDOS(&file);

		swapPalette(1);
		updateIndicatorsDOS(_gfx->_palette);
	} else if (_renderMode == Common::kRenderHercG) {
		file.open("SCN1H.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kHerculesPaletteGreen, 0, 2);
		}
		file.close();
		file.open("DSIDEH.EXE");

		if (!file.isOpen())
			error("Failed to open DSIDEH.EXE");

		_sound = loadSpeakerFxDOS(&file, 0x3fb5 + 0x200, 0x3e66 + 0x200, 20);
		loadFonts(&file, 0x9328);
		loadMessagesFixedSize(&file, 0x3ca3, 16, 27);
		loadGlobalObjects(&file, 0x3364, 23);
		load8bitBinary(&file, 0x9490, 4);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kHerculesPaletteGreen, 0, 2);
		loadIndicatorsDOS(&file);
		updateIndicatorsDOS((byte *)&kHerculesPaletteGreen);
	} else
		error("Invalid or unsupported render mode %s for Dark Side", Common::getRenderModeDescription(_renderMode));

	if (ConfMan.getBool("opl_music")) {
		delete _playerMusic;
		_playerMusic = new DarkSideOPLMusicPlayer();
	}
}

void DarkEngine::drawDOSUI(Graphics::Surface *surface) {
	bool isHercules = _renderMode == Common::kRenderHercG;
	auto hudX = [isHercules](int x) { return isHercules ? 2 * x + 32 : x; };
	int lowerHudOffsetY = isHercules ? 108 : 0;
	int topHudOffsetY = isHercules ? 48 : 0;

	uint32 color = _renderMode == Common::kRenderCGA ? 3 : (isHercules ? 1 : 14);
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = isHercules ? 0 : _currentArea->_usualBackgroundColor;
	if (!isHercules && _gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	// Drawing the horizontal compass should be done first, so that the background is properly filled
	drawHorizontalCompass(hudX(200), 143 + lowerHudOffsetY, _yaw, front, back, surface);
	Common::Rect stepBackgroundRect = Common::Rect(hudX(69), 177 + lowerHudOffsetY, hudX(98), 185 + lowerHudOffsetY);
	surface->fillRect(stepBackgroundRect, back);

	Common::Rect positionBackgroundRect = Common::Rect(hudX(199), 135 + lowerHudOffsetY, hudX(232), 160 + lowerHudOffsetY);
	surface->fillRect(positionBackgroundRect, back);

	int score = _gameStateVars[k8bitVariableScore];
	int ecds = _gameStateVars[kVariableActiveECDs];
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), hudX(199), 137 + lowerHudOffsetY, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), hudX(199), 145 + lowerHudOffsetY, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), hudX(199), 153 + lowerHudOffsetY, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), hudX(71), 168 + lowerHudOffsetY, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), hudX(71), 177 + lowerHudOffsetY, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), hudX(95), 8 + topHudOffsetY, front, back, surface);
	drawStringInSurface(Common::String::format("%3d%%", ecds), hudX(192), 8 + topHudOffsetY, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, hudX(112), 177 + lowerHudOffsetY, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else
		drawStringInSurface(_currentArea->_name, hudX(112), 177 + lowerHudOffsetY, front, back, surface);

	int energy = _gameStateVars[k8bitVariableEnergy]; // called fuel in this game
	int shield = _gameStateVars[k8bitVariableShield];

	_gfx->readFromPalette(_renderMode == Common::kRenderEGA ? 9 : 1, r, g, b);
	uint32 blue = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	if (shield >= 0) {
		Common::Rect shieldBar;
		shieldBar = Common::Rect(hudX(72), 140 + lowerHudOffsetY, hudX(151 - (_maxShield - shield)), 141 + lowerHudOffsetY);
		surface->fillRect(shieldBar, blue);
		shieldBar = Common::Rect(hudX(72), 145 + lowerHudOffsetY, hudX(151 - (_maxShield - shield)), 146 + lowerHudOffsetY);
		surface->fillRect(shieldBar, blue);

		shieldBar = Common::Rect(hudX(72), 142 + lowerHudOffsetY, hudX(151 - (_maxShield - shield)), 144 + lowerHudOffsetY);
		surface->fillRect(shieldBar, front);
	}

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(hudX(72), 148 + lowerHudOffsetY, hudX(151 - (_maxEnergy - energy)), 149 + lowerHudOffsetY);
		surface->fillRect(energyBar, blue);
		energyBar = Common::Rect(hudX(72), 153 + lowerHudOffsetY, hudX(151 - (_maxEnergy - energy)), 154 + lowerHudOffsetY);
		surface->fillRect(energyBar, blue);

		energyBar = Common::Rect(hudX(72), 150 + lowerHudOffsetY, hudX(151 - (_maxEnergy - energy)), 152 + lowerHudOffsetY);
		surface->fillRect(energyBar, front);
	}
	uint32 clockColor = _renderMode == Common::kRenderEGA ? _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF) : front;
	drawBinaryClock(surface, hudX(300), isHercules ? 220 : 124, clockColor, back);
	drawIndicator(surface, hudX(160), 136 + lowerHudOffsetY);
	drawVerticalCompass(surface, hudX(24), isHercules ? 152 : 76, _pitch, blue);
}

} // End of namespace Freescape
