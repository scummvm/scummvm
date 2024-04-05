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

void DarkEngine::loadAssetsAtariFullGame() {
	Common::SeekableReadStream *stream = decryptFile("1.drk", "0.drk", 840);
	parseAmigaAtariHeader(stream);

	_border = loadAndConvertNeoImage(stream, 0xd710);
	load8bitBinary(stream, 0x20918, 16);
	loadMessagesVariableSize(stream, 0x3f6f, 66);
	loadPalettes(stream, 0x205e6);
	loadGlobalObjects(stream, 0x32f6, 24);

	for (auto &it : _areaMap) {
		addWalls(it._value);
		addECDs(it._value);
		addSkanner(it._value);
	}
}

} // End of namespace Freescape
