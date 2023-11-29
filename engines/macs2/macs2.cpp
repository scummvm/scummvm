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

#include "macs2/macs2.h"
#include "macs2/detection.h"
#include "macs2/console.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"

namespace Macs2 {

Macs2Engine *g_engine;

void Macs2Engine::readResourceFile() {
	Common::File file;
	if (!file.open("RESOURCE.MCS"))
		error("readResourceFile(): Error reading MCS file");

	file.seek(0x23BE09);

	
	// file.read(data, 320 * 240);

	// uint8 b0 = data[0];

	_bgImageShip = new Graphics::Surface();
	_bgImageShip->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	uint8* lengthData = new uint8[2];
	uint8* data = new uint8[320];

	// TODO: Consider if it can be that the data is more than this. Maybe the tooling of the engine can make bad calls and
	// try to RLE something which would be better not RLE encoded.
	
	for (int y = 0; y < 200; y++) {
		// TODO: Use the proper read function, it seems to be available
		file.read(lengthData, 2);
		uint16 length = lengthData[1] << 8 | lengthData[0];
		file.read(data, length);
		uint16 remainingPixels = 320;
		uint8* dataPointer = data;
		uint16 x = 0;
		while (remainingPixels > 0) {
			const uint8& value = dataPointer[0];
			dataPointer++;
			if (value != 0xF0) {
				_bgImageShip->setPixel(x, y, value);
				remainingPixels--;
				x++;
			}
			else {
				// We need to decode the RLE data
				const uint8& runlength = dataPointer[0];
				dataPointer++;
				const uint8& encodedValue = dataPointer[0];
				dataPointer++;
				for (int i = 0; i < runlength; i++) {
					_bgImageShip->setPixel(x++, y, encodedValue);
				}
				remainingPixels -= runlength;
			}
		}
	}

	// Load the palette
	file.seek(0x00248BCB);
	file.read(_pal, 256 * 3);

	// Adjust the palette
	for (int i = 0; i < 256 * 3; i++) {
		_pal[i] = (_pal[i] * 259 + 33) >> 6;
	}

	// return check_cast<uint8>((c * 259 + 33) >> 6);

	// Iterate over 0 to 199 for each row
	// Load the amount of bytes for the row
	// Load the row data and do run-length decoding
	// Save into the right row of the surface


	// Load the data for a character

	file.seek(0x6FB2);
	_charASCII = file.readByte();
	_charWidth = file.readUint16LE();
	_charHeight = file.readUint16LE();
	_charData = new byte[_charWidth * _charHeight];

	
	file.read(_charData, _charWidth * _charHeight);

	// Load the data for a border part
	file.seek(0x64C6);

	_borderWidth = file.readUint16LE();
	_borderHeight = file.readUint16LE();
	_borderData = new byte[_borderWidth * _borderHeight];
	file.read(_borderData, _borderWidth * _borderHeight);

	// Load the script
	file.seek(0x000A3B98);
	uint16 scriptLength = file.readUint16LE();
	_scriptData = new byte[scriptLength];
	file.read(_scriptData, scriptLength);
	_scriptStream = new Common::MemoryReadStream(_scriptData, scriptLength);
}

Macs2Engine::Macs2Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Macs2") {
	g_engine = this;
}

Macs2Engine::~Macs2Engine() {
}

void Macs2Engine::ExecuteScript(Common::MemoryReadStream* stream) {
	// TODO: Change to a proper end condition
	for (;;) {
		// Read an opcode (would be 0037:9F07)
		byte opcode1 = stream->readByte();

		// Read another value - TODO: Not sure yet what this does
		byte val1 = stream->readByte();
	}
}

uint32 Macs2Engine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String Macs2Engine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error Macs2Engine::run() {

	readResourceFile();

	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);

	

	// Set the engine's debugger console
	setDebugger(new Console());

	runGame();

	return Common::kNoError;
}

Common::Error Macs2Engine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Macs2
