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

#include "common/system.h"
#include "chewy/chewy.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/file.h"
#include "chewy/io_game.h"

namespace Chewy {

IOGame::IOGame(McgaGraphics *out, InputMgr *in, Cursor *cur) {
	_out = out;
	_in = in;
	_cur = cur;
}

char *IOGame::io_init(iog_init *iostruc) {
	_io = iostruc;
	for (int16 i = 0; i < 20; i++)
		_fileFind[i][0] = 0;
	get_savegame_files();

	return &_fileFind[0][0];
}

void IOGame::save_entry(int16 slotNum, char *fname) {
	Common::String desc(&_fileFind[slotNum][1]);
	g_engine->saveGameState(slotNum, desc);
}

void IOGame::load(int16 slotNum, char *fname) {
	get_savegame_files();

	_cur->hide_cur();
	if (_fileFind[slotNum][0] == 1) {
		(void)g_engine->loadGameState(slotNum);
	}
}

int16 IOGame::get_savegame_files() {
	SaveStateList saveList = g_engine->listSaves();
	int ret = 0;

	for (int i = 0; i < 20; i++) {
		_fileFind[i][0] = 0;

		for (uint j = 0; j < saveList.size(); ++j) {
			if (saveList[j].getSaveSlot() == i) {
				Common::String name = saveList[j].getDescription();
				_fileFind[i][0] = 1;
				strncpy(&_fileFind[i][1], name.c_str(), USER_NAME + 3);
				_fileFind[i][USER_NAME + 3] = '\0';
				++ret;
				break;
			}
		}
	}

	return ret;
}

} // namespace Chewy
