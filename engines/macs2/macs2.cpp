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
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"

namespace Macs2 {

Macs2Engine *g_engine;

Graphics::ManagedSurface Macs2Engine::readRLEImage(int64 offs, Common::File& file)
{
	file.seek(offs);

	Graphics::ManagedSurface result = new Graphics::Surface();
	result.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	

	// TODO: Fix length
	uint8* data = new uint8[1024];

	// TODO: Consider if it can be that the data is more than this. Maybe the tooling of the engine can make bad calls and
	// try to RLE something which would be better not RLE encoded.

	// TODO: Fix start position
	for (int y = 0; y < 200; y++) {
		uint16 length = file.readUint16LE();
		file.read(data, length);
		uint16 remainingPixels = 320;
		uint8* dataPointer = data;
		uint16 x = 0;
		while (remainingPixels > 0) {
			const uint8& value = dataPointer[0];
			dataPointer++;
			if (value != 0xF0) {
				/* if (x >= 320) {
					// TODO: This happens during loading the map
					break;
				}*/
				result.setPixel(x, y, value);
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
					/* if (x >= 320) {
						// TODO: This happens for loading the map - maybe this code is a bit different?
						break;
					} */
					result.setPixel(x++, y, encodedValue);
				}
				remainingPixels -= runlength;
			}
		}
	}

	return result;
}



void Macs2Engine::readResourceFile() {
	Common::File file;
	if (!file.open("RESOURCE.MCS"))
		error("readResourceFile(): Error reading MCS file");

	file.seek(0x23BE09);


	// file.read(data, 320 * 240);

	// uint8 b0 = data[0];

	_bgImageShip.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

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
				_bgImageShip.setPixel(x, y, value);
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
					_bgImageShip.setPixel(x++, y, encodedValue);
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

	// And the highlight part
	file.seek(0x6962);

	_borderHighlightWidth = file.readUint16LE();
	_borderHighlightHeight = file.readUint16LE();
	_borderHighlightData = new byte[_borderHighlightWidth * _borderHighlightHeight];
	file.read(_borderHighlightData, _borderHighlightWidth * _borderHighlightHeight);

	// The flag animation frames
	file.seek(0x00250D47);
	_flagData = new byte * [3];
	_flagWidths = new uint16[3];
	_flagHeights = new uint16[3];
	_flagWidths[0] = file.readUint16LE();
	_flagHeights[0] = file.readUint16LE();
	_flagData[0] = new byte[_flagWidths[0] * _flagHeights[0]];
	file.read(_flagData[0], _flagWidths[0] * _flagHeights[0]);

	file.seek(0x00250B3C);
	_flagWidths[1] = file.readUint16LE();
	_flagHeights[1] = file.readUint16LE();
	_flagData[1] = new byte[_flagWidths[1] * _flagHeights[1]];
	file.read(_flagData[1], _flagWidths[1] * _flagHeights[1]);

	file.seek(0x00250931);
	_flagWidths[2] = file.readUint16LE();
	_flagHeights[2] = file.readUint16LE();
	_flagData[2] = new byte[_flagWidths[2] * _flagHeights[2]];
	file.read(_flagData[2], _flagWidths[2] * _flagHeights[2]);

	// Load the script
	file.seek(0x000A3B98);
	uint16 scriptLength = file.readUint16LE();
	_scriptData = new byte[scriptLength];
	file.read(_scriptData, scriptLength);
	_scriptStream = new Common::MemoryReadStream(_scriptData, scriptLength);

	// Try executing the script
	ExecuteScript(_scriptStream);

	// Load the background map
	// _map = readRLEImage(0x0024BD9B, file);
	// _map = readRLEImage(0x00248FCE, file);
	_map = readRLEImage(0x0024B0DF, file);

	// Load the data for the mouse cursor

	// TODO: Figure out which is the correct one
	// file.seek(0x00003C74);
	file.seek(0x0000524A);
	_cursorWidth = file.readUint16LE();
	_cursorHeight = file.readUint16LE();
	_cursorData = new byte[_cursorWidth * _cursorHeight];
	file.read(_cursorData, _cursorWidth* _cursorHeight);

	// Load the shading table
	file.seek(0x00248ECB);
	_shadingTable = new byte[256];
	file.read(_shadingTable, 256);

}

Macs2Engine::Macs2Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Macs2") {
	g_engine = this;
}

Macs2Engine::~Macs2Engine() {
}

void Func9F4D(Common::MemoryReadStream * stream, uint16& out1, uint16& out2) {
	// Read an opcode (would be 0037:9F07) - [bp-1h]
	byte opcode = stream->readByte();
	debug("Script read (byte): %.2x at offset %.4x\n", opcode, stream->pos());

	uint16 value = stream->readUint16LE();
	debug("Script read (word): %.4x at offset %.4x\n", value, stream->pos());

	if (opcode == 0xFF) {
		// TODO: Long list of opcode handling here
		if (value == 0x26) {
			// TODO: Implement this part:
	/* l0037_A19E:
		cmp	byte ptr[1014h], 0h
			jz	0A1B1h

			l0037_A1A5 :
		mov	word ptr[bp - 4h], 1h
			mov	word ptr[bp - 2h], 0h
			jmp	0A1B9h

			l0037_A1B1 :
		xor ax, ax
			mov[bp - 4h], ax
			mov[bp - 2h], ax */

			// For now just returning 0 by default
			out1 = 0;
			out2 = 0;
			return;
		}
	}
}

byte ScriptReadByte(Common::MemoryReadStream* stream) {
	const int64 pos = stream->pos();
	const byte result = stream->readByte();
	debug("Script read (byte): %.2x at offset %.4x\n", result, pos);
	return result;
}

void FuncA3D2(Common::MemoryReadStream* stream) {
	
	uint16 skipValue = 1; // [bp-4h] - TODO: Better name
	// TODO: Figure out end condition
	for (;;) {
		const byte opcode = ScriptReadByte(stream);
		const byte val = ScriptReadByte(stream);
		if (opcode > 3) {
			if (opcode <= 6) {
				skipValue++;
			}
			if (opcode == 8) {
				if (skipValue == 1) {
					skipValue--;
				}
			}
			if (opcode == 7) {
				skipValue--;
			}
			// Do the skipping
			stream->seek(val, SEEK_CUR);
			debug("A3D2 skipping %u bytes for opcode %.2x (%u)", val, opcode, skipValue);
			// TODO: Add a log here
			if (skipValue != 0) {
				// Continue the loop if there is data left in the stream
			}
			else {
				if (skipValue != 0) {
					// TODO: Implement:
					// mov	word ptr [1028h],1Dh
					// TODO: Add an assert here to see if this ever happens in practice
				}
				break;
			}
			// TODO: Continue here
		}
		else {
			break;
		}
	}
}




void Macs2Engine::ExecuteScript(Common::MemoryReadStream* stream) {
	// Implements roughly 01E7:DB56 and friends
	// TODO: Change to a proper end condition
	// TODO: Do some bookkeeping on the pointers into the script
	for (;;) {
		

		// l0037_DB89:


		// Read an opcode (would be 0037:9F07) - [bp-1h]
		byte opcode1 = ScriptReadByte(stream);

		// Read another value - TODO: Not sure yet what this does - [bp-2h]
		byte val1 = ScriptReadByte(stream);

		// TODO: Handle other opcodes above
		if (opcode1 == 0x01) {
			assert(false);
			// TODO: Implement - this looks like some kind of bookkeeping since it doesn't determine if we continue or not?
			// TODO: Maybe skip for now and note if it should be implemented later
				/*
				call	far 0037h:9F07h
				call	far 0037h : 9F23h
				mov[bp - 11h], ax
				call	far 0037h : 9F4Dh
				mov	cx, ax
				mov	bx, dx
				mov	ax, [bp - 11h]
				shl	ax, 2h
				les	di, [06C6h]
				add	di, ax
				mov	es : [di - 4h] , cx
				mov	es : [di - 2h] , bx
				// This jumps ahead to the end of the loop
				jmp	0E3BAh
				*/
		}
		else if (opcode1 == 0x02 || opcode1 == 0x03) {
			// TODO: Implement
			assert(false);
		}
		if (opcode1 == 0x04) {
			// l0037_DC44:
			uint16 v1;
			uint16 v2;
			Func9F4D(stream, v1, v2);
			if ((v1 | v2) == 0) {
				FuncA3D2(stream);
			}
			else {
				// TODO: Implement
				assert(false);
			}
		}
		else {
			break;
		}

		// TODO: Handle other opcodes below
		

		
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

void GlyphData::ReadFromeFile(Common::File &file) {
	// TODO: Implement
}

} // End of namespace Macs2
