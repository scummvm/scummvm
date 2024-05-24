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
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void CastleEngine::loadAssetsAmigaDemo() {
	Common::File file;
	file.open("x");
	if (!file.isOpen())
		error("Failed to open 'x' file");

	loadMessagesVariableSize(&file, 0x8bb2, 178);
	loadRiddles(&file, 0x96c8, 20);
	load8bitBinary(&file, 0x162a6, 16);
	loadPalettes(&file, 0x151a6);

	file.seek(0x2be96); // Area 255
	_areaMap[255] = load8bitArea(&file, 16);
	file.close();

	_areaMap[2]->_groundColor = 1;
	for (auto &it : _areaMap)
		it._value->addStructure(_areaMap[255]);

	_areaMap[1]->addFloor();
	_areaMap[2]->addFloor();
}

} // End of namespace Freescape