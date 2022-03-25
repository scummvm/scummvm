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

#include "chewy/chewy.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/io_game.h"

namespace Chewy {

Common::StringArray &IOGame::io_init() {
	SaveStateList saveList = g_engine->listSaves();

	_fileFind.resize(1000);

	for (int i = 0; i < saveList.size(); i++) {
		_fileFind[i] = "";

		for (uint j = 0; j < saveList.size(); ++j) {
			if (saveList[j].getSaveSlot() == i) {
				Common::String name = saveList[j].getDescription();
				_fileFind[i] = name;
				break;
			}
		}
	}

	return _fileFind;
}

void IOGame::save_entry(int16 slotNum) {
	Common::String desc = _fileFind[slotNum];
	g_engine->saveGameState(slotNum, desc);
}

} // namespace Chewy
