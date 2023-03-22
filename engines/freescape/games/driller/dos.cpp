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
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

extern byte kCGAPalettePinkBlueWhiteData[4][3];
extern byte kEGADefaultPaletteData[16][3];

void DrillerEngine::initDOS() {
	if (_renderMode == Common::kRenderEGA)
		_viewArea = Common::Rect(40, 16, 280, 117);
	else if (_renderMode == Common::kRenderCGA)
		_viewArea = Common::Rect(36, 16, 284, 117);
	else
		error("Invalid or unknown render mode");

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

void DrillerEngine::renderPixels8bitTitleImage(Graphics::ManagedSurface *surface, int &i, int &j, int pixels) {
	int c1 = pixels >> 4;
	int c2 = pixels & 0xf;

	if (i == 320) {
		return;
	}

	if (_renderMode == Common::kRenderCGA) {
		surface->setPixel(i, j, getPixel8bitTitleImage(c1 / 4));
		i++;
		if (i == 320) {
			return;
		}
	}

	surface->setPixel(i, j, getPixel8bitTitleImage(c1));
	i++;

	if (i == 320) {
		return;
	}

	if (_renderMode == Common::kRenderCGA) {
		surface->setPixel(i, j, getPixel8bitTitleImage(c2 / 4));
		i++;

		if (i == 320) {
			return;
		}
	}

	surface->setPixel(i, j, getPixel8bitTitleImage(c2));
	i++;
}

Graphics::ManagedSurface *DrillerEngine::load8bitTitleImage(Common::SeekableReadStream *file, int offset) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(_screenW, _screenH, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, 320, 200), 0);

	int i = 0;
	int j = 0;
	int command = -1;
	int singlePixelsToProcess = 0;
	bool repeatedPixelsToProcess = false;
	file->seek(offset);
	while (!file->eos()) {
		assert(i <= 320);
		int pixels = -1;
		int repetition = -1;

		if (singlePixelsToProcess == 0 && !repeatedPixelsToProcess) {
			if (command < 0)
				command = file->readByte();

			//debug("reading command: %x at %lx", command, file->pos() - 1);

			assert(command >= 0x7f);
			singlePixelsToProcess = (0xff - command + 2) * 2;
			//debug("single Pixels to process: %d", singlePixelsToProcess);

			repeatedPixelsToProcess = true;
			if (i == 320) {
				j++;
				i = 0;
			}
			command = -1;
			continue;
		}

		if (singlePixelsToProcess > 0) {
			singlePixelsToProcess--;
			pixels = file->readByte();
			//debug("reading pixels: %x at %d, %d", pixels, i, j);
			renderPixels8bitTitleImage(surface, i, j, pixels);
		} else if (repeatedPixelsToProcess) {
			repetition = file->readByte() + 1;
			//debug("reading repetition: %x", repetition - 1);
			assert(repetition > 0);
			if (repetition >= 0x80) {
				command = repetition - 1;
				repeatedPixelsToProcess = false;
				continue;
			}

			if (i == 320) {
				j++;
				i = 0;
				continue;
			}

			int pixels1 = file->readByte();
			//debug("reading pixels: %x", pixels1);

			int pixels2 = file->readByte();
			//debug("reading pixels: %x", pixels2);

			if (repetition >= 1) {
				while (repetition > 0) {
					repetition--;

					if (i == 320) {
						j++;
						i = 0;
					}

					if (j == 200)
						return surface;

					//sdebug("repeating pixels: %x at %d, %d", pixels1, i, j);
					renderPixels8bitTitleImage(surface, i, j, pixels1);

					if (i == 320) {
						j++;
						i = 0;
					}

					if (j == 200)
						return surface;

					//debug("repeating pixels: %x at %d, %d", pixels2, i, j);
					renderPixels8bitTitleImage(surface, i, j, pixels2);
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

byte kEGADefaultPaletteData[16][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0xaa},
	{0x00, 0xaa, 0x00},
	{0xaa, 0x00, 0x00},
	{0xaa, 0x00, 0xaa},
	{0xaa, 0x55, 0x00},
	{0x55, 0xff, 0x55},
	{0xff, 0x55, 0x55},
	{0x12, 0x34, 0x56},
	{0xff, 0xff, 0x55},
	{0xff, 0xff, 0xff},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00}
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
			_title->setPalette((byte*)&kEGADefaultPaletteData, 0, 16);
		}
		file.close();
		file.open("EGATITLE.RL");
		if (file.isOpen()) {
			_title = load8bitTitleImage(&file, 0x1b3);
			_title->setPalette((byte*)&kEGADefaultPaletteData, 0, 16);
		}
		file.close();

		file.open("DRILLE.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLE.EXE");

		loadMessagesFixedSize(&file, 0x4135, 14, 20);
		loadFonts(&file, 0x99dd);
		loadGlobalObjects(&file, 0x3b42);
		load8bitBinary(&file, 0x9b40, 16);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte*)&kEGADefaultPaletteData, 0, 16);
	} else if (_renderMode == Common::kRenderCGA) {
		file.open("SCN1C.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte*)&kCGAPalettePinkBlueWhiteData, 0, 4);
		}
		file.close();
		file.open("CGATITLE.RL");
		if (file.isOpen()) {
			_title = load8bitTitleImage(&file, 0x1b3);
			_title->setPalette((byte*)&kCGAPalettePinkBlueWhiteData, 0, 4);
		}
		file.close();
		file.open("DRILLC.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLC.EXE");

		loadFonts(&file, 0x07a4a);
		loadMessagesFixedSize(&file, 0x2585, 14, 20);
		load8bitBinary(&file, 0x7bb0, 4);
		loadGlobalObjects(&file, 0x1fa2);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte*)&kCGAPalettePinkBlueWhiteData, 0, 4);
	} else
		error("Unsupported video mode for DOS");
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
	loadGlobalObjects(&file, 0x8c);
	_border = load8bitDemoImage(&file, 0x6220);
	_border->setPalette((byte*)&kCGAPalettePinkBlueWhiteData, 0, 4);

	// Fixed for a corrupted area names in the demo data
	_areaMap[2]->_name = "LAPIS LAZULI";
	_areaMap[3]->_name = "EMERALD";
	_areaMap[8]->_name = "TOPAZ";
	file.close();
}

void DrillerEngine::drawDOSUI(Graphics::Surface *surface) {
	uint32 color = _renderMode == Common::kRenderCGA ? 1 : 14;
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
	drawStringInSurface(_currentArea->_name, 196, 185, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 150, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 150, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 150, 161, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 57, 161, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 57, 161, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 46, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), _renderMode == Common::kRenderCGA ? 44 : 46, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 238, 129, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 208, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 230, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 8, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 190, 177, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 191, 177, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

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

} // End of namespace Freescape