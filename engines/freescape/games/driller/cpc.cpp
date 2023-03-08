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
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

byte *parseEDSK(const Common::String filename, int &size) {
	debugC(1, kFreescapeDebugParser, "Trying to parse edsk file: %s", filename.c_str());
	Common::File file;
	file.open(filename);
	if (!file.isOpen())
		error("Failed to open %s", filename.c_str());

	int totalSize = file.size();
	byte *edskBuffer = (byte *)malloc(totalSize);
	file.read(edskBuffer, totalSize);
	file.close();

	// We don't know the final size, but we allocate enough
	byte *memBuffer = (byte *)malloc(totalSize);

	byte nsides = edskBuffer[49];
	assert(nsides == 1);
	int ntracks = 0;
	int i = 256;
	int j = 0;
	while (i + 1 < totalSize) {
		byte ssize = edskBuffer[i + 0x14];
		debugC(1, kFreescapeDebugParser, "i: %x ssize: %d, number: %d", i, ssize, edskBuffer[i + 0x10]);
		assert(ssize == 3 || edskBuffer[i + 0x0] == 'T');
		assert(ssize == 3 || edskBuffer[i + 0x1] == 'r');
		assert(ssize == 3 || edskBuffer[i + 0x2] == 'a');
		//assert(ssize == 3 || ntracks == edskBuffer[i + 0x10]);
		int start = i + 0x100;
		debugC(1, kFreescapeDebugParser, "sector size: %d", ssize);
		if (ssize == 2) {
			i = i + 9 * 512 + 256;
		} else if (ssize == 5) {
			i = i + 8 * 512 + 256;
		} else if (ssize == 0) {
			i = totalSize - 1;
		} else if (ssize == 3) {
			break; // Not sure
		} else {
			error("ssize: %d", ssize);
		}
		int osize = i - start;
		debugC(1, kFreescapeDebugParser, "copying track %d start: %x size: %x, dest: %x", ntracks, start, osize, j);
		memcpy(memBuffer + j, edskBuffer + start, osize);
		j = j + osize;
		ntracks++;
	}
	size = j;

	if (0) { // Useful to debug where exactly each object is located in memory once it is parsed
		i = 0;
		while(i < j) {
			debugN("%05x: ", i);
			for (int k = 0; k <= 16; k++) {
				debugN("%02x ", memBuffer[i]);
				i++;
			}
			debugN("\n");
		}
	}
	free(edskBuffer);
	return memBuffer;
}

void deobfuscateDrillerCPCVirtualWorlds(byte *memBuffer) {
	// Deofuscation / loader code
	for (int j = 0; j < 0x200; j++) {
		memBuffer[0x14000 + j] = memBuffer[0x14200 + j];
		memBuffer[0x14200 + j] = memBuffer[0x13400 + j];
		memBuffer[0x14400 + j] = memBuffer[0x13800 + j];
		memBuffer[0x14600 + j] = memBuffer[0x13c00 + j];
	}

	for (int j = 0; j < 0x200; j++) {
		memBuffer[0x13c00 + j] = memBuffer[0x13a00 + j];
		memBuffer[0x13a00 + j] = memBuffer[0x13600 + j];
		memBuffer[0x13800 + j] = memBuffer[0x13200 + j];
		memBuffer[0x13600 + j] = memBuffer[0x12e00 + j];
		memBuffer[0x12e00 + j] = memBuffer[0x13000 + j];
		memBuffer[0x13000 + j] = memBuffer[0x12200 + j];
		memBuffer[0x13200 + j] = memBuffer[0x12600 + j];
		memBuffer[0x13400 + j] = memBuffer[0x12a00 + j];
	}

	for (int i = 6; i >= 0; i--) {
		//debug("copying 0x200 bytes to %x from %x", 0x12000 + 0x200*i, 0x11400 + 0x400*i);
		for (int j = 0; j < 0x200; j++) {
			memBuffer[0x12000 + 0x200*i + j] = memBuffer[0x11400 + 0x400*i + j];
		}
	}

	for (int j = 0; j < 0x200; j++) {
		memBuffer[0x11c00 + j] = memBuffer[0x11e00 + j];
		memBuffer[0x11e00 + j] = memBuffer[0x11000 + j];
	}
}

byte kCPCPaletteTitleData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0x80, 0xff},
	{0xff, 0x00, 0x00},
	{0xff, 0xff, 0x00},
};

byte kCPCPaletteBorderData[4][3] = {
	{0x00, 0x00, 0x00},
	{0xff, 0x80, 0x00},
	{0x80, 0xff, 0xff},
	{0x00, 0x80, 0x00},
};

Graphics::ManagedSurface *readCPCImage(Common::SeekableReadStream *file) {
	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, 320, 200), 0);

	int x, y;
	file->seek(0x80);
	for (int block = 0; block < 8; block++) {
		for (int line = 0; line < 25; line++) {
			for (int offset = 0; offset < 320 / 4; offset++) {
				byte cpc_byte = file->readByte(); // Get CPC byte

				// Process first pixel
				int pixel_0 = ((cpc_byte & 0x08) >> 2) | ((cpc_byte & 0x80) >> 7); // %Aa
				y = line * 8 + block ; // Coord Y for the pixel
				x = 4 * offset + 0; // Coord X for the pixel
				surface->setPixel(x, y, pixel_0);

				// Process second pixel
				int pixel_1 = ((cpc_byte & 0x04) >> 1) | ((cpc_byte & 0x40) >> 6); // %Bb
				y = line * 8 + block ; // Coord Y for the pixel
				x = 4 * offset + 1; // Coord X for the pixel
				surface->setPixel(x, y, pixel_1);

				// Process third pixel
				int pixel_2 = (cpc_byte & 0x02)        | ((cpc_byte & 0x20) >> 5); // %Cc
				y = line * 8 + block ; // Coord Y for the pixel
				x = 4 * offset + 2; // Coord X for the pixel
				surface->setPixel(x, y, pixel_2);

				// Process fourth pixel
				int pixel_3 = ((cpc_byte & 0x01) << 1) | ((cpc_byte & 0x10) >> 4); // %Dd
				y = line * 8 + block ; // Coord Y for the pixel
				x = 4 * offset + 3; // Coord X for the pixel
				surface->setPixel(x, y, pixel_3);
			}
		}
		// We should skip the next 48 bytes, because they are padding the block to be 2048 bytes
		file->seek(48, SEEK_CUR);
	}
	return surface;
}

void DrillerEngine::loadAssetsCPCFullGame() {
	Common::File file;

	file.open("DSCN1.BIN");
	if (!file.isOpen())
		error("Failed to open DSCN1.BIN");

	_title = readCPCImage(&file);
	_title->setPalette((byte*)&kCPCPaletteTitleData, 0, 4);

	file.close();
	file.open("DSCN2.BIN");
	if (!file.isOpen())
		error("Failed to open DSCN2.BIN");

	_border = readCPCImage(&file);
	_border->setPalette((byte*)&kCPCPaletteBorderData, 0, 4);

	file.close();
	file.open("DRILL.BIN");

	if (!file.isOpen())
		error("Failed to open DRILL.BIN");

	loadMessagesFixedSize(&file, 0x214c, 14, 20);
	loadFonts(&file, 0x5b69);
	loadGlobalObjects(&file, 0x1d07);
	load8bitBinary(&file, 0x5ccb, 16);
}

void DrillerEngine::drawCPCUI(Graphics::Surface *surface) {
	uint32 color = 1;
	uint8 r, g, b;

	_gfx->selectColorFromFourColorPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 0;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 200, 185, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.x())), 150, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.z())), 150, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", int(2 * _position.y())), 150, 161, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 54, 161, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 54, 161, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 46, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 44, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 239, 129, front, back, surface);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	drawStringInSurface(Common::String::format("%02d", hours), 209, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", minutes), 232, 8, front, back, surface);
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 8, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 191, 177, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (_messagesList.size() > 0) {
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
		Common::Rect backBar(25, 184, 89 - energy, 191);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(88 - energy, 184, 88, 191);
		surface->fillRect(energyBar, front);
	}

	if (shield >= 0) {
		Common::Rect backBar(25, 177, 89 - shield, 183);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(88 - shield, 177, 88, 183);
		surface->fillRect(shieldBar, front);
	}
}

} // End of namespace Freescape