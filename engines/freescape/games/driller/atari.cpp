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
#include "freescape/games/driller/driller.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

Common::SeekableReadStream *DrillerEngine::decryptFileAtari(const Common::Path &filename) {
	Common::File file;
	file.open(filename);
	if (!file.isOpen())
		error("Failed to open %s", filename.toString().c_str());

	int size = file.size();
	byte *encryptedBuffer = (byte *)malloc(size);
	file.read(encryptedBuffer, size);
	file.close();

	byte *a6 = encryptedBuffer + 0x118;
	byte *a5 = encryptedBuffer + size - 4;
	uint64 key = 0xb9f11bce;

	while (a6 <= a5) {
		uint64 d0 = (a6[0] << 24) | (a6[1] << 16) | (a6[2] << 8) | a6[3];
		d0 += key;
		d0 = uint32(d0);

		a6[0] = byte((d0 >> 24) & 0xFF);
		a6[1] = byte((d0 >> 16) & 0xFF);
		a6[2] = byte((d0 >> 8) & 0xFF);
		a6[3] = byte(d0 & 0xFF);

		key += 0x51684624;
		key = uint32(key);
		a6 += 4;
	}

	return (new Common::MemoryReadStream(encryptedBuffer, size));
}

void DrillerEngine::loadAssetsAtariFullGame() {

	if (_variant & GF_ATARI_RETAIL) {
		Common::SeekableReadStream *stream = decryptFileAtari("x.prg");

		_border = loadAndConvertNeoImage(stream, 0x14b96);
		_borderExtra = loadAndConvertNeoImage(stream, 0x1c916);
		_title = loadAndConvertNeoImage(stream, 0x3f6);

		loadFonts(stream, 0x8a92);
		loadMessagesFixedSize(stream, 0xda22, 14, 20);
		loadGlobalObjects(stream, 0xd116, 8);
		load8bitBinary(stream, 0x2afb8, 16);
		loadPalettes(stream, 0x2ab76);
		//loadSoundsFx(&file, 0x30da6, 25);
	} else if (_variant & GF_ATARI_BUDGET) {
		Common::File file;
		file.open("x.prg");

		if (!file.isOpen())
			error("Failed to open 'x.prg' executable for AtariST");

		if (isSpaceStationOblivion()) {
			_border = loadAndConvertNeoImage(&file, 0x13544);
			byte *palette = (byte *)malloc(16 * 3);
			for (int i = 0; i < 16; i++) { // gray scale palette
				palette[i * 3 + 0] = i * (255 / 16);
				palette[i * 3 + 1] = i * (255 / 16);
				palette[i * 3 + 2] = i * (255 / 16);
			}
			_title = loadAndConvertNeoImage(&file, 0x10, palette);

			loadFonts(&file, 0x8a32 - 0x1d6);
			loadMessagesFixedSize(&file, 0xc5d8 - 0x1da, 14, 20);
			loadGlobalObjects(&file, 0xbccc - 0x1da, 8);
			load8bitBinary(&file, 0x29b3c - 0x1d6, 16);
			loadPalettes(&file, 0x296fa - 0x1d6);
			loadSoundsFx(&file, 0x30da6 - 0x1d6, 25);
		} else {
			_border = loadAndConvertNeoImage(&file, 0x1371a);
			_title = loadAndConvertNeoImage(&file, 0x396);

			loadFonts(&file, 0x8a32);
			loadMessagesFixedSize(&file, 0xc5d8, 14, 20);
			loadGlobalObjects(&file, 0xbccc, 8);
			load8bitBinary(&file, 0x29b3c, 16);
			loadPalettes(&file, 0x296fa);
			loadSoundsFx(&file, 0x30da6, 25);
		}
	}
}

void DrillerEngine::loadAssetsAtariDemo() {
	Common::File file;
	file.open("lift.neo");
	if (!file.isOpen())
		error("Failed to open 'lift.neo' file");

	_title = loadAndConvertNeoImage(&file, 0);

	file.close();
	file.open("console.neo");
	if (!file.isOpen())
		error("Failed to open 'console.neo' file");

	_border = loadAndConvertNeoImage(&file, 0);

	file.close();
	file.open("demo.cmd");
	if (!file.isOpen())
		error("Failed to open 'demo.cmd' file");

	loadDemoData(&file, 0, 0x1000);

	file.close();
	file.open("data");

	if (!file.isOpen())
		error("Failed to open 'data' file");

	load8bitBinary(&file, 0x442, 16);
	loadPalettes(&file, 0x0);

	file.close();
	if (_variant & GF_ATARI_MAGAZINE_DEMO) {
		file.open("auto_x.prg");
		if (!file.isOpen())
			error("Failed to open 'auto_x.prg' file");
		_demoMode = false;
	} else {
		file.open("x.prg");
		if (!file.isOpen())
			error("Failed to open 'x.prg' file");
	}

	if (_variant & GF_ATARI_MAGAZINE_DEMO) {
		loadFonts(&file, 0x7ee);
		loadMessagesFixedSize(&file, 0x40d2, 14, 20);
		loadGlobalObjects(&file, 0x3e88, 8);
	} else {
		loadFonts(&file, 0x7bc);
		loadMessagesFixedSize(&file, 0x3b90, 14, 20);
		loadGlobalObjects(&file, 0x3946, 8);
	}

	file.close();
	file.open("soundfx");
	if (!file.isOpen())
		error("Failed to open 'soundfx' executable for AtariST demo");

	loadSoundsFx(&file, 0, 25);
}

} // End of namespace Freescape
