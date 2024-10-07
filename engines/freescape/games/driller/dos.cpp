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

#include "freescape/freescape.h"
#include "freescape/games/driller/driller.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

extern byte kCGAPalettePinkBlueWhiteData[4][3];
extern byte kEGADefaultPalette[16][3];
extern byte kHerculesPaletteGreen[2][3];

byte kDrillerCGAPalettePinkBlue[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0xaa, 0xaa},
	{0xaa, 0x00, 0xaa},
	{0xaa, 0xaa, 0xaa},
};

byte kDrillerCGAPaletteRedGreen[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0xaa, 0x00},
	{0xaa, 0x00, 0x00},
	{0xaa, 0x55, 0x00},
};

static const CGAPaletteEntry rawCGAPaletteByArea[] {
	{1, (byte *)kDrillerCGAPaletteRedGreen},
	{2, (byte *)kDrillerCGAPalettePinkBlue},
	{3, (byte *)kDrillerCGAPaletteRedGreen},
	{4, (byte *)kDrillerCGAPalettePinkBlue},
	{5, (byte *)kDrillerCGAPaletteRedGreen},
	{6, (byte *)kDrillerCGAPalettePinkBlue},
	{7, (byte *)kDrillerCGAPaletteRedGreen},
	{8, (byte *)kDrillerCGAPalettePinkBlue},
	{9, (byte *)kDrillerCGAPaletteRedGreen},
	{10, (byte *)kDrillerCGAPalettePinkBlue},
	{11, (byte *)kDrillerCGAPaletteRedGreen},
	{12, (byte *)kDrillerCGAPalettePinkBlue},
	{13, (byte *)kDrillerCGAPaletteRedGreen},
	{14, (byte *)kDrillerCGAPalettePinkBlue},
	{15, (byte *)kDrillerCGAPaletteRedGreen},
	{16, (byte *)kDrillerCGAPalettePinkBlue},
	{17, (byte *)kDrillerCGAPalettePinkBlue},
	{18, (byte *)kDrillerCGAPalettePinkBlue},
	{19, (byte *)kDrillerCGAPaletteRedGreen},
	{20, (byte *)kDrillerCGAPalettePinkBlue},
	{21, (byte *)kDrillerCGAPaletteRedGreen},
	{22, (byte *)kDrillerCGAPalettePinkBlue},
	{23, (byte *)kDrillerCGAPaletteRedGreen},
	{25, (byte *)kDrillerCGAPalettePinkBlue},
	{27, (byte *)kDrillerCGAPaletteRedGreen},
	{28, (byte *)kDrillerCGAPalettePinkBlue},

	{31, (byte *)kDrillerCGAPaletteRedGreen},
	{32, (byte *)kDrillerCGAPalettePinkBlue},
	{127, (byte *)kDrillerCGAPaletteRedGreen},
	{0, 0}   // This marks the end
};

void DrillerEngine::initDOS() {
	if (_renderMode == Common::kRenderEGA)
		_viewArea = Common::Rect(40, 16, 280, 117);
	else if (_renderMode == Common::kRenderHercG)
		_viewArea = Common::Rect(112, 64, 607, 224);
	else if (_renderMode == Common::kRenderCGA)
		_viewArea = Common::Rect(36, 16, 284, 117);
	else
		error("Invalid or unknown render mode");

	_rawCGAPaletteByArea = (const CGAPaletteEntry *)&rawCGAPaletteByArea;
	_moveFowardArea = Common::Rect(73, 144, 101, 152);
	_moveLeftArea = Common::Rect(73, 150, 86, 159);
	_moveRightArea = Common::Rect(88, 152, 104, 160);
	_moveBackArea = Common::Rect(73, 160, 101, 168);
	_moveUpArea = Common::Rect(219, 144, 243, 155);
	_moveDownArea = Common::Rect(219, 157, 243, 167);
	_deployDrillArea = Common::Rect(140, 175, 179, 191);
	_infoScreenArea = Common::Rect(130, 125, 188, 144);
}

/*
 The following functions are only used for decoding title images for
 the US release of Driller ("Space Station Oblivion")
*/

uint32 DrillerEngine::getPixel8bitTitleImage(int index) {
	if (index < 4 || _renderMode == Common::kRenderEGA) {
		return index;
	}
	return index / 4;
}

void DrillerEngine::renderPixels8bitTitleImage(Graphics::ManagedSurface *surface, int &x, int &y, int pixels) {
	int c1 = pixels >> 4;
	int c2 = pixels & 0xf;

	if (x == 320) {
		return;
	}

	if (_renderMode == Common::kRenderCGA) {
		surface->setPixel(x, y, getPixel8bitTitleImage(c1 / 4));
		x++;
		if (x == 320) {
			return;
		}
	}

	surface->setPixel(x, y, getPixel8bitTitleImage(c1));
	x++;

	if (x == 320) {
		return;
	}

	if (_renderMode == Common::kRenderCGA) {
		surface->setPixel(x, y, getPixel8bitTitleImage(c2 / 4));
		x++;

		if (x == 320) {
			return;
		}
	}

	surface->setPixel(x, y, getPixel8bitTitleImage(c2));
	x++;
}

Graphics::ManagedSurface *DrillerEngine::load8bitTitleImage(Common::SeekableReadStream *file, int offset) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(_screenW, _screenH, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, 320, 200), 0);

	file->seek(offset);
	for (int y = 0; y < 200; ++y) {
		if (file->eos ()) break;
		
		// Start of line data (0x02) or [premature] end of data (0x00)
		int sol = file->readByte();
		if (sol == 0) break;
		assert(sol == 2);
		
		int x = 0;
		while (x < 320) {
			int command = file->readByte();
			if (command & 0x80) {
				// Copy 2*N bytes verbatim
				int repeat = (257 - command) * 2;
				for (int i = 0; i < repeat; ++i) {
					int pixels = file->readByte();
					renderPixels8bitTitleImage(surface, x, y, pixels);
				}
			} else {
				// Repeat 2 bytes of the input N times
				int repeat = command + 1;
				int pixels1 = file->readByte();
				int pixels2 = file->readByte();
				for (int i = 0; i < repeat; ++i) {
					renderPixels8bitTitleImage(surface, x, y, pixels1);
					renderPixels8bitTitleImage(surface, x, y, pixels2);
				}
			}
		}
	}
	return surface;
}

byte kCGAPalettePinkBlueWhiteData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x55, 0xff, 0xff},
	{0xff, 0x55, 0xff},
	{0xff, 0xff, 0xff},
};

/*
 The following function is only used for decoding images for
 the Driller DOS demo
*/

Graphics::ManagedSurface *DrillerEngine::load8bitDemoImage(Common::SeekableReadStream *file, int offset) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, 320, 200), 0);
	file->seek(offset);
	int i = 0;
	int j = 0;
	while (true) {
		byte pixels = file->readByte();
		for (int b = 0; b < 4; b++) {
			int color = pixels & 3;
			pixels = pixels >> 2;
			surface->setPixel(i + (3 - b), 2 * j, color);
		}
		i = i + 4;
		if (i == 320) {
			i = 0;
			j++;
		}
		if (j == 100)
			break;
	}
	file->seek(0xc0, SEEK_CUR);

	i = 0;
	j = 0;
	while (true) {
		byte pixels = file->readByte();
		for (int b = 0; b < 4; b++) {
			int color = pixels & 3;
			pixels = pixels >> 2;
			surface->setPixel(i + (3 - b), 2 * j + 1, color);
		}
		i = i + 4;
		if (i == 320) {
			i = 0;
			j++;
		}
		if (j == 100)
			break;
	}
	return surface;
}

void DrillerEngine::loadAssetsDOSFullGame() {
	Common::File file;
	if (_renderMode == Common::kRenderEGA) {
		file.open("SCN1E.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte*)&kEGADefaultPalette, 0, 16);
		}
		file.close();
		file.open("EGATITLE.RL");
		if (file.isOpen()) {
			_title = load8bitTitleImage(&file, 0x1b2);
			_title->setPalette((byte*)&kEGADefaultPalette, 0, 16);
		}
		file.close();

		file.open("DRILLE.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLE.EXE");

		loadSpeakerFxDOS(&file, 0x4397 + 0x200, 0x4324 + 0x200);
		loadMessagesFixedSize(&file, 0x4135, 14, 20);
		loadFonts(&file, 0x99dd);
		loadGlobalObjects(&file, 0x3b42, 8);
		load8bitBinary(&file, 0x9b40, 16);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte*)&kEGADefaultPalette, 0, 16);
	} else if (_renderMode == Common::kRenderCGA) {
		file.open("SCN1C.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte*)&kCGAPalettePinkBlueWhiteData, 0, 4);
		}
		file.close();
		file.open("CGATITLE.RL");
		if (file.isOpen()) {
			_title = load8bitTitleImage(&file, 0x1b2);
			_title->setPalette((byte*)&kCGAPalettePinkBlueWhiteData, 0, 4);
		}
		file.close();
		file.open("DRILLC.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLC.EXE");

		loadSpeakerFxDOS(&file, 0x27e7 + 0x200, 0x2774 + 0x200);

		loadFonts(&file, 0x07a4a);
		loadMessagesFixedSize(&file, 0x2585, 14, 20);
		load8bitBinary(&file, 0x7bb0, 4);
		loadGlobalObjects(&file, 0x1fa2, 8);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte*)&kCGAPalettePinkBlueWhiteData, 0, 4);
		swapPalette(1);
	} else if (_renderMode == Common::kRenderHercG) {
		file.open("SCN1H.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte*)&kHerculesPaletteGreen, 0, 2);
		}
		file.close();
		file.open("DRILLH.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLH.EXE");

		//loadSpeakerFxDOS(&file, 0x27e7 + 0x200, 0x2774 + 0x200);

		loadFonts(&file, 0x8871);
		loadMessagesFixedSize(&file, 0x3411, 14, 20);
		load8bitBinary(&file, 0x89e0, 4);
		loadGlobalObjects(&file, 0x2d02, 8);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte*)&kHerculesPaletteGreen, 0, 2);
	} else
		error("Unsupported video mode for DOS");

	if (_renderMode != Common::kRenderHercG) {
		_indicators.push_back(loadBundledImage("driller_tank_indicator"));
		_indicators.push_back(loadBundledImage("driller_ship_indicator"));

		_indicators[0]->convertToInPlace(_gfx->_texturePixelFormat);
		_indicators[1]->convertToInPlace(_gfx->_texturePixelFormat);
	}
}

void DrillerEngine::loadAssetsDOSDemo() {
	Common::File file;
	_renderMode = Common::kRenderCGA; // DOS demos is CGA only
	_viewArea = Common::Rect(36, 16, 284, 117); // correct view area
	_gfx->_renderMode = _renderMode;
	file.open("d1");
	if (!file.isOpen())
		error("Failed to open 'd1' file");

	_title = load8bitDemoImage(&file, 0x0);
	_title->setPalette((byte*)&kCGAPalettePinkBlueWhiteData, 0, 4);

	file.close();
	file.open("d2");
	if (!file.isOpen())
		error("Failed to open 'd2' file");

	loadFonts(&file, 0x4eb0);
	loadMessagesFixedSize(&file, 0x636, 14, 20);
	load8bitBinary(&file, 0x55b0, 4);
	loadGlobalObjects(&file, 0x8c, 5);
	_border = load8bitDemoImage(&file, 0x6220);
	_border->setPalette((byte*)&kCGAPalettePinkBlueWhiteData, 0, 4);

	// Fixes corrupted area names in the demo data
	_areaMap[2]->_name = "LAPIS LAZULI";
	_areaMap[3]->_name = "EMERALD";
	_areaMap[8]->_name = "TOPAZ";
	file.close();

	_indicators.push_back(loadBundledImage("driller_tank_indicator"));
	_indicators.push_back(loadBundledImage("driller_ship_indicator"));

	_indicators[0]->convertToInPlace(_gfx->_texturePixelFormat);
	_indicators[1]->convertToInPlace(_gfx->_texturePixelFormat);
}

void DrillerEngine::drawDOSUI(Graphics::Surface *surface) {
	uint32 color = _renderMode == Common::kRenderCGA || _renderMode == Common::kRenderHercG ? 1 : 14;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	Common::Point currentAreaPos = _renderMode == Common::kRenderHercG ? Common::Point(437, 293) : Common::Point(197, 185);
	drawStringInSurface(_currentArea->_name, currentAreaPos.x, currentAreaPos.y, front, back, surface);

	Common::Point coordinateXPos = _renderMode == Common::kRenderHercG ? Common::Point(345, 253) : Common::Point(151, 145);
	Common::Point coordinateYPos = _renderMode == Common::kRenderHercG ? Common::Point(345, 261) : Common::Point(151, 153);
	Common::Point coordinateZPos = _renderMode == Common::kRenderHercG ? Common::Point(345, 269) : Common::Point(151, 161);

	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), coordinateXPos.x, coordinateXPos.y, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), coordinateYPos.x, coordinateYPos.y, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), coordinateZPos.x, coordinateZPos.y, front, back, surface);

	Common::Point playerHeightPos = _renderMode == Common::kRenderHercG ? Common::Point(157, 269) : Common::Point(57, 161);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), playerHeightPos.x, playerHeightPos.y, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), playerHeightPos.x, playerHeightPos.y, front, back, surface);

	Common::Point anglePos = _renderMode == Common::kRenderHercG ? Common::Point(141, 253) : Common::Point(47, 145);
	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), anglePos.x, anglePos.y, front, back, surface);

	Common::Point playerStepsPos;

	if (_renderMode == Common::kRenderHercG)
		playerStepsPos = Common::Point(130, 261);
	else if (_renderMode == Common::kRenderCGA)
		playerStepsPos = Common::Point(44, 153);
	else
		playerStepsPos = Common::Point(47, 153);

	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), playerStepsPos.x, playerStepsPos.y, front, back, surface);

	Common::Point scorePos = _renderMode == Common::kRenderHercG ? Common::Point(522, 237) : Common::Point(238, 129);
	drawStringInSurface(Common::String::format("%07d", score), scorePos.x, scorePos.y, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);

	Common::Point hoursPos = _renderMode == Common::kRenderHercG ? Common::Point(462, 56) : Common::Point(208, 8);
	drawStringInSurface(Common::String::format("%02d", hours), hoursPos.x, hoursPos.y, front, back, surface);

	Common::Point minutesPos = _renderMode == Common::kRenderHercG ? Common::Point(506, 56) : Common::Point(231, 8);
	drawStringInSurface(Common::String::format("%02d", minutes), minutesPos.x, minutesPos.y, front, back, surface);

	Common::Point secondsPos = _renderMode == Common::kRenderHercG ? Common::Point(554, 56) : Common::Point(255, 8);
	drawStringInSurface(Common::String::format("%02d", seconds), secondsPos.x, secondsPos.y, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	Common::Point messagePos = _renderMode == Common::kRenderHercG ? Common::Point(424, 285) : Common::Point(191, 177);
	if (deadline <= _countdown) {
		drawStringInSurface(message, messagePos.x, messagePos.y, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, messagePos.x, messagePos.y, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (_renderMode != Common::kRenderHercG) {
		if (energy >= 0) {
			Common::Rect backBar(20, 185, 88 - energy, 191);
			surface->fillRect(backBar, back);
			Common::Rect energyBar(87 - energy, 185, 88, 191);
			surface->fillRect(energyBar, front);
		}

		if (shield >= 0) {
			Common::Rect backBar(20, 177, 88 - shield, 183);
			surface->fillRect(backBar, back);

			Common::Rect shieldBar(87 - shield, 177, 88, 183);
			surface->fillRect(shieldBar, front);
		}
	}

	if (_indicators.size() >= 2) {
		if (!_flyMode)
			surface->copyRectToSurface(*_indicators[0], 132, 128, Common::Rect(_indicators[0]->w, _indicators[0]->h));
		else
			surface->copyRectToSurface(*_indicators[1], 132, 128, Common::Rect(_indicators[1]->w, _indicators[1]->h));
	}

	color = _renderMode == Common::kRenderHercG ? 1 : 2;
	_gfx->readFromPalette(color, r, g, b);
	uint32 other = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Point compassYawPos = _renderMode == Common::kRenderHercG ? Common::Point(214, 264) : Common::Point(87, 156);
	drawCompass(surface, compassYawPos.x, compassYawPos.y, _yaw, 10, other);
	Common::Point compassPitchPos = _renderMode == Common::kRenderHercG ? Common::Point(502, 264) : Common::Point(230, 156);
	drawCompass(surface, compassPitchPos.x, compassPitchPos.y, _pitch - 30, 10, other);
}

} // End of namespace Freescape
