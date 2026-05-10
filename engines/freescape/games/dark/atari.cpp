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
#include "freescape/games/dark/dark.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void DarkEngine::loadAssetsAtariFullGame() {
	Common::File file;
	file.open("0.drk");
	_title = loadAndConvertNeoImage(&file, 0x13ec);

	// Atari ST Dark Side: same COLOR5 cycling as Amiga.
	{
		static const uint16 kDarkSideCyclingTable[] = {
			0x000, 0xE6D, 0x600, 0x900, 0xC00, 0xF00, 0xF30, 0xF60,
			0xF90, 0xFC0, 0xFF0, 0xAF0, 0x5F0, 0x6F8, 0x7FD, 0x7EF,
			0xBDF, 0xDDF, 0xBCF, 0x9BF, 0x7BF, 0x6BF, 0x5AF, 0x4AF,
			0x29F, 0x18F, 0x07F, 0x04C, 0x02A, 0x007
		};
		for (int i = 0; i < 30; i++)
			_gfx->_colorCyclingTable.push_back(kDarkSideCyclingTable[i]);
	}
	_gfx->_colorCyclingPaletteIndex = 5;
	_gfx->_colorCyclingSpeed = 1;
	_gfx->_colorCyclingTimer = 0;

	file.close();

	Common::SeekableReadStream *stream = decryptFileAmigaAtari("1.drk", "0.drk", 840);
	parseAmigaAtariHeader(stream);

	_border = loadAndConvertNeoImage(stream, 0xd710);

	Common::Array<Graphics::ManagedSurface *> chars;
	chars = getCharsAmigaAtariInternal(8, 8, - 7 - 8, 16, 16, stream, 0xd06a, 85);
	_fontBig = Font(chars);

	chars = getCharsAmigaAtariInternal(8, 8, 0, 10, 8, stream, 0xd49a, 85);
	_fontMedium = Font(chars);

	chars = getCharsAmigaAtariInternal(8, 5, - 7 - 8, 10, 16, stream, 0xd49a, 85);
	_fontSmall = Font(chars);
	_fontSmall.setCharWidth(4);

	_fontLoaded = true;
	load8bitBinary(stream, 0x20918, 16);
	loadMessagesVariableSize(stream, 0x3f6f, 66);
	loadPalettes(stream, 0x204d6);
	loadGlobalObjects(stream, 0x32f6, 24);
	loadSoundsFx(stream, 0x266e8, 11);

	for (auto &area : _areaMap) {
		// Center and pad each area name so we do not have to do it at each frame
		area._value->_name = centerAndPadString(area._value->_name, 26);
	}
}

} // End of namespace Freescape
