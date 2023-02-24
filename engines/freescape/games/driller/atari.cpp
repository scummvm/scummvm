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

void DrillerEngine::loadAssetsAtariFullGame() {
	Common::File file;
	file.open("x.prg");

	if (!file.isOpen())
		error("Failed to open 'x.prg' executable for AtariST");

	_border = loadAndConvertNeoImage(&file, 0x1371a);
	byte *palette = (byte *)malloc(16 * 3);
	for (int i = 0; i < 16; i++) { // gray scale palette
		palette[i * 3 + 0] = i * (255 / 16);
		palette[i * 3 + 1] = i * (255 / 16);
		palette[i * 3 + 2] = i * (255 / 16);
	}
	_title = loadAndConvertNeoImage(&file, 0x10, palette);

	loadFonts(&file, 0x8a32);
	loadMessagesFixedSize(&file, 0xc5d8, 14, 20);
	loadGlobalObjects(&file, 0xbccc);
	load8bitBinary(&file, 0x29b3c, 16);
	loadPalettes(&file, 0x296fa);
	loadSoundsFx(&file, 0x30da6, 25);
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
	file.open("x.prg");
	if (!file.isOpen())
		error("Failed to open 'x.prg' file");

	loadFonts(&file, 0x7bc);
	loadMessagesFixedSize(&file, 0x3b90, 14, 20);
	loadGlobalObjects(&file, 0x3946);

	file.close();
	file.open("soundfx");
	if (!file.isOpen())
		error("Failed to open 'soundfx' executable for AtariST demo");

	loadSoundsFx(&file, 0, 25);
}

} // End of namespace Freescape