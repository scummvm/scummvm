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

Common::SeekableReadStream *DarkEngine::decryptFile(const Common::Path &packed, const Common::Path &unpacker, uint32 unpackArrayOffset) {
	Common::File file;
	file.open(packed);
	if (!file.isOpen())
		error("Failed to open %s", packed.toString().c_str());

	int size = file.size();
	byte *encryptedBuffer = (byte *)malloc(size);
	file.read(encryptedBuffer, size);
	file.close();

	uint32 d7 = 0;
	uint32 d6 = 0;
	byte *a6 = encryptedBuffer;
	byte *a5 = encryptedBuffer + size - 1;

	while (a6 <= a5) {
		uint64 d0 = (a6[0] << 24) | (a6[1] << 16) | (a6[2] << 8) | a6[3];
		d0 = d0 + d6;
		d0 = uint32(d0);
		d0 = ((d0 << 3) & 0xFFFFFFFF) | ((d0 >> 29) & 0xFFFFFFFF);
		d0 ^= 0x71049763;
		d0 -= d7;
		d0 = ((d0 << 16) & 0xFFFF0000) | ((d0 >> 16) & 0xFFFF);

		a6[0] = byte((d0 >> 24) & 0xFF);
		//debug("%c", a6[0]);
		a6[1] = byte((d0 >> 16) & 0xFF);
		//debug("%c", a6[1]);
		a6[2] = byte((d0 >> 8) & 0xFF);
		//debug("%c", a6[2]);
		a6[3] = byte(d0 & 0xFF);
		//debug("%c", a6[3]);

		d6 += 5;
		d6 = ((d6 >> 3) & 0xFFFFFFFF) | ((d6 << 29) & 0xFFFFFFFF);
		d6 ^= 0x04000000;
		d7 += 4;
		a6 += 4;
	}

	file.open(unpacker);
	if (!file.isOpen())
		error("Failed to open %s", unpacker.toString().c_str());

	int originalSize = size;
	size = file.size();
	byte *unpackArray = (byte *)malloc(size);
	file.read(unpackArray, size);
	file.close();

	byte *unpackArrayPtr = unpackArray + unpackArrayOffset;
	uint32 i = 2 * 1024;
	do {
		uint8 ptr0 = unpackArrayPtr[2 * i];
		uint8 ptr1 = unpackArrayPtr[2 * i + 1];
		uint8 val0 = unpackArrayPtr[2 * (i - 1)];
		uint8 val1 = unpackArrayPtr[2 * (i - 1) + 1];

		encryptedBuffer[2 * (ptr1 + 256 * ptr0)] = val0;
		encryptedBuffer[2 * (ptr1 + 256 * ptr0) + 1] = val1;

		i = i - 2;
	} while (i > 0);

	return (new Common::MemoryReadStream(encryptedBuffer, originalSize));
}

void DarkEngine::parseAmigaAtariHeader(Common::SeekableReadStream *stream) {
	stream->seek(0x22);
	int size = stream->readUint16BE();
	debugC(1, kFreescapeDebugParser, "Header table size %d", size);
	for (int i = 0; i < size; i++) {
		debugC(1, kFreescapeDebugParser, "Location: %x ", stream->readUint32BE());
		Common::String filename;
		while (char c = stream->readByte())
			filename += c;

		for (int j = filename.size() + 1; j < 16; j++)
			stream->readByte();

		debugC(1, kFreescapeDebugParser, "Filename: %s", filename.c_str());
	}
}

void DarkEngine::loadAssetsAmigaFullGame() {
	Common::SeekableReadStream *stream = decryptFile("1.drk", "0.drk", 798);
	parseAmigaAtariHeader(stream);

	_border = loadAndConvertNeoImage(stream, 0x1b762);
	load8bitBinary(stream, 0x2e96a, 16);
	loadPalettes(stream, 0x2e528);
	loadGlobalObjects(stream, 0x30f0 - 50, 24);
	loadMessagesVariableSize(stream, 0x3d37, 66);
}

void DarkEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0xCC, 0x00);
	uint32 orange = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0x88, 0x00);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xEE, 0x00, 0x00);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	uint32 grey = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x60, 0x60, 0x60);

	int score = _gameStateVars[k8bitVariableScore];
	int ecds = _gameStateVars[kVariableActiveECDs];
	drawString(kDarkFontSmall, Common::String::format("%04d", int(2 * _position.x())), 18, 178, red, red, black, surface);
	drawString(kDarkFontSmall, Common::String::format("%04d", int(2 * _position.z())), 18, 184, red, red, black, surface);
	drawString(kDarkFontSmall, Common::String::format("%04d", int(2 * _position.y())), 18, 190, red, red, black, surface);

	drawString(kDarkFontBig, Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 73, 178, red, red, black, surface);
	drawString(kDarkFontBig, Common::String::format("%3d", _playerSteps[_playerStepIndex]), 73, 186, red, red, black, surface);
	drawString(kDarkFontBig, Common::String::format("%07d", score), 93, 16, yellow, orange, black, surface);
	drawString(kDarkFontBig, Common::String::format("%3d%%", ecds), 181, 16, yellow, orange, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawString(kDarkFontSmall, message, 32, 157, white, white, transparent, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	}

	drawString(kDarkFontSmall, _currentArea->_name, 32, 151, white, white, transparent, surface);
	drawBinaryClock(surface, 6, 110, white, grey);

	int x = 229;
	int y = 180;
	for (int i = 0; i < _maxShield / 2; i++) {
		if (i < _gameStateVars[k8bitVariableShield] / 2) {
			surface->drawLine(x, y, x, y + 3, orange);
			surface->drawLine(x, y + 1, x, y + 2, yellow);
		} else
			surface->drawLine(x, y, x, y + 3, red);
		x += 2;
	}

	x = 229;
	y = 188;
	for (int i = 0; i < _maxEnergy / 2; i++) {
		if (i < _gameStateVars[k8bitVariableEnergy] / 2) {
			surface->drawLine(x, y, x, y + 3, orange);
			surface->drawLine(x, y + 1, x, y + 2, yellow);
		} else
			surface->drawLine(x, y, x, y + 3, red);
		x += 2;
	}
}

void DarkEngine::initAmigaAtari() {
	_viewArea = Common::Rect(32, 33, 287, 130);
}

void DarkEngine::drawString(const DarkFontSize size, const Common::String &str, int x, int y, uint32 primaryColor, uint32 secondaryColor, uint32 backColor, Graphics::Surface *surface) {
	if (!_fontLoaded)
		return;
	Common::String ustr = str;
	ustr.toUppercase();

	int multiplier1 = 0;
	int multiplier2 = 0;
	int sizeX = 0;
	int sizeY = 0;
	int sep = 0;

	switch (size) {
		case kDarkFontBig:
			multiplier1 = 16;
			multiplier2 = 16;
			sizeY = 8;
			sizeX = 8;
			sep = 8;
			_font = _fontBig;
		break;
		case kDarkFontMedium:
			multiplier1 = 10;
			multiplier2 = 8;
			sizeY = 8;
			sizeX = 8;
			sep = 8;
			_font = _fontMedium;
		break;
		case kDarkFontSmall:
			multiplier1 = 10;
			multiplier2 = 16;
			sizeY = 5;
			sizeX = 8;
			sep = 4;
			_font = _fontSmall;
		break;
		default:
			error("Invalid font size %d", size);
		break;
	}

	for (uint32 c = 0; c < ustr.size(); c++) {
		assert(ustr[c] >= 32);
		int position = 8 * (multiplier1*(ustr[c] - 32));
		for (int j = 0; j < sizeY; j++) {
			for (int i = 0; i < sizeX; i++) {
				if (_font.get(position + j * multiplier2 + i)) {
					surface->setPixel(x + 8 - i + sep * c, y + j, primaryColor);
				} /*else if (_font.get(position + j * multiplier2 + i)) {
					surface->setPixel(x + 8 - i + 8 * c, y + j, primaryColor);
				}*/ else {
					surface->setPixel(x + 8 - i + sep * c, y + j, backColor);
				}
			}
		}
	}
}

} // End of namespace Freescape
